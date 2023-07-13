#include "TcpServer.h"

int ThreadRecv::IsRun = 1;

ClientObject::ClientObject()
{
	m_cs = INVALID_SOCKET;  //��Ч��
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

	FD_ZERO(&fd_read);  //��ʼ��

	//���������׽���
	sListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (sListen == SOCKET_ERROR)
	{
		cout << "�����׽��ִ���ʧ�ܣ�" << endl;
		return ;
	}
}

//���׽���
void TcpServer::Bind(unsigned short port)
{
	sockaddr_in sock_in;
	sock_in.sin_family = AF_INET;    //Э����
	sock_in.sin_port = htons(port);   //�˿ں�
	sock_in.sin_addr.S_un.S_addr = INADDR_ANY;   //ip
	//���׽���
	int ret = bind(sListen, (const sockaddr*)&sock_in, sizeof(sock_in));

	if (ret == SOCKET_ERROR)
	{
		cout << "���׽���ʧ�ܣ�" << endl;
		return ;
	}
}

//�����׽���
void TcpServer::Listen()
{
	if (listen(sListen, 10) == SOCKET_ERROR)
	{
		cout << "����ʧ�ܣ�" << endl;
		return ;
	}
	FD_SET(sListen, &fd_read);   //��socket���ڼ�����
}

//ѡ��
void TcpServer::OnSelect()
{
	while (true)
	{
		FD_SET tmp = fd_read; //fd_read��Ϊ����
		const timeval tv = { 1,0 };   //����ȴ�ʱ��

		//select����ɸѡ���������¼����׽��ֻ���tmp������
		int ret = select(NULL, &tmp, NULL, NULL, &tv);
		if (ret == 0)  //���û�������¼�����
		{
			Sleep(1);
			continue;
		}
		for (int i = 0; i < tmp.fd_count; i++)
		{
			//��������׽����������¼���֤���пͻ��������ӷ�����
			if (tmp.fd_array[i] == sListen)
			{
				sockaddr_in clientAddr;
				int nlen = sizeof(sockaddr_in);
				//���ܿͻ��˵�����
				SOCKET sClient = accept(sListen, (sockaddr*)&clientAddr, &nlen);

				if (sClient == SOCKET_ERROR)
				{
					cout << "���ܿͻ���ʧ�ܣ�" << endl;
					return ;
				}

				cout << "��ͻ��˽������ӣ�" << inet_ntoa(clientAddr.sin_addr) << endl;

				ClientObject* pcb = new ClientObject;
				pcb->setSocket(sClient);
				m_vecClient.push_back(pcb);

				//������̴߳���  �����߳��д���ͻ������ٵ��߳�
				ThreadRecv* tmp = m_pThreadRecv;
				for (int i = 1; i < m_threadCount; i++)
				{
					if (tmp->getMapClientSize() > m_pThreadRecv[i].getMapClientSize())
					{
						tmp = &m_pThreadRecv[i];
					}
				}
				tmp->addtoClientBuffer(pcb);
				FD_SET(sClient, &fd_read);   //��socket���ڼ�����
			}
			else  //����ͻ����׽����������¼���֤���ͻ����ڷ���Ϣ
			{
				
			}
		}

		//ʹ���߳�ʵ�ֶ�������������ͨ��
		//CreateThread(NULL, NULL, ThreadProc, (LPVOID)&sClient, NULL, NULL);
	}
}

//��������
void ClientObject::OnMsgProc(ThreadRecv* pThead, MsgHead* pMsgHead)
{
	pThead->m_pBaseThis->OnMsgProc(this, pMsgHead);
}

ThreadRecv::ThreadRecv()
{
	clientchange = true;
	InitializeCriticalSection(&m_sec);   //��ʼ���ٽ���Դ����
}
ThreadRecv::~ThreadRecv()
{
	WaitForSingleObject(m_h,INFINITE);
	DeleteCriticalSection(&m_sec);      //�ͷ��ٽ���Դ����
}

DWORD WINAPI ThreadRecv::TreadPro(LPVOID lp)
{
	ThreadRecv* threadRecv = (ThreadRecv*)lp;
	while (true)
	{
		//������ת�Ƶ�������
		if (!threadRecv->m_vecClientBuffer.empty())
		{
			//�̼߳��ͬ��-����
			EnterCriticalSection(&threadRecv->m_sec);
			for (int s = 0; s < threadRecv->m_vecClientBuffer.size(); s++)
			{
				threadRecv->m_mapClient[threadRecv->m_vecClientBuffer[s]->getSocket()] = threadRecv->m_vecClientBuffer[s];
			}
			threadRecv->m_vecClientBuffer.clear();
			threadRecv->clientchange = true;
			LeaveCriticalSection(&threadRecv->m_sec);
		}

		//����ͻ��˷����仯�ˣ�����m_fd_recv
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
		FD_SET fd_tmp = threadRecv->m_fd_recv; //m_fd_recv��Ϊ����
		const timeval tv = { 1,0 };   //����ȴ�ʱ��

		//select����ɸѡ���������¼����׽��ֻ���tmp������
		int ret = select(NULL, &fd_tmp, NULL, NULL, &tv);
		if (ret > 0) 
		{
			for (int i = 0; i < fd_tmp.fd_count; i++)
			{
				//��������
				char buff[1024] = { 0 };

				auto iter = threadRecv->m_mapClient.find(fd_tmp.fd_array[i]);
				if (iter != threadRecv->m_mapClient.end())
				{
					char* clientBuffer = iter->second->getBuffer();
					int lastPos = iter->second->getLastPos();
					int nlen = recv(fd_tmp.fd_array[i], clientBuffer+lastPos, PACKET_MAX_SIZE-lastPos, 0);

					if (nlen > 0)
					{
						//�ҵ�map�еĿͻ��˶��󲢽�������
						MsgHead* pHead = (MsgHead*)clientBuffer;
						lastPos = lastPos + nlen;
						//ѭ�����
						while (lastPos >= sizeof(MsgHead))    //  = ����ӣ����⣩
						{
							if (lastPos >= pHead->dataLen)     //  = ����ӣ����⣩
							{
								//��������
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
						//����ʵ�ͻ��ˣ�map<>�����Ƴ���ǰ�ͻ���SOCKET  (sClient)
						cout << "�ͻ��˶Ͽ�����! " << fd_tmp.fd_array[i] << endl;
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

//�����߳�
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
	//��������и�д��
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
