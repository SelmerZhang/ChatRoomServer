
// ChatRoomServerDlg.h: 头文件
//

#pragma once
#include "afxwin.h"


// CChatRoomServerDlg 对话框
class CChatRoomServerDlg : public CDialog
{
// 构造
public:
	CChatRoomServerDlg(CWnd* pParent = NULL);	// 标准构造函数
	void DisplayLog(CString& strinfo);
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CHATROOMSERVER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CEdit display_log;
	CListBox onlineUserList;
public:
	void UpdateServerList();
};
