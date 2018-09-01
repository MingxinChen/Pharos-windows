#ifndef FRIEND_INFO_H
#define FRIEND_INFO_H

#include <QDateTime>
#include <string>
#include <ctime>
using namespace std;

class Friend_Info
{
public:
    string      username;        // 用户名
    string      headerPath;      // 用户随机头像
    bool        online;          // 是否在线

    Friend_Info();
    Friend_Info(char* szusername, bool on);

    string getListItemMsg();

};

#endif // FRIEND_INFO_H
