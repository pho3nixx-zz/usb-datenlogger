#ifndef COMMANDERDATALOG_H
#define COMMANDERDATALOG_H

#include <QtGui/QWidget>
#include <QtCore>
#include <QtSerialPort/QSerialPort>
#include "iousbhandle.h"
#include <QDateTime>


namespace Ui {
class CommanderDataLog;
}

class CommanderDataLog : public QWidget
{
    Q_OBJECT
    
public:

    struct Settings {
        QString name;
        const static qint32 baudRate = QSerialPort::Baud38400;
        const static QSerialPort::DataBits dataBits = QSerialPort::Data8;
        const static QSerialPort::Parity parity= QSerialPort::NoParity;
        const static QSerialPort::StopBits stopBits = QSerialPort::OneStop;
        const static QSerialPort::FlowControl flowControl = QSerialPort::NoFlowControl;
    };

    explicit CommanderDataLog(QWidget *parent = 0);
    ~CommanderDataLog();
    
protected:
    void changeEvent(QEvent *e);

    
private slots:

    void on_lbSerialportList_currentIndexChanged(int index);
    void on_btInit_clicked();
    int openSerialPort();
    void closeSerialPort();

    int writeData(const QByteArray &data);
    void readData();
    void handleError(QSerialPort::SerialPortError error);
    void readEnded();


    void on_btReadMem_clicked();

    void on_btBeenden_clicked();

    void on_btDataDownload_clicked();

    void on_btSave_clicked();

public slots:
    void on_TimerSlot();


private:
    QStringList DataLoggerOptions;
    int fillSerialPortList();
    Ui::CommanderDataLog *ui;
    void enableAllTabs();
    void disableAllTabs();
    bool showingColon;
    QTimer *timer;
    Settings currentSettings;
    QSerialPort *serial;
    int serialPortController;
    QByteArray data;
    int bitcountOfDecInt (int &aDecInt);
    void enableDownloadButton();

    void handleRequest();

    //handle SerialIO Request I,M,Q,D,S,C,J,A
    void handleOptionI(const QList<QByteArray> &values);
    void handleOptionM(const QList<QByteArray> &values);
    void handleOptionQ();
    void handleOptionD(const QList<QByteArray> &values);
    void handleOptionS();
    void handleOptionC();
    void handleOptionJ();
    void handleOptionA();
    void handleUnknown();

    // Vars for storing Results:
    QString mLoggerName;
    QString mLoggerType;
    QString mLoggerVersion;
    uint mDataCount;
    uint mDataInterval;
    QString mDataUnit;
    QString mDateHex1;
    QString mDateHex2;
    uint mSecondsSince2010;
    int mDataValCount;

    QByteArray mDataStore;

    int row;

    QByteArray mdelimiter;





};

#endif // COMMANDERDATALOG_H
