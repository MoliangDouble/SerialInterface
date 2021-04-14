#include "maindialog.h"
#include "ui_maindialog.h"

MainDialog::MainDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MainDialog)
{
    ui->setupUi(this);
    count=0;
    start=0;
    serial=new QSerialPort();
    ui->baudNum->setCurrentIndex(0);
    ui->checkNum->setCurrentIndex(0);
    ui->dataNum->setCurrentIndex(0);
    ui->dataNum->setCurrentIndex(0);
    connect(ui->serialOperate,&QPushButton::pressed,this,&MainDialog::onOpenButtonClicked);
    connect(ui->serial_search,&QPushButton::clicked,this,&MainDialog::onSearchButtonClicked);
    connect(ui->clearTBox,&QPushButton::clicked,this,&MainDialog::onClearRButtonClicked);
    connect(ui->transButton,&QPushButton::clicked,this,&MainDialog::onSendButtonClicked);
    connect(ui->chartCheck,&QCheckBox::stateChanged,this,&MainDialog::onChartCheckChanged);
    connect(this,&MainDialog::drawChart,this,&MainDialog::drawChartView);
    connect(serial,&QSerialPort::readyRead,this,&MainDialog::SerialRead);
    connect(ui->chartclear,&QPushButton::clicked,this,&MainDialog::onChartClearClicked);
    ui->chartclear->setVisible(false);
    ui->chartclear->setEnabled(false);
    lineSeries=new QLineSeries();
    lineSeries->setUseOpenGL(true);
    chart=new QChart();
    chart->addSeries(lineSeries);
    chartView=new QChartView(chart);
    chartView->setRubberBand(QChartView::RectangleRubberBand);
    axisX=new QValueAxis();
    axisX->setRange(0,400);
    axisX->setLabelFormat("%d");
    axisX->setTitleText("时间/ms");
    axisX->setMinorTickCount(4);
    axisY=new QValueAxis();
    axisY->setRange(-1,2);
    axisX->setTickCount(10);
    axisY->setTickCount(2);
    axisY->setTitleText("light impluse");
    chart->setAxisX(axisX,lineSeries);
    chart->setAxisY(axisY,lineSeries);
    chart->legend()->hide();
    //chart->setTitle("graph");

}

MainDialog::~MainDialog()
{
    if(serial!=NULL)
    {
        if(serial->isOpen())
            serial->close();
        delete serial;
    }
    delete ui;
}

void MainDialog::onOpenButtonClicked()
{
    qDebug()<<"enter open function";
    if(ui->serialOperate->text().trimmed()=="打开串口")
    {
        qDebug()<<"enter open port";
        if((ui->serialPorts->currentIndex()==-1)||(ui->dataNum->currentIndex()==-1)||
                (ui->stop->currentIndex()==-1)||(ui->baudNum->currentIndex()==-1)||
                (ui->checkNum->currentIndex()==-1))
        {
            QMessageBox::about(this,"警告","有项为空");
            return;
        }
       serial->setPortName(ui->serialPorts->currentText());
       serial->setBaudRate(ui->baudNum->currentText().toInt());
       switch (ui->dataNum->currentText().toInt()) {
       case 7:
           serial->setDataBits(QSerialPort::Data7);
           break;
       case 8:
           serial->setDataBits(QSerialPort::Data8);
           break;
       default:
           break;
       }
       switch (ui->stop->currentText().toInt()) {
       case 1:
           serial->setStopBits(QSerialPort::OneStop);
           break;
       case 2:
           serial->setStopBits(QSerialPort::TwoStop);
           break;
       default:
           break;
       }
       switch (ui->checkNum->currentIndex()) {
       case 0:
           serial->setParity(QSerialPort::NoParity);
           break;
       case 1:
           serial->setParity(QSerialPort::OddParity);
           break;
       case 2:
           serial->setParity(QSerialPort::EvenParity);
            break;
       default:
           break;
       }
       serial->setFlowControl(QSerialPort::NoFlowControl);
       serial->open(QIODevice::ReadWrite);
       if(!serial->isOpen())
       {
           QMessageBox::about(this,"提示","串口打开失败");
       }
       else
       {
           ui->serialOperate->setText(QString("关闭串口"));
           ui->baudNum->setEnabled(false);
           ui->checkNum->setEnabled(false);
           ui->serialPorts->setEnabled(false);
           ui->dataNum->setEnabled(false);
           ui->stop->setEnabled(false);
           ui->baudNum->setEnabled(false);
           QMessageBox::about(this,"提示","串口打开成功");

       }

    }
    else if(ui->serialOperate->text().trimmed()=="关闭串口")
    {
        serial->close();
        ui->serialOperate->setText("打开串口");
        ui->baudNum->setEnabled(true);
        ui->checkNum->setEnabled(true);
        ui->serialPorts->setEnabled(true);
        ui->dataNum->setEnabled(true);
        ui->stop->setEnabled(true);
        ui->baudNum->setEnabled(true);
        QMessageBox::about(this,"提示","串口关闭成功");
    }
    else
    {
        QMessageBox::about(this,"提示","出问题了");
    }
}

void MainDialog::onSendButtonClicked()
{
    if(!serial->isOpen())
    {
        QMessageBox::about(this,"警告","串口未打开");
        return;
    }
    QByteArray datas=ui->trBox->toPlainText().toUtf8();
    if(ui->tIsHex->isChecked())
    {
        int cnt=datas.size();
        char*data=datas.data();
        for(int i=0;i<cnt;i++)
        {
            if(data[i]>'0'&& data[i]<='9')
                continue;
            else if(data[i]>='a' && data[i]<='f')
                continue;
            else if(data[i]>='A' && data[i]<='F')
                continue;
            else if(data[i]==' ')
                continue;
            else
            {
                QMessageBox::warning(this,"警告","有非16进制字符");
                return;
            }
        }
        datas=datas.fromHex(datas);
    }
    if(ui->enter->isChecked())
    {
        int len=datas.size();
        datas.insert(len,"\r\n");
    }
    serial->write(datas);
}

void MainDialog::onSearchButtonClicked()
{
    ui->serialPorts->clear();
    foreach (const QSerialPortInfo sif, QSerialPortInfo::availablePorts()) {
        ui->serialPorts->addItem(sif.portName());
    }
}
//每200条信息重置一次
void MainDialog::SerialRead()
{

    QString path=QDir::currentPath()+"/datas.txt";
    if(serial->isOpen())
    {
        while(!serial->atEnd())
        {
            QByteArray buffer=serial->read(100);
            count++;
            if(ui->risHex->isChecked())
            {
                buffer=buffer.toHex();
            }
            QString str=QString(buffer);
            ui->reBox->insertPlainText(str);
            if(ui->chartCheck->isChecked())
            {
                emit drawChart();
            }
            //qDebug()<<count;
            if(count==200)
            {
                count=0;

                QTextCodec* codex=QTextCodec::codecForName("GBK");
                QStringList strlists=ui->reBox->toPlainText().split("\n");
                QFile file(path);
                file.open(QIODevice::Append);
                if(file.isOpen())
                {
                    foreach (QString s, strlists) {
                        QByteArray buf=s.toUtf8();
                        buf=codex->fromUnicode(buf);
                        file.write(buf+"\r\n");
                    }
                }
                file.close();
                ui->reBox->clear();
                //ui->reBox->insertPlainText("");
            }
        }
    }
}

void MainDialog::onChartCheckChanged()
{
    if(ui->chartCheck->isChecked())
    {
        resize(QSize(900,750));
        chartView->setEnabled(true);
        chartView->setVisible(true);
        ui->chartclear->setVisible(true);
        ui->chartclear->setEnabled(true);
        ui->chartLayout->addWidget(chartView);
    }
    else
    {
        resize(QSize(900,514));
        chartView->setVisible(false);
        chartView->setEnabled(false);
        ui->chartclear->setVisible(false);
        ui->chartclear->setEnabled(false);
        ui->chartLayout->removeWidget(chartView);
    }
}

void MainDialog::onClearRButtonClicked()
{
    ui->reBox->clear();
}

void MainDialog::drawChartView()
{
    if(ui->chartCheck->isChecked())
    {
        if(!ui->reBox->toPlainText().isEmpty())
        {
            QStringList texts=ui->reBox->toPlainText().split("\n");

            for(int i=0;i<texts.size();i++)
            {
                if(texts.at(i)=="")
                {
                    texts.removeAt(i);
                }
            }
            int size=texts.size();
            QString interval=texts.takeLast();
            qDebug()<<"time is "+interval;
            QString status=texts.at(size-2);
            qDebug()<<"status is"+status;
            if(interval.toInt()==0)
            {
                qDebug()<<"类型错误";
                QMessageBox::about(this,"警告","类型错误");
                return;
            }
            start+=interval.toInt();
            if(chart->isVisible())
            {
               if(status.trimmed()=="low to high")
               {
                 *lineSeries<<(QPointF(start,0));
                 *lineSeries<<(QPointF(start,1));
                  if(axisX->max()<=start)
                  {
                      axisX->setMax(start+150);
                      //chart->scroll(200,0);
                     // axisX->setMin(start-100);
                  }
               }
               else if(status.trimmed()=="high to low")
               {
                 *lineSeries<<(QPointF(start,1));
                 *lineSeries<<(QPointF(start,0));
                   if(axisX->max()<=start)
                   {
                       axisX->setMax(start+150);
                       //chart->scroll(200,0);
                       //axisX->setMin(start-100);
                   }
               }
               else
               {

               }
            }
        }
    }
}

void MainDialog::onChartClearClicked()
{
    lineSeries->clear();
    start=0;
}


void MainDialog::on_reBox_textChanged()
{
    ui->reBox->moveCursor(QTextCursor::End);
}
