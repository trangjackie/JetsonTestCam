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

    api = new tesseract::TessBaseAPI();
    // Initialize tesseract-ocr with English, without specifying tessdata path
    if (api->Init(NULL, "eng")) {
        qDebug("Could not initialize tesseract.");
    }
    api->Init(NULL, "eng", tesseract::OEM_LSTM_ONLY);
    api->SetPageSegMode(tesseract::PSM_AUTO);
    api->SetVariable("tessedit_char_whitelist","ABCDEFGHIJKLMNOPQT0123456789");


    // For openalpr
    std::string country = "vn2";
    std::string configFile = "/usr/local/share/openalpr/openalpr.defaults.conf";
    int topn = 4;
    bool detectRegion = false;


    _alpr = new alpr::Alpr(country, configFile);
    _alpr->setTopN(topn);
    _alpr->setDetectRegion(detectRegion);
    if (_alpr->isLoaded() == false)
    {
       qDebug() << "Error loading OpenALPR" ;
    }
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

            // "text":"29E23576*","
            std::string str = json_.c_str();
            std::size_t pos = str.find("text\":\"");      // position of "text":"" in str

            //if ((pos>0)&&(str.size()>pos+18))
            if (((int)pos)>0)
            {
                //qDebug("pos: %d",pos);
                //qDebug("result: %s", json_.c_str());
                std::string str2 = str.substr(pos, pos+17);     // get from "live" to the end
                std::size_t npos = str2.find("\",\""); // position of ","

                std::string str3 = str2.substr(7, (int)npos-7);
                //qDebug("Number: %s", str3.c_str());
                finalresult = QString::fromUtf8(str3.c_str());
                finalresult2 = finalresult;
                // take the box of license plate
                //"text":"TTA88*","warpedBox":[521,291,674,291,674,375,521,375]}]}
                // 521,291-----521,375
                //    |           |
                // 674,291-----674,375,
                str2 = str.substr(pos);
                npos = str2.find("warpedBox\"");
                str3 = str2.substr(npos+12, npos+12+71);
                QString postionstr = QString(str3.c_str());
                QStringList list_p = postionstr.split(',');
                //                qDebug("p: "
                //                         + list_p[0].toLatin1()
                //                        + " " + list_p[1].toLatin1()
                //                        + " " + list_p[2].toLatin1()
                //                        + " " + list_p[3].toLatin1()
                //                        + " " + list_p[4].toLatin1()
                //                        + " " + list_p[5].toLatin1()
                //                        + " " + list_p[6].toLatin1());
                int x1 = (int)list_p[0].toFloat();
                int y1 = (int)list_p[1].toFloat();
                int x2 = (int)list_p[2].toFloat();
                int y2 = (int)list_p[3].toFloat();
                int x3 = (int)list_p[4].toFloat();
                int y3 = (int)list_p[5].toFloat();
                int x4 = (int)list_p[6].toFloat();
                int y4 = (int)list_p[7].toFloat();

                int h = abs(y1-y3);
                int w = abs(x1-x3);
                //QRect(int x, int y, int width, int height)
                QRect rect = QRect(x1,y1,w,h);
                //QRect rect2 = QRect(x1,y1,w,h);
                QRect rect2 = QRect(x1+30,y1,w,h);
                //qDebug()<<"p: "<< x1 << y1 << w << h;
                QImage plateimg = imgIn.copy(rect);
                //QImage plateimg2 = imgIn.copy(rect2);
                //QImage plateimg2 = QImage(w,(int)((float)h*0.4f),plateimg.format());
                //Qt::GlobalColor color = Qt::GlobalColor::red;
                //plateimg2.fill(color);

                //                QPainter p;
                //                p.begin(&plateimg2);
                //                  p.setPen(QPen(Qt::white));
                //                  p.setFont(QFont("Times", 22, QFont::Bold));
                //                  p.drawText(plateimg2.rect(), Qt::AlignCenter, "1 2 4");
                //                  p.end();

                //QImage reimg(plateimg.width()*2, plateimg.height(), plateimg.format()); // image to hold the join of image 1 & 2
                //QPainter painter(&reimg);
                //painter.drawImage(0, 0, plateimg); // xi, yi is the position for imagei
                //painter.drawImage(plateimg.width()-10, 0, plateimg2);
                //painter.drawImage(30, plateimg.height()-15, plateimg2);
                //emit processedImage(finalresult, reimg);
                //                ULTALPR_SDK_ASSERT((result = UltAlprSdkEngine::process(
                //                            ULTALPR_SDK_IMAGE_TYPE_RGBA32, // If you're using data from your camera then, the type would be YUV-family instead of RGB-family. https://www.doubango.org/SDKs/anpr/docs/cpp-api.html#_CPPv4N15ultimateAlprSdk22ULTALPR_SDK_IMAGE_TYPEE
                //                            (const void*)reimg.constBits(),
                //                            reimg.width(),
                //                            reimg.height()
                //                            )).isOK());
                //                // Print latest result
                //                const std::string& json2_ = result.json();
                //                if (!json2_.empty())
                //                {
                //                    std::string str4 = json2_.c_str();
                //                    //qDebug("result: %s", json2_.c_str());
                //                    std::size_t pos2 = str4.find("text\":\"");      // position of "text":"" in str
                //                    if (((int)pos2)>0)
                //                    {
                //                        std::string str5 = str4.substr(pos2);     // get from "live" to the end
                //                        std::size_t npos2 = str5.find("\",\""); // position of ","
                //                        std::string str6 = str5.substr(7, npos2-7);
                //                        finalresult2 = QString::fromUtf8(str6.c_str());
                //                    }
                //                }

                // Open input image with leptonica library
                //Pix *image = QImage2Pix(plateimg);
                //api->SetImage(image);
                //QImage pimg = plateimg.convertToFormat(Qt::RadialGradientPattern)
                im = cv::Mat(plateimg.height(),plateimg.width(),CV_8UC4,
                             plateimg.bits(),plateimg.bytesPerLine());
                cv::cvtColor(im, im,cv::COLOR_BGRA2GRAY);
                cv::Mat im2;
                cv::GaussianBlur(im, im2, cv::Size(0, 0), 3);
                cv::addWeighted(im, 1.5, im2, -0.5, 0, im);
                // denoise image
                //cv::GaussianBlur(im, im, cv::Size(7, 7), 0);
                cv::bitwise_not(im,im);
                //cv::threshold(im,im,100,255,cv::THRESH_BINARY);

                api->SetImage(im.data, im.cols, im.rows, 1, im.step);

                // Get OCR result
                std::string str = api->GetUTF8Text();
                qDebug("OCR output:\n%s", str.c_str());
                // find the last digit from output of OCR
                int slength = finalresult.length();
                if ((int)str.length()>=slength)
                {
                    if (isdigit(str[slength-1]))
                    {
                        finalresult2[slength-1] = str[slength-1];
                    }
                }

                // Destroy used object and release memory
                //api->End();
                //delete api;
                //delete [] outText;
                //pixDestroy(&image);


                // Openalpr test
                //detectandshow(_alpr, im);


                emit processedImage(finalresult2, plateimg, im);
            }

        }
    }
}

/**
   * @brief QImage2Pix QImage converted to PIX without sharing data
 * @param image
 * @return
 */
PIX* ImageThread::QImage2Pix(const QImage &image)
{
    PIX * pix;
    int width = image.width();
    int height = image.height();
    int depth = image.depth();
    pix = pixCreate(width, height, depth);
    if(image.isNull() )
    {
        //qDebug() << "image is null";
        return nullptr;
    }
    if( image.colorCount() )
    {
        QVector<QRgb> table = image.colorTable();

        PIXCMAP * map = pixcmapCreate(8);

        int n = table.size();
        for(int i = 0; i < n; i++)
        {
            pixcmapAddColor(map, qRed(table[i]), qGreen(table[i]), qBlue(table[i]));
        }
        pixSetColormap(pix, map);
    }
    int bytePerLine = image.bytesPerLine();
    l_uint32* start = pixGetData(pix);
    l_int32 wpld = pixGetWpl(pix);
    if(image.format() == QImage::Format_Mono || image.format() == QImage::Format_Indexed8 || image.format() == QImage::Format_RGB888)
    {
        for(int i = 0; i < height; i++)
        {
            const uchar * lines = image.scanLine(i);
            uchar * lined = (uchar *)(start + wpld * i) ;
            memcpy(lined , lines, static_cast<size_t>(bytePerLine));
        }
    }
    else if (image.format() == QImage::Format_RGB32 || image.format() == QImage::Format_ARGB32)
    {
        //qDebug() << "QImage::Format_RGB32";
        for(int i = 0; i < image.height(); i++)
        {
            const QRgb * lines = (const QRgb *)image.scanLine(i);
            l_uint32 * lined = start + wpld * i ;
            for(int j = 0; j < width; j ++)
            {
                uchar rval = qRed(lines[j]);
                uchar gval = qGreen(lines[j]);
                uchar bval = qBlue(lines[j]);
                l_uint32 pixel;
                composeRGBPixel(rval, gval, bval, &pixel);
                lined[j] = pixel;
            }
        }
    }
    return pix;
}


bool ImageThread::detectandshow( alpr::Alpr* alpr, cv::Mat frame)
{

  QElapsedTimer timer;
  timer.start();

  std::vector<alpr::AlprRegionOfInterest> regionsOfInterest;
  regionsOfInterest.push_back(alpr::AlprRegionOfInterest(0, 0, frame.cols, frame.rows));
  alpr::AlprResults results;
  if (regionsOfInterest.size()>0) results = alpr->recognize(frame.data, frame.elemSize(), frame.cols, frame.rows, regionsOfInterest);

  //qDebug() << "Total Time to process image:" << timer.elapsed() << "ms" << results.plates.size();


  {
    for (int i = 0; i < (int)results.plates.size(); i++)
    {
      qDebug() << "plate" << i << ": " << results.plates[i].topNPlates.size() << " results";


      for (int k = 0; k < (int)results.plates[i].topNPlates.size(); k++)
      {
        // Replace the multiline newline character with a dash
        std::string no_newline = results.plates[i].topNPlates[k].characters;
        std::replace(no_newline.begin(), no_newline.end(), '\n','-');

        qDebug() << "    - " << no_newline.c_str() << "\t confidence: " << results.plates[i].topNPlates[k].overall_confidence;

      }
    }
  }

  return results.plates.size() > 0;
}
