#include "addfriendcell.h"
#include "ui_addfriendcell.h"
#include "dataType/clientinfo.h"

extern ClientInfo* myClient;

AddFriendCell::AddFriendCell(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AddFriendCell)
{
    ui->setupUi(this);
}

AddFriendCell::~AddFriendCell()
{
    delete ui;
}

// 点击添加按钮
void AddFriendCell::on_addF_addButton_clicked()
{
    string command = "addf ";
    command = command + ui->addF_idInput->text().toStdString();
    myClient->ProcCommand(command);
}
