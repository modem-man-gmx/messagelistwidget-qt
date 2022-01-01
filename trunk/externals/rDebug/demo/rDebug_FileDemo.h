#ifndef RDEBUG_FILEDEMO_H
#define RDEBUG_FILEDEMO_H

#include <QObject>
#include <QTextStream>

//#include <QDebug>
#include "../src/rDebug.h"




class JobRunner : public QObject
{
Q_OBJECT

public:
    JobRunner( const QString& JobName = QString(), QObject *parent = nullptr, rDebugLevel::rMsgType MaxLogLevel = rDebugLevel::rMsgType::Informational )
      : QObject(parent)
      , m_MaxLogLevel(MaxLogLevel)
      , m_JobName(JobName)
    {
      rDebug_GlobalLevel::set(m_MaxLogLevel);
    }


signals:
    void done(void);

public slots:
    void on_run(void)
    {   // Do processing here
        qDebug() << __PRETTY_FUNCTION__ << "running...";


        QTextStream out(stdout);
        out << "Hello, here we are! I am the " << m_JobName << endl;

        uint i=8;
        do
        {
            qDebug() << "qDebug loop" << i <<"performed";
        } while( --i >0 );

        out << "------------------------------" << endl;
        do
        {
            rDebug(i) << "rDebug loop" << i <<"performed";
        } while( ++i <=8 );
        out << "------------------------------" << endl;

        qDebug()    << "qDebug   : C++ Style Debug Message";
        qDebug(        "qDebug   : C-  Style Debug Message" );
        qInfo()     << "qInfo    : C++ Style Info Message";
        qInfo(         "qInfo    : C-  Style Info Message" );
        qWarning()  << "qWarning : C++ Style Warning Message";
        qWarning(      "qWarning : C-  Style Warning Message" );
        qCritical() << "qCritical: C++ Style Critical Error Message";
        qCritical(     "qCritical: C-  Style Critical Error Message" );
        qSystem()   << "qSystem  : C++ Style Critical Error Message";
        qSystem(       "qSystem  : C-  Style Critical Error Message" );
#       if 0 // qFatal will immediately close the app, so we just enable it if we want to test this special behaviour
        // qFatal does not have a C++ style method.
        qFatal(        "qFatal   : C-  Style Fatal Error Message" );
#       endif
        out << "------------------------------" << endl;
        unsigned lvl = static_cast<unsigned>(rDebugLevel::rMsgType::Debug);

        rDebug     () << "rDebug()    : C++ Style, Syslog alike level " << lvl-- << " Message";
        rInfo      () << "rInfo()     : C++ Style, Syslog alike level " << lvl-- << " Message";
        rNote      () << "rNote()     : C++ Style, Syslog alike level " << lvl-- << " Message";
        rWarning   () << "rWarning()  : C++ Style, Syslog alike level " << lvl-- << " Message";
        rError     () << "rError()    : C++ Style, Syslog alike level " << lvl-- << " Message";
        rCritical  () << "rCritical() : C++ Style, Syslog alike level " << lvl-- << " Message";
#       if 0 // rFatal will immediately close the app, so we just enable it if we want to test this special behaviour
        rFatal     () << "rFatal()    : C++ Style, Syslog alike level " << lvl-- << " Message";
        rEmergency () << "rEmergency(): C++ Style, Syslog alike level " << lvl-- << " Message";
        rAlert     () << "rAlert()    : C++ Style, Syslog alike level " << lvl-- << " Message (an alias for qAlert())";
#       endif
        rSystem    () << "rSystem()   : C++ Style, Syslog alike level " << rDebugLevel::rMsgType::Error << " Message (an alias for qSystem())";
        out << "==============================" << endl;
        emit done();
    }

    void on_done(void)
    {   // say goodbye
        QTextStream(stdout) << "=== Good Bye! ===" << endl;
    }


private:
    rDebugLevel::rMsgType  m_MaxLogLevel;
    const QString          m_JobName;
};


#endif // RDEBUG_FILEDEMO_H
