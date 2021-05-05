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

}

MainWindow::~MainWindow()
{
    delete ui;
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
    // try gphoto2
    //int		retval, nrcapture = 0;
    //struct timeval	tval;
    context = sample_create_context();

    //gp_log_add_func(GP_LOG_ERROR, errordumper, NULL);
    /*gp_log_add_func(GP_LOG_DATA, errordumper, NULL); */
    gp_camera_new(&camera);
    /* This call will autodetect cameras, take the
         * first one from the list and use it. It will ignore
         * any others... See the *multi* examples on how to
         * detect and use more than the first one.
         */
    ret = gp_camera_init (camera, context);
    if (ret < GP_OK) {
        ui->textEdit->insertPlainText("No camera auto detected.");
        qDebug("No camera auto detected.");
        gp_camera_free (camera);
    }
    else
    {
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
            /* Simple query of a string configuration variable. */
            //            ret = get_config_value_string (camera, "owner", &owner, context);
            //            if (ret >= GP_OK) {
            //                printf("Owner: %s\n", owner);
            //                free (owner);
            //            }


            gp_camera_exit (camera, context);
            gp_camera_free (camera);
            gp_context_unref (context);
        }
    }
}
void MainWindow::on_pushButton_disconnectcamera_clicked()
{
    CapturePreview();
    //QCoreApplication::quit();
}

void MainWindow::CapturePreview()
{
    int	retval;
    int	capturecnt = 0;

    retval = gp_camera_init(camera, context);
    if (retval != GP_OK) {
        qDebug("  Retval: %d\n", retval);
        exit (1);
    }
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
        loadFile(PREVIEW);
        gp_file_unref(file);

    }
    gp_camera_exit(camera, context);
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

    setImage(newImage);

    //setWindowFilePath(fileName);

    const QString message = tr("Opened \"%1\", %2x%3, Depth: %4")
        .arg(QDir::toNativeSeparators(fileName)).arg(image.width()).arg(image.height()).arg(image.depth());
    statusBar()->showMessage(message);
    return true;
}
