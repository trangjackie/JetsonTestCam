#ifndef IMAGETHREAD_H
#define IMAGETHREAD_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QSize>
#include <QThread>
#include <QWaitCondition>
#include <QImage>

#include <ultimateALPR-SDK-API-PUBLIC.h>
#include "alpr_utils.h"

class ImageThread : public QThread
{
    Q_OBJECT
public:
    explicit ImageThread(QObject *parent = nullptr);
~ImageThread();
    void processImage(QImage img);
    std::string filepath;
    QImage imgIn;
    QString finalresult;
signals:
    void processedImage(QString res);


protected:
    void run() override;

private:
    QMutex mutex;
    QWaitCondition condition;
    bool restart = false;
    bool abort = false;
     char* __jsonConfig;
};

#endif // IMAGETHREAD_H
