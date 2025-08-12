#include "datacollector.h"
#include "ui_datacollector.h"
#include <QApplication>
#include <QDebug>
#include <QTextStream>
#include <QDir>
#include <QDateTime>
#include <QFile>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QFile>
#include <QFileDialog>
#include <QCoreApplication>
#include <QTextStream>
#include <QThread>

DataCollector::DataCollector(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::DataCollector)
{
    ui->setupUi(this);
    ui->data_frame->setEnabled(false);
    ui->Info_frame->setEnabled(false);
    ui->DEBUG_frame->setEnabled(false);

    ui->Power_pushButton->setEnabled(false);
    ui->Scan_pushButton->setText("Scan");
    timer0Id = 0;
}

DataCollector::~DataCollector()
{
    delete ui;
}

QByteArray DataCollector::serial_tx( QByteArray hex_line)
{
QPixmap redled (":/ledred.png");
QPixmap greenled(":/ledgreen.png");
    if ( serial_started == 0 )
        return "1";
    serial.flush();
    serial.write(hex_line);
    if(serial.waitForReadyRead(WAIT_REPLY))
    {
        serial_reply = serial.readAll();
    }
    else
    {
        serial.flush();
        serial_reply = "";
    }
    qApp->processEvents();
    return serial_reply;
}

void DataCollector::on_Port_comboBox_currentTextChanged(const QString &arg1)
{
    QPixmap redled (":/ledred.png");
    QPixmap greenled(":/ledgreen.png");

    serial.close();
    if ( arg1 == "Invalid")
    {
        killTimer(timer0Id);
        ui->Comm_label->setPixmap(redled);
        ui->data_frame->setEnabled(false);
        ui->Info_frame->setEnabled(false);
        ui->DEBUG_frame->setEnabled(false);
        ui->statusbar->showMessage("Serial port closed");
        ui->Power_pushButton->setEnabled(false);
        return;
    }
    serial_started = 0;
    serial.setPortName(arg1);
    if(serial.open(QIODevice::ReadWrite))
    {
        if(!serial.setBaudRate(QSerialPort::Baud115200))
        {
            ui->Comm_label->setPixmap(redled);
            ui->statusbar->showMessage(arg1+" : "+serial.errorString());
            ui->data_frame->setEnabled(false);
            ui->Info_frame->setEnabled(false);
            ui->DEBUG_frame->setEnabled(false);
            ui->Power_pushButton->setEnabled(false);
        }
        else
        {
            ui->Comm_label->setPixmap(greenled);
            serial_started = 1;
            ui->statusbar->showMessage("Serial port "+arg1+" opened");
            serial.setReadBufferSize (1024);
            ui->Power_pushButton->setEnabled(true);
            ui->Info_frame->setEnabled(true);
            ui->DEBUG_frame->setEnabled(true);
        }
    }
    else
    {
        ui->Comm_label->setPixmap(redled);
        ui->statusbar->showMessage(arg1+" : "+serial.errorString());
        ui->data_frame->setEnabled(false);
        ui->Info_frame->setEnabled(false);
        ui->DEBUG_frame->setEnabled(false);
    }
}


void DataCollector::on_Power_pushButton_clicked()
{
    QByteArray reply;
    QString cmd;
    QPixmap redled (":/ledred.png");
    QPixmap greenled(":/ledgreen.png");

    if ( ui->Power_pushButton->text() == "Power ON" )
    {
        cmd = "<P>";
        ui->Power_pushButton->setText("Power OFF");
        ui->data_frame->setEnabled(true);
    }
    else
    {
        ui->Power_pushButton->setText("Power ON");
        cmd = "<O>";
        ui->data_frame->setEnabled(false);
    }

    serial.flush();
    qDebug()<< cmd;
    serial_tx(cmd.toUtf8());
}


void DataCollector::on_Scan_pushButton_clicked()
{
    QByteArray  reply;
    QString     cmd;
    QPixmap     redled (":/ledred.png");
    QPixmap     greenled(":/ledgreen.png");

    toggle = 0;
    if ( ui->Scan_pushButton->text() == "Stop")
    {
        ui->Scan_pushButton->setText("Scan");
        killTimer(timer0Id);
        ui->statusbar->showMessage("Stopped");
        ui->SCAN_label->setPixmap(greenled);

        cmd = "<H>";
        if ( (reply = serial_tx(cmd.toUtf8())) != "1" )
        {
            qDebug()<< "Received";
        }
    }
    else
    {
        ui->Scan_pushButton->setText("Stop");
        timer0Id = startTimer(1000);
        ui->SCAN_label->setPixmap(redled);
        cmd_counter1 = cmd_counter2 = cmd_counter3 = cmd_counter4 = 0;
        cmd = "<S " + ui->NumberOfSensors_comboBox->currentText()+" >";
        if ( (reply = serial_tx(cmd.toUtf8())) != "1" )
        {
            qDebug()<< "Received";
        }
        ui->statusbar->showMessage("Running Scan");
    }

}


void DataCollector::on_GetSensorInfoCommand_pushButton_clicked()
{
    QString cmd;

    cmd = "<J "+ui->SensorInfoLine_comboBox->currentText()+" "+ui->SensorInfoSensor_comboBox->currentText()+">";
    serial.flush();
    serial_tx(cmd.toUtf8());
    ui->label_SensorInfo->setText(serial_reply);
}


void DataCollector::on_ConcentratorVersion_pushButton_clicked()
{
    QString cmd;
    cmd = "<v>";
    serial.flush();
    qDebug()<< cmd;
    serial_tx(cmd.toUtf8());
    ui->label_CONCENTRATORVERSION->setText(serial_reply);
}


#ifdef Q_OS_WIN
QString dirPath= "c:/MembraneLog_3.2";
#else
QString dirPath= "/Devel/MembraneLog";
#endif
void DataCollector::algo( QByteArray reply,int dsc)
{
/*
 * 				Algo.adcread = (float )(AcqSystem.adc_in_value - 18);
                Algo.dac = (float )AcqSystem.dac_out_value;
                if ( Algo.adcread < (Algo.dac+10.0F))
                    Algo.RW = 1600.0F;
                else
                {
                    Algo.RW = (Algo.R2 / ((Algo.adcread/Algo.dac)-1));
                    if ( AcqSystem.iterations )
                        Algo.RW -= 10.0F;
                }
                algo_stop();
                AcqSystem.conductivity_value = (uint16_t )AcqSystem.adc_in_value;
                */
    /*
     * type address  dac_value  conductivity_value  iterations temperature
     * "01  08       0800       08ce                0000       0024"
    */
    const char* DataAsString = reply.constData();
    sscanf(DataAsString,"%d %d %04x %04x %04x %04x",&int_type,&int_address,&int_dac_value,&int_adc_value,&int_iterations,&int_temperature);
    if ( int_address == ui->QSENSOR_DEBUG_comboBox->currentText().toInt())
    adcread = (float )int_adc_value;
    dac = (float )int_dac_value;
    if ( ui->AlgoDebugEnable_checkBox->isChecked() == true )
    {
        if ( int_address == ui->QSENSOR_DEBUG_comboBox->currentText().toInt())
            if ( dsc == ui->QLINE_DEBUG_comboBox->currentText().toInt())
                qDebug()<< "On " << dsc << " -> adcread : "<< adcread << " dac: " << dac << "int_iterations : " << int_iterations;
    }

    if ( adcread < (dac+19.0F))
        RW=30000;
    else
    {
        RW = 10.0F / (((adcread-18.0F)/dac)-1);
        if ( int_iterations )
            RW -= 10.0F;
        if ( RW < 0.0F )
            RW = 0.1F;
    }
    /*
    if ( int_address == ui->QSENSOR_DEBUG_comboBox->currentText().toInt())
    {
        if ( dsc == ui->QLINE_DEBUG_comboBox->currentText().toInt())
        {
            if ( int_type )
                qDebug()<< "RW : "<< qSetRealNumberPrecision(9.3) << RW;// << " uS :"<< (1000.0F / RW) << qSetRealNumberPrecision(9.3);
        //        qDebug()<< " uS :"<< (1000.0F / RW) << qSetRealNumberPrecision(9.3);
        //        qDebug()<< "RW : "<< qSetRealNumberPrecision(9.3) << RW;
        }
    }
    */


    /*
    float pin_distance = 13.4F;
    float pin_dia = 0.75F;
    float pin_height = 5.0F;
    float area = pin_distance * pin_dia;
     * r = l / (sigma*a)
     * l = distanza tra i pin in cm. ( 1.3 )
     * a = area della sez trasversale della soluzione tra i puntali in cm2 ( 1.3 * 0.3 )
     * sigma = conducibilità in simens / cm è quello che cerchi pirla
     *
     * sigma = l / (r * a)
     * 1.3 / ( r * 0.12 * 0.3 )

    if ( int_type )
        qDebug()<< "uS/cm : "<< qSetRealNumberPrecision(9.3) << (1.3F / (0.13F * 0.3F * RW * 1000)) * 10000 ;
     */


}
void DataCollector::store_sensor_data( int dsc , QByteArray reply)
{
    QDateTime currentDateTime = QDateTime::currentDateTime();
    QDate currentDate = currentDateTime.date();
    QTime currentTime = currentDateTime.time();
    QDir top_directory(dirPath);
    int  concentrator_counter = 1;
    QByteArray q_concentrator_counter;
    q_concentrator_counter.setNum(concentrator_counter);
    QString TopDir_dayPath = dirPath+"/"+currentDate.toString("yyMMdd")+"_CON"+q_concentrator_counter+"_iCON";
    int cmd_counter,total_readout,calibration,tempPT1000;

    if ( !top_directory.exists())
    {
        qDebug()<< dirPath << " created";
        top_directory.mkpath(dirPath);
    }

    QDir top_directory_folderpath(TopDir_dayPath);
    if ( !top_directory_folderpath.exists())
    {
        qDebug()<< TopDir_dayPath << " created";
        top_directory_folderpath.mkpath(TopDir_dayPath);
    }

    QByteArray q_dsc;
    q_dsc.setNum(dsc);
    filename = TopDir_dayPath+"/"+currentDate.toString("yyMMdd")+"_CON"+q_concentrator_counter+"_DSC"+q_dsc+".csv";

    QFile file(filename);

    if ( ! file.exists())
    {
        qDebug()<< filename << " : File not present, created";
        CsvFile.setFileName(filename);
        CsvFile.open(QIODevice::Append | QIODevice::Text);
        CsvFileStream.setDevice(&CsvFile);
        CsvFileStream << "################################################################\n";
        CsvFileStream << "Concentrator version,v1.1-241212\n";
        CsvFileStream << "Sensors      version,v1.1-241212\n";
        CsvFileStream << "-,-\n";
        CsvFileStream << "-,-\n";
        CsvFileStream << "################################################################\n";
        CsvFileStream << "Time stamp,Sequence number,Concentrator,DSC,Sensor,Scale,Readout,Total Readout,Total Noise,Temp Micro,Temp PT1000,DAC,RW,Active,Status\n";
    }
    else
    {
        CsvFile.setFileName(filename);
        CsvFile.open(QIODevice::Append | QIODevice::Text);
        CsvFileStream.setDevice(&CsvFile);
    }

    calibration = 0;
    tempPT1000  = 0;
    switch(dsc)
    {
    case 1  :    cmd_counter = cmd_counter1; cmd_counter1++;break;
    case 2  :    cmd_counter = cmd_counter2; cmd_counter2++;break;
    case 3  :    cmd_counter = cmd_counter3; cmd_counter3++;break;
    case 4  :    cmd_counter = cmd_counter4; cmd_counter4++;break;
    }

    const char* DataAsString = reply.constData();
    QString timestamp = currentDate.toString("dd/MM/yy")+" "+currentTime.toString("hh:mm:ss");
    sscanf(DataAsString,"%d %d %04x %04x %04x %04x",&int_type,&int_address,&int_dac_value,&int_adc_value,&int_iterations,&int_temperature);
    if ( int_type == 1 )
    {
        algo(reply,dsc);
        if ((float_csv_loaded == 1) && (ui->useK_checkBox->isChecked() == true))
        {
            total_readout = int_adc_value * float_csv[int_iterations];
            if ( ui->CSVDebugEnable_checkBox->isChecked() == true )
               if ( dsc == ui->QLINE_DEBUG_comboBox->currentText().toInt())
                    if (( int_address == ui->QSENSOR_DEBUG_comboBox->currentText().toInt()) || (ui->QSENSOR_DEBUG_comboBox->currentText() == "All" ))
                        qDebug() << timestamp << "," << cmd_counter << "," << concentrator_counter << "," << dsc << "," << int_address << "," << int_iterations+1 << "," << int_adc_value << "," << total_readout << "," << calibration << "," << int_temperature << "," << tempPT1000<< "," << int_dac_value<< "," << RW << ",Y,A";
            CsvFileStream << timestamp << "," << cmd_counter << "," << concentrator_counter << "," << dsc << "," << int_address << "," << int_iterations+1 << "," << int_adc_value << "," << total_readout << "," << calibration << "," << int_temperature << "," << tempPT1000<< "," << int_dac_value<< "," << RW << ",Y,A\n";
        }
        else
        {
            total_readout = int_adc_value * (int_iterations+1);
            if ( ui->CSVDebugEnable_checkBox->isChecked() == true )
               if ( dsc == ui->QLINE_DEBUG_comboBox->currentText().toInt())
                    if (( int_address == ui->QSENSOR_DEBUG_comboBox->currentText().toInt()) || (ui->QSENSOR_DEBUG_comboBox->currentText() == "All" ))
                        qDebug() << timestamp << "," << cmd_counter << "," << concentrator_counter << "," << dsc << "," << int_address << "," << int_iterations+1 << "," << int_adc_value << "," << total_readout << "," << calibration << "," << int_temperature << "," << tempPT1000<< "," << int_dac_value<< "," << RW << ",Y,A";
            CsvFileStream << timestamp << "," << cmd_counter << "," << concentrator_counter << "," << dsc << "," << int_address << "," << int_iterations+1 << "," << int_adc_value << "," << total_readout << "," << calibration << "," << int_temperature << "," << tempPT1000<< "," << int_dac_value<< "," << RW << ",Y,A\n";
        }
    }
    CsvFile.close();
}

void DataCollector::timerEvent(QTimerEvent *event)
{
    QByteArray reply;
    QByteArray Command;
    QByteArray qline;
    QByteArray qsensor;
    int dsc,sensor;
    QPixmap redled (":/ledred.png");
    QPixmap greenled(":/ledgreen.png");

    if ( event->timerId() == timer0Id )
    {
        int sensor_debug = ui->QSENSOR_DEBUG_comboBox->currentText().toInt();// 0 if All
        int line_debug = ui->QLINE_DEBUG_comboBox->currentText().toInt(); // 0 if All

        if ( toggle )
            ui->SCAN_label->setPixmap(redled);
        else
            ui->SCAN_label->setPixmap(greenled);
        toggle++;
        toggle &= 1;

        for(dsc=1;dsc<NUM_DSC+1;dsc++)
        {
            for(sensor=FIRST_WSENSOR;sensor<LAST_WSENSOR+2;sensor++)
            {
                qline.setNum(dsc);
                qsensor.setNum(sensor);
                Command = "<A "+qline+" "+qsensor+">";
                if ( (reply = serial_tx(Command)) != "" )
                {
                    if ( ui->DSCDebugEnable_checkBox->isChecked() == true)
                    {
                        if (( line_debug == 0) && ( sensor_debug == 0))
                            qDebug()<< qline << " " << qsensor << " " << reply;
                        if (( line_debug == dsc) && ( sensor_debug == 0))
                            qDebug()<< qline << " " << qsensor << " " << reply;
                        if (( line_debug == 0) && ( sensor_debug == sensor))
                            qDebug()<< qline << " " << qsensor << " " << reply;
                        if (( line_debug == dsc) && ( sensor_debug == sensor))
                            qDebug()<< qline << " " << qsensor << " " << reply;
                    }
                    store_sensor_data(dsc,reply);
                }
            }
        }
    }
}

void DataCollector::on_SelectAlgoCSVFile_pushButton_clicked()
{
    QString filters = "CSV files (*.csv)";
#ifdef Q_OS_WIN
    csvk_filename = QFileDialog::getOpenFileName(this, tr("Open CSV File"), "c:/MembraneData",filters);
#else
    csvk_filename = QFileDialog::getOpenFileName(this, tr("Open CSV File"), "/Devel/MembraneData",filters);
#endif
    QFileInfo ficsv(csvk_filename);
    QString base = ficsv.completeBaseName() + "." +ficsv.completeSuffix();
    ui->label_CSVFILE->setText(base);
    QFile file(csvk_filename);

    if (!file.open(QIODevice::ReadOnly))
        qDebug()<<"File not found";
    else
    {
        int i=0;
        while(!file.atEnd())
        {
            QString line = file.readLine();
            QStringList cols = line.split(",");
            float_csv[i] = cols.at(0).toFloat();
            i++;
            if ( i >= 14 )
                qDebug()<<"Error " << i;

        }
        file.close();
        qDebug() <<  "###########";
        for(i=0;i<14;i++)
            qDebug()<<float_csv[i];
        float_csv_loaded = 1;
    }
}

