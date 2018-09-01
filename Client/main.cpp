#include "interface/mainwindow.h"
#include "interface/log.h"
#include "interface/reg.h"
#include "dataType/clientinfo.h"
#include <QApplication>
#include <winsock.h>
#include <windows.h>
#include <queue>
using namespace std;

queue<pair<string, string>> mailBox;
MainWindow* mainwindowPointer;
ClientInfo* myClient;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qRegisterMetaType<string>("string");

    Log logwindow;
    Reg regwindow;
    MainWindow mainwindow;
    mainwindowPointer = &mainwindow;
    ClientInfo client;
    myClient = &client;

    logwindow.show();

    QObject::connect(&logwindow, SIGNAL(showMainwindow()), &mainwindow, SLOT(receiveLogin()));
    QObject::connect(&logwindow, SIGNAL(showRegwindow()), &regwindow, SLOT(receiveToRegister()));

    QObject::connect(&regwindow, SIGNAL(showMainwindow()), &mainwindow, SLOT(receiveRegister()));
    QObject::connect(&regwindow, SIGNAL(showLogwindow()), &logwindow, SLOT(receiveToLogin()));

    QObject::connect(&mainwindow, SIGNAL(quit()), &a, SLOT(quit()));

    QObject::connect(&client, SIGNAL(messageAlert(Chat_Info*, char*, char*)), &mainwindow, SLOT(receiveMessage(Chat_Info*, char*, char*)));
    QObject::connect(&client, SIGNAL(informAlert(string, string)), &mainwindow, SLOT(receiveInform(string,string)));

    QObject::connect(&client, SIGNAL(NewFriendAlert(Friend_Info*)), &mainwindow, SLOT(addFriend(Friend_Info*)));
    QObject::connect(&client, SIGNAL(NewRoomAlert(Chat_Info*)), &mainwindow, SLOT(addRoom(Chat_Info*)));

    QObject::connect(&client, SIGNAL(refreshStatus()), &mainwindow, SLOT(refreshAddressList()));

    return a.exec();
}
