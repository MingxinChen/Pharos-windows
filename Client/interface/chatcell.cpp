#include "chatcell.h"
#include "ui_chatcell.h"

extern ClientInfo* myClient;

ChatCell::ChatCell(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChatCell)
{
    ui->setupUi(this);
}

ChatCell::ChatCell(QWidget *parent, Chat_Info *room) :
    QWidget(parent),
    ui(new Ui::ChatCell)
{
    ui->setupUi(this);
    this->roomId = room->roomId;
    this->setToolTip(QString::fromStdString(roomId));

    // 房间名
    ui->chat_object->setText(room->getName().c_str());
    // 房间信息
    //ui->chat_information->insertPlainText(room->getStatue().c_str());
    // 聊天记录
    /*
    11111111111111111111111111111111111111111111111111111111111
    */

}

ChatCell::~ChatCell()
{
    delete ui;
}

// 显示一条聊天记录
void ChatCell::showMessage(string username, string message) {
    QDateTime datetime = QDateTime::currentDateTime();
    ui->recordList->insertPlainText(QString::fromStdString(username)
                                    + " "
                                    + datetime.toString("yyyy-MM-dd hh:mm:ss")
                                    + "\n"
                                    + QString::fromStdString(message)
                                    + QString::fromStdString("\n\n"));
    ui->recordList->moveCursor(QTextCursor::End);
}

// 点击发送按钮
void ChatCell::on_chat_sendButton_clicked()
{
    string message = ui->chat_messageInput->toPlainText().toStdString();
    if(message == "") {
        return;
    }
    myClient->ProcCommand("chat " + roomId + " " + message);
    ui->chat_messageInput->clear();
}

void ChatCell::on_chat_breakButton_clicked()
{
    /*// 作为客人
    if(myClient->m_bParent_RoomSet.find(roomId) == myClient->m_bParent_RoomSet.end()) {
        myClient->ProcCommand("leav " + roomId);
    }
    // 作为主人
    else {
        myClient->ProcCommand("tran " + roomId);
    }*/
}

void ChatCell::on_chat_refreshButton_clicked()
{
    /*if(myClient->m_ChatList[roomId]->online == true) {
        ui->chat_information->insertPlainText("房间已在活跃状态");
        return;
    }

    // 如果是静默的房间记录，作为Parent重建
    string command = "wake ";
    string namelist = " ";
    map<string, char>::iterator i;
    for(i=myClient->m_ChatList[roomId]->status.begin();i!=myClient->m_ChatList[roomId]->status.end();i++) {
        namelist = namelist + i->first;
    }
    command = command + ("" + myClient->m_ChatList[roomId]->roomSize) + namelist;

    myClient->ProcCommand(command);*/
}

void ChatCell::on_chat_exitButton_clicked()
{
    /*// 作为客人
    if(myClient->m_bParent_RoomSet.find(roomId) == myClient->m_bParent_RoomSet.end()) {
        myClient->ProcCommand("quit " + roomId);
    }
    // 作为主人
    else {
        if(myClient->ProcCommand("tran " + roomId)) {
            myClient->ProcCommand("quit " + roomId);
        }
    }*/
}

void ChatCell::on_chat_enterButton_clicked()
{
    /*if(myClient->m_ChatList[roomId]->online == false) {
        ui->chat_information->insertPlainText("房间不在活跃状态，请重建房间");
        return;
    }

    if(myClient->m_bParent_RoomSet.find(roomId) == myClient->m_bParent_RoomSet.end()) {
        myClient->ProcCommand("ente " + roomId);
    }*/
}
