#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <gphoto2.h>
#include <gphoto2-camera.h>
#include <gphoto2-context.h>
#include <gphoto2-port-result.h>
#include <gphoto2-widget.h>

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
private slots:
    void on_pushButton_clicked();

    void on_pushButton_disconnectcamera_clicked();

private:
    Ui::MainWindow *ui;
    Camera		*camera;
    GPContext	*context;

};
#endif // MAINWINDOW_H
