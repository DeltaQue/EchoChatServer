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
	// [1] WinSock �ʱ�ȭ
	// MAKEWORD => WORD [ 1byte ][ 1byte ]
	WSADATA wsaData;
	int iniResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iniResult != 0) {
		cerr << "Can't initialize winsock!! Quiting" << endl;
		return -1;
	}
	


	// [2] Listen ���� ����
	// af(adress family) : AF_INET(IPv4), AF_INET6(IPv6)
	// type : SOCK_STREAM(TCP), SOCK_DGRAM(UDP)
	// protocol : IPPROTO_TCP(TCP), IPPROTO_UDP(UDP)
	SOCKET listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSock == INVALID_SOCKET) {
		cerr << "Can't create a socket! Quiting" << endl;
		WSACleanup();
		return -1;
	}


	// [3] ���Ͽ� IP�ּҿ� Port��ȣ ���ε�
	// INADDR_ANY : ��� NIC�� IP�ּҿ� ���ε�
	// htonl : ȣ��Ʈ�κ��� �� ������� TCP/IP ��Ʈ��ũ ����Ʈ ������ u_long�� ��ȯ
	// htons : ȣ��Ʈ�κ��� �� ������� TCP/IP ��Ʈ��ũ ����Ʈ ������ u_short�� ��ȯ
	// ��Ʈ��ũ ����Ʈ ������ �� �����!
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


	// [4] ������ ������ accept �� �� �ִ� ���°� �ǵ����� (listen)
	int listenResult = listen(listenSock, SOMAXCONN);
	if (listenResult == SOCKET_ERROR) {
		cerr << "Can't listen a socket! Quiting" << endl;
		closesocket(listenSock);
		WSACleanup();
		return -1;
	}


	// [5] Ŭ���̾�Ʈ ��û�� ������ accept �Լ��� ���� ������ ����
	sockaddr_in clientSockInfo;
	int clientSize = sizeof(clientSockInfo);

	SOCKET clientSocket = accept(listenSock, reinterpret_cast<sockaddr*>(&clientSockInfo), &clientSize);
	if (clientSocket == INVALID_SOCKET) {
		cerr << "Can't accpet a socket! Quiting" << endl;
		closesocket(listenSock);
		WSACleanup();
		return -1;
	}

	// [6] ������ �Ǿ����� listening ���� ������ �ݽ��ϴ�. (����ؼ� �ٸ� Ŭ���̾�Ʈ ������ �޴´ٸ� ����)
	int closeResult = closesocket(listenSock);

	// [7] Ŭ���̾�Ʈ�� ��û�� �ް� ������ ������ ����
	char host[NI_MAXHOST];		// Ŭ���̾�Ʈ host �̸�
	char service[NI_MAXHOST];	// Ŭ���̾�Ʈ PORT ��ȣ
	ZeroMemory(host, NI_MAXHOST);	// memset(host, 0, NI_MAXHOST)�� ����
	ZeroMemory(service, NI_MAXHOST);

	// clientSockInfo�� ����� IP �ּҸ� ���� ������ ������ ����ϴ�.
	// host �̸��� host��, ��Ʈ ��ȣ�� service�� ����˴ϴ�.
	// getnameinfo()�� ������ 0�� ��ȯ�մϴ�. ���н� 0�� �ƴ� ���� ��ȯ�մϴ�.
	if (getnameinfo((sockaddr*)&clientSockInfo, sizeof(clientSockInfo), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0) {
		cout << host << " connected On Port " << service << endl;
	}
	else {
		inet_ntop(AF_INET, &clientSockInfo.sin_addr, host, NI_MAXHOST);
		cout << host << " connected On Port " << ntohs(clientSockInfo.sin_port) << endl;
	}

	// While Loop : Ŭ���̾�Ʈ�� �޼����� �޾Ƽ� ��� ��, Ŭ���̾�Ʈ���� �ٽ� �����ϴ�.
	enum eBufSize { BUF_SIZE = 4096 };
	char buf[BUF_SIZE];

	while (true) {
		ZeroMemory(buf, BUF_SIZE);

		// Wait for cleint to send data
		// �޼����� ���������� �޾�����, recv �Լ��� �޼����� ũ�⸦ ��ȯ�Ѵ�.
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