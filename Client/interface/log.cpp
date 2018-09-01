#include "log.h"
#include "ui_log.h"
using namespace std;

Log::Log(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Log)
{
    ui->setupUi(this);
    setWindowTitle("Pharos - 登陆");
}

Log::~Log()
{
    delete ui;
}

// 接受跳转到聊天窗口的信号
void Log::receiveToLogin() {
    this->show();
}

// 点击登陆按钮
void Log::on_log_loginButton_clicked() {
    this->hide();
    string userId = ui->log_userId->text().toStdString();
    string password = ui->log_password->text().toStdString();

    // 处理登陆结果
    string message = conn_login(userId, password);
    if(message.compare("pass") == 0) {
        emit showMainwindow();
    }
    else {
        this->show();
        QMessageBox::information(NULL, tr("错误"), tr(message.c_str()));
    }
}

// 点击注册按钮
void Log::on_log_registerButton_clicked() {
    this->hide();
    emit showRegwindow();
}
