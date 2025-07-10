#ifndef MYTALK_H
#define MYTALK_H

#include <QThread>
#include <QDialog>
#include <QString>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QAudio>
#include <QAudioFormat>
#include <QAudioInput>
#include <QAudioOutput>
#include <QAudioDeviceInfo>
#include <QIODevice>
#include <QDebug>
#include <QThread>
#include <QtCore/QCoreApplication>
#include <QFileDialog>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QDebug>
#include <QUrlQuery>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFileDialog>
#include <QtCore/QObject>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSslConfiguration>
#include <QTextCodec>
#include <QCoreApplication>

class mytalk : public QThread
{
    Q_OBJECT
public:
    explicit mytalk(QObject *parent = nullptr);
    void start_talk();
    void end_talk();
    QString get_contrl_command();

protected:
    void run();

signals:
    void start_speech_recognize();
    void gettoken();
    void get_contrl_command_over();

public slots:
    void speech_recognize();
    void get_token_over(QNetworkReply *reply);
    void connect_api();
    void printf_character(QNetworkReply *reply);

private:
    QFile m_a_file;
    QAudioInput *m_a_input;
    QNetworkAccessManager *manager;
    QNetworkAccessManager *token_manager;
    QString token;
    QString data;                       //原始控制命令信息
    QString contrl_command;             //封装控制命令信息
    char *ad_data;
    void ctr_data();
    void token_init();


    QTcpSocket *socket;
};

#endif // MYTALK_H
