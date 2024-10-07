//
// Created by SSAFY on 2024-10-04.
//
#ifndef BRIDGE_H
#define BRIDGE_H

#include <string>
#include <WinSock2.h>

#define HOST "127.0.0.1"
#define PORT 8747

extern SOCKET sock;

std::string utf8_encode(const std::wstring& wstr);
std::wstring utf8_decode(const std::string& str);

void ErrorHandling(const std::string& message);
std::string init(const std::wstring& nickname);
std::string submit(const std::string& string_to_send);
std::string receive();
void close();

#endif


