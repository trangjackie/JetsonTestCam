#include "laser.h"

laser::laser()
{
    // UART controller
    qDebug() << "init UART";
    uart = new QSerialPort();
    connect(uart, SIGNAL(errorOccurred(QSerialPort::SerialPortError)), this,
            SLOT(handleUartError(QSerialPort::SerialPortError)));


    //connect(this, SIGNAL(parameterUpdated()),this,SLOT(requestDistance()));
    //connect(this, SIGNAL(distanceUpdated()), this, SLOT(recordData()));

    // setup default parameters for uart connectors
    uart->setPortName("ttyUSB0"); // "ttyUSB0"
    uart->setBaudRate(QSerialPort::Baud115200); //(QSerialPort::Baud19200);
    uart->setDataBits(QSerialPort::Data8);
    uart->setParity(QSerialPort::Parity::NoParity);
    uart->setStopBits(QSerialPort::StopBits::OneStop);
    uart->setFlowControl(QSerialPort::FlowControl::NoFlowControl);

    connectUart();

    stop_measurement();
    connect(uart, SIGNAL(readyRead()), this, SLOT(readUartData()));
}

laser::~laser()
{
    uart->close();
}





bool laser::connectUart()
{
    // connect emulator port
    if (uart->open(QIODevice::ReadWrite)) {
        qDebug()<< "Laser: Serial port opened" ;
        return true;
    } else {
        qDebug() << "Laser: Serial port connect false";
        return  false;
    }
}

void laser::handleUartError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        qDebug( "Laser: Serial port error: ResourceError");
    }
    if (error == QSerialPort::DeviceNotFoundError){
        qDebug( "Laser: Serial port error: DeviceNotFoundError");
    }
}

void laser::readUartData()
{
    stop_measurement();//stop laser
    QByteArray data;
    data.append( uart->readAll());
    //while (uart->waitForReadyRead(1))
            data.append(uart->readAll());

    if (uart->error() == QSerialPort::ReadError) {
        qDebug("Laser: Failed to read from port ");
    } else if (uart->error() == QSerialPort::TimeoutError && data.isEmpty()) {
        qDebug("Laser: No data was currently available for reading from port ");
    }

    QString str = QString::fromStdString(data.toStdString());
    qDebug() << "Laser data: " << str;


}

void laser::request_distance()
{
    QByteArray cmd1("\x0D\x0A\x4F\x4E\x0D\x0A");
    uart->write(cmd1);
}
void laser::stop_measurement()
{
    QByteArray cmd2("\x0D\x0A\x4F\x46\x46\x0D\x0A");
    uart->write(cmd2);
}
void laser::request_speed()
{
    QByteArray cmd3("\x0D\x0A\x4D\x4F\x44\x45\x3D\x31\x0D\x0A");
    uart->write(cmd3);
}
