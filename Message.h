#pragma once
/*
定义了发送消息的格式，发送的消息包括了消息头和消息内容，
消息头中包含了消息类型，来自哪个用户，发往哪个用户
*/

#define MSG_REGISTER 1   //注册
#define MSG_LOGIN 2      //登录
#define MSG_LOGOUT 3     //注销
#define MSG_UPDATE 4     //更新用户列表
#define MSG_SEND 5	     //发送消息
#define MSG_TRANSMIT 6   //发送待服务器转发的消息
#define MSG_OFFLINE 7    //发送离线消息
#define MSG_FINDPSW 8    //找回密码
#define MSG_REGFAIL 9  //注册失败
#define MSG_REGSUCCESS 10  //注册成功

#pragma pack(push,1)
typedef struct type_message {
	int type;  //消息类型
	char to_user[20];     //送往的客户端，若没有，则为空串
	char from_user[20];   //发送消息的客户端
	union send_data
	{
		char chatMsg[1024];
		struct reg_msg
		{
			char userName[20];
			char password[40];
			char answer[40];
		}regMsg;
		struct login_msg
		{
			char userName[20];
			char password[40];
			SOCKADDR addr;
			SOCKADDR fileaddr;
			int if_success;       //0代表登录失败，1代表登录成功
		}loginMsg;
		struct findpsw_msg
		{
			char userName[20];
			char password[40];
			char answer[40];
			int if_success;    //密保验证成功为1，失败为0，为2时，代表要修改密码，为3代表修改成功
		}findpswMsg;
		struct userinfo_msg
		{
			int count;       //用户数量
			char userinfo[1020];
		}userinfoMsg;
	}data;      //数据部分，限制长度为1024字节
}Message;

typedef struct otheruser_info
{
	char username[20];
	char online;
	SOCKADDR addr;
	SOCKADDR fileaddr;
}OtheruserInfo;
#pragma pack(pop)