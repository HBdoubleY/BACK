#ifndef ASR_H
#define ASR_H

#include <QMainWindow>
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

namespace Ui {
class ASR;
}

class ASR : public QMainWindow
{
    Q_OBJECT

public:
    QString get_contrl_command();

    explicit ASR(QWidget *parent = 0);
    ~ASR();

    void start_talk();

    void end_talk();

private slots:
    void on_pushButton_pressed();

    void on_pushButton_released();

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

    void token_init();
    void ctr_data();

    Ui::ASR *ui;

    QTcpSocket *socket;
signals:
    void start_speech_recognize();
    void gettoken();
    void get_contrl_command_over();
};

#endif // ASR_H
