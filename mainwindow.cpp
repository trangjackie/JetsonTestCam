#include "mainwindow.h"
#include "ui_mainwindow.h"



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->pushButton->setText("Kết nối camera");
    ui->pushButton_disconnectcamera->setText("Start");

    ui->labelImage->setBackgroundRole(QPalette::Base);
    ui->labelImage->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    ui->labelImage->setScaledContents(true);

    ui->scrollArea->setBackgroundRole(QPalette::Dark);
    ui->scrollArea->setVisible(false);

    // before connect camera
    // this process have to be killed: /usr/lib/gvfs/gvfsd-gphoto2
    // we will disable this from system, not inside this program
    //GetCameraInfor();
    context = sample_create_context();
    gp_camera_new(&camera);
    /* This call will autodetect cameras, take the
         * first one from the list and use it. It will ignore
         * any others... See the *multi* examples on how to
         * detect and use more than the first one.
         */
    int ret = gp_camera_init (camera, context);
    if (ret < GP_OK) {
        ui->textEdit->insertPlainText("No camera auto detected.");
        qDebug("No camera auto detected.");
        gp_camera_free (camera);
        gp_context_unref (context);
        camera = nullptr;
    }

    timer = new QTimer(this);
    timer->setInterval(75); // 10 fps = 100ms
    connect(timer, SIGNAL(timeout()), this, SLOT(CapturePreview()));


    //InitALPR_SDK();

    // For image processing thread
    connect(&thread, &ImageThread::processedImage,
            this, &MainWindow::setTextbox);
}

MainWindow::~MainWindow()
{
    delete ui;

    //
    gp_camera_exit(camera, context);
    gp_camera_free(camera);
    gp_context_unref(context);
    context = NULL;
    gp_camera_unref(camera);
    camera = NULL;


    // DeInit
    // Call this function before exiting the app to free the allocate resources
    // You must not call process() after calling this function
    UltAlprSdkResult result;
    ULTALPR_SDK_ASSERT((result = UltAlprSdkEngine::deInit()).isOK());
}


static void
ctx_error_func (GPContext *context, const char *str, void *data)
{
    //fprintf  (stderr, "\n*** Contexterror ***              \n%s\n",str);
    //fflush   (stderr);
    qDebug("\n*** GPhoto2: Contexterror ***              \n%s\n",str);
}

static void
ctx_status_func (GPContext *context, const char *str, void *data)
{
    //fprintf  (stderr, "%s\n", str);
    //fflush   (stderr);
    qDebug("GPhoto2: %s\n",str);
}

GPContext* MainWindow::sample_create_context() {
    GPContext *context;

    /* This is the mandatory part */
    context = gp_context_new();

    /* All the parts below are optional! */
    gp_context_set_error_func (context, ctx_error_func, NULL);
    gp_context_set_status_func (context, ctx_status_func, NULL);

    /* also:
    gp_context_set_cancel_func    (p->context, ctx_cancel_func,  p);
        gp_context_set_message_func   (p->context, ctx_message_func, p);
        if (isatty (STDOUT_FILENO))
                gp_context_set_progress_funcs (p->context,
                        ctx_progress_start_func, ctx_progress_update_func,
                        ctx_progress_stop_func, p);
     */
    return context;
}


//
void MainWindow::on_pushButton_clicked()
{
    GetCameraInfor();
}

void MainWindow::GetCameraInfor()
{
    int		ret;
    CameraText	text;
    ui->textEdit->clear();

    /* Simple query the camera summary text */
    ret = gp_camera_get_summary (camera, &text, context);
    if (ret < GP_OK) {
        ui->textEdit->insertPlainText("Camera failed retrieving summary.\n");
        gp_camera_free (camera);
    }
    else
    {
        QString str = QString("Summary:\n%1\n").arg( text.text);
        ui->textEdit->insertPlainText(str);
        qDebug("Summary:\n%s\n",text.text);
    }
}
void MainWindow::on_pushButton_disconnectcamera_clicked()
{
    imgType = 0;
    timer->start();
    //QCoreApplication::quit();
}

void MainWindow::CapturePreview()
{
    // entangle_camera_begin_job(cam);
    // err = gp_camera_capture_preview(cam->cam, datafile, cam->ctx);
    // entangle_camera_end_job(cam);
    int	retval;
    int	capturecnt = 0;


    //while (1)
    {
        CameraFile *file;
        char output_file[32];
        /*
             * Capture a preview on every loop. Save as preview.jpg.
             */
        retval = gp_file_new(&file);
        if (retval != GP_OK) {
            qDebug("gp_file_new: %d\n", retval);
            exit(1);
        }

        retval = gp_camera_capture_preview(camera, file, context);
        if (retval != GP_OK) {
            qDebug("gp_camera_capture_preview failed: %d\n", retval);
            //exit(1);
        }
        retval = gp_file_save(file, PREVIEW);
        if (retval != GP_OK) {
            qDebug("saving preview failed: %d\n", retval);
            exit(1);
        }
        //
        if (imgType==0)
        {
            loadFile(PREVIEW);
        }
        else if (imgType==1)
        {
            if (ALPRSDK_inited)
                loadFileAlpr();
            else
                InitALPR_SDK();
        }
        else if (imgType==2)
            loadFileCV();
        else if (imgType==3)
            loadFileCVCuda();

        gp_file_unref(file);
    }
}

void MainWindow::setImage(const QImage &newImage)
{
    image = newImage;
    if (image.colorSpace().isValid())
        image.convertToColorSpace(QColorSpace::SRgb);
    ui->labelImage->setPixmap(QPixmap::fromImage(image));
    //! [4]
    scaleFactor = 1.0;

    ui->scrollArea->setVisible(true);

}

bool MainWindow::loadFile(const QString &fileName)
{
    QImageReader reader(fileName);
    reader.setAutoTransform(true);
    const QImage newImage = reader.read();
    if (newImage.isNull()) {
        //        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
        //                                 tr("Cannot load %1: %2")
        //                                 .arg(QDir::toNativeSeparators(fileName), reader.errorString()));

        return false;
    }
    //! [2]
    thread.processImage(newImage);
    //testALPR2(newImage);
    setImage(newImage);

    //setWindowFilePath(fileName);

    return true;
}



bool MainWindow::loadFileCV()
{
    cv::Mat img;
    img = cv::imread(PREVIEW);
    ui->labelImage->setPixmap(QPixmap::fromImage(QImage(img.data, img.cols, img.rows, img.step, QImage::Format_BGR888)));
    ui->scrollArea->setVisible(true);
    return true;
}
bool MainWindow::loadFileCVCuda()
{
    cv::Mat img;
    img = cv::imread(PREVIEW, cv::IMREAD_GRAYSCALE);
    cv::cuda::GpuMat dst, src;
    src.upload(img);
    //cv::Ptr<cv::cuda::CLAHE> ptr_clahe = cv::cuda::createCLAHE(5.0, cv::Size(8, 8));
    //ptr_clahe->apply(src, dst);
    //cv::cuda::cvtColor(src, dst, cv::COLOR_BGR2GRAY);
    cv::Ptr<cv::cuda::Filter> gaussianFilter = cv::cuda::createGaussianFilter(src.type(),
                                                                              src.type(),
                                                                              cv::Size(7, 7),
                                                                              0);
    gaussianFilter->apply(src, src);
    cv::Ptr<cv::cuda::CannyEdgeDetector> cannyFilter = cv::cuda::createCannyEdgeDetector(50, 100);
    cannyFilter->detect(src, dst);
    cv::Mat result;
    dst.download(img);

    ui->labelImage->setPixmap(QPixmap::fromImage(QImage(img.data, img.cols, img.rows, img.step, QImage::Format_Grayscale8)));
    ui->scrollArea->setVisible(true);
    return true;
}

void MainWindow::on_pushButton_CameraReconnect_clicked()
{
    //    gp_camera_exit(camera, context);
    //    gp_camera_free(camera);
    //    gp_context_unref(context);
    //    context = NULL;
    //    gp_camera_unref(camera);
    //    camera = NULL;

    context = sample_create_context();
    gp_camera_new(&camera);

    int ret = gp_camera_init (camera, context);
    if (ret < GP_OK) {
        ui->textEdit->insertPlainText("No camera auto detected.");
        qDebug("No camera auto detected.");
        gp_camera_free (camera);
        gp_context_unref (context);
        camera = nullptr;
    }
}

void MainWindow::on_pushButton_disconnectcamera_2_released()
{
    if (imgType<3)
        imgType += 1;
    else
        imgType = 0;
    QString str = QString("PMode (%1)").arg(imgType);
    ui->pushButton_disconnectcamera_2->setText(str);
    timer->start();
}

void MainWindow::InitALPR_SDK()
{
    __jsonConfig =
        "{"
        "\"debug_level\": \"info\","
        "\"debug_write_input_image_enabled\": false,"
        "\"debug_internal_data_path\": \".\","
        ""
        "\"num_threads\": -1,"
        "\"gpgpu_enabled\": true,"
        "\"openvino_enabled\": true,"
        "\"openvino_device\": \"GPU\","
        ""
        "\"detect_roi\": [0, 0, 0, 0],"
        "\"detect_minscore\": 0.1,"
        ""
        "\"pyramidal_search_enabled\": true,"
        "\"pyramidal_search_sensitivity\": 0.28,"
        "\"pyramidal_search_minscore\": 0.3,"
        "\"pyramidal_search_min_image_size_inpixels\": 800,"
        ""
        "\"klass_lpci_enabled\": true,"
        "\"klass_vcr_enabled\": true,"
        "\"klass_vmm_enabled\": true,"
        ""
        "\"recogn_minscore\": 0.3,"
        "\"recogn_score_type\": \"min\""
        ""
        ",\"assets_folder\": \"/home/trang/Projects/Test/assets\""
        ",\"charset\": \"latin\""
        "}";

    // Local variable
    UltAlprSdkResult result;

    // Initialize the engine (should be done once)
    ULTALPR_SDK_ASSERT((result = UltAlprSdkEngine::init(
                __jsonConfig
                )).isOK());
    ALPRSDK_inited = true;
}

bool MainWindow::loadFileAlpr()
{
    QImageReader reader(PREVIEW);
    reader.setAutoTransform(true);
    const QImage newImage = reader.read();
    if (newImage.isNull()) {

        return false;
    }

    testALPR2(newImage);

    setImage(newImage);

    return true;
}

void MainWindow::testALPR()
{
    // local variables
    UltAlprSdkResult result;

    std::string pathFileImage;

    // Parsing args
    std::map<std::string, std::string > args;
    //    if (!alprParseArgs(argc, argv, args)) {
    //        printUsage();
    //        return -1;
    //    }

    pathFileImage = PREVIEW;

    // Decode image
    AlprFile fileImage;
    if (!alprDecodeFile(pathFileImage, fileImage)) {
        qDebug("Failed to read image file: %s", pathFileImage.c_str());
    }
    else
    {
        // Recognize/Process
        // We load the models when this function is called for the first time. This make the first inference slow.
        // Use benchmark application to compute the average inference time: https://github.com/DoubangoTelecom/ultimateALPR-SDK/tree/master/samples/c%2B%2B/benchmark
        ULTALPR_SDK_ASSERT((result = UltAlprSdkEngine::process(
                    fileImage.type, // If you're using data from your camera then, the type would be YUV-family instead of RGB-family. https://www.doubango.org/SDKs/anpr/docs/cpp-api.html#_CPPv4N15ultimateAlprSdk22ULTALPR_SDK_IMAGE_TYPEE
                    fileImage.uncompressedData,
                    fileImage.width,
                    fileImage.height
                    )).isOK());
        //qDebug("Processing done.");
        // Print latest result
        const std::string& json_ = result.json();
        if (!json_.empty())
        {
            //qDebug("result: %s", json_.c_str());
            // "text":"29E23576*","
            std::string str = json_.c_str();
            std::size_t pos = str.find("text\":\"");      // position of "text":"" in str

            //if ((pos>0)&&(str.size()>pos+18))
            if (((int)pos)>0)
            {
                //qDebug("pos: %d",pos);
                std::string str2 = str.substr(pos, pos+17);     // get from "live" to the end
                std::size_t npos = str2.find("\",\""); // position of ","

                std::string str3 = str2.substr(7, (int)npos-7);
                qDebug("Number: %s", str3.c_str());
                ui->lineEdit_LPN->clear();
                ui->lineEdit_LPN->setText(str3.c_str());
            }

        }
    }
}

void MainWindow::testALPR2(QImage imgIn)
{
    UltAlprSdkResult result;

    //qDebug("Image size: %dx%d = %d",imgIn.width(), imgIn.height(), imgIn.sizeInBytes());
    {
        // Recognize/Process
        // We load the models when this function is called for the first time. This make the first inference slow.
        // Use benchmark application to compute the average inference time: https://github.com/DoubangoTelecom/ultimateALPR-SDK/tree/master/samples/c%2B%2B/benchmark
        //qDebug("Start Process.");
        ULTALPR_SDK_ASSERT((result = UltAlprSdkEngine::process(
                    ULTALPR_SDK_IMAGE_TYPE_RGBA32, // If you're using data from your camera then, the type would be YUV-family instead of RGB-family. https://www.doubango.org/SDKs/anpr/docs/cpp-api.html#_CPPv4N15ultimateAlprSdk22ULTALPR_SDK_IMAGE_TYPEE
                    (stbi_uc*)imgIn.bits(),
                    imgIn.width(),
                    imgIn.height()
                    )).isOK());
        //qDebug("Processing done.");
        // Print latest result
        const std::string& json_ = result.json();
        if (!json_.empty())
        {
            //qDebug("result: %s", json_.c_str());
            // "text":"29E23576*","
            std::string str = json_.c_str();
            std::size_t pos = str.find("text\":\"");      // position of "text":"" in str

            //if ((pos>0)&&(str.size()>pos+18))
            if (((int)pos)>0)
            {
                //qDebug("pos: %d",pos);
                std::string str2 = str.substr(pos, pos+17);     // get from "live" to the end
                std::size_t npos = str2.find("\",\""); // position of ","

                std::string str3 = str2.substr(7, (int)npos-7);
                qDebug("Number: %s", str3.c_str());

                ui->lineEdit_LPN->clear();
                ui->lineEdit_LPN->setText(str3.c_str());
            }

        }
    }
}
void MainWindow::setTextbox(QString res)
{
    ui->lineEdit_LPN->clear();
    ui->lineEdit_LPN->setText(res);
}

