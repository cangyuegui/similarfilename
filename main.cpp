#include "MainWindow.h"
#include "cos.h"

#include <QApplication>
#include <QDebug>

#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QString qm = a.applicationDirPath() + "/language.qm";
    if (QFile::exists(qm))
    {
        QTranslator* q = new QTranslator;
        if (q->load(qm))
        {
            a.installTranslator(q);
        }
    }

    MainWindow w;
    w.show();

    if (argc != 2 || QString(argv[1]) != "test")
    {
        w.hideTest();
    }

    //qDebug() << levenshtein_distance("321", "1234");

    return a.exec();
}
