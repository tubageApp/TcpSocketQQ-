#include<iostream>
#include"TcpClient.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS

using namespace std;

DWORD WINAPI ThreadProc(LPVOID lp)
{
	TcpClient *sClient = (TcpClient*)lp;
	cout << "show join create leave talk exit" << endl;
	while (true)
	{
		char buff[1024] = { 0 };
		gets_s(buff, 1024);
		
		if (strcmp(buff, "show") == 0)
		{
			//显示当前服务器下具体有哪些聊天室群
			MsgShow msgShow;
			sClient->SendData(&msgShow);
			//send(sClient,(const char*)&msgShow, msgShow.dataLen, 0);
		}
		else if (strncmp(buff, "join", strlen("join")) == 0)
		{
			//加入一个聊天室 join123
			int id = atoi(buff + strlen("join"));
			MsgJoin msgJoin(id);
			sClient->SendData(&msgJoin);
			sClient->roomID = id;
			//send(sClient, (const char*)&msgJoin, msgJoin.dataLen, 0);
		}
		//else if (strncmp(buff, "create", strlen("create")) == 0)
		//{
		//	//创建一个聊天室
		//	int id = atoi(buff + strlen("create"));
		//	MsgCreate msgCreate(id);
		//	sClient->SendData(&msgCreate);
		//}
		else if (strcmp(buff, "create") == 0)
		{
			//创建一个聊天室
			MsgCreate msgCreate;
			sClient->SendData(&msgCreate);
		}
		else if (strcmp(buff, "talk") == 0)
		{
			//聊天
			cout << "开始聊天" << endl;
			while (true)
			{
				MsgTalk msgTalk(sClient->roomID);
				gets_s(msgTalk.getBuff(), 1000);
				if (strcmp(msgTalk.getBuff(), "leave") == 0)
				{
					cout << "结束聊天..." << endl;
					MsgLeave msgLeave;
					msgLeave.roomID = sClient->roomID;
					sClient->SendData(&msgLeave);
					//send(sClient, (const char*)&msgLeave, msgLeave.dataLen, 0);
					break;
				}
				sClient->SendData(&msgTalk);
				//send(sClient, (const char*)&msgTalk, msgTalk.dataLen, 0);
			}
		}
		else if (strcmp(buff, "exit") == 0)
		{
			//退出
			//ExitThread()
			break;
		}
	}
	return -1;
}

#define TCPCLIENT_NUM 1

int main()
{
	TcpClient tcpClient;

	//连接服务器
	tcpClient.connectServer("192.168.44.1",12306);

	//多线程实现收发数据分离操作
	//发送指令
	HANDLE h = CreateThread(NULL, NULL, ThreadProc, (LPVOID)&tcpClient, NULL, NULL);

	//发送数据
	//while (true)
	//{
	//	char buff[1024] = { 0 };
	//	gets_s(buff, 1024);
	//	send(sClient, buff, strlen(buff), 0);
	//}

	while (true)
	{
		tcpClient.RecvData();
	}

	WaitForSingleObject(h, INFINITE);

	getchar();

	return 0;
}