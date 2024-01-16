import socket
import struct

class Client:
    def __init__(self):
        self.host = "127.0.0.1"  # Server IP address
        self.port = 12345
        self.client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.client_socket.connect((self.host, self.port))

    def send(self, data):
        data = data.encode()
        total_size = len(data)
        self.client_socket.send(struct.pack('<I', total_size))
        self.client_socket.send(data)

    def rec(self):
        length_data = self.client_socket.recv(4)
        length_data = struct.unpack('<I', length_data)[0]
        data = self.client_socket.recv(length_data)
        return data.decode()

    def recF(self):
        # because the streamsize size is 8 we need size 8  here
        length_data = self.client_socket.recv(8)
        length_data = struct.unpack('<Q', length_data)[0]
        data = self.client_socket.recv(length_data)
        return data

    def Glist(self):
        self.send("list")
        while True:
            data=self.rec()
            print(data)
            if data=="End":
                break

    def Q(self):
        self.send("Q")
        self.client_socket.close()

    def delete(self,name):
        self.send("delete")
        self.send(name)
        print(self.rec())

    def info(self,name):
        self.send("info")
        self.send(name)
        print(self.rec())
        print(self.rec())

    def Gfile(self,name):
        self.send("get")
        self.send(name)
        with open(name, 'wb') as file:
            file.write(self.recF())

    def put(self, name):
        self.send("put")
        self.send(name)
        with open(name, 'rb') as file:
            data=file.read()
        dataLenght=len(data)
        self.client_socket.send(struct.pack('<Q', dataLenght))
        self.client_socket.send(data)

    def receive_send(self, command):
        if command=="list":
            self.Glist()
        if command=="delete":
            name=input("name:")
            self.delete(name)
        if command=="get":
            name = input("name:")
            self.Gfile(name)
        if command=="Q":
            self.Q()
        if command=="info":
            name = input("name:")
            self.info(name)
        if command=="put":
            name=input("name:")
            self.put(name)


def main():
    while True:
        a=input()
        client=Client()
        client.receive_send(a)
        if a=="Q":
            break


if __name__ == "__main__":
    main()