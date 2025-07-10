#include "client.h"
#include "ui_client.h"

client::client(QWidget *parent) : QWidget(parent), ui(new Ui::client){
    ui->setupUi(this);

    QPalette back = this->palette();
    QImage backPic(":/icon/空间站.jpeg");
    QImage fitPic = backPic.scaled(this->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    back.setBrush(QPalette::Window, QBrush(fitPic));
    this->setPalette(back);

    //标志位初始化
    Lflag = true;
    Fflag = true;
    Bflag = true;

    //设置窗口
    this->setWindowTitle("第九组 智能管家");
    this->setWindowIcon(QIcon(":/icon/智能机器人.png"));
    this->setMaximumSize(800, 600);
    this->setMinimumSize(800, 600);

#if 0
    //设置控件
    QLabel *Illumination_lab = new QLabel("光照：");
    QLabel *Temperature_lab = new QLabel("温度：");
    QLabel *Humidity_lab = new QLabel("湿度：");
    Temperature_edit = new QLineEdit;
    Illumination_edit = new QLineEdit;
    Humidity_edit = new QLineEdit;
    Temperature_edit->setReadOnly(true);
    Illumination_edit->setReadOnly(true);
    Humidity_edit->setReadOnly(true);
    Temperature_edit->setStyleSheet("background:transparent;border-width:0;border-style:outset");
    Illumination_edit->setStyleSheet("background:transparent;border-width:0;border-style:outset");
    Humidity_edit->setStyleSheet("background:transparent;border-width:0;border-style:outset");
    Temperature_edit->setText("--");
    Illumination_edit->setText("--");
    Humidity_edit->setText("--");
    Led_pushbutton = new QPushButton("灯");
    Led_pushbutton->setMaximumSize(150, 100);
    Fan_pushbutton = new QPushButton("风扇");
    Fan_pushbutton->setMaximumSize(150, 100);
    Beep_pushbutton = new QPushButton("蜂鸣器");
    Beep_pushbutton->setMaximumSize(150, 100);
    sound_widget = new ASR;
    picture = new QLabel;
    QHBoxLayout *hbox_sensor = new QHBoxLayout;
    hbox_sensor->addWidget(Illumination_lab);
    hbox_sensor->addWidget(Illumination_edit);
    hbox_sensor->addWidget(Temperature_lab);
    hbox_sensor->addWidget(Temperature_edit);
    hbox_sensor->addWidget(Humidity_lab);
    hbox_sensor->addWidget(Humidity_edit);
    QHBoxLayout *hbox_modul = new QHBoxLayout;
    hbox_modul->addWidget(sound_widget);
    hbox_modul->addWidget(picture);
    QHBoxLayout *hbox_button = new QHBoxLayout;
    hbox_button->addWidget(Led_pushbutton);
    hbox_button->addWidget(Fan_pushbutton);
    hbox_button->addWidget(Beep_pushbutton);
    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addLayout(hbox_sensor);
    vbox->addLayout(hbox_modul);
    vbox->addLayout(hbox_button);
    this->setLayout(vbox);
#endif
    //创建cmd通信子线程
    myserial *serial_thread = new myserial;
    serial_thread->start();
    connect(serial_thread, serial_thread->recv_serial, this, [this, serial_thread]{
        QString read_str =serial_thread->get_read_buf();
        QJsonObject json = QJsonDocument::fromJson(read_str.toLocal8Bit().data()).object();
        QString temperature_str = json.value("temperature").toString();
        ui->Temperature_label->setText(temperature_str);
        QString Illumination_str = json.value("illumination").toString();
        ui->Illumination_label->setText(Illumination_str);
        QString Humidity_str = json.value("humidity").toString();;
        ui->Humidity_label->setText(Humidity_str);
    });


    //创建语音识别子线程
    mytalk *talk_thread = new mytalk;
    talk_thread->start();
    connect(ui->talk_button, ui->talk_button->pressed, this, [this, talk_thread]{
        talk_thread->start_talk();
    });
    connect(ui->talk_button, ui->talk_button->released, this, [this, talk_thread]{
        talk_thread->end_talk();
    });
    connect(talk_thread, talk_thread->get_contrl_command_over, this, [this, talk_thread, serial_thread]{
        serial_thread->serial_socket->write(talk_thread->get_contrl_command().toStdString().c_str());
    });

    //创建视频传输子线程
    myvideo *video_thread = new myvideo;
    video_thread->start();
    connect(video_thread, video_thread->write_jpg_finished, this, [this]{
        QPixmap pix("C:/Users/HY/Desktop/test.jpeg");
        ui->picture->setPixmap(pix);
    });

    connect(ui->Led_pushbutton, ui->Led_pushbutton->clicked, this, [this, serial_thread]{
        if(Lflag){
            serial_thread->serial_socket->write("open_led");
            Lflag = false;
        }else{
            serial_thread->serial_socket->write("close_led");
            Lflag = true;
        }
    });

    connect(ui->Fan_pushbutton, ui->Fan_pushbutton->clicked, this, [this, serial_thread]{
        if(Fflag){
            serial_thread->serial_socket->write("open_fan_s");
            Fflag = false;
        }else{
            serial_thread->serial_socket->write("close_fan");
            Fflag = true;
        }
    });

    connect(ui->Beep_pushbutton, ui->Beep_pushbutton->clicked, this, [this, serial_thread]{
        if(Bflag){
            serial_thread->serial_socket->write("open_beep");
            Bflag = false;
        }else{
            serial_thread->serial_socket->write("close_beep");
            Bflag = true;
        }
    });
#if 0
    connect(serial_socket, serial_socket->readyRead, this, [this]{
        char read_buf[100] = {0};
        serial_socket->read(read_buf, 100);
        QString read_str(read_buf);

        QJsonObject json = QJsonDocument::fromJson(read_str.toLocal8Bit().data()).object();
        QString temperature_str = json.value("temperature").toString();
        ui->Temperature_label->setText(temperature_str);
        QString Illumination_str = json.value("illumination").toString();
        ui->Illumination_label->setText(Illumination_str);
        QString Humidity_str = json.value("humidity").toString();;
        ui->Humidity_label->setText(Humidity_str);
    });
#endif
}

client::~client(){
    delete ui;
}


