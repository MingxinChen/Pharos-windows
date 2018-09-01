#ifndef ADDFRIENDCELL_H
#define ADDFRIENDCELL_H

#include <QWidget>
#include "dataType/ClientInfo.h"
using namespace std;

namespace Ui {
class AddFriendCell;
}

class AddFriendCell : public QWidget
{
    Q_OBJECT

public:
    explicit AddFriendCell(QWidget *parent = 0);
    ~AddFriendCell();

private slots:
    void on_addF_addButton_clicked();

private:
    Ui::AddFriendCell *ui;
};

#endif // ADDFRIENDCELL_H
