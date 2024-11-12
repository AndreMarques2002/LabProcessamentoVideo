#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include <iostream>

using namespace cv;
using namespace std;

/* Variáveis e Constantes do programa original do site OpenCv)*/
int histSize = 256;
float range[] = {0, 256};
const float* histRange[] = {range}; 
    
/* função feita pelo André para construir e salvar o histograma dos 3 canais (RGB) */
void saveHistogramImage(const Mat& b_hist, const Mat& g_hist, const Mat& r_hist, const string& filename) 
{
    int hist_w = 512, hist_h = 400;
    int bin_w = cvRound((double) hist_w/histSize);
    Mat histImage(hist_h, hist_w, CV_8UC3, Scalar(0, 0, 0));

    normalize(b_hist, b_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );
    normalize(g_hist, g_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );
    normalize(r_hist, r_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );
    
    for (int i = 1; i < histSize; i++) 
    {
        line( histImage, Point( bin_w*(i-1), hist_h - cvRound(b_hist.at<float>(i - 1))),
             Point(bin_w * i, hist_h - cvRound(b_hist.at<float>(i))),
             Scalar(255, 255, 255), 2, 8, 0);
        line( histImage, Point( bin_w*(i-1), hist_h - cvRound(g_hist.at<float>(i-1)) ),
              Point( bin_w*(i), hist_h - cvRound(g_hist.at<float>(i)) ),
              Scalar( 0, 255, 0), 2, 8, 0  );
        line( histImage, Point( bin_w*(i-1), hist_h - cvRound(r_hist.at<float>(i-1)) ),
              Point( bin_w*(i), hist_h - cvRound(r_hist.at<float>(i)) ),
              Scalar( 0, 0, 255), 2, 8, 0  );
    }

    imwrite(filename, histImage);
}

int main(int argc, char** argv) 
{
    VideoCapture cap(0); // Abre a webcam padrão
    if (!cap.isOpened()) {
        cout << "Erro ao abrir a webcam!" << endl;
        return -1;
    }

    Mat frame, b_equalized, g_equalized, r_equalized;
    Mat channels_equalized;

    while (true) 
    {
        cap >> frame; // Captura um frame da webcam
        if (frame.empty()) 
        {
            cout << "Erro ao capturar frame!" << endl;
            break;
        }
        	
		vector<Mat> bgr_planes; // Cria um vetor do tipo Mat
		split( frame, bgr_planes ); // Divide a imagem em canais BGR no vetor declarado acima
		
		equalizeHist(bgr_planes[0], b_equalized); // equalizando os histogramas de cada canal individualmente
		equalizeHist(bgr_planes[1], g_equalized);
		equalizeHist(bgr_planes[2], r_equalized);
 
		bool uniform = true, accumulate = false;
 
		Mat b_Eq_hist, g_Eq_hist, r_Eq_hist; // b_Eq_hist = blue equalized histogram
		
		// Calculo dos histogramas equalizados dos 3 canais. 
		calcHist( &b_equalized, 1, 0, Mat(), b_Eq_hist, 1, &histSize, histRange, uniform, accumulate );
		calcHist( &g_equalized, 1, 0, Mat(), g_Eq_hist, 1, &histSize, histRange, uniform, accumulate );
		calcHist( &r_equalized, 1, 0, Mat(), r_Eq_hist, 1, &histSize, histRange, uniform, accumulate );

		Mat b_hist, g_hist, r_hist;
		
		// Calculo dos histogramas não equalizados dos 3 canais.
		calcHist( &bgr_planes[0], 1, 0, Mat(), b_hist, 1, &histSize, histRange, uniform, accumulate );
		calcHist( &bgr_planes[1], 1, 0, Mat(), g_hist, 1, &histSize, histRange, uniform, accumulate );
		calcHist( &bgr_planes[2], 1, 0, Mat(), r_hist, 1, &histSize, histRange, uniform, accumulate );

		vector<Mat> bgr_equalized_planes; 
		bgr_equalized_planes = {b_equalized, g_equalized, r_equalized}; //armazena os canais equalizados dentro do novo vetor
		merge(bgr_equalized_planes, channels_equalized); //une os 3 canais equalizados novamente para formar a imagem colorida
				
        imshow("Webcam - Equalized", channels_equalized);
		imshow("Webcam - Normal", frame);

        char c = (char)waitKey(30);
        if (c == 's') 
        {
            imwrite("webcam_equalized.jpg", channels_equalized);
            imwrite("webcam_not_equalized.jpg", frame);
            saveHistogramImage(b_Eq_hist, g_Eq_hist, r_Eq_hist, "webcam_with_histogram_equalization.jpg");
            saveHistogramImage(b_hist, g_hist, r_hist, "webcam_without_histogram_equalization.jpg");
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
