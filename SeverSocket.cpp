// ServerSocket.cpp : implementation file
//

#include "stdafx.h"
#include "ChatRoomServer.h"
#include "ServerSocket.h"
#include "ChatRoomServer.h"
#include "ChatRoomServerDlg.h"
#include "Message.h"
#include <fstream>
using namespace std;

// ServerSocket

ServerSocket::ServerSocket()
	:m_UserName(_T(""))
{
	
}

ServerSocket::~ServerSocket()
{

}


void ServerSocket::OnAccept(int nErrorCode)
{
	//�յ���һ����������
	ServerSocket* toClientSocket;
	toClientSocket = new ServerSocket();
	toClientSocket->m_ptoList = &this->m_pList;
	Accept(*toClientSocket);
	m_pList.AddTail(toClientSocket);

	CSocket::OnAccept(nErrorCode);
}

//���տͻ��˷�������Ϣ
void ServerSocket::OnReceive(int nErrorCode)
{
	char *pMsg = new char[sizeof(Message)];  //�����Messageͬ�ȴ�С���ַ����ռ䲢��pMsgָ����
	if (!pMsg)
	{
		TRACE0("ServerSocket::OnReceive �ڴ治�㣡");
		return;
	}
	memset(pMsg, 0, sizeof(Message));
	Receive(pMsg, sizeof(Message));    //������Ϣ��pMsg��
	Message* rcvMsg = (Message*)pMsg;
	switch (rcvMsg->type)	//�Բ�ͬ���͵���Ϣ����ͬ�Ĵ���
	{
		case MSG_REGISTER:   //ע����Ϣ
		{
			this->OnRegister(rcvMsg);
			break;
		}
		case MSG_FINDPSW:   //�һ�������Ϣ
		{
			this->OnFindPassword(rcvMsg);
			break;
		}
		case MSG_LOGIN:
		{
			this->OnLogin(rcvMsg);        //�����¼��Ϣ
			((CChatRoomServerDlg*)theApp.m_pMainWnd)->UpdateServerList();      //�����û��б�
			break;
		}
		case MSG_OFFLINE:
		{
			Message* offlinemsg = new Message;
			*offlinemsg = *rcvMsg;
			theApp.offlineMessage.AddTail(offlinemsg);
			break;
		}

		default: break;
	}
	delete pMsg;
	CSocket::OnReceive(nErrorCode);
}


void ServerSocket::SaveUserinfoToFile()    //�����û���Ϣ���ļ���
{
	fstream fp("./UserInfo.txt", ios::out | ios::binary);
	if (!fp)
	{
		AfxMessageBox(_T("���û���Ϣ�ļ�ʧ�ܣ�"));
		return;
	}
	POSITION ps = UserInfoList.GetHeadPosition();
	while (ps != NULL)
	{
		UserInfo* pTemp = (UserInfo*)UserInfoList.GetNext(ps);
		fp.write((char*)pTemp, sizeof(UserInfo));
	}
	fp.close();
}


void ServerSocket::OnRegister(Message* rcvMsg)
{
	Message* sendMsg = new Message;
	memset(sendMsg, 0, sizeof(Message));
	POSITION ps = UserInfoList.GetHeadPosition();
	while (ps != NULL)
	{
		UserInfo* pTemp = (UserInfo*)UserInfoList.GetNext(ps);
		if ((CString)(pTemp->username) == rcvMsg->data.regMsg.userName)  //�ж��û����Ƿ��Ѵ���
		{
			//���û����Ѵ���
			sendMsg->type = MSG_REGFAIL;
			Send(sendMsg, sizeof(Message));     //����ע��ʧ�ܵ���Ϣ
			delete sendMsg;
			return;
		}
	}
	//���û���������
	UserInfo* newUser = new UserInfo;
	memset(newUser, 0, sizeof(UserInfo));
	strcpy_s(newUser->username, 20, rcvMsg->data.regMsg.userName);
	strcpy_s(newUser->password, 40, rcvMsg->data.regMsg.password);
	strcpy_s(newUser->answer, 40, rcvMsg->data.regMsg.answer);
	newUser->userIndex = UserInfoList.GetCount();
	UserInfoList.AddTail(newUser);       //ע��ɹ����������û���Ϣ���û���Ϣ
	this->SaveUserinfoToFile();
	sendMsg->type = MSG_REGSUCCESS;
	Send(sendMsg, sizeof(Message));     //����ע��ɹ�����Ϣ
	delete sendMsg;
	CTime time;
	time = CTime::GetCurrentTime();
	CString strTime = time.Format("%Y-%m-%d  %H:%M:%S  ");
	strTime = strTime + _T("  �û���Ϊ ") + newUser->username + _T(" �����û�ע����...\r\n");
	((CChatRoomServerDlg*)theApp.GetMainWnd())->DisplayLog(strTime);
	return;
}


void ServerSocket::OnFindPassword(Message* rcvMsg)   //�����һ�������Ϣ
{
	Message* sendMsg = new Message;
	memset(sendMsg, 0, sizeof(Message));
	POSITION ps = UserInfoList.GetHeadPosition();
	while (ps != NULL)
	{
		UserInfo* pTemp = (UserInfo*)UserInfoList.GetNext(ps);
		if ((CString)(pTemp->username) == rcvMsg->data.regMsg.userName)  //�ҵ���Ӧ�û���Ϣ
		{
			if (rcvMsg->data.findpswMsg.if_success == 2)   //�����޸ĳɹ�����Ϣ
			{
				strcpy_s(pTemp->password, 40, rcvMsg->data.findpswMsg.password);
				sendMsg->type = MSG_FINDPSW;
				sendMsg->data.findpswMsg.if_success = 3;
				Send(sendMsg, sizeof(Message));
				delete sendMsg;
				CTime time;
				time = CTime::GetCurrentTime();
				CString strTime = time.Format("%Y-%m-%d  %H:%M:%S  ");
				strTime = strTime + rcvMsg->data.findpswMsg.userName + _T("  �û��޸�������...\r\n");
				((CChatRoomServerDlg*)theApp.GetMainWnd())->DisplayLog(strTime);
				this->SaveUserinfoToFile();
				return;
			}
			else if ((CString)(pTemp->answer) == rcvMsg->data.regMsg.answer)
			{
				if (rcvMsg->data.findpswMsg.if_success == 0)   //������ʾ�޸��������Ϣ
				{
					sendMsg->type = MSG_FINDPSW;
					strcpy_s(sendMsg->data.findpswMsg.userName, 20, rcvMsg->data.findpswMsg.userName);
					sendMsg->data.findpswMsg.if_success = 1;
					Send(sendMsg, sizeof(Message));
					delete sendMsg;
					return;
				}
			}
			else
			{
				sendMsg->type = MSG_FINDPSW;         //�ܱ��𰸲���ȷ
				sendMsg->data.findpswMsg.if_success = 0;
				Send(sendMsg, sizeof(Message));
				delete sendMsg;
				return;
			}
		}
	}
	//�û���Ϣ�б��в����ڸ��û���
	sendMsg->type = MSG_FINDPSW;
	sendMsg->data.findpswMsg.if_success = 0;
	Send(sendMsg, sizeof(Message));
	delete sendMsg;
	return;
}


void ServerSocket::UpdateClientList()     //���¿ͻ����û��б�
{
	Message* sendMsg = new Message;
	memset(sendMsg, 0, sizeof(Message));
	int userNum = UserInfoList.GetCount();
	int n = 0;
	OtheruserInfo* userInfo = new OtheruserInfo[userNum];   //������¼�����û���Ϣ�����飬�����û������Ƿ����ߣ��Է��ļ���IP�Ͷ˿ں�
	POSITION ps = UserInfoList.GetHeadPosition();
	while (ps != NULL)
	{
		UserInfo* pTemp = (UserInfo*)UserInfoList.GetNext(ps);
		strcpy_s(userInfo[n].username, 20, pTemp->username);
		userInfo[n].online = pTemp->online;
		userInfo[n].addr = pTemp->addr;
		userInfo[n].fileaddr = pTemp->fileaddr;
		n++;
	}
	sendMsg->type = MSG_UPDATE;
	sendMsg->data.userinfoMsg.count = n;
	memcpy_s(sendMsg->data.userinfoMsg.userinfo, sizeof(OtheruserInfo)*n, userInfo, sizeof(OtheruserInfo)*n);
	ServerSocket* pSocket;
	ps = m_ptoList->GetHeadPosition();
	while (ps != NULL)
	{
		pSocket = (ServerSocket*)UserInfoList.GetNext(ps);
		pSocket->Send(sendMsg, sizeof(Message));
	}
	delete userInfo;
	userInfo = 0;
	delete sendMsg;
	sendMsg = 0;
}


//�û���¼
void ServerSocket::OnLogin(Message* rcvMsg)
{
	Message* sendMsg = new Message;
	memset(sendMsg, 0, sizeof(Message));
	sendMsg->type = MSG_LOGIN;
	POSITION ps = UserInfoList.GetHeadPosition();
	while (ps != NULL)
	{
		UserInfo* pTemp = (UserInfo*)UserInfoList.GetNext(ps);
		if ((CString)(pTemp->username) == rcvMsg->data.regMsg.userName)  //�ҵ���Ӧ�û���Ϣ
		{
			if ((CString)(pTemp->password) == rcvMsg->data.regMsg.password)   //������ȷ����¼�ɹ�
			{
				pTemp->online = 1;									//�����û���Ϣ�ĵ�¼״̬
				pTemp->addr = rcvMsg->data.loginMsg.addr;			//�����û���Ϣ�ļ�����ַ
				pTemp->fileaddr = rcvMsg->data.loginMsg.fileaddr;
				sendMsg->data.loginMsg.if_success = 1;
				strcpy_s(sendMsg->data.loginMsg.userName, 20, rcvMsg->data.regMsg.userName);
				this->Send(sendMsg, sizeof(Message));     //��ͻ��˷��͵�¼�ɹ���Ϣ
				UpdateClientList();        //��ͻ��˷����û����������Ϣ
				this->m_UserName = rcvMsg->data.regMsg.userName;
				CTime time;				   //�ڷ���������ʾ�û���¼��Ϣ
				time = CTime::GetCurrentTime();
				CString strTime = time.Format("%Y-%m-%d  %H:%M:%S  ");
				strTime = strTime + this->m_UserName + _T("  �û���¼...\r\n");

				((CChatRoomServerDlg*)theApp.GetMainWnd())->DisplayLog(strTime);
				((CChatRoomServerDlg*)theApp.m_pMainWnd)->UpdateServerList();   //���·����������û��б�


				POSITION ps = theApp.offlineMessage.GetHeadPosition();     //����Ƿ����������û���������Ϣ
				POSITION pstemp;
				Message* pTemp;
				while (ps != NULL)
				{
					pstemp = ps;
					pTemp = (Message*)theApp.offlineMessage.GetNext(ps);
					if ((CString)rcvMsg->data.loginMsg.userName == pTemp->to_user)
					{
						Send(pTemp, sizeof(Message));
						theApp.offlineMessage.RemoveAt(pstemp);
					}
				}
				return;
			}
			else
			{
				sendMsg->data.loginMsg.if_success = 0;
				this->Send(sendMsg, sizeof(Message));   //������󣬵�¼ʧ��
				return;
			}
		}
	}
	sendMsg->data.loginMsg.if_success = 0;
	this->Send(sendMsg, sizeof(Message));   //�û��������ڣ���¼ʧ��
	return;
}

//�û�����
void ServerSocket::OnClose(int nErrorCode)
{
	CTime time;
	time = CTime::GetCurrentTime();
	CString strTime = time.Format("%Y-%m-%d  %H:%M:%S  ");
	strTime = strTime + this->m_UserName + _T("  �û��뿪...\r\n");
	((CChatRoomServerDlg*)theApp.GetMainWnd())->DisplayLog(strTime);
	POSITION pss = m_ptoList->Find(this);
	m_ptoList->RemoveAt(pss);
	POSITION ps = UserInfoList.GetHeadPosition();
	while (ps != NULL)
	{
		UserInfo* pTemp = (UserInfo*)UserInfoList.GetNext(ps);
		if ((CString)(pTemp->username) == this->m_UserName)
		{
			pTemp->online = 0;           //�û���Ϣ�б��и��û�����Ϊ������
		}
	}
	//֪ͨ�ͻ���ˢ����������
	this->UpdateClientList();
	((CChatRoomServerDlg*)theApp.m_pMainWnd)->UpdateServerList();
	this->Close();
	//���ٸ��׽���
	delete this;
	CSocket::OnClose(nErrorCode);
}


