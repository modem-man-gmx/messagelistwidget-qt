#include "rDebug_SignalSlotDemo.h"

#include <QCoreApplication>
#include <QTimer>
#include <QTextStream>

//#include <QDebug>
#include "../src/rDebug.h"
#include "../src/rDebugLevel.h"







int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    JobRunner* simple_job = new JobRunner( "simple_job", &a );
    JobRunner* second_job = new JobRunner( "second_job (All)", &a, rDebugLevel::rMsgType::All );

    SigHandler sighandler;
    rDebug_Signaller rLogSignalSlot( rDebugLevel::rMsgType::All );

    QObject::connect( simple_job, SIGNAL(setMaxLevel(int)), &rLogSignalSlot, SLOT(sig_setMaxLevel(int)) );
    QObject::connect( second_job, SIGNAL(setMaxLevel(int)), &rLogSignalSlot, SLOT(sig_setMaxLevel(int)) );

    QObject::connect( &rLogSignalSlot, SIGNAL(sig_logline(const FileLineFunc_t&,const QDateTime&,int,uint64_t,const QString&)),
                      &sighandler,     SLOT(   on_logline(const FileLineFunc_t&,const QDateTime&,int,uint64_t,const QString&)) );



    QObject::connect( simple_job, SIGNAL(done()), simple_job, SLOT(on_done()) );
    QObject::connect( simple_job, SIGNAL(done()), second_job, SLOT(on_run()) );
    QObject::connect( second_job, SIGNAL(done()), second_job, SLOT(on_done())    );
    QObject::connect( second_job, SIGNAL(done()), &a,         SLOT(quit())    );

    // perform the job from applications event loop.
    QTimer::singleShot( 10/*ms*/, simple_job, SLOT(on_run()) );

    return a.exec();
}

