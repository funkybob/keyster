#include <stdlib.h>
#include <stdio.h>

#include <ctype.h>  // isspace

#include <netinet/in.h>
#include <arpa/inet.h>
#include <uv.h>
#include "sglib.h"

typedef struct node {
    struct node *left;
    struct node *right;
    char colour_field;

    size_t data_len;
    char *data;
    char *key;
} node;

#define CMPARATOR(x,y) (strcmp(x->key, y->key))

SGLIB_DEFINE_RBTREE_PROTOTYPES(node, left, right, colour_field, CMPARATOR);
SGLIB_DEFINE_RBTREE_FUNCTIONS(node, left, right, colour_field, CMPARATOR);

node *root;

static void alloc_cb(uv_handle_t *handle, size_t size, uv_buf_t *buf) {
  /* libuv suggests a buffer size but leaves it up to us to create one of any size we see fit */
  buf->base = malloc(size);
  buf->len = size;
  // if (buf->base == NULL) log_error("alloc_cb buffer didn't properly initialize");
}

static void on_send(uv_udp_send_t* req, int status) {
    char *buf = req->data;
    free(buf);
    free(req);
}

static void send_reply(uv_udp_t* handle, char cmd, char* key, char *data, size_t data_len, const struct sockaddr* addr) {
    size_t key_len, total_len;
    char *buffer;
    uv_buf_t buf;
    uv_udp_send_t *req;

    key_len = strlen(key);
    total_len = key_len + data_len + 2; // cmd + key + nul + data
    buffer = malloc(total_len);
    buffer[0] = cmd;
    strcpy(&buffer[1], key);
    if(data_len > 0) {
        memcpy(&buffer[key_len+2], data, data_len);
    }
    buf = uv_buf_init(buffer, total_len);

    req = malloc(sizeof(uv_udp_send_t));
    req->data = (void *)buf.base;

    uv_udp_send(req, handle, &buf, 1, addr, on_send);
}

static void on_recv(uv_udp_t* handle, ssize_t nread, const uv_buf_t* buf, const struct sockaddr* addr, unsigned flags) {
    node tmp, *link;
    size_t key_len;
    int result;

    if(nread < 0) {
        /* error */
        printf("Receive error: %zd\n", nread);
    }
    else if(nread == 0 && addr == NULL) {
        // printf("No read and NULL address\n");
    }
    else {

        switch(buf->base[0]) {
        case 'X':
            uv_stop(handle->loop);
            break;

        case 'G': // G[et]KEY{NUL}
            key_len = strlen(buf->base) - 1;
            tmp.key = &buf->base[1];

            link = sglib_node_find_member(root, &tmp);
            if(link == NULL) {
                send_reply(handle, 'g', tmp.key, NULL, 0, addr);
            } else {
                send_reply(handle, 'g', link->key, link->data, link->data_len, addr);
            }
            break;

        case 'S':   // Set KEY{NUL}VALUE
            key_len = strlen(buf->base) - 1;
            tmp.key = &buf->base[1];
            link = sglib_node_find_member(root, &tmp);
            if(link == NULL) {
                link = malloc(sizeof(node));
                link->key = strdup(tmp.key);
                link->data_len = nread - key_len - 1;
                link->data = malloc(link->data_len);
                memcpy(link->data, &buf->base[key_len+2], link->data_len);
                sglib_node_add(&root, link);
            } else {
                free(link->data);
                link->data_len = nread - key_len - 1;
                link->data = malloc(link->data_len);
                memcpy(link->data, &buf->base[key_len+2], link->data_len);
            }
            send_reply(handle, 's', link->key, link->data, link->data_len, addr);
            break;

        case 'D':   // DELETE KEY{NUL}
            key_len = strlen(buf->base) - 1;
            tmp.key = &buf->base[1];
            result = sglib_node_delete_if_member(&root, &tmp, &link);
            // Free the node
            if(result > 0) {
                send_reply(handle, 'd', link->key, link->data, link->data_len, addr);
                free(link->key);
                free(link->data);
                free(link);
            } else {
                send_reply(handle, 'd', tmp.key, NULL, 0, addr);
            }
            break;

        default:
            // Error?
            printf("Unknown command: %c", buf->base[0]);
            break;
        }
    }

    free(buf->base);
}

int main(int argc, char **argv) {
    uv_loop_t loop;
    uv_udp_t skt;
    struct sockaddr_in addr;

    // empty the list;
    root = NULL;

    // create our address ... tedium
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_len = sizeof(struct sockaddr_in);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(6347);

    uv_loop_init(&loop);

    // listen to the world
    uv_udp_init(&loop, &skt);
    uv_udp_bind(&skt, (const struct sockaddr *)&addr, 0);
    uv_udp_recv_start(&skt, alloc_cb, on_recv);

    uv_run(&loop, UV_RUN_DEFAULT);

    uv_loop_close(&loop);
}
