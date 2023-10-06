#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QFileDialog>
#include <QDir>
#include <QStandardItemModel>
#include <QTextCodec>
#include <QMessageBox>

#include "cos.h"

#ifdef Q_OS_WIN32

#include "windows.h"

bool MoveToTrashImpl(const QString& file ){
    QFileInfo fileinfo( file );
    if( !fileinfo.exists() )
        return false;

    SHFILEOPSTRUCTW fileop;
    memset( &fileop, 0, sizeof( fileop ) );
    fileop.wFunc = FO_DELETE;
    std::wstring wstdfile = fileinfo.absoluteFilePath().toStdWString();
    fileop.pFrom = wstdfile.c_str();
    fileop.fFlags = FOF_ALLOWUNDO /*| FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT*/;
    int rv = SHFileOperationW( &fileop );
    if( 0 != rv ){
        return false;
    }

    return true;
}
#endif


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QStandardItemModel* m = new QStandardItemModel(this);
    ui->pathview->setModel(m);
    setHeadData();
}

MainWindow::~MainWindow()
{
    delete ui;
}

QString sectionNumber(const QString &name, const QString& sec)
{
    QString rname = name.simplified();
    QStringList sections = rname.split(sec, Qt::SkipEmptyParts);
    if (sections.size() > 1)
    {
        QString pre = sections[0];
        bool isint = false;
        pre.toInt(&isint);
        if (isint)
        {
            sections.removeFirst();
        }
        else if (pre.contains(".com") || pre.contains(".cc") || pre.contains(".cn")  || pre.contains(".org"))
        {
            sections.removeFirst();
        }
        rname = sections.join("-");
    }

    return rname;
}

QString MainWindow::filtername(const QString &name)
{
    QString rname = name.simplified();
    rname = sectionNumber(rname, "-");
    rname = sectionNumber(rname, "_");

    QStringList sections = rname.split(".", Qt::SkipEmptyParts);
    if (sections.size() > 1)
    {
        sections.removeLast();
        rname = sections.join(".");
    }

    QString toSimple("+.-\343\200\212\343\200\213\343\200\220\343\200\221");
    for (auto c : toSimple)
    {
        rname = rname.remove(c);
    }

    return rname;
}

void MainWindow::sortNames()
{
    resfilepaths.clear();
    bool first = true;

    QString debugStr = ui->search_edit->text();

    struct CellGuess
    {
        int size;
        QHash<QChar, unsigned char> chars;
        QFileInfo fi;
    };
    QPair<QString, CellGuess> f;

    QVector<QPair<QString, CellGuess>> suballfilepaths;
    for (const QPair<QString, QFileInfo>& p : allfilepaths)
    {
        /*
        if (!debugStr.isEmpty())
        {
            if (p.first.contains(debugStr))
            {
                int xxx = 0;
            }
        }
        */

        CellGuess cg;
        cg.fi = p.second;
        cg.size = p.first.size();
        for (auto c : p.first)
        {
           cg.chars[c] += 1;
        };
        suballfilepaths.push_back({p.first, cg});
    }

    uint64_t docount = 0;

    while (!suballfilepaths.empty())
    {
        if (first)
        {
            first = false;
            f = suballfilepaths.takeFirst();
            resfilepaths.push_back({f.first, f.second.fi});
            docount += 1;
            if (docount % 100000 == 0)
            {
                qDebug() << docount;
            }

            continue;
        }

        auto mitr = suballfilepaths.begin();
        //int mindis = 1000000;
        double mindis = 0;
        for (auto i = suballfilepaths.begin(); i != suballfilepaths.end(); ++i)
        {
            /*
            int ld = levenshtein_distance(i->first, f.first);
            if (ld < mindis)
            {
                mitr = i;
                mindis = ld;
            }
            */

            docount += 1;
            if (docount % 100000 == 0)
            {
                qDebug() << docount;
            }

            QPair<QString, CellGuess>& ff = *i;



            double g = 0;

            if (f.second.size > ff.second.size)
            {
                g = guess(ff.second.chars, f.second.chars, ff.second.size, f.second.size);

                /*
                if (!debugStr.isEmpty())
                {
                    if (f.first.contains(debugStr) && ff.first.contains(debugStr))
                    {
                        qDebug() << guess(ff.second.chars, f.second.chars, ff.second.size, f.second.size);
                        exit(0);
                    }
                }
                */

            }
            else
            {
                g = guess(f.second.chars, ff.second.chars, f.second.size, ff.second.size);

                /*
                if (!debugStr.isEmpty())
                {
                    if (f.first.contains(debugStr) && ff.first.contains(debugStr))
                    {
                        qDebug() << guess(f.second.chars, ff.second.chars, f.second.size, ff.second.size);
                        exit(0);
                    }
                }
                */
            }

            if (g < 0.3)
            {
                continue;
            }

            if (g > 0.8)
            {
                mitr = i;
                mindis = g;
                break;
            }

            if (g > mindis)
            {
                mitr = i;
                mindis = g;
            }
        }

        f = *mitr;
        resfilepaths.push_back({f.first, f.second.fi});
        suballfilepaths.erase(mitr);
    }
}

void MainWindow::updateDir(const QString &dir)
{
    QDir ddir(dir);
    QList<QDir> dDirs = {ddir};
    while (!dDirs.isEmpty())
    {
        QDir sdir = dDirs.takeFirst();
        if (!sdir.exists())
        {
            continue;
        }
        QFileInfoList ff = sdir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
        for (auto f : ff)
        {
            if (f.isFile())
            {
                QString tpath = f.fileName();
                QStringList tpaths = tpath.split(".", Qt::SkipEmptyParts);
                if (tpaths.empty())
                {
                    continue;
                }

                 QPair<QString, QFileInfo> cell;
                 cell.first = filtername(tpath);
                 cell.second = f;
                 allfilepaths.push_back(cell);
            }
        }
    }

    sortNames();
    updateModel();
}

void MainWindow::setHeadData()
{
    auto m = qobject_cast<QStandardItemModel*>(ui->pathview->model());
    m->setColumnCount(5);
    m->setHeaderData(0, Qt::Horizontal, tr(""));
    m->setHeaderData(1, Qt::Horizontal, tr("type"));
    m->setHeaderData(2, Qt::Horizontal, tr("name"));
    m->setHeaderData(3, Qt::Horizontal, tr("fullpath"));
    m->setHeaderData(4, Qt::Horizontal, tr("size"));
}

void MainWindow::updateModel()
{
    auto m = qobject_cast<QStandardItemModel*>(ui->pathview->model());
    m->clear();
    setHeadData();

    for (auto f : resfilepaths)
    {
        QStandardItem* icheck = new QStandardItem;
        icheck->setCheckable(true);
        QStandardItem* i0 = new QStandardItem;
        i0->setText(f.second.suffix());
        QStandardItem* i1 = new QStandardItem;
        i1->setEditable(false);
        i1->setText(f.first);
        QStandardItem* i2 = new QStandardItem;
        i2->setEditable(false);
        i2->setText(f.second.absoluteFilePath());
        QStandardItem* i3 = new QStandardItem;
        i3->setEditable(false);
        double ssize = static_cast<double>(f.second.size());
        QString sssize = QString("%1b").arg(QString::number(ssize));
        if (ssize >= 1024)
        {
            ssize = static_cast<double>(ssize) / 1024.0;
            sssize = QString("%1kb").arg(QString::number(ssize, 'f', 2));
        }

        if (ssize >= 1024)
        {
            ssize = static_cast<double>(ssize) / 1024.0;
            sssize = QString("%1mb").arg(QString::number(ssize, 'f', 2));
        }

        if (ssize >= 1024)
        {
            ssize = static_cast<double>(ssize) / 1024.0;
            sssize = QString("%1gb").arg(QString::number(ssize, 'f', 2));
        }
        i3->setText(sssize);

        m->insertRow(m->rowCount(), {icheck, i0, i1, i2, i3});
    }
}


void MainWindow::on_select_path_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, "select dir", ".");
    if (dir.isEmpty())
    {
        return;
    }

    doDir = dir;
    allfilepaths.clear();
    updateDir(dir);
    sortNames();
    updateModel();
}

void MainWindow::on_refresh_clicked()
{
    allfilepaths.clear();
    updateDir(doDir);
    sortNames();
    updateModel();
}

void MainWindow::on_test_clicked()
{
    QString file = QFileDialog::getOpenFileName(this, "test files", ".", "*.txt");
    if (file.isEmpty())
    {
        return;
    }

    QFile tfile(file);
    tfile.open(QFile::ReadOnly);

    QByteArray tcontent = tfile.readAll();
    QTextCodec *codec = QTextCodec::codecForName("GBK");
    QString str = codec->toUnicode(tcontent);

    allfilepaths.clear();
    QStringList sstrs = str.split("\n", Qt::SkipEmptyParts);
    for (auto fileline : sstrs)
    {
        if (fileline.endsWith("\r"))
        {
            fileline = fileline.mid(0, fileline.length() - 1);
        }
        QFileInfo fi(fileline);

        QString tpath = fi.fileName();
        QStringList tpaths = tpath.split(".", Qt::SkipEmptyParts);
        if (tpaths.empty())
        {
            continue;
        }

         QPair<QString, QFileInfo> cell;
         cell.first = filtername(tpath);;
         cell.second = fi;
         allfilepaths.push_back(cell);
    }

    sortNames();
    //resfilepaths.clear();
    //sort_strings(allfilepaths, resfilepaths);
    updateModel();

}

void MainWindow::on_search_clicked()
{
    QString stext = ui->search_edit->text();
    QModelIndex findindex;
    for (int i = searchline + 1; i < ui->pathview->model()->rowCount(); ++i)
    {
        for (int j = 2; j <= 3; ++j)
        {
            QModelIndex mi = ui->pathview->model()->index(i, j);
            if (mi.data().toString().contains(stext))
            {
                findindex = mi;
                searchline = i;
                break;
            }
        }

        if (findindex.isValid())
        {
            break;
        }
    }

    if (findindex.isValid())
    {
        ui->pathview->scrollTo(findindex);
        ui->pathview->selectionModel()->select(findindex, QItemSelectionModel::Rows | QItemSelectionModel::Select);
    }
    else
    {
        searchline = -1;
    }
}

void MainWindow::on_pathview_customContextMenuRequested(const QPoint &pos)
{
    QModelIndex cindex = ui->pathview->indexAt(pos);
    if (!cindex.isValid())
    {
        return;
    }
    delIndex = cindex;

    QMenu dmenu(this);
    QAction* del = dmenu.addAction(tr("Delete"));
    connect(del, SIGNAL(triggered(bool)), this, SLOT(del()));
    QAction* delSel = dmenu.addAction(tr("Delete Selected"));
    connect(delSel, SIGNAL(triggered(bool)), this, SLOT(delSel()));
    QPoint globalpos = ui->pathview->viewport()->mapToGlobal(pos);
    dmenu.exec(globalpos);

    delIndex = QModelIndex();
}

void MainWindow::del()
{
    if (delIndex.isValid())
    {
        QModelIndex pathindex = ui->pathview->model()->index(delIndex.row(), 3);
        QString filepath = pathindex.data().toString();

        if (QMessageBox::question(this, tr("confirm delete "), tr("are ypu sure delete file %1").arg(filepath)) != QMessageBox::StandardButton::Yes)
        {
            return;
        }

        bool res = QFile::remove(filepath);

        if (res)
        {
            ui->pathview->model()->removeRows(pathindex.row(), 1);
            QMessageBox::information(this, tr("delete success"), tr("delete file ssuccess ") + filepath);
        }
        else
        {
            QMessageBox::critical(this, tr("delete failure"), tr("delete file failure ") + filepath);
        }
    }

    delIndex = QModelIndex();
}

void MainWindow::delSel()
{
    QModelIndexList clist;
    for (int i = ui->pathview->model()->rowCount() - 1; i >= 0; --i)
    {
         QModelIndex mindex = ui->pathview->model()->index(i, 0);
         if (mindex.data(Qt::CheckStateRole).toBool())
         {
             QModelIndex pathindex = ui->pathview->model()->index(i, 3);
             clist.push_back(pathindex);
         }
    }

    if (clist.isEmpty())
    {
        return;
    }

    if (QMessageBox::question(this, tr("confirm delete "), tr("are ypu sure delete %1 files?").arg(QString::number(clist.size()))) != QMessageBox::StandardButton::Yes)
    {
        return;
    }

    int delfiles = 0;
    int ffiles = 0;
    for (auto index : clist)
    {
        QString filepath = index.data().toString();
        bool res = QFile::remove(filepath);

        if (res)
        {
            ui->pathview->model()->removeRows(index.row(), 1);
            delfiles += 1;
        }
        else
        {
            ffiles += 1;
        }
    }

    if (ffiles == 0)
    {
        QMessageBox::information(this, tr("delete success"), tr("delete %1 files ssuccess ").arg(QString::number(delfiles)));
    }
    else
    {
        QMessageBox::critical(this, tr("delete failure"), tr("delete %1 file failure ").arg(QString::number(ffiles)));
    }
}
