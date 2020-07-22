#include <QtCore>
#include <QCoreApplication>
#include <QSettings>
#include <QDir>
#include <QDate>
#include <QDebug>
#include <QRegularExpression>
#include <QBasicTimer>
#include <QTimer>
#include <iostream>

using namespace std;

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

            ++itfile;
        }
        for(int i = 0; i < completelist.count(); i++)
        {
            if(complete_todelete.at(i) == false)
            {
                filename = completelist.at(i);
                if(bListfiles)
                    cout << "File " << filename.toStdString() << " moved" << endl;
                MoveToDirectory(filename, complete_startdate.at(i), dir);
            }
            else
            {
                filename = completelist.at(i);
                if(bListfiles)
                    cout << "File " << filename.toStdString() << " deleted" << endl;
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
    }
    if (!dirdatemonth.exists())
    {
        gelukt = dir.mkdir(dir.absolutePath() + "/" + fileyear + "/" + filemonth);
    }
    if (!dirdateday.exists())
    {
        gelukt = dir.mkdir(dir.absolutePath() + "/" + fileyear + "/" + filemonth + "/" + fileday);
    }
    if (!dirdateday.exists(filename))
    {
        gelukt = dir.rename(dir.absolutePath() + "/" + filename,
                            dir.absolutePath() + "/" +  fileyear + "/" + filemonth + "/" + fileday + "/" + filename);

    }
    else
        dir.remove(filename);

}

