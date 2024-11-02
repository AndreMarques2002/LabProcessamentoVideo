#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include <iostream>

using namespace cv;
using namespace std;

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

int main(int argc, char** argv) {
    VideoCapture cap(0); // Abre a webcam padrão
    if (!cap.isOpened()) {
        cout << "Erro ao abrir a webcam!" << endl;
        return -1;
    }

    Mat frame, gray, equalized;
    Mat hist, equalizedHist;
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

        calcHist(&gray, 1, 0, Mat(), hist, 1, &histSize, histRange, true, false);
        calcHist(&equalized, 1, 0, Mat(), equalizedHist, 1, &histSize, histRange, true, false);

        imshow("Webcam - Gray", gray);
        imshow("Webcam - Equalized", equalized);

        char c = (char)waitKey(30);
        if (c == 's') {
            imwrite("webcam_gray.jpg", gray);
            imwrite("webcam_equalized.jpg", equalized);
            saveHistogramImage(hist, "webcam_histogram_before.jpg");
            saveHistogramImage(equalizedHist, "webcam_histogram_after.jpg");
        } else if (c == 'q') {
            break;
        }
    }

    cap.release();
    destroyAllWindows();

    return 0;
}
