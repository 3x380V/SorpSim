/*! \file main.cpp
    \brief SorpSim application entry point

    This file is part of SorpSim and is distributed under terms in the file LICENSE.

    Developed by Zhiyao Yang and Dr. Ming Qu for ORNL.

    \author Zhiyao Yang (zhiyaoYang)
    \author Dr. Ming Qu
    \author Nicholas Fette (nfette)

    \copyright 2015, UT-Battelle, LLC
    \copyright 2017-2018, Nicholas Fette

*/

#include "mainwindow.h"
#include "sorputils.h"
#include <QApplication>
#include <QDateTime>
#include <QDomImplementation>
#include <QFile>
#include <QTextStream>

/// start a new mainwindow and leave all interactions to the mainwindow
/// - once the mainwindow is closed, the application is closed as well
/// - first subroutine to run at SorpSim's launch
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QDomImplementation::setInvalidDataPolicy(QDomImplementation::ReturnNullNode);

    QFile logfile(Sorputils::sorpLog());
    logfile.open(QFile::Append | QFile::Text);
    QTextStream log(&logfile);
    log << QString("[%1] Starting up.").arg(QDateTime::currentDateTime().toString(Qt::ISODate)) << Qt::endl;

    QDir dir = qApp->applicationDirPath();
    log << "applicationDirPath() =" << dir.path() << Qt::endl;
    log << "absolutePath() =" << dir.absolutePath() << Qt::endl;
    log << "canonicalPath =" << dir.canonicalPath() << Qt::endl;

    QDir dir2;
    log << "Default dir =" << dir2.path() << Qt::endl;
    log << "absolutePath =" << dir2.absolutePath() << Qt::endl;
    log << "canonicalPath =" << dir2.canonicalPath() << Qt::endl;

    log << Qt::endl << Qt::endl;
    log.flush();
    logfile.close();

    Sorputils::init();

    MainWindow w;
    w.show();
    return a.exec();
}
