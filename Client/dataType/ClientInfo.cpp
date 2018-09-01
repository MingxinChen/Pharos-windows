#include <assert.h>
#include <stdio.h>
#include <set>
#include "ClientInfo.h"

extern ClientInfo* myClient;

// ******初始化******===============================================================================================================================
ClientInfo::ClientInfo()
{
    m_sSocket = INVALID_SOCKET;
    m_bExitThread = false;
    m_bLogin = false;
    m_bUserListCmp = false;
    m_bMessageACK = false;
    Initialize();
}

ClientInfo::~ClientInfo()
{
    Logout();

    // 通知接收线程退出
    if(m_hThread != NULL)
    {
        m_bExitThread = TRUE;
        ::WSASetEvent(m_ol.hEvent);
        ::WaitForSingleObject(m_hThread, 300);
        ::CloseHandle(m_hThread);
    }

    if(INVALID_SOCKET != m_sSocket)
    {
        ::closesocket(m_sSocket);
    }

    ::WSACloseEvent(m_ol.hEvent);

    ::DeleteCriticalSection(&m_PeerListLock);
    ::WSACleanup();
}

bool ClientInfo::Initialize()
{
    if (INVALID_SOCKET != m_sSocket)
    {
        myPrintf("Error: Socket Already Been Initialized!\n");
        return false;
    }

    // 初始化WS2_32.dl
    WSADATA wsaData;
    WORD sockVersion = MAKEWORD(2, 2);
    if(::WSAStartup(sockVersion, &wsaData) != 0)
    {
        myPrintf("Error: Initialize WS2_32.dll Failed!\n");
        exit(-1);
    }

    // 初始化锁
    ::InitializeCriticalSection(&m_PeerListLock);

    memset(&m_ol, 0, sizeof(m_ol));
    m_ol.hEvent = ::WSACreateEvent();

    // 创建一个重叠I/O socket
    m_sSocket = ::WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (INVALID_SOCKET == m_sSocket)
    {
        myPrintf("Error: Initialize Socket Failed!\n");
        return false;
    }

    // 绑定socket
    sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(0);
    addr.sin_addr.S_un.S_addr = INADDR_ANY;
    int nAddrLen = sizeof(addr);
    if (SOCKET_ERROR == ::bind(m_sSocket, (sockaddr*)(&addr), nAddrLen))
    {
        myPrintf("Error: Bind Socket Failed!\n");
        ::closesocket(m_sSocket);
        return false;
    }

    // 得到有效的端口号
    ::getsockname(m_sSocket, (sockaddr*)(&addr), &nAddrLen);
    unsigned short usPort = ntohs(addr.sin_port);

    // 得到本机的IP地址
    char szHost[256];
    ::gethostname(szHost, 256);
    hostent* pHost = ::gethostbyname(szHost);

    // 得到本机所有适配器的IP地址和端口号,这些就是私有地址/端口号
    char *pIP;
    for (int i = 0; i < MAX_ADDNUM - 1; ++i) {
        if (NULL == (pIP = pHost->h_addr_list[i])) {
            break;
        }

        memcpy(&(m_PeerInfo.IPAddr[i].dwIP), pIP, pHost->h_length);
        m_PeerInfo.IPAddr[i].usPort = usPort;
        ++m_PeerInfo.nAddrNum;
    }

    // 创建接收进程
    if (NULL == (::CreateThread(NULL, 0, RecvThreadProc, this, 0, NULL)))
    {
        myPrintf("Error: Create RecvThreadProc Failed!\n");
        return false;
    }

    return true;
}

DWORD WINAPI ClientInfo::RecvThreadProc(LPVOID lpParam)
{
    ClientInfo* pThis = (ClientInfo*)lpParam;
    char szBuff[MAX_PACKET_SIZE];
    int nRet;
    sockaddr_in remoteAddr = {0};
    int nAddrLen = sizeof(sockaddr_in);
    WSABUF wsaBuff;
    wsaBuff.buf = szBuff;
    wsaBuff.len = MAX_PACKET_SIZE;
    DWORD dwRecv, dwFlags = 0;
    MSGDef::TMSG_HEADER *pMsgHeader;

    while (true)
    {
        nRet = ::WSARecvFrom(pThis->m_sSocket, &wsaBuff, 1, &dwRecv, &dwFlags,
                            (sockaddr*)(&remoteAddr), &nAddrLen, &pThis->m_ol, NULL);

        if(SOCKET_ERROR == nRet && ::WSAGetLastError() == WSA_IO_PENDING)
        {
            ::WSAGetOverlappedResult(pThis->m_sSocket, &pThis->m_ol, &dwRecv, TRUE, &dwFlags);
        }

        // 首先查看是否要退出
        if(pThis->m_bExitThread)
            break;

        // 解析不同的消息进行处理
        pMsgHeader = (MSGDef::TMSG_HEADER*)szBuff;
        switch (pMsgHeader->cMsgID)
        {
        case MSG_USERLOGACK:			// 接收到服务器发来的登陆确认
            {
                pThis->ProcUserLogAckMsg(pMsgHeader);
            }
            break;
        case MSG_GETUSERLIST:			// 更新用户列表
            {
                pThis->ProcGetUserList(pMsgHeader);
            }
            break;
        case MSG_USERLISTCMP:			// 更新用户列表完毕
            {
                pThis->ProcUserListCmpMsg();
            }
            break;
        case MSG_P2PMSG:				// 有一个节点向我们发送消息
            {
                pThis->ProcP2PMsg(pMsgHeader, remoteAddr);
            }
            break;
        case MSG_P2PMSGACK:				// 向某个节点发送消息之后的回复
            {
                pThis->m_bMessageACK = true;
            }
            break;
        case MSG_P2PCONNECT:			// 请求打洞
            {
                pThis->ProcP2PConnectMsg(pMsgHeader, remoteAddr);
            }
            break;
        case MSG_P2PCONNECTACK:			// 请求打洞的回复
            {
                pThis->ProcP2PConnectAckMsg(pMsgHeader, remoteAddr);
            }
            break;
        case MSG_USERACTIVEQUERY:		// 用户活跃回复
            {
                pThis->ProcUserActiveQueryMsg(pMsgHeader, remoteAddr);
            }
            break;
        case MSG_NEWFRIENDASK:			// 添加好友请求
            {
                pThis->ProNewFriendAsk(pMsgHeader, remoteAddr);
            }
            break;
        case MSG_NEWFRIENDAGREE:        // 同意添加好友
            {
                pThis->ProNewFriendReply(true, pMsgHeader, remoteAddr);
            }
            break;
        case MSG_NEWFRIENDREFUSE:       // 拒绝添加好友
            {
                pThis->ProNewFriendReply(false, pMsgHeader, remoteAddr);
            }
            break;
        case MSG_NEWROOMIDREPLY:        // 房间ID回复
            {
                pThis->ProNewRoomIdReply(pMsgHeader, remoteAddr);
            }
            break;
        case MSG_NEWROOMASK:            // 加入房间申请
            {
                pThis->ProNewRoomAsk(pMsgHeader, remoteAddr);
            }
            break;
        case MSG_NEWROOMAGREE:          // 同意加入房间
            {
                pThis->ProNewRoomReply(true, pMsgHeader, remoteAddr);
            }
            break;
        case MSG_NEWROOMREFUSE:         // 拒绝加入房间
            {
                pThis->ProNewRoomReply(false, pMsgHeader, remoteAddr);
            }
            break;
        case MSG_NEWROOMNAMELIST:       // 房间名单
            {
                pThis->ProNamelist(pMsgHeader, remoteAddr);
            }
            break;
        case MSG_CHATSTARTALERT:        // 开始讨论
            {
                pThis->ProStartChat(pMsgHeader, remoteAddr);
            }
            break;
        case MSG_NODEACTIVEQUE:         // 存活确认
            {
                pThis->ProActiveQue(pMsgHeader, remoteAddr);
            }
            break;
        case MSG_LINKSTATE:
            {
                pThis->ProLinkState(pMsgHeader, remoteAddr);
            }
            break;
        }
    }

    return 0;
}

bool ClientInfo::ProcCommand(string command)
{
    if (command.length() < 4){
        return false;
    }

    command = command + " ";
    qDebug(command.c_str());

    char szCommand[10];
    char szUserName[MAX_USERNAME];
    char szRoommate[MAX_ROOMMATE][MAX_USERNAME];
    char roomSizeChar[NUM_ROOMSIZECHAR];
    char roomId[NUM_ROOMID];
    int roomSize = 0;
    int pos = 0;

    memset(szCommand, '\0', 10);
    memset(szUserName, '\0', MAX_USERNAME);
    memset(szRoommate, '\0', MAX_ROOMMATE * MAX_USERNAME);
    memset(roomSizeChar, '\0', NUM_ROOMSIZECHAR);
    memset(roomId, '\0', NUM_ROOMID);

    strncpy(szCommand, command.c_str(), 4);

    if(strcmp(szCommand, "getu") == 0) {
        return GetUserList();
    }

    // 刷新状态
    GetUserList();
    emit refreshStatus();

    // 申请添加好友：命令 addf id
    if (strcmp(szCommand, "addf") == 0)
    {
        pos = 5;
        strcpy(szUserName, getInputElement(command, pos, ' ').c_str());

        return SendNewFriendAsk(szUserName);
    }
    // 同意添加好友：命令 rpok id
    else if (strcmp(szCommand, "rpok") == 0)
    {
        pos = 5;
        strcpy(szUserName, getInputElement(command, pos, ' ').c_str());

        return SendNewFriendReply(true, szUserName);
    }
    // 拒绝添加好友：命令 rpno id
    else if (strcmp(szCommand, "rpno") == 0)
    {
        pos = 5;
        strcpy(szUserName, getInputElement(command, pos, ' ').c_str());

        return SendNewFriendReply(false, szUserName);
    }
    // 建立房间：命令 room 3 A B C
    else if (strcmp(szCommand, "room") == 0)
    {
        pos = 5;
        strcpy(roomSizeChar, getInputElement(command, pos, ' ').c_str());
        roomSize = atoi(roomSizeChar);

        int row = 0;
        while (row != roomSize - 1) {
            strcpy(szRoommate[row], getInputElement(command, ++pos, ' ').c_str());
            row++;
        }
        strcpy(szRoommate[row], getInputElement(command, ++pos, ' ').c_str());

        return SetUpARoom(szRoommate, roomSize);
    }
    // 同意加入房间：命令 jiok roomid user
    else if (strcmp(szCommand, "jiok") == 0) {
        pos = 5;
        strcpy(roomId, getInputElement(command, pos, ' ').c_str());
        strcpy(szUserName, getInputElement(command, ++pos, ' ').c_str());

        return SendNewRoomReply(true, szUserName, roomId);
    }
    // 拒绝加入房间：命令 jiok roomid user
    else if (strcmp(szCommand, "jino") == 0) {
        pos = 5;
        strcpy(roomId, getInputElement(command, pos, ' ').c_str());
        strcpy(szUserName, getInputElement(command, ++pos, ' ').c_str());

        return SendNewRoomReply(false, szUserName, roomId);
    }
    // 发送名单：命令 list roomid
    else if (strcmp(szCommand, "list") == 0) {
        pos = 5;
        strcpy(roomId, getInputElement(command, pos, ' ').c_str());

        return SetNameList(roomId);
    }
    // 开启房间：命令 open roomid
    else if (strcmp(szCommand, "open") == 0) {
        pos = 5;
        strcpy(roomId, getInputElement(command, pos, ' ').c_str());

        return SetChatStart(roomId);
    }
    // 发消息：命令 chat roomid XXXXX
    else if (strcmp(szCommand, "chat") == 0) {
        pos = 5;
        strcpy(roomId, getInputElement(command, pos, ' ').c_str());

        char szMsg[256];
        strcpy(szMsg, &command[pos + 1]);

        return SetSendText(roomId, szMsg, (int)strlen(szMsg));
    }
    // 离开房间：命令 leav roomid
    /*else if (strcmp(szCommand, "leav") == 0) {
        pos = 5;
        strcpy(roomId, getInputElement(command, pos, ' ').c_str());

        return SendRoomGuestOperation(roomId, "leave");
    }
    // 进入房间：命令 ente roomid
    else if (strcmp(szCommand, "ente") == 0) {
        pos = 5;
        strcpy(roomId, getInputElement(command, pos, ' ').c_str());

        return SendRoomGuestOperation(roomId, "enter");
    }
    // 退出房间：命令 quit roomid
    else if (strcmp(szCommand, "quit") == 0) {
        pos = 5;
        strcpy(roomId, getInputElement(command, pos, ' ').c_str());

        return SendRoomGuestOperation(roomId, "quit");
    }*/
    // 退出
    else if (strcmp(szCommand, "exit") == 0)
    {
        exit(0);
    }
    myPrintf("Error: Invalid Command!\n");
    return false;
}

// ******打洞相关******===============================================================================================================================
bool ClientInfo::UDP_breakHold(char* pszUserName, Peer_Info* pPeerInfo)
{
    // 请求打洞，并且重新设置P2P地址
    pPeerInfo->P2PAddr.dwIP = 0;

    // 构建P2P打洞封包
    MSGDef::TMSG_P2PCONNECT tP2PConnect(m_PeerInfo);
    strcpy(tP2PConnect.szUserName, pszUserName);

    // 首先直接发向目标,向目标节点的所有适配器发送打洞消息
    sockaddr_in peerAddr = { 0 };
    peerAddr.sin_family = AF_INET;
    int nAddrNum = pPeerInfo->nAddrNum;
    for (int j = 0; j < nAddrNum; ++j)
    {
        peerAddr.sin_addr.S_un.S_addr = pPeerInfo->IPAddr[j].dwIP;
        peerAddr.sin_port = htons(pPeerInfo->IPAddr[j].usPort);
        ::sendto(m_sSocket, (char*)(&tP2PConnect), sizeof(tP2PConnect), 0, (sockaddr*)&peerAddr, sizeof(peerAddr));
    }

    // 然后通过服务器转发，请求对方向自己打洞
    sockaddr_in serverAddr = { 0 };
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.S_un.S_addr = m_dwServerIP;
    serverAddr.sin_port = htons(SERVER_PORT);
    ::sendto(m_sSocket, (char*)(&tP2PConnect), sizeof(tP2PConnect), 0, (sockaddr*)&serverAddr, sizeof(serverAddr));

    // 等待对方的P2PCONNECTACK消息
    for (int j = 0; j < 10; ++j) {
        if (0 != pPeerInfo->P2PAddr.dwIP) {
            break;
        }
        ::Sleep(300);
    }

    return true;
}

// 一个节点请求建立P2P连接（打洞），可能是服务器发来的，也可能是其它节点发来的
bool ClientInfo::ProcP2PConnectMsg(MSGDef::TMSG_HEADER *pMsgHeader, const sockaddr_in& sockAddr)
{
    MSGDef::TMSG_P2PCONNECT *pP2PConnect = (MSGDef::TMSG_P2PCONNECT *)pMsgHeader;
    MSGDef::TMSG_P2PCONNECTACK tP2PConnectAck(m_PeerInfo);

    if (sockAddr.sin_addr.S_un.S_addr != m_dwServerIP)	// 节点发过来的消息
    {
        ::EnterCriticalSection(&m_PeerListLock);
        Peer_Info *pPeerInfo = m_PeerList.GetAPeer( pP2PConnect->PeerInfo.szUserName);

        if (NULL != pPeerInfo) {
            if (0 == pPeerInfo->P2PAddr.dwIP) {
                // 更新该节点用于P2P通信的IP地址和端口号
                pPeerInfo->P2PAddr.dwIP = sockAddr.sin_addr.S_un.S_addr;
                pPeerInfo->P2PAddr.usPort = ntohs(sockAddr.sin_port);
                int temp = ntohs(sockAddr.sin_port);
                myPrintf("Process: 重设P2P地址 %s -> %s:%d \n", pPeerInfo->szUserName,
                    ::inet_ntoa(sockAddr.sin_addr), temp);
            }
        }
        ::LeaveCriticalSection(&m_PeerListLock);
        ::sendto(m_sSocket, (char*)(&tP2PConnectAck), sizeof(tP2PConnectAck), 0, (sockaddr*)(&sockAddr), sizeof(sockAddr));
    }
    else // 服务器转发的消息
    {
        // 向节点的所有终端发送打洞消息
        sockaddr_in peerAddr = { 0 };
        peerAddr.sin_family = AF_INET;
        for(int i = 0, nAddrNum = pP2PConnect->PeerInfo.nAddrNum; i < nAddrNum; ++i) {
            peerAddr.sin_addr.S_un.S_addr = pP2PConnect->PeerInfo.IPAddr[i].dwIP;
            peerAddr.sin_port = htons(pP2PConnect->PeerInfo.IPAddr[i].usPort);
            ::sendto(m_sSocket, (char*)(&tP2PConnectAck), sizeof(tP2PConnectAck), 0, (sockaddr*)(&peerAddr), sizeof(peerAddr));
        }
    }

    return true;
}

// 接收到节点的打洞消息，在这里设置该节点的P2P通信地址
bool ClientInfo::ProcP2PConnectAckMsg(MSGDef::TMSG_HEADER *pMsgHeader, const sockaddr_in& sockAddr)
{
    MSGDef::TMSG_P2PCONNECTACK* pP2PConnectAck = (MSGDef::TMSG_P2PCONNECTACK*)pMsgHeader;

    ::EnterCriticalSection(&m_PeerListLock);
    Peer_Info* pPeerInfo = m_PeerList.GetAPeer(pP2PConnectAck->PeerInfo.szUserName);

    if (NULL != pPeerInfo)
    {
        if (0 == pPeerInfo->P2PAddr.dwIP)
        {
            pPeerInfo->P2PAddr.dwIP = sockAddr.sin_addr.S_un.S_addr;
            pPeerInfo->P2PAddr.usPort = ntohs(sockAddr.sin_port);

            int temp = ntohs(sockAddr.sin_port);
            myPrintf("Process: 重设P2P地址 %s -> %s:%d \n", pP2PConnectAck->PeerInfo.szUserName,
                ::inet_ntoa(sockAddr.sin_addr), temp);
        }
    }
    ::LeaveCriticalSection(&m_PeerListLock);

    return true;
}

// ******登陆******===============================================================================================================================
bool ClientInfo::Login(char *pszUserName, char *pszServerIP)
{
    assert(NULL != pszUserName);
    assert(NULL != pszServerIP);

    int nUserNameLen;
    if ((nUserNameLen = (int)strlen(pszUserName)) > MAX_USERNAME - 1)
    {
        myPrintf("Error: Input User Name Too Large!\n");
        return false;
    }

    if (true == m_bLogin)
    {
        myPrintf("Error: Already Login Server!\n");
        return false;
    }

    // 保存server端IP地址和用户名
    m_dwServerIP = ::inet_addr(pszServerIP);
    memcpy(m_PeerInfo.szUserName, pszUserName, nUserNameLen);

    sockaddr_in serverAddr = {0};
    serverAddr.sin_addr.S_un.S_addr = m_dwServerIP;
    serverAddr.sin_family = AF_INET; //AF_INET（又称 PF_INET）是 IPv4 网络协议的套接字类型，AF_INET6 则是 IPv6 的；而 AF_UNIX 则是 Unix 系统本地通信
    serverAddr.sin_port = htons(SERVER_PORT); //common define

    // 向server发送登陆消息
    MSGDef::TMSG_USERLOGIN tUserLogin(m_PeerInfo); //common  define
    for(int i = 0; i< MAX_TRY_NUMBER; ++i) {
        ::sendto(m_sSocket, (char*)(&tUserLogin), sizeof(MSGDef::TMSG_USERLOGIN), 0, (sockaddr*)&serverAddr, sizeof(serverAddr));

        for(int j = 0; j < 10; j++) {
            if(m_bLogin) {
                return true;
            }
            ::Sleep(300);
        }
    }

    myPrintf("Error: Login Server Failed!\n");
    return false;
}

bool ClientInfo::Logout()
{
    if (true == m_bLogin)
    {
        sockaddr_in serverAddr = {0};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.S_un.S_addr = m_dwServerIP;
        serverAddr.sin_port = htons(SERVER_PORT);

        MSGDef::TMSG_USERLOGOUT tUserLogout(m_PeerInfo);
        ::sendto(m_sSocket, (char*)(&tUserLogout), sizeof(tUserLogout), 0, (sockaddr*)&serverAddr, sizeof(serverAddr));
        m_bLogin = false;

        return true;
    }
    else
    {
        return false;
    }
}

// 接收到服务器发来的登陆确认
bool ClientInfo::ProcUserLogAckMsg(MSGDef::TMSG_HEADER *pMsgHeader)
{
    MSGDef::TMSG_USERLOGACK *pUserLogAckMsg = (MSGDef::TMSG_USERLOGACK *)pMsgHeader;

    memcpy(&m_PeerInfo, &pUserLogAckMsg->PeerInfo, sizeof(Peer_Info));
    int nAddrNum =  pUserLogAckMsg->PeerInfo.nAddrNum;
    in_addr LoginAddr;
    LoginAddr.S_un.S_addr = pUserLogAckMsg->PeerInfo.IPAddr[nAddrNum].dwIP;

    m_bLogin = true;

    myPrintf("Login P2P Server Success!\n");
    return true;
}

// 服务器询问是否存活
bool ClientInfo::ProcUserActiveQueryMsg(MSGDef::TMSG_HEADER *pMsgHeader, const sockaddr_in& sockAddr)
{
    MSGDef::TMSG_USERACTIVEQUERY tUserActiveQuery(m_PeerInfo);
    ::sendto(m_sSocket, (char*)(&tUserActiveQuery), sizeof(tUserActiveQuery), 0, (sockaddr*)&sockAddr, sizeof(sockAddr));

    return true;
}

// ******得到peerInfo******===============================================================================================================================
bool ClientInfo::GetUserList()
{
    sockaddr_in serverAddr = {0};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.S_un.S_addr = m_dwServerIP;
    serverAddr.sin_port = ntohs(SERVER_PORT);

    MSGDef::TMSG_GETUSERLIST tMsgGetUserList(m_PeerInfo);

    ::EnterCriticalSection(&m_PeerListLock); //lock,用于读取m_PeerList的临界区对象
    m_bUserListCmp = false; // 是否得到了用户列表
    m_PeerList.DeleteAllPeer();
    ::LeaveCriticalSection(&m_PeerListLock); //unlock

    for (int i= 0; i < MAX_TRY_NUMBER; ++i){
        ::sendto(m_sSocket, (char*)(&tMsgGetUserList), sizeof(tMsgGetUserList), 0, (sockaddr*)(&serverAddr), sizeof(sockaddr_in));

        for (int j = 0; j < 10; ++j){
            if (true == m_bUserListCmp){
                map<string, Friend_Info*>::iterator i;
                for(i=m_FriendList.begin();i!=m_FriendList.end();i++) {
                    char name[MAX_USERNAME];
                    strcpy(name, i->first.c_str());
                    Peer_Info* peer = m_PeerList.GetAPeer(name);
                    if(peer != NULL) {
                        i->second->online = true;
                    }
                    else {
                        i->second->online = false;
                    }
                }
                return true;
            }
            ::Sleep(300);
        }
    }

    return false;
}

// 更新用户列表
bool ClientInfo::ProcGetUserList(MSGDef::TMSG_HEADER *pMsgHeader)
{
    MSGDef::TMSG_GETUSERLIST* pMsgGetUserList = (MSGDef::TMSG_GETUSERLIST*)pMsgHeader;

    pMsgGetUserList->PeerInfo.P2PAddr.dwIP = 0;

    ::EnterCriticalSection(&m_PeerListLock);
    m_PeerList.AddPeer(pMsgGetUserList->PeerInfo);
    ::LeaveCriticalSection(&m_PeerListLock);

    return true;
}

// 更新用户列表完毕
bool ClientInfo::ProcUserListCmpMsg()
{
    m_bUserListCmp = true;

    return true;
}

// ******添加好友*******=============================================================================================================
bool ClientInfo::SendNewFriendAsk(char *pszUserName)
{
    // 检查是否已经添加
    if (m_FriendList.find(string(pszUserName)) != m_FriendList.end()) {
        myPrintf("Error: 该用户已在好友列表中\n");
        return false;
    }

    MSGDef::TMSG_NEWFRIENDASK tP2PFriAsk(m_PeerInfo);
    m_bMessageACK = false;
    Peer_Info* pPeerInfo;
    for (int i = 0; i < MAX_TRY_NUMBER; ++i)
    {
        pPeerInfo = m_PeerList.GetAPeer(pszUserName);
        if (NULL == pPeerInfo) {
            myPrintf("Error: 无法获取用户的通信地址\n");
            return false;
        }

        if (0 != pPeerInfo->P2PAddr.dwIP)
        {
            sockaddr_in peerAddr = { 0 };
            peerAddr.sin_family = AF_INET;
            peerAddr.sin_port = ntohs(pPeerInfo->P2PAddr.usPort);
            peerAddr.sin_addr.S_un.S_addr = pPeerInfo->P2PAddr.dwIP;

            ::sendto(m_sSocket, (char*)(&tP2PFriAsk), sizeof(tP2PFriAsk), 0, (sockaddr*)(&peerAddr), sizeof(peerAddr));

            for (int j = 0; j < 10; ++j) {
                if (true == m_bMessageACK) {
                    myPrintf("Result: 发送好友申请成功\n");
                    return true;
                }
                ::Sleep(300);
            }
        }
        UDP_breakHold(pszUserName, pPeerInfo);
    }

    return false;
}

bool ClientInfo::ProNewFriendReply(bool answer, MSGDef::TMSG_HEADER *pMsgHeader, const sockaddr_in& sockAddr)
{
    // 发送收到消息的确认消息
    MSGDef::TMSG_P2PMSGACK tP2PMsgAck(m_PeerInfo);
    ::sendto(m_sSocket, (char*)(&tP2PMsgAck), sizeof(tP2PMsgAck), 0, (sockaddr*)(&sockAddr), sizeof(sockAddr));

    if (answer == true) {
        MSGDef::TMSG_NEWFRIENDAGREE *pP2PFriRep = (MSGDef::TMSG_NEWFRIENDAGREE *)pMsgHeader;

        myPrintf("Result: %s同意好友申请\n", pP2PFriRep->PeerInfo.szUserName);

        // 修改m_FriendList
        string name = string(pP2PFriRep->PeerInfo.szUserName);
        Friend_Info* newFriend = new Friend_Info(pP2PFriRep->PeerInfo.szUserName, true);
        m_FriendList[name] = newFriend;
        // 修改界面和文件系统
        emit NewFriendAlert(newFriend);
    }
    else {
        MSGDef::TMSG_NEWFRIENDREFUSE *pP2PFriRep = (MSGDef::TMSG_NEWFRIENDREFUSE *)pMsgHeader;

        myPrintf("Result: %s拒绝好友申请\n", pP2PFriRep->PeerInfo.szUserName);
    }
    return true;
}

bool ClientInfo::ProNewFriendAsk(MSGDef::TMSG_HEADER *pMsgHeader, const sockaddr_in& sockAddr)
{
    // 发送收到消息的确认消息
    MSGDef::TMSG_P2PMSGACK tP2PMsgAck(m_PeerInfo);
    ::sendto(m_sSocket, (char*)(&tP2PMsgAck), sizeof(tP2PMsgAck), 0, (sockaddr*)(&sockAddr), sizeof(sockAddr));

    MSGDef::TMSG_NEWFRIENDASK *pP2PFriAsk = (MSGDef::TMSG_NEWFRIENDASK *)pMsgHeader;
    myPrintf("Message: %s请求添加好友\n", pP2PFriAsk->PeerInfo.szUserName);

    // 添加一条通知，修改界面
    string inform = string(pP2PFriAsk->PeerInfo.szUserName);
    emit informAlert("friend", inform);
    return true;
}

bool ClientInfo::SendNewFriendReply(bool answer, char *pszUserName)
{
    m_bMessageACK = false;
    Peer_Info* pPeerInfo;
    for (int i = 0; i < MAX_TRY_NUMBER; ++i){
        pPeerInfo = m_PeerList.GetAPeer(pszUserName);
        if (NULL == pPeerInfo){
            myPrintf("Error: 无法获取用户的通信地址\n");
            return false;
        }

        if (0 != pPeerInfo->P2PAddr.dwIP)
        {
            sockaddr_in peerAddr = { 0 };
            peerAddr.sin_family = AF_INET;
            peerAddr.sin_port = ntohs(pPeerInfo->P2PAddr.usPort);
            peerAddr.sin_addr.S_un.S_addr = pPeerInfo->P2PAddr.dwIP;

            if (answer == true) {
                MSGDef::TMSG_NEWFRIENDAGREE tP2PFriRep(m_PeerInfo);
                ::sendto(m_sSocket, (char*)(&tP2PFriRep), sizeof(tP2PFriRep), 0, (sockaddr*)(&peerAddr), sizeof(peerAddr));
            }
            else {
                MSGDef::TMSG_NEWFRIENDREFUSE tP2PFriRep(m_PeerInfo);
                ::sendto(m_sSocket, (char*)(&tP2PFriRep), sizeof(tP2PFriRep), 0, (sockaddr*)(&peerAddr), sizeof(peerAddr));
            }

            for (int j = 0; j < 10; ++j){
                if (true == m_bMessageACK){
                    if(answer == true) {
                        // 修改m_FriendList
                        string name = string(pszUserName);
                        Friend_Info* newFriend = new Friend_Info(pszUserName, true);
                        m_FriendList[name] = newFriend;

                        // 修改界面和文件系统
                        emit NewFriendAlert(newFriend);
                    }
                    return true;
                }
                ::Sleep(300);
            }
        }
        UDP_breakHold(pszUserName, pPeerInfo);
    }

    return false;
}

// ******建立房间*******===============================================================================================================================
bool ClientInfo::SetUpARoom(char szRoommate[MAX_ROOMMATE][MAX_USERNAME], int roomSize)
{
    // 获取房间id
    char roomId[NUM_ROOMID];
    char* temp = SendNewRoomIdApply();
    if (temp != NULL) {
        strcpy(roomId, temp);
        myPrintf("Process: 得到房间ID %s\n", roomId);
    }
    else {
        myPrintf("Error: 申请房间ID失败\n");
    }

    // 发送ASK，收集房间客人状态
    set<char*> roommates;
    roommates.insert(m_PeerInfo.szUserName);
    for (int i = 0; i < roomSize; i++) {
        if (SendNewRoomAsk(szRoommate[i], roomId)) {
            roommates.insert(szRoommate[i]);
        }
        else {
            myPrintf("Error: 用户%s不可达\n", szRoommate[i]);
        }
    }

    if (roommates.size() == 1) {
        myPrintf("Error: 无法连接上任何一个用户\n");
        return false;
    }

    // 建立房间开始等待
    roomSize = roommates.size();
    Chat_Info* newChat = new Chat_Info(roomId, m_PeerInfo.szUserName, roommates, roomSize, 'O', QDateTime::currentDateTime());

    if (NULL == (::CreateThread(NULL, 0, ManageRoom_Parent, newChat, 0, NULL)))
    {
        myPrintf("Error: Create ManageRoom_Parent Failed!\n");
        return false;
    }
    if (NULL == (::CreateThread(NULL, 0, ManageRoom_Child, newChat, 0, NULL)))
    {
        myPrintf("Error: Create ManageRoom_Child Failed!\n");
        return false;
    }

    // 修改m_ChatList
    m_ChatList[roomId] = newChat;

    // 修改界面和文件系统
    emit NewRoomAlert(newChat);

    /*while (newChat->c_allReply == false) {
        ::Sleep(3000);
    }*/
    // 等待信号量
    WaitForSingleObject(newChat->c_allReplySemaphore, INFINITE);

    // 所有客人回复后，查看房间人数
    if (m_ChatList[roomId]->roomSize == 1) {
        m_ChatList.erase(roomId);
        myPrintf("没有人愿意加入房间\n");
        return false;
    }

    if (SetNameList(roomId) == false) {
        myPrintf("Error: 发送NameList失败\n");
        return false;
    }

    if (SetChatStart(roomId) == false) {
        myPrintf("Error: 发送ChatStart失败\n");
        return false;
    }

    myPrintf("Reslut: 成功建立房间\n");
    return true;
}

char* ClientInfo::SendNewRoomIdApply()
{
    m_bParent_roomIdReady = false; //未收到应答

    sockaddr_in serverAddr = { 0 };
    serverAddr.sin_addr.S_un.S_addr = m_dwServerIP;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);

    MSGDef::TMSG_NEWROOMIDAPPLY tRoomIdAsk(m_PeerInfo);
    for (int i = 0; i< MAX_TRY_NUMBER; ++i){
        ::sendto(m_sSocket, (char*)(&tRoomIdAsk), sizeof(tRoomIdAsk), 0, (sockaddr*)&serverAddr, sizeof(serverAddr));

        for (int j = 0; j < 10; j++){
            if (m_bParent_roomIdReady == true) {
                return m_bParent_roomId;
            }
            ::Sleep(300);
        }
    }

    return NULL;
}

bool ClientInfo::ProNewRoomIdReply(MSGDef::TMSG_HEADER *pMsgHeader, const sockaddr_in& sockAddr)
{
    // 发送收到消息的确认消息
    MSGDef::TMSG_P2PMSGACK tP2PMsgAck(m_PeerInfo);
    ::sendto(m_sSocket, (char*)(&tP2PMsgAck), sizeof(tP2PMsgAck), 0, (sockaddr*)(&sockAddr), sizeof(sockAddr));

    MSGDef::TMSG_NEWROOMIDREPLY *pP2PIdRep = (MSGDef::TMSG_NEWROOMIDREPLY *)pMsgHeader;
    strcpy(m_bParent_roomId, pP2PIdRep->roomId);
    m_bParent_roomIdReady = true;

    return true;
}

bool ClientInfo::SendNewRoomAsk(char *pszUserName, char *roomId)
{
    MSGDef::TMSG_NEWROOMASK tP2PRooAsk(m_PeerInfo);
    strcpy(tP2PRooAsk.roomId, roomId);

    m_bMessageACK = false;
    Peer_Info* pPeerInfo;
    for (int i = 0; i < MAX_TRY_NUMBER; ++i) {
        pPeerInfo = m_PeerList.GetAPeer(pszUserName);
        if (NULL == pPeerInfo) {
            myPrintf("Error: 无法获取用户的通信地址\n");
            return false;
        }

        if (0 != pPeerInfo->P2PAddr.dwIP)
        {
            sockaddr_in peerAddr = { 0 };
            peerAddr.sin_family = AF_INET;
            peerAddr.sin_port = ntohs(pPeerInfo->P2PAddr.usPort);
            peerAddr.sin_addr.S_un.S_addr = pPeerInfo->P2PAddr.dwIP;

            ::sendto(m_sSocket, (char*)(&tP2PRooAsk), sizeof(tP2PRooAsk), 0, (sockaddr*)(&peerAddr), sizeof(peerAddr));

            for (int j = 0; j < 10; ++j) {
                if (true == m_bMessageACK) {
                    return true;
                }
                ::Sleep(300);
            }
        }
        UDP_breakHold(pszUserName, pPeerInfo);
    }

    return false;
}

bool ClientInfo::ProNewRoomReply(bool answer, MSGDef::TMSG_HEADER *pMsgHeader, const sockaddr_in& sockAddr)
{
    // 发送收到消息的确认消息
    MSGDef::TMSG_P2PMSGACK tP2PMsgAck(m_PeerInfo);
    ::sendto(m_sSocket, (char*)(&tP2PMsgAck), sizeof(tP2PMsgAck), 0, (sockaddr*)(&sockAddr), sizeof(sockAddr));

    string key = "";
    if (answer == true) {
        MSGDef::TMSG_NEWROOMAGREE *pP2PRooRep = (MSGDef::TMSG_NEWROOMAGREE *)pMsgHeader;

        myPrintf("%s同意加入房间%s.\n", pP2PRooRep->PeerInfo.szUserName, pP2PRooRep->roomId);

        key = string(pP2PRooRep->roomId);
        if (m_ChatList.find(key) != m_ChatList.end()) {
            m_ChatList[key]->SetState(pP2PRooRep->PeerInfo.szUserName, 'A');
        }
        else {
            return false;
        }
    }
    else {
        MSGDef::TMSG_NEWROOMREFUSE *pP2PRooRep = (MSGDef::TMSG_NEWROOMREFUSE *)pMsgHeader;

        myPrintf("%s拒绝加入房间%s.\n", pP2PRooRep->PeerInfo.szUserName, pP2PRooRep->roomId);

        key = string(pP2PRooRep->roomId);
        if (m_ChatList.find(key) != m_ChatList.end()) {
            m_ChatList[key]->DeleteUser(pP2PRooRep->PeerInfo.szUserName);
            m_ChatList[key]->roomSize--;
        }
        else {
            return false;
        }
    }

    if (m_ChatList[key]->AllGuestReply()) {
        myPrintf("所有客人均已回复\n");
    }

    return true;
}

bool ClientInfo::SetNameList(char* roomId)
{
    string key = string(roomId);

    MSGDef::TMSG_NEWROOMNAMELIST tNameList(m_PeerInfo);
    strcpy(tNameList.roomId, roomId); 
    tNameList.roomSize = m_ChatList[key]->roomSize;

    map<string, char>::iterator i;
    int row = 0;
    for (i = m_ChatList[key]->state.begin(); i != m_ChatList[key]->state.end(); i++) {
        strcpy(tNameList.namelist[row], i->first.c_str());
        row++;
    }

    char guestname[MAX_USERNAME];
    for (i = m_ChatList[key]->state.begin(); i != m_ChatList[key]->state.end(); i++) {
        strcpy(guestname, i->first.c_str());
        if (strcmp(guestname, m_PeerInfo.szUserName) != 0) {
            if (!SendNameList(guestname, &tNameList)) {
                myPrintf("Error: 发送Namelist给用户%s失败\n", guestname);
            }
        }
    }
    return true;
}

bool ClientInfo::SendNameList(char *pszUserName, MSGDef::TMSG_NEWROOMNAMELIST *tNameList)
{
    m_bMessageACK = false;
    Peer_Info* pPeerInfo;
    for (int i = 0; i < MAX_TRY_NUMBER; ++i)
    {
        pPeerInfo = m_PeerList.GetAPeer(pszUserName);
        if (NULL == pPeerInfo) {
            myPrintf("Error: 无法获取用户的通信地址\n");
            return false;
        }

        if (0 != pPeerInfo->P2PAddr.dwIP) {
            sockaddr_in peerAddr = { 0 };
            peerAddr.sin_family = AF_INET;
            peerAddr.sin_port = ntohs(pPeerInfo->P2PAddr.usPort);
            peerAddr.sin_addr.S_un.S_addr = pPeerInfo->P2PAddr.dwIP;

            ::sendto(m_sSocket, (char*)(tNameList), sizeof(*tNameList), 0, (sockaddr*)(&peerAddr), sizeof(peerAddr));

            for (int j = 0; j < 10; ++j) {
                if (true == m_bMessageACK) {
                    return true;
                }
                ::Sleep(300);
            }
        }
        UDP_breakHold(pszUserName, pPeerInfo);
    }

    return false;
}

bool ClientInfo::SetChatStart(char* roomId)
{
    string key = string(roomId);

    m_ChatList[key]->online = true;

    MSGDef::TMSG_CHATSTARTALERT tChatStart(m_PeerInfo);
    strcpy(tChatStart.roomId, roomId);

    map<string, char>::iterator i;
    char guestname[MAX_USERNAME];
    for (i = m_ChatList[key]->state.begin(); i != m_ChatList[key]->state.end(); i++) {
        strcpy(guestname, i->first.c_str());
        if (strcmp(guestname, m_PeerInfo.szUserName) != 0) {
            if (SendChatStart(guestname, &tChatStart)) {
                m_ChatList[key]->SetState(guestname, 'T');
                myPrintf("用户%s进入房间\n", guestname);
            }
            else {
                m_ChatList[key]->SetState(guestname, 'F');
                myPrintf("用户%s进入房间失败\n", guestname);
            }
        }
    }

    // 非末端节点
    m_ChatList[key]->m_endNode = false;

    return true;
}

bool ClientInfo::SendChatStart(char *pszUserName, MSGDef::TMSG_CHATSTARTALERT *tChatStart)
{
    m_bMessageACK = false;
    Peer_Info* pPeerInfo;
    for (int i = 0; i < MAX_TRY_NUMBER; ++i) {
        pPeerInfo = m_PeerList.GetAPeer(pszUserName);
        if (NULL == pPeerInfo) {
            myPrintf("Error: 无法获取用户的通信地址\n");
            return false;
        }

        if (0 != pPeerInfo->P2PAddr.dwIP) {
            sockaddr_in peerAddr = { 0 };
            peerAddr.sin_family = AF_INET;
            peerAddr.sin_port = ntohs(pPeerInfo->P2PAddr.usPort);
            peerAddr.sin_addr.S_un.S_addr = pPeerInfo->P2PAddr.dwIP;

            ::sendto(m_sSocket, (char*)(tChatStart), sizeof(*tChatStart), 0, (sockaddr*)(&peerAddr), sizeof(peerAddr));

            for (int j = 0; j < 10; ++j) {
                if (true == m_bMessageACK) {
                    return true;
                }
                ::Sleep(300);
            }
        }
        UDP_breakHold(pszUserName, pPeerInfo);
    }

    return false;
}

bool ClientInfo::ProNewRoomAsk(MSGDef::TMSG_HEADER *pMsgHeader, const sockaddr_in& sockAddr)
{
    // 发送收到消息的确认消息
    MSGDef::TMSG_P2PMSGACK tP2PMsgAck(m_PeerInfo);
    ::sendto(m_sSocket, (char*)(&tP2PMsgAck), sizeof(tP2PMsgAck), 0, (sockaddr*)(&sockAddr), sizeof(sockAddr));

    MSGDef::TMSG_NEWROOMASK *pP2PRooAsk = (MSGDef::TMSG_NEWROOMASK *)pMsgHeader;
    myPrintf("Message: %s请求加入房间%s\n", pP2PRooAsk->PeerInfo.szUserName, pP2PRooAsk->roomId);

    // 添加通知
    string inform = string(pP2PRooAsk->roomId) + " " + string(pP2PRooAsk->PeerInfo.szUserName);
    emit informAlert("room", inform);

    return true;
}

bool ClientInfo::SendNewRoomReply(bool answer, char *pszUserName, char *roomId)
{
    m_bMessageACK = false;
    Peer_Info* pPeerInfo;
    for (int i = 0; i < MAX_TRY_NUMBER; ++i)
    {
        pPeerInfo = m_PeerList.GetAPeer(pszUserName);
        if (NULL == pPeerInfo) {
            myPrintf("Error: 无法获取用户的通信地址\n");
            return false;
        }

        if (0 != pPeerInfo->P2PAddr.dwIP) {
            sockaddr_in peerAddr = { 0 };
            peerAddr.sin_family = AF_INET;
            peerAddr.sin_port = ntohs(pPeerInfo->P2PAddr.usPort);
            peerAddr.sin_addr.S_un.S_addr = pPeerInfo->P2PAddr.dwIP;

            if (answer == true) {
                MSGDef::TMSG_NEWROOMAGREE tP2PRooRep(m_PeerInfo);
                strcpy(tP2PRooRep.roomId, roomId);
                ::sendto(m_sSocket, (char*)(&tP2PRooRep), sizeof(tP2PRooRep), 0, (sockaddr*)(&peerAddr), sizeof(peerAddr));
            }
            else {
                MSGDef::TMSG_NEWROOMREFUSE tP2PRooRep(m_PeerInfo);
                strcpy(tP2PRooRep.roomId, roomId);
                ::sendto(m_sSocket, (char*)(&tP2PRooRep), sizeof(tP2PRooRep), 0, (sockaddr*)(&peerAddr), sizeof(peerAddr));
            }

            for (int j = 0; j < 10; ++j){
                if (true == m_bMessageACK){
                    return true;
                }
                ::Sleep(300);
            }
        }
        UDP_breakHold(pszUserName, pPeerInfo);
    }

    return false;
}

bool ClientInfo::ProNamelist(MSGDef::TMSG_HEADER *pMsgHeader, const sockaddr_in& sockAddr)
{
    // 发送收到消息的确认消息
    MSGDef::TMSG_P2PMSGACK tP2PMsgAck(m_PeerInfo);
    ::sendto(m_sSocket, (char*)(&tP2PMsgAck), sizeof(tP2PMsgAck), 0, (sockaddr*)(&sockAddr), sizeof(sockAddr));

    MSGDef::TMSG_NEWROOMNAMELIST *pP2Pnl = (MSGDef::TMSG_NEWROOMNAMELIST *)pMsgHeader;

    set<char*> namelist;
    for (int i = 0; i < pP2Pnl->roomSize; i++) {
        namelist.insert(pP2Pnl->namelist[i]);
    }

    // 建立房间
    Chat_Info* newChat = new Chat_Info(pP2Pnl->roomId, pP2Pnl->PeerInfo.szUserName, namelist, pP2Pnl->roomSize, 'A', QDateTime::currentDateTime());

    if (NULL == (::CreateThread(NULL, 0, ManageRoom_Parent, newChat, 0, NULL)))
    {
        myPrintf("Error: Create ManageRoom_Parent Failed!\n");
        return false;
    }
    if (NULL == (::CreateThread(NULL, 0, ManageRoom_Child, newChat, 0, NULL)))
    {
        myPrintf("Error: Create ManageRoom_CHILD Failed!\n");
        return false;
    }

    // 修改m_ChatList
    string key = string(pP2Pnl->roomId);
    m_ChatList[key] = newChat;
    // 修改界面和文件系统
    emit NewRoomAlert(newChat);

    // 修改父节点的状态
    m_ChatList[key]->SetState(pP2Pnl->PeerInfo.szUserName, 'T');

    return true;
}

bool ClientInfo::ProStartChat(MSGDef::TMSG_HEADER *pMsgHeader, const sockaddr_in& sockAddr)
{
    // 发送收到消息的确认消息
    MSGDef::TMSG_P2PMSGACK tP2PMsgAck(m_PeerInfo);
    ::sendto(m_sSocket, (char*)(&tP2PMsgAck), sizeof(tP2PMsgAck), 0, (sockaddr*)(&sockAddr), sizeof(sockAddr));

    MSGDef::TMSG_CHATSTARTALERT *pP2Pcs = (MSGDef::TMSG_CHATSTARTALERT *)pMsgHeader;
    qDebug("Process: %s通知房间%s开启\n", pP2Pcs->PeerInfo.szUserName, pP2Pcs->roomId);

    string key = string(pP2Pcs->roomId);
    if(m_ChatList.find(key) != m_ChatList.end()) {
        m_ChatList[key]->online = true;
        // 修改侧边栏
        emit refreshStatus();
        return true;
    }
    return false;
}

// ******发送消息*******==================================================================================================================
bool ClientInfo::SetSendText(char *roomid, char *pszText, int nTextLen)
{
    if (NULL == roomid || NULL == pszText
        || strlen(roomid) > MAX_USERNAME
        || nTextLen > MAX_PACKET_SIZE - sizeof(Peer_Info) - sizeof(MSGDef::TMSG_HEADER))
    {
        return false;
    }

    string key = string(roomid);
    if (m_ChatList.find(key) == m_ChatList.end()) {
        myPrintf("Error: 房间未建立\n");
        return false;
    }

    if (m_ChatList[key]->online == false) {
        myPrintf("Error: 房间静默\n");
        return false;
    }

    // 消息包
    MSGDef::TMSG_P2PMSG tP2PMsg(m_PeerInfo);
    strcpy(tP2PMsg.roomId, roomid);
    strcpy(tP2PMsg.szMsg, pszText);
    strcpy(tP2PMsg.user, m_PeerInfo.szUserName);

    // 向所有邻居发送
    map<string, char>::iterator i;
    char username[MAX_USERNAME];
    for (i = m_ChatList[key]->state.begin(); i != m_ChatList[key]->state.end(); i++) {
        strcpy(username, i->first.c_str());
        if (i->second == 'T' && strcmp(username, m_PeerInfo.szUserName) != 0) {
            if(!SendText(username, &tP2PMsg)) {
                myPrintf("Error: 消息发送给%s失败\n", username);
            }
        }
    }

    // 修改界面
    emit messageAlert(m_ChatList[key], m_PeerInfo.szUserName, pszText);
    return true;
}

bool ClientInfo::SendText(char *pszUserName, MSGDef::TMSG_P2PMSG *msg)
{
    m_bMessageACK = false;
    Peer_Info* pPeerInfo;
    for (int i = 0; i < MAX_TRY_NUMBER; ++i)
    {
        pPeerInfo = m_PeerList.GetAPeer(pszUserName);
        if (NULL == pPeerInfo) {
            myPrintf("Error: 无法获取用户的通信地址\n");
            return false;
        }

        if (0 != pPeerInfo->P2PAddr.dwIP)
        {
            sockaddr_in peerAddr = { 0 };
            peerAddr.sin_family = AF_INET;
            peerAddr.sin_port = ntohs(pPeerInfo->P2PAddr.usPort);
            peerAddr.sin_addr.S_un.S_addr = pPeerInfo->P2PAddr.dwIP;

            ::sendto(m_sSocket, (char*)(msg), sizeof(*msg), 0, (sockaddr*)(&peerAddr), sizeof(peerAddr));

            for (int j = 0; j < 10; ++j) {
                if (true == m_bMessageACK) {
                    return true;
                }
                ::Sleep(300);
            }
        }
        UDP_breakHold(pszUserName, pPeerInfo);
    }

    return false;
}

bool ClientInfo::ProcP2PMsg(MSGDef::TMSG_HEADER *pMsgHeader, const sockaddr_in& sockAddr)
{
    // 发送收到消息的确认消息
    MSGDef::TMSG_P2PMSGACK tP2PMsgAck(m_PeerInfo);
    ::sendto(m_sSocket, (char*)(&tP2PMsgAck), sizeof(tP2PMsgAck), 0, (sockaddr*)(&sockAddr), sizeof(sockAddr));

    MSGDef::TMSG_P2PMSG *pP2PMsg = (MSGDef::TMSG_P2PMSG *)pMsgHeader;
    string key = string(pP2PMsg->roomId);

    if (m_ChatList.find(key) == m_ChatList.end()) {
        return false;
    }

    //修改界面
    emit messageAlert(m_ChatList[key], pP2PMsg->user, pP2PMsg->szMsg);

    // 不是末端节点，转发
    if (!m_ChatList[key]->m_endNode) {
        myPrintf("transmit to ");

        MSGDef::TMSG_P2PMSG tP2PMsg(m_PeerInfo);
        strcpy(tP2PMsg.roomId, pP2PMsg->roomId);
        strcpy(tP2PMsg.szMsg, pP2PMsg->szMsg);
        strcpy(tP2PMsg.user, pP2PMsg->user);

        ::EnterCriticalSection(&(m_ChatList[key]->m_forwardLock));

        m_ChatList[key]->m_sender = string(pP2PMsg->PeerInfo.szUserName);
        m_ChatList[key]->m_forwardMsg = &tP2PMsg;
        m_ChatList[key]->m_forwardAlert = true;

        ::LeaveCriticalSection(&(m_ChatList[key]->m_forwardLock));
    }

    return true;
}

// ******管理房间*******===============================================================================================================================
DWORD WINAPI ClientInfo::ManageRoom_Parent(LPVOID lpParam)
{
    Chat_Info* pThis = (Chat_Info*)lpParam;

    string key = pThis->roomId;

    char roomId[NUM_ROOMID];
    strcpy(roomId, pThis->roomId.c_str());

    map<string, char>::iterator i;
    char node1[MAX_USERNAME];
    strcpy(node1, myClient->m_PeerInfo.szUserName);

    int n_my;
    int n_user;

    while (true) {
        // 转发消息
        if (pThis->m_forwardAlert == true && pThis->online == true) {

            ::EnterCriticalSection(&(pThis->m_forwardLock));

            char username[MAX_USERNAME];
            for (i = pThis->state.begin(); i != pThis->state.end(); i++) {
                if (i->second == 'T' && i->first != pThis->m_sender && i->first != string(myClient->m_PeerInfo.szUserName)) {
                    strcpy(username, i->first.c_str());
                    myClient->SendText(username, pThis->m_forwardMsg);
                    printf("%s ", username);
                }
            }
            printf("\n");
            pThis->m_forwardAlert = false;

            ::LeaveCriticalSection(&(pThis->m_forwardLock));
        }

        // 转发LSA
        if (pThis->w_newLinkState == true && pThis->online == false) {
            pThis->w_newLinkState = false;

            n_my = pThis->w_graph->node[string(myClient->m_PeerInfo.szUserName)];
            char username[MAX_USERNAME];
            for (i = pThis->state.begin(); i != pThis->state.end(); i++) {
                n_user = pThis->w_graph->node[i->first];
                strcpy(username, i->first.c_str());
                if (n_my != n_user && pThis->w_graph->edge[n_my][n_user] == 1) {
                    myClient->SendLinkState(username, roomId);
                }
            }
        }
    }
    return 0;
}

DWORD WINAPI ClientInfo::ManageRoom_Child(LPVOID lpParam)
{
    Chat_Info* pThis = (Chat_Info*)lpParam;

    map<string, char>::iterator i;

    char username[MAX_USERNAME];
    char roomId[NUM_ROOMID];
    strcpy(roomId, pThis->roomId.c_str());

    while (true) {
        // 收到LSA重选
        if (pThis->w_selectionAlert == true && pThis->online == true) {
            pThis->online = false;
            pThis->m_endNode = true;
            myClient->ManageSelection(pThis);
            printf("finish rebuild process.\n");
            pThis->online = true;
        }

        // 测试相邻节点是否存活
        for (i = pThis->state.begin(); i != pThis->state.end(); i++) {
            if (i->second == 'T' && i->first != string(myClient->m_PeerInfo.szUserName)) {
                strcpy(username, i->first.c_str());
                if (!myClient->SendNodeQue(username, roomId, 'o')) {
                    printf("Node %s down\n", username);
                    pThis->setNewGraph();
                    pThis->w_selectionAlert = true;

                    break;
                }
            }

            ::Sleep(5000);
        }
    }
    return 0;
}

bool ClientInfo::ManageSelection(Chat_Info* chat)
{
    myPrintf("Process: ManageSelection\n");

    map<string, char>::iterator i;
    char roomId[NUM_ROOMID];
    char myName[MAX_USERNAME];

    strcpy(roomId, chat->roomId.c_str());
    strcpy(myName, m_PeerInfo.szUserName);

    for (i = chat->state.begin(); i != chat->state.end(); i++) {
        char username[MAX_USERNAME];
        strcpy(username, i->first.c_str());
        if (strcmp(myName, username) != 0) {
            if (SendLinkState(username, roomId)) {
                chat->addEdge(myName, username, 1);
            }
        }
    }

    ::Sleep(20000);
    //chat->DisplayGraph();

    chat->calculateTree(myName);

    chat->w_selectionAlert = false;
    return true;
}

bool ClientInfo::SendNodeQue(char *pszUserName, char *roomId, char type)
{
    MSGDef::TMSG_NODEACTIVEQUE tP2Pact(m_PeerInfo);
    strcpy(tP2Pact.roomId, roomId);
    tP2Pact.type = type;

    m_bMessageACK = false;
    Peer_Info* pPeerInfo;
    for (int i = 0; i < MAX_TRY_NUMBER; ++i) {
        pPeerInfo = m_PeerList.GetAPeer(pszUserName);
        if (NULL == pPeerInfo) {
            myPrintf("Error: 无法获取用户的通信地址\n");
            return false;
        }

        if (0 != pPeerInfo->P2PAddr.dwIP) {
            sockaddr_in peerAddr = { 0 };
            peerAddr.sin_family = AF_INET;
            peerAddr.sin_port = ntohs(pPeerInfo->P2PAddr.usPort);
            peerAddr.sin_addr.S_un.S_addr = pPeerInfo->P2PAddr.dwIP;

            ::sendto(m_sSocket, (char*)(&tP2Pact), sizeof(tP2Pact), 0, (sockaddr*)(&peerAddr), sizeof(peerAddr));

            for (int j = 0; j < 10; ++j) {
                if (true == m_bMessageACK) {
                    return true;
                }
                ::Sleep(300);
            }
        }
        UDP_breakHold(pszUserName, pPeerInfo);
    }

    return false;
}

bool ClientInfo::ProActiveQue(MSGDef::TMSG_HEADER *pMsgHeader, const sockaddr_in& sockAddr)
{
    // 发送收到消息的确认消息
    MSGDef::TMSG_P2PMSGACK tP2PMsgAck(m_PeerInfo);
    ::sendto(m_sSocket, (char*)(&tP2PMsgAck), sizeof(tP2PMsgAck), 0, (sockaddr*)(&sockAddr), sizeof(sockAddr));

    MSGDef::TMSG_NODEACTIVEQUE* pP2Pact = (MSGDef::TMSG_NODEACTIVEQUE*)pMsgHeader;
    string key = string(pP2Pact->roomId);

    if (m_ChatList.find(key) == m_ChatList.end()) {
        return false;
    }

    return true;
}

bool ClientInfo::SendLinkState(char *pszUserName, char *roomId)
{
    MSGDef::TMSG_LINKSTATE tP2Psta(m_PeerInfo);
    strcpy(tP2Psta.roomId, roomId);
    m_ChatList[string(roomId)]->copyEdge(tP2Psta.edge);

    m_bMessageACK = false;
    Peer_Info* pPeerInfo;
    for (int i = 0; i < MAX_TRY_NUMBER; ++i) {
        pPeerInfo = m_PeerList.GetAPeer(pszUserName);
        if (NULL == pPeerInfo) {
            myPrintf("Error: 无法获取用户的通信地址\n");
            return false;
        }

        if (0 != pPeerInfo->P2PAddr.dwIP) {
            sockaddr_in peerAddr = { 0 };
            peerAddr.sin_family = AF_INET;
            peerAddr.sin_port = ntohs(pPeerInfo->P2PAddr.usPort);
            peerAddr.sin_addr.S_un.S_addr = pPeerInfo->P2PAddr.dwIP;

            ::sendto(m_sSocket, (char*)(&tP2Psta), sizeof(tP2Psta), 0, (sockaddr*)(&peerAddr), sizeof(peerAddr));

            for (int j = 0; j < 10; ++j) {
                if (true == m_bMessageACK) {
                    return true;
                }
                ::Sleep(300);
            }
        }
        UDP_breakHold(pszUserName, pPeerInfo);
    }

    return false;
}

bool ClientInfo::ProLinkState(MSGDef::TMSG_HEADER *pMsgHeader, const sockaddr_in& sockAddr)
{
    // 发送收到消息的确认消息
    MSGDef::TMSG_P2PMSGACK tP2PMsgAck(m_PeerInfo);
    ::sendto(m_sSocket, (char*)(&tP2PMsgAck), sizeof(tP2PMsgAck), 0, (sockaddr*)(&sockAddr), sizeof(sockAddr));

    MSGDef::TMSG_LINKSTATE* pP2Psta = (MSGDef::TMSG_LINKSTATE*)pMsgHeader;
    string key = string(pP2Psta->roomId);

    if (m_ChatList.find(key) == m_ChatList.end()) {
        return false;
    }

    if (m_ChatList[key]->w_graph == NULL) {
        m_ChatList[key]->setNewGraph();
        m_ChatList[key]->copyEdge(pP2Psta->edge);
        m_ChatList[key]->w_selectionAlert = true;

        return true;
    }

    if (m_ChatList[key]->isNewEdge(pP2Psta->edge) == true) {
        m_ChatList[key]->copyEdge(pP2Psta->edge);
        m_ChatList[key]->w_newLinkState = true;
    }

    return true;
}

