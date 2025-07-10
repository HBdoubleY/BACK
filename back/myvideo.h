#ifndef MYVIDEO_H
#define MYVIDEO_H

#include <QThread>
#include <QTcpSocket>
#include <QString>
#include <QTimer>
#include <QTime>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include <QFile>
#include <QIODevice>
#include <QDebug>
#include <QImage>
#include <QPixmap>
#include <QFileDialog>
#include <QFileInfo>

class myvideo : public QThread
{
    Q_OBJECT
public:
    explicit myvideo(QObject *parent = nullptr);

protected:
    void run();

signals:
    void write_jpg_finished();

public slots:

private:
    QTcpSocket *video_socket;
    QFile *file;
    char buf_size[10];
    int size;
    char buf_data[1843200];
    int flag;
};

#endif // MYVIDEO_H
