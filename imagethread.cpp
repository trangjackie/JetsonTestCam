#include "imagethread.h"

ImageThread::ImageThread(QObject *parent) : QThread(parent)
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
}

ImageThread::~ImageThread()
{
    mutex.lock();
    // DeInit
    // Call this function before exiting the app to free the allocate resources
    // You must not call process() after calling this function
    UltAlprSdkResult result;
    ULTALPR_SDK_ASSERT((result = UltAlprSdkEngine::deInit()).isOK());

    abort = true;
    condition.wakeOne();
    mutex.unlock();
    wait();
}

void ImageThread::processImage(QImage img)
{
    QMutexLocker locker(&mutex);

    this->imgIn = img;
    //Image size: 960x720 = 2764800

    if (!isRunning()) {
        start(LowPriority);
    } else {
        restart = true;
        condition.wakeOne();
    }
}
void ImageThread::run()
{

    // local variables
    UltAlprSdkResult result;

    //qDebug("Image size: %dx%d = %d",imgIn.width(), imgIn.height(), imgIn.sizeInBytes());
    {
        // Recognize/Process
        // We load the models when this function is called for the first time. This make the first inference slow.
        // Use benchmark application to compute the average inference time: https://github.com/DoubangoTelecom/ultimateALPR-SDK/tree/master/samples/c%2B%2B/benchmark
        //qDebug("Start Process.");
        ULTALPR_SDK_ASSERT((result = UltAlprSdkEngine::process(
                    ULTALPR_SDK_IMAGE_TYPE_RGBA32, // If you're using data from your camera then, the type would be YUV-family instead of RGB-family. https://www.doubango.org/SDKs/anpr/docs/cpp-api.html#_CPPv4N15ultimateAlprSdk22ULTALPR_SDK_IMAGE_TYPEE
                    (const void*)imgIn.constBits(),
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
                //qDebug("Number: %s", str3.c_str());

                finalresult = QString::fromUtf8(str3.c_str());
                emit processedImage(finalresult);
            }

        }
    }
}
