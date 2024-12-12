/* Grupo VGA - Sistema de Reconhecimento de Placas Veiculares
André Marques da Silva RA: 11202021067
Gabriel Batista Veloso RA: 11201921267
Vinícius de Souza Feitosa RA: 11202021889
*/

#include <opencv2/opencv.hpp>
#include <tesseract/baseapi.h>
#include <iostream>
#include <chrono>
#include <ctime>
#include <sstream>
#include <filesystem>
#include <regex>
#include <map>

using namespace cv;
using namespace std;
namespace fs = std::filesystem;

// Função para aplicar processamento prévio à imagem
Mat preprocessImage(const Mat &img) {
    Mat gray, resized, dilated, eroded;

    // Redimensiona para aumentar a resolução
    resize(img, resized, Size(), 1.2, 1.2, INTER_CUBIC);

    // Converte para escala de cinza
    cvtColor(resized, gray, COLOR_BGR2GRAY);

    // Dila e erode para destacar caracteres
    Mat kernel = getStructuringElement(MORPH_RECT, Size(1, 1));
    dilate(gray, dilated, kernel, Point(-1, -1), 1);
    erode(dilated, eroded, kernel, Point(-1, -1), 1);

    return eroded;
}

// Função para encontrar regiões candidatas
vector<Rect> detectRegions(const Mat &img) {
    Mat edges;
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;

    // Detecção de bordas
    Canny(img, edges, 50, 200);

    // Encontrar contornos
    findContours(edges, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);

    vector<Rect> candidateRegions;
    for (const auto &contour : contours) {
        Rect boundingBox = boundingRect(contour);

        // Filtrar regiões com base em proporção e tamanho
        double aspectRatio = (double)boundingBox.width / boundingBox.height;
        if (aspectRatio > 2.0 && aspectRatio < 6.0 && boundingBox.area() > 1000) {
            candidateRegions.push_back(boundingBox);
        }
    }
    return candidateRegions;
}

// Função para realizar OCR em uma região
string performOCR(const Mat &roi, tesseract::TessBaseAPI &ocrEngine) {
    Mat thresh;

    // Aplicar suavização e limiarização
    threshold(roi, thresh, 0, 255, THRESH_BINARY + THRESH_OTSU);

    // Realizar OCR
    ocrEngine.SetImage(thresh.data, thresh.cols, thresh.rows, 1, thresh.step);
    ocrEngine.SetSourceResolution(70);
    string text = string(ocrEngine.GetUTF8Text());
	
    // Limpar o texto detectado
    text = regex_replace(text, regex("[^A-Za-z0-9]"), "");
    transform(text.begin(), text.end(), text.begin(), ::toupper);

    return text;
}

int main() {
    // Banco de dados de placas cadastradas
    map<string, pair<string, string>> bancoDeDados = {
        {"BRA2E19", {"Andre", "Ford Fiesta"}},
        {"BRAZE19", {"Andre", "Ford Fiesta"}},
        {"ABC1D34", {"Gabriel", "Chevrolet Onix"}},
        {"PBA2019", {"Vinicius", "Honda Fit"}},
        {"PBAZO19", {"Vinicius", "Honda Fit"}},
    };

    // Verificar e criar a pasta "placas_salvas"
    string pasta = "placas_salvas";
    if (!fs::exists(pasta)) {
        fs::create_directory(pasta);
        cout << "Pasta 'placas_salvas' criada." << endl;
    }

    // Abrir o video
    VideoCapture cap("video_placas_final.mp4");
    if (!cap.isOpened()) {
        cerr << "Erro ao abrir a webcam!" << endl;
        return -1;
    }

    // Inicializar o OCR (Tesseract)
    tesseract::TessBaseAPI ocr;
    ocr.Init(NULL, "eng");

    cout << "Pressione a barra de espaço para verificar a placa detectada." << endl;
    cout << "Pressione 'q' para sair." << endl;

    while (true) {
        Mat frame;
        cap >> frame; // Captura um frame da webcam

        if (frame.empty()) {
            cerr << "Erro ao capturar o frame!" << endl;
            break;
        }

        // Mostrar o feed da câmera ao vivo
        imshow("Sistema de reconhecimento de placas veiculares", frame);

        char key = (char)waitKey(30);
        if (key == 'q' || key == 'Q') {
            break;
        } else if (key == ' '){ // Pressionou espaço para capturar
            // Pré-processamento da imagem
            Mat preprocessed = preprocessImage(frame);

            // Detectar regiões candidatas
            vector<Rect> regions = detectRegions(preprocessed);

            for (const auto &region : regions) {
                Mat roi = preprocessed(region); // Recortar a região da imagem

                // Realizar OCR
                string placaTexto = performOCR(roi, ocr);

                // Validar texto detectado
                if (placaTexto.size() == 7) { // Apenas placas com 7 caracteres
                    cout << "Texto detectado: " << placaTexto << endl;

                    // Verificar no banco de dados
                    if (bancoDeDados.find(placaTexto) != bancoDeDados.end()) {
                        cout << "Acesso permitido!" << endl;
                        cout << "Motorista: " << bancoDeDados[placaTexto].first << endl;
                        cout << "Modelo do carro: " << bancoDeDados[placaTexto].second << endl;

                        Mat imagemAltaRes;
                        resize(preprocessed, imagemAltaRes, Size(), 5.0, 5.0, INTER_CUBIC); // Aumenta a resolução com interpolação cúbica

                        // Salvar imagem processada da ROI
                        auto now = chrono::system_clock::now();
                        time_t now_c = chrono::system_clock::to_time_t(now);
                        stringstream datetime;
                        datetime << put_time(localtime(&now_c), "%H-%M-%S_%d-%m-%Y");

                        string filename = pasta + "/placa_" + datetime.str() + ".jpg";
                        imwrite(filename, imagemAltaRes); // Salvar a imagem melhorada
                        cout << "Imagem salva como: " << filename << endl;
                    } else {
                        cout << "Acesso negado! Placa não encontrada no sistema." << endl;
                    }
                }
            }
        }
    }

    // Finalizar OCR e liberar recursos
    ocr.End();
    cap.release();
    destroyAllWindows();

    cout << "Programa encerrado." << endl;
    return 0;
}
