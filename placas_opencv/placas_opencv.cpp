/**
 * @file placas_opencv.cpp
 * @brief Background subtraction and OCR using webcam input
 */

#include <iostream>
#include <sstream>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>

using namespace cv;
using namespace std;

const char* params
    = "{ help h         |           | Print usage }"
      "{ algo           | MOG2      | Background subtraction method (KNN, MOG2) }";

int main(int argc, char* argv[])
{
    CommandLineParser parser(argc, argv, params);
    parser.about( "This program shows how to use background subtraction methods provided by "
                  " OpenCV. You can process both videos and images.\n" );
    if (parser.has("help"))
    {
        //print help information
        parser.printMessage();
        return 0;
    }

    //! [create]
    //create Background Subtractor objects
    Ptr<BackgroundSubtractor> pBackSub;
    if (parser.get<String>("algo") == "MOG2")
        pBackSub = createBackgroundSubtractorMOG2();
    else
        pBackSub = createBackgroundSubtractorKNN();
    //! [create]

    //! [capture]
    // Use the webcam as the input source
    VideoCapture capture(0);
    if (!capture.isOpened()){
        //error in opening the video input
        cerr << "Unable to open the webcam" << endl;
        return 0;
    }
    //! [capture]

    // Initialize Tesseract OCR
    tesseract::TessBaseAPI ocr;
    if (ocr.Init(NULL, "eng", tesseract::OEM_LSTM_ONLY)) {
        cerr << "Erro ao inicializar o Tesseract" << endl;
        return -1;
    }

    Mat frame, fgMask;
    while (true) {
        capture >> frame;
        if (frame.empty())
            break;

        //! [apply]
        //update the background model
        pBackSub->apply(frame, fgMask);
        //! [apply]

        //! [display_frame_number]
        //get the frame number and write it on the current frame
        rectangle(frame, cv::Point(10, 2), cv::Point(100,20),
                  cv::Scalar(255,255,255), -1);
        stringstream ss;
        ss << capture.get(CAP_PROP_POS_FRAMES);
        string frameNumberString = ss.str();
        putText(frame, frameNumberString.c_str(), cv::Point(15, 15),
                FONT_HERSHEY_SIMPLEX, 0.5 , cv::Scalar(0,0,0));
        //! [display_frame_number]

        //! [show]
        //show the current frame and the fg masks
        imshow("Frame", frame);
        imshow("FG Mask", fgMask);
        //! [show]

        // Perform OCR on the foreground mask
        ocr.SetImage(fgMask.data, fgMask.cols, fgMask.rows, 1, fgMask.step);
        string text = string(ocr.GetUTF8Text());

        // Print the recognized text to the terminal
        cout << "Placa: " << text << endl;

        //get the input from the keyboard
        int keyboard = waitKey(30);
        if (keyboard == 'q' || keyboard == 27)
            break;
    }

    // Clean up
    ocr.End();
    return 0;
}
