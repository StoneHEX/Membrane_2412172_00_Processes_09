#ifndef DATACOLLECTOR_H
#define DATACOLLECTOR_H

#include <QMainWindow>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QFile>
#include <QCoreApplication>
#include <QTextStream>

QT_BEGIN_NAMESPACE
namespace Ui { class DataCollector; }
QT_END_NAMESPACE

#define WAIT_REPLY              100
#define NUM_DSC             4
#define FIRST_WSENSOR       1
#define LAST_WSENSOR        8
#define TEMP_SENSOR         9

class DataCollector : public QMainWindow
{
    Q_OBJECT

public:
    DataCollector(QWidget *parent = nullptr);
    ~DataCollector();

private slots:
    void on_Port_comboBox_currentTextChanged(const QString &arg1);

    void on_Power_pushButton_clicked();

    void on_Scan_pushButton_clicked();

    void on_GetSensorInfoCommand_pushButton_clicked();

    void on_ConcentratorVersion_pushButton_clicked();

    void on_SelectAlgoCSVFile_pushButton_clicked();

private:
    Ui::DataCollector *ui;

    QSerialPort serial;
    QByteArray serial_reply;
    int serial_started;

    QByteArray serial_tx( QByteArray hex_line);
    void store_sensor_data(int dsc , QByteArray reply );
    void algo( QByteArray reply,int dsc);

    int timer0Id;
    int timerint;

    QFile CsvFile;

    QTextStream CsvFileStream;
    QString filename;
    QString csvk_filename;

    int cmd_counter1,cmd_counter2,cmd_counter3,cmd_counter4;
    int toggle;

    float float_csv[32];
    int float_csv_loaded;

    float adcread,dac,conductivity_value,RW;
    int int_type,int_address,int_dac_value,int_adc_value,int_iterations,int_temperature;


protected:
    void timerEvent(QTimerEvent *event);
};
#endif // DATACOLLECTOR_H
