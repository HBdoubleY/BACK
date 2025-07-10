#ifndef MYSERIAL_H
#define MYSERIAL_H

#include <QThread>
#include <QTcpSocket>
#include <QString>
#include <QDebug>

class myserial : public QThread
{
    Q_OBJECT
public:
    explicit myserial(QObject *parent = nullptr);
    QString get_read_buf();

    QTcpSocket *serial_socket;

protected:
    void run();

signals:
    void recv_serial();

public slots:

private:
    QString read_buf;
};

#endif // MYSERIAL_H
