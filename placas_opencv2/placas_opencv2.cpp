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

int main() {
    // Banco de dados de placas cadastradas (simulado com um map)
    map<string, pair<string, string>> bancoDeDados = {
        {"BRA2E19", {"Andre", "Ford Fiesta"}},
    };

    // Verificar e criar a pasta "placas_salvas" se não existir
    string pasta = "placas_salvas";
    if (!fs::exists(pasta)) {
        fs::create_directory(pasta);
        cout << "Pasta 'placas_salvas' criada." << endl;
    }

    // Abrir a câmera (webcam)
    VideoCapture cap(0); // Usar a câmera padrão
    if (!cap.isOpened()) {
        cerr << "Erro ao abrir a webcam!" << endl;
        return -1;
    }

    // Inicializar o OCR (Tesseract)
    tesseract::TessBaseAPI ocr;
    ocr.Init(NULL, "eng"); // Inicializa o OCR com o idioma inglês

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
            break; // Sair do loop se 'q' for pressionado
        } else if (key == ' ') { // Se a barra de espaço for pressionada
            // Pré-processamento da imagem antes de fazer o OCR
            Mat gray, processed;

            // Converter para escala de cinza
            cvtColor(frame, gray, COLOR_BGR2GRAY);

            // Aumentar a escala da imagem (DPI inferior a 300 dpi):
            resize(gray, processed, Size(), 1.2, 1.2, INTER_CUBIC);

            // Aplicar operações morfológicas para reduzir ruídos (dilation e erosian)
            Mat kernel = getStructuringElement(MORPH_RECT, Size(1, 1));
            dilate(processed, processed, kernel, Point(-1, -1), 1);
            erode(processed, processed, kernel, Point(-1, -1), 1);

            // Aplicar suavização e limiarização
            GaussianBlur(processed, processed, Size(5, 5), 0);
            threshold(processed, processed, 0, 255, THRESH_BINARY + THRESH_OTSU);

            // Configurar o Tesseract para usar a imagem processada
            ocr.SetImage(processed.data, processed.cols, processed.rows, 1, processed.step);

            // Realizar o OCR e obter o texto
            string placaTexto = string(ocr.GetUTF8Text());

            // Limpar a string do texto da placa
            placaTexto = regex_replace(placaTexto, regex("[^A-Za-z0-9]"), ""); // Remove caracteres especiais e hifens
            transform(placaTexto.begin(), placaTexto.end(), placaTexto.begin(), ::toupper); // Converte para maiúsculas

            if (!placaTexto.empty()) {
                cout << "Texto detectado (normalizado): " << placaTexto << endl;

                // Verificar se a placa está no banco de dados
                if (bancoDeDados.find(placaTexto) != bancoDeDados.end()) {
                    cout << "Acesso permitido!" << endl;
                    cout << "Motorista: " << bancoDeDados[placaTexto].first << endl;
                    cout << "Modelo do carro: " << bancoDeDados[placaTexto].second << endl;
                } else {
                    cout << "Acesso negado! Placa não encontrada no sistema." << endl;
                }

                // Salvar a imagem da placa (opcional)
                auto now = chrono::system_clock::now();
                time_t now_c = chrono::system_clock::to_time_t(now);
                stringstream datetime;
                datetime << put_time(localtime(&now_c), "%H-%M-%S_%d-%m-%Y");
                
                // Adicionar a data/hora na imagem
                stringstream datetimeImage;
                datetimeImage << put_time(localtime(&now_c), "%H:%M:%S %d/%m/%Y");
                putText(processed, datetimeImage.str(), Point(10, 50), FONT_HERSHEY_SIMPLEX, 1.0, Scalar(255, 255, 255), 2);
                

                string filename = pasta + "/placa_" + datetime.str() + ".jpg";
                if (!imwrite(filename, processed)) {
                    cerr << "Erro ao salvar a imagem!" << endl;
                } else {
                    cout << "Imagem salva como: " << filename << endl;
                }
            } else {
                cout << "Nenhuma placa detectada." << endl;
            }
        }
    }

    // Finalizar o OCR e liberar recursos
    ocr.End();
    cap.release();
    destroyAllWindows();

    cout << "Programa encerrado." << endl;

    return 0;
}
