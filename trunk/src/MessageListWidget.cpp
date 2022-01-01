/**
 * Project "MessageListWidget for Qt"
 *
 * MessageListWidget.cpp
 * 
 * Just want to see your debugs/logs in a QtWidget, like QListView? 
 * Want to start with Qt, and like to work together, bring your ideas and code?
 *
 * You are welcome, here we go!
 *
 * This is mostly a simplified implementation of a line-by-line list window which
 * can reside at the bottom of your application and show what happens inside, or 
 * give the user some journal information. All the stuff a lot of communication
 * related graphical UI programs are doing since years.
 *
 * Also want to have this?
 *
 * You simply add this two files to your project and bind its SLOT with a SIGNAL
 * Currently, I'm in a demo state, where I include parts of my other project, "rDebug".
 * We'll get more flexible in the future. For now, just delete the unwanted include "rDebugCodeloc.h"
 * or let it as it is, or re-implement it (just a 10-liner class).
 * Basically, you just need to connect your SIGNAL to one of the four SLOTs.
 * best to start with the simplest.
 *
*
 * Plans:
 * add QSettings for storing/loading last view
 * some dialog elements to change the filtering LogLevel, change font, auto scroll, minimize
 * maybe grid view to arrange / hide some colums like the date
 * maybe colored lines depending on log severity
 * maybe icons instead log level names
 *
 * finished Plans::
 * add QObject::tr() to the LogLevel names
 * 
 * copyright 2019 Sergeant Kolja, GERMANY
 * 
 * distributed under the terms of the 2-clause license also known as "Simplified BSD License" or "FreeBSD License"
 * License is compatible with GPL and LGPL
 */ 

#include "MessageListWidget.h"

#include <QFont>
#include <QDateTime>

MessageListWidget::MessageListWidget(QWidget *parent, const QString& objectName, int initialMaxLevel)
//  : QWidget(parent)
  : QListWidget(parent)
  , mpListing(this)
  , mMaxLevel(initialMaxLevel)
  , mWithDate(true)
  , mWithLevel(true)
  , mWithID(false)
  , mWithID_digits(8)
  , mWithID_base(10)
{
  setObjectName( objectName );
  setToolTip( objectName );  // helpful for debugging, may be removed later
  setFont( QFont( "Courier", 8) );
}


MessageListWidget::~MessageListWidget()
{
  while( mpListing->count() )
  {
    QListWidgetItem *pItemTodelete = mpListing->takeItem(0);
    delete pItemTodelete; pItemTodelete=nullptr;
  }
}

/*
void MessageListWidget::setToolTip( const QString& ToolTip )
{
    setToolTip( ToolTip );
}*/


void MessageListWidget::paintMessage( const QDateTime& Time, int Lvl, uint64_t LogId, const QString& line )
{
  if( mMaxLevel < Lvl )
    return;

  QString Message("");

  if( mWithDate  ) Message.append( getDateTimeStr( Time ) ).append(' ');
  if( mWithLevel ) Message.append("[").append( getLevelName( Lvl ) ).append("] ");
  if( mWithID    ) Message.append( getLogIdStr( LogId ) ).append(", ");
  Message.append( line );

  QListWidgetItem *newItem = new QListWidgetItem;
  newItem->setText( Message );
  // can also have icon and so on ...
  mpListing->addItem( newItem );
  while( mpListing->count() > 500 )
  {
    QListWidgetItem *pItemTodelete = mpListing->takeItem(0);
    delete pItemTodelete;
  }
  mpListing->scrollToBottom();
}


void MessageListWidget::on_logline( const FileLineFunc_t& /*CodeLocation*/, const QDateTime& Time, int Level, uint64_t LogId, const QString& line )
{
  paintMessage( Time, Level, LogId, line );
}

void MessageListWidget::on_logline( const QDateTime& Time, int Level, uint64_t LogId, const QString& line )
{
  paintMessage( Time, Level, LogId, line );
}

void MessageListWidget::on_logline( const QDateTime& Time, int Level, const QString& line )
{
  paintMessage( Time, Level, 0, line );
}

void MessageListWidget::on_logline(int Level, const QString& line)
{
  QDateTime Now( QDate::currentDate(), QTime::currentTime(), Qt::LocalTime );
  paintMessage( Now, Level, 0llu, line );
}


void MessageListWidget::on_setMaxLevel(int NewMaxLevel)
{
    mMaxLevel = NewMaxLevel;
}


void MessageListWidget::on_setWithDate( bool WithDate )
{
    mWithDate = WithDate;
}


void MessageListWidget::on_setWithLevel( bool WithLevel )
{
    mWithLevel = WithLevel;
}


void MessageListWidget::on_setWithNumID( bool WithID )
{
    mWithID = WithID;
}


void MessageListWidget::on_setNumIDFormat(int digits, int base )
{
    mWithID_digits = digits;
    mWithID_base = base;
}




QString MessageListWidget::getLevelName( int Level ) const
{
    QString LevelName;

    switch( Level )
    {
        case 7 : LevelName = QObject::tr("Debg","[Debg], Debug-level messages Messages that contain information normally of use only when debugging a program."); break;
        case 6 : LevelName = QObject::tr("Info","[Info], message type of unspecified messages"); break;
        case 5 : LevelName = QObject::tr("Note","[Note], Normal but significant conditions. Conditions that are not error conditions, but that may require special handling."); break;
        case 4 : LevelName = QObject::tr("Warn","[Warn], Warning conditions"); break;
        case 3 : LevelName = QObject::tr("Err!","[Err!], Error conditions"); break;
        case 2 : LevelName = QObject::tr("Crit","[Crit], Critical conditions, like hard device errors."); break;
        case 1 : LevelName = QObject::tr("Alrt","[Alrt], Action must be taken immediately, A condition that should be corrected immediately, such as a corrupted system database."); break;
        case 0 : // fall through
        default: LevelName = QObject::tr("Emrg","[Emrg], System is unusable. A panic condition."); break;
    }
    return LevelName;
}


QString MessageListWidget::getDateTimeStr(const QDateTime& Time) const
{
  QString TimestampAsString( QString("%1").arg( Time.toString(QObject::tr("yyyy-MM-dd HH:mm:ss,zzz","local date time format for logging"))) );
  return TimestampAsString;
}


QString MessageListWidget::getLogIdStr(uint64_t LogId) const
{
  if( mWithID_digits <= 8 && mWithID_base != 16) // typical 8-digit-fixed size logid
  {   return QString("%1").arg( LogId, mWithID_digits, mWithID_base, QLatin1Char('0') );
  }
  else if( mWithID_digits <= 8 && mWithID_base == 16 )// large numbers as hex
  {   return QString("0x%1").arg( LogId, mWithID_digits, mWithID_base, QLatin1Char('0') );
  }
  else if( mWithID_base != 16 )// large numbers
  {   return QString("%1:%2").arg( LogId>>32, mWithID_digits-8, mWithID_base, QLatin1Char('0') ).arg( LogId&0xFFFFFFFF, 8, mWithID_base, QLatin1Char('0') );
  }
  else // large numbers as hex with colon in the middle
  {   return QString("0x%1:%2").arg( LogId>>32, mWithID_digits-8, 16, QLatin1Char('0') ).arg( LogId&0xFFFFFFFF, 8, 16, QLatin1Char('0') );
  }
}
