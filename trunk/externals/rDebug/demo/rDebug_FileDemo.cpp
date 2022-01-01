#include "rDebug_FileDemo.h"

#include <QtGlobal>  // for statements like if (QT_VERSION >= 0x050000)
#if defined(QT_VERSION) && (QT_VERSION>=0x050000)
#    include <QStandardPaths>
#elif defined(QT_VERSION) && (QT_VERSION>=0x040000)
#    include <QDesktopServices>
#else
#    error "please tell me, how to access system paths"
#endif

#include <QCoreApplication>
#include <QTimer>

//#include <QDebug>
#include "../src/rDebug.h"
#include "../src/rDebugLevel.h"


QString getDataLocation( void )
{
#   if (QT_VERSION >= 0x050000)
    return QStandardPaths::writableLocation(QStandardPaths::DataLocation);
#   else
    return QDesktopServices::storageLocation(QDesktopServices::DataLocation);
#   endif
}


QString getHomeLocation( void )
{
#   if (QT_VERSION >= 0x050000)
    return QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
#   else
    return QDesktopServices::storageLocation(QDesktopServices::HomeLocation);
#   endif
}



#define APPLICATION_NAME "rDebug_FileDemo"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    JobRunner* simple_job = new JobRunner( "simple_job", &a );
    JobRunner* second_job = new JobRunner( "second_job (All)", &a, rDebugLevel::rMsgType::All );

    /* for the demo, it's okay to create the logfile just parallel to the current directory
     * but in real life, you'll need it in tmp or userdir
     * like getDataLocation()
     *      getHomeLocation()
     * giving an empty filename will auto generate <tmp>/<appname>.log
     */
    rDebug_Filewriter rLogFile( QString( getHomeLocation() + '/' + APPLICATION_NAME ".log" ),
                                rDebugLevel::rMsgType::All,
                                3/*Max Backups of full logfiles*/,
                                4096 /*4k per file would be nice for the demo, but internally we use at least 64k, sorry*/ );


  //QObject::connect( simple_job, SIGNAL(done()), &a,         SLOT(quit())    );
    QObject::connect( simple_job, SIGNAL(done()), simple_job, SLOT(on_done()) );
    QObject::connect( simple_job, SIGNAL(done()), second_job, SLOT(on_run()) );
    QObject::connect( second_job, SIGNAL(done()), second_job, SLOT(on_done())    );
    QObject::connect( second_job, SIGNAL(done()), &a,         SLOT(quit())    );

    // perform the job from applications event loop.
    QTimer::singleShot( 10/*ms*/, simple_job, SLOT(on_run()) );

    return a.exec();
}

