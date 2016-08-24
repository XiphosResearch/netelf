#!/usr/bin/env python
from __future__ import print_function
import os.path
import sys
from struct import pack
from socket import socket, AF_INET, SOCK_STREAM, SOL_SOCKET, SO_REUSEADDR


def run_server(filedata, ip, port):
	print("[*] Listening on %s:%d" % (ip, port))
	sock = socket(AF_INET, SOCK_STREAM)
	sock.setsockopt(SOL_SOCKET, SO_REUSEADDR, 1)
	sock.bind((ip, port))
	sock.listen(1)
	try:
		while True:
			try:
				conn, client_addr = sock.accept()
				print("[+] Sending payload (%d bytes) to %r" % (
					len(filedata), client_addr,))
				try:
					conn.sendall(pack('!I', len(filedata)))
					conn.sendall(filedata)
				finally:
					conn.close()
			except Exception as ex:

				print("Client failed!")
				pass
	except KeyboardInterrupt:
		print("[!] Shutting down...")
	sock.close()
	return 1


def main(args):
	if not args:
		print("Usage: server.py <filename> [ip] [port]")
		return 1
	filename = args[0]
	ip = '0.0.0.0'
	port = 38248
	if len(args) > 1:
		ip = args[1]
		if len(args) > 2:
			port = int(args[2])
	filedata = open(filename, 'rb').read()
	return run_server(filedata, ip, port)


if __name__ == "__main__":
	sys.exit(main(sys.argv[1:]))
