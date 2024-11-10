#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include <iostream>

using namespace cv;
using namespace std;

int threshold_value = 0;
int threshold_type = 3;
int const max_value = 255;
int const max_type = 4;
int const max_binary_value = 255; /* variáveis e constantes da limiarização */

const char* window_name = "Threshold Demo";
 
const char* trackbar_type = "Type: \n 0: Binary \n 1: Binary Inverted \n 2: Truncate \n 3: To Zero \n 4: To Zero Inverted";
const char* trackbar_value = "Value";


Mat frame, gray, equalized;
Mat hist, equalizedHist;
Mat dst;
    
void saveHistogramImage(const Mat& hist, const string& filename) {
    int hist_w = 512, hist_h = 400;
    int bin_w = cvRound((double) hist_w / hist.rows);
    Mat histImage(hist_h, hist_w, CV_8UC3, Scalar(0, 0, 0));

    normalize(hist, hist, 0, histImage.rows, NORM_MINMAX, -1, Mat());

    for (int i = 1; i < hist.rows; i++) {
        line(histImage, Point(bin_w * (i - 1), hist_h - cvRound(hist.at<float>(i - 1))),
             Point(bin_w * i, hist_h - cvRound(hist.at<float>(i))),
             Scalar(255, 255, 255), 2, 8, 0);
    }

    imwrite(filename, histImage);
}

static void Threshold_Demo( int, void* )
{
    /* 0: Binary
     1: Binary Inverted
     2: Threshold Truncated
     3: Threshold to Zero
     4: Threshold to Zero Inverted
    */
    threshold( equalized, dst, threshold_value, max_binary_value, threshold_type );
}

int main(int argc, char** argv) 
{
    VideoCapture cap(0); // Abre a webcam padrão
    if (!cap.isOpened()) {
        cout << "Erro ao abrir a webcam!" << endl;
        return -1;
    }
    
    namedWindow( window_name, WINDOW_AUTOSIZE ); // Cria janela para exibição
    
    createTrackbar( trackbar_type,
                    window_name, &threshold_type,
                    max_type, Threshold_Demo ); // Create a Trackbar to choose type of Threshold
 
    createTrackbar( trackbar_value,
                    window_name, &threshold_value,
                    max_value, Threshold_Demo ); // Create a Trackbar to choose Threshold value
   
    int histSize = 256;
    float range[] = {0, 256};
    const float* histRange[] = {range};

    while (true) {
        cap >> frame; // Captura um frame da webcam
        if (frame.empty()) {
            cout << "Erro ao capturar frame!" << endl;
            break;
        }

        cvtColor(frame, gray, COLOR_BGR2GRAY);
        equalizeHist(gray, equalized);

		Threshold_Demo( 0, 0 ); /* chama função threshold*/

        calcHist(&gray, 1, 0, Mat(), hist, 1, &histSize, histRange, true, false);
        calcHist(&equalized, 1, 0, Mat(), equalizedHist, 1, &histSize, histRange, true, false);


        imshow("Webcam - Gray", gray);
        imshow("Webcam - Equalized", equalized);
		imshow( window_name, dst );

        char c = (char)waitKey(30);
        if (c == 's') {
			imwrite("webcam_binary_threshold.jpg", dst);
            imwrite("webcam_gray.jpg", gray);
            imwrite("webcam_equalized.jpg", equalized);
            saveHistogramImage(hist, "webcam_histogram_before.jpg");
            saveHistogramImage(equalizedHist, "webcam_histogram_after.jpg");
        } 
        else if (c == 'q') 
        {
            break;
        }
    }

    cap.release();
    destroyAllWindows();

    return 0;
}
