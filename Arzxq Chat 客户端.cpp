#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <thread>
#include <cstdio>
#include <string>

#pragma comment(lib,"ws2_32.lib")

using namespace std;


#pragma region 全局变量

SOCKET client;					// 本地套接字
sockaddr_in sai_client;			// 存放客户端IP地址、端口

// 消息格式
struct umsg {
	int type;					// 协议（1：登录，2：退出，3：发消息）
	char name[64];				// 用户名字
	char text[512];				// 文本
};

#pragma endregion

#pragma region 依赖函数

// 监听服务器消息
void recvMessage()
{


	while (1) {
		umsg msg;
		int ret_recv = recv(
			client, 		// 本地套接字
			(char*)&msg,	// 存放接收的消息
			sizeof(msg),	// 消息大小
			0				// 不指定调用方式
		);
		if (ret_recv <= 0) { cout << "recv failed: " << GetLastError() << endl; break; }

		// 打印消息
		cout << msg.name << ": " << msg.text << endl;
	}
}

#pragma endregion

int main()
{
	SetConsoleTitle(L"CMD Chat");
	cout << "欢迎使用 CMD 聊天软件|Welcome to CMD chat software" << endl;
	Sleep (500);
	cout << "当前在线人数: 获取失败 \n 服务器连接成功 \n 官网: CMD-Chat.Arzxq.com" << endl;


	// 初始化 WSA ，激活 socket
	WSADATA wsaData;
	if (WSAStartup(
		MAKEWORD(2, 2), 	// 规定 socket 版本
		&wsaData			// 接收 socket 的更多信息
	)) {
		cout << "WSAStartup failed : " << GetLastError() << endl;
	}

	// 初始化 socket、客户端信息
	client = socket(
		AF_INET,		// IPV4
		SOCK_STREAM,	// TCP
		0				// 不指定协议
	);
	sai_client.sin_family = AF_INET;									// IPV4
	inet_pton(AF_INET, "182.61.26.129", &sai_client.sin_addr);			// 服务器 IP地址
	sai_client.sin_port = htons(8090);									// 端口

	// 连接服务器
	int ret_connect = connect(
		client, 					// 本地套接字
		(sockaddr*)&sai_client, 	// 目标
		sizeof(sai_client)
	); if (ret_connect != 0) { cout << "连接失败:" << GetLastError() << endl; }

	cout <<"请输入用户名并回车确认" << endl;

	// 输入用户名
	umsg msg;
	msg.type = 1;
	string name;
	getline(cin, name);
	strncpy_s(msg.name, sizeof(msg.name), name.c_str(), 64);
	strncpy_s(msg.text, sizeof(msg.text), "", 512);


	cout << "登录成功!" << endl;


	// 发送登录消息
	send(
		client,			// 本地套接字
		(char*)&msg,	// 发送的消息
		sizeof(msg),	// 消息大小
		0				// 不指定调用方式
	);
	int error_send = GetLastError();
	if (error_send != 0) { cout << "send failed:" << error_send << endl; }

	// 接收服务器消息
	HANDLE h_recvMes = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)recvMessage, 0, 0, 0);
	if (!h_recvMes) { cout << "CreateThread failed :" << GetLastError() << endl; }

	// 发送消息
	while (1) {
		string content;
		getline(cin, content);

		// 退出消息
		if (content == "quit") {
			msg.type = 2;
			send(
				client,			// 本地套接字
				(char*)&msg,	// 发送的消息
				sizeof(msg),	// 消息大小
				0				// 不指定调用方式
			);
			error_send = GetLastError();
			if (error_send != 0) { cout << "send failed:" << error_send << endl; }
			closesocket(client);
			WSACleanup();
			return 0;
		}

		// 会话消息
		msg.type = 3;
		strncpy_s(msg.text, sizeof(msg.text), content.c_str(), 512);
		send(
			client,			// 本体套接字
			(char*)&msg,	// 发送的消息
			sizeof(msg),	// 消息大小
			0				// 不指定调用方式
		);
		error_send = GetLastError();
		if (error_send != 0) { cout << "send failed:" << error_send << endl; }

	}

	getchar();
	return 0;
}