#include "TcpServer.h"

int ThreadRecv::IsRun = 1;

ClientObject::ClientObject()
{
	m_cs = INVALID_SOCKET;  //无效的
	m_lastPos = 0;
}

ClientObject::~ClientObject()
{
	if (m_cs != INVALID_SOCKET)
	{
		closesocket(m_cs);
	}
}

SOCKET ClientObject::getSocket()
{
	return m_cs;
}

void ClientObject::setSocket(SOCKET s)
{
	m_cs = s;
}

char* ClientObject::getBuffer()
{
	return m_buffer;
}

int ClientObject::getLastPos()
{
	return m_lastPos;
}

void ClientObject::setLastPos(int pos)
{
	m_lastPos = pos;
}

void ClientObject::SendData(MsgHead* pMsgHead)
{
	send(m_cs, (const char*)pMsgHead, pMsgHead->dataLen, 0);
}

TcpServer::TcpServer()
{
	m_threadCount = 0;

	m_pThreadRecv = NULL;

	FD_ZERO(&fd_read);  //初始化

	//创建监听套接字
	sListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (sListen == SOCKET_ERROR)
	{
		cout << "监听套接字创建失败！" << endl;
		return ;
	}
}

//绑定套接字
void TcpServer::Bind(unsigned short port)
{
	sockaddr_in sock_in;
	sock_in.sin_family = AF_INET;    //协议组
	sock_in.sin_port = htons(port);   //端口号
	sock_in.sin_addr.S_un.S_addr = INADDR_ANY;   //ip
	//绑定套接字
	int ret = bind(sListen, (const sockaddr*)&sock_in, sizeof(sock_in));

	if (ret == SOCKET_ERROR)
	{
		cout << "绑定套接字失败！" << endl;
		return ;
	}
}

//监听套接字
void TcpServer::Listen()
{
	if (listen(sListen, 10) == SOCKET_ERROR)
	{
		cout << "监听失败！" << endl;
		return ;
	}
	FD_SET(sListen, &fd_read);   //把socket放在集合里
}

//选择
void TcpServer::OnSelect()
{
	while (true)
	{
		FD_SET tmp = fd_read; //fd_read作为备份
		const timeval tv = { 1,0 };   //定义等待时间

		//select进行筛选，有网络事件的套接字会在tmp中留下
		int ret = select(NULL, &tmp, NULL, NULL, &tv);
		if (ret == 0)  //如果没有网络事件发生
		{
			Sleep(1);
			continue;
		}
		for (int i = 0; i < tmp.fd_count; i++)
		{
			//如果监听套接字有网络事件，证明有客户端在连接服务器
			if (tmp.fd_array[i] == sListen)
			{
				sockaddr_in clientAddr;
				int nlen = sizeof(sockaddr_in);
				//接受客户端的连接
				SOCKET sClient = accept(sListen, (sockaddr*)&clientAddr, &nlen);

				if (sClient == SOCKET_ERROR)
				{
					cout << "接受客户端失败！" << endl;
					return ;
				}

				cout << "与客户端建立连接！" << inet_ntoa(clientAddr.sin_addr) << endl;

				ClientObject* pcb = new ClientObject;
				pcb->setSocket(sClient);
				m_vecClient.push_back(pcb);

				//分配给线程处理  看看线程中处理客户端最少的线程
				ThreadRecv* tmp = m_pThreadRecv;
				for (int i = 1; i < m_threadCount; i++)
				{
					if (tmp->getMapClientSize() > m_pThreadRecv[i].getMapClientSize())
					{
						tmp = &m_pThreadRecv[i];
					}
				}
				tmp->addtoClientBuffer(pcb);
				FD_SET(sClient, &fd_read);   //把socket放在集合里
			}
			else  //如果客户端套接字有网络事件，证明客户端在发消息
			{
				
			}
		}

		//使用线程实现多个进程与服务器通信
		//CreateThread(NULL, NULL, ThreadProc, (LPVOID)&sClient, NULL, NULL);
	}
}

//处理数据
void ClientObject::OnMsgProc(ThreadRecv* pThead, MsgHead* pMsgHead)
{
	pThead->m_pBaseThis->OnMsgProc(this, pMsgHead);
}

ThreadRecv::ThreadRecv()
{
	clientchange = true;
	InitializeCriticalSection(&m_sec);   //初始化临界资源对象
}
ThreadRecv::~ThreadRecv()
{
	WaitForSingleObject(m_h,INFINITE);
	DeleteCriticalSection(&m_sec);      //释放临界资源对象
}

DWORD WINAPI ThreadRecv::TreadPro(LPVOID lp)
{
	ThreadRecv* threadRecv = (ThreadRecv*)lp;
	while (true)
	{
		//生产者转移到消费者
		if (!threadRecv->m_vecClientBuffer.empty())
		{
			//线程间的同步-加锁
			EnterCriticalSection(&threadRecv->m_sec);
			for (int s = 0; s < threadRecv->m_vecClientBuffer.size(); s++)
			{
				threadRecv->m_mapClient[threadRecv->m_vecClientBuffer[s]->getSocket()] = threadRecv->m_vecClientBuffer[s];
			}
			threadRecv->m_vecClientBuffer.clear();
			threadRecv->clientchange = true;
			LeaveCriticalSection(&threadRecv->m_sec);
		}

		//如果客户端发生变化了，更新m_fd_recv
		if (threadRecv->clientchange)
		{
			auto iterBegin = threadRecv->m_mapClient.begin();
			int i = 0;
			for (; iterBegin != threadRecv->m_mapClient.end(); iterBegin++, i++)
			{
				threadRecv->m_fd_recv.fd_array[i] = iterBegin->second->getSocket();
			}
			threadRecv->m_fd_recv.fd_count = i;
		}
		FD_SET fd_tmp = threadRecv->m_fd_recv; //m_fd_recv作为备份
		const timeval tv = { 1,0 };   //定义等待时间

		//select进行筛选，有网络事件的套接字会在tmp中留下
		int ret = select(NULL, &fd_tmp, NULL, NULL, &tv);
		if (ret > 0) 
		{
			for (int i = 0; i < fd_tmp.fd_count; i++)
			{
				//接受数据
				char buff[1024] = { 0 };

				auto iter = threadRecv->m_mapClient.find(fd_tmp.fd_array[i]);
				if (iter != threadRecv->m_mapClient.end())
				{
					char* clientBuffer = iter->second->getBuffer();
					int lastPos = iter->second->getLastPos();
					int nlen = recv(fd_tmp.fd_array[i], clientBuffer+lastPos, PACKET_MAX_SIZE-lastPos, 0);

					if (nlen > 0)
					{
						//找到map中的客户端对象并接收数据
						MsgHead* pHead = (MsgHead*)clientBuffer;
						lastPos = lastPos + nlen;
						//循环拆包
						while (lastPos >= sizeof(MsgHead))    //  = 必须加（问题）
						{
							if (lastPos >= pHead->dataLen)     //  = 必须加（问题）
							{
								//处理数据
								iter->second->OnMsgProc(threadRecv,(MsgHead*)pHead);
								memcpy(clientBuffer, clientBuffer + pHead->dataLen, lastPos - pHead->dataLen);
								lastPos -= pHead->dataLen;
							}
							else
							{
								iter->second->OnMsgProc(threadRecv,(MsgHead*)buff);
								break;
							}
						}
						iter->second->setLastPos(lastPos);
					}
					else
					{
						//从真实客户端（map<>）中移除当前客户端SOCKET  (sClient)
						cout << "客户端断开连接! " << fd_tmp.fd_array[i] << endl;
						threadRecv->m_mapClient.erase(iter);
						threadRecv->clientchange = true;
						//break;
					}
				}
			}
		}
		//else
		//{
		//	Sleep(1);
		//	continue;
		//}
	}
	return 0;
}

void ThreadRecv::start()
{
	m_h = CreateThread(0, 0, TreadPro, (LPVOID)this, 0, 0);
}

int ThreadRecv::getMapClientSize()
{
	return m_mapClient.size();
}

void ThreadRecv::addtoClientBuffer(ClientObject* p)
{
	EnterCriticalSection(&m_sec);
	m_vecClientBuffer.push_back(p);
	LeaveCriticalSection(&m_sec);
}

void ThreadRecv::setBaseObject(TcpServer* p)
{
	this->m_pBaseThis = p;
}

//启动线程
void TcpServer::StartThread(int nCount)
{
	m_threadCount = nCount;
	m_pThreadRecv = new ThreadRecv[nCount];
	for (int i = 0; i < nCount; i++)
	{
		m_pThreadRecv[i].start();
		m_pThreadRecv[i].setBaseObject(this);
	}
}

void TcpServer::OnMsgProc(ClientObject* pClientObject, MsgHead* pMsgHead)
{
	//派生类进行改写了
}


TcpServer::~TcpServer()
{
	ThreadRecv::IsRun = 0;

	closesocket(sListen);

	for (int s = m_vecClient.size() - 1; s >= 0; s--)
	{
		delete m_vecClient[s];
	}

	if (m_pThreadRecv != NULL)
	{
		delete []m_pThreadRecv;
	}
}
