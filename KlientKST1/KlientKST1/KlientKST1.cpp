#include <iostream>
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <string> 
#include <fstream>
#include <vector>

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
		clientSocket = socket(AF_INET, SOCK_STREAM, 0);
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

	int RecieveSend(std::string choice, std::string name) {
		const char* message = choice.c_str();
		send(clientSocket, message, (int)strlen(message), 0);

		std::ofstream outFile(name, std::ios::binary);
		if (!outFile.is_open()) {
			std::cerr << "Failed to open file: " << name << std::endl;
			return 1;
		}

		char sizeBuffer[1024];
		int bytesReceived = recv(clientSocket, sizeBuffer, sizeof(sizeBuffer), 0);
		if (bytesReceived <= 0) {
			std::cerr << "Failed to receive file size." << std::endl;
			return 1;
		}
		std::cout << bytesReceived << " size" << std::endl;
		std::streamsize fileSize = std::stoll(std::string(sizeBuffer, bytesReceived));
		std::vector<char> fileBuffer(fileSize);
		std::streamsize totalBytesReceived = 0;
		while (totalBytesReceived < fileSize) {
			bytesReceived = recv(clientSocket, fileBuffer.data(), fileBuffer.size(), 0);
			if (bytesReceived <= 0) {
				std::cerr << "Failed to receive file data or connection closed." << std::endl;
				break;
			}
			outFile.write(fileBuffer.data(), bytesReceived);
			totalBytesReceived += bytesReceived;
		}
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
	std::string choice;
	std::string text;
	//std::cout << "Enter a choice: ";
	std::getline(std::cin, choice);
	std::getline(std::cin, text);
	// Initialize Winsock
	Client cl;
	cl.SetUP();
	cl.RecieveSend(choice, text);
	return 0;
}