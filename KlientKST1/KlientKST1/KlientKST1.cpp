#include <iostream>
#include <WinSock2.h>
#include <Ws2tcpip.h>
// Linking the library needed for network communication
#pragma comment(lib, "ws2_32.lib")


class Client
{
public:
	int SetUP() {
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		{
			std::cerr << "WSAStartup failed" << std::endl;
			return 1;
		}
		clientSocket=socket(AF_INET, SOCK_STREAM, 0);
		if (clientSocket == INVALID_SOCKET)
		{
			std::cerr << "Error creating socket: " << WSAGetLastError() << std::endl;
			WSACleanup();
			return 1;
		}
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_port = htons(port);
		InetPton(AF_INET, serverIp, &serverAddr.sin_addr);
		if (connect(clientSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR)
		{
			std::cerr << "Connect failed with error: " << WSAGetLastError() << std::endl;
			closesocket(clientSocket);
			WSACleanup();
			return 1;
		}
	}

	int RecieveSend() {
		const char* message = "Hello, server! How are you?";
		send(clientSocket, message, (int)strlen(message), 0);
		// Receive the response from the server
		char buffer[1024];
		memset(buffer, 0, 1024);
		int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
		if (bytesReceived > 0)
		{
			std::cout << "Received from server: " << buffer << std::endl;
		}
		// Clean up
		closesocket(clientSocket);
		WSACleanup();
		return 0;
	}

private:
	WSADATA wsaData;
	int port = 12345;
	PCWSTR serverIp = L"127.0.0.1";
	SOCKET clientSocket;
	sockaddr_in serverAddr;
};


int main()
{
	// Initialize Winsock
	Client cl;
	cl.SetUP();
	cl.RecieveSend();
	return 0;
}
