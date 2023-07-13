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
			//��ʾ��ǰ�������¾�������Щ������Ⱥ
			MsgShow msgShow;
			sClient->SendData(&msgShow);
			//send(sClient,(const char*)&msgShow, msgShow.dataLen, 0);
		}
		else if (strncmp(buff, "join", strlen("join")) == 0)
		{
			//����һ�������� join123
			int id = atoi(buff + strlen("join"));
			MsgJoin msgJoin(id);
			sClient->SendData(&msgJoin);
			sClient->roomID = id;
			//send(sClient, (const char*)&msgJoin, msgJoin.dataLen, 0);
		}
		//else if (strncmp(buff, "create", strlen("create")) == 0)
		//{
		//	//����һ��������
		//	int id = atoi(buff + strlen("create"));
		//	MsgCreate msgCreate(id);
		//	sClient->SendData(&msgCreate);
		//}
		else if (strcmp(buff, "create") == 0)
		{
			//����һ��������
			MsgCreate msgCreate;
			sClient->SendData(&msgCreate);
		}
		else if (strcmp(buff, "talk") == 0)
		{
			//����
			cout << "��ʼ����" << endl;
			while (true)
			{
				MsgTalk msgTalk(sClient->roomID);
				gets_s(msgTalk.getBuff(), 1000);
				if (strcmp(msgTalk.getBuff(), "leave") == 0)
				{
					cout << "��������..." << endl;
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
			//�˳�
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

	//���ӷ�����
	tcpClient.connectServer("192.168.44.1",12306);

	//���߳�ʵ���շ����ݷ������
	//����ָ��
	HANDLE h = CreateThread(NULL, NULL, ThreadProc, (LPVOID)&tcpClient, NULL, NULL);

	//��������
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