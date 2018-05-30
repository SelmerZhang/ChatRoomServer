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
	//收到到一个连接请求
	ServerSocket* toClientSocket;
	toClientSocket = new ServerSocket();
	toClientSocket->m_ptoList = &this->m_pList;
	Accept(*toClientSocket);
	m_pList.AddTail(toClientSocket);

	CSocket::OnAccept(nErrorCode);
}

//接收客户端发来的消息
void ServerSocket::OnReceive(int nErrorCode)
{
	char *pMsg = new char[sizeof(Message)];  //申请和Message同等大小的字符串空间并用pMsg指向它
	if (!pMsg)
	{
		TRACE0("ServerSocket::OnReceive 内存不足！");
		return;
	}
	memset(pMsg, 0, sizeof(Message));
	Receive(pMsg, sizeof(Message));    //接收消息到pMsg中
	Message* rcvMsg = (Message*)pMsg;
	switch (rcvMsg->type)	//对不同类型的消息做不同的处理
	{
		case MSG_REGISTER:   //注册消息
		{
			this->OnRegister(rcvMsg);
			break;
		}
		case MSG_FINDPSW:   //找回密码消息
		{
			this->OnFindPassword(rcvMsg);
			break;
		}
		case MSG_LOGIN:
		{
			this->OnLogin(rcvMsg);        //处理登录消息
			((CChatRoomServerDlg*)theApp.m_pMainWnd)->UpdateServerList();      //更新用户列表
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


void ServerSocket::SaveUserinfoToFile()    //保存用户信息到文件中
{
	fstream fp("./UserInfo.txt", ios::out | ios::binary);
	if (!fp)
	{
		AfxMessageBox(_T("打开用户信息文件失败！"));
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
		if ((CString)(pTemp->username) == rcvMsg->data.regMsg.userName)  //判断用户名是否已存在
		{
			//若用户名已存在
			sendMsg->type = MSG_REGFAIL;
			Send(sendMsg, sizeof(Message));     //发送注册失败的信息
			delete sendMsg;
			return;
		}
	}
	//若用户名不存在
	UserInfo* newUser = new UserInfo;
	memset(newUser, 0, sizeof(UserInfo));
	strcpy_s(newUser->username, 20, rcvMsg->data.regMsg.userName);
	strcpy_s(newUser->password, 40, rcvMsg->data.regMsg.password);
	strcpy_s(newUser->answer, 40, rcvMsg->data.regMsg.answer);
	newUser->userIndex = UserInfoList.GetCount();
	UserInfoList.AddTail(newUser);       //注册成功，加入新用户信息到用户信息
	this->SaveUserinfoToFile();
	sendMsg->type = MSG_REGSUCCESS;
	Send(sendMsg, sizeof(Message));     //发送注册成功的信息
	delete sendMsg;
	CTime time;
	time = CTime::GetCurrentTime();
	CString strTime = time.Format("%Y-%m-%d  %H:%M:%S  ");
	strTime = strTime + _T("  用户名为 ") + newUser->username + _T(" 的新用户注册了...\r\n");
	((CChatRoomServerDlg*)theApp.GetMainWnd())->DisplayLog(strTime);
	return;
}


void ServerSocket::OnFindPassword(Message* rcvMsg)   //处理找回密码消息
{
	Message* sendMsg = new Message;
	memset(sendMsg, 0, sizeof(Message));
	POSITION ps = UserInfoList.GetHeadPosition();
	while (ps != NULL)
	{
		UserInfo* pTemp = (UserInfo*)UserInfoList.GetNext(ps);
		if ((CString)(pTemp->username) == rcvMsg->data.regMsg.userName)  //找到对应用户信息
		{
			if (rcvMsg->data.findpswMsg.if_success == 2)   //返回修改成功的消息
			{
				strcpy_s(pTemp->password, 40, rcvMsg->data.findpswMsg.password);
				sendMsg->type = MSG_FINDPSW;
				sendMsg->data.findpswMsg.if_success = 3;
				Send(sendMsg, sizeof(Message));
				delete sendMsg;
				CTime time;
				time = CTime::GetCurrentTime();
				CString strTime = time.Format("%Y-%m-%d  %H:%M:%S  ");
				strTime = strTime + rcvMsg->data.findpswMsg.userName + _T("  用户修改了密码...\r\n");
				((CChatRoomServerDlg*)theApp.GetMainWnd())->DisplayLog(strTime);
				this->SaveUserinfoToFile();
				return;
			}
			else if ((CString)(pTemp->answer) == rcvMsg->data.regMsg.answer)
			{
				if (rcvMsg->data.findpswMsg.if_success == 0)   //返回提示修改密码的消息
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
				sendMsg->type = MSG_FINDPSW;         //密保答案不正确
				sendMsg->data.findpswMsg.if_success = 0;
				Send(sendMsg, sizeof(Message));
				delete sendMsg;
				return;
			}
		}
	}
	//用户信息列表中不存在该用户名
	sendMsg->type = MSG_FINDPSW;
	sendMsg->data.findpswMsg.if_success = 0;
	Send(sendMsg, sizeof(Message));
	delete sendMsg;
	return;
}


void ServerSocket::UpdateClientList()     //更新客户端用户列表
{
	Message* sendMsg = new Message;
	memset(sendMsg, 0, sizeof(Message));
	int userNum = UserInfoList.GetCount();
	int n = 0;
	OtheruserInfo* userInfo = new OtheruserInfo[userNum];   //创建记录所有用户信息的数组，包括用户名，是否在线，对方的监听IP和端口号
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


//用户登录
void ServerSocket::OnLogin(Message* rcvMsg)
{
	Message* sendMsg = new Message;
	memset(sendMsg, 0, sizeof(Message));
	sendMsg->type = MSG_LOGIN;
	POSITION ps = UserInfoList.GetHeadPosition();
	while (ps != NULL)
	{
		UserInfo* pTemp = (UserInfo*)UserInfoList.GetNext(ps);
		if ((CString)(pTemp->username) == rcvMsg->data.regMsg.userName)  //找到对应用户信息
		{
			if ((CString)(pTemp->password) == rcvMsg->data.regMsg.password)   //密码正确，登录成功
			{
				pTemp->online = 1;									//更新用户信息的登录状态
				pTemp->addr = rcvMsg->data.loginMsg.addr;			//更新用户信息的监听地址
				pTemp->fileaddr = rcvMsg->data.loginMsg.fileaddr;
				sendMsg->data.loginMsg.if_success = 1;
				strcpy_s(sendMsg->data.loginMsg.userName, 20, rcvMsg->data.regMsg.userName);
				this->Send(sendMsg, sizeof(Message));     //向客户端发送登录成功信息
				UpdateClientList();        //向客户端发送用户在线情况信息
				this->m_UserName = rcvMsg->data.regMsg.userName;
				CTime time;				   //在服务器上显示用户登录信息
				time = CTime::GetCurrentTime();
				CString strTime = time.Format("%Y-%m-%d  %H:%M:%S  ");
				strTime = strTime + this->m_UserName + _T("  用户登录...\r\n");

				((CChatRoomServerDlg*)theApp.GetMainWnd())->DisplayLog(strTime);
				((CChatRoomServerDlg*)theApp.m_pMainWnd)->UpdateServerList();   //更新服务器在线用户列表


				POSITION ps = theApp.offlineMessage.GetHeadPosition();     //检查是否有送往该用户的离线消息
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
				this->Send(sendMsg, sizeof(Message));   //密码错误，登录失败
				return;
			}
		}
	}
	sendMsg->data.loginMsg.if_success = 0;
	this->Send(sendMsg, sizeof(Message));   //用户名不存在，登录失败
	return;
}

//用户下线
void ServerSocket::OnClose(int nErrorCode)
{
	CTime time;
	time = CTime::GetCurrentTime();
	CString strTime = time.Format("%Y-%m-%d  %H:%M:%S  ");
	strTime = strTime + this->m_UserName + _T("  用户离开...\r\n");
	((CChatRoomServerDlg*)theApp.GetMainWnd())->DisplayLog(strTime);
	POSITION pss = m_ptoList->Find(this);
	m_ptoList->RemoveAt(pss);
	POSITION ps = UserInfoList.GetHeadPosition();
	while (ps != NULL)
	{
		UserInfo* pTemp = (UserInfo*)UserInfoList.GetNext(ps);
		if ((CString)(pTemp->username) == this->m_UserName)
		{
			pTemp->online = 0;           //用户信息列表中该用户更新为不在线
		}
	}
	//通知客户端刷新在线名单
	this->UpdateClientList();
	((CChatRoomServerDlg*)theApp.m_pMainWnd)->UpdateServerList();
	this->Close();
	//销毁该套接字
	delete this;
	CSocket::OnClose(nErrorCode);
}


