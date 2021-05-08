#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QColorSpace>
#include <QDir>
#include <QImageReader>
#include <QTimer>
#include <QThread>

#include <gphoto2.h>
#include <gphoto2-camera.h>
#include <gphoto2-context.h>
#include <gphoto2-port-result.h>
#include <gphoto2-widget.h>
#include <gphoto2-file.h>

#include <opencv2/opencv.hpp>
#include <opencv2/cudacodec.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/cudaimgproc.hpp>
#include <opencv2/cudaarithm.hpp>
#include <opencv2/cudafeatures2d.hpp>
#include <opencv2/cudafilters.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/dnn/all_layers.hpp>

#include <ultimateALPR-SDK-API-PUBLIC.h>
#include "alpr_utils.h"

#include "imagethread.h"

#define PREVIEW		"preview.jpg"

//using namespace ultimateAlprSdk;
//#	define ASSET_MGR_PARAM()

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    GPContext* sample_create_context(void);
    void GetCameraInfor();

    void InitALPR_SDK();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_disconnectcamera_clicked();
    void CapturePreview();
    void on_pushButton_CameraReconnect_clicked();

    void on_pushButton_disconnectcamera_2_released();

    void setTextbox(QString res);

private:
    Ui::MainWindow *ui;
    Camera		*camera;
    GPContext	*context;
    QImage image;
    double scaleFactor = 1;
    void setImage(const QImage &newImage);
    bool loadFile(const QString &fileName);
    bool loadFileAlpr();
    bool loadFileCV();
    bool loadFileCVCuda();
    void testALPR();
    void testALPR2(QImage imgIn);
    QTimer *timer;

    float CONFIDENCE_THRESHOLD = 0;
    float NMS_THRESHOLD = 0.4;
    int NUM_CLASSES = 80;

    // colors for bounding boxes
    cv::Scalar colors[3];

    int imgType = 0;

     char* __jsonConfig;
     bool ALPRSDK_inited = false;

     ImageThread thread;

};
#endif // MAINWINDOW_H
