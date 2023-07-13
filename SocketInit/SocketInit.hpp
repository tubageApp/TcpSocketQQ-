#ifndef _SOCKET_INIT_H_
#define _SOCKET_INIT_H_

#include<iostream>
using namespace std;

//网络通信需要包含的头文件、需要加载的库文件
#include<WinSock2.h>
#pragma comment(lib,"ws2_32.lib")

class SocketInit 
{
public:
	//初始化Socket,加载动态链接库
	SocketInit()
	{
		WORD socketVersion = MAKEWORD(2, 2);
		WSADATA wsaData;
		if (WSAStartup(socketVersion, &wsaData) != 0)
		{
			cout << "动态链接库加载失败! " << endl;
		}
		else
		{
			cout << "动态链接库加载成功! " << endl;
		}
	}

	~SocketInit()
	{
		WSACleanup();
	}
};

#endif // !_SOCKET_INIT_H_
