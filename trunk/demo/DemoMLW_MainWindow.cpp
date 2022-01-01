#include "DemoMLW_MainWindow.h"
#include <QLabel>
#include <QDateTime>
#include <QPushButton>

MainWindow::MainWindow( QWidget *parent )
  : QMainWindow( parent )
  , m_CentralWidget( parent )
  , m_LayOut( nullptr )
  , m_ButtonLine(        new QPushButton( "Push me for a message", parent ) )
//  , m_ButtonIncLevel(    new QPushButton( "Increment MaxLevel", parent ) )
  , m_AdjustMaxLevel(    new QSlider( Qt::Horizontal, parent ) )
  , m_ButtonWithDate(    new QPushButton( "With Date", parent ) )
  , m_ButtonWithLevel(   new QPushButton( "With Level", parent ) )
  , m_ButtonWithId(      new QPushButton( "With ID", parent ) )
  , m_ButtonWithId_Short(new QPushButton( "ID is 32 bit", parent ) )
  , m_ButtonWithId_Hex(  new QPushButton( "ID is hex", parent ) )
  , m_Count(0)
  , m_MaxLevel(4), m_digits(8), m_base(16)
  , m_Messages( parent, "the Logging is shown here", INT_MAX )
{
  // --- define main windows widget as central one:
  setCentralWidget( &m_CentralWidget );

  m_LayOut = new QVBoxLayout();
  m_LayOut->addWidget( new QLabel( "This is a Label, just as a placeholder for your ideas", this ), 0, Qt::AlignTop | Qt::AlignCenter );
  m_LayOut->addWidget( m_ButtonLine );
  m_LayOut->addWidget( m_ButtonWithDate );    m_ButtonWithDate->setCheckable(true);    m_ButtonWithDate->setChecked(true);
  m_LayOut->addWidget( m_ButtonWithLevel);    m_ButtonWithLevel->setCheckable(true);   m_ButtonWithLevel->setChecked(true);
  m_LayOut->addWidget( m_ButtonWithId   );    m_ButtonWithId->setCheckable(true);      m_ButtonWithId->setChecked(false);
  m_LayOut->addWidget( m_ButtonWithId_Short); m_ButtonWithId_Short->setCheckable(true);m_ButtonWithId_Short->setChecked(true);
  m_LayOut->addWidget( m_ButtonWithId_Hex);   m_ButtonWithId_Hex->setCheckable(true);  m_ButtonWithId_Hex->setChecked(false);
  m_LayOut->addWidget( m_AdjustMaxLevel );
  m_LayOut->addWidget( &m_Messages, 30, Qt::AlignBottom );
  m_CentralWidget.setLayout( m_LayOut );

  connect( m_ButtonLine, SIGNAL(clicked(bool)), this, SLOT(on_cklicked(bool)) );

  connect( this, SIGNAL(sig_logline(                                         int,           const QString&)), &m_Messages, SLOT(on_logline(int, const QString&)) );
  connect( this, SIGNAL(sig_logline(const QDateTime&,                        int,           const QString&)), &m_Messages, SLOT(on_logline(const QDateTime&,int,const QString&)) );
  connect( this, SIGNAL(sig_logline(                       const QDateTime&, int, uint64_t, const QString&)), &m_Messages, SLOT(on_logline(const QDateTime&,int,uint64_t,const QString&)) );
  connect( this, SIGNAL(sig_logline(const FileLineFunc_t&, const QDateTime&, int, uint64_t, const QString&)), &m_Messages, SLOT(on_logline(const FileLineFunc_t&,const QDateTime&,int,uint64_t,const QString&)) );
  /////
  //connect( m_ButtonIncLevel,SIGNAL( clicked(bool) ), this,        SLOT(on_increment_level(bool)  ) );
  connect( m_AdjustMaxLevel,SIGNAL(valueChanged(int)), this,        SLOT(on_set_level(int)         ) );
  connect( m_ButtonWithDate,  SIGNAL( toggled(bool) ), &m_Messages, SLOT(on_setWithDate(bool)      ) );
  connect( m_ButtonWithLevel, SIGNAL( toggled(bool) ), &m_Messages, SLOT(on_setWithLevel(bool)     ) );
  connect( m_ButtonWithId,    SIGNAL( toggled(bool) ), &m_Messages, SLOT(on_setWithNumID(bool)     ) );
  connect( m_ButtonWithId_Short,SIGNAL(toggled(bool)), this,        SLOT(on_shortID(bool)          ) );
  connect( m_ButtonWithId_Hex,SIGNAL( toggled(bool) ), this,        SLOT(on_HexID(bool)            ) );
  connect( this, SIGNAL( sig_setMaxLevel(int)       ), &m_Messages, SLOT(on_setMaxLevel(int)       ) );
  connect( this, SIGNAL( sig_setNumIDFormat(int,int)), &m_Messages, SLOT(on_setNumIDFormat(int,int)) );
  connect( this, SIGNAL( sig_setInitLevel(int))      , m_AdjustMaxLevel, SLOT(setValue(int)        ) );

  m_AdjustMaxLevel->setTickPosition( QSlider::TicksAbove );
  m_AdjustMaxLevel->setTickInterval( 1 );
  m_AdjustMaxLevel->setMinimum(-1);
  m_AdjustMaxLevel->setMaximum(8);

  emit sig_setInitLevel(INT_MAX);
  this->resize( 640, 320 );
  m_Messages.updateGeometry();
  m_Messages.show();
  m_Messages.update();
}


MainWindow::~MainWindow()
{}


void MainWindow::on_shortID(bool checked )
{
  m_digits = (checked) ? 8 : 16;
  emit sig_logline( 4, QString("sig_setNumIDFormat(%1,%2)").arg(m_digits).arg(m_base) );
  emit sig_setNumIDFormat( m_digits, m_base );
}


void MainWindow::on_HexID(bool checked )
{
  m_base = (checked) ? 16 : 10;
  emit sig_logline( 4, QString("sig_setNumIDFormat(%1,%2)").arg(m_digits).arg(m_base) );
  emit sig_setNumIDFormat( m_digits, m_base );
}

#if 0
void MainWindow::on_increment_level(bool/*clicked*/)
{
  m_MaxLevel++;
  if( m_MaxLevel>7 ) m_MaxLevel=0;
  emit sig_logline( 4, QString("increment max. Log Level to %1").arg(m_MaxLevel) );
  emit sig_setMaxLevel( m_MaxLevel );
  m_ButtonIncLevel->setText( QString("set MaxLevel to %1+1").arg(m_MaxLevel) );
}
#endif

void MainWindow::on_set_level(int requestedMaxLevel)
{
  m_MaxLevel=requestedMaxLevel;
  if( m_MaxLevel>7 ) m_MaxLevel=7;
  if( m_MaxLevel<0 ) m_MaxLevel=-1;
  emit sig_logline( 4, QString("increment max. Log Level to %1").arg(m_MaxLevel) );
  emit sig_setMaxLevel( m_MaxLevel );
  emit sig_setInitLevel(m_MaxLevel);
}


void MainWindow::on_cklicked(bool/*clicked*/)
{
  QDateTime Time( QDate::currentDate(), QTime::currentTime(), Qt::LocalTime );
  FileLineFunc_t CodeLocation( __BASE_FILE__, __LINE__, __PRETTY_FUNCTION__ );

  switch( m_Count % 8 )
  {
    case 0:
      emit sig_logline( 7, "this is the simplest log entry, Level=DEBUG" );
      break;
    case 1:
      emit sig_logline( 6, QString("this is a very simple log entry number %1. next will be more compilicated").arg(m_Count) );
      break;
    case 2:
      Time = Time.addDays(1);       Time = Time.addMonths(1);      Time = Time.addYears(1);
      Time = Time.addSecs( -3600 );
      emit sig_logline( Time, 5, "a line with 'faked' time stamp" );
      break;
    case 3:
      emit sig_logline( Time, 4, m_Count, QString("this is a line with just the numeric ID, which should be identical to %1").arg(m_Count) );
      break;
    case 4:
      emit sig_logline( CodeLocation, Time, 3, m_Count, QString("this is a line with code location and numeric ID, which should be identical to %1").arg(m_Count) );
      break;
    case 5:
      emit sig_logline( CodeLocation, Time, 2, m_Count, "this Level is 'Critical', the next one can stop your application" );
      break;
    case 6:
      emit sig_logline( CodeLocation, Time, 1, m_Count, "this Level is 'FATAL' and WILL stop your application if enabled qDebug() like logging" );
      break;
    case 7:
      emit sig_logline( CodeLocation, Time, 0, m_Count, "this Level is 'EMERGENCY' and MUST stop your application if enabled qDebug() like logging" );
      break;
  }
  m_Count++;
}
