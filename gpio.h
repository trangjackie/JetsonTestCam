#ifndef GPIO_H
#define GPIO_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QSize>
#include <QWaitCondition>
#include <QDebug>
#include <QElapsedTimer>


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

#define SYSFS_GPIO_DIR "/sys/class/gpio"
#define POLL_TIMEOUT (3 * 1000) /* 3 seconds */
#define MAX_BUF 64

class gpio: public QThread
{
    Q_OBJECT
public:
    explicit gpio(QObject *parent = nullptr);
    ~gpio();
    void processGPIO();
signals:
    void processedGPIO(QString res);
protected:
    void run() override;

private:
    QMutex mutex;
    QWaitCondition condition;
    bool restart = false;
    bool abort = false;

    int gpio_export(unsigned int gpio);
    int gpio_unexport(unsigned int gpio);
    int gpio_set_dir(unsigned int gpio, unsigned int out_flag);
    int gpio_set_value(unsigned int gpio, unsigned int value);
    int gpio_get_value(unsigned int gpio, unsigned int *value);
    int gpio_set_edge(unsigned int gpio, char *edge);
    int gpio_fd_open(unsigned int gpio);
    int gpio_fd_close(int fd);
};

#endif // GPIO_H
