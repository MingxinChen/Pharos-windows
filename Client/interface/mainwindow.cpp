#include "mainwindow.h"
#include "ui_mainwindow.h"

extern ClientInfo* myClient;
extern queue<pair<string, string>> mailBox;
using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //this->setWindowFlags(Qt::FramelessWindowHint);    // 取消标题栏
    setWindowTitle("Pharos");
    // 没有新消息，隐藏消息提示
    ui->title_informButton->setVisible(false);

    // 连接侧边栏按钮和槽
    QObject::connect(ui->roomAddressList, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(roomAddressClick(QListWidgetItem*)));
    QObject::connect(ui->friendAddressList,SIGNAL(itemClicked(QListWidgetItem*)),this,SLOT(friendAddressClick(QListWidgetItem*)));
    // 连接标题栏按钮和槽
    QObject::connect(ui->title_exitButton, SIGNAL(clicked(bool)), this, SLOT(close()));
    QObject::connect(ui->title_backwardButton, SIGNAL(clicked(bool)), this, SLOT(backward()));
    QObject::connect(ui->title_forwardButton, SIGNAL(clicked(bool)), this, SLOT(forward()));
    // 连接回车键和搜索函数
    QObject::connect(ui->searchInput, SIGNAL(returnPressed()), this, SLOT(on_searchButton_clicked()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

// 登陆成功，初始化mainwindow
void MainWindow::receiveLogin() {
    this->show();
    interfaceInitial();
}

// 注册成功，初始化mainwindow
void MainWindow::receiveRegister() {
    this->show();
    interfaceInitial();
}

// 初始化
void MainWindow::interfaceInitial() {
    // 设置名字
    ui->side_clientName->setText(QString::fromStdString(myClient->m_PeerInfo.szUserName));

    // 获取好友列表，初始化m_FriendList
    getFriendAddressList();

    // 获取房间列表，初始化m_ChatList
    getRoomAddressList();

    // 修改状态
    myClient->ProcCommand("getu");

    // 显示friendAddressList和roomAddressList侧边栏
    showAddressList();
    ui->roomAddressList->hide();
    ui->friendAddressList->show();

    // 显示setting页面
    ui->title_settingButton->click();
}

// 得到一条新消息
void MainWindow::receiveMessage(Chat_Info* room, char* username, char* message) {
    deque<QWidget*>::iterator i;
    int pos = 1;
    for(i=desk.begin();i!=desk.end();i++, pos++) {
       if((*i)->toolTip().toStdString() == room->roomId) {
           while(pos != 1) {
               backward();
               pos--;
           }
           ChatCell* temp = (ChatCell*)(*i);
           temp->showMessage(string(username), string(message));
           return;
       }
    }
    ChatCell* newChat = new ChatCell(ui->desk, room);
    newChat->showMessage(string(username), string(message));
    newPage(newChat);
}

// 得到一个新通知（添加好友请求或者房间建立请求）
void MainWindow::receiveInform(string type, string message) {
    // 设置消息提示
    ui->title_informButton->setVisible(true);
    pair<string, string> newMail(type, message);
    mailBox.push(newMail);
}

// 点击标题栏消息提示
void MainWindow::on_title_informButton_clicked()
{
    if(!mailBox.empty()) {
        pair<string, string> newMail = mailBox.front();
        mailBox.pop();

        InformCell* informcell = new InformCell(ui->desk, newMail);
        informcell->move((ui->desk->width() - informcell->width()) / 2,(ui->desk->height() - informcell->height()) / 2);
        informcell->show();
    }

    // mailBox清空则隐藏消息提示
    if(mailBox.empty()) {
        ui->title_informButton->setVisible(false);
    }
}

// 点击标题栏设置按钮
void MainWindow::on_title_settingButton_clicked() {
    SettingCell* newpage = new SettingCell(ui->desk);
    newPage(newpage);
}

// 点击侧边栏显示friendAddressList按钮
void MainWindow::on_friendButton_clicked() {
    ui->roomAddressList->hide();
    ui->friendAddressList->show();
}

// 点击侧边栏显示roomAddressList按钮
void MainWindow::on_roomButton_clicked() {
    ui->roomAddressList->show();
    ui->friendAddressList->hide();
}

// 点击侧边栏添加好友按钮
void MainWindow::on_addFriendButton_clicked() {
    AddFriendCell* newpage = new AddFriendCell(ui->desk);
    newPage(newpage);
}

// 点击侧边栏添加房间按钮
void MainWindow::on_addRoomButton_clicked() {
    // 修改好友状态，重置addressList
    myClient->ProcCommand("getu");
    showAddressList();
    // 显示添加房间页面
    AddChatCell* newpage = new AddChatCell(ui->desk);
    newPage(newpage);
}

// 点击搜索按钮或者在搜索栏focus时点回车
void MainWindow::on_searchButton_clicked()
{
    string key = ui->searchInput->text().toStdString();

    // 搜索用户
    if(ui->roomAddressList->isHidden()) {
        map<string, Friend_Info*>::iterator i;
        int row = 0;
        for(i=myClient->m_FriendList.begin();i!=myClient->m_FriendList.end();i++, row++) {
            unsigned int pos = i->second->username.find(key);
            if(pos != string::npos) {
                break;
            }
        }
        // 找到了用户
        if(i != myClient->m_FriendList.end()) {
            // 根据用户的行数设置focus
            QListWidgetItem* item = ui->friendAddressList->item(row);
            ui->friendAddressList->setCurrentItem(item);
            // 显示用户信息页
            UserInfoCell* userInfoCell = new UserInfoCell(ui->desk, i->second);
            newPage(userInfoCell);
        }
        else {
            QMessageBox::information(NULL, tr("提示"), tr("找不到相关用户！"));
        }
    }
    // 搜索房间
    else {
        map<string, Chat_Info*>::iterator i;
        int row = 0;
        for(i=myClient->m_ChatList.begin();i!=myClient->m_ChatList.end();i++, row++) {
            unsigned int pos = i->second->getName().find(key);
            if(pos != string::npos) {
                break;
            }
        }
        // 找到了房间
        if(i != myClient->m_ChatList.end()) {
            // 设置focus
            QListWidgetItem* item = ui->roomAddressList->item(row);
            ui->roomAddressList->setCurrentItem(item);
            // 显示聊天页面
            ChatCell* chatCellTest = new ChatCell(ui->desk, i->second);
            newPage(chatCellTest);
        }
        else {
            QMessageBox::information(NULL, tr("提示"), tr("找不到相关聊天！"));
        }
    }
}

// 点击用户
void MainWindow::friendAddressClick(QListWidgetItem* item) {
    string key = item->toolTip().toStdString();
    UserInfoCell* userInfoCell = new UserInfoCell(ui->desk, myClient->m_FriendList[key]);
    newPage(userInfoCell);
}

// 点击房间
void MainWindow::roomAddressClick(QListWidgetItem* item) {
    string key = item->toolTip().toStdString();
    deque<QWidget*>::iterator i;
    int pos = 1;

    for(i=desk.begin();i!=desk.end();i++, pos++) {
       if((*i)->toolTip().toStdString() == key) {
           while(pos != 1) {
               backward();
               pos--;
           }
           return;
       }
    }

    ChatCell* chatCellTest = new ChatCell(ui->desk, myClient->m_ChatList[key]);
    newPage(chatCellTest);
}

// 刷新addresslist
void MainWindow::refreshAddressList()
{
    showAddressList();
}

// 显示addressList
void MainWindow::showAddressList() {
    // 取消滚动条
    ui->friendAddressList->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->roomAddressList->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // 清空
    ui->friendAddressList->clear();
    ui->roomAddressList->clear();

    // 好友
    map<string, Friend_Info*>::iterator i;
    for(i=myClient->m_FriendList.begin();i!=myClient->m_FriendList.end();i++) {
        addIntoFriendAddressList(i->second, false);
    }

    // 房间
    map<string, Chat_Info*>::iterator j;
    for(j=myClient->m_ChatList.begin();j!=myClient->m_ChatList.end();j++) {
        addIntoRoomAddressList(j->second, false);
    }
}

// 向侧边栏添加一项好友item
void MainWindow::addIntoFriendAddressList(Friend_Info* user, bool fouce) {
    addressListItem* newFriend = new addressListItem(0, user);

    QListWidgetItem* newitem = new QListWidgetItem(ui->friendAddressList);
    newitem->setToolTip(user->username.c_str());      // tooltip为id
    ui->friendAddressList->addItem(newitem);
    ui->friendAddressList->setItemWidget(newitem, newFriend);
    newitem->setSizeHint(QSize(0, 60));

    // 显示friendAddressList并focus新添加的item
    if(fouce) {
        ui->friendButton->click();
        ui->friendAddressList->setCurrentItem(newitem);
    }
}

// 向侧边栏添加一项房间item
void MainWindow::addIntoRoomAddressList(Chat_Info* room, bool fouce) {
    addressListItem* newRoom = new addressListItem(0, room);

    QListWidgetItem* newitem = new QListWidgetItem(ui->roomAddressList);
    newitem->setToolTip(room->roomId.c_str());      // tooltip为id
    ui->roomAddressList->addItem(newitem);
    ui->roomAddressList->setItemWidget(newitem, newRoom);
    newitem->setSizeHint(QSize(0, 60));

    // 显示roomAddressList并focus新添加的item
    if(fouce) {
        ui->roomButton->click();
        ui->roomAddressList->setCurrentItem(newitem);
    }
}

// 添加好友
void MainWindow::addFriend(Friend_Info* user) {
    addIntoFriendAddressList(user, true);
    addFriendRecordFile(user);
}

// 删除好友
void MainWindow::deleteFriend(string username) {
    // 找到user在friendList中的序号
    map<string, Friend_Info*>::iterator i;
    int row = 0;
    for(i=myClient->m_FriendList.begin();i!=myClient->m_FriendList.end();i++, row++) {
        if(i->second->username == username) {
            break;
        }
    }
    // 从侧边栏删除
    QListWidgetItem* item = ui->friendAddressList->item(row);
    ui->friendAddressList->removeItemWidget(item);
    // 从friendList删除
    myClient->m_FriendList.erase(username);
    // 从文件系统删除
    deleteFriendRecordFile(username);

    // 刷新addressList
    showAddressList();
    // 删除当前页
    deleteCurrentPage();
}

// 添加房间
void MainWindow::addRoom(Chat_Info* room)
{
    addIntoRoomAddressList(room, true);
    addRoomRecordFile(room);
}

// 删除房间
void MainWindow::deleteRoom(string roomid) {
    // 找到room在chatList中的序号
    map<string, Chat_Info*>::iterator i;
    int row = 0;
    for(i=myClient->m_ChatList.begin();i!=myClient->m_ChatList.end();i++, row++) {
        if(i->second->roomId == roomid) {
            break;
        }
    }
    // 从侧边栏删除
    QListWidgetItem* item = ui->roomAddressList->item(row);
    ui->roomAddressList->removeItemWidget(item);
    // 从roomList删除
    myClient->m_ChatList.erase(roomid);
    // 从文件系统删除
    deleteRoomRecordFile(roomid);

    // 刷新addressList
    showAddressList();
    // 删除当前页
    deleteCurrentPage();
}

// 新添加页面
void MainWindow::newPage(QWidget *newpage) {
    if(!desk.empty()) {
        QWidget* lastpage = desk.front();
        lastpage->hide();
    }
    newpage->show();
    desk.push_front(newpage);

    //too much widget
    if(desk.size() > 10) {
        desk.pop_back();
    }
}

// 向前一页
void MainWindow::forward() {
    QWidget* lastpage = desk.front();
    lastpage->hide();
    QWidget* newpage = desk.back();
    desk.pop_back();
    newpage->show();
    desk.push_front(newpage);
}

// 后退一页
void MainWindow::backward() {
    QWidget* lastpage = desk.front();
    desk.pop_front();
    lastpage->hide();
    desk.push_back(lastpage);
    QWidget* newpage = desk.front();
    newpage->show();
}

// 后退并删除当前页
void MainWindow::deleteCurrentPage() {
    QWidget* lastpage = desk.front();
    desk.pop_front();
    lastpage->hide();
    QWidget* newpage = desk.front();
    newpage->show();
}


