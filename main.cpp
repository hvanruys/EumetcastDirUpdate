#include <QtCore>
#include <QCoreApplication>
#include <QSettings>
#include <QDir>
#include <QDate>
#include <QDebug>
#include <QRegularExpression>
#include <QBasicTimer>
#include <QTimer>

struct FileTemplate {
    QString filetemplate;
    int startdate;
};
QList<FileTemplate> filetemplates;
QList<QString> segmentdirectories;

QStringList completelist;
QList<bool> complete_todelete;
QList<int> complete_startdate;

void CleanupDirectories(bool bListfiles);
void MoveTheFiles(const QString &thedir, bool bListfiles);
void MoveToDirectory(QString filename, int thestartdate, QDir dir);

void MyRun()
{
    CleanupDirectories(true);
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);


    QSettings settings("EumetcastWatcherOut.ini", QSettings::IniFormat);

    int size = settings.beginReadArray("filetemplates");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        FileTemplate mytemplate;
        mytemplate.filetemplate = settings.value("template").toString();
        mytemplate.startdate = settings.value("startdate").toInt();
        filetemplates.append(mytemplate);
    }
    settings.endArray();

    //    for(int i = 0; i < filetemplates.size(); i++)
    //    {
    //        printf("%d - %s\n", filetemplates.at(i).startdate, filetemplates.at(i).filetemplate.toStdString().c_str());
    //    }

    size = settings.beginReadArray("segmentdirectories");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        QString mydirectory;
        mydirectory = settings.value("directory").toString();
        segmentdirectories.append(mydirectory);
    }
    settings.endArray();

    //    for(int i = 0; i < segmentdirectories.size(); i++)
    //    {
    //        printf("%s\n", segmentdirectories.at(i).toStdString().c_str());
    //    }

    CleanupDirectories(false);

    QTimer *timer = new QTimer(&a);
    QObject::connect(timer, &QTimer::timeout, &a, MyRun);
    timer->start(5000);

    return a.exec();
}

void CleanupDirectories(bool bListfiles)
{
    if (segmentdirectories.count() > 0)
    {
        QStringList::Iterator its = segmentdirectories.begin();

        while( its != segmentdirectories.end() )
        {
            QString stits = *its;
            MoveTheFiles(stits, bListfiles);
            ++its;
        }
    }
}

void MoveTheFiles(const QString &thedir, bool bListfiles)
{
    QString filename;
    QString restfile;
    long count = 0;
    bool gelukt = false;
    QDir dir( thedir );
    QString fileyear;
    QString filemonth;
    QString fileday;

    int thecount = 0;

    completelist.clear();
    complete_todelete.clear();
    complete_startdate.clear();

    dir.setFilter(QDir::Files | QDir::NoSymLinks);
    completelist = dir.entryList();
    for(int i = 0; i < completelist.count(); i++)
    {
        complete_todelete.append(true);
        complete_startdate.append(0);
    }

    //qDebug() << "MoveTheFiles dir = " << thedir;

    if (filetemplates.count() > 0)
    {

        QList<FileTemplate>::Iterator itfile = filetemplates.begin();

        while( itfile != filetemplates.end() )
        {
            thecount = 0;
            QString wildcard = QRegularExpression::wildcardToRegularExpression((*itfile).filetemplate);
            QRegularExpression re(wildcard);

            int intfrom = 0;
            intfrom = completelist.indexOf(re, intfrom);
            while(intfrom != -1)
            {
                filename = completelist.at(intfrom);
                complete_todelete[intfrom] = false;
                complete_startdate[intfrom] = (*itfile).startdate;
                thecount++;
                intfrom = completelist.indexOf(re, intfrom + 1);
            }
            //qDebug() << thecount << " " << (*itfile).filetemplate;

            ++itfile;
        }
        for(int i = 0; i < completelist.count(); i++)
        {
            if(complete_todelete.at(i) == false)
            {
                filename = completelist.at(i);
                if(bListfiles)
                    qDebug() << "File " << filename << " moved";
                MoveToDirectory(filename, complete_startdate.at(i), dir);
            }
            else
            {
                filename = completelist.at(i);
                if(bListfiles)
                    qDebug() << "File " << filename << " deleted";
                dir.remove(filename);
            }
        }

    }


}

void MoveToDirectory(QString filename, int thestartdate, QDir dir)
{
    QString fileyear;
    QString filemonth;
    QString fileday;
    bool gelukt;

    if(filename.mid(0, 6) == "OR_ABI")
    {
        int nyear = filename.mid( thestartdate, 4).toInt();
        int ndayofyear = filename.mid( thestartdate + 4, 3).toInt();
        QDate filedate = QDate(nyear, 1, 1).addDays(ndayofyear - 1);
        fileyear = filedate.toString("yyyy");
        filemonth = filedate.toString("MM");
        fileday = filedate.toString("dd");
    }
    else
    {
        fileyear = filename.mid( thestartdate, 4);
        filemonth = filename.mid( thestartdate + 4, 2);
        fileday = filename.mid( thestartdate + 6, 2);
    }

    QDir dirdateyear(dir.absolutePath() + "/" + fileyear);
    QDir dirdatemonth(dir.absolutePath() + "/" + fileyear + "/" + filemonth);
    QDir dirdateday(dir.absolutePath() + "/" + fileyear + "/" + filemonth + "/" + fileday);

    gelukt = false;
    if (!dirdateyear.exists())
    {
        gelukt = dir.mkdir(dir.absolutePath() + "/" + fileyear);
        //qDebug() << dir.absolutePath() + "/" + fileyear + " " + (gelukt ? "OK" : "not OK");
    }
    if (!dirdatemonth.exists())
    {
        gelukt = dir.mkdir(dir.absolutePath() + "/" + fileyear + "/" + filemonth);
        //qDebug() << dir.absolutePath() + "/" + fileyear + "/" + filemonth  + " " + (gelukt ? "OK" : "not OK");
    }
    if (!dirdateday.exists())
    {
        gelukt = dir.mkdir(dir.absolutePath() + "/" + fileyear + "/" + filemonth + "/" + fileday);
        //qDebug() << dir.absolutePath() + "/" + fileyear + "/" + filemonth + "/" + fileday  + " " + (gelukt ? "OK" : "not OK");
    }
    if (!dirdateday.exists(filename))
    {
        gelukt = dir.rename(dir.absolutePath() + "/" + filename,
                            dir.absolutePath() + "/" +  fileyear + "/" + filemonth + "/" + fileday + "/" + filename);

    }

}

//#include "main.moc"

//void inspectDirectories()
//{
//    printf("start inspectDirectories()\n");

//    if (segmentdirectories.count() > 0)
//    {
//        QStringList::Iterator its = segmentdirectories.begin();

//        while( its != segmentdirectories.end() )
//        {
//            QString stits = *its;
//            inspectDirectory(stits);
//            ++its;
//        }
//    }

//}

//void MoveTheFiles(const QString &thedir)
//{
//    QString filename;
//    QString restfile;
//    long count = 0;
//    bool gelukt = false;
//    QDir dir( thedir );
//    QString fileyear;
//    QString filemonth;
//    QString fileday;

//    completelist.clear();
//    complete_todelete.clear();

//    dir.setFilter(QDir::Files | QDir::NoSymLinks);
//    completelist = dir.entryList();
//    for(int i = 0; i < completelist.count(); i++)
//    {
//        complete_todelete.append(false);
//    }

//    if (filetemplates.count() > 0)
//    {

//        QList<FileTemplate>::Iterator itfile = filetemplates.begin();

//        while( itfile != filetemplates.end() )
//        {
//            QDir filterdir(thedir);

//            int thestartdate = (*itfile).startdate;
//            QStringList filters;
//            filters << (*itfile).filetemplate;

//            filterdir.setFilter(QDir::Files | QDir::NoSymLinks);
//            filterdir.setNameFilters(filters);
//            QStringList filterlist = filterdir.entryList();
//            qDebug() << (*itfile).filetemplate << " startdate = " << (*itfile).startdate << " #listentries = " << filterlist.count();

//            for (int j = 0; j < filterlist.size(); ++j)
//            {
//                qDebug() << filterlist.at(j);
//                filename = filterlist.at(j);
//                if(filename.mid(0, 6) == "OR_ABI")
//                {
//                    int nyear = filename.mid( thestartdate, 4).toInt();
//                    int ndayofyear = filename.mid( thestartdate + 4, 3).toInt();
//                    QDate filedate = QDate(nyear, 1, 1).addDays(ndayofyear - 1);
//                    fileyear = filedate.toString("yyyy");
//                    filemonth = filedate.toString("MM");
//                    fileday = filedate.toString("dd");
//                }
//                else
//                {
//                    fileyear = filename.mid( thestartdate, 4);
//                    filemonth = filename.mid( thestartdate + 4, 2);
//                    fileday = filename.mid( thestartdate + 6, 2);
//                }

//                QDir dirdateyear(dir.absolutePath() + "/" + fileyear);
//                QDir dirdatemonth(dir.absolutePath() + "/" + fileyear + "/" + filemonth);
//                QDir dirdateday(dir.absolutePath() + "/" + fileyear + "/" + filemonth + "/" + fileday);

//                gelukt = false;
//                if (!dirdateyear.exists())
//                {
//                    gelukt = dir.mkdir(dir.absolutePath() + "/" + fileyear);
//                    qDebug() << dir.absolutePath() + "/" + fileyear + " " + (gelukt ? "OK" : "not OK");
//                }
//                if (!dirdatemonth.exists())
//                {
//                    gelukt = dir.mkdir(dir.absolutePath() + "/" + fileyear + "/" + filemonth);
//                    qDebug() << dir.absolutePath() + "/" + fileyear + "/" + filemonth  + " " + (gelukt ? "OK" : "not OK");
//                }
//                if (!dirdateday.exists())
//                {
//                    gelukt = dir.mkdir(dir.absolutePath() + "/" + fileyear + "/" + filemonth + "/" + fileday);
//                    qDebug() << dir.absolutePath() + "/" + fileyear + "/" + filemonth + "/" + fileday  + " " + (gelukt ? "OK" : "not OK");
//                }
//                if (!dirdateday.exists(filename))
//                {
//                    gelukt = dir.rename(dir.absolutePath() + "/" + filename,
//                                        dir.absolutePath() + "/" +  fileyear + "/" + filemonth + "/" + fileday + "/" + filename);
//                    QByteArray datagram = (dir.absolutePath() + "/" +  fileyear + "/" + filemonth + "/" + fileday + "/" + filename).toLatin1();

//                }
//            }
//            ++itfile;
//        }
//    }

//}

//void DeleteTheFiles(const QString &thedir)
//{
//    QDir dir( thedir );
//    QString filename;

//    dir.setFilter(QDir::Files | QDir::NoSymLinks);
//    QStringList list = dir.entryList();
//    for (int i = 0; i < list.size(); ++i)
//    {
//        filename = list.at(i);
//        qDebug() << "Removing file " << filename;
//        dir.remove(filename);
//    }
//}

//bool inspectDirectory(const QString &thedir)
//{
//    QString filename;
//    QString restfile;
//    long count = 0;
//    bool gelukt = false;
//    bool movefile = false;
//    QDir dir( thedir );
//    QString fileyear;
//    QString filemonth;
//    QString fileday;

//    dir.setFilter(QDir::Files | QDir::NoSymLinks);
//    dir.setSorting(QDir::Name); //::Time);
//    QStringList list = dir.entryList();

//    for (int i = 0; i < list.size(); ++i)
//    {
//        filename = list.at(i);
//        if (filetemplates.count() > 0)
//        {

//            QList<FileTemplate>::Iterator itfile = filetemplates.begin();

//            while( itfile != filetemplates.end() )
//            {
//                //qDebug() << *itfile << "  " << *itdate;
//                QDir filterdir(thedir);

//                int thestartdate = (*itfile).startdate;
//                QStringList filters;
//                filters << (*itfile).filetemplate;

//                filterdir.setFilter(QDir::Files | QDir::NoSymLinks);
//                filterdir.setSorting(QDir::Name); //::Time);
//                filterdir.setNameFilters(filters);
//                QStringList filterlist = filterdir.entryList();

//                movefile = false;
//                for (int j = 0; j < filterlist.size(); ++j)
//                {
//                    if(list.at(i) == filterlist.at(j))
//                    {
//                        movefile = true;
//                        break;
//                    }
//                }
//                if (movefile)
//                {
//                    if(filename.mid(0, 6) == "OR_ABI")
//                    {
//                        int nyear = filename.mid( thestartdate, 4).toInt();
//                        int ndayofyear = filename.mid( thestartdate + 4, 3).toInt();
//                        QDate filedate = QDate(nyear, 1, 1).addDays(ndayofyear - 1);
//                        fileyear = filedate.toString("yyyy");
//                        filemonth = filedate.toString("MM");
//                        fileday = filedate.toString("dd");
//                    }
//                    else
//                    {
//                        fileyear = filename.mid( thestartdate, 4);
//                        filemonth = filename.mid( thestartdate + 4, 2);
//                        fileday = filename.mid( thestartdate + 6, 2);
//                    }

//                    QDir dirdateyear(dir.absolutePath() + "/" + fileyear);
//                    QDir dirdatemonth(dir.absolutePath() + "/" + fileyear + "/" + filemonth);
//                    QDir dirdateday(dir.absolutePath() + "/" + fileyear + "/" + filemonth + "/" + fileday);

//                    gelukt = false;
//                    if (!dirdateyear.exists())
//                    {
//                        gelukt = dir.mkdir(dir.absolutePath() + "/" + fileyear);
//                        qDebug() << dir.absolutePath() + "/" + fileyear + " " + (gelukt ? "OK" : "not OK");
//                    }
//                    if (!dirdatemonth.exists())
//                    {
//                        gelukt = dir.mkdir(dir.absolutePath() + "/" + fileyear + "/" + filemonth);
//                        qDebug() << dir.absolutePath() + "/" + fileyear + "/" + filemonth  + " " + (gelukt ? "OK" : "not OK");
//                    }
//                    if (!dirdateday.exists())
//                    {
//                        gelukt = dir.mkdir(dir.absolutePath() + "/" + fileyear + "/" + filemonth + "/" + fileday);
//                        qDebug() << dir.absolutePath() + "/" + fileyear + "/" + filemonth + "/" + fileday  + " " + (gelukt ? "OK" : "not OK");
//                    }
//                    if (!dirdateday.exists(filename))
//                    {
//                        gelukt = dir.rename(dir.absolutePath() + "/" + filename,
//                                            dir.absolutePath() + "/" +  fileyear + "/" + filemonth + "/" + fileday + "/" + filename);
//                        QByteArray datagram = (dir.absolutePath() + "/" +  fileyear + "/" + filemonth + "/" + fileday + "/" + filename).toLatin1();

//                    }


//                    break;
//                }
//                else
//                {
//                }

//                ++itfile;
//            }

//            gelukt = dir.remove(filename);

//        }

//        //        if(sendUDP) // not in thread ....
//        //        {
//        //            if(movefile)
//        //                writeTolistwidget(" File " + dir.absolutePath() + "/" + filename + " moved");
//        //            else
//        //                writeTolistwidget(" File " + dir.absolutePath() + "/" + filename + " is deleted");
//        //        }

//        qApp->processEvents();

//        ////////////////////////////////////////////////////////////////////////////////////////
//        //    this->thread()->msleep(500);
//        //////////////////////////////////////////////////////////////////////////////////////////


//    }

//    return movefile;
//}

