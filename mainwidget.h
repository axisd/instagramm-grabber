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
    quint64 id;
    QNetworkAccessManager *m_manager;

    QSslSocket *socket;

    void getFile(const QString &__url);
    void saveLastTimestump();
    void loadLastTimestump();

    bool checkId(const QString &__id);
    void saveId();
    void loadId();
    QString imgDir;
    QTimer *timer;

public slots:
    void runParse(QByteArray __json);
    void slot_netwManagerFinished(QNetworkReply *);

    void connectInstgrm();
    void getJSON();
    void runPr();

    void upp();

signals:
    void data4parse(QByteArray);
};

#endif // MAINWIDGET_H
