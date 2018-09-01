#include "settingcell.h"
#include "ui_settingcell.h"

extern ClientInfo* myClient;

SettingCell::SettingCell(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SettingCell)
{
    ui->setupUi(this);

    // 用户名
    ui->setting_username->setText(myClient->m_PeerInfo.szUserName);

    // 密码
    ui->setting_password->setText(myClient->password.c_str());

    // ip地址
    int nAddrNum =  myClient->m_PeerInfo.nAddrNum;
    in_addr LoginAddr;
    LoginAddr.S_un.S_addr = myClient->m_PeerInfo.IPAddr[nAddrNum].dwIP;
    string text = "IP " + string(::inet_ntoa(LoginAddr)) + " : " + QString::number(ntohs(myClient->m_PeerInfo.IPAddr[nAddrNum].usPort)).toStdString();
    ui->setting_ip->setText(text.c_str());

    // 头像
    QPixmap header(myClient->headerPath.c_str());
    ui->setting_header->setPixmap(header);
    ui->setting_header->setScaledContents(true);

}

SettingCell::~SettingCell()
{
    delete ui;
}

// 点击保存修改
void SettingCell::on_setting_save_clicked()
{

}

// 点击销号
void SettingCell::on_setting_disappear_clicked()
{

}

// 点击清空
void SettingCell::on_setting_clean_clicked()
{
    uninstall();
}
