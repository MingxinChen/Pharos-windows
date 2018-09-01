#ifndef ADDRESSLISTITEM_H
#define ADDRESSLISTITEM_H

#include <QWidget>
#include <string>
#include "dataType/FriendInfo.h"
#include "dataType/ChatInfo.h"
#include "dataType/ClientInfo.h"
using namespace std;

namespace Ui {
class addressListItem;
}

class addressListItem : public QWidget
{
    Q_OBJECT

public:
    explicit addressListItem(QWidget *parent = 0);
    addressListItem(QWidget *parent, Friend_Info* user);
    addressListItem(QWidget *parent, Chat_Info* room);
    ~addressListItem();

    Ui::addressListItem *ui;
};

#endif // ADDRESSLISTITEM_H
