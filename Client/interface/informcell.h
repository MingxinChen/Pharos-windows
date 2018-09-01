#ifndef INFORMCELL_H
#define INFORMCELL_H

#include <QDialog>
#include <string>
#include "dataType/ClientInfo.h"
#include "function/system.h"
using namespace std;

namespace Ui {
class InformCell;
}

class InformCell : public QDialog
{
    Q_OBJECT

public:
    pair<string, string> mail;

    explicit InformCell(QWidget *parent = 0);
    InformCell(QWidget *parent, pair<string, string> mail);
    ~InformCell();

private slots:
    void on_inform_confirm_clicked();
    void on_inform_refuse_clicked();

private:
    Ui::InformCell *ui;
};

#endif // INFORMCELL_H
