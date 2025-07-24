#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include "QSerialPort"
#include "QSerialPortInfo"
#include <QMessageBox>
#include <QDateTime>
#include <QTimer>
#include <QFileDialog>
#include <QKeyEvent>
#include <QFileInfo>
#include <QTextCodec>
QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void resizeEvent(QResizeEvent *event) override;//重写resize函数
    void on_pushButton_refreshPort_clicked();
    void Start_InitOperate(QPixmap pixmap);
    void on_pushButton_Serial_clicked();
    void comSerail_readToRead(); //textEdit接收数据
    void getCurrentTime();//获取当前时间函数
    void getSysTimeSecend();//更新并显示时间
    void validateHex();//非法Hex发送检验函数
    void on_pushButton_stop_clicked();//停止接收，继续接收
    void on_pushButton_clear_clicked();//清空接收窗口内容
    void on_pushButton_changePath_clicked();//更改保存文件的路径
    void on_pushButton_SaveData_clicked();//点击保存接收的数据
    void on_pushButton_Tx_clicked();//点击发送数据
    void on_checkBox_HexSend_stateChanged(int arg1);//是否16进制发送数据
    void on_pushButton_TxClear_clicked();//清除发送框文本内容
    void on_pushButton_numClear_clicked();//清除发送接收计数
    void on_checkBox_TimerSend_stateChanged(int arg1);//定时发送数据
    void on_pushButton_OpenFile_clicked();//打开待发送的文件
    void on_pushButton_SendFile_clicked();//发送文件数据
    void on_pushButton_StopSendFile_clicked();//终止发送文件数据

private:
    Ui::Widget *ui;//ui界面
    QSerialPort *comSerail;//串口实例
    QTimer *timerContinousSend;//定时器发送数据
    QTimer *timerOneSend;//定时器
    bool on_flag = 0;  //串口打开标志
    QString currentDateString;//时间戳
    bool dateShowStatus = 0;//时间戳显示标志位
    qint32 RecCntTotal = 0;//接收数据总大小
    qint32 SendCntTotal = 0;//发送数据总大小
    bool stopContinueBtn = 1;//停止和继续接收标志位
    QString sendHistory;//发送历史
    QString WritefileName;//保存数据的文件名称
    QString SendfileName;//待发送数据的文件名称

};
#endif // WIDGET_H
