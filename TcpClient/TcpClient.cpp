#include "TcpClient.h"
#include<iostream>
using namespace std;

TcpClient::TcpClient()
{
	//创建客户端套接字
	sClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (sClient == SOCKET_ERROR)
	{
		cout << "客户套接字创建失败！" << endl;
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
	sock_in.sin_family = AF_INET;    //协议组
	sock_in.sin_port = htons(port);   //端口号
	sock_in.sin_addr.S_un.S_addr =  inet_addr(ip);   //ip "192.168.44.1"

	if (connect(sClient, (const sockaddr*)&sock_in, sizeof(sockaddr_in)) == SOCKET_ERROR)
	{
		cout << "连接服务器失败！" << endl;
		return ;
	}
}

//接收数据
void TcpClient::RecvData()
{
	while (true)
	{
		char buff[1024] = { 0 };

		FD_SET fd_recv;   //存储SOCKET对象
		FD_ZERO(&fd_recv);  //初始化
		FD_SET(sClient, &fd_recv);   //把socket放在集合里
		const timeval tv = { 1,0 };   //定义等待时间
		int s = select(NULL, &fd_recv, NULL, NULL, &tv);
		if (s > 0)
		{
			int ret = recv(sClient, buff, 1024, 0);
			if (ret > 0)
			{
				//收数据
				OnMsgProc((MsgHead*) buff);
			}
			else
			{
				cout << "客户端接收数据失败..." << endl;
				break;
			}
		}
	}
}

//发送数据
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
		cout << "--房间信息--" << endl;
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
		cout << "创建房间成功！" << endl;
		roomID = reply->roomID;
		cout << "--新房间信息--" << endl;
		cout << "RoomID:" << reply->roomID;
		cout << "(" << reply->onlineNum << "/" << reply->totalNum << ")" << endl;
		cout << "-----end-----" << endl;
	}
	break;
	case MSG_TALK:
	{
		MsgTalk* reply = (MsgTalk*)msgHead;
		cout << reply->ID << "的消息：";
		cout << reply->getBuff() << endl;
	}
	break;
	default:
		//cout << "消息解析失败！........" << endl;
		break;
	}
}

//接口
SOCKET TcpClient::getSocket()
{
	return sClient;
}