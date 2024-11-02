#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
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
    CommandLineParser parser(argc, argv, "{@input | Gabriel.jpg | input image}");
    Mat src = imread(samples::findFile(parser.get<String>("@input")), IMREAD_COLOR);
    if (src.empty()) {
        cout << "Could not open or find the image!\n" << endl;
        return -1;
    }

    Mat gray;
    cvtColor(src, gray, COLOR_BGR2GRAY);

    Mat hist;
    int histSize = 256;
    float range[] = {0, 256};
    const float* histRange[] = {range};
    calcHist(&gray, 1, 0, Mat(), hist, 1, &histSize, histRange, true, false);

    Mat equalized;
    equalizeHist(gray, equalized);

    Mat equalizedHist;
    calcHist(&equalized, 1, 0, Mat(), equalizedHist, 1, &histSize, histRange, true, false);

    imshow("Source image", src);
    imshow("Gray image", gray);
    imshow("Equalized Image", equalized);
//    imshow("Histograma", hist);
//    imshow("Histograma Eq", equalizedHist);

    while (true) {
        char c = (char)waitKey();
        if (c == 's') {
            imwrite("gray_image.jpg", gray);
            imwrite("equalized_image.jpg", equalized);
            saveHistogramImage(hist, "histogram_before.jpg");
            saveHistogramImage(equalizedHist, "histogram_after.jpg");
        } else if (c == 'q') {
            break;
        }
    }

    return 0;
}
