#ifndef USERINFOCELL_H
#define USERINFOCELL_H

#include <QWidget>
#include "dataType/ClientInfo.h"
#include "dataType/ChatInfo.h"
#include "mainwindow.h"
using namespace std;
namespace Ui {
class UserInfoCell;
}

class UserInfoCell : public QWidget
{
    Q_OBJECT

public:
    string username;

    explicit UserInfoCell(QWidget *parent = 0);
    UserInfoCell(QWidget *parent, Friend_Info *user);
    ~UserInfoCell();

private slots:
    void on_user_deleteFriendButton_clicked();

private:
    Ui::UserInfoCell *ui;

};

#endif // USERINFOCELL_H
