#include "TalkingRoom.h"
#define ROOM_MEMBER_MAX_NUM 50

TalkingRoom::TalkingRoom()
{
		InitializeCriticalSection(&m_cs);
}


void TalkingRoom::addMember(ClientObject* co)
{
	EnterCriticalSection(&m_cs);
	if (m_roomMember.size() < ROOM_MEMBER_MAX_NUM)
	{
		m_roomMember[co->getSocket()] = co;
	}
	LeaveCriticalSection(&m_cs);
}

void TalkingRoom::deleteMember(ClientObject* co)
{
	EnterCriticalSection(&m_cs);
	auto iter = m_roomMember.find(co->getSocket());
	if (iter != m_roomMember.end())
	{
		m_roomMember.erase(iter);
	}
	LeaveCriticalSection(&m_cs);
}

void TalkingRoom::sendToAll(ClientObject* co, MsgHead* pMsgHead)
{
	auto iter = m_roomMember.begin();
	while (iter != m_roomMember.end())
	{
		if (iter->first != co->getSocket())
		{
			iter->second->SendData(pMsgHead);
		}
		iter++;
	}
}

TalkingRoom::~TalkingRoom()
{
	DeleteCriticalSection(&m_cs);
}