#pragma once
#include"../SocketInit/SocketInit.hpp"
#include"../MessageType/msgType.hpp"
class TcpClient
{
	SocketInit socketinit;
	SOCKET sClient;
public:
	int roomID;   //找到客户端所在的房间
	TcpClient();
	virtual ~TcpClient();

	//连接服务器
	void connectServer(const char* ip, unsigned short port);

	//接收数据
	void RecvData();

	//发送数据
	void SendData(MsgHead* pMsgHead);

	//接口
	SOCKET getSocket();

	//处理数据
	void OnMsgProc(MsgHead* buff);
};

