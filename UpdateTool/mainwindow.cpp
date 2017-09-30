#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    myPort = new MySeriorport;

    QStringList portList = myPort->initSeriorPortInfos();
    initPortCombox(portList); //初始化串口信息
}

MainWindow::~MainWindow()
{
    delete ui;
}


/**
 * @brief 刷新串口信息
 */
void MainWindow::initPortCombox(QStringList &portList)
{
    QString portName = ui->comboBox_port->currentText();

    ui->comboBox_port->clear();

    int ret = portList.size();

    if(ret)
    {
        //重新设置combox
        for(int i = 0 ; i < ret ; i++)
        {
            QString str = portList.at(i);
            //            qDebug() << "str:" <<str;
            ui->comboBox_port->insertItem(i,str);
        }

        //如果刷新后刷新前当前项仍存在，那么切换到该项，如果已不存在，那么切换到第一项
        if(portList.contains(portName))  //如果portlist含有之前的串口
        {
            int index = getIndex(portList,portName);
            qDebug() << "index:" << index;
            if(index != -1)
            {
                ui->comboBox_port->setCurrentIndex(index);
            }else
            {
                qDebug() << "串口不存在!!!";
            }
        }else
        {
            qDebug()<< "该串口已不存在";
            ui->comboBox_port->setCurrentText(0);
        }

    }else
    {
        qDebug() << "当前无串口连接";
        ui->pushButton_port->setText(tr(""));
    }

    //刷新串口信息，将会触发combox的index变化，将在对应槽函数中刷新buttontext；
}

/**
 * @brief 点击按钮刷新串口信息
 */
void MainWindow::on_pushButton_12_clicked()
{
    QStringList portList = myPort->updataSeriorPortinfos();
    initPortCombox(portList);
}

int MainWindow::getIndex(QStringList &list,QString &str)
{
    if(list.contains(str))
    {
        for(int i = 0 ; i < list.size() ;i++)
        {
            if(list.at(i) == str)
            {
                return i;
            }
        }
    }
    return -1;
}

/**
 * @brief 初始化按钮文字
 */
void MainWindow::initButtonText()
{
    QString str = ui->comboBox_port->currentText();

    if(!str.isEmpty())
    {
        if(myPort->portIsOpen(str))
        {
            ui->pushButton_port->setText(tr("关闭串口"));
        }else
            ui->pushButton_port->setText(tr("打开串口"));
    }else
        ui->pushButton_port->setText(tr(""));
}

void MainWindow::on_comboBox_port_currentIndexChanged(int index)
{

    mCurrentPort = ui->comboBox_port->currentText();
    if(!mCurrentPort.isEmpty())
    {
        initButtonText();
        myPort->viewAllPortStatus();
    }
}

/**
 * @brief 打开或者关闭串口
 */
void MainWindow::on_pushButton_port_clicked()
{
    QString str = ui->pushButton_port->text();
    QString portName = ui->comboBox_port->currentText();
    if(str == "打开串口")
    {
        myPort->openPort(portName);
    }else if(str == "关闭串口")
        myPort->closePort(portName);
    else
        qDebug() << "当前串口不存在！";

    initButtonText();
}

void MainWindow::sleep(unsigned int msec)
{
    QTime dieTime = QTime::currentTime().addMSecs(msec);
    while( QTime::currentTime() < dieTime )
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

/**
 * @brief MainWindow::on_selectfileBtn_clicked  选择升级文件
 */
void MainWindow::on_selectfileBtn_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this);
    ui->nameEdit->setText(filename);
    mUpdateFile = filename;
}

/**
 * @brief MainWindow::on_startBtn_clicked  点击升级按钮
 */
void MainWindow::on_startBtn_clicked()
{
    int ret = 0 ;
    sendUpdateCmd();

    do {
        if(responseUpdate() == true)
            break;
        sleep(1*1000);
        ret++;
    } while (ret < 5);  //收到应答，立即向下执行，否则等待5s再向下执行

    if(!mUpdateFile.isEmpty())
        sendFile();  //发送文件
}

/**
 * @brief MainWindow::sendUpdateCmd  发送升级命令
 */
void MainWindow::sendUpdateCmd()
{
    QByteArray array;
    uchar addr;

    if(!ui->addrEdit->text().isEmpty())
        addr = uchar(ui->addrEdit->text().toInt());
    else
        QMessageBox::warning(this,tr("waring"),tr("请选择执行板地址"),tr("确定"));

    send_to_packet(addr,array);

    if(!mCurrentPort.isEmpty() && myPort->portIsOpen(mCurrentPort))
        myPort->sendData(array,mCurrentPort);
}

/**
 * @brief MainWindow::responseUpdate  升级命令回应
 */
bool MainWindow::responseUpdate()
{
    QString responseStr = tr("Start Updata");

    if(!mCurrentPort.isEmpty() && myPort->portIsOpen(mCurrentPort))
    {
        QByteArray array = myPort->readData(mCurrentPort);
        QString str = QString(array);
        if(str == responseStr)
            return true;
    }
    return false;
}

/**
 * @brief MainWindow::sendFile  发送文件
 */
void MainWindow::sendFile()
{
    QFile file(mUpdateFile);

    int packetNum = getPacketNum(file.size()) ;
    qDebug() << "packet num" << packetNum;

    if(packetNum == 0)  //升级文件是否为空
    {
        QMessageBox::warning(this,tr("waring"),tr("升级文件为空，请重新选择！"),tr("确定"));
        return;
    }

    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) //升级文件是否打开
    {
        QMessageBox::warning(this,tr("warning"),tr("文件打开失败"),tr("确定"));
        return;
    }

    QTextStream in(&file);
    int ret = 0; //第几包
    int tick = 0;//用于延时
    bool recvsuccessful = false; //数据包接收成功

    while (!in.atEnd())
    {
        QByteArray array;
        uchar addr;

        ret++;
        QString str=in.read(TEXT_MAX_LEN);//读取长度为TEXT_MAX_LEN的内容
        char *data = str.toLatin1().data();

        if(!ui->addrEdit->text().isEmpty())
            addr = uchar(ui->addrEdit->text().toInt());

        text_send_packet(addr ,data,array);

        QString info = QString("packet %1 size is %2").arg(ret).arg(array.size());
        qDebug() << info;

        do {
            if(!mCurrentPort.isEmpty() && myPort->portIsOpen(mCurrentPort))
                myPort->sendData(array,mCurrentPort);

            do {
                tick++;
                if(responseSendFile(ret) == true)
                {
                    recvsuccessful = true; //接收成功
                    break;
                }
                sleep(1*1000);
            } while (tick < 3);

        } while (!recvsuccessful); //发送数据包，直到该数据包接收成功
    }

    if(ret == packetNum)
        QMessageBox::information(this,tr("information"),tr("软件升级完毕！"),tr("确定"));
    else
        QMessageBox::warning(this,tr("information"),tr("数据发送可能存在丢失，请检查！"),tr("确定"));
}

/**
 * @brief MainWindow::getPacketNum 获取文件数据包个数
 * @param bytes 该文件大小(字节总数)
 * @return
 */
int MainWindow::getPacketNum(int bytes)
{
    qDebug() << "file size"<< bytes;
    int num = 0 ;

    if(bytes > 0)
        num = bytes/TEXT_MAX_LEN;

    if(bytes%TEXT_MAX_LEN != 0)
        num++;

    return num;
}

/**
 * @brief MainWindow::responseSendFile 发送数据包接收成功应答
 * @param num
 */
bool MainWindow::responseSendFile(int num)
{
    QString responseStr = QString("Receive Packet %1 successful").arg(num);
    if(!mCurrentPort.isEmpty() && myPort->portIsOpen(mCurrentPort))
    {
        QByteArray array = myPort->readData(mCurrentPort);
        QString str = QString(array);
        if(str == responseStr)
            return true;
    }
    return false;
}
