#include <iostream>
#include <WinSock2.h>
#include <fstream>
#include <sstream>
#include <vector>
// Linking the library needed for network communication
#pragma comment(lib, "ws2_32.lib")
class Server
{
public:
	int setUP() {

		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		{
			std::cerr << "WSAStartup failed" << std::endl;
			return 1;
		}
		serverSocket = socket(AF_INET, SOCK_STREAM, 0);
		if (serverSocket == INVALID_SOCKET)
		{
			std::cerr << "Error creating socket: " << WSAGetLastError() << std::endl;
			WSACleanup();
			return 1;
		}
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_addr.s_addr = INADDR_ANY;
		serverAddr.sin_port = htons(port);
		if (bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR)
		{
			std::cerr << "Bind failed with error: " << WSAGetLastError() << std::endl;
			closesocket(serverSocket);
			WSACleanup();
			return 1;
		}
		// Listen for incoming connections
		if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR)
		{
			std::cerr << "Listen failed with error: " << WSAGetLastError() << std::endl;
			closesocket(serverSocket);
			WSACleanup();
			return 1;
		}
		clientSocket = accept(serverSocket, nullptr, nullptr);
		if (clientSocket == INVALID_SOCKET)
		{
			std::cerr << "Accept failed with error: " << WSAGetLastError() << std::endl;
			closesocket(serverSocket);
			WSACleanup();
			return 1;
		}
	}
	int ReceiveSend() {
		char buffer[1024];
		memset(buffer, 0, 1024);
		int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);

		if (bytesReceived > 0) {
			std::string command(buffer, bytesReceived);
			if (command == "get") {
				std::string filepath = "C:\\Users\\Давід\\source\\repos\\Lab1\\ServerKST1\\ServerKST1\\testforlab1.txt";
				std::ifstream file(filepath, std::ios::binary | std::ios::ate);
				if (!file.is_open()) {
					std::cerr << "Failed to open file." << std::endl;
					return 1;
				}

				std::streamsize fileSize = file.tellg();
				file.seekg(0, std::ios::beg);

				std::string fileSizeStr = std::to_string(fileSize) + "\n";
				send(clientSocket, fileSizeStr.c_str(), fileSizeStr.size(), 0);

				std::vector<char> fileBuffer(fileSize);
				std::streamsize totalBytesSent = 0;
				file.seekg(0, std::ios::beg);
				while (totalBytesSent < fileSize) {
					file.read(fileBuffer.data(), fileBuffer.size());
					int bytesToSend = static_cast<int>(file.gcount());
					int result = send(clientSocket, fileBuffer.data(), bytesToSend, 0);
					totalBytesSent += bytesToSend;
				}
			}


		}
		closesocket(clientSocket);
		closesocket(serverSocket);
		WSACleanup();
		return 0;
	}

private:
	WSADATA wsaData;
	int port = 12345;
	SOCKET serverSocket;
	sockaddr_in serverAddr;
	SOCKET clientSocket;


};
int main()
{
	Server serv;
	serv.setUP();
	serv.ReceiveSend();

}