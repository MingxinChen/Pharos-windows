#ifndef __COMMON_DEFINE_H__
#define __COMMON_DEFINE_H__

#include <Windows.h>
#include "PeerList.h"

//#pragma comment(lib, "WS2_32")	// 链接到WS2_32.lib

#define MAX_TRY_NUMBER		5
#define SERVER_PORT			4096
#define MAX_PACKET_SIZE		1024

#define G_UNREACHABLE	1000

// 各种消息标识宏
#define INVALID_MSG             -1
#define MSG_USERLOGIN           1						// 用户登陆
#define MSG_USERLOGACK          2						// 发送确认用户登陆的信息
#define MSG_GETUSERLIST         3						// 获取用户列表
#define MSG_USERLISTCMP         4						// 用户列表传送完毕
#define MSG_P2PMSG              5						// 发送P2P信息
#define MSG_P2PCONNECT          6						// 有用户请求让另一个用户向它发送打洞消息
#define MSG_P2PMSGACK           7                       // 消息ACK
#define MSG_P2PCONNECTACK       8                       // 打洞消息的ACK
#define MSG_USERLOGOUT          9						// 通知server用户退出
#define MSG_USERACTIVEQUERY     10                      // 查询用户是否还存在
// 加好友
#define MSG_NEWFRIENDASK        11                      // 请求添加好友
#define MSG_NEWFRIENDAGREE      12                      // 同意添加好友
#define MSG_NEWFRIENDREFUSE     13                      // 拒绝添加好友
// 加房间
#define MSG_NEWROOMIDAPPLY		14						// 请求房间id
#define MSG_NEWROOMIDREPLY      15                      // 服务器回复的房间id
#define MSG_NEWROOMASK          16                      // 请求建立房间（host, id）
#define MSG_NEWROOMAGREE        17                      // 同意加入房间（host, id）
#define MSG_NEWROOMREFUSE       18                      // 拒绝加入房间（host, id）
#define MSG_NEWROOMNAMELIST     19                      // 房间名单
#define MSG_CHATSTARTALERT		20						// 开始讨论
#define MSG_LEAVEROOM			21						// 离开房间
#define MSG_ENTERROOM			22						// 进入房间
#define MSG_QUITROOM			23						// 退出房间
// 维护
#define MSG_NODEACTIVEQUE		24
#define MSG_LINKSTATE			25

class MSGDef										// 定义消息的结构体
{
public:
#pragma pack(1)										// 使结构体的数据按照1字节来对齐,省空间

    // 消息头
    struct TMSG_HEADER
    {
        char    cMsgID;								// 消息标识

        TMSG_HEADER(char MsgID = INVALID_MSG)
            : cMsgID(MsgID)
        {
        }
    };

    // 用户登陆
    struct TMSG_USERLOGIN
        : TMSG_HEADER
    {
        Peer_Info PeerInfo;

        TMSG_USERLOGIN(const Peer_Info &rPeerinfo)
            : TMSG_HEADER(MSG_USERLOGIN)
        {
            PeerInfo = rPeerinfo;
        }
    };

    // 发送确认用户登陆的信息
    struct TMSG_USERLOGACK
        : TMSG_HEADER
    {
        Peer_Info PeerInfo;

        TMSG_USERLOGACK(const Peer_Info& rPeerInfo)
            : TMSG_HEADER(MSG_USERLOGACK)
        {
            PeerInfo = rPeerInfo;
        }
    };

    // 获取用户列表
    struct TMSG_GETUSERLIST
        : TMSG_HEADER
    {
        Peer_Info PeerInfo;

        TMSG_GETUSERLIST(const Peer_Info& rPeerInfo)
            : TMSG_HEADER(MSG_GETUSERLIST)
        {
            PeerInfo = rPeerInfo;
        }
    };

    // 更新用户列表完毕
    struct TMSG_USERLISTCMP
        : TMSG_HEADER
    {
        TMSG_USERLISTCMP()
            : TMSG_HEADER(MSG_USERLISTCMP)
        {

        }
    };

    // 一个client向另一个client发送消息
    struct TMSG_P2PMSG
        : TMSG_HEADER
    {
        Peer_Info	PeerInfo;
        char		roomId[NUM_ROOMID];
        char		user[MAX_USERNAME];
        char		szMsg[MAX_PACKET_SIZE - sizeof(TMSG_HEADER) - sizeof(Peer_Info) - sizeof(char[NUM_ROOMID]) - sizeof(char[MAX_USERNAME])];

        TMSG_P2PMSG(const Peer_Info& rPeerInfo)
            : TMSG_HEADER(MSG_P2PMSG)
        {
            PeerInfo = rPeerInfo;
            memset(roomId, '\0', NUM_ROOMID);
            memset(user, '\0', MAX_USERNAME);
            memset(szMsg, 0, MAX_PACKET_SIZE - sizeof(TMSG_HEADER) - sizeof(PeerInfo) - sizeof(roomId) - sizeof(user));
        }
    };

    // 有用户请求让另一个用户向它发送打洞消息
    struct TMSG_P2PCONNECT
        : TMSG_HEADER
    {
        Peer_Info	PeerInfo;
        char		szUserName[MAX_USERNAME];

        TMSG_P2PCONNECT(const Peer_Info& rPeerInfo)
            : TMSG_HEADER(MSG_P2PCONNECT)
        {
            PeerInfo = rPeerInfo;
            memset(szUserName, '\0', MAX_USERNAME);

        }
    };

    // client收到另一个client发送的消息之后的确认
    struct TMSG_P2PMSGACK
        : TMSG_HEADER
    {
        Peer_Info PeerInfo;

        TMSG_P2PMSGACK(const Peer_Info& rPeerInfo)
            : TMSG_HEADER(MSG_P2PMSGACK)
        {
            PeerInfo = rPeerInfo;
        }
    };

    // 接收到节点的打洞消息，在这里设置它的P2P通信地址
    struct TMSG_P2PCONNECTACK
        : TMSG_HEADER
    {
        Peer_Info PeerInfo;

        TMSG_P2PCONNECTACK(const Peer_Info& rPeerInfo)
            : TMSG_HEADER(MSG_P2PCONNECTACK)
        {
            PeerInfo = rPeerInfo;
        }
    };

    // 通知server用户退出
    struct TMSG_USERLOGOUT
        : TMSG_HEADER
    {
        Peer_Info PeerInfo;
        TMSG_USERLOGOUT(const Peer_Info& rPeerInfo)
            : TMSG_HEADER(MSG_USERLOGOUT)
        {
            PeerInfo = rPeerInfo;
        }
    };

    // 查询用户是否还存在
    struct TMSG_USERACTIVEQUERY
        : TMSG_HEADER
    {
        Peer_Info PeerInfo;

        TMSG_USERACTIVEQUERY(const Peer_Info& rPeerInfo = Peer_Info())
            : TMSG_HEADER(MSG_USERACTIVEQUERY)
        {
            PeerInfo = rPeerInfo;
        }
    };

    // 请求添加好友
    struct TMSG_NEWFRIENDASK
        : TMSG_HEADER
    {
        Peer_Info PeerInfo;

        TMSG_NEWFRIENDASK(const Peer_Info& rPeerInfo)
            : TMSG_HEADER(MSG_NEWFRIENDASK)
        {
            PeerInfo = rPeerInfo;
        }
    };

    // 同意添加好友
    struct TMSG_NEWFRIENDAGREE
        : TMSG_HEADER
    {
        Peer_Info PeerInfo;

        TMSG_NEWFRIENDAGREE(const Peer_Info& rPeerInfo)
            : TMSG_HEADER(MSG_NEWFRIENDAGREE)
        {
            PeerInfo = rPeerInfo;
        }
    };

    // 拒绝添加好友
    struct TMSG_NEWFRIENDREFUSE
        : TMSG_HEADER
    {
        Peer_Info PeerInfo;

        TMSG_NEWFRIENDREFUSE(const Peer_Info& rPeerInfo)
            : TMSG_HEADER(MSG_NEWFRIENDREFUSE)
        {
            PeerInfo = rPeerInfo;
        }
    };

    // 请求房间id
    struct TMSG_NEWROOMIDAPPLY
        : TMSG_HEADER
    {
        Peer_Info PeerInfo;

        TMSG_NEWROOMIDAPPLY(const Peer_Info& rPeerInfo)
            : TMSG_HEADER(MSG_NEWROOMIDAPPLY)
        {
            PeerInfo = rPeerInfo;
        }
    };

    // 服务器回复的房间id
    struct TMSG_NEWROOMIDREPLY
        : TMSG_HEADER
    {
        Peer_Info PeerInfo;
        char      roomId[NUM_ROOMID];

        TMSG_NEWROOMIDREPLY(const Peer_Info& rPeerInfo)
            : TMSG_HEADER(MSG_NEWROOMIDREPLY)
        {
            PeerInfo = rPeerInfo;
            memset(roomId, '\0', NUM_ROOMID);
        }
    };

    // 请求建立房间（host, id）
    struct TMSG_NEWROOMASK
        : TMSG_HEADER
    {
        Peer_Info PeerInfo;
        char      roomId[NUM_ROOMID];
        //int       roomSize;
        //char      namelist[MAX_ROOMMATE][MAX_USERNAME];

        TMSG_NEWROOMASK(const Peer_Info& rPeerInfo)
            : TMSG_HEADER(MSG_NEWROOMASK)
        {
            PeerInfo = rPeerInfo;
            memset(roomId, '\0', NUM_ROOMID);
        }
    };

    // 同意加入房间（host, id）
    struct TMSG_NEWROOMAGREE
        : TMSG_HEADER
    {
        Peer_Info PeerInfo;
        char      roomId[NUM_ROOMID];

        TMSG_NEWROOMAGREE(const Peer_Info& rPeerInfo)
            : TMSG_HEADER(MSG_NEWROOMAGREE)
        {
            PeerInfo = rPeerInfo;
            memset(roomId, '\0', NUM_ROOMID);
        }
    };

    // 拒绝加入房间（host）
    struct TMSG_NEWROOMREFUSE
        : TMSG_HEADER
    {
        Peer_Info PeerInfo;
        char      roomId[NUM_ROOMID];

        TMSG_NEWROOMREFUSE(const Peer_Info& rPeerInfo)
            : TMSG_HEADER(MSG_NEWROOMREFUSE)
        {
            PeerInfo = rPeerInfo;
            memset(roomId, '\0', NUM_ROOMID);
        }
    };
    // 房间名单
    struct TMSG_NEWROOMNAMELIST
        : TMSG_HEADER
    {
        Peer_Info PeerInfo;
        char      roomId[NUM_ROOMID];
        int       roomSize;
        char      namelist[MAX_ROOMMATE][MAX_USERNAME];

        TMSG_NEWROOMNAMELIST(const Peer_Info& rPeerInfo)
            : TMSG_HEADER(MSG_NEWROOMNAMELIST)
        {
            PeerInfo = rPeerInfo;
            memset(roomId, '\0', NUM_ROOMID);
            memset(namelist, '\0', MAX_ROOMMATE * MAX_USERNAME);

        }
    };
    // 房间开始讨论
    struct TMSG_CHATSTARTALERT
        : TMSG_HEADER
    {
        Peer_Info PeerInfo;
        char      roomId[NUM_ROOMID];

        TMSG_CHATSTARTALERT(const Peer_Info& rPeerInfo)
            : TMSG_HEADER(MSG_CHATSTARTALERT)
        {
            PeerInfo = rPeerInfo;
            memset(roomId, '\0', NUM_ROOMID);
        }
    };
    // 离开房间
    struct TMSG_LEAVEROOM
        : TMSG_HEADER
    {
        Peer_Info PeerInfo;
        char      roomId[NUM_ROOMID];

        TMSG_LEAVEROOM(const Peer_Info& rPeerInfo)
            : TMSG_HEADER(MSG_LEAVEROOM)
        {
            PeerInfo = rPeerInfo;
            memset(roomId, '\0', NUM_ROOMID);
        }
    };
    // 进入房间
    struct TMSG_ENTERROOM
        : TMSG_HEADER
    {
        Peer_Info PeerInfo;
        char      roomId[NUM_ROOMID];

        TMSG_ENTERROOM(const Peer_Info& rPeerInfo)
            : TMSG_HEADER(MSG_ENTERROOM)
        {
            PeerInfo = rPeerInfo;
            memset(roomId, '\0', NUM_ROOMID);
        }
    };
    // 退群
    struct TMSG_QUITROOM
        : TMSG_HEADER
    {
        Peer_Info PeerInfo;
        char      roomId[NUM_ROOMID];

        TMSG_QUITROOM(const Peer_Info& rPeerInfo)
            : TMSG_HEADER(MSG_QUITROOM)
        {
            PeerInfo = rPeerInfo;
            memset(roomId, '\0', NUM_ROOMID);
        }
    };
    // 节点存活确认
    struct TMSG_NODEACTIVEQUE
        : TMSG_HEADER
    {
        Peer_Info PeerInfo;
        char	  roomId[NUM_ROOMID];
        char	  type; // o代表日常询问父节点，s代表重选过程的询问

        TMSG_NODEACTIVEQUE(const Peer_Info& rPeerInfo)
            : TMSG_HEADER(MSG_NODEACTIVEQUE)
        {
            PeerInfo = rPeerInfo;
        }
    };
    // 链路状态
    struct TMSG_LINKSTATE
        :TMSG_HEADER
    {
        Peer_Info PeerInfo;
        char			roomId[NUM_ROOMID];
        int				edge[MAX_ROOMMATE][MAX_ROOMMATE];

        TMSG_LINKSTATE(const Peer_Info& rPeerInfo)
            : TMSG_HEADER(MSG_LINKSTATE)
        {
            PeerInfo = rPeerInfo;
            for (int i = 0; i < MAX_ROOMMATE; i++) {
                for (int j = 0; j < MAX_ROOMMATE; j++) {
                    edge[i][j] = G_UNREACHABLE;
                }
            }
        }
    };
#pragma pack()
};

#endif // __COMMON_DEFINE_H__
