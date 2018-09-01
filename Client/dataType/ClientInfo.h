#ifndef __P2P_CLIENT_H__
#define __P2P_CLIENT_H__

#include <WinSock2.h>
#include <list>
#include <QObject>
#include "CommonDefine.h"
#include "ChatInfo.h"
#include "FriendInfo.h"
#include "function/system.h"
using namespace std;

class ClientInfo : public QObject
{
    Q_OBJECT

public:
    // 初始化
    ClientInfo();
    ~ClientInfo();
    bool Initialize();
    static DWORD WINAPI RecvThreadProc(LPVOID lpParam);
    bool ProcCommand(string command);

    // 打洞相关
    bool UDP_breakHold(char* pszUserName, Peer_Info* pPeerInfo);
    bool ProcP2PConnectMsg(MSGDef::TMSG_HEADER *pMsgHeader, const sockaddr_in& sockAddr);
    bool ProcP2PConnectAckMsg(MSGDef::TMSG_HEADER *pMsgHeader, const sockaddr_in& sockAddr);

    // 登陆
    bool Login(char *pszUserName, char *pszServerIP);
    bool Logout();
    bool ProcUserLogAckMsg(MSGDef::TMSG_HEADER *pMsgHeader);
    bool ProcUserActiveQueryMsg(MSGDef::TMSG_HEADER *pMsgHeader, const sockaddr_in& sockAddr);

    // 得到peerInfo
    bool GetUserList();
    bool ProcGetUserList(MSGDef::TMSG_HEADER *pMsgHeader);
    bool ProcUserListCmpMsg();

    // 添加好友
    bool SendNewFriendAsk(char *pszUserName);
    bool SendNewFriendReply(bool answer, char *pszUserName);
    bool ProNewFriendAsk(MSGDef::TMSG_HEADER *pMsgHeader, const sockaddr_in& sockAddr);
    bool ProNewFriendReply(bool answer, MSGDef::TMSG_HEADER *pMsgHeader, const sockaddr_in& sockAddr);

    // 添加房间
    bool SetUpARoom(char szRoommate[MAX_ROOMMATE][MAX_USERNAME], int roomSize);
    char* SendNewRoomIdApply();
    bool SendNewRoomAsk(char *pszUserName, char *roomId);
    bool SendNewRoomReply(bool answer, char *pszUserName, char *roomId);
    bool SetNameList(char* roomId);
    bool SendNameList(char *pszUserName, MSGDef::TMSG_NEWROOMNAMELIST *tNameList);
    bool SetChatStart(char* roomId);
    bool SendChatStart(char *pszUserName, MSGDef::TMSG_CHATSTARTALERT *tChatStart);
    bool ProNewRoomIdReply(MSGDef::TMSG_HEADER *pMsgHeader, const sockaddr_in& sockAddr);
    bool ProNewRoomAsk(MSGDef::TMSG_HEADER *pMsgHeader, const sockaddr_in& sockAddr);
    bool ProNewRoomReply(bool answer, MSGDef::TMSG_HEADER *pMsgHeader, const sockaddr_in& sockAddr);
    bool ProNamelist(MSGDef::TMSG_HEADER *pMsgHeader, const sockaddr_in& sockAddr);
    bool ProStartChat(MSGDef::TMSG_HEADER *pMsgHeader, const sockaddr_in& sockAddr);

    // 发送消息
    bool SetSendText(char *roomid, char *pszText, int nTextLen);
    bool SendText(char *pszUserName, MSGDef::TMSG_P2PMSG *msg);
    bool ProcP2PMsg(MSGDef::TMSG_HEADER *pMsgHeader, const sockaddr_in& sockAddr);

    // 管理房间
    static DWORD WINAPI ManageRoom_Parent(LPVOID lpParam);
    static DWORD WINAPI ManageRoom_Child(LPVOID lpParam);
    bool ManageSelection(Chat_Info* chat);
    bool SendNodeQue(char *pszUserName, char *roomId, char type);
    bool ProActiveQue(MSGDef::TMSG_HEADER *pMsgHeader, const sockaddr_in& sockAddr);
    bool SendLinkState(char *pszUserName, char *roomId);
    bool ProLinkState(MSGDef::TMSG_HEADER *pMsgHeader, const sockaddr_in& sockAddr);

    // temp
    bool SendRoomGuestOperation(char* roomId, string op);
    bool ProGuestOperation(string op, MSGDef::TMSG_HEADER *pMsgHeader, const sockaddr_in& sockAddr);

    map<string, Chat_Info*>     m_ChatList;
    map<string, Friend_Info*>   m_FriendList;

    string                      headerPath = "./images/headers/10.png";
    string                      password = "123456";

    Peer_Info                   m_PeerInfo;				// 本机信息
    PeerList                    m_PeerList;				// 与本机相连的节点链表
    SOCKET                      m_sSocket;				// 套接字
    HANDLE                      m_hThread;				// 线程句柄
    DWORD                       m_dwServerIP;			// server IP地址
    WSAOVERLAPPED               m_ol;					// 用于等待网络事件的重叠结构
    CRITICAL_SECTION            m_PeerListLock;			// 用于读取m_PeerList的临界区对象
    bool                        m_bExitThread;			// 是否退出线程
    bool                        m_bLogin;				// 是否已经登陆服务器了
    bool                        m_bUserListCmp;			// 是否得到了用户列表
    bool                        m_bMessageACK;			// 是否接收到消息应答

    bool                        m_bParent_roomIdReady;
    char                        m_bParent_roomId[NUM_ROOMID];

signals:
    void messageAlert(Chat_Info* room, char* username, char* message);   // 获得新消息，告知mainwindow
    void informAlert(string type, string message);                    // 获得新通知，告知mainwindwo
    void NewFriendAlert(Friend_Info* newFriend);              // 告知界面
    void NewRoomAlert(Chat_Info* newRoom);              // 告知界面
    void refreshStatus();       // 刷新用户和房间addresslist状态
};

#endif // __P2P_CLIENT_H__
