# Python TCP Client B
import socket 

host = '192.168.4.1'
port = 8001
BUFFER_SIZE = 2000 
MESSAGE = raw_input("tcpClientB: Enter message/ Enter exit:")
 
tcpClientB = socket.socket(socket.AF_INET, socket.SOCK_STREAM) 
tcpClientB.connect((host, port))

while MESSAGE != 'exit':
    tcpClientB.send(MESSAGE)     
    data = tcpClientB.recv(BUFFER_SIZE)
    print " Client received data:", data
    MESSAGE = raw_input("tcpClientB: Enter message to continue/ Enter exit:")

tcpClientB.close() 
