#include "gpio.h"

gpio::gpio(QObject *parent) : QThread(parent)
{
    // GPIO16 = pin 19 control LED of the button
    gpio_export(16);
    gpio_set_dir(16, 1);
    gpio_set_value(16,0);
    //gpio_set_edge(16, "rising");
    //gpio_fd = gpio_fd_open(gpio);

    // GPIO17 = pin 21 connect to source of the button
    //gpio_export(17);
    //gpio_set_dir(17, 1);
    //gpio_set_value(17,1);

    // GPIO 38 = pin 33
    gpio_export(38);
    gpio_set_dir(38, 0); // input
    QString str1 = "both";
    QByteArray ba = str1.toLocal8Bit();
    char *c_str2 = ba.data();
    gpio_set_edge(38, c_str2);

}

gpio::~gpio()
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

void gpio::processGPIO()
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

void gpio::run()
{

    // local variables
    QString result = "r";
    unsigned int value = 0;

    struct pollfd fdset[2];
    int nfds = 2;
    int gpio_fd, timeout, rc;
    char *buf[MAX_BUF];
    int len;
    //gpio_export(15);
    //gpio_set_dir(15, 0); // input
    //gpio_set_edge(15, "falling");
    gpio_fd = gpio_fd_open(38);



    timeout = POLL_TIMEOUT;

    while (1)
    {
        memset((void*)fdset, 0, sizeof(fdset));

        fdset[0].fd = STDIN_FILENO;
        fdset[0].events = POLLIN;

        fdset[1].fd = gpio_fd;
        fdset[1].events = POLLPRI;

        rc = poll(fdset, nfds, timeout);

        if (rc < 0) {
            printf("\npoll() failed!\n");
            break;
        }

        if (rc == 0) {
            //printf(".");
        }

        if (fdset[1].revents & POLLPRI) {
            lseek(fdset[1].fd, 0, SEEK_SET);
            len = read(fdset[1].fd, buf, MAX_BUF);
            //printf("\npoll() GPIO %d interrupt occurred\n", gpio);
            //printf("\n +");
            gpio_get_value(38, &value);
            if (value==1)
            {
                gpio_set_value(16,0);
                result = "r";
                //printf("r ");

            }
            else
            {
                gpio_set_value(16,1);
                result = "F";
                //printf("F");
            }

            emit processedGPIO(result);
            qDebug() <<"gpio: "<< result;
        }

//        if (fdset[0].revents & POLLIN) {
//            (void)read(fdset[0].fd, buf, 1);
//            //printf("\npoll() stdin read 0x%2.2X\n", (unsigned int) buf[0]);


//        }

        fflush(stdout);
        QThread::msleep(100);
    }

}

/****************************************************************
 * gpio_export
 ****************************************************************/
int gpio::gpio_export(unsigned int gpio)
{
    int fd, len;
    char buf[MAX_BUF];

    fd = open(SYSFS_GPIO_DIR "/export", O_WRONLY);
    if (fd < 0) {
        perror("gpio/export");
        return fd;
    }

    len = snprintf(buf, sizeof(buf), "%d", gpio);
    write(fd, buf, len);
    close(fd);

    return 0;
}

/****************************************************************
 * gpio_unexport
 ****************************************************************/
int gpio::gpio_unexport(unsigned int gpio)
{
    int fd, len;
    char buf[MAX_BUF];

    fd = open(SYSFS_GPIO_DIR "/unexport", O_WRONLY);
    if (fd < 0) {
        perror("gpio/export");
        return fd;
    }

    len = snprintf(buf, sizeof(buf), "%d", gpio);
    write(fd, buf, len);
    close(fd);
    return 0;
}

/****************************************************************
 * gpio_set_dir
 ****************************************************************/
int gpio::gpio_set_dir(unsigned int gpio, unsigned int out_flag)
{
    int fd, len;
    char buf[MAX_BUF];

    len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR  "/gpio%d/direction", gpio);

    fd = open(buf, O_WRONLY);
    if (fd < 0) {
        perror("gpio/direction");
        return fd;
    }

    if (out_flag==1)
        write(fd, "out", 4);
    else
        write(fd, "in", 3);

    close(fd);
    return 0;
}

/****************************************************************
 * gpio_set_value
 ****************************************************************/
int gpio::gpio_set_value(unsigned int gpio, unsigned int value)
{
    int fd, len;
    char buf[MAX_BUF];

    len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);

    fd = open(buf, O_WRONLY);
    if (fd < 0) {
        perror("gpio/set-value");
        return fd;
    }

    if (value)
        write(fd, "1", 2);
    else
        write(fd, "0", 2);

    close(fd);
    return 0;
}

/****************************************************************
 * gpio_get_value
 ****************************************************************/
int gpio::gpio_get_value(unsigned int gpio, unsigned int *value)
{
    int fd, len;
    char buf[MAX_BUF];
    char ch;

    len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);

    fd = open(buf, O_RDONLY);
    if (fd < 0) {
        perror("gpio/get-value");
        return fd;
    }

    read(fd, &ch, 1);

    if (ch != '0') {
        *value = 1;
    } else {
        *value = 0;
    }

    close(fd);
    return 0;
}


/****************************************************************
 * gpio_set_edge
 ****************************************************************/

int gpio::gpio_set_edge(unsigned int gpio, char *edge)
{
    int fd, len;
    char buf[MAX_BUF];

    len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/edge", gpio);

    fd = open(buf, O_WRONLY);
    if (fd < 0) {
        perror("gpio/set-edge");
        return fd;
    }

    write(fd, edge, strlen(edge) + 1);
    close(fd);
    return 0;
}

/****************************************************************
 * gpio_fd_open
 ****************************************************************/

int gpio::gpio_fd_open(unsigned int gpio)
{
    int fd, len;
    char buf[MAX_BUF];

    len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);

    fd = open(buf, O_RDONLY | O_NONBLOCK );
    if (fd < 0) {
        perror("gpio/fd_open");
    }
    return fd;
}

/****************************************************************
 * gpio_fd_close
 ****************************************************************/

int gpio::gpio_fd_close(int fd)
{
    return close(fd);
}
