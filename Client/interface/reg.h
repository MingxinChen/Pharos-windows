#ifndef REG_H
#define REG_H

#include <QDialog>
#include "function/connectioncs.h"
#include "log.h"
#include "mainwindow.h"
using namespace std;

namespace Ui {
class Reg;
}

class Reg : public QDialog
{
    Q_OBJECT

public:
    explicit Reg(QWidget *parent = 0);
    ~Reg();

private:
    Ui::Reg *ui;

private slots:
    void receiveToRegister();
    void on_reg_registerButton_clicked();
    void on_reg_loginButton_clicked();

signals:
    void showMainwindow();
    void showLogwindow();
    void quit();
};

#endif // REG_H
