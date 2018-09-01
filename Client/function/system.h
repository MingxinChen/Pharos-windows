#ifndef SYSTEM_H
#define SYSTEM_H

#include <qdebug.h>
#include <QFile>
#include <QDataStream>
#include <QTextCodec>
#include <stdio.h>
#include "stdlib.h"
#include <stdarg.h>
#include "dataType/ChatInfo.h"
#include "dataType/FriendInfo.h"
#include "dataType/ClientInfo.h"
using namespace std;

void uninstall();   // 消除痕迹

bool deleteRoomRecordFile(string roomId);       // 文件处理，删除房间记录
bool deleteFriendRecordFile(string username);     // 删除好友记录
bool addRoomRecordFile(Chat_Info* room);         // 添加房间记录
bool addFriendRecordFile(Friend_Info* user);       // 添加好友记录

bool getFriendAddressList();          // 获取文件中的好友列表
bool getRoomAddressList();            // 获取房间列表

string getInputElement(string szCommandLine, int& pos, char end);

bool myPrintf(char* form,... );
#endif // SYSTEM_H
