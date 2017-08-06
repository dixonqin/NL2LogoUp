#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>


MainWindow:: MainWindow(QWidget *parent):
  QMainWindow(parent),
  ui(new Ui::MainWindow),
  m_isRecord(false),
  m_RecordTimerId(0),
  m_RecordTime(0),
  hi(0),
  hm(0)
{
  parser = new AIMLParser;
//  if(!parser->loadAIMLSet("English"))//just for test
//  {
//    qDebug() << "loadAIML error!";
//  }
  if(!parser->loadAIMLSet("LogoUp"))
  {
    //qDebug() << "loadAIML error!";
  }
  state = 0;
  ui->setupUi(this);

}

MainWindow:: ~MainWindow()
{
  delete parser;
  delete ui;
}

void MainWindow:: keyPressEvent(QKeyEvent *e)
{
  switch(e->key())
  {
  case Qt::Key_Up:
      if (hm == 0)
        break;
      ui->lineEdit->setText(history[hi++]);
      if (hi == hm)
        hi = 0;
      break;
  case Qt::Key_Down:
      if (hm == 0)
        break;
      ui->lineEdit->setText(history[hi--]);
      if (hi == -1)
        hi = hm - 1;
      break;
  case Qt::Key_F1:
      on_pushButton_2_clicked();
      break;
  case Qt::Key_F12:
      ui->textEdit->setText("");
      break;
  case Qt::Key_F10:
      QMessageBox::information(this,QString::fromUtf8("帮助"), QString::fromUtf8(" 按F1开始或结束录音\n 按F12清空结果\n 按方向键上下获取输入历史"));
      break;
  }
}

void MainWindow:: insertHistory(QString str)
{
  for (int i = hm; i > 0; i--)
  {
    history[i] = history[i-1];
  }
  history[0] = str;
  if (hm < 5)
    hm++;
}

void MainWindow:: on_pushButton_clicked()
{
  QString input = ui->lineEdit->text();
  if (input.trimmed().isEmpty())
    return;
  QString res = parser->getResFromCN(input);
  //ui->textEdit->append("input: "+input);
  //ui->textEdit->append("output: "+res);
  ui->textEdit->append(res);
  ui->lineEdit->clear();
  insertHistory(input);
}

void MainWindow:: on_lineEdit_returnPressed()
{
  QString input = ui->lineEdit->text();
  if (input.trimmed().isEmpty())
    return;
  QString res = parser->getResFromCN(input);
  //ui->textEdit->append("input: "+input);
  //ui->textEdit->append("output: "+res);
  ui->textEdit->append(res);
  ui->lineEdit->clear();
  insertHistory(input);
}

void MainWindow::on_pushButton_2_clicked()
{
  if(state)//state == 1
  {
    ui->pushButton_2->setText("voice");
    state = 0;
    onStopRecording();
    const char* session_begin_params = "sub = iat, domain = iat, language = zh_cn, accent = mandarin, sample_rate = 8000, result_type = plain, result_encoding = gb2312";
    int ret = MSP_SUCCESS;
    const char* login_params = "appid = 595344ed, work_dir = ."; // 登录参数，appid与msc库绑定,请勿随意改动
    ret = MSPLogin(NULL, NULL, login_params); //第一个参数是用户名，第二个参数是密码，均传NULL即可，第三个参数是登录参数
    if (ret != MSP_SUCCESS)
      qDebug() << "login error";
    ret = upload_userwords();
    if (ret != MSP_SUCCESS)
      qDebug() << "upload error";
    run_iat("../../audio/record.wav", session_begin_params);
    MSPLogout(); //退出登录
    ui->lineEdit->setText(result);
  }
  else//state == 0
  {
    ui->pushButton_2->setText("stop");
    state = 1;
    onStartRecord();
  }
}

void MainWindow:: onStartRecord()
{
  // 如果当前没有开始录音则允许录音;
  if (!m_isRecord)
  {
    // 判断本地设备是否支持该格式
    QAudioDeviceInfo audioDeviceInfo = QAudioDeviceInfo::defaultInputDevice();
    // 判断本地是否有录音设备;
    if (!audioDeviceInfo.isNull())
    {
      m_isRecord = true;
      destinationFile.setFileName(RAW_RECORD_FILENAME);
      destinationFile.open(QIODevice::WriteOnly | QIODevice::Truncate);

      // 设置音频文件格式;
      QAudioFormat format;
      // 设置采样频率;
      format.setSampleRate(8000);
      // 设置通道数;
      format.setChannelCount(1);
      // 设置每次采样得到的样本数据位值;
      format.setSampleSize(16);
      // 设置编码方法;
      format.setCodec("audio/pcm");
      // 设置采样字节存储顺序;
      format.setByteOrder(QAudioFormat::LittleEndian);
      // 设置采样类型;
      format.setSampleType(QAudioFormat::UnSignedInt);

      // 判断当前设备设置是否支持该音频格式;
      if (!audioDeviceInfo.isFormatSupported(format))
      {
        qDebug() << "Default format not supported, trying to use the nearest.";
        format = audioDeviceInfo.nearestFormat(format);
      }
      m_audioInput = new QAudioInput(format, this);
      m_audioInput->start(&destinationFile);

      // 开启时钟，用于更新当前录音时间;
      if (m_RecordTimerId == 0)
      {
        m_RecordTimerId = startTimer(100);
      }
    }
    else
    {
      // 没有录音设备;
      qDebug()<< "Current No Record Device";
    }
  }
}

qint64 MainWindow:: addWavHeader(QString catheFileName , QString wavFileName)
{
  // 开始准备WAV的文件头
  WAVFILEHEADER WavFileHeader;
  qstrcpy(WavFileHeader.RiffName, "RIFF");
  qstrcpy(WavFileHeader.WavName, "WAVE");
  qstrcpy(WavFileHeader.FmtName, "fmt ");

  WavFileHeader.nFmtLength = 16;  //  表示 FMT 的长度
  WavFileHeader.nAudioFormat = 1; //这个表示 PCM 编码;

  qstrcpy(WavFileHeader.DATANAME, "data");

  WavFileHeader.nBitsPerSample = 16;
  WavFileHeader.nBytesPerSample = 2;
  WavFileHeader.nSampleRate = 8000;
  WavFileHeader.nBytesPerSecond = 16000;
  WavFileHeader.nChannleNumber = 1;

  QFile cacheFile(catheFileName);
  QFile wavFile(wavFileName);

  if (!cacheFile.open(QIODevice::ReadWrite))
  {
    return -1;
  }
  if (!wavFile.open(QIODevice::WriteOnly))
  {
    return -2;
  }

  int nSize = sizeof(WavFileHeader);
  qint64 nFileLen = cacheFile.bytesAvailable();

  WavFileHeader.nRiffLength = nFileLen - 8 + nSize;
  WavFileHeader.nDataLength = nFileLen;

  // 先将wav文件头信息写入，再将音频数据写入;
  wavFile.write((char *)&WavFileHeader, nSize);
  wavFile.write(cacheFile.readAll());

  cacheFile.close();
  wavFile.close();

  return nFileLen;
}

void MainWindow:: onStopRecording()
{
  // 当前正在录音时，停止录音;
  if (m_isRecord)
  {
    // 关闭时钟，并初始化数据;
    killTimer(m_RecordTimerId);
    m_RecordTime = 0;
    m_RecordTimerId = 0;
    m_isRecord = false;
    if (m_audioInput != NULL)
    {
      m_audioInput->stop();
      destinationFile.close();
      delete m_audioInput;
      m_audioInput = NULL;
    }

    // 将生成的.raw文件转成.wav格式文件;
    if (addWavHeader(RAW_RECORD_FILENAME, WAV_RECORD_FILENAME) > 0)
      qDebug() << "RecordFile Save Success";
    }
}

/* 上传用户词表 */
int MainWindow:: upload_userwords()
{
  char* userwords = NULL;
  unsigned int len = 0;
  unsigned int read_len = 0;
  FILE* fp = NULL;
  int ret	 = -1;

  fp = fopen("../../userwords.txt", "rb");
  if (NULL == fp)
  {
    qDebug() << "open [userwords.txt] failed! ";
    if (NULL != fp)
    {
      fclose(fp);
      fp = NULL;
    }
    if (NULL != userwords)
    {
      free(userwords);
      userwords = NULL;
    }
    return 1;
  }

  fseek(fp, 0, SEEK_END);
  len = ftell(fp); //获取音频文件大小
  fseek(fp, 0, SEEK_SET);

  userwords = (char*)malloc(len + 1);
  if (NULL == userwords)
  {
    qDebug() << "out of memory! ";
    if (NULL != fp)
    {
      fclose(fp);
      fp = NULL;
    }
    if (NULL != userwords)
    {
      free(userwords);
      userwords = NULL;
    }
    return 1;
  }

  read_len = fread((void*)userwords, 1, len, fp); //读取用户词表内容
  if (read_len != len)
  {
    qDebug() << "read [userwords.txt] failed!";
    if (NULL != fp)
    {
      fclose(fp);
      fp = NULL;
    }
    if (NULL != userwords)
    {
      free(userwords);
      userwords = NULL;
    }
    return 1;
  }
  userwords[len] = '\0';

  MSPUploadData("userwords", userwords, len, "sub = uup, dtt = userword", &ret); //上传用户词表
  if (MSP_SUCCESS != ret)
  {
    qDebug() << "MSPUploadData failed ! errorCode:" << ret;
    if (NULL != fp)
    {
      fclose(fp);
      fp = NULL;
    }
    if (NULL != userwords)
    {
      free(userwords);
      userwords = NULL;
    }
    return 1;
  }

  if (NULL != fp)
  {
    fclose(fp);
    fp = NULL;
  }
  if (NULL != userwords)
  {
    free(userwords);
    userwords = NULL;
  }
  return ret;
}

void MainWindow:: run_iat(const char* audio_file, const char* session_begin_params)
{
  const char* session_id = NULL;
  char rec_result[BUFFER_SIZE] = {NULL};
  char hints[HINTS_SIZE] = {NULL}; //hints为结束本次会话的原因描述，由用户自定义
  unsigned int total_len = 0;
  int aud_stat = MSP_AUDIO_SAMPLE_CONTINUE; //音频状态
  int ep_stat = MSP_EP_LOOKING_FOR_SPEECH; //端点检测
  int rec_stat = MSP_REC_STATUS_SUCCESS; //识别状态
  int errcode = MSP_SUCCESS ;

  FILE* f_pcm =NULL;
  char* p_pcm = NULL;
  long pcm_count = 0;
  long pcm_size = 0;
  long read_size = 0;


  if (NULL == audio_file)
  {

    if (NULL != f_pcm)
    {
      fclose(f_pcm);
      f_pcm = NULL;
    }
    if (NULL != p_pcm)
    {
      free(p_pcm);
      p_pcm = NULL;
    }
    return;
  }

  f_pcm = fopen(audio_file, "rb");
  if (NULL == f_pcm)
  {
    qDebug() << "open "<< audio_file << " failed! ";
    if (NULL != f_pcm)
    {
      fclose(f_pcm);
      f_pcm = NULL;
    }
    if (NULL != p_pcm)
    {
      free(p_pcm);
      p_pcm = NULL;
    }
    return;
  }

  fseek(f_pcm, 0, SEEK_END);
  pcm_size = ftell(f_pcm); //获取音频文件大小
  fseek(f_pcm, 0, SEEK_SET);

  p_pcm = (char *)malloc(pcm_size);
  if (NULL == p_pcm)
  {
    qDebug() << "out of memory! ";
    if (NULL != f_pcm)
    {
      fclose(f_pcm);
      f_pcm = NULL;
    }
    if (NULL != p_pcm)
    {
      free(p_pcm);
      p_pcm = NULL;
    }
    return;
  }

  read_size = fread((void *)p_pcm, 1, pcm_size, f_pcm); //读取音频文件内容
  if (read_size != pcm_size)
  {
    qDebug() << "read "<< audio_file <<" error!";
    if (NULL != f_pcm)
    {
      fclose(f_pcm);
      f_pcm = NULL;
    }
    if (NULL != p_pcm)
    {
      free(p_pcm);
      p_pcm = NULL;
    }
    return;
  }
  session_id = QISRSessionBegin(NULL, session_begin_params, &errcode); //听写不需要语法，第一个参数为NULL
  if (MSP_SUCCESS != errcode)
  {
    qDebug() << "QISRSessionBegin failed! error code:" << errcode;
    if (NULL != f_pcm)
    {
      fclose(f_pcm);
      f_pcm = NULL;
    }
    if (NULL != p_pcm)
    {
      free(p_pcm);
      p_pcm = NULL;
    }

    QISRSessionEnd(session_id, hints);
    return;
  }

  while (1)
  {
    unsigned int len = 10 * FRAME_LEN; // 每次写入200ms音频(16k，16bit)：1帧音频20ms，10帧=200ms。16k采样率的16位音频，一帧的大小为640Byte
    int ret = 0;

    if (pcm_size < 2 * len)
      len = pcm_size;
    if (len <= 0)
      break;

    aud_stat = MSP_AUDIO_SAMPLE_CONTINUE;
    if (0 == pcm_count)
      aud_stat = MSP_AUDIO_SAMPLE_FIRST;

    ret = QISRAudioWrite(session_id, (const void *)&p_pcm[pcm_count], len, aud_stat, &ep_stat, &rec_stat);
    if (MSP_SUCCESS != ret)
    {
      qDebug() << "QISRAudioWrite failed! error code:" << ret;
      if (NULL != f_pcm)
      {
        fclose(f_pcm);
        f_pcm = NULL;
      }
      if (NULL != p_pcm)
      {
        free(p_pcm);
        p_pcm = NULL;
      }

      QISRSessionEnd(session_id, hints);
      return;
    }

    pcm_count += (long)len;
    pcm_size  -= (long)len;

    if (MSP_REC_STATUS_SUCCESS == rec_stat) //已经有部分听写结果
    {
      const char *rslt = QISRGetResult(session_id, &rec_stat, 0, &errcode);
      if (MSP_SUCCESS != errcode)
      {
        qDebug() << "QISRGetResult failed! error code:" << errcode;
        if (NULL != f_pcm)
        {
          fclose(f_pcm);
          f_pcm = NULL;
        }
        if (NULL != p_pcm)
        {
          free(p_pcm);
          p_pcm = NULL;
        }

        QISRSessionEnd(session_id, hints);
        return;
      }
      if (NULL != rslt)
      {
        unsigned int rslt_len = strlen(rslt);
        total_len += rslt_len;
        if (total_len >= BUFFER_SIZE)
        {
          qDebug() << "no enough buffer for rec_result !";
          if (NULL != f_pcm)
          {
            fclose(f_pcm);
            f_pcm = NULL;
          }
          if (NULL != p_pcm)
          {
            free(p_pcm);
            p_pcm = NULL;
          }

          QISRSessionEnd(session_id, hints);
          return;
        }
        strncat(rec_result, rslt, rslt_len);
      }
    }

    if (MSP_EP_AFTER_SPEECH == ep_stat)
      break;
    Sleep(200); //模拟人说话时间间隙。200ms对应10帧的音频
  }
  errcode = QISRAudioWrite(session_id, NULL, 0, MSP_AUDIO_SAMPLE_LAST, &ep_stat, &rec_stat);
  if (MSP_SUCCESS != errcode)
  {
    qDebug() << "QISRAudioWrite failed! error code:"<<errcode;
    if (NULL != f_pcm)
    {
      fclose(f_pcm);
      f_pcm = NULL;
    }
    if (NULL != p_pcm)
    {
      free(p_pcm);
      p_pcm = NULL;
    }

    QISRSessionEnd(session_id, hints);
    return;
  }

  while (MSP_REC_STATUS_COMPLETE != rec_stat)
  {
    const char *rslt = QISRGetResult(session_id, &rec_stat, 0, &errcode);
    if (MSP_SUCCESS != errcode)
    {
      qDebug() << "QISRGetResult failed, error code:" << errcode;
      if (NULL != f_pcm)
      {
        fclose(f_pcm);
        f_pcm = NULL;
      }
      if (NULL != p_pcm)
      {
        free(p_pcm);
        p_pcm = NULL;
      }

      QISRSessionEnd(session_id, hints);
      return;
    }
    if (NULL != rslt)
    {
      unsigned int rslt_len = strlen(rslt);
      total_len += rslt_len;
      if (total_len >= BUFFER_SIZE)
      {
        qDebug() << "no enough buffer for rec_result !";
        if (NULL != f_pcm)
        {
          fclose(f_pcm);
          f_pcm = NULL;
        }
        if (NULL != p_pcm)
        {
          free(p_pcm);
          p_pcm = NULL;
        }

        QISRSessionEnd(session_id, hints);
        return;
      }
      strncat(rec_result, rslt, rslt_len);
    }
    Sleep(150); //防止频繁占用CPU
  }
  QString temp = QString::fromLocal8Bit(rec_result);
  result = temp.remove(temp.length()-1, 1);
  if (NULL != f_pcm)
  {
    fclose(f_pcm);
    f_pcm = NULL;
  }
  if (NULL != p_pcm)
  {
    free(p_pcm);
    p_pcm = NULL;
  }

  QISRSessionEnd(session_id, hints);
}

