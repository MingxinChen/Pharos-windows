#ifndef CONNECTIONCS_H
#define CONNECTIONCS_H

#include <string>
#include <list>
#include <qdebug.h>
#include "dataType/clientinfo.h"
#include "system.h"
#include <QMessageBox>
#include <qtextbrowser.h>
#include <set>
using namespace std;

string conn_login(string username, string password);        // 连接界面与通信，处理登陆事件
string conn_register(string username, string password);     // 处理注册事件
#endif // CONNECTIONCS_H
