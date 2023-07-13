#pragma once
#include<iostream>
#include<vector>
#include<map>
#include"../SocketInit/SocketInit.hpp"
#include"../MessageType/msgType.hpp"

using namespace std;

class ThreadRecv;
//��װһ���ͻ�����
class ClientObject
{
private:
	SOCKET m_cs;
public:
	ClientObject();
	virtual ~ClientObject();
	SOCKET getSocket();
	void setSocket(SOCKET s);

	//��������
	void OnMsgProc(ThreadRecv* pThead, MsgHead* pMsgHead);

	char* getBuffer();
	int getLastPos();
	void setLastPos(int pos);

	//��������
	void SendData(MsgHead* pMsgHead);
private:
	//��������ճ�����
	char m_buffer[PACKET_MAX_SIZE * 2];
	int m_lastPos;
};

class TcpServer;

//�����߳���
class ThreadRecv
{
	HANDLE m_h;
private:
	vector<ClientObject*>m_vecClientBuffer;   //��ʱ�Ŀͻ��ˣ����̹߳���
	map<SOCKET,ClientObject*>m_mapClient;   //��ʵ��һ���ֿͻ���
	FD_SET m_fd_recv;
	int clientchange;  //���  �ж��Ƿ��пͻ��˽���
	CRITICAL_SECTION m_sec;   //���ٽ���Դ
	
public:
	static int IsRun;
	ThreadRecv();
	~ThreadRecv();
	void start();  //�����̺߳���
	static DWORD WINAPI TreadPro(LPVOID lp);   //�̺߳��������������������Ϊ��̬
	int getMapClientSize(); //��ȡ�ͻ�������

	void addtoClientBuffer(ClientObject* p);
	void setBaseObject(TcpServer* p);

	TcpServer* m_pBaseThis;
};

class TcpServer
{
	SocketInit socketinit;
	SOCKET sListen;

	FD_SET fd_read;   //�洢SOCKET����

	ThreadRecv* m_pThreadRecv;  //�����̶߳���

	//����������ȥά��һ���ͻ�������
	vector<ClientObject*>m_vecClient;    //�������еĿͻ���

	int m_threadCount;

public:
	TcpServer();
	virtual ~TcpServer();

	//���׽���
	void Bind(unsigned short port);

	//�����׽���
	void Listen();
	
	//ѡ��
	void OnSelect();

	//�����߳�
	void StartThread(int nCount);

	//����ָ��
	virtual void OnMsgProc(ClientObject* pClientObject,MsgHead* pMsgHead);

};

