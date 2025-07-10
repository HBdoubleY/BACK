#include "mytalk.h"

mytalk::mytalk(QObject *parent) : QThread(parent)
{
    //设置音频信息采样格式
    QAudioFormat talk_format;
    talk_format.setSampleRate(16000);
    talk_format.setChannelCount(1);
    talk_format.setCodec("audio/pcm");
    talk_format.setByteOrder(QAudioFormat::LittleEndian);
    talk_format.setSampleType(QAudioFormat::UnSignedInt);
    talk_format.setSampleSize(16);

    m_a_input = new QAudioInput(talk_format, this);        //麦克风输入初始化
    m_a_file.setFileName("录音.pcm");                    // a_file.setFileName("录音.pcm");//初始化录音文件
}

void mytalk::run(){
    connect(this,SIGNAL(start_speech_recognize()),this,SLOT(speech_recognize()));

}

void mytalk::speech_recognize()
{
    token_init();
    connect(this,SIGNAL(gettoken()),this,SLOT(connect_api()));
}

void mytalk::connect_api()
{
    qDebug()<<"Execute connect_api";
    QString baseurl="http://vop.baidu.com/server_api";
    //极速版地址：QString baseurl="https://vop.baidu.com/pro_api";
    QUrl asrurl(baseurl);

    QUrlQuery query;//以RAW形式发送
    query.addQueryItem("token",token);
    query.addQueryItem("cuid","12345");
    asrurl.setQuery(query);

    QNetworkRequest request;
    manager = new QNetworkAccessManager(this);
    qDebug() << manager->supportedSchemes();//检查qt支持的协议

    QSslConfiguration config;//ssl加密认证，https协议所需
    QSslConfiguration conf = request.sslConfiguration();
    conf.setPeerVerifyMode(QSslSocket::VerifyNone);
    conf.setProtocol(QSsl::TlsV1SslV3);
    request.setSslConfiguration(conf);

    request.setHeader(QNetworkRequest::ContentTypeHeader, "audio/pcm;rate=16000");  //raw格式发送，设置header

    QString filename = QDir::currentPath();
    filename.append("/录音.pcm");

    QFile file(filename);
    QFileInfo info(filename);

    qDebug()<<"size:"<<info.size();

    ad_data= (char *)malloc(info.size());//打开音频文件，并开辟内存存储，等待发送

    file.open(QIODevice::ReadOnly);
    QByteArray send_data=file.read(info.size());

    request.setUrl(asrurl);

    manager->post(request,send_data);

    connect(manager,&QNetworkAccessManager::finished,this,&mytalk::printf_character);//读取返回值
}

void mytalk::token_init()
{
    this->disconnect(SIGNAL(gettoken()));//释放信号

    QString token_baseurl="https://aip.baidubce.com/oauth/2.0/token";
    //极速版地址，QString token_baseurl="https://openapi.baidu.com";

    QUrlQuery token_qurey;
    token_qurey.addQueryItem("grant_type","client_credentials");
    token_qurey.addQueryItem("client_id","eBQkqNFoahwAT5a9neY4R6T2");
    token_qurey.addQueryItem("client_secret","Vk8ZGOvLHhW0wSyKXkyooUx5oQZOzdvD");

    QUrl token_url(token_baseurl);
    token_url.setQuery(token_qurey);

    QNetworkRequest token_request;
    token_manager =new QNetworkAccessManager(this);

    QSslConfiguration token_config;//https协议复查
    QSslConfiguration conf = token_request.sslConfiguration();
    conf.setPeerVerifyMode(QSslSocket::VerifyNone);
    conf.setProtocol(QSsl::TlsV1SslV3);
    token_request.setSslConfiguration(conf);

    token_request.setUrl(token_url);
    token_manager->get(token_request);
    connect(token_manager,&QNetworkAccessManager::finished,this,&mytalk::get_token_over);
}

void mytalk::printf_character(QNetworkReply *reply)
{
    free(ad_data);//释放数据缓冲区
    QByteArray rece_data= reply->readAll();
    QJsonObject json =QJsonDocument::fromJson(rece_data).object();
    QJsonArray array = json.value("result").toArray();                          //获取识别结果
    QJsonDocument doc1;
    doc1.setArray(array);
    QString tmp_ctr_data(doc1.toJson());
    data=tmp_ctr_data;
    ctr_data();//发送命令
    qDebug()<<contrl_command;
    emit get_contrl_command_over();
}

void mytalk::start_talk(){
    qDebug()<<"Start the recording.";
    m_a_file.open(QIODevice::ReadWrite|QIODevice::Truncate);        //打开录音文件，准备写入
    m_a_input->start(&m_a_file);
}

void mytalk::end_talk(){
    qDebug()<<"End of the recording.";
    m_a_input->stop();
    m_a_file.close();
    emit start_speech_recognize();
}

void mytalk::get_token_over(QNetworkReply *reply)
{
    QByteArray rece_data= reply->readAll();
    QJsonObject boken_json =QJsonDocument::fromJson(rece_data).object();
    token = boken_json.value("access_token").toString();//获取access_token
    emit gettoken();//发送信号：已得到token。
}

void mytalk::ctr_data()
{
    if(data.contains("开")){
        if(data.contains(("灯"))){
                contrl_command="open_led";
        }
        if(data.contains(("报警器"))||data.contains("蜂鸣器")){
                contrl_command="open_beep";
        }
        if(data.contains(("风扇"))){
            if(data.contains("档位")||data.contains("档")){
                if(data.contains("高")){
                    contrl_command="open_fan_h";
                }
                if(data.contains("中")){
                    contrl_command="open_fan_m";
                }
                else
                    contrl_command="open_fan_s";
            }
            else contrl_command="open_fan_s";
        }
        if(data.contains(("数码管"))){
            contrl_command="open_sdt";
        }
    }
    else if(data.contains("关")){
        if(data.contains(("灯"))){
                contrl_command="close_led";
        }
        if(data.contains(("报警器"))){
            contrl_command="close_beep";
        }
        if(data.contains(("风扇"))){
            contrl_command="close_fan";
        }
        if(data.contains(("数码管"))){
            contrl_command="close_sdt";
        }
    }
    else contrl_command = "Contrl command error!";
}

QString mytalk::get_contrl_command()
{
    return contrl_command;
}

