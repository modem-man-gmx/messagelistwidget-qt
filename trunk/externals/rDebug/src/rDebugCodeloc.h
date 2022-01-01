#ifndef RDEBUGCODELOC_H
#define RDEBUGCODELOC_H

/**
 * Project "rDebug"
 *
 * rDebugCodeloc.cpp
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


class FileLineFunc_t
{
public:
  FileLineFunc_t( const char *file, int line, const char* func )
    : mFile(file)
    , mLine(line)
    , mFunc(func)
    {}
  const char* mFile;
  const int   mLine;
  const char* mFunc;
};

#endif // RDEBUGCODELOC_H
