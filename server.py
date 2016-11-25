#!/usr/bin/env python
from __future__ import print_function
import argparse
import sys
import os.path
from struct import pack
from socket import socket, AF_INET, SOCK_STREAM, SOL_SOCKET, SO_REUSEADDR


def encode_args(argv):
    data = '\0'
    offlist = []
    for arg in argv:
        offs = len(data)
        data += arg + '\0'
        offlist.append(offs)
    offlist.append(0)
    offlist.reverse()
    outargs = ''.join([pack('!I', N) for N in offlist])
    return pack('!II', len(data), len(offlist)) + outargs + data


def run_server(filedata, args):
    print("[*] Listening on %s:%d" % (args.ip, args.port))
    sock = socket(AF_INET, SOCK_STREAM)
    sock.setsockopt(SOL_SOCKET, SO_REUSEADDR, 1)
    sock.bind((args.ip, args.port))
    sock.listen(1)
    try:
        while True:
            try:
                conn, client_addr = sock.accept()
                print("[+] Sending payload (%d bytes) to %r" % (
                    len(filedata), client_addr))
                try:
                    conn.sendall(encode_args(args.argv))
                    conn.sendall(pack('!I', len(filedata)))
                    conn.sendall(filedata)
                finally:
                    conn.close()
            except Exception as ex:
                print(ex)
                print("Client failed!")
                pass
    except KeyboardInterrupt:
        print("[!] Shutting down...")
    sock.close()
    return 1


def main(args):
    parser = argparse.ArgumentParser(description='Process some integers.')
    parser.add_argument('--name', metavar='name', type=str, default=None,
                        help='Program name (argv[0])', nargs=1)
    parser.add_argument('--ip', metavar='ip', type=str, default=["127.0.0.1"],
                        help='TCP listen ip', nargs=1)
    parser.add_argument('--port', metavar='port', type=int, default=1337,
                        help='TCP listen port', nargs=1)
    parser.add_argument('cmd', nargs=1, help='Local executable name or path')
    parser.add_argument('argv', nargs='*', help='command arguments')
    args = parser.parse_args()

    prog_name = os.path.basename(args.name if args.name else args.cmd[0])
    args.argv.insert(0, prog_name)

    args.ip = args.ip[0]

    print("IP:   ", args.ip)
    print("Port: ", args.port)
    print("File: ", args.cmd[0])
    print("argv: ", args.argv)

    filedata = open(args.cmd[0], 'rb').read()
    return run_server(filedata, args)


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
