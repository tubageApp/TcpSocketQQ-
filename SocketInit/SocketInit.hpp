#ifndef _SOCKET_INIT_H_
#define _SOCKET_INIT_H_

#include<iostream>
using namespace std;

//����ͨ����Ҫ������ͷ�ļ�����Ҫ���صĿ��ļ�
#include<WinSock2.h>
#pragma comment(lib,"ws2_32.lib")

class SocketInit 
{
public:
	//��ʼ��Socket,���ض�̬���ӿ�
	SocketInit()
	{
		WORD socketVersion = MAKEWORD(2, 2);
		WSADATA wsaData;
		if (WSAStartup(socketVersion, &wsaData) != 0)
		{
			cout << "��̬���ӿ����ʧ��! " << endl;
		}
		else
		{
			cout << "��̬���ӿ���سɹ�! " << endl;
		}
	}

	~SocketInit()
	{
		WSACleanup();
	}
};

#endif // !_SOCKET_INIT_H_
