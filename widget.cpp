#include "widget.h"
#include "ui_widget.h"
#include <QDebug>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    this->setLayout(ui->horizontalLayout_all);
    // 初始化串口对象
    comSerail = new QSerialPort(this);
    timerOneSend = new QTimer(this);
    timerContinousSend = new QTimer(this);//定时器发送数据
    // 连接刷新按钮信号
    connect(ui->pushButton_refreshPort, &QPushButton::clicked, this, &Widget::on_pushButton_refreshPort_clicked);
    //绑定QSerialPort::readyRead的信号与槽，用于读取数据
    connect(comSerail,&QSerialPort::readyRead,this,&Widget::comSerail_readToRead);
    //定时器，每隔1s钟更新时间
    connect(timerOneSend,&QTimer::timeout,[=](){
        getSysTimeSecend();
    });
    //绑定定时器，用于定时发送数据
    connect(timerContinousSend,&QTimer::timeout,this,&Widget::on_pushButton_Tx_clicked);
    connect(ui->textEdit_Tx, &QTextEdit::textChanged, this, &Widget::validateHex);
    // 首次加载串口列表
    on_pushButton_refreshPort_clicked();
    // 设置串口打开图标
    QPixmap pixmap0("F:/Qt/QT_learning/QtSerialPort/QtSerialPort/icon/off.png");  // 资源路径或文件路径
    Start_InitOperate(pixmap0);

}

Widget::~Widget()
{
    delete ui;
    if (comSerail->isOpen()) {
        comSerail->close();
    }
    delete comSerail;
}
//重写resize函数，是背景图片自适应缩放
void Widget::resizeEvent(QResizeEvent *event) {
    // 获取当前窗口尺寸
    QSize newSize = event->size();
    // 加载原始图片并缩放
    QImage backgroundImage(":/back.jpg");
    QImage scaledImage = backgroundImage.scaled(
        newSize,
        Qt::IgnoreAspectRatio,      // 忽略宽高比（完全填充）
        Qt::SmoothTransformation    // 平滑缩放
    );
    // 更新调色板
    QPalette palette;
    palette.setBrush(QPalette::Window, QBrush(scaledImage));
    this->setPalette(palette);
    this->setAutoFillBackground(true);  // 确保自动填充生效
    QWidget::resizeEvent(event);  // 调用基类处理
}

//获取所有的可用串口列表，并显示
void Widget::on_pushButton_refreshPort_clicked()
{
    // 清空现有列表
    ui->comboBox_SerialList->clear();
    // 获取可用串口
    const QList<QSerialPortInfo> portList  = QSerialPortInfo::availablePorts();

    // 构建显示文本
    QStringList portNames;
    foreach (const QSerialPortInfo &info, portList) {
        QString portName = info.portName();
        if (!portName.isEmpty()) {
//            portName += "-" + info.description();
            portNames << portName;
        }
    }
    // 添加到下拉框
    ui->comboBox_SerialList->addItems(portNames);
}

//界面初始化操作
void Widget::Start_InitOperate(QPixmap pixmap)
{
    // 1. 加载图片并设置图标
    ui->pushButton_Serial->setIcon(QIcon(pixmap));
    ui->pushButton_Serial->setIconSize(QSize(25, 25));  // 图标实际显示大小
    ui->pushButton_Serial->move(13, 13);  // X,Y坐标定位
    ui->pushButton_Serial->setStyleSheet("text-align: center;");  // 文字左对齐，留出图标空间

    //2. 串口默认配置
    timerOneSend->start(1000);
    //3. Qlabel样式表红色字体
    ui->label_Tips->setStyleSheet("color: rgb(255, 0, 0);");
    //3. 设置window样式
//    setWindowIcon(QIcon(":/logo.ico").pixmap(50,50));
    this->setWindowTitle("果壳儿串口V1.0");
    //4. 接收文本框设置
    ui->textEdit_Rx->setStyleSheet(
        "background-color: rgba(0, 0, 0, 10); "  // 背景透明
        "border: 3px solid #FFFFFF;"           // 自定义边框
        "color: #000000;"                      // 文本颜色
    );
    //5. 发送文本框设置
    ui->textEdit_Tx->setStyleSheet(
        "background-color: rgba(0, 0, 0, 10); "  // 背景透明
        "border: 3px solid #FFFFFF;"           // 自定义边框
        "color: #000000;"                      // 文本颜色
    );

}

//打开串口开始接收的槽函数
void Widget::on_pushButton_Serial_clicked()
{
    on_flag = !on_flag;
    if(on_flag){
        QPixmap pixmap1("F:/Qt/QT_learning/QtSerialPort/QtSerialPort/icon/on.png");  // 资源路径或文件路径
        Start_InitOperate(pixmap1);
        ui->pushButton_Serial->setText("关闭串口");

        //1.设置串口
        comSerail->setPortName(ui->comboBox_SerialList->currentText());
        //2.设置波特率
        comSerail->setBaudRate(ui->comboBox_BaudList->currentText().toInt());
        //3.设置数据位
        comSerail->setDataBits(QSerialPort::DataBits(ui->comboBox_DataList->currentText().toUInt()));
        //4.设置校验位
        switch (ui->comboBox_VerifyList->currentIndex()) {
        case 0:
            comSerail->setParity(QSerialPort::NoParity);
            break;
        case 1:
            comSerail->setParity(QSerialPort::OddParity);
            break;
        case 2:
            comSerail->setParity(QSerialPort::EvenParity);
            break;
        case 3:
            comSerail->setParity(QSerialPort::MarkParity);
            break;
        case 4:
            comSerail->setParity(QSerialPort::SpaceParity);
            break;
        default:
            comSerail->setParity(QSerialPort::NoParity);
        }
        //5.设置停止位
        comSerail->setStopBits(QSerialPort::StopBits
                                (ui->comboBox_StopList->currentText().toUInt()));
        //6.设置流控
        if(ui->checkBox_DTR->isChecked() || ui->checkBox_RTS->isChecked()){
            comSerail->setDataTerminalReady(ui->checkBox_DTR->isChecked());//DTR
            comSerail->setRequestToSend(ui->checkBox_RTS->isChecked());//RTS
        }
        else
            comSerail->setFlowControl(QSerialPort::NoFlowControl);//无流控
        //7.打开串口
        if(comSerail->open(QIODevice::ReadWrite)){
            qDebug() << "success open serial";
        }else
        {
            QMessageBox msgBox(QMessageBox::Question,QString(tr("果壳儿小助手")),
                               QString(tr("串口可能被占用,或设备被拔出")));
            msgBox.exec();
        }
      }

    else{
        comSerail->close();
        QPixmap pixmap0("F:/Qt/QT_learning/QtSerialPort/QtSerialPort/icon/off.png");  // 资源路径或文件路径
        Start_InitOperate(pixmap0);
        ui->pushButton_Serial->setText("打开串口");
    }
}
// textEdit接收数据
void Widget::comSerail_readToRead()
{
    if(!stopContinueBtn){
        comSerail->clear();
        return;
    }
    QString receiveMsg = comSerail->readAll();
    if(receiveMsg !=NULL) {
        RecCntTotal += receiveMsg.size();//累计接收数据个数
        //16进制显示
        if(ui->checkBox_HexRec->isChecked()){
            QByteArray tmpHexString = receiveMsg.toUtf8().toHex(' ').toUpper();//加以空格分开
            tmpHexString = tmpHexString+' ';
//          if(ui->checkBox_autoEnter->isChecked()) tmpHexString.append("\r\n");//是否自动换行
            if(ui->checkBox_timeStamp->isChecked())
                ui->textEdit_Rx->insertPlainText("【"+currentDateString+"】"+tmpHexString.append("\r\n"));
            else{
                if(ui->checkBox_autoEnter->isChecked())
                   ui->textEdit_Rx->insertPlainText(QString::fromUtf8(tmpHexString.append("\r\n")));
                else
                    ui->textEdit_Rx->insertPlainText(QString::fromUtf8(tmpHexString));
            }
        }else{
            if(ui->checkBox_timeStamp->isChecked())
               ui->textEdit_Rx->insertPlainText("【"+currentDateString+"】"+receiveMsg.append("\r\n"));
            else{
                if(ui->checkBox_autoEnter->isChecked())
                   ui->textEdit_Rx->insertPlainText(receiveMsg.append("\r\n"));
                else
                    ui->textEdit_Rx->insertPlainText(receiveMsg);
            }
        }
        ui->label_RxNUM->setText(QString::number(RecCntTotal));//显示接收数据
    }
    ui->textEdit_Rx->moveCursor(QTextCursor::End);
    ui->textEdit_Rx->ensureCursorVisible();
    //ui->textEditReceive->setFocus();
}
//点击发送数据
void Widget::on_pushButton_Tx_clicked()
{
    int writeCnt = 0;
    const char* sendData = ui->textEdit_Tx->toPlainText().toLocal8Bit().constData();
    //发送hex格式判断
    if(ui->checkBox_HexSend->isChecked()){
        QString tmpString  = ui->textEdit_Tx->toPlainText().replace(" ", "");
        QByteArray tmpArray = tmpString.toLocal8Bit().toHex().toUpper();
        //1.判断是否是偶数位
        if(tmpArray.size()%2 !=0 ){
            ui->label_Tips->setText("数据是奇数位！");
            return;
        }
        //2.转换为10进制QByteArrar再发送
        QByteArray qbArray = QByteArray::fromHex(tmpArray);
        // 转换为十进制数值发送
        QString decimalStr;
        // 按每两位解析十六进制字节
        for (int i = 0; i < qbArray.size(); i += 2) {
            QByteArray byte = qbArray.mid(i, 2);
            char c = static_cast<char>(byte.toInt(nullptr, 16)); // 转换为ASCII字符
            decimalStr.append(c);
        }
        QByteArray DeciResult = decimalStr.toUtf8(); // 结果为 "12345"
        //3.是否发送新行
        if(ui->checkBox_SendNewLine->isChecked()){
            DeciResult.append("\r\n");
        }
        //写入数据
        writeCnt = comSerail->write(DeciResult);
    }else{//1.如果不是16进制
        //发送新行
        if(ui->checkBox_SendNewLine->isChecked()){
            QByteArray arraySend(sendData,strlen(sendData));
            arraySend.append("\r\n");
            writeCnt = comSerail->write(arraySend);
        }else{
            writeCnt = comSerail->write(sendData);
        }
    }
    if(writeCnt == -1){
        ui->label_Tips->setText("发送失败!");
    }else{
        SendCntTotal += writeCnt;
        ui->label_Tips->setText("发送成功！");
        ui->label_TxNUM->setText(QString::number(SendCntTotal));

    }
}
//获取当前时间
void Widget::getCurrentTime()
{
        QDateTime dateTime = QDateTime::currentDateTime();
        QDate date = dateTime.date();
        int year = date.year();
        int month = date.month();
        int day = date.day();
        QTime time = dateTime.time();
        int hour = time.hour();
        int minute = time.minute();
        int second = time.second();

        currentDateString = QString("%1-%2-%3 %4:%5:%6")
                .arg(year,2,10,QChar('0'))
                .arg(month,2,10,QChar('0'))
                .arg(day,2,10,QChar('0'))
                .arg(hour,2,10,QChar('0'))
                .arg(minute,2,10,QChar('0'))
                .arg(second,2,10,QChar('0'));

}
//显示当前时间
void Widget::getSysTimeSecend()
{
    getCurrentTime();
    ui->lineEdit_currentTime->setText(currentDateString);
}
//定时发送数据
void Widget::on_checkBox_TimerSend_stateChanged(int arg1)
{
        //qDebug() << checked;
        if(arg1){
            timerContinousSend->start( ui->lineEdit_Period->text().toInt());
            ui->lineEdit_Period->setDisabled(true);
            ui->textEdit_Tx->setDisabled(true);
            qDebug()<<ui->lineEdit_Period->text().toInt();
        }else{
            timerContinousSend->stop();
            ui->lineEdit_Period->setDisabled(false);
            ui->textEdit_Tx->setDisabled(false);
        }
}
//停止或者继续接收数据
void Widget::on_pushButton_stop_clicked()
{
    stopContinueBtn = !stopContinueBtn;
}
//清空接收窗口内容
void Widget::on_pushButton_clear_clicked()
{
    ui->textEdit_Rx->clear();
}
//清空发送窗口按钮
void Widget::on_pushButton_TxClear_clicked()
{
    ui->textEdit_Tx->clear();
}
//计数清零
void Widget::on_pushButton_numClear_clicked()
{
    RecCntTotal = 0;
    SendCntTotal = 0;
    ui->label_RxNUM->setText(QString::number(RecCntTotal));
    ui->label_TxNUM->setText(QString::number(SendCntTotal));
}
//打开待发送的文件
void Widget::on_pushButton_OpenFile_clicked()
{
    SendfileName = QFileDialog::getOpenFileName(this,tr("打开文件"),
                                              "C:/Users/Public/Desktop/send",
                                              tr("Text (*.txt)"));
    ui->lineEdit_SendPath->setText(SendfileName);
}
//更改保存文件的路径
void Widget::on_pushButton_changePath_clicked()
{
    WritefileName = QFileDialog::getSaveFileName(this,tr("选择保存路径"),
                                                    "C:/Users/Public/Desktop/save",
                                                    tr("Text (*.txt)"));
    ui->lineEdit_SavePath->setText(WritefileName);
}
//开始发送文件数据
void Widget::on_pushButton_SendFile_clicked()
{
    if (QFile::exists(SendfileName)) {
        qDebug() << "文件存在";
    } else {
        QMessageBox::warning(this, "提示", "文件或路径不存在！");
    }
    // 检查串口是否打开
    if (!comSerail->isOpen()) {
        QMessageBox::warning(this, "提示", "串口未打开！");
        return;
    }
    // 读取文件内容
    // 检测文件实际编码（示例：优先 UTF-8，其次 GBK）
    QFile file(SendfileName);
    QByteArray rawData;
    if (file.open(QIODevice::ReadOnly)) {
        rawData = file.readAll();  // 保留所有原始字节
        file.close();
    }
    // 发送数据
    qint64 bytesWritten = comSerail->write(rawData);
    if (bytesWritten == -1) {
        QMessageBox::critical(this, "错误", "发送失败：" + comSerail->errorString());
    } else {
        SendCntTotal += bytesWritten;
        ui->label_TxNUM->setText(QString::number(SendCntTotal));
        QMessageBox::information(this, "提示", "文件发送完毕~");
    }
}
//点击保存接收的数据
void Widget::on_pushButton_SaveData_clicked()
{
    if(WritefileName != NULL){
        QFile file(WritefileName);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            return;
        QTextStream out(&file);
        //out.setCodec("UTF-8");
        out << ui->textEdit_Rx->toPlainText();
        file.close();
    }
}

void Widget::validateHex()
{
    ui->textEdit_Tx->blockSignals(true);
    if(ui->checkBox_HexSend->isChecked()){
        QString originalText = ui->textEdit_Tx->toPlainText();
        QString filteredText = originalText;
        // 移除所有非法字符（保留 0-9, A-F, a-f, 空格）
        QRegularExpression re("[^0-9A-Fa-f\\s]");
        filteredText.remove(re);
        ui->textEdit_Tx->setText(filteredText);
//        qDebug()<<"原始文本："<<filteredText;
        ui->textEdit_Tx->moveCursor(QTextCursor::End);
        ui->textEdit_Tx->ensureCursorVisible();
    }
    ui->textEdit_Tx->blockSignals(false);
}
//是否选择Hex发送
void Widget::on_checkBox_HexSend_stateChanged(int arg1)
{
    ui->textEdit_Tx->blockSignals(true);
    QString tmpString  = ui->textEdit_Tx->toPlainText();//QT继承了linux风格,换行符只编码成\n
    if(arg1==Qt::Checked){
        tmpString.replace("\n", "\r\n"); // 强制替换 LF 为 CRLF
        QByteArray tmpArray = tmpString.toLocal8Bit().toHex(' ').toUpper();
        ui->textEdit_Tx->setText(QString::fromLocal8Bit(tmpArray));
//        qDebug()<<tmpArray;
    }
    else{
        QString UTF8String = tmpString.trimmed().replace(" ", "");
        QByteArray byteArray = QByteArray::fromHex(UTF8String.toUtf8());;
        QString asciiText;
        for (const auto &byte : byteArray) {
            asciiText.append(static_cast<char>(byte));
        }
//        qDebug()<<asciiText;
        ui->textEdit_Tx->setText(asciiText);
    }
    ui->textEdit_Tx->blockSignals(false);
}

//终止发送文件数据
void Widget::on_pushButton_StopSendFile_clicked()
{
     comSerail->clear(QSerialPort::Output);
}
