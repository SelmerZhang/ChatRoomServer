#pragma once

#include "stdafx.h"
#include "Message.h"
/*
  服务器端的socket，用于监听连接和接收、发送消息
*/

class ServerSocket : public CSocket
{
public:
	ServerSocket();
	virtual ~ServerSocket();
public:
	CPtrList m_pList;//保存服务器上所有socket的链表，表头为监听socket
	CPtrList *m_ptoList;
	CString m_UserName; //连接的客户端的用户名
public:
	virtual void OnClose(int nErrorCode);
	virtual void OnAccept(int nErrorCode); //接受客户端的连接请求
	virtual void OnReceive(int nErrorCode); //收到客户端发来的消息
	void OnRegister(Message* rcvMsg);    //处理注册消息
	void SaveUserinfoToFile();
	void OnFindPassword(Message* rcvMsg);
	void UpdateClientList();
	void OnLogin(Message* rcvMsg);
};

typedef struct file_user_info    //存储用户信息的结构体
{
	char username[20];
	char password[40];
	char answer[40];
	char online;
	int userIndex;
	SOCKADDR addr;
	SOCKADDR fileaddr;
}UserInfo;

extern CPtrList UserInfoList;   //维护用户信息的全局链表