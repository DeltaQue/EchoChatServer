/*!
 * Simple chat program (server side).cpp - http://github.com/hassanyf
 * Version - 2.0.1
 *
 * Copyright (c) 2016 Hassan M. Yousuf
 */

#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4996)
#define DEFAULT_PORT "27015"

#include <iostream>
#include <string.h>
#include <stdlib.h>

#include <WinSock2.h>
#include <WS2tcpip.h>

using namespace std;

#pragma comment(lib, "ws2_32.lib")

using std::cout;
using std::cerr;
using std::endl;

enum ePort { SERVER_PORT = 54000 };

int main()
{
	// [1] WinSock 초기화
	// MAKEWORD => WORD [ 1byte ][ 1byte ]
	WSADATA wsaData;
	int iniResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iniResult != 0) {
		cerr << "Can't initialize winsock!! Quiting" << endl;
		return -1;
	}
	


	// [2] Listen 소켓 생성
	// af(adress family) : AF_INET(IPv4), AF_INET6(IPv6)
	// type : SOCK_STREAM(TCP), SOCK_DGRAM(UDP)
	// protocol : IPPROTO_TCP(TCP), IPPROTO_UDP(UDP)
	SOCKET listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSock == INVALID_SOCKET) {
		cerr << "Can't create a socket! Quiting" << endl;
		WSACleanup();
		return -1;
	}


	// [3] 소켓에 IP주소와 Port번호 바인딩
	// INADDR_ANY : 모든 NIC의 IP주소에 바인딩
	// htonl : 호스트로부터 빅 엔디안인 TCP/IP 네트워크 바이트 순서로 u_long을 반환
	// htons : 호스트로부터 빅 엔디안인 TCP/IP 네트워크 바이트 순서로 u_short을 반환
	// 네트워크 바이트 순서는 빅 엔디안!
	sockaddr_in hint{};
	hint.sin_family = AF_INET;
	hint.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	hint.sin_port = htons(SERVER_PORT);

	int bindResult = bind(listenSock, reinterpret_cast<sockaddr*>(&hint), sizeof(hint));
	if (bindResult == SOCKET_ERROR) {
		cerr << "Can't bind a socket! Quiting" << endl;
		closesocket(listenSock);
		WSACleanup();
		return -1;
 	}


	// [4] 소켓이 연결을 accept 할 수 있는 상태가 되도록함 (listen)
	int listenResult = listen(listenSock, SOMAXCONN);
	if (listenResult == SOCKET_ERROR) {
		cerr << "Can't listen a socket! Quiting" << endl;
		closesocket(listenSock);
		WSACleanup();
		return -1;
	}


	// [5] 클라이언트 요청이 들어오면 accept 함수를 통해 연결을 수락
	sockaddr_in clientSockInfo;
	int clientSize = sizeof(clientSockInfo);

	SOCKET clientSocket = accept(listenSock, reinterpret_cast<sockaddr*>(&clientSockInfo), &clientSize);
	if (clientSocket == INVALID_SOCKET) {
		cerr << "Can't accpet a socket! Quiting" << endl;
		closesocket(listenSock);
		WSACleanup();
		return -1;
	}

	// [6] 연결이 되었으면 listening 중인 소켓을 닫습니다. (계속해서 다른 클라이언트 연결을 받는다면 생략)
	int closeResult = closesocket(listenSock);

	// [7] 클라이언트의 요청을 받고 수행할 동작을 구현
	char host[NI_MAXHOST];		// 클라이언트 host 이름
	char service[NI_MAXHOST];	// 클라이언트 PORT 번호
	ZeroMemory(host, NI_MAXHOST);	// memset(host, 0, NI_MAXHOST)과 동일
	ZeroMemory(service, NI_MAXHOST);

	// clientSockInfo에 저장된 IP 주소를 통해 도메인 정보를 얻습니다.
	// host 이름은 host에, 포트 번호는 service에 저장됩니다.
	// getnameinfo()는 성공시 0을 반환합니다. 실패시 0이 아닌 값을 반환합니다.
	if (getnameinfo((sockaddr*)&clientSockInfo, sizeof(clientSockInfo), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0) {
		cout << host << " connected On Port " << service << endl;
	}
	else {
		inet_ntop(AF_INET, &clientSockInfo.sin_addr, host, NI_MAXHOST);
		cout << host << " connected On Port " << ntohs(clientSockInfo.sin_port) << endl;
	}

	// While Loop : 클라이언트의 메세지를 받아서 출력 후, 클라이언트에게 다시 보냅니다.
	enum eBufSize { BUF_SIZE = 4096 };
	char buf[BUF_SIZE];

	while (true) {
		ZeroMemory(buf, BUF_SIZE);

		// Wait for cleint to send data
		// 메세지를 성공적으로 받았으면, recv 함수는 메세지의 크기를 반환한다.
		int bytesReceived = recv(clientSocket, buf, BUF_SIZE, 0);
		if (bytesReceived == SOCKET_ERROR) {
			cerr << "Error in recv(). Quitting" << endl;
			break;
		}
		else if (bytesReceived == 0) {
			cout << "Client disconnected " << endl;
			break;
		}

		// Echo message back to client
		cout << buf << endl;
		send(clientSocket, buf, bytesReceived + 1, 0);
	}

	// Close the client socket
	closesocket(clientSocket);

	// Cleanup winsock <-> WSAStartup
	WSACleanup();

	return 0;
}