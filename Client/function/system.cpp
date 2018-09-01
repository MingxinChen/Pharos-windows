#include "system.h"
using namespace std;

extern ClientInfo* myClient;

void uninstall() {
    qDebug("uninstall, unfinished");
}

bool deleteFriendRecordFile(string username) {
    return true;
}

bool deleteRoomRecordFile(string roomId) {
    return true;
}

bool addFriendRecordFile(Friend_Info* user) {
    return true;
}

bool addRoomRecordFile(Chat_Info* room) {
    return true;
}

bool getFriendAddressList() {
    myClient->m_FriendList.clear();

    myClient->m_FriendList["Bobi"] = new Friend_Info("Bobi", false);
    myClient->m_FriendList["麦克"] = new Friend_Info("麦克", false);
    myClient->m_FriendList["酥皮"] = new Friend_Info("酥皮", false);
    myClient->m_FriendList["Bruice"] = new Friend_Info("Bruice", false);
    myClient->m_FriendList["Sia"] = new Friend_Info("Sia", false);
    myClient->m_FriendList["提"] = new Friend_Info("提", false);
    myClient->m_FriendList["二桶"] = new Friend_Info("二桶", false);
    myClient->m_FriendList["Heros"] = new Friend_Info("Heros", false);
    myClient->m_FriendList["Diana"] = new Friend_Info("Diana", false);
    myClient->m_FriendList["Cap"] = new Friend_Info("Cap", false);
    myClient->m_FriendList["Bucky冬"] = new Friend_Info("Bucky冬", false);

    return true;
}

bool getRoomAddressList() {
    myClient->m_ChatList.clear();

    set<char*> roommates;
    roommates.insert("Bobi蹲");
    roommates.insert("Grayson");
    roommates.insert("Bucky冬");
    myClient->m_ChatList["8483"] = new Chat_Info("8483", "Bobi蹲", roommates, 3, 'F', QDateTime::fromString("2017/12/21 11:15:30", "yyyy/mm/dd hh:mm:ss"));

    roommates.clear();
    roommates.insert("酥皮");
    roommates.insert("Grayson");
    roommates.insert("Cap");
    myClient->m_ChatList["8586"] = new Chat_Info("8586", "Grayson", roommates, 3, 'F', QDateTime::fromString("2017/12/23 09:07:30", "yyyy/mm/dd hh:mm:ss"));

    return true;
}

void static EraseNewLine(char szChar[])
{
    for (int i = 0; i < strlen(szChar); ++i)
    {
        if (szChar[i] == '\n')
        {
            szChar[i] = '\0';
            break;
        }
    }
}

string getInputElement(string szCommandLine, int& pos, char end)
{
    string element = "";
    for (int i = pos; i < pos + MAX_USERNAME; ++i) {
        char c = szCommandLine[i];
        if (end != c) {
            char temp[2];
            temp[0] = c;
            temp[1] = '\0';
            element = element + string(temp);
        }
        else {
            pos = i;
            return element;
        }
    }
}

bool myPrintf(char* form,... )
{
    va_list args;
    va_start(args,form);

    string sform = string(form);
    qDebug(sform.c_str());
    int start = 0;
    int end = sform.find("%", start);
    while (end != string::npos) {
        string temp = sform.substr(start, end - start - 1);
        qDebug(temp.c_str());

        end++;
        if(form[end] == 's') {
            char* tmp = va_arg(args,char*);
            qDebug(tmp);
        }
        else if(form[end] == 'd') {
            int tmp = va_arg(args,int);
            qDebug(QString::number(tmp).toStdString().c_str());
        }

        start = end + 1;
        end = sform.find("%", start);
    }
    va_end(args);
    return true;
}
