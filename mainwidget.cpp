#include "mainwidget.h"
#include "ui_mainwidget.h"

MainWidget::MainWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWidget),
    lastTimestamp(0)
{
    ui->setupUi(this);
    connect(ui->pushButton,SIGNAL(clicked()),this,SLOT(runPr()));

    m_manager_img = new QNetworkAccessManager(this);
    connect(m_manager_img, SIGNAL(finished(QNetworkReply*)), this, SLOT(slot_netwManagerFinished4Img(QNetworkReply*)));

    m_manager_video = new QNetworkAccessManager(this);
    connect(m_manager_video, SIGNAL(finished(QNetworkReply*)), this, SLOT(slot_netwManagerFinished4Vid(QNetworkReply*)));

    socket = new QSslSocket(this);
    connect(this,SIGNAL(data4parse(QByteArray)),this,SLOT(runParse(QByteArray)));

    imgDir = qApp->applicationDirPath().append(QDir::separator()).append("img").append(QDir::separator());
    QDir dir(imgDir);
    if(!dir.exists())
    {
        dir.mkdir(imgDir);
    }
    loadLastTimestump();

    ui->textEdit->setReadOnly(true);
    ui->textEdit->setUndoRedoEnabled(false);
    ui->textEdit->document()->setMaximumBlockCount(80);
}

MainWidget::~MainWidget()
{
    delete m_manager_img;
    delete m_manager_video;
    delete socket;
    delete ui;
}

void MainWidget::runParse(QByteArray __json)
{
    if(__json.size() == 47)
    {
        lastTimestamp-=1000;
        ui->textEdit->append(QDateTime::currentDateTime().toString("dd-MM-yy hh:mm:ss").append(QString("Parsed datra is small")));
        return;
    }

    QJson::Parser parser;
    bool ok;
    ui->textEdit->append(QDateTime::currentDateTime().toString("dd-MM-yy hh:mm:ss").append(QString(" - Parse (%1)").arg(__json.size())));
    ui->textEdit->append(__json);

    QVariantMap result = parser.parse(__json, &ok).toMap();
    if (!ok)
    {
        ui->textEdit->append(QDateTime::currentDateTime().toString("dd-MM-yy hh:mm:ss").append(" - Пока парсили возникла ошибочка - беда-то."));
        ui->textEdit->append(__json);
        ui->textEdit->append("\n");
        return;
    }

    QVariantList dataList = result["data"].toList();
    QList<quint64> timeList;

    foreach(QVariant record, dataList)
    {
        QVariantMap map = record.toMap();

        if(map.value("created_time").toULongLong() > lastTimestamp)
        {
            if(map["type"].toString() == "image")
            {
                QVariantMap img = map["images"].toMap();
                QVariantMap img_std_rez_url = img["standard_resolution"].toMap();
                getImage(img_std_rez_url.value("url").toString());
                ui->textEdit->append(img_std_rez_url.value("url").toString());
            }

            if(map["type"].toString() == "video")
            {
                QVariantMap video = map["videos"].toMap();
                QVariantMap video_std_rez_url = video["standard_resolution"].toMap();
                getVideo(video_std_rez_url.value("url").toString());
                ui->textEdit->append(video_std_rez_url.value("url").toString());
            }

            timeList.append(map.value("created_time").toLongLong());
        }
    }

    if(timeList.size() > 0)
    {
        lastTimestamp = timeList.at(0);
        saveLastTimestump();
    }

    ui->textEdit->append("---END PARESE---\n\n\n");
}

void MainWidget::getImage(const QString __url)
{
    QUrl url(__url);

    QNetworkRequest request(url);
    m_manager_img->get(request);
}

void MainWidget::getVideo(const QString __url)
{
    QUrl url(__url);

    QNetworkRequest request(url);
    m_manager_video->get(request);
}

void MainWidget::saveLastTimestump()
{
    QString fileName(qApp->applicationDirPath().append(QDir::separator()).append("lastTS.txt"));
    QFile file(fileName);
    if(!file.open(QIODevice::WriteOnly))
    {
        ui->textEdit->append("Невозможно сохранить lastTimeSump");
        return;
    }
    QTextStream st(&file);
    st << lastTimestamp;
    file.close();
}

void MainWidget::loadLastTimestump()
{
    QString fileName(qApp->applicationDirPath().append(QDir::separator()).append("lastTS.txt"));
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly))
    {
        ui->textEdit->append("Невозможно загрузить lastTimeSump");
        return;
    }
    QTextStream st(&file);
    long long temp;
    st >> temp;
    if(temp > 0)
    {
        lastTimestamp = temp;
    }
    file.close();
    ui->textEdit->append(QString("Last TS - %1").arg(lastTimestamp));
}

void MainWidget::slot_netwManagerFinished4Img(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        ui->textEdit->append("Error in" + reply->url().toString() + ":" + reply->errorString());
        return;
    }
    QVariant attribute = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if (attribute.isValid()) {
        QUrl url = attribute.toUrl();
        ui->textEdit->append("must go to:" + url.toString());
        return;
    }
    ui->textEdit->append("ContentType:" + reply->header(QNetworkRequest::ContentTypeHeader).toString());
    QByteArray jpegData = reply->readAll();

    QPixmap pixmap;
    pixmap.loadFromData(jpegData);

    QString imdi(imgDir);

    pixmap.save(imdi.append(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmsszzz")).append(".jpg"), "JPG");
}

void MainWidget::slot_netwManagerFinished4Vid(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        ui->textEdit->append("Error in" + reply->url().toString() + ":" + reply->errorString());
        return;
    }
    QVariant attribute = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if (attribute.isValid()) {
        QUrl url = attribute.toUrl();
        ui->textEdit->append("must go to:" + url.toString());
        return;
    }
    ui->textEdit->append("ContentType:" + reply->header(QNetworkRequest::ContentTypeHeader).toString());
    QString vidi(imgDir);

    QFile file(vidi.append(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmsszzz")).append(".mp4"));
    if (!file.open(QIODevice::WriteOnly))
    {
        ui->textEdit->append(QString("Could not open %1 for writing: %2")
                             .arg(vidi)
                             .arg(file.errorString())
                             );
        return;
    }
    file.write(reply->readAll());
    file.close();
}

void MainWidget::connectInstgrm()
{
    socket->connectToHostEncrypted("api.instagram.com", 443);
    if ( !socket->waitForEncrypted() )
    {
        ui->textEdit->append(socket->errorString());
        return;
    }
    socket->waitForConnected(500);
    if(socket->state() == QAbstractSocket::ConnectedState)
    {
        ui->textEdit->append("Connect succes\n");
    }
    else
    {
        ui->textEdit->append(QString("Connect error: %1\n").arg(socket->error()));
    }
}

void MainWidget::getJSON(const int __timestump)
{
    if(socket->state() != QSslSocket::ConnectedState)
    {
        ui->textEdit->append(QDateTime::currentDateTime().toString("dd-MM-yy hh:mm:ss").append(" - Reconnect"));
        connectInstgrm();
    }

    QByteArray arr;
    arr.append(QString("GET /v1/users/22252058/media/recent?min_timestamp=%1&access_token=315332474.ab103e5.8b6cb34f9fce405cb430bf5fa7968190 HTTP/1.1\r\n"
                   "X-HostCommonName: api.instagram.com\r\n"
                   "Host: api.instagram.com\r\n"
                   "X-Target-URI: https://api.instagram.com\r\n"
                   "Connection: Keep-Alive\r\n\r\n").arg(__timestump));

    //GET /v1/users/22252058/media/recent?access_token=315332474.1fb234f.ec19ec4d253c40e888c1e68c9ffe1d00 HTTP/1.1
    ui->textEdit->append(QDateTime::currentDateTime().toString("dd-MM-yy hh:mm:ss").append(" - ").append("GET"));
    socket->write(arr);

    bool fail = false;
    QByteArray data, json;
    while ( socket->waitForReadyRead(1000) )
    {
        data.append(socket->readAll());
        if(!data.contains("HTTP/1.1 200 OK"))
        {
            ui->textEdit->append(QDateTime::currentDateTime().toString("dd-MM-yy hh:mm:ss").append(" - ").append(data.left(data.indexOf("{"))));
            fail = true;
        }
    }
    ui->textEdit->append(QDateTime::currentDateTime().toString("dd-MM-yy hh:mm:ss").append(" - ").append(data.left(data.indexOf("{"))));

    if(fail)
    {
        ui->textEdit->append(QDateTime::currentDateTime().toString("dd-MM-yy hh:mm:ss").append(" - ").append("FAIL"));
        return;
    }

    QByteArray json_src = data.mid(data.indexOf("{"), data.size());
    QString json_str(json_src);
    QStringList json_str_list = json_str.split("\r\n", QString::SkipEmptyParts);
    for(int i = 0; i < json_str_list.size(); i+=2)
    {
        json.append(json_str_list.at(i));
    }
    runParse(json);
}

void MainWidget::runPr()
{
    connectInstgrm();
    getJSON(lastTimestamp);
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(upp()));
    timer->start(30000);
}

void MainWidget::upp()
{
    getJSON(lastTimestamp);
}
