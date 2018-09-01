#ifndef SETTINGCELL_H
#define SETTINGCELL_H

#include <QWidget>
#include "dataType/ClientInfo.h"
#include "function/connectioncs.h"
#include "function/system.h"
#include <qdebug.h>
using namespace std;
namespace Ui {
class SettingCell;
}

class SettingCell : public QWidget
{
    Q_OBJECT

public:
    explicit SettingCell(QWidget *parent = 0);
    ~SettingCell();

private slots:
    void on_setting_save_clicked();
    void on_setting_disappear_clicked();
    void on_setting_clean_clicked();

private:
    Ui::SettingCell *ui;
};

#endif // SETTINGCELL_H
