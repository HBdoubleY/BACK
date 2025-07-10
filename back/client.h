#ifndef CLIENT_H
#define CLIENT_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextLayout>
#include <QTcpSocket>
#include <QTimer>
#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include <QTextCodec>
#include <QByteArray>
#include <QPaintEvent>
#include <QThread>

#include "myvideo.h"
#include "mytalk.h"
#include "myserial.h"

namespace Ui {
class client;
}

class client : public QWidget
{
    Q_OBJECT

public:
    explicit client(QWidget *parent = 0);
    ~client();

//    void paintEvent(QPaintEvent *event);

private slots:

signals:

private:
    Ui::client *ui;
#if 0
    //光照参数显示
    QLineEdit *Illumination_edit;

    //温度参数显示
    QLineEdit *Temperature_edit;

    //湿度参数显示
    QLineEdit *Humidity_edit;

    //灯按钮
    QPushButton *Led_pushbutton;
    bool Lflag;

    //风扇按钮
    QPushButton *Fan_pushbutton;
    bool Fflag;

    //蜂鸣器按钮
    QPushButton *Beep_pushbutton;
    bool Bflag;

    QLabel *picture;
#endif
    bool Lflag;
    bool Fflag;
    bool Bflag;
    //语音识别对象
//    ASR *talk_module;

    //监控显示图像
    QFile *file;

};


#endif // CLIENT_H
