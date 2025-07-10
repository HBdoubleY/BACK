#include "myserial.h"

myserial::myserial(QObject *parent) : QThread(parent)
{
    //连接套接字
    serial_socket = new QTcpSocket;
    serial_socket->connectToHost("192.168.6.222", 9999);
}

void myserial::run(){
    connect(serial_socket, serial_socket->readyRead, this, [this]{
        char buf[100] = {0};
        serial_socket->read(buf, 100);
        read_buf = buf;
        //qDebug()<< read_buf;
        emit recv_serial();
    });
}

QString myserial::get_read_buf(){
    return read_buf;
}
