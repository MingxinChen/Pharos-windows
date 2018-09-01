#include "reg.h"
#include "ui_reg.h"
using namespace std;

Reg::Reg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Reg)
{
    ui->setupUi(this);
    setWindowTitle("Pharos - 注册");
}

Reg::~Reg()
{
    delete ui;
}

void Reg::receiveToRegister() {
    this->show();
}

void Reg::on_reg_loginButton_clicked() {
    this->hide();
    emit showLogwindow();
}

void Reg::on_reg_registerButton_clicked() {
    this->hide();
    string userId = ui->reg_userId->text().toStdString();
    string password = ui->reg_password->text().toStdString();

    string message = conn_register(userId, password);
    if(message.compare("pass") == 0) {
        emit showMainwindow();
    }
    else {
        this->show();
        QMessageBox::information(NULL, tr("错误"), tr(message.c_str()));
    }
}
