#include "gps.h"

gps::gps(QObject *parent) : QThread(parent)
{


}

gps::~gps()
{
    mutex.lock();
    // DeInit
    // Call this function before exiting the app to free the allocate resources
    // You must not call process() after calling this function

    abort = true;
    condition.wakeOne();
    mutex.unlock();
    wait();
}

void gps::processGPS()
{
    QMutexLocker locker(&mutex);

    //this->imgIn = img;
    //Image size: 960x720 = 2764800

    if (!isRunning()) {
        start(LowPriority);
    } else {
        restart = true;
        condition.wakeOne();
    }
}

void gps::run()
{

    // local variables
    QString result = "r";
    emit processedGPS(result);

}

