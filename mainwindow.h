#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QColorSpace>
#include <QDir>
#include <QImageReader>
#include <gphoto2.h>
#include <gphoto2-camera.h>
#include <gphoto2-context.h>
#include <gphoto2-port-result.h>
#include <gphoto2-widget.h>
#include <gphoto2-file.h>

#define PREVIEW		"preview.jpg"

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
    void CapturePreview();
private slots:
    void on_pushButton_clicked();

    void on_pushButton_disconnectcamera_clicked();

private:
    Ui::MainWindow *ui;
    Camera		*camera;
    GPContext	*context;
    QImage image;
    double scaleFactor = 1;
    void setImage(const QImage &newImage);
    bool loadFile(const QString &fileName);
};
#endif // MAINWINDOW_H
