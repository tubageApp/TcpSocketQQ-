#pragma once
#include<iostream>
#include<vector>
#include<map>
#include"../SocketInit/SocketInit.hpp"
#include"../MessageType/msgType.hpp"

using namespace std;

class ThreadRecv;
//封装一个客户端类
class ClientObject
{
private:
	SOCKET m_cs;
public:
	ClientObject();
	virtual ~ClientObject();
	SOCKET getSocket();
	void setSocket(SOCKET s);

	//处理数据
	void OnMsgProc(ThreadRecv* pThead, MsgHead* pMsgHead);

	char* getBuffer();
	int getLastPos();
	void setLastPos(int pos);

	//发送数据
	void SendData(MsgHead* pMsgHead);
private:
	//处理数据粘包情况
	char m_buffer[PACKET_MAX_SIZE * 2];
	int m_lastPos;
};

class TcpServer;

//创建线程类
class ThreadRecv
{
	HANDLE m_h;
private:
	vector<ClientObject*>m_vecClientBuffer;   //临时的客户端，多线程共享
	map<SOCKET,ClientObject*>m_mapClient;   //真实的一部分客户端
	FD_SET m_fd_recv;
	int clientchange;  //标记  判断是否有客户端接入
	CRITICAL_SECTION m_sec;   //化临界资源
	
public:
	static int IsRun;
	ThreadRecv();
	~ThreadRecv();
	void start();  //启动线程函数
	static DWORD WINAPI TreadPro(LPVOID lp);   //线程函数放在类里面必须声明为静态
	int getMapClientSize(); //获取客户端数量

	void addtoClientBuffer(ClientObject* p);
	void setBaseObject(TcpServer* p);

	TcpServer* m_pBaseThis;
};

class TcpServer
{
	SocketInit socketinit;
	SOCKET sListen;

	FD_SET fd_read;   //存储SOCKET对象

	ThreadRecv* m_pThreadRecv;  //创建线程对象

	//服务器里面去维护一个客户端容器
	vector<ClientObject*>m_vecClient;    //保存所有的客户端

	int m_threadCount;

public:
	TcpServer();
	virtual ~TcpServer();

	//绑定套接字
	void Bind(unsigned short port);

	//监听套接字
	void Listen();
	
	//选择
	void OnSelect();

	//启动线程
	void StartThread(int nCount);

	//处理指令
	virtual void OnMsgProc(ClientObject* pClientObject,MsgHead* pMsgHead);

};

