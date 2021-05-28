#ifndef GPS_H
#define GPS_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QSize>
#include <QWaitCondition>
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



class gps: public QThread
{
    Q_OBJECT
public:
    explicit gps(QObject *parent = nullptr);
    ~gps();
    void processGPS();
signals:
    void processedGPS(QString res);
protected:
    void run() override;

private:
    QMutex mutex;
    QWaitCondition condition;
    bool restart = false;
    bool abort = false;


};

#endif // GPS_H
