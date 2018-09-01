#include "addresslistitem.h"
#include "ui_addresslistitem.h"
using namespace std;

extern ClientInfo* myClient;

addressListItem::addressListItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::addressListItem)
{
    ui->setupUi(this);
}

addressListItem::addressListItem(QWidget *parent, Friend_Info* user) :
    QWidget(parent),
    ui(new Ui::addressListItem)
{
    ui->setupUi(this);
    // 用户名
    ui->add_username->setText(user->username.c_str());
    // 头像
    QPixmap header(user->headerPath.c_str());
    ui->add_header->setPixmap(header);
    ui->add_header->setScaledContents(true);
    // 状态
    char name[MAX_USERNAME];
    strcpy(name, user->username.c_str());
    Peer_Info* peer = myClient->m_PeerList.GetAPeer(name);
    if(peer != NULL) {
        int nAddrNum =  peer->nAddrNum;
        in_addr LoginAddr;
        LoginAddr.S_un.S_addr = peer->IPAddr[nAddrNum].dwIP;
        string text = "[在线] IP " + string(::inet_ntoa(LoginAddr)) + " : " + QString::number(ntohs(peer->IPAddr[nAddrNum].usPort)).toStdString();
        ui->add_statue->setText(text.c_str());
    }
    else {
        string text = "[离线]";
        ui->add_statue->setText(text.c_str());
    }
}

addressListItem::addressListItem(QWidget *parent, Chat_Info* room) :
    QWidget(parent),
    ui(new Ui::addressListItem)
{
    ui->setupUi(this);
    // 房间名
    ui->add_username->setText(room->getName().c_str());
    // 头像
    QPixmap header(room->headerPath.c_str());
    ui->add_header->setPixmap(header);
    ui->add_header->setScaledContents(true);
    // 状态
    ui->add_statue->setText(room->getListItemMsg().c_str());
}

addressListItem::~addressListItem()
{
    delete ui;
}
