#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

// Janela para captura de vídeo e exibição
const String window_capture_name = "Video Capture";
const String window_detection_name = "Object Detection";

// Faixas de HSV para cores diferentes
int low_H_yellow = 20, low_S_yellow = 100, low_V_yellow = 100;
int high_H_yellow = 84, high_S_yellow = 255, high_V_yellow = 255;

int low_H_green = 35, low_S_green = 48, low_V_green = 33;
int high_H_green = 130, high_S_green = 197, high_V_green = 255;

int low_H_red1 = 0, low_S_red1 = 100, low_V_red1 = 100;
int high_H_red1 = 10, high_S_red1 = 255, high_V_red1 = 255;

int low_H_red2 = 170, low_S_red2 = 100, low_V_red2 = 100;
int high_H_red2 = 180, high_S_red2 = 255, high_V_red2 = 255;

int main(int argc, char* argv[])
{
    // Captura de vídeo (por padrão, usa a webcam)
    VideoCapture cap(argc > 1 ? atoi(argv[1]) : 0);
    if (!cap.isOpened()) {
        cout << "Erro ao abrir a câmera!" << endl;
        return -1;
    }

    // Criação de janelas
    namedWindow(window_capture_name);
    namedWindow(window_detection_name);

    Mat frame, frame_HSV, mask_yellow, mask_green, mask_red1, mask_red2, mask_red, mask_combined;

    while (true) {
        // Captura o frame
        cap >> frame;
        if (frame.empty()) {
            cout << "Fim do vídeo." << endl;
            break;
        }

        // Converte o frame de BGR para HSV
        cvtColor(frame, frame_HSV, COLOR_BGR2HSV);

        // Criação das máscaras para cada cor
        inRange(frame_HSV, Scalar(low_H_yellow, low_S_yellow, low_V_yellow), Scalar(high_H_yellow, high_S_yellow, high_V_yellow), mask_yellow);
        inRange(frame_HSV, Scalar(low_H_green, low_S_green, low_V_green), Scalar(high_H_green, high_S_green, high_V_green), mask_green);
        inRange(frame_HSV, Scalar(low_H_red1, low_S_red1, low_V_red1), Scalar(high_H_red1, high_S_red1, high_V_red1), mask_red1);
        inRange(frame_HSV, Scalar(low_H_red2, low_S_red2, low_V_red2), Scalar(high_H_red2, high_S_red2, high_V_red2), mask_red2);

        // Combina as duas faixas de vermelho
        mask_red = mask_red1 | mask_red2;

        // Combina todas as máscaras (amarelo, verde e vermelho)
        mask_combined = mask_yellow | mask_green | mask_red;

        // Mostrar os frames originais e a detecção combinada
        imshow(window_capture_name, frame);
        imshow(window_detection_name, mask_combined);

        // Pressione 'q' ou 'ESC' para sair
        char key = (char)waitKey(30);
        if (key == 'q' || key == 27) {
            break;
        }
    }
    return 0;
}
