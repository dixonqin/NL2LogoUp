#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#define RAW_RECORD_FILENAME     "../../audio/record.raw"			// 录音文件名;
#define WAV_RECORD_FILENAME     "../../audio/record.wav"			// 录音文件转wav格式文件名;
#include <QMainWindow>
#include <QFile>
#include <QString>
#include <QtMultimedia/QAudioInput>
#include <QtMultimedia/QAudioOutput>
#include <QtMultimedia/QAudioDeviceInfo>
#include <QKeyEvent>
#include <QMessageBox>
#include "aimlparser.h"

#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <conio.h>
#include <errno.h>

#include "include/qisr.h"
#include "include/msp_cmn.h"
#include "include/msp_errors.h"

#define BUFFER_SIZE	4096
#define FRAME_LEN	640
#define HINTS_SIZE  100

#ifdef _WIN64
#pragma comment(lib,"libs/msc_x64.lib")
#else
#pragma comment(lib,"libs/msc.lib")
#endif

const qint64 TIME_TRANSFORM = 1000 * 1000;

struct WAVFILEHEADER
{
  // RIFF 头;
  char RiffName[4];
  unsigned long nRiffLength;

  // 数据类型标识符;
  char WavName[4];

  // 格式块中的块头;
  char FmtName[4];
  unsigned long nFmtLength;

  // 格式块中的块数据;
  unsigned short nAudioFormat;
  unsigned short nChannleNumber;
  unsigned long nSampleRate;
  unsigned long nBytesPerSecond;
  unsigned short nBytesPerSample;
  unsigned short nBitsPerSample;

  // 数据块中的块头;
  char    DATANAME[4];
  unsigned long   nDataLength;
};
namespace Ui {
  class MainWindow;
}


class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();
  AIMLParser *parser;
  int hi;
  int hm;
  void keyPressEvent(QKeyEvent *e);
  void insertHistory(QString str);

  //for record
  int state= 0;//0 is stop, 1 is recording
  qint64 addWavHeader(QString catheFileName, QString wavFileName);
  void handleStateChanged(QAudio::State state);
  void onStartRecord();
  void onStopRecording();
  QString result;
  //for recognize
  int upload_userwords();
  void run_iat(const char* audio_file, const char* session_begin_params);
private:
  QFile destinationFile;
  QAudioInput* m_audioInput;
  QAudioOutput* m_audioOutput;
  QFile sourceFile;
  bool m_isRecord;
  int m_RecordTimerId;
  int m_RecordTime;
  QString history[6];
  Ui::MainWindow *ui;
private slots:
  void on_pushButton_clicked();

  void on_lineEdit_returnPressed();

  void on_pushButton_2_clicked();

};


#endif // MAINWINDOW_H
