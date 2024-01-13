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
		serverSocket = socket(AF_INET, SOCK_STREAM, 0);//AF_INET2 The Internet Protocol version 4 (IPv4)address family.
		//SOCK_STREAM-wo-way, connection-based byte streams with an OOB data transmission mechanism
		if (serverSocket == INVALID_SOCKET)
		{

			std::cerr << "Error creating socket: " << WSAGetLastError() << std::endl;
			WSACleanup();
			return 1;
		}
		serverAddr.sin_family = AF_INET; // indicates te IP version (like iPv4)
		serverAddr.sin_addr.s_addr = INADDR_ANY; // it recieves messages from any IP
		serverAddr.sin_port = htons(port); // port number on which server listens 

		//need to bind socket to local address
		if (bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) //
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
		return 0;
	}
	int ReceiveSend() {


		int choiceLenght;
		recv(clientSocket, (char*)&choiceLenght, sizeof(int), 0);
		std::vector<char> choiceBuffer(choiceLenght);
		recv(clientSocket, choiceBuffer.data(), choiceLenght, 0);
		std::string choice(choiceBuffer.begin(), choiceBuffer.end());


		if (choice == "get") {
		int nameLenght;
		recv(clientSocket, (char*)&nameLenght, sizeof(int), 0);
		std::vector<char> nameBuffer(nameLenght);
		recv(clientSocket, nameBuffer.data(), nameLenght, 0);
		std::string name(nameBuffer.begin(), nameBuffer.end());
		std::string filepath = "C:\\Users\\Давід\\source\\repos\\Lab1\\ServerKST1\\ServerKST1\\database\\" + name;
		
			std::ifstream file(filepath, std::ios::binary | std::ios::ate);
			std::streamsize fileSize = file.tellg();
			file.seekg(0, std::ios::beg);
			send(clientSocket, (char*)&fileSize, sizeof(std::streamsize), 0);
			std::vector<char> fileBuffer(fileSize);
			if (file.read(fileBuffer.data(), fileSize)) {
				send(clientSocket, fileBuffer.data(), fileSize, 0);
			}
			
			file.close();
		}
		if (choice=="list")
		{

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
	

	while (true)
	{
		std::cout << "started" << std::endl;
		serv.setUP();
		serv.ReceiveSend();
		std::cout << "ended" << std::endl;

	}
}