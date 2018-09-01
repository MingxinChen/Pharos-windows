#ifndef ADDCHATCELL_H
#define ADDCHATCELL_H

#include <QWidget>
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <QVBoxLayout>
#include <set>
#include "dataType/ClientInfo.h"
#include "function/system.h"
using namespace std;

namespace Ui {
class AddChatCell;
}

class AddChatCell : public QWidget
{
    Q_OBJECT

public:
    explicit AddChatCell(QWidget *parent = 0);
    ~AddChatCell();

private slots:
    void on_addC_addButton_clicked();

private:
    Ui::AddChatCell *ui;
    QButtonGroup* buttongroup;
};

#endif // ADDCHATCELL_H
