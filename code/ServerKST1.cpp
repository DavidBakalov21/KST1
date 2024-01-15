#include <iostream>
#include <WinSock2.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <filesystem>
#include <cstdio>
#include <format>
// Linking the library needed for network communication
#pragma comment(lib, "ws2_32.lib")
class SetuperServ {
public:
	SOCKET clientSocket;
	SOCKET serverSocket;
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
private:
	WSADATA wsaData;
	int port = 12345;
	
	sockaddr_in serverAddr;
	
};
class Server
{
public:
	Server() {
	sr.setUP();
	}
	void sendText(const std::string& text) {

		int textLength = text.size();
		send(sr.clientSocket, (char*)&textLength, sizeof(int), 0);
		send(sr.clientSocket, text.c_str(), textLength, 0);
	}
	std::string receiveText() {
		int textLenght;
		recv(sr.clientSocket, (char*)&textLenght, sizeof(int), 0);
		std::vector<char> textBuffer(textLenght);
		recv(sr.clientSocket, textBuffer.data(), textLenght, 0);
		return std::string(textBuffer.begin(), textBuffer.end());
		
	}

	void GetF() {
		std::string name = receiveText();

		std::string filepath = "C:\\Users\\Давід\\source\\repos\\Lab1\\ServerKST1\\ServerKST1\\database\\" + name;

		std::ifstream file(filepath, std::ios::binary | std::ios::ate);
		std::streamsize fileSize = file.tellg();
		file.seekg(0, std::ios::beg);
		send(sr.clientSocket, (char*)&fileSize, sizeof(std::streamsize), 0);
		std::vector<char> fileBuffer(fileSize);
		if (file.read(fileBuffer.data(), fileSize)) {
			send(sr.clientSocket, fileBuffer.data(), fileSize, 0);
		}

		file.close();
	}

	void PUT() {
		std::string name = receiveText();
		std::streamsize fileSize;
		if (recv(sr.clientSocket, (char*)&fileSize, sizeof(std::streamsize), 0) == SOCKET_ERROR) {
			std::cout << "something went wrong when receiving file size" << std::endl;
			std::cout << WSAGetLastError() << std::endl;
		}

		std::ofstream outFile("C:\\Users\\Давід\\source\\repos\\Lab1\\ServerKST1\\ServerKST1\\database\\" + name, std::ios::binary);
		std::vector<char> fileBuffer(fileSize);
		if (recv(sr.clientSocket, fileBuffer.data(), fileSize, 0) != SOCKET_ERROR) {
			outFile.write(fileBuffer.data(), fileSize);
		}
		else
		{
			std::cout << "something went wrong when receiving file data" << std::endl;
			std::cout << WSAGetLastError() << std::endl;
		}
		outFile.close();
		std::string conf = "Confirm";
		sendText(conf);
	}

	int ReceiveSend() {
		std::string choice = receiveText();
		if (choice == "get") {
			GetF();
		}
		if (choice=="list")
		{
			for (const auto& entry : std::filesystem::directory_iterator("C:\\Users\\Давід\\source\\repos\\Lab1\\ServerKST1\\ServerKST1\\database")) {
				std::string Name = entry.path().filename().string();
				sendText(Name);
			}
			std::string end = "End";
			sendText(end);
		}
		if (choice=="put")
		{
			PUT();
		}
		if (choice=="Q")
		{
			closesocket(sr.clientSocket);
			closesocket(sr.serverSocket);
			WSACleanup();
			return 78;
		}
		if (choice=="delete")
		{
			std::string name = receiveText();
			std::string filepath = "C:\\Users\\Давід\\source\\repos\\Lab1\\ServerKST1\\ServerKST1\\database\\" + name;
			remove(filepath.c_str());
			std::string del = "deleted";
			sendText(del);

		}
		if (choice=="info")
		{
			std::string name = receiveText();
			std::string filepath = "C:\\Users\\Давід\\source\\repos\\Lab1\\ServerKST1\\ServerKST1\\database\\" + name;
			std::ifstream file(filepath, std::ios::binary | std::ios::ate);
			std::streamsize fileSize = file.tellg();
			std::filesystem::file_time_type modtime = std::filesystem::last_write_time(filepath);
			std::string size = std::to_string(fileSize);
			std::string lastMod = std::format("{}", modtime);
			sendText(size);
			sendText(lastMod);

		}
		closesocket(sr.clientSocket);
		closesocket(sr.serverSocket);
		WSACleanup();
		return 0;
	}
private:
	SetuperServ sr;
};
int main()
{
	while (true)
	{
		Server serv;
		std::cout << "started" << std::endl;
		if (serv.ReceiveSend() == 78) {
			break;
		}
		std::cout << "ended" << std::endl;

	}
}