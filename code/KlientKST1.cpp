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
		return 0;
	}

	int ReceiveSend(std::string name, std::string choice) {

		if (choice == "get")
		{


			const char* nameS = name.c_str();
			int commandLength = choice.size();
			send(clientSocket, (char*)&commandLength, sizeof(int), 0);
			send(clientSocket, choice.c_str(), choice.size(), 0);
			int nameLength = name.size();
			send(clientSocket, (char*)&nameLength, sizeof(int), 0);
			send(clientSocket, nameS, name.size(), 0);
			std::streamsize fileSize;
			if (recv(clientSocket, (char*)&fileSize, sizeof(std::streamsize), 0) == SOCKET_ERROR) {
				std::cout << "something went wrong when receiving file size" << std::endl;
				std::cout << WSAGetLastError() << std::endl;
			}

			std::ofstream outFile(name, std::ios::binary);
			std::vector<char> fileBuffer(fileSize);
			if (recv(clientSocket, fileBuffer.data(), fileSize, 0) != SOCKET_ERROR) {
				outFile.write(fileBuffer.data(), fileSize);
			}
			else
			{
				std::cout << "something went wrong when receiving file data" << std::endl;
				std::cout << WSAGetLastError() << std::endl;
			}
			outFile.close();
		}
		if (choice == "list")
		{
			int commandLength = choice.size();
			send(clientSocket, (char*)&commandLength, sizeof(int), 0);
			send(clientSocket, choice.c_str(), choice.size(), 0);
			while (true)
			{
				int nameLenght;

				recv(clientSocket, (char*)&nameLenght, sizeof(int), 0);
				std::vector<char> nameBuffer(nameLenght);
				recv(clientSocket, nameBuffer.data(), nameLenght, 0);
				std::string name(nameBuffer.begin(), nameBuffer.end());
				std::cout << name << std::endl;
				if (name == "End")
				{
					break;
				}
			}


		}
		if (choice == "put") {
			int commandLength = choice.size();
			send(clientSocket, (char*)&commandLength, sizeof(int), 0);
			send(clientSocket, choice.c_str(), choice.size(), 0);

			int nameLength = name.size();
			const char* nameS = name.c_str();
			send(clientSocket, (char*)&nameLength, sizeof(int), 0);
			send(clientSocket, nameS, name.size(), 0);
			std::string filepath = "C:\\Users\\Давід\\source\\repos\\Lab1\\KlientKST1\\KlientKST1\\client\\" + name;
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
	Client cl;


	while (true)
	{
		cl.SetUP();
		std::cout << "Enter:" << std::endl;
		std::string choice;

		std::string text;
		//std::cout << "Enter a choice: ";
		std::getline(std::cin, choice);
		if (choice == "Q")
		{
			break;
		}
		std::getline(std::cin, text);
		// Initialize Winsock


		cl.ReceiveSend(text, choice);
		std::cout << "ended" << std::endl;

	}
	//}

	return 0;
}