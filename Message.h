#pragma once
/*
�����˷�����Ϣ�ĸ�ʽ�����͵���Ϣ��������Ϣͷ����Ϣ���ݣ�
��Ϣͷ�а�������Ϣ���ͣ������ĸ��û��������ĸ��û�
*/

#define MSG_REGISTER 1   //ע��
#define MSG_LOGIN 2      //��¼
#define MSG_LOGOUT 3     //ע��
#define MSG_UPDATE 4     //�����û��б�
#define MSG_SEND 5	     //������Ϣ
#define MSG_TRANSMIT 6   //���ʹ�������ת������Ϣ
#define MSG_OFFLINE 7    //����������Ϣ
#define MSG_FINDPSW 8    //�һ�����
#define MSG_REGFAIL 9  //ע��ʧ��
#define MSG_REGSUCCESS 10  //ע��ɹ�

#pragma pack(push,1)
typedef struct type_message {
	int type;  //��Ϣ����
	char to_user[20];     //�����Ŀͻ��ˣ���û�У���Ϊ�մ�
	char from_user[20];   //������Ϣ�Ŀͻ���
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
			int if_success;       //0�����¼ʧ�ܣ�1�����¼�ɹ�
		}loginMsg;
		struct findpsw_msg
		{
			char userName[20];
			char password[40];
			char answer[40];
			int if_success;    //�ܱ���֤�ɹ�Ϊ1��ʧ��Ϊ0��Ϊ2ʱ������Ҫ�޸����룬Ϊ3�����޸ĳɹ�
		}findpswMsg;
		struct userinfo_msg
		{
			int count;       //�û�����
			char userinfo[1020];
		}userinfoMsg;
	}data;      //���ݲ��֣����Ƴ���Ϊ1024�ֽ�
}Message;

typedef struct otheruser_info
{
	char username[20];
	char online;
	SOCKADDR addr;
	SOCKADDR fileaddr;
}OtheruserInfo;
#pragma pack(pop)