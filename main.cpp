#include "MainWindow.h"
#include "cos.h"

#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;
    w.show();

    if (argc != 2 || QString(argv[1]) != "test")
    {
        w.hideTest();
    }

    //qDebug() << levenshtein_distance("321", "1234");

    return a.exec();
}
