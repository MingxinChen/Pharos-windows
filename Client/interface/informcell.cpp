#include "informcell.h"
#include "ui_informcell.h"

extern ClientInfo* myClient;

InformCell::InformCell(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InformCell)
{
    ui->setupUi(this);
}

InformCell::InformCell(QWidget *parent, pair<string, string> mail) :
    QDialog(parent),
    ui(new Ui::InformCell)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint);    // 取消标题栏
    this->mail = mail;

    if(mail.first == "friend") {
        ui->inform_content->insertPlainText("用户");
        ui->inform_content->insertPlainText(mail.second.c_str());
        ui->inform_content->insertPlainText("请求添加为您的好友？");
    }
    else if(mail.first == "room") {
        int pos = 0;
        mail.second += " ";
        string room = getInputElement(mail.second, pos, ' ');
        string name = getInputElement(mail.second, ++pos, ' ');

        ui->inform_content->insertPlainText("用户");
        ui->inform_content->insertPlainText(name.c_str());
        ui->inform_content->insertPlainText("询问是否加入房间");
        ui->inform_content->insertPlainText(room.c_str());
        ui->inform_content->insertPlainText("？");
    }
}

InformCell::~InformCell()
{
    delete ui;
}

void InformCell::on_inform_confirm_clicked()
{
    if(mail.first == "friend") {
        string command = "rpok " + mail.second;
        myClient->ProcCommand(command);
    }
    else if(mail.first == "room") {
        string command = "jiok " + mail.second;
        myClient->ProcCommand(command);
    }
    this->hide();
}

void InformCell::on_inform_refuse_clicked()
{
    if(mail.first == "friend") {
        string command = "rpno " + mail.second;
        myClient->ProcCommand(command);
    }
    else if(mail.first == "room") {
        string command = "jino " + mail.second;
        myClient->ProcCommand(command);
    }
    this->hide();
}
