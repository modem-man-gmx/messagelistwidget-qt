#ifndef RDEBUG_H
#define RDEBUG_H
/**
 * Project "rDebug"
 *
 * rDebug.h
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
 
#include <QDate>
#include <QString>
#include <QTextStream>
#include <stdarg.h>  // va_list
#include <QTextStream>
//#include <QDataStream>
#include <QPoint>
#include <QSize>
#include <QRect>
#include <QFile>

#include "rDebugLevel.h"
#include "rDebugCodeloc.h"


#ifdef qDebug
// -- for reference: what is Qt4 doing here?
// #define qDebug    QMessageLogger(QT_MESSAGELOG_FILE, QT_MESSAGELOG_LINE, QT_MESSAGELOG_FUNC).debug
// #define qInfo     QMessageLogger(QT_MESSAGELOG_FILE, QT_MESSAGELOG_LINE, QT_MESSAGELOG_FUNC).info
// #define qWarning  QMessageLogger(QT_MESSAGELOG_FILE, QT_MESSAGELOG_LINE, QT_MESSAGELOG_FUNC).warning
// #define qCritical QMessageLogger(QT_MESSAGELOG_FILE, QT_MESSAGELOG_LINE, QT_MESSAGELOG_FUNC).critical
// #define qFatal    QMessageLogger(QT_MESSAGELOG_FILE, QT_MESSAGELOG_LINE, QT_MESSAGELOG_FUNC).fatal
# undef qDebug
# undef qInfo
# undef qWarning
# undef qCritical
# undef qFatal
// redefine qDebug so a
//    qDebug() << "hello"
// will become a
//    rDebugBase(__FILE__, __LINE__, __PRETTY_FUNCTION__).debug() << hello
// ---- qDebug emulation -------------------
# define qDebug     rDebugBase( __FILE__, __LINE__, __PRETTY_FUNCTION__ ).debug
# define qInfo      rDebugBase( __FILE__, __LINE__, __PRETTY_FUNCTION__ ).info
# define qWarning   rDebugBase( __FILE__, __LINE__, __PRETTY_FUNCTION__ ).warning
# define qCritical  rDebugBase( __FILE__, __LINE__, __PRETTY_FUNCTION__ ).critical
# define qSystem    rDebugBase( __FILE__, __LINE__, __PRETTY_FUNCTION__ ).error
# define qFatal     rDebugBase( __FILE__, __LINE__, __PRETTY_FUNCTION__ ).fatal  // break down application, calling abort()
// ---- rDebug more syslog-like logging (is also upcomming with Qt5+6) --------
# define rDebug     rDebugBase( __FILE__, __LINE__, __PRETTY_FUNCTION__ ).debug     // syslog 7
# define rInfo      rDebugBase( __FILE__, __LINE__, __PRETTY_FUNCTION__ ).info      // syslog 6
# define rNote      rDebugBase( __FILE__, __LINE__, __PRETTY_FUNCTION__ ).note      // syslog 5
# define rWarning   rDebugBase( __FILE__, __LINE__, __PRETTY_FUNCTION__ ).warning   // syslog 4
# define rError     rDebugBase( __FILE__, __LINE__, __PRETTY_FUNCTION__ ).error     // syslog 3
# define rCritical  rDebugBase( __FILE__, __LINE__, __PRETTY_FUNCTION__ ).critical  // syslog 2
# define rFatal     rDebugBase( __FILE__, __LINE__, __PRETTY_FUNCTION__ ).fatal     // syslog 1 + break down application, calling abort() to be compatible with qFatal
# define rEmergency rDebugBase( __FILE__, __LINE__, __PRETTY_FUNCTION__ ).emergency // syslog 0 + break down application, calling abort()
/* --- aliases: */
# define rAlert     rDebugBase( __FILE__, __LINE__, __PRETTY_FUNCTION__ ).emergency // syslog 0 alias ( + break down ... )
# define rSystem    rDebugBase( __FILE__, __LINE__, __PRETTY_FUNCTION__ ).error     // another syslog 3 alias
#endif


// -----------------------
// this is controlling the gloabl Message filtering by a global level,
// independend from filtering by each sink.
// usage:
//    rDebug_GlobalLevel globalLogLevel( rDebugLevel::rMsgType::All );
//    rDebug_GlobalLevel globalLogLevel( rDebugLevel::rMsgType::Debug );
//    ...
//    rDebug_GlobalLevel globalLogLevel( rDebugLevel::rMsgType::Emergency );
//    rDebug_GlobalLevel globalLogLevel( rDebugLevel::rMsgType::Silent );
//
// note:
// rDebug_Signaller and rDebug_Filewriter, as well as rDebugBase have their own, sink-specific MaxLevel
//    rDebug_Signaller::setMaxLevel(n)
//    rDebug_Filewriter::setMaxLevel(n)
//    rDebugBase::setMaxLevel(n)
// which are checked, after a message passed a rDebug_GlobalLevel::set(m) set value. So each of the 3
// sinks can reduce verbosity idividually, while all together can obey an common global level of
// verbosity.
// -----------------------
class rDebug_GlobalLevel
{
public:
  rDebug_GlobalLevel( rDebugLevel::rMsgType MaxLevel = rDebugLevel::rMsgType::Informational );
  static void set( rDebugLevel::rMsgType MaxLevel );
  static rDebugLevel::rMsgType get();

private:
  static rDebugLevel::rMsgType mMaxLevel;
};





// -----------------------
class rDebugBase;
// -----------------------
// bind Qt's Signal-Slot system as one sink to the logging
// note:
//    - there is always only ONE sink connected here. To use multiple sinks, add multiple Slots to the signal
//    - to filter by level, use setMaxLevel( rDebugLevel::rMsgType::xxxx ), don't forget the rDebug_GlobalLevel
//      to be set at least to the same value, or it will win the filtering (may also be wanted)
//    - if using multiple sinks with different Levels (f.i. file with all-logging and listview with only errors),
//      implement different filtering there
// -----------------------
class rDebug_Signaller : public QObject
{
  Q_OBJECT

  friend class rDebugBase;
public:
  rDebug_Signaller(rDebugLevel::rMsgType MaxLevel = rDebugLevel::rMsgType::Informational);
  virtual ~rDebug_Signaller();
  static void setMaxLevel( rDebugLevel::rMsgType MaxLevel );
  void signal_line( const FileLineFunc_t& CodeLocation, const QDateTime& Time, rDebugLevel::rMsgType Level, uint64_t LogId, const QString& line );

public slots:
  void sig_setMaxLevel( rDebugLevel::rMsgType MaxLevel ) {setMaxLevel(MaxLevel);}
  void sig_setMaxLevel( int MaxLevel ) {setMaxLevel(static_cast<rDebugLevel::rMsgType>(MaxLevel));}

signals:
  void sig_logline( const FileLineFunc_t& CodeLocation, const QDateTime& Time, int Level, uint64_t LogId, const QString& line );

private:
  static rDebug_Signaller*     pSignaller;
  static rDebugLevel::rMsgType mMaxLevel;
};



// -----------------------
class rDebug_Filewriter
{
  friend class rDebugBase;
public:
  rDebug_Filewriter(const QString& fileName, rDebugLevel::rMsgType MaxLevel = rDebugLevel::rMsgType::Informational, qint16 MaxBackups=2 , qint64 MaxSize=0x100000);
  virtual ~rDebug_Filewriter();
  static void setMaxLevel( rDebugLevel::rMsgType MaxLevel );
  void setMaxSize( qint64 MaxSize=0x100000 );
  void setMaxBackups( qint16 MaxBackups );
  static void enableCodeLocations(bool enable);
  void write_file( const FileLineFunc_t& CodeLocation, const QDateTime& Time, rDebugLevel::rMsgType Level, uint64_t LogId, const QString& line );
  void write_file_raw(const FileLineFunc_t& CodeLocation, const QDateTime& Time, rDebugLevel::rMsgType Level, uint64_t LogId, const QString& line);

protected:
  void write_wrap( const char* Location, const char* Reason );

private:
  void open( const QString& fileName, const char* Location, const char* Reason );
  void close(const char* Location, const char* Reason);
  bool oversized( const QString& fileName );
  bool oversized( QFile& openedFile );
  void rotate(void);
  void rotate_ondemand(void);

private:
  static rDebug_Filewriter*    pFilewriter;
  static rDebugLevel::rMsgType mMaxLevel;
  static bool                  mDumpCodeLocation;
  QString                      mFileName;
  qint64                       mMaxSize;
  qint16                       mMaxBackups;
  QFile*                       mpLogfile;
};




// -----------------------
// -----------------------
class rDebugBase
{
public:
  explicit rDebugBase( const char *file, int line, const char* func, rDebugLevel::rMsgType Level=rDebugLevel::rMsgType::Warning, uint64_t LogId=0 );
  virtual ~rDebugBase();

public:
  rDebugBase& integerBase(int base);
  rDebugBase& operator<<( QChar ch );
  rDebugBase& operator<<( bool flg );
  rDebugBase& operator<<( char ch );
  rDebugBase& operator<<( signed short num );
  rDebugBase& operator<<( unsigned short num );
  rDebugBase& operator<<( signed int num );
  rDebugBase& operator<<( unsigned int num );
  rDebugBase& operator<<( signed long lnum );
  rDebugBase& operator<<( unsigned long lnum );
  rDebugBase& operator<<( qint64 i64 );
  rDebugBase& operator<<( quint64 i64 );
  rDebugBase& operator<<( float flt );
  rDebugBase& operator<<( double dbl );
  rDebugBase& operator<<( const char * ptr );
  rDebugBase& operator<<( const QString & str );
  rDebugBase& operator<<( const QStringRef & str );
  rDebugBase& operator<<( const QLatin1String & str );
  rDebugBase& operator<<( const QByteArray & ba );
  rDebugBase& operator<<( const void * vptr );
  rDebugBase& operator<<( const QTextStream& qts );
  rDebugBase& operator<<( const QPoint& d );
  rDebugBase& operator<<( const QSize& d );
  rDebugBase& operator<<( const QRect& d );

  // Emergency / Alert / Critical / Error / Warning / Notice / Informational / Debug
  rDebugBase& debug(    uint64_t LogId=0, const char *msg = nullptr, ... );
  rDebugBase& info(     uint64_t LogId=0, const char *msg = nullptr, ... );
  rDebugBase& note(     uint64_t LogId=0, const char *msg = nullptr, ... );
  rDebugBase& warning(  uint64_t LogId=0, const char *msg = nullptr, ... );
  rDebugBase& error(    uint64_t LogId=0, const char *msg = nullptr, ... );
  rDebugBase& critical( uint64_t LogId=0, const char *msg = nullptr, ... );
  rDebugBase& emergency(uint64_t LogId=0, const char *msg = nullptr, ... );
  rDebugBase& fatal(    uint64_t LogId=0, const char *msg = nullptr, ... );

  rDebugBase& debug(    const char *msg, ... );
  rDebugBase& info(     const char *msg, ... );
  rDebugBase& note(     const char *msg, ... );
  rDebugBase& warning(  const char *msg, ... );
  rDebugBase& error(    const char *msg, ... );
  rDebugBase& critical( const char *msg, ... );
  rDebugBase& emergency(const char *msg, ... );
  rDebugBase& fatal(    const char *msg, ... );

public:
  static void setMaxLevel(rDebugLevel::rMsgType MaxLevel);
  static QString getLevelName( rDebugLevel::rMsgType Level );
  static QString getDateTimeStr(const QDateTime& Time);
  static QString getLogIdStr(uint64_t LogId, int FormatLen=8);

protected:
  void output( rDebugLevel::rMsgType currLevel );
  void writer(rDebugLevel::rMsgType Level, uint64_t LogId, bool withLogId, const char* msg, va_list valist );

private:
  void QDebugBackendWriter(  rDebugLevel::rMsgType currLevel );
  void QSignalBackendWriter( rDebugLevel::rMsgType currLevel );
  void QFileBackendWriter(   rDebugLevel::rMsgType currLevel );

private:
  static rDebugLevel::rMsgType mMaxLevel;
  int         mBase;
  FileLineFunc_t mFileLineFunc;
  rDebugLevel::rMsgType    mLevel;
  uint        mFacility;
  QDateTime   mTime;
  uint64_t    mLogId;
  QString     mMsgBuffer;
  QTextStream mMsgStream;
  bool        mWithLogId;
};

typedef rDebugBase& (*rDebugBaseManipulator)( rDebugBase& );// manipulator function

rDebugBase& operator<<( rDebugBase& s, rDebugBaseManipulator f );

// like QTextStream
rDebugBase &bin(rDebugBase &s);
rDebugBase &oct(rDebugBase &s);
rDebugBase &dec(rDebugBase &s);
rDebugBase &hex(rDebugBase &s);

#endif // RDEBUG_H
