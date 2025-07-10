#ifndef LOGIN_H
#define LOGIN_H

#include <QWidget>
#include <QTcpSocket>
#include <QJsonObject>
#include <QJsonDocument>
#include <QMessageBox>

#include "client.h"
#include "ui_client.h"


namespace Ui {
class login;
}

class login : public QWidget
{
    Q_OBJECT

public:
    explicit login(QWidget *parent = 0);
    ~login();

private slots:
    void read_handle();     //读数据槽函数

    void on_register_button_clicked();

    void on_login_button_clicked();

private:
    Ui::login *ui;
    QTcpSocket *client_socket;  //登录套接字
    client *smart_home_client;  //客户端主界面类对象创建

    char read_buf[100];

};

#endif // LOGIN_H
