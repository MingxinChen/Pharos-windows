#ifndef CHATCELL_H
#define CHATCELL_H

#include <QWidget>
#include <string>
#include <QDateTime>
#include "dataType/ChatInfo.h"
#include "dataType/ClientInfo.h"
#include "mainwindow.h"
using namespace std;

namespace Ui {
class ChatCell;
}

class ChatCell : public QWidget
{
    Q_OBJECT

public:
    string roomId;

    explicit ChatCell(QWidget *parent = 0);
    ChatCell(QWidget *parent, Chat_Info *room);
    ~ChatCell();

    void showMessage(string username, string message);

private slots:
    void on_chat_sendButton_clicked();
    void on_chat_breakButton_clicked();
    void on_chat_refreshButton_clicked();
    void on_chat_exitButton_clicked();

    void on_chat_enterButton_clicked();

private:
    Ui::ChatCell *ui;
};

#endif // CHATCELL_H
