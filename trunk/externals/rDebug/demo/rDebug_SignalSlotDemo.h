#ifndef RDEBUG_SIGNALSLOT_H
#define RDEBUG_SIGNALSLOT_H

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
      rDebugBase::setMaxLevel(rDebugLevel::rMsgType::Emergency);
    }


signals:
    void done(void);
    void setMaxLevel(int);

public slots:
    void on_run(void)
    {   // Do processing here
        qDebug() << __PRETTY_FUNCTION__ << "running...";

        if( m_JobName=="simple_job" )
        {   emit setMaxLevel( 7 );
        }

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



class SigHandler : public QObject
{
Q_OBJECT

public:
    SigHandler( QObject *parent = nullptr )
      : QObject(parent)
      , out(stdout)
      { out << "Here we go with the SIGNAL/SLOT parts of the rDebug" << endl;
      }

    ~SigHandler()
    { out << "Here we are done with the SIGNAL/SLOT parts of the rDebug" << endl;
    }

public slots:
    void on_logline( const FileLineFunc_t& CodeLocation, const QDateTime& Time, int Level, uint64_t LogId, const QString& line )
    {
      //out << "----------[" << Level << "]-----------------" << endl;
      out << QString("SIGNAL/SLOT: %1: [%2] {id:%3} %4 (from: %5:%6 - %7)")
                    .arg(Time.toString("yyyy-MM-dd HH:mm:ss,zzz"))
                    .arg(Level)
                    .arg(LogId)
                    .arg(line)
                    .arg(CodeLocation.mFile).arg(CodeLocation.mLine).arg(CodeLocation.mFunc)
          << endl;
    }

private:
    QTextStream out;
};


#endif // RDEBUG_SIGNALSLOT_H
