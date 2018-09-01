#ifndef LOG_H
#define LOG_H

#include <QDialog>
#include "function/connectioncs.h"
#include "reg.h"
#include "mainwindow.h"
using namespace std;

namespace Ui {
class Log;
}

class Log : public QDialog
{
    Q_OBJECT

public:
    explicit Log(QWidget *parent = 0);
    ~Log();

private:
    Ui::Log *ui;

private slots:
    void receiveToLogin();
    void on_log_loginButton_clicked();
    void on_log_registerButton_clicked();

signals:
    void showMainwindow();
    void showRegwindow();
    void quit();
};

#endif // LOG_H
