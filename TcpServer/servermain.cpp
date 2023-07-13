#include<iostream>
#include"TcpServer.h"
#include"TalkingRoom.h"
using namespace std;
#include<queue>

class RecourceMange
{
private:
	std::queue<int>m_queRoomId;
	static RecourceMange* instance;
	RecourceMange() {}
public:

	void InitRes()
	{
		for (int i = 1; i < 1000; i++)
		{
			m_queRoomId.push(i);
		}
	}
	static RecourceMange* getInstance()
	{
		if (instance == NULL)
		{
			instance = new RecourceMange;
		}
		return instance;
	}
	int getRoomId()
	{
		int t = m_queRoomId.front();
		m_queRoomId.pop();
		return t;
	}
	void PushRoomId(int id)
	{
		m_queRoomId.push(id);
	}
};

RecourceMange* RecourceMange::instance = NULL;

class MyTcpServer :public TcpServer
{
	map<int, TalkingRoom*> m_room;
public:
	void showRoom(ClientObject* pClientObject, MsgHead* pMsgHead)
	{
		MsgShowReply showReply;
		showReply.roomCount = m_room.size();
		auto iter = m_room.begin();
		int i = 0;
		while (iter != m_room.end())
		{
			showReply.rooms[i].roomID = iter->second->m_roomID;
			showReply.rooms[i].onlineNum = iter->second->m_roomMember.size();
			showReply.rooms[i].totalNum = ROOM_MAX_NUM;
			i++;
			iter++;
		}
		pClientObject->SendData(&showReply);
	}

	//void CreateRoom(ClientObject* pClientObject, MsgHead* pMsgHead, MsgCreate* msgCreate)
	//{
	//	TalkingRoom* tr = new TalkingRoom;
	//	tr->m_roomID = msgCreate->roomID;
	//	tr->addMember(pClientObject);
	//	m_room[msgCreate->roomID] = tr;
	//	showRoom(pClientObject, pMsgHead);
	//}
	void CreateRoom(ClientObject* pClientObject, MsgHead* pMsgHead)
	{
		RecourceMange* instance = RecourceMange::getInstance();
		instance->InitRes();
		int roomID = instance->getRoomId();
		TalkingRoom* tr = new TalkingRoom;
		tr->m_roomID = roomID;
		tr->addMember(pClientObject);
		m_room[roomID] = tr;
		//showRoom(pClientObject, pMsgHead);
		MsgCreateReply msgReply;
		msgReply.roomID = roomID;
		msgReply.onlineNum = 1;
		msgReply.totalNum = ROOM_MAX_NUM;
		pClientObject->SendData((MsgHead*)&msgReply);
	}

	void JoinRoom(ClientObject* pClientObject, MsgHead* pMsgHead)
	{
		MsgJoin* join = (MsgJoin*)pMsgHead;
		auto iter = m_room.find(join->roomID);
		if (iter != m_room.end())
		{
			iter->second->addMember(pClientObject);
		}
		showRoom(pClientObject, pMsgHead);
	}

	void LeaveRoom(ClientObject* pClientObject, MsgHead* pMsgHead)
	{
		MsgLeave* leave = (MsgLeave*)pMsgHead;
		auto iter = m_room.find(leave->roomID);
		if (iter != m_room.end())
		{
			iter->second->deleteMember(pClientObject);
		}
		//showRoom(pClientObject, pMsgHead);
	}

	void Talking(ClientObject* pClientObject, MsgHead* pMsgHead)
	{
		MsgTalk* talk = (MsgTalk*)pMsgHead;
		auto iter = m_room.find(talk->roomID);
		if (iter != m_room.end())
		{
			talk->ID = pClientObject->getSocket();
			iter->second->sendToAll(pClientObject, pMsgHead);
		}
	}

	void OnMsgProc(ClientObject* pClientObject, MsgHead* pMsgHead)
	{
		switch (pMsgHead->msgType)
		{
		case MSG_SHOW:
		{
			cout << "请求显示聊天室..." << endl;
			showRoom(pClientObject, pMsgHead);
			break; 
		}
		case MSG_JOIN:
		{
			MsgJoin* msgJoin = (MsgJoin*)pMsgHead;
			cout << "请求加入聊天室：" << msgJoin->roomID << endl;
			JoinRoom(pClientObject, pMsgHead);
			break;
		}
		case MSG_CREATE:
		{
			MsgCreate* msgCreate = (MsgCreate*)pMsgHead;
			cout << "请求创建聊天室..." << endl;
			//CreateRoom(pClientObject, pMsgHead, msgCreate);
			CreateRoom(pClientObject, pMsgHead);
			break;
		}
		case MSG_TALK:
		{
			MsgTalk* talk = (MsgTalk*)pMsgHead;
			cout << talk->getBuff() << endl;
			Talking(pClientObject, pMsgHead);
			break;
		}
		case MSG_LEAVE:
		{
			cout << pClientObject->getSocket()<< "离开聊天室..." << endl;
			LeaveRoom(pClientObject, pMsgHead);
			break;
		}
		default:
			cout << "消息解析失败..." << endl;
			break;
		}
	}
};

//DWORD WINAPI ThreadProc(LPVOID lp)
//{
//	SOCKET sClient = *(SOCKET*)lp;
//	//接受数据
//	while (true)
//	{
//		char buff[1024] = { 0 };
//		int result = recv(sClient, buff, 1024, 0);
//		if (result > 0)
//		{
//			cout << "接收的数据：" << buff << endl;
//		}
//		else
//		{
//			cout << "客户端断开连接! " << endl;
//			break;
//		}
//	}
//	return NULL;
//}

int main()
{
	MyTcpServer tcpServer;

	tcpServer.Bind(12306);

	tcpServer.Listen();

	tcpServer.StartThread(4);

	//接收客户端连接，接收数据
	tcpServer.OnSelect();

	getchar();

	return 0;
}