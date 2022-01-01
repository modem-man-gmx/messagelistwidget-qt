/**
 * Project "rDebug"
 *
 * rDebug.cpp
 * 
 * Loving qDebug? But missing some things? 
 * Just want to see your debugs/logs in a QtWidget, like QListView? 
 * But got crashes with qInstallMsgHandler (4.x) / qInstallMessageHandler (5.x)?
 * Missing file name, line numbers with qDebug 4.x?
 *
 * You are welcome, here we go!
 *
 * This is mostly a simplified re-implementation, which under the hood also uses parts of qDebug(). 
 * You can use
 *    qDebug( "printf mask %s","is worse");
 * or 
 *    qDebug() << "streams" << "are more safe";
 * or a mix of both. You can select between all 7 BSD-Syslog levels (plus "Silent" and "All").
 * A two-liner can send Qt SIGNAL with the data, time stamp, location and level to your QListWidget (or whatever).
 * Another two-liner can write to file and use different log level filtering. 
 * For the start, some features of Qt5, like 
 *    qDebug( &stream_device ) << "are not supported"; // and may never come.
 * 
 * copyright 2019 Sergeant Kolja, GERMANY
 * 
 * distributed under the terms of the 2-clause license also known as "Simplified BSD License" or "FreeBSD License"
 * License is compatible with GPL and LGPL
 */ 
 
#include <QtGlobal>  // for statements like if (QT_VERSION >= 0x050000)
#if defined(QT_VERSION) && (QT_VERSION>=0x050000)
//#    error "QT5"
#elif defined(QT_VERSION) && (QT_VERSION>=0x040000)
//#    error "QT4"
#else
#    error "please tell me, how to access system paths"
#endif

#include <QtGlobal>
#include <QDebug>
#include <QByteArray>
#include <QCoreApplication>
#include <QFileInfo>
#include <QDir>
#include <stdio.h>
#include <stdarg.h>  // va_list

#include "rDebugLevel.h"



void to_xDebug( rDebugLevel::rMsgType Level, const QString& msg )
{
  #pragma GCC diagnostic push
  switch(Level)
  {
    #pragma GCC diagnostic ignored "-Wimplicit-fallthrough=2"
    case rDebugLevel::rMsgType::Emergency    : /*0*/ // intentionally fallthrough
    case rDebugLevel::rMsgType::Alert        : /*1*/ qFatal( "%s", msg.toLocal8Bit().constData() ); //break; Will and shall break the app via abort()
    case rDebugLevel::rMsgType::Critical     : /*2*/ // intentionally fallthrough
    case rDebugLevel::rMsgType::Error        : /*3*/ qCritical( "%s", msg.toLocal8Bit().constData() ); break;
#if !defined( QT_NO_WARNING_OUTPUT ) // suppression of WARNING and HIGHER
    case rDebugLevel::rMsgType::Warning      : /*4*/ qWarning( "%s", msg.toLocal8Bit().constData() ); break;
    #if defined(QT_VERSION) && (QT_VERSION>=0x050000)
    case rDebugLevel::rMsgType::Notice       : /*5*/ qDebug().noquote() << msg; break;
    #elif defined(QT_VERSION) && (QT_VERSION>=0x040000)
    case rDebugLevel::rMsgType::Notice       : /*5*/ qDebug() << msg; break;
    #endif
# if !defined( QT_NO_INFO_OUTPUT ) // suppression of INFO and HIGHER
    #if defined(QT_VERSION) && (QT_VERSION>=0x050000)
    case rDebugLevel::rMsgType::Informational: /*6*/ qDebug().noquote() << msg; break;
    #elif defined(QT_VERSION) && (QT_VERSION>=0x040000)
    case rDebugLevel::rMsgType::Informational: /*6*/ qDebug() << msg; break;
    #endif
# if !defined( QT_NO_DEBUG_OUTPUT ) // suppression of DEBUG (and higher, but there is no higher)
#  if !defined( QT_NO_DEBUG ) // in release builds, we always suppress DEBUG type messages
    #if defined(QT_VERSION) && (QT_VERSION>=0x050000)
    case rDebugLevel::rMsgType::Debug        : /*7*/ qDebug().noquote() << msg; break;
    #elif defined(QT_VERSION) && (QT_VERSION>=0x040000)
    case rDebugLevel::rMsgType::Debug        : /*7*/ qDebug() << msg; break;
    #endif
#  endif // !defined( QT_NO_DEBUG ) // in release builds, we always suppress DEBUG type messages
# endif // !defined( QT_NO_DEBUG_OUTPUT ) // suppression of DEBUG (and higher, but there is no higher)
# endif // !defined( QT_NO_INFO_OUTPUT ) // suppression of INFO and HIGHER
#endif //!defined( QT_NO_WARNING_OUTPUT ) // suppression of WARNING and HIGHER
    default                                  : break;
  }

# if defined( QT_FATAL_WARNINGS )
  if( Level <= rDebugLevel::rMsgType::Warning )
  {  std::abort();  // abort Critical, Error, Warning to fail early
  }
# endif
#pragma GCC diagnostic pop
}


#include "rDebug.h" // this __MUST__ be after the implementation of to_xDebug, to use the overloaded Macros there


#ifndef SYSLOG_FACILITY
#define SYSLOG_FACILITY 16 // 16-23 are application default values
#endif

#ifndef SYSLOG_LEVEL_MAX
#define SYSLOG_LEVEL_MAX rDebugLevel::rMsgType::Warning
#endif

#ifndef SYSLOG_WITH_NUMERIC_8DIGITS_ID
#define SYSLOG_WITH_NUMERIC_8DIGITS_ID true
#endif
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

static bool SkipOutputByPreprocessor( rDebugLevel::rMsgType
#if defined( QT_NO_DEBUG )
                                      Level
#endif
                                    )
{
#   if defined( QT_NO_WARNING_OUTPUT ) // suppression of WARNING and HIGHER
    if( Level >= rDebugLevel::rMsgType::Warning ) return true;
#   endif
#   if defined( QT_NO_INFO_OUTPUT ) // suppression of INFO and HIGHER
    if( Level >= rDebugLevel::rMsgType::Informational ) return true;
#   endif

#   if defined( QT_NO_DEBUG_OUTPUT )
#   if defined( QT_NO_DEBUG ) // in release builds, we always suppress DEBUG type messages
    if( Level >= rDebugLevel::rMsgType::Debug ) return true;
#   endif
#   endif
    return false;
}

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

rDebugLevel::rMsgType rDebug_GlobalLevel::mMaxLevel = SYSLOG_LEVEL_MAX;


rDebug_GlobalLevel::rDebug_GlobalLevel(rDebugLevel::rMsgType MaxLevel)
{
  rDebug_GlobalLevel::mMaxLevel = MaxLevel;
}

void rDebug_GlobalLevel::set(rDebugLevel::rMsgType MaxLevel)
{
  rDebug_GlobalLevel::mMaxLevel = MaxLevel;
}

rDebugLevel::rMsgType rDebug_GlobalLevel::get(void)
{
  return rDebug_GlobalLevel::mMaxLevel;
}

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

rDebugLevel::rMsgType rDebug_Signaller::mMaxLevel = SYSLOG_LEVEL_MAX;
rDebug_Signaller*     rDebug_Signaller::pSignaller = nullptr;


rDebug_Signaller::rDebug_Signaller(rDebugLevel::rMsgType MaxLevel)
{
  rDebug_Signaller::mMaxLevel = MaxLevel;
  if( MaxLevel <= rDebugLevel::rMsgType::Silent )
  { rDebug_Signaller::pSignaller = nullptr;
    return;
  }
  rDebug_Signaller::pSignaller = this;
}


rDebug_Signaller::~rDebug_Signaller()
{
  rDebug_Signaller::pSignaller = nullptr;
}

void rDebug_Signaller::setMaxLevel(rDebugLevel::rMsgType MaxLevel)
{
  rDebug_Signaller::mMaxLevel = MaxLevel;
}

void rDebug_Signaller::signal_line( const FileLineFunc_t& CodeLocation, const QDateTime& Time, rDebugLevel::rMsgType Level, uint64_t LogId, const QString& line )
{
  if( rDebug_Signaller::mMaxLevel >= Level)
  {
    int lvl = static_cast<int>(Level);
    emit sig_logline( CodeLocation, Time, lvl, LogId, line );
  }
}


/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

rDebugLevel::rMsgType rDebug_Filewriter::mMaxLevel = SYSLOG_LEVEL_MAX;
bool                  rDebug_Filewriter::mDumpCodeLocation = false;
rDebug_Filewriter*    rDebug_Filewriter::pFilewriter = nullptr;

rDebug_Filewriter::rDebug_Filewriter(const QString& fileName, rDebugLevel::rMsgType MaxLevel, qint16 MaxBackups, qint64 MaxSize)
  : mFileName(fileName)
  , mMaxSize( qMax( MaxSize, static_cast<qint64>(0x10000) ) )
  , mMaxBackups(MaxBackups)
  , mpLogfile(nullptr)
{
  rDebug_Filewriter::mMaxLevel = MaxLevel;
  if( MaxLevel <= rDebugLevel::rMsgType::Silent )
  { rDebug_Filewriter::pFilewriter = nullptr;
    return;
  }

  rDebug_Filewriter::pFilewriter = this;

  if( mFileName.isEmpty() )
  { mFileName = QDir::tempPath() + '/' + QFileInfo( QCoreApplication::applicationFilePath() ).fileName() + ".log";
  }

  if (!mFileName.isEmpty())
  {
      if( oversized(mFileName) )
      { rotate();
      }
      open( mFileName, "CTor", "========== logfile opened ==========" );
  }
}


rDebug_Filewriter::~rDebug_Filewriter()
{
  close( "DTor", "========== logfile closed ==========" );
  rDebug_Filewriter::pFilewriter = nullptr;
}


void rDebug_Filewriter::setMaxLevel(rDebugLevel::rMsgType MaxLevel)
{
  rDebug_Filewriter::mMaxLevel = MaxLevel;
}


void rDebug_Filewriter::setMaxSize(qint64 MaxSize)
{
  mMaxSize = MaxSize;
}


void rDebug_Filewriter::setMaxBackups( qint16 MaxBackups )
{
  mMaxBackups = MaxBackups;
}


void rDebug_Filewriter::enableCodeLocations( bool enable )
{
    rDebug_Filewriter::mDumpCodeLocation = enable;
}



void rDebug_Filewriter::write_file(const FileLineFunc_t& CodeLocation, const QDateTime& Time, rDebugLevel::rMsgType Level, uint64_t LogId, const QString& line)
{
  if( rDebug_Filewriter::mMaxLevel < Level )
    return;
  if( SkipOutputByPreprocessor( Level ) )
    return;

  rotate_ondemand();

  write_file_raw( CodeLocation, Time, Level, LogId, line );
}


/* ToDo:
 * support QT_MESSAGE_PATTERN environment variable.
 * For example: QT_MESSAGE_PATTERN="[%{type}] %{appname} (%{file}:%{line}) - %{message}"
 */
void rDebug_Filewriter::write_file_raw(const FileLineFunc_t& CodeLocation, const QDateTime& Time, rDebugLevel::rMsgType Level, uint64_t LogId, const QString& line)
{
  if( SkipOutputByPreprocessor( Level ) )
    return;

  QTextStream WholeMsg( mpLogfile );
  WholeMsg << QString("%1 [%2] %3, %4") \
                  .arg( rDebugBase::getDateTimeStr( Time ) ) \
                  .arg( rDebugBase::getLevelName( Level ) ) \
                  .arg( rDebugBase::getLogIdStr( LogId, 0 ) ) \
                  .arg( line ) \
                  ;
  if( rDebug_Filewriter::mDumpCodeLocation )
  {
    WholeMsg << QString(" {from %1 in %2:%3}") \
                  .arg( CodeLocation.mFunc ? CodeLocation.mFunc : "func" ) \
                  .arg( CodeLocation.mFile ? CodeLocation.mFile : "file" ) \
                  .arg( CodeLocation.mLine ) \
                  ;
  }
  WholeMsg << QString("\n");
  WholeMsg.flush();
}


void rDebug_Filewriter::write_wrap(const char* Location, const char* Reason)
{
  FileLineFunc_t here(__FILE__, __LINE__, Location);
  QDateTime Now( QDate::currentDate(), QTime::currentTime(), Qt::LocalTime );
  write_file_raw( here, Now, rDebugLevel::rMsgType::Notice, 0, Reason );
}


void rDebug_Filewriter::open(const QString& fileName, const char* Location, const char* Reason)
{
  mpLogfile = new QFile(fileName);
  mpLogfile->open(QIODevice::Append | QIODevice::Text);
  write_wrap( Location, Reason );
}


void rDebug_Filewriter::close(const char* Location, const char* Reason)
{
  write_wrap( Location, Reason );
  mpLogfile->close();
  mpLogfile = nullptr;
}


void rDebug_Filewriter::rotate(void)
{
  const int32_t MinBackupIndex = 1;
  QFileInfo fi( mFileName );
  int32_t Index = MinBackupIndex;
  for( ; Index<mMaxBackups ; ++Index )
  {
    QFileInfo probe( fi.path() + '/' + fi.completeBaseName() + QString(".%1.").arg(Index) + fi.suffix() );
    if( ! probe.exists() )
      break;
  }
  // Index is last used backup + 1 now
  for( ; Index>(MinBackupIndex-1) ; --Index )
  {
    QString free_file( fi.path() + '/' + fi.completeBaseName() + QString(".%1.").arg(Index  ) + fi.suffix() );
    QString used_file( fi.path() + '/' + fi.completeBaseName() + QString(".%1.").arg(Index-1) + fi.suffix() );
    if( Index==MinBackupIndex )
      used_file = mFileName;

    QFile( used_file ).rename( free_file );
  }
}


void rDebug_Filewriter::rotate_ondemand(void)
{
  if( oversized( *mpLogfile ) )
  {
    bool isopen = ( mpLogfile && mpLogfile->isOpen() ) ? true : false;
    if( isopen )
    {
      const char *Who = "Rotator";
      const char *Reason = "~~~~~~~~~~ logfile rotated ~~~~~~~~~~";
      close( Who, Reason );

      rotate();

      open( mFileName, Who, Reason );
    }
    else
    {
      rotate();
    }
  }
}


bool rDebug_Filewriter::oversized( const QString& fileName )
{
  QFileInfo fi( fileName );
  if( fi.exists() && fi.size() > (mMaxSize - 128) ) // 128 bytes reserve to enshure the "closed/rolled" entry fits also
      return true;
  return false;
}


bool rDebug_Filewriter::oversized( QFile& openedFile )
{
  QFileInfo fi( openedFile );
  if( fi.isReadable() && fi.size() > (mMaxSize - 128) ) // 128 bytes reserve to enshure the "closed/rolled" entry fits also
      return true;
  return false;
}

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
rDebugLevel::rMsgType rDebugBase::mMaxLevel = SYSLOG_LEVEL_MAX;


rDebugBase::rDebugBase(const char *file, int line, const char* func, rDebugLevel::rMsgType Level, uint64_t LogId)
  : mBase(10)
  , mFileLineFunc(file,line,func)
  , mLevel(Level)
  , mFacility(SYSLOG_FACILITY)
  , mTime( QDate::currentDate(), QTime::currentTime(), Qt::LocalTime )
  , mLogId(LogId)
  , mMsgBuffer("")
  , mMsgStream(&mMsgBuffer)
  , mWithLogId(SYSLOG_WITH_NUMERIC_8DIGITS_ID)
{}


rDebugBase::~rDebugBase()
{
  if( SkipOutputByPreprocessor( mLevel ) )
    return;

  output( mLevel );
}


void rDebugBase::setMaxLevel(rDebugLevel::rMsgType MaxLevel)
{
  rDebugBase::mMaxLevel = MaxLevel;
}


void rDebugBase::output( rDebugLevel::rMsgType currLevel )
{
  QSignalBackendWriter(currLevel );
  QFileBackendWriter(  currLevel );
  QDebugBackendWriter( currLevel ); // always need to be the last, because this one has the right of calling std::abort(), so the others need to be finished before
}


/* ToDo:
 * support QT_MESSAGE_PATTERN environment variable.
 * For example: QT_MESSAGE_PATTERN="[%{type}] %{appname} (%{file}:%{line}) - %{message}"
 */
void rDebugBase::QDebugBackendWriter( rDebugLevel::rMsgType currLevel )
{
  if( rDebug_GlobalLevel::get() < currLevel )
    return;

  if( rDebugBase::mMaxLevel < currLevel )
    return;

  if( SkipOutputByPreprocessor( currLevel ) )
    return;

  QString     mStringDevice("");
  QTextStream WholeMsg( &mStringDevice );
  if( mWithLogId )
      WholeMsg << QString("%1 [%2] %3, %4") \
                  .arg( getDateTimeStr( mTime ) ) \
                  .arg( getLevelName( mLevel ) ) \
                  .arg( getLogIdStr( mLogId ) ) \
                  .arg( mMsgBuffer ) ;
  else
    WholeMsg << QString("%1 [%2] %4") \
                  .arg( getDateTimeStr( mTime ) ) \
                  .arg( getLevelName( mLevel ) ) \
                  .arg( mMsgBuffer ) ;

  WholeMsg.flush();

  to_xDebug( mLevel, mStringDevice );
}


void rDebugBase::QSignalBackendWriter( rDebugLevel::rMsgType currLevel )
{
  if( rDebug_GlobalLevel::get() < currLevel )
    return;
  if( SkipOutputByPreprocessor( currLevel ) )
    return;

  if( rDebug_Signaller::pSignaller )
  {   rDebug_Signaller::pSignaller->signal_line( mFileLineFunc, mTime, mLevel, mLogId, mMsgBuffer );
  }
}


void rDebugBase::QFileBackendWriter( rDebugLevel::rMsgType currLevel )
{
  if( rDebug_GlobalLevel::get() < currLevel )
    return;
  if( SkipOutputByPreprocessor( currLevel ) )
    return;

  if( rDebug_Filewriter::pFilewriter )
  {   rDebug_Filewriter::pFilewriter->write_file( mFileLineFunc, mTime, mLevel, mLogId, mMsgBuffer );
  }
}


void rDebugBase::writer( rDebugLevel::rMsgType Level, uint64_t LogId, bool withLogId, const char* msg, va_list valist )
{
  mWithLogId = withLogId;
  mLogId     = (LogId) ? LogId : static_cast<uint64_t>(QCoreApplication::applicationPid());
  mLevel     = Level;

  if( rDebug_GlobalLevel::get() < Level )
    return;

  if(!msg)
    return;

  size_t allocated = 1024;
  char* str = new char[allocated];
  *str=0;
  int attempted;
  attempted = vsnprintf( str, allocated, msg, valist );
  if( attempted >= static_cast<int>(allocated) )
  {
    delete [] str;
    allocated = static_cast<size_t>(attempted + 1);
    str = new char[ allocated ];
    vsnprintf( str, allocated, msg, valist );
  }

  mMsgBuffer.append(str);
  delete [] str;
}




QString rDebugBase::getLevelName( rDebugLevel::rMsgType Level )
{
    QString LevelName;

    switch( Level )
    {
        case rDebugLevel::rMsgType::Debug        : LevelName = QObject::tr("Debg","[Debg], Debug-level messages Messages that contain information normally of use only when debugging a program."); break;
        case rDebugLevel::rMsgType::Informational: LevelName = QObject::tr("Info","[Info], message type of unspecified messages"); break;
        case rDebugLevel::rMsgType::Notice       : LevelName = QObject::tr("Note","[Note], Normal but significant conditions. Conditions that are not error conditions, but that may require special handling."); break;
        case rDebugLevel::rMsgType::Warning      : LevelName = QObject::tr("Warn","[Warn], Warning conditions"); break;
        case rDebugLevel::rMsgType::Error        : LevelName = QObject::tr("Err!","[Err!], Error conditions"); break;
        case rDebugLevel::rMsgType::Critical     : LevelName = QObject::tr("Crit","[Crit], Critical conditions, like hard device errors."); break;
        case rDebugLevel::rMsgType::Alert        : LevelName = QObject::tr("Alrt","[Alrt], Action must be taken immediately, A condition that should be corrected immediately, such as a corrupted system database."); break;
        case rDebugLevel::rMsgType::Emergency    : // fall through
        default                                  : LevelName = QObject::tr("Emrg","[Emrg], System is unusable. A panic condition."); break;
    }
    return LevelName;
}



QString rDebugBase::getDateTimeStr(const QDateTime& Time)
{
  QString TimestampAsString( QString("%1").arg( Time.toString(QObject::tr("yyyy-MM-dd HH:mm:ss,zzz","local date time format for logging"))) );
  return TimestampAsString;
}

QString rDebugBase::getLogIdStr(uint64_t LogId, int FormatLen)
{
  if( FormatLen == 8 ) // typical 8-digit-fixed size logid
    return QString("%1").arg( LogId, 8, 10, QLatin1Char('0') );
  else if( FormatLen == 16 )
    return QString("0x%1:%2").arg( LogId>>32, 8, 16, QLatin1Char('0') ).arg( LogId&0xFFFFFFFF, 8, 16, QLatin1Char('0') );
  else
    return QString("%1").arg( LogId );
}


rDebugBase& rDebugBase::integerBase( int base )
{
  this->mBase = base;
  return *this;
}


rDebugBase& rDebugBase::operator<<( QChar ch )
{
  mMsgStream << ch;
  return *this;
}


rDebugBase& rDebugBase::operator<<( bool flg )
{
  mMsgStream << ((flg) ? "true" : "false");
  return *this;
}


rDebugBase& rDebugBase::operator<<( char ch )
{
  mMsgStream << ch;
  return *this;
}


rDebugBase& rDebugBase::operator<<( signed short num )
{
  mMsgStream.setIntegerBase(mBase);
  mMsgStream << num;
  return *this;
}


rDebugBase& rDebugBase::operator<<( unsigned short num )
{
  mMsgStream.setIntegerBase(mBase);
  mMsgStream << num;
  return *this;
}


rDebugBase& rDebugBase::operator<<( signed int num )
{
  mMsgStream.setIntegerBase(mBase);
  mMsgStream << num;
  return *this;
}


rDebugBase& rDebugBase::operator<<( unsigned int num )
{
  mMsgStream.setIntegerBase(mBase);
  mMsgStream << num;
  return *this;
}


rDebugBase& rDebugBase::operator<<( signed long lnum )
{
  mMsgStream.setIntegerBase(mBase);
  mMsgStream << lnum;
  return *this;
}


rDebugBase& rDebugBase::operator<<( unsigned long lnum )
{
  mMsgStream.setIntegerBase(mBase);
  mMsgStream << lnum;
  return *this;
}


rDebugBase& rDebugBase::operator<<( qint64 i64 )
{
  mMsgStream.setIntegerBase(mBase);
  mMsgStream << i64;
  return *this;
}


rDebugBase& rDebugBase::operator<<( quint64 i64 )
{
  mMsgStream.setIntegerBase(mBase);
  mMsgStream << i64;
  return *this;
}


rDebugBase& rDebugBase::operator<<( float flt )
{
  mMsgStream << flt;
  return *this;
}


rDebugBase& rDebugBase::operator<<( double dbl )
{
  mMsgStream << dbl;
  return *this;
}


rDebugBase& rDebugBase::operator<<( const char * ptr )
{
  mMsgStream << ((ptr)?ptr:"(nullptr)");
  return *this;
}


rDebugBase& rDebugBase::operator<<( const QString & str )
{
  mMsgStream << str;
  return *this;
}


rDebugBase& rDebugBase::operator<<( const QStringRef & str )
{
  #if defined(QT_VERSION) && (QT_VERSION>=0x050000)
  mMsgStream << str;
  #elif defined(QT_VERSION) && (QT_VERSION>=0x040000)
  mMsgStream << str.toString();
  #endif
  return *this;
}


rDebugBase& rDebugBase::operator<<( const QLatin1String & str )
{
  mMsgStream << str;
  return *this;
}


rDebugBase& rDebugBase::operator<<( const QByteArray & ba )
{
  mMsgStream << ba;
  return *this;
}


rDebugBase& rDebugBase::operator<<( const void * vptr )
{
  mMsgStream << "0x" << hex << vptr;
  return *this;
}

rDebugBase& rDebugBase::operator<<(const QTextStream& qts)
{
  mMsgStream << qts.string();
  return *this;
}

rDebugBase&rDebugBase::operator<<(const QPoint& d)
{
  mMsgStream << "@(" << d.x() << "," << d.y() << ")";
  return *this;
}


rDebugBase&rDebugBase::operator<<(const QSize& d)
{
  mMsgStream << "@(" << d.width() << "x" << d.height() << ")";
  return *this;
}


rDebugBase&rDebugBase::operator<<(const QRect& d)
{
  mMsgStream << "QRect(" << d.x() << "," << d.y() << " " << d.width() << "x" << d.height() << ")";
  return *this;
}


rDebugBase& rDebugBase::debug(uint64_t LogId, const char* msg, ... )
{
  va_list args;
  va_start(args, msg);
  writer( rDebugLevel::rMsgType::Debug, LogId, true, msg, args );
  va_end(args);
  return (*this);
}

rDebugBase& rDebugBase::info(uint64_t LogId, const char* msg, ... )
{
  va_list args;
  va_start(args, msg);
  writer( rDebugLevel::rMsgType::Informational, LogId, true, msg, args );
  va_end(args);
  return (*this);
}

rDebugBase& rDebugBase::note(uint64_t LogId, const char* msg, ... )
{
  va_list args;
  va_start(args, msg);
  writer( rDebugLevel::rMsgType::Notice, LogId, true, msg, args );
  va_end(args);
  return (*this);
}

rDebugBase& rDebugBase::warning(uint64_t LogId, const char* msg, ... )
{
  va_list args;
  va_start(args, msg);
  writer( rDebugLevel::rMsgType::Warning, LogId, true, msg, args );
  va_end(args);
  return (*this);
}

rDebugBase& rDebugBase::error(uint64_t LogId, const char* msg, ... )
{
  va_list args;
  va_start(args, msg);
  writer( rDebugLevel::rMsgType::Error, LogId, true, msg, args );
  va_end(args);
  return (*this);
}

rDebugBase& rDebugBase::critical(uint64_t LogId, const char* msg, ... )
{
  va_list args;
  va_start(args, msg);
  writer( rDebugLevel::rMsgType::Critical, LogId, true, msg, args );
  va_end(args);
  return (*this);
}

rDebugBase& rDebugBase::emergency(uint64_t LogId, const char* msg, ... )
{
  va_list args;
  va_start(args, msg);
  writer( rDebugLevel::rMsgType::Alert, LogId, true, msg, args );
  va_end(args);
  return (*this);
}

rDebugBase& rDebugBase::fatal(uint64_t LogId, const char* msg, ... )
{
  va_list args;
  va_start(args, msg);
  writer( rDebugLevel::rMsgType::Emergency, LogId, true, msg, args );
  va_end(args);
  return (*this);
}

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

rDebugBase& rDebugBase::debug(const char* msg, ... )
{
  va_list args;
  va_start(args, msg);
  writer( rDebugLevel::rMsgType::Debug, 0, false, msg, args );
  va_end(args);
  return (*this);
}

rDebugBase& rDebugBase::info(const char* msg, ... )
{
  va_list args;
  va_start(args, msg);
  writer( rDebugLevel::rMsgType::Informational, 0, false, msg, args );
  va_end(args);
  return (*this);
}

rDebugBase& rDebugBase::note(const char* msg, ... )
{
  va_list args;
  va_start(args, msg);
  writer( rDebugLevel::rMsgType::Notice, 0, false, msg, args );
  va_end(args);
  return (*this);
}

rDebugBase& rDebugBase::warning(const char* msg, ... )
{
  va_list args;
  va_start(args, msg);
  writer( rDebugLevel::rMsgType::Warning, 0, false, msg, args );
  va_end(args);
  return (*this);
}

rDebugBase& rDebugBase::error(const char* msg, ... )
{
  va_list args;
  va_start(args, msg);
  writer( rDebugLevel::rMsgType::Error, 0, false, msg, args );
  va_end(args);
  return (*this);
}

rDebugBase& rDebugBase::critical(const char* msg, ... )
{
  va_list args;
  va_start(args, msg);
  writer( rDebugLevel::rMsgType::Critical, 0, false, msg, args );
  va_end(args);
  return (*this);
}

rDebugBase& rDebugBase::emergency(const char* msg, ... )
{
  va_list args;
  va_start(args, msg);
  writer( rDebugLevel::rMsgType::Alert, 0, false, msg, args );
  va_end(args);
  return (*this);
}

rDebugBase& rDebugBase::fatal(const char* msg, ... )
{
  va_list args;
  va_start(args, msg);
  writer( rDebugLevel::rMsgType::Emergency, 0, false, msg, args );
  va_end(args);
  return (*this);
}


/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

rDebugBase& operator<<( rDebugBase& s, rDebugBaseManipulator f )
{ return (*f)(s); }

rDebugBase& hex( rDebugBase& s )
{
  return s.integerBase(16);
}

rDebugBase& dec( rDebugBase& s )
{
  return s.integerBase(10);
}

rDebugBase& oct( rDebugBase& s )
{
  return s.integerBase(8);
}

rDebugBase& bin( rDebugBase& s )
{
  return s.integerBase(2);
}

