
import socket
import struct


class Keyser(object):
    def __init__(self, host='localhost', port=6347):
        self.target = (host, port)
        self.skt = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    def _parse_reply(self, rsp):
        cmd = rsp[0]
        key_len = rsp.find('\x00')
        key = rsp[1:key_len]
        value = rsp[key_len+1:]
        return cmd, key, value

    def _send_msg(self, cmd, key, value=None):
        pkt = '%s%s\x00' % (cmd, key,)
        if value is not None:
            pkt = pkt + value
        self.skt.sendto(pkt, self.target)

    def __getitem__(self, key):
        self._send_msg('G', key)
        rsp, src = self.skt.recvfrom(65536)
        cmd, key, value = self._parse_reply(rsp)
        if value:
            return value
        raise KeyError(key)

    def __setitem__(self, key, value):
        self._send_msg('S', key, value)
        rsp, src = self.skt.recvfrom(65536)
        cmd, key, value = self._parse_reply(rsp)
        return value

    def __delitem__(self, key):
        self._send_msg('D', key)
        rsp, src = self.skt.recvfrom(65536)
        cmd, key, value = self._parse_reply(rsp)
        return value
