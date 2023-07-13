#pragma once
#include<map>
#include"TcpServer.h"
class TalkingRoom
{
private:
	CRITICAL_SECTION m_cs;
public:
	int m_roomID;
	std::map<SOCKET, ClientObject*>m_roomMember;
	TalkingRoom();
	~TalkingRoom();
	void addMember(ClientObject* co);
	void deleteMember(ClientObject* co);
	void sendToAll(ClientObject* co, MsgHead* pMsgHead);
};

