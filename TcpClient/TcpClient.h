#pragma once
#include"../SocketInit/SocketInit.hpp"
#include"../MessageType/msgType.hpp"
class TcpClient
{
	SocketInit socketinit;
	SOCKET sClient;
public:
	int roomID;   //�ҵ��ͻ������ڵķ���
	TcpClient();
	virtual ~TcpClient();

	//���ӷ�����
	void connectServer(const char* ip, unsigned short port);

	//��������
	void RecvData();

	//��������
	void SendData(MsgHead* pMsgHead);

	//�ӿ�
	SOCKET getSocket();

	//��������
	void OnMsgProc(MsgHead* buff);
};

