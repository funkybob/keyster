
import socket

from django.core.cache.backends.base import BaseCache


class KeysterCache(BaseCache):
    def __init__(self, location, params):
        super(KeysterCache, self).__init__(params)
        self.skt = socket.socket(sock.AF_INET, sock.SOCK_DGRAM)
        self._target = (location, params.get('PORT', 6347))


    def _send_msg(self, cmd, key, value=None):
        pkt = '%s%s\x00' % (cmd, key,)
        if value is not None:
            pkt = pkt + value
        self.skt.sendto(pkt, self.target)
        rsp, src = self.skt.recvfrom(65536)
        cmd = rsp[0]
        key_len = rsp.find('\x00')
        key = rsp[1:key_len]
        value = rsp[key_len+1:]
        return cmd, key, value

    def add(self, key, value, timeout=DEFAULT_TIMEOUT, version=None):
        key = self.make_key(key, version)
        cmd, key, value = self._send_msg('A', key, value)
        # XXX
        return True

    def get(self, key, default=None, version=None):
        key = self.make_key(key, version)
        cmd, key, value = self._send_msg('G', key)
        return value or default

    def set(self, key, value, timeout=DEFAULT_TIMEOUT, version=None):
        key = self.make_key(key, version)
        cmd, key, value = self._send_msg('S', key, value)
        return value

    def delete(self, key, version=None):
        key = self.make_key(key, version)
        cmd, key, value = self._send_msg('D', key)
        return value

    def clear(self):
        self._send_msg('Z')
