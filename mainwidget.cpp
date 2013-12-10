#include "mainwidget.h"
#include "ui_mainwidget.h"

MainWidget::MainWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWidget),
    lastTimestamp(0),
    id(0)
{
    ui->setupUi(this);
    connect(ui->pushButton,SIGNAL(clicked()),this,SLOT(runPr()));

    m_manager = new QNetworkAccessManager(this);
    connect(m_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(slot_netwManagerFinished(QNetworkReply*)));

    socket = new QSslSocket(this);
    connect(this,SIGNAL(data4parse(QByteArray)),this,SLOT(runParse(QByteArray)));

    imgDir = qApp->applicationDirPath().append(QDir::separator()).append("img").append(QDir::separator());
    QDir dir(imgDir);
    if(!dir.exists())
    {
        dir.mkdir(imgDir);
    }
    loadLastTimestump();
    loadId();

    ui->textEdit->setReadOnly(true);
    ui->textEdit->setUndoRedoEnabled(false);
    ui->textEdit->document()->setMaximumBlockCount(80);
}

MainWidget::~MainWidget()
{
    delete m_manager;
    delete socket;
    delete ui;
}

void MainWidget::runParse(QByteArray __json)
{
    if(__json.size() == 47)
    {
        lastTimestamp-=1000;
        ui->textEdit->append(QDateTime::currentDateTime().toString("dd-MM-yy hh:mm:ss").append(QString(" - Parsed data is small")));
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
    QStringList idList;

    foreach(QVariant record, dataList)
    {
        QVariantMap map = record.toMap();

        if(checkId(map.value("id").toString()))
        {
            QVariantMap file;
            QVariantMap file_std_rez_url;

            if(map["type"].toString() == "image")
            {
                file = map["images"].toMap();
                file_std_rez_url = file["standard_resolution"].toMap();
            }

            if(map["type"].toString() == "video")
            {
                file = map["videos"].toMap();
                file_std_rez_url = file["standard_resolution"].toMap();
            }

            getFile(file_std_rez_url.value("url").toString());
            ui->textEdit->append(file_std_rez_url.value("url").toString());

            idList.append(map.value("id").toString());
        }
    }

    if(idList.size() > 0)
    {
        QStringList list = idList.at(0).split("_");
        if(list.size() == 2)
        {
            id = list.at(0).toULongLong();
        }

        saveId();
    }

    ui->textEdit->append("---END PARESE---\n\n\n");
}

void MainWidget::getFile(const QString &__url)
{
    QUrl url(__url);

    QNetworkRequest request(url);
    m_manager->get(request);
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

bool MainWidget::checkId(const QString &__id)
{
    QStringList list = __id.split("_");
    quint64 temp;
    if(list.size() == 2)
    {
        temp = list.at(0).toULongLong();
        if(temp > id)
        {
            return true;
        }
    }

    return false;
}

void MainWidget::saveId()
{
    QString fileName(qApp->applicationDirPath().append(QDir::separator()).append("lastId.txt"));
    QFile file(fileName);
    if(!file.open(QIODevice::WriteOnly))
    {
        ui->textEdit->append("Невозможно сохранить Id");
        return;
    }
    QTextStream st(&file);
    st << id;
    file.close();
}

void MainWidget::loadId()
{
    QString fileName(qApp->applicationDirPath().append(QDir::separator()).append("lastId.txt"));
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly))
    {
        ui->textEdit->append("Невозможно загрузить Id");
        return;
    }

    QTextStream st(&file);
    quint64 temp;
    st >> temp;
    if(temp > 0)
    {
        id = temp;
    }
    file.close();
    ui->textEdit->append(QString("Last Id - %1").arg(id));
}

void MainWidget::slot_netwManagerFinished(QNetworkReply *reply)
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
    QString ext;

    if(reply->header(QNetworkRequest::ContentTypeHeader).toString() == "video/mp4")
    {
        ext.append(".mp4");
    }

    if(reply->header(QNetworkRequest::ContentTypeHeader).toString() == "image/jpeg")
    {
        ext.append(".jpg");
    }

    QFile file(vidi.append(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmsszzz")).append(ext));
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

void MainWidget::getJSON()
{
    if(socket->state() != QSslSocket::ConnectedState)
    {
        ui->textEdit->append(QDateTime::currentDateTime().toString("dd-MM-yy hh:mm:ss").append(" - Reconnect"));
        connectInstgrm();
    }

    QByteArray arr;
    if(id == 0)
    {
        arr.append(QString("GET /v1/users/self/feed?access_token=315332474.ab103e5.8b6cb34f9fce405cb430bf5fa7968190 HTTP/1.1\r\n"
                       "X-HostCommonName: api.instagram.com\r\n"
                       "Host: api.instagram.com\r\n"
                       "X-Target-URI: https://api.instagram.com\r\n"
                       "Connection: Keep-Alive\r\n\r\n")
                   );
    }
    else
    {
        arr.append(QString("GET /v1/users/self/feed?min_id=%1&access_token=315332474.ab103e5.8b6cb34f9fce405cb430bf5fa7968190 HTTP/1.1\r\n"
                       "X-HostCommonName: api.instagram.com\r\n"
                       "Host: api.instagram.com\r\n"
                       "X-Target-URI: https://api.instagram.com\r\n"
                       "Connection: Keep-Alive\r\n\r\n")
                   .arg(id)
                   );
    }

    ui->textEdit->append(QDateTime::currentDateTime().toString("dd-MM-yy hh:mm:ss").append(" - ").append(arr));
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
    getJSON();
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(upp()));
    timer->start(30000);
}

void MainWidget::upp()
{
    getJSON();
}
