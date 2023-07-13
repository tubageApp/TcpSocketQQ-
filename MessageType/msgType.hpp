#pragma once

#define PACKET_MAX_SIZE 1024
#define ROOM_MAX_NUM 100

//#define MSG_SHOW 1    定义宏

//使用枚举
enum MSG_TYPE
{
	MSG_SHOW,
	MSG_SHOW_REPLY,
	MSG_JOIN,
	MSG_CREATE,
	MSG_CREATE_REPLY,
	MSG_LEAVE,
	MSG_TALK
};

//消息的封装

class MsgHead  //消息基类
{
public:
	int msgType;   //消息类型
	int dataLen;   //消息长度
};

class MsgShow : public MsgHead
{
public:
	MsgShow()
	{
		msgType = MSG_SHOW;
		dataLen = sizeof(MsgShow);
	}
};

class MsgShowReply : public MsgHead
{
public:
	int roomCount;
	struct RoomInfo
	{
		int roomID;
		int onlineNum;
		int totalNum;
	};
	RoomInfo rooms[ROOM_MAX_NUM];
public:
	MsgShowReply()
	{
		msgType = MSG_SHOW_REPLY;
		dataLen = sizeof(MsgShowReply);
	}
};

class MsgJoin : public MsgHead
{
public:
	int roomID;

	MsgJoin(int id)
	{
		msgType = MSG_JOIN;
		dataLen = sizeof(MsgJoin);
		roomID = id;
	}
};

class MsgCreate : public MsgHead
{
public:
	MsgCreate()
	{
		msgType = MSG_CREATE;
		dataLen = sizeof(MsgCreate);
	}
};

class MsgCreateReply : public MsgHead
{
public:
	int roomID;
	int onlineNum;
	int totalNum;
public:
	MsgCreateReply()
	{
		msgType = MSG_CREATE_REPLY;
		dataLen = sizeof(MsgCreateReply);
	}
};

class MsgLeave : public MsgHead
{
public:
	int roomID;
	MsgLeave()
	{
		roomID = 0;
		msgType = MSG_LEAVE;
		dataLen = sizeof(MsgLeave);
	}
};

class MsgTalk : public MsgHead
{
	char buff[1000];
public:
	int roomID;
	int ID;
	MsgTalk(int id)
	{
		msgType = MSG_TALK;
		dataLen = sizeof(MsgTalk);
		this->roomID = id;
		ID = 0;
	}
	char* getBuff()
	{
		return buff;
	}
};