#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileInfo>
#include <QPair>
#include <QModelIndex>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QString filtername(const QString& name);
    void sortNames();
    void updateDir(const QString& dir);
    void setHeadData();
    void updateModel();

private slots:
    void on_select_path_clicked();
    void on_refresh_clicked();

    void on_test_clicked();

    void on_search_clicked();

    void on_pathview_customContextMenuRequested(const QPoint &pos);

    void del();
    void delSel();

private:
    Ui::MainWindow *ui;

    QString doDir;
    QVector<QPair<QString, QFileInfo>> allfilepaths;
    QVector<QPair<QString, QFileInfo>> resfilepaths;
    int searchline = 0;
    QModelIndex delIndex;
};
#endif // MAINWINDOW_H
