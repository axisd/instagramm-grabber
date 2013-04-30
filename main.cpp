#include "mainwidget.h"
#include <QApplication>
#include <QTextCodec>

int main(int argc, char *argv[])
{
    QTextCodec::setCodecForTr( QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForCStrings( QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForLocale( QTextCodec::codecForName("UTF-8"));

    QApplication a(argc, argv);
    MainWidget w;
    w.show();
    
    return a.exec();
}
