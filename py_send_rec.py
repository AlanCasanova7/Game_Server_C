import struct
import socket

UDP_IP = "127.0.0.1"
UDP_PORT = 9999

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
pack = struct.pack("=B", 1)

sock.sendto(pack, (UDP_IP, UDP_PORT))

#sock.bind((UDP_IP, UDP_PORT))

data, addr = sock.recvfrom(4096)
print("received message: ", data)
