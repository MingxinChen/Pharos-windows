#include "connectioncs.h"
using namespace std;

extern ClientInfo* myClient;
extern QTextBrowser* infoShell;

// 登陆，回复log界面是否可以跳转
string conn_login(string userId, string password) {
    // 联系服务器
    char username[MAX_USERNAME];
    strcpy(username, userId.c_str());
    if(false == myClient->Login(username, "118.89.25.104")) {
        return "Login false!";
    }
    return "pass";
}

// 注册，回复reg界面
string conn_register(string userId, string password) {
    return "wrong";
}
