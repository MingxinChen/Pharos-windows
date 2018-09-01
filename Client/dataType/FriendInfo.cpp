#include "FriendInfo.h"

//extern ClientInfo* myClient;
int randSeed = 1;

Friend_Info::Friend_Info()
{
}

Friend_Info::Friend_Info(char* szusername, bool on)
{
    username = string(szusername);
    online = on;

    srand((int)time(0) + randSeed);
    randSeed++;
    int hnum = rand()%12;
    headerPath = "./images/headers/" + QString::number(hnum).toStdString() + ".png";
}

string Friend_Info::getListItemMsg()
{
    string msg;
    if(online) {
        msg = "[在线]";
        /*char name[MAX_USERNAME];
        strcpy(name, username.c_str());
        Peer_Info* peer = myClient->m_PeerList.GetAPeer(name);
        if(peer != NULL) {
            int nAddrNum =  peer->nAddrNum;
            in_addr LoginAddr;
            LoginAddr.S_un.S_addr = peer->IPAddr[nAddrNum].dwIP;
            msg = "[在线]" + string(::inet_ntoa(LoginAddr)) + " : " + QString::number(ntohs(peer->IPAddr[nAddrNum].usPort)).toStdString();
        }*/
    }
    else {
        msg = "[离线]";
    }
    return msg;
}
