#include "MainWindow.h"
#include "cos.h"

#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    //qDebug() << levenshtein_distance("321", "1234");

    return a.exec();
}
