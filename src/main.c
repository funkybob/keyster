#include <stdlib.h>
#include <stdio.h>
#include <string.h> // strlen, strnlen
#include <ctype.h>  // isspace
#include <unistd.h> // getopt
#include <netinet/in.h> // sockaddr_in
#include <arpa/inet.h> // htons
#include <uv.h>
#include "sglib.h"

typedef struct node {
    char *data;
    char *key;
    size_t data_len;

    struct node *left;
    struct node *right;
    char colour_field;
} node;

#define COMPARATOR(x,y) (strcmp(x->key, y->key))

SGLIB_DEFINE_RBTREE_PROTOTYPES(node, left, right, colour_field, COMPARATOR);
SGLIB_DEFINE_RBTREE_FUNCTIONS(node, left, right, colour_field, COMPARATOR);

node *root;

static void alloc_cb(uv_handle_t *handle, size_t size, uv_buf_t *buf) {
  buf->base = malloc(size);
  buf->len = size;
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
    struct sglib_node_iterator it;
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
        tmp.key = &buf->base[1];
        key_len = strnlen(tmp.key, nread-1);
        if(key_len == nread-1) {
            // bad packet!
            goto out;
        }
        tmp.data = &buf->base[key_len+2];
        tmp.data_len = nread - key_len - 2;

        switch(buf->base[0]) {
        case 'X':
            uv_stop(handle->loop);
            break;

        case 'G': // G[et]KEY{NUL}
            link = sglib_node_find_member(root, &tmp);
            if(link == NULL) {
                send_reply(handle, 'g', tmp.key, NULL, 0, addr);
            } else {
                send_reply(handle, 'g', link->key, link->data, link->data_len, addr);
            }
            break;

        case 'S':   // Set KEY{NUL}VALUE
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

        case 'Z':   // Zero
            for(link=sglib_node_it_init(&it, root); link != NULL; link=sglib_node_it_next(&it)) {
                free(link->key);
                free(link->data);
                free(link);
            }
            // Don't bother deleting items from the tree.
            // Once we've freed all the memory, just null the root.
            // XXX Not even remotely thread safe!
            root = NULL;
            send_reply(handle, 'z', "", NULL, 0, addr);

        default:
            // Error?
            printf("Unknown command: %c", buf->base[0]);
            break;
        }
    }
out:
    free(buf->base);
}

int main(int argc, char **argv) {
    uv_loop_t loop;
    uv_udp_t skt;

    struct sockaddr_in addr;

    char *host = NULL;
    int port = 6347;

    int opt;

    // ensure the tree is blank;
    root = NULL;

    while ((opt = getopt(argc, argv, "a:p:")) != -1) {
        switch (opt) {
        case 'a':
            host = optarg;
            break;
        case 'p':
            port = atoi(optarg);
            break;
        default:
            fprintf(stderr, "Usage: %s [-a bindto] [-p port]\n", argv[0]);
            exit(-1);
        }
    }

    uv_ip4_addr((host == NULL) ? "127.0.0.1" : host, port, &addr);

    uv_loop_init(&loop);

    // listen to the world
    uv_udp_init(&loop, &skt);
    uv_udp_bind(&skt, (const struct sockaddr *)&addr, 0);
    uv_udp_recv_start(&skt, alloc_cb, on_recv);

    uv_run(&loop, UV_RUN_DEFAULT);

    uv_loop_close(&loop);
}
