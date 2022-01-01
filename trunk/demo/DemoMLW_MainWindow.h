#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLayout>
#include <QPushButton>
#include <QSlider>

#include <MessageListWidget.h>
#include <rDebugCodeloc.h>

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(QWidget *parent = 0);
  ~MainWindow();

signals:
    void sig_logline( const FileLineFunc_t& CodeLocation, const QDateTime& Time, int Level, uint64_t LogId, const QString& line );
    void sig_logline( const QDateTime& Time, int Level, uint64_t LogId, const QString& line );
    void sig_logline( const QDateTime& Time, int Level, const QString& line );
    void sig_logline( int Level, const QString& line );
    ////
    void sig_setMaxLevel(int MaxLevel);
    void sig_setInitLevel(int MaxLevel); // to set the same level to slider and widget
    void sig_setWithDate(bool);
    void sig_setWithLevel(bool);
    void sig_setWithNumID(bool);
    void sig_setNumIDFormat(int digits,int base);

public slots:
    //void on_increment_level(bool clicked);
    void on_set_level(int requestedMaxLevel);
    void on_cklicked(bool clicked);
    void on_shortID(bool);
    void on_HexID(bool);

private: // data
  QWidget           m_CentralWidget;
  QBoxLayout*       m_LayOut;
  QPushButton*      m_ButtonLine;
  //QPushButton*      m_ButtonIncLevel;
  QSlider*          m_AdjustMaxLevel;
  QPushButton*      m_ButtonWithDate;
  QPushButton*      m_ButtonWithLevel;
  QPushButton*      m_ButtonWithId;
  QPushButton*      m_ButtonWithId_Short;
  QPushButton*      m_ButtonWithId_Hex;
  uint64_t          m_Count;
  int               m_MaxLevel;
  int               m_digits, m_base;
  MessageListWidget m_Messages;
};

#endif // MAINWINDOW_H
