#!/usr/bin/env python
from __future__ import print_function
import os.path
import sys
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


def run_server(filedata, ip, port, argv):
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
					len(filedata), client_addr))
				try:
					conn.sendall(encode_args(argv))
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
	if len(args) < 4:
		print("Usage: server.py <ip> <port> <filename> <argv0> [argv1 ...]")
		return 1
	ip, port, filename = args[:3]
	argv = args[3:]
	port = int(port)
	
	print("IP:   ", ip)
	print("Port: ", port)
	print("File: ", filename)
	print("argv: ", argv)

	filedata = open(filename, 'rb').read()
	return run_server(filedata, ip, port, argv)


if __name__ == "__main__":
	sys.exit(main(sys.argv[1:]))
