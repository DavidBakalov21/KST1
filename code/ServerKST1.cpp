#include <iostream>
#include <WinSock2.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <filesystem>
#include <cstdio>
#include <format>
#include <thread>
//Refactor code
// Error casse hanlding when client crashes
// mutexes for couts
// Linking the library needed for network communication
#pragma comment(lib, "ws2_32.lib")
const int EXIT_CODE = 78;
class SetuperServ {
public:
	SOCKET clientSocket;
	SOCKET serverSocket;
	int setUP() 
	{
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
		return 0;
	}
	int acceptUser()
	{
		clientSocket = accept(serverSocket, nullptr, nullptr);
		if (clientSocket == INVALID_SOCKET)
		{
			std::cerr << "Accept failed with error: " << WSAGetLastError() << std::endl;
			closesocket(serverSocket);
			WSACleanup();
			return 1;
		}
		return clientSocket;
	}
private:
	WSADATA wsaData;
	int port = 12345;
	sockaddr_in serverAddr;
};
class Server
{
public:
	Server()
	{
		sr.setUP();
	}

	void sendText(const std::string& text, const int clientSocket)
	{

		int textLength = text.size();
		send(clientSocket, (char*)&textLength, sizeof(int), 0);
		send(clientSocket, text.c_str(), textLength, 0);
	}

	std::string receiveText(const int clientSocket) 
	{
		int textLenght;
		recv(clientSocket, (char*)&textLenght, sizeof(int), 0);
		std::vector<char> textBuffer(textLenght);
		recv(clientSocket, textBuffer.data(), textLenght, 0);
		return std::string(textBuffer.begin(), textBuffer.end());

	}

	void GetF(const int clientSocket)
	{
		std::string name = receiveText(clientSocket);
		std::string filepath = "C:\\Users\\Давід\\source\\repos\\Lab1\\ServerKST1\\ServerKST1\\" + folder + "\\" + name;
		std::ifstream file(filepath, std::ios::binary | std::ios::ate);
		std::streamsize fileSize = file.tellg();
		file.seekg(0, std::ios::beg);
		send(clientSocket, (char*)&fileSize, sizeof(std::streamsize), 0);
		std::streamsize totalSent = 0;
		char buffer[2500];
		while (totalSent < fileSize)
		{
			std::streamsize remaining = fileSize - totalSent;
			std::streamsize currentChunkSize = (remaining < 2500) ? remaining : 2500;
			file.read(buffer, currentChunkSize);
			send(clientSocket, buffer, currentChunkSize, 0);
			//std::cout << "Chunk size is:" << currentChunkSize << std::endl;
			totalSent += currentChunkSize;
		}
		file.close();
	}

	void PUT(const int clientSocket)
	{
		std::string name = receiveText(clientSocket);
		std::streamsize fileSize;
		if (recv(clientSocket, (char*)&fileSize, sizeof(std::streamsize), 0) == SOCKET_ERROR) 
		{
			std::cout << "something went wrong when receiving file size" << std::endl;
			std::cout << WSAGetLastError() << std::endl;
		}
		std::ofstream outFile("C:\\Users\\Давід\\source\\repos\\Lab1\\ServerKST1\\ServerKST1\\" + folder + "\\" + name, std::ios::binary);
		std::streamsize totalReceived = 0;
		while (totalReceived < fileSize)
		{
			char buffer[2500];
			std::streamsize bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
			outFile.write(buffer, bytesReceived);
			totalReceived += bytesReceived;
			//lock
			std::cout << "current file size:" << totalReceived << std::endl;
			//unlock
		}
		outFile.close();
		std::string conf = "Confirm";
		sendText(conf, clientSocket);
	}

	void ReceiveName(const int clientSocket) 
	{
		folder = receiveText(clientSocket);
		if (!std::filesystem::create_directory("C:\\Users\\Давід\\source\\repos\\Lab1\\ServerKST1\\ServerKST1\\" + folder))
		{
			std::cout << "Directory already exists" << std::endl;
		}
	}

	void MultithreadServer() 
	{
		std::vector<std::thread> threads;
		while (true)
		{
			int client = sr.acceptUser();
			if (client!=1)
			{
				threads.emplace_back([this, client]() { ReceiveSend(client); });
			}
		}
		for (auto& t : threads) 
		{
			t.join();
		}
	}

	int ReceiveSend(const int clientSocket)
	{
		while (true)
		{
			std::cout << "started" << std::endl;
			ReceiveName(clientSocket);
			std::string choice = receiveText(clientSocket);
			if (choice == "list")
			{
				for (const auto& entry : std::filesystem::directory_iterator("C:\\Users\\Давід\\source\\repos\\Lab1\\ServerKST1\\ServerKST1\\" + folder + "\\")) 
				{
					std::string Name = entry.path().filename().string();
					sendText(Name, clientSocket);
				}
				std::string end = "End";
				sendText(end, clientSocket);
			}
			if (choice == "get") 
			{
				GetF(clientSocket);
			}
			if (choice == "put")
			{
				PUT(clientSocket);
			}
			if (choice == "Q")
			{
				closesocket(clientSocket);
				return EXIT_CODE;
			}
			if (choice == "delete")
			{
				std::string name = receiveText(clientSocket);
				std::string filepath = "C:\\Users\\Давід\\source\\repos\\Lab1\\ServerKST1\\ServerKST1\\" + folder + "\\" + name;
				remove(filepath.c_str());
				std::string del = "deleted";
				sendText(del,clientSocket);

			}
			if (choice == "info")
			{
				std::string name = receiveText(clientSocket);
				std::string filepath = "C:\\Users\\Давід\\source\\repos\\Lab1\\ServerKST1\\ServerKST1\\" + folder + "\\" + name;
				std::ifstream file(filepath, std::ios::binary | std::ios::ate);
				std::streamsize fileSize = file.tellg();
				std::filesystem::file_time_type modtime = std::filesystem::last_write_time(filepath);
				std::string size = std::to_string(fileSize);
				std::string lastMod = std::format("{}", modtime);
				sendText(size, clientSocket);
				sendText(lastMod, clientSocket);

			}
			std::cout << "ended" << std::endl;
		}
		closesocket(clientSocket);
		closesocket(sr.serverSocket);
		WSACleanup();
		return 0;
	}
private:
	std::string folder;
	SetuperServ sr;
};

int main()
{
	Server serv;
	serv.MultithreadServer();
	return 0;
}