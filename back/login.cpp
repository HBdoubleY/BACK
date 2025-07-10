#include "login.h"
#include "ui_login.h"

login::login(QWidget *parent) : QWidget(parent), ui(new Ui::login){
    ui->setupUi(this);

    //设置窗口
    this->setWindowTitle("第九组 智能管家");
    this->setWindowIcon(QIcon(":/icon/智能机器人.png"));
    //this->setWindowFlags(this->windowFlags() | Qt::FramelessWindowHint);
    QPalette back = this->palette();
    QImage backPic(":/icon/login.jpeg");
    QImage fitPic = backPic.scaled(this->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    back.setBrush(QPalette::Window, QBrush(fitPic));
    this->setPalette(back);

    /*套接字创建*/
    client_socket = new QTcpSocket(this);
    client_socket->connectToHost("192.168.6.222",7777);

    connect(client_socket, SIGNAL(readyRead()), this, SLOT(read_handle()));
}

login::~login(){
    delete ui;
}

//接收登录返回信息
void login::read_handle(){
    memset(read_buf, 0, sizeof(read_buf));
    client_socket->read(read_buf, 100);
    if(0 == strncmp(read_buf,"passwd mistake!", 15))    QMessageBox::question(this, "passwd mistake!", "plese input your name and passwd again", 0);
    if(0 == strncmp(read_buf,"username no exsit!", 18))    QMessageBox::question(this, "username no exsit!", "plese input your name and passwd again", 0);
    if(0 == strncmp(read_buf,"register successful!", 20))    QMessageBox::question(this, "register successful!", "welcome!welcome to use", 0);
    if(0 == strncmp(read_buf,"this username already exsited!", 30))    QMessageBox::question(this, "register defeated!", "this username already exsited!", 0);
    //进行跳转操作
    if(0 == strncmp(read_buf,"login successful!", 19)){
        /*客户端主窗口创建*/
        smart_home_client = new client;
        smart_home_client->show();
        this->hide();
    }
}

void login::on_register_button_clicked()
{
    QString name_str;
    QString password_str;
    name_str = ui->username_line->text();
    password_str = ui->password_line->text();
    QJsonObject json;
    json.insert("type", "register");
    json.insert("username", name_str);
    json.insert("password", password_str);
    // 构建 JSON 文档
    QJsonDocument document;
    document.setObject(json);
    QByteArray byteArray = document.toJson(QJsonDocument::Compact);
    QString strJson(byteArray);
    client_socket->write(strJson.toStdString().c_str(), 100);
}

void login::on_login_button_clicked()
{
    QString name_str;
    QString password_str;
    name_str = ui->username_line->text();
    password_str = ui->password_line->text();
    QJsonObject json;
    json.insert("type", "login");
    json.insert("username", name_str);
    json.insert("password", password_str);
    // 构建 JSON 文档
    QJsonDocument document;
    document.setObject(json);
    QByteArray byteArray = document.toJson(QJsonDocument::Compact);
    QString strJson(byteArray);
    client_socket->write(strJson.toStdString().c_str(), 100);
}
