#!/usr/bin/python

import socket
import select

server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
server_socket.bind(('', 2007))
server_socket.listen(5)
# server_socket.setblocking(0)
poll = select.poll() # epoll() should work the same
poll.register(server_socket.fileno(), select.POLLIN)

connections = {}
while True:
    events = poll.poll(10000)  # 10 seconds
    for fileno, event in events:
        if fileno == server_socket.fileno():
            (client_socket, client_address) = server_socket.accept()
            print "got connection from", client_address
            # client_socket.setblocking(0)
            poll.register(client_socket.fileno(), select.POLLIN) #register read event by select.POLLIN
            connections[client_socket.fileno()] = client_socket
        elif event & select.POLLIN: #check whether curret event is read event
            client_socket = connections[fileno]
            data = client_socket.recv(5)
            if data:
                client_socket.send(data) # sendall() partial?
                print "send data"
            else:
                poll.unregister(fileno)
                client_socket.close()
                del connections[fileno]
                print "connection down"
