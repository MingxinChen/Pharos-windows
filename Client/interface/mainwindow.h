#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <deque>
#include <QWidget>
#include "function/connectioncs.h"
#include "function/system.h"
#include "addresslistitem.h"
#include "userinfocell.h"
#include "chatcell.h"
#include "settingcell.h"
#include "addfriendcell.h"
#include "addchatcell.h"
#include "informcell.h"
#include "dataType/clientinfo.h"
#include <queue>
#include <typeinfo>
using namespace std;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void addIntoFriendAddressList(Friend_Info* user, bool fouce);
    void addIntoRoomAddressList(Chat_Info* room, bool fouce);

    void deleteFriend(string username);
    void deleteRoom(string roomid);

private:
    Ui::MainWindow *ui;
    deque<QWidget*> desk;

    void interfaceInitial();
    //sidebar address list
    void showAddressList();
    //manage desk
    void newPage(QWidget* newpage);


private slots:
    //login and register
    void receiveLogin();
    void receiveRegister();
    //message alert
    void receiveMessage(Chat_Info* room, char* username, char* message);
    void receiveInform(string type, string message);
    //buttons
    void on_friendButton_clicked();
    void on_roomButton_clicked();
    void on_searchButton_clicked();
    void on_title_settingButton_clicked();
    void on_addFriendButton_clicked();
    void on_addRoomButton_clicked();
    //address click
    void roomAddressClick(QListWidgetItem*);
    void friendAddressClick(QListWidgetItem*);
    //manage desk
    void forward();
    void backward();
    void deleteCurrentPage();
    // 管理好友与房间
    void addFriend(Friend_Info* user);
    void addRoom(Chat_Info* room);
    void refreshAddressList();

    void on_title_informButton_clicked();

signals:
    void quit();
};

#endif // MAINWINDOW_H
