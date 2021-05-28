#ifndef LASER_H
#define LASER_H

#include <QObject>
#include <QThread>

#include <QSize>

#include <QDebug>
#include <QElapsedTimer>
#include <QSerialPort>


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>



class laser: public QObject
{
    Q_OBJECT
public:
    laser();
    ~laser();
    void request_distance();
    void stop_measurement();
    void request_speed();
signals:
    void processedLaser(QString res);

private slots:
    void handleUartError(QSerialPort::SerialPortError error);
    void readUartData();
private:


    QSerialPort *uart;
    bool connectUart();

};

#endif // LASER_H
