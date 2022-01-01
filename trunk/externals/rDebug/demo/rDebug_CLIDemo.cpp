#include "rDebug_CLIDemo.h"

#include <QCoreApplication>
#include <QTimer>

//#include <QDebug>
#include "../src/rDebug.h"
#include "../src/rDebugLevel.h"


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    JobRunner* simple_job = new JobRunner( "simple_job", &a );
    JobRunner* second_job = new JobRunner( "second_job (All)", &a, rDebugLevel::rMsgType::All );

  //QObject::connect( simple_job, SIGNAL(done()), &a,         SLOT(quit())    );
    QObject::connect( simple_job, SIGNAL(done()), simple_job, SLOT(on_done()) );
    QObject::connect( simple_job, SIGNAL(done()), second_job, SLOT(on_run()) );
    QObject::connect( second_job, SIGNAL(done()), second_job, SLOT(on_done())    );
    QObject::connect( second_job, SIGNAL(done()), &a,         SLOT(quit())    );

    // perform the job from applications event loop.
    QTimer::singleShot( 10/*ms*/, simple_job, SLOT(on_run()) );

    return a.exec();
}
