#include "TcpClient.h"
#include<iostream>
using namespace std;

TcpClient::TcpClient()
{
	//�����ͻ����׽���
	sClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (sClient == SOCKET_ERROR)
	{
		cout << "�ͻ��׽��ִ���ʧ�ܣ�" << endl;
		return ;
	}
	roomID = 0;
}
TcpClient::~TcpClient()
{
	closesocket(sClient);
}

void TcpClient::connectServer(const char* ip, unsigned short port)
{
	sockaddr_in sock_in;
	sock_in.sin_family = AF_INET;    //Э����
	sock_in.sin_port = htons(port);   //�˿ں�
	sock_in.sin_addr.S_un.S_addr =  inet_addr(ip);   //ip "192.168.44.1"

	if (connect(sClient, (const sockaddr*)&sock_in, sizeof(sockaddr_in)) == SOCKET_ERROR)
	{
		cout << "���ӷ�����ʧ�ܣ�" << endl;
		return ;
	}
}

//��������
void TcpClient::RecvData()
{
	while (true)
	{
		char buff[1024] = { 0 };

		FD_SET fd_recv;   //�洢SOCKET����
		FD_ZERO(&fd_recv);  //��ʼ��
		FD_SET(sClient, &fd_recv);   //��socket���ڼ�����
		const timeval tv = { 1,0 };   //����ȴ�ʱ��
		int s = select(NULL, &fd_recv, NULL, NULL, &tv);
		if (s > 0)
		{
			int ret = recv(sClient, buff, 1024, 0);
			if (ret > 0)
			{
				//������
				OnMsgProc((MsgHead*) buff);
			}
			else
			{
				cout << "�ͻ��˽�������ʧ��..." << endl;
				break;
			}
		}
	}
}

//��������
void TcpClient::SendData(MsgHead* pMsgHead)
{
	send(sClient, (const char*)pMsgHead, pMsgHead->dataLen, 0);
}

void TcpClient::OnMsgProc(MsgHead* msgHead)
{
	switch (msgHead->msgType)
	{
	case MSG_SHOW_REPLY:
	{
		MsgShowReply* reply = (MsgShowReply*)msgHead;
		cout << "--������Ϣ--" << endl;
		for (int i = 0; i < reply->roomCount; i++)
		{
			cout << "RoomID:" << reply->rooms[i].roomID;
			cout << "(" << reply->rooms[i].onlineNum << "/" << reply->rooms[i].totalNum << ")" << endl;
		}
		cout << "-----end-----" << endl;
		break;
	}
	case MSG_CREATE_REPLY:
	{
		MsgCreateReply* reply = (MsgCreateReply*)msgHead;
		cout << "��������ɹ���" << endl;
		roomID = reply->roomID;
		cout << "--�·�����Ϣ--" << endl;
		cout << "RoomID:" << reply->roomID;
		cout << "(" << reply->onlineNum << "/" << reply->totalNum << ")" << endl;
		cout << "-----end-----" << endl;
	}
	break;
	case MSG_TALK:
	{
		MsgTalk* reply = (MsgTalk*)msgHead;
		cout << reply->ID << "����Ϣ��";
		cout << reply->getBuff() << endl;
	}
	break;
	default:
		//cout << "��Ϣ����ʧ�ܣ�........" << endl;
		break;
	}
}

//�ӿ�
SOCKET TcpClient::getSocket()
{
	return sClient;
}