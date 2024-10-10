//
// Created by SSAFY on 2024-10-04.
//
#include "bridge.h"
#include <iostream>
#include <cstring>
#include <locale>
#include <codecvt>

#pragma warning(disable : 4996)
#pragma comment(lib, "ws2_32.lib")

SOCKET sock;

std::string utf8_encode(const std::wstring& wstr) {
	std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
	return conv.to_bytes(wstr);
}

std::wstring utf8_decode(const std::string& str) {
	std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
	return conv.from_bytes(str);
}

void ErrorHandling(const std::string& message) {
	std::cerr << "[ERROR] " << message << std::endl;
	exit(1);
}

std::string init(const std::wstring& nickname) {
	WSADATA wsaData;
	SOCKADDR_IN sockAddr;
	std::wstring init_command = L"INIT " + nickname;
	std::string enc_command = utf8_encode(init_command);

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup failure.");

	sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
		ErrorHandling("Socket Creating Failure.");

	std::memset(&sockAddr, 0, sizeof(sockAddr));
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_addr.s_addr = inet_addr(HOST);
	sockAddr.sin_port = htons(PORT);

	std::cout << "[STATUS] Trying to connect to " << HOST << ":" << PORT << std::endl;
	if (connect(sock, (SOCKADDR*)&sockAddr, sizeof(sockAddr)) == SOCKET_ERROR) {
		ErrorHandling("Failed to connect. Please check if Battle SSAFY is waiting for connection.");
	}
	else {
		std::cout << "[STATUS] Connected" << std::endl;
		return submit(enc_command);
	}

	return "";
}

std::string submit(const std::string& string_to_send) {
	if (send(sock, string_to_send.c_str(), string_to_send.length(), 0) == SOCKET_ERROR) {
		ErrorHandling("Failed to send data. Please check if Battle SSAFY is waiting for connection.");
	}

	return receive();
}

std::string receive() {
	char buffer[1024];
	int strLen = recv(sock, buffer, sizeof(buffer) - 1, 0);

	if (strLen == -1) {
		ErrorHandling("Failed to receive data. Please check if Battle SSAFY is waiting for connection.");
	}

	buffer[strLen] = '\0';
	std::string game_data(buffer);

	std::wstring decoded_data = utf8_decode(game_data);
	std::string result(decoded_data.begin(), decoded_data.end());

	if (int(game_data[0]) > 0) {
		return game_data;
	}
	else {
		close();
	}

	return "";
}

void close() {
	if (sock != INVALID_SOCKET) {
		closesocket(sock);
		WSACleanup();
		std::cout << "[STATUS] Connection closed" << std::endl;
	}
	else {
		std::cerr << "[ERROR] Network connection has been corrupted." << std::endl;
	}
}

