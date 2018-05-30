#pragma once

#include "stdafx.h"
#include "Message.h"
/*
  �������˵�socket�����ڼ������Ӻͽ��ա�������Ϣ
*/

class ServerSocket : public CSocket
{
public:
	ServerSocket();
	virtual ~ServerSocket();
public:
	CPtrList m_pList;//���������������socket��������ͷΪ����socket
	CPtrList *m_ptoList;
	CString m_UserName; //���ӵĿͻ��˵��û���
public:
	virtual void OnClose(int nErrorCode);
	virtual void OnAccept(int nErrorCode); //���ܿͻ��˵���������
	virtual void OnReceive(int nErrorCode); //�յ��ͻ��˷�������Ϣ
	void OnRegister(Message* rcvMsg);    //����ע����Ϣ
	void SaveUserinfoToFile();
	void OnFindPassword(Message* rcvMsg);
	void UpdateClientList();
	void OnLogin(Message* rcvMsg);
};

typedef struct file_user_info    //�洢�û���Ϣ�Ľṹ��
{
	char username[20];
	char password[40];
	char answer[40];
	char online;
	int userIndex;
	SOCKADDR addr;
	SOCKADDR fileaddr;
}UserInfo;

extern CPtrList UserInfoList;   //ά���û���Ϣ��ȫ������