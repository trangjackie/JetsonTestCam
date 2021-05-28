#ifndef IMAGETHREAD_H
#define IMAGETHREAD_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QSize>
#include <QWaitCondition>
#include <QImage>
#include <QDebug>
#include <QPainter>
#include <QColor>
#include <QElapsedTimer>

#include <ultimateALPR-SDK-API-PUBLIC.h>
#include "alpr_utils.h"

#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>

#include <opencv2/opencv.hpp>
#include <opencv2/cudacodec.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/cudaimgproc.hpp>
#include <opencv2/cudaarithm.hpp>
#include <opencv2/cudafeatures2d.hpp>
#include <opencv2/cudafilters.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/dnn/all_layers.hpp>
#include <alpr.h>
#include <ctype.h>


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
    QString finalresult2;
    cv::Mat im;
signals:
    void processedImage(QString res, QImage plateimg, cv::Mat im);


protected:
    void run() override;

private:
    QMutex mutex;
    QWaitCondition condition;
    bool restart = false;
    bool abort = false;
     char* __jsonConfig;
     tesseract::TessBaseAPI *api;
     PIX* QImage2Pix(const QImage &image);

     alpr::Alpr* _alpr;
     bool detectandshow(alpr::Alpr* alpr, cv::Mat frame);
};

#endif // IMAGETHREAD_H
