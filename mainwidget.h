#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include <QFile>
#include <QDir>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QDateTime>
#include <QUrl>
#include <QTimer>
#include <QSslSocket>

#include "parser.h"

namespace Ui {
class MainWidget;
}

class MainWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit MainWidget(QWidget *parent = 0);
    ~MainWidget();
    
private:
    Ui::MainWidget *ui;
    quint64 lastTimestamp;
    QByteArray readFile(const QString __filename);
    QNetworkAccessManager *m_manager_img;
    QNetworkAccessManager *m_manager_video;

    QSslSocket *socket;

    void getImage(const QString __url);
    void getVideo(const QString __url);
    void saveLastTimestump();
    void loadLastTimestump();
    QString imgDir;
    QTimer *timer;

public slots:
    void runParse(QByteArray __json);
    void slot_netwManagerFinished4Img(QNetworkReply *);
    void slot_netwManagerFinished4Vid(QNetworkReply *);

    void connectInstgrm();
    void getJSON(const int __timestump);
    void runPr();

    void upp();

signals:
    void data4parse(QByteArray);
};

#endif // MAINWIDGET_H
