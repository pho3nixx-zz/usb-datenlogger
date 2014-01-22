#include "commanderdatalog.h"
#include "ui_commanderdatalog.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>

#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QMessageBox>

CommanderDataLog::CommanderDataLog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CommanderDataLog)
{

    DataLoggerOptions << "i" << "m" << "q" << "d" << "s" << "c" << "j" << "a" ;


    //Init Vars
    mdelimiter= QByteArray(".");
    mLoggerName = "";
    mLoggerType = "";
    mLoggerVersion = "";

    mDataCount = 0;
    mDataInterval = 0;
    mDataUnit = "";
    mDateHex1 = "";
    mDateHex2 = "";
    mSecondsSince2010=0;
    mDataValCount = 2;
    row = 0;



    showingColon = true;
    timer = new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(on_TimerSlot()));
    timer->start(1000);
    serial = new QSerialPort(this);
    serialPortController=0;
    data="";
    connect(serial, SIGNAL(error(QSerialPort::SerialPortError)), this,
            SLOT(handleError(QSerialPort::SerialPortError)));

    connect(serial, SIGNAL(readyRead()), this, SLOT(readData()));
    connect(serial, SIGNAL(readChannelFinished()), this, SLOT(readEnded()));


    ui->setupUi(this);
    int success = fillSerialPortList();
    QStringList labels;
    labels << tr("Nr") << tr("Temp")<< tr("relH")<< tr("Unit")<< tr("Datetime");
    ui->tableWidget->setHorizontalHeaderLabels(labels);
    ui->tableWidget->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);
    ui->tableWidget->verticalHeader()->hide();
    ui->tableWidget->setShowGrid(true);
    ui->tableWidget->setRowCount(row);

    QFile mFile("outie.csv");
    mFile.open(QIODevice::WriteOnly);
}

CommanderDataLog::~CommanderDataLog()
{
    delete ui;
}

void CommanderDataLog::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void CommanderDataLog::enableAllTabs()
{
    ui->tabAlarm->setEnabled(true);
    ui->tabDownload->setEnabled(true);
    ui->tabSetting->setEnabled(true);
    ui->tabRealTime->setEnabled(true);

}

void CommanderDataLog::disableAllTabs()
{
     ui->tabAlarm->setEnabled(false);
     ui->tabDownload->setEnabled(false);
     ui->tabSetting->setEnabled(false);
     ui->tabRealTime->setEnabled(false);

}

void CommanderDataLog::on_btInit_clicked()
{

    currentSettings.name = ui->lbSerialportList->currentText();

    if (openSerialPort() == 0)
    {
        ui->btInit->setDisabled(true);

        if (serialPortController==1 && serial->isWritable() )
        {
            serial->write("I\r", 2);
        }

        ui->btReadMem->setEnabled(true);
    }



}


 int CommanderDataLog::fillSerialPortList()
 {
     ui->lbSerialportList->clear();
     foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
     {
         QStringList list;
         list << info.portName();
         ui->lbSerialportList->addItem(list.last(), list);
     }

     return 0;
 }

void CommanderDataLog::on_lbSerialportList_currentIndexChanged(int index)
{
    if (index != -1)
    {
        QStringList list = ui->lbSerialportList->itemData(index).toStringList();

    }
}

int CommanderDataLog::openSerialPort()
{
    QMessageBox msgBox;
    serial->setPortName(currentSettings.name);
    if (serial->open(QIODevice::ReadWrite)) {
        if (serial->setBaudRate(currentSettings.baudRate)
                && serial->setDataBits(currentSettings.dataBits)
                && serial->setParity(currentSettings.parity)
                && serial->setStopBits(currentSettings.stopBits)
                && serial->setFlowControl(currentSettings.flowControl))
        {

            //msgBox.setText(tr("Successfully Connected to %1").arg(currentSettings.name));
            //msgBox.exec();
            serialPortController=1; //mark as opened
        }
        else
        {
            serial->close();

            msgBox.setText(tr("Fehler beim Setzen der Parameter\n")+ serial->errorString());
            msgBox.exec();
            return -1;
        }
    }
    else {
             msgBox.setText(tr("Fehler beim Öffnen\n")+ serial->errorString());
             msgBox.exec();
             return -1;

        }
    return 0;
}

void CommanderDataLog::closeSerialPort()
{
    serialPortController=0;
    serial->close();
}

int CommanderDataLog::writeData(const QByteArray &data)
{
    return serial->write(data);
}

void CommanderDataLog::readData()
{

//    QByteArray data = serial->readAll();
//    console->putData(data);

    data.append(serial->readAll());
    //qDebug() << "Retmsg (" << data.size() << "): " << data;  // Häppchen ausgeben
    if (data.endsWith('\r'))
    {
        data.chop(1);
        emit handleRequest();
        //qDebug() << "  Carriage Return Gefunden, Übertragung komplett. (" << data.size() << "): " << data;
    }

}

void CommanderDataLog::handleError(QSerialPort::SerialPortError error)
{
    QMessageBox msgBox;
    if (error == QSerialPort::ResourceError) {
        msgBox.setText(tr("Kritischer Fehler\n")+ serial->errorString());
        closeSerialPort();
    }
}


void CommanderDataLog::on_TimerSlot()
{
    showingColon = !showingColon;               // toggle/blink colon
    QString t = QTime::currentTime().toString().left(5);
    if ( !showingColon )
        t[2] = ' ';
    if ( t[0] == '0' )
        t[0] = ' ';
    ui->lcdTime->display(t);                              // set LCD to current time
    QDate date = QDate::currentDate();
    QString d;
    d.sprintf( "%4d-%02d-%02d", date.year(), date.month(), date.day() );
    ui->lcdDate->display(d);


}

void CommanderDataLog::readEnded()
{
     qDebug() << "Fertig (" << data.size() << "): " << data;
}

void CommanderDataLog::handleRequest()
{

    QList<QByteArray> val = data.split(' ');

    switch(DataLoggerOptions.indexOf(val.at(0))){

    case 0: //I
        CommanderDataLog::handleOptionI(val);
    break;

    case 1:
        CommanderDataLog::handleOptionM(val);
    break;

    case 2:
    CommanderDataLog::handleOptionQ();
    break;

    case 3:
    CommanderDataLog::handleOptionD(val);
    break;
    case 4:
    CommanderDataLog::handleOptionS();
    break;
    case 5:
    CommanderDataLog::handleOptionC();
    break;
    case 6:
    CommanderDataLog::handleOptionJ();
    break;
    case 7:
    CommanderDataLog::handleOptionA();
    break;

    default:
        CommanderDataLog::handleUnknown();

    break;
    }

}

void CommanderDataLog::handleOptionI(const QList<QByteArray> &values)
{
    data="";
    mLoggerName = values.at(1);
    mLoggerType = values.at(2);
    mLoggerVersion = values.at(3);

    ui->lblDeviceName->setText(mLoggerName);
    ui->lblDeviceType->setText(mLoggerType + " " + mLoggerVersion);
//            qDebug() << "Auswertestring (" << data.size() << "): " << data;
//            qDebug() << "Geräte-Name : " << values.at(1);
//            qDebug() << "Geräte-Typ : " << values.at(2);
//            qDebug() << "Geräte-Version : " << values.at(3);



}

int CommanderDataLog::bitcountOfDecInt (int &aDecInt)
{

        int number = aDecInt; // this is input number
        uint oneCount = number & 1 ? 1 : 0;
        while(number = number >> 1)
        {
            if(number & 1)
                ++oneCount;
        }


        qDebug() << "# of ones : " << oneCount;
        return oneCount;
}

// Handle M
void CommanderDataLog::handleOptionM(const QList<QByteArray> &values)
{
    //m 14 3600 M 6db ab5e 3

    qDebug() << "Auswertestring (" << data.size() << "): " << data;

    data="";
    bool ok;

    mDataCount = values.at(1).toUInt(&ok,10);
    mDataValCount = values.at(6).toInt(&ok,10);
    qDebug() <<"datacount" << mDataCount << "# ValCount : " << mDataValCount << "values "<< values << "values at 6: "<< values.at(6) << "last Val: " << values.last();
    mDataValCount = bitcountOfDecInt(mDataValCount);
    ui->tableWidget->setRowCount(mDataCount / mDataValCount);
    mDataInterval = values.at(2).toInt(&ok,10);
    mDateHex1 = values.at(4);
    mDateHex2 = values.at(5);
    QByteArray mix; //Storing seconds since 2010 as hex
    mix.append(values.at(4));
    mix.append(values.at(5));

    mSecondsSince2010 = mix.toUInt(&ok,16);


    ui->lblDataCount->setText(values.at(1));
    ui->lblDataRate->setText(values.at(2));
    // M = Metrisch (°Celsius) I = Imperial (°Fahrenheit)
    mDataUnit = values.at(3);
    if (values.at(3)=="M") ui->lblDataUnit->setText("Metrisch (Celsius)");
    if (values.at(3)=="I") ui->lblDataUnit->setText("Imperial (Fahrenheit)");

//                qDebug() << "Auswertestring (" << data.size() << "): " << data;
//                qDebug() << "Anzahl geloggter Daten : " << values.at(1);
//                qDebug() << "Abtastrate (sek) : " << values.at(2);
//                qDebug() << "Einheit : " << values.at(3);
//                qDebug() << "SekundenSeit2010 : " << mSecondsSince2010;

        enableAllTabs();
        if (mDataCount > 0)
        enableDownloadButton();

}

void CommanderDataLog::enableDownloadButton()
{
    ui->btDataDownload->setEnabled(true);
}

void CommanderDataLog::handleOptionQ()
{

}


void CommanderDataLog::handleOptionD(const QList<QByteArray> &values)
{

    QDateTime dt;
    dt = QDateTime::fromString("2010-01-01 00:00:00", "yyyy-MM-dd hh:mm:ss");
    int secsto2010 = dt.toTime_t(); // 1262300400
    //uint newsecs =secsto2010 + mSecondsSince2010 + row * mDataInterval;
    uint newsecs = 1262300400 + mSecondsSince2010 + row * mDataInterval;
    QDateTime newdate = QDateTime::fromTime_t(newsecs);

    //qDebug() << "Auswertestring (" << data.size() << "):" << data << "row:" << row <<"1970 bis 2010:" << secsto2010<< "Seconds2010: " << mSecondsSince2010 << "+" << (row * mDataInterval) << "secs gesamt: " << newsecs << "Datum:"<< newdate.toString("yyyy-MM-dd hh:mm:ss");
    //mDataStore.append(data+"\n");


    QByteArray temperature= values.at(1);
    int pos1 = temperature.size() -1;
    temperature.insert(pos1, mdelimiter);
    QByteArray humidity= values.at(2);
    int pos2=humidity.size() -1;
    humidity.insert(pos2, mdelimiter);
    qDebug() << newdate.toString("yyyy-MM-dd hh:mm:ss")<<";"<< temperature<< ";"<< humidity;
    data="";



    QTableWidgetItem *Nr = new QTableWidgetItem();
    Nr->setData(0,row);
    ui->tableWidget->setItem(row, 0, Nr);

    QTableWidgetItem *temp = new QTableWidgetItem();
    temp->setData(0,temperature);
    ui->tableWidget->setItem(row, 1, temp);

    QTableWidgetItem *relH = new QTableWidgetItem();
    relH->setData(0,humidity);
    ui->tableWidget->setItem(row, 2, relH);

    QTableWidgetItem *unit = new QTableWidgetItem();
    unit->setData(0,mDataUnit);
    ui->tableWidget->setItem(row, 3, unit);

    QTableWidgetItem *datum = new QTableWidgetItem();
    datum->setData(0,newdate.toString("yyyy-MM-dd hh:mm:ss"));
    ui->tableWidget->setItem(row, 4, datum);



    //ui->tableWidget->setItem(row, 1, new QTableWidgetItem(*values.at(1),0));
//    ui->tableWidget->setItem(row, 2, values.at(2));
//    ui->tableWidget->setItem(row, 3, mDataUnit);
//    ui->tableWidget->setItem(row, 4, mDateHex1);
    row += 1;

}
void CommanderDataLog::handleOptionS()
{

}
void CommanderDataLog::handleOptionC()
{

}
void CommanderDataLog::handleOptionJ()
{

}
void CommanderDataLog::handleOptionA()
{

}

void CommanderDataLog::handleUnknown()
{
    QMessageBox msgBox;
    msgBox.setText(tr("Unexpected Return. Try Again. Answer was: " + data));
}


void CommanderDataLog::on_btReadMem_clicked()
{

    if (serialPortController==1 && serial->isWritable() )
    {
        serial->write("M\r", 2);
    }

}

void CommanderDataLog::on_btBeenden_clicked()
{
    if (serialPortController==1)
    {
        serialPortController=0;
        serial->close();
        //qDebug() << "Serial Port geschlossen";
    }


}


void CommanderDataLog::on_btDataDownload_clicked()
{
    QMessageBox msgBox;

    if (mDataCount>0)
    {
        serial->write("D\r", 2);
    }
    else
    {
        msgBox.setText(tr("No Data."));
        msgBox.exec();
    }

}

void CommanderDataLog::on_btSave_clicked()
{
    QFile file("outtake.csv");
    file.open(QIODevice::WriteOnly);
    file.write(mDataStore);
    file.close();

}
