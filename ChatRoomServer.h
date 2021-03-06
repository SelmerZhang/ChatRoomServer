
// ChatRoomServer.h: PROJECT_NAME 应用程序的主头文件
//

#pragma once

#ifndef __AFXWIN_H__
	#error "在包含此文件之前包含“stdafx.h”以生成 PCH 文件"
#endif

#include "resource.h"		// 主符号
#include "ServerSocket.h"

// CChatRoomServerApp:
// 有关此类的实现，请参阅 ChatRoomServer.cpp
//

class CChatRoomServerApp : public CWinApp
{
public:
	CChatRoomServerApp();

// 重写
public:
	virtual BOOL InitInstance();
// 实现
	DECLARE_MESSAGE_MAP()
	ServerSocket* listenSocket;   //监听socket，之后的socket都保存在监听socket的成员链表中
	ServerSocket* GetServerSocket() const;
	void LoadUserinfoFromFile();
public:	
	CPtrList offlineMessage;
};

extern CChatRoomServerApp theApp;
