#include "myvideo.h"

myvideo::myvideo(QObject *parent) : QThread(parent)
{
    flag = 0;
    size = 0;
    file = new QFile("C:/Users/HY/Desktop/test.jpeg");
    video_socket = new QTcpSocket;
    video_socket->connectToHost("192.168.6.222", 8888);
}

void myvideo::run(){
    connect(video_socket, video_socket->readyRead, this, [this]{
        switch (flag) {
            case 0:{
                video_socket->read(buf_size, sizeof(buf_size));
                size = atoi(buf_size);
                flag = 1;
                break;
            }
            case 1:{
                video_socket->read(buf_data, size);
                file->open(QIODevice::WriteOnly | QIODevice::Truncate);
                file->write(buf_data, size);
                file->close();
                emit write_jpg_finished();
                flag = 0;
                break;
            }
        }
    });
}
