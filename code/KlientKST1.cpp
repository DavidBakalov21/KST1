#include <iostream>
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <string> 
#include <fstream>
#include <vector>

#pragma comment(lib, "ws2_32.lib")
class Setuper 
{
public:
	SOCKET clientSocket;
	int SetUP() 
	{
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
private:
	WSADATA wsaData;
	int port = 12345;
	PCWSTR serverIp = L"127.0.0.1";
	sockaddr_in serverAddr;
};

class Client
{
public:
	Client() 
	{
		st.SetUP();
	}

	void sendText(const std::string& text)
	{
		int textLength = text.size();
		send(st.clientSocket, (char*)&textLength, sizeof(int), 0);
		send(st.clientSocket, text.c_str(), textLength, 0);
	}

	std::string receiveText() 
	{
		int textLenght;
		recv(st.clientSocket, (char*)&textLenght, sizeof(int), 0);
		std::vector<char> textBuffer(textLenght);
		recv(st.clientSocket, textBuffer.data(), textLenght, 0);
		return std::string(textBuffer.begin(), textBuffer.end());
	}

	void List(const std::string& choice) 
	{
		sendText(choice);
		while (true)
		{
			std::string name = receiveText();
			std::cout << name << std::endl;
			if (name == "End")
			{
				break;
			}
		}
	}

	void Put(const std::string& name, const std::string& choice) 
	{
		sendText(choice);
		sendText(name);
		std::string filepath = "C:\\Users\\Давід\\source\\repos\\Lab1\\KlientKST1\\KlientKST1\\client\\" + name;
		std::ifstream file(filepath, std::ios::binary | std::ios::ate);
		std::streamsize fileSize = file.tellg();
		file.seekg(0, std::ios::beg);
		send(st.clientSocket, (char*)&fileSize, sizeof(std::streamsize), 0);
		std::streamsize totalSent = 0;
		char buffer[2500];
		while (totalSent < fileSize)
		{
			std::streamsize remaining = fileSize - totalSent;
			std::streamsize currentChunkSize = (remaining < 2500) ? remaining : 2500;
			file.read(buffer, currentChunkSize);
			send(st.clientSocket, buffer, currentChunkSize, 0);
			std::cout << "Chunk size is:" << currentChunkSize << std::endl;
			totalSent += currentChunkSize;
		}
		file.close();
		std::string confirmation = receiveText();
		std::cout << confirmation << std::endl;
	}

	void GetF(const std::string& name, const std::string& choice) 
	{
		sendText(choice);
		sendText(name);
		std::streamsize fileSize;
		if (recv(st.clientSocket, (char*)&fileSize, sizeof(std::streamsize), 0) == SOCKET_ERROR) 
		{
			std::cout << "something went wrong when receiving file size" << std::endl;
			std::cout << WSAGetLastError() << std::endl;
		}
		std::cout << "File size is:" << fileSize << std::endl;
		std::ofstream outFile(name, std::ios::binary);
		std::streamsize totalReceived = 0;
		while (totalReceived < fileSize)
		{
			char buffer[2500];
			std::streamsize bytesReceived = recv(st.clientSocket, buffer, sizeof(buffer), 0);
			outFile.write(buffer, bytesReceived);
			totalReceived += bytesReceived;
			std::cout << "current file size:" << totalReceived << std::endl;
		}
		outFile.close();
	}

	int ReceiveSend() 
	{
		while (true)
		{
			std::cout << "name" << std::endl;
			std::string name;
			std::getline(std::cin, name);
			sendText(name);
			std::cout << "Enter:" << std::endl;
			std::string choice;
			std::getline(std::cin, choice);
			if (choice == "list")
			{
				List(choice);
			}
			if (choice == "get")
			{
				std::string name;
				std::getline(std::cin, name);
				GetF(name, choice);
			}
			if (choice == "put")
			{
				std::string name;
				std::getline(std::cin, name);
				Put(name, choice);
			}
			if (choice == "Q")
			{
				sendText(choice);
				break;
			}
			if (choice == "delete") 
			{
				std::string name;
				std::getline(std::cin, name);
				sendText(choice);
				sendText(name);
				std::cout << receiveText() << std::endl;
			}
			if (choice == "info")
			{
				std::string name;
				std::getline(std::cin, name);
				sendText(choice);
				sendText(name);
				std::cout << "size: " << receiveText() << std::endl;
				std::cout << "last modified: " << receiveText() << std::endl;
			}
		}
		closesocket(st.clientSocket);
		WSACleanup();
		return 0;
	}
private:
	Setuper st;
};
int main()
{
	Client cl;
	cl.ReceiveSend();
	return 0;
}