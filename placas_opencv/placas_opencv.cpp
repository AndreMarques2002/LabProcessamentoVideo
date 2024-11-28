/*
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
    // Banco de dados de placas cadastradas
    map<string, pair<string, string>> bancoDeDados = {
        {"BRA2E19", {"Andre", "Ford Fiesta"}},
        {"ABC1D34", {"Gabriel", "Chevrolet Onix"}},
        {"PBA2019", {"Vinicius", "Honda Fit"}},
        {"PBA2O19", {"Vinicius", "Honda Fit"}},
        {"RIO2A18", {"Pedro", "Volkswagen Jetta"}},
        {"RI02A18", {"Pedro", "Volkswagen Jetta"}}
    };

    // Verificar e criar a pasta "placas_salvas"
    string pasta = "placas_salvas";
    if (!fs::exists(pasta)) {
        fs::create_directory(pasta);
        cout << "Pasta 'placas_salvas' criada." << endl;
    }

    // Abrir a câmera (webcam)
    VideoCapture cap(0);
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
        } else if (key == ' ') { // Pressionou espaço para capturar
            // Pré-processamento
            Mat gray, blurred, edges, thresh;
            cvtColor(frame, gray, COLOR_BGR2GRAY); // Converter para escala de cinza
            GaussianBlur(gray, blurred, Size(5, 5), 0); // Suavizar para reduzir ruído
            Canny(blurred, edges, 50, 200); // Detectar bordas com Canny

            // Detectar contornos
            vector<vector<Point>> contornos;
            vector<Vec4i> hierarquia;
            findContours(edges, contornos, hierarquia, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

            // Processar cada contorno
            for (size_t i = 0; i < contornos.size(); i++) {
                Rect boundingBox = boundingRect(contornos[i]);

                // Filtrar regiões por tamanho e proporção
                double aspectRatio = (double)boundingBox.width / boundingBox.height;
                if (aspectRatio > 2.0 && aspectRatio < 6.0 && boundingBox.area() > 1000) {
                    Mat roi = gray(boundingBox); // Recortar a região da imagem

                    // Suavizar e limiarizar a ROI
                    threshold(roi, thresh, 0, 255, THRESH_BINARY + THRESH_OTSU);

                    // OCR na região identificada
                    ocr.SetImage(thresh.data, thresh.cols, thresh.rows, 1, thresh.step);
                    string placaTexto = string(ocr.GetUTF8Text());

                    // Limpar o texto detectado
                    placaTexto = regex_replace(placaTexto, regex("[^A-Za-z0-9]"), "");
                    transform(placaTexto.begin(), placaTexto.end(), placaTexto.begin(), ::toupper);

                    // Ignorar palavras irrelevantes (ex.: "BRASIL")
                    placaTexto = regex_replace(placaTexto, regex("BRASIL"), "");

                    if (!placaTexto.empty()) {
                        cout << "Texto detectado: " << placaTexto << endl;

                        // Verificar no banco de dados
                        if (bancoDeDados.find(placaTexto) != bancoDeDados.end()) {
                            cout << "Acesso permitido!" << endl;
                            cout << "Motorista: " << bancoDeDados[placaTexto].first << endl;
                            cout << "Modelo do carro: " << bancoDeDados[placaTexto].second << endl;

                            // Salvar imagem da ROI apenas se a placa estiver no banco de dados
                            auto now = chrono::system_clock::now();
                            time_t now_c = chrono::system_clock::to_time_t(now);
                            stringstream datetime;
                            datetime << put_time(localtime(&now_c), "%H-%M-%S_%d-%m-%Y");

                            string filename = pasta + "/placa_" + datetime.str() + ".jpg";
                            imwrite(filename, thresh);
                            cout << "Imagem salva como: " << filename << endl;
                        } else {
                            cout << "Acesso negado! Placa não encontrada no sistema." << endl;
                        }
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
Retire a palavra BR também. Além disso, TODAS as placas possuem exatamente 7 caracteres (qtd de números + qtd de letras). Não deve se ler nem aparecer no terminal leituras com qtd de caracteres diferentes de 7. Adicione comentários nas funções aplicadas no código.

Com base na imagem fornecida, o que mais posso fazer para melhorar a acurácia da identificação da placa?


*/
/*
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
    // Banco de dados de placas cadastradas
    map<string, pair<string, string>> bancoDeDados = {
        {"RIO2A18", {"Pedro", "Volkswagen Jetta"}},
        {"RI02A18", {"Pedro", "Volkswagen Jetta"}},
        {"ABC1D34", {"Gabriel", "Chevrolet Onix"}},
        {"PBA2019", {"Vinicius", "Honda Fit"}},
        {"PBA2O19", {"Vinicius", "Honda Fit"}},
        {"BRA2E19", {"Andre", "Ford Fiesta"}}
    };
    
    // Verificar e criar a pasta "placas_salvas" para armazenar capturas de imagens
    string pasta = "placas_salvas";
    if (!fs::exists(pasta)) {
        fs::create_directory(pasta);
        cout << "Pasta 'placas_salvas' criada." << endl;
    }

    // Abrir a câmera (webcam)
    VideoCapture cap(0);
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
        } else if (key == ' ') { // Pressionou espaço para capturar
            // Pré-processamento
            Mat gray, blurred, edges, thresh;
            cvtColor(frame, gray, COLOR_BGR2GRAY); // Converter para escala de cinza
            GaussianBlur(gray, blurred, Size(5, 5), 0); // Suavizar para reduzir ruído
            Canny(blurred, edges, 50, 200); // Detectar bordas com Canny

            // Detectar contornos
            vector<vector<Point>> contornos;
            vector<Vec4i> hierarquia;
            findContours(edges, contornos, hierarquia, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

            // Processar cada contorno
            for (size_t i = 0; i < contornos.size(); i++) {
                // Determinar o retângulo delimitador (bounding box)
                Rect boundingBox = boundingRect(contornos[i]);

                // Filtrar regiões por tamanho e proporção (ajuda a identificar retângulos semelhantes às placas)
                double aspectRatio = (double)boundingBox.width / boundingBox.height;
                if (aspectRatio > 2.0 && aspectRatio < 6.0 && boundingBox.area() > 1000) {
                    Mat roi = gray(boundingBox); // Recortar a região da imagem

                    // Aplicar upscaling na ROI para melhorar o OCR
                    resize(roi, roi, Size(), 0.8, 0.8, INTER_CUBIC);

                    // Suavizar e limiarizar a ROI (região de interesse)
                    threshold(roi, thresh, 0, 255, THRESH_BINARY + THRESH_OTSU);

                    // OCR na região identificada
                    ocr.SetImage(thresh.data, thresh.cols, thresh.rows, 1, thresh.step);
                    string placaTexto = string(ocr.GetUTF8Text());

                    // Limpar o texto detectado: remover caracteres inválidos
                    placaTexto = regex_replace(placaTexto, regex("[^A-Za-z0-9]"), "");
                    transform(placaTexto.begin(), placaTexto.end(), placaTexto.begin(), ::toupper);

                    // Verificar se a leitura é válida e no formato esperado
                    if (placaTexto.size() == 7 && placaTexto.find("BR") == string::npos) {
                        cout << "Placa detectada: " << placaTexto << endl;

                        // Verificar no banco de dados
                        if (bancoDeDados.find(placaTexto) != bancoDeDados.end()) {
                            cout << "Acesso permitido!" << endl;
                            cout << "Motorista: " << bancoDeDados[placaTexto].first << endl;
                            cout << "Modelo do carro: " << bancoDeDados[placaTexto].second << endl;

                            // Salvar imagem da ROI apenas se a placa estiver no banco de dados
                            auto now = chrono::system_clock::now();
                            time_t now_c = chrono::system_clock::to_time_t(now);
                            stringstream datetime;
                            datetime << put_time(localtime(&now_c), "%H-%M-%S_%d-%m-%Y");

                            string filename = pasta + "/placa_" + datetime.str() + ".jpg";
                            imwrite(filename, thresh);
                            cout << "Imagem salva como: " << filename << endl;
                        } else {
                            cout << "Acesso negado! Placa não encontrada no sistema." << endl;
                        }
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
    // Banco de dados de placas cadastradas
    map<string, pair<string, string>> bancoDeDados = {
        {"RIO2A18", {"Andre", "Ford Fiesta"}},
        {"RI02A18", {"Andre", "Ford Fiesta"}},
        {"ABC1D34", {"Gabriel", "Chevrolet Onix"}},
        {"PBA2019", {"Vinicius", "Honda Fit"}},
        {"PBA2O19", {"Vinicius", "Honda Fit"}}
    };

    // Verificar e criar a pasta "placas_salvas" para armazenar capturas de imagens
    string pasta = "placas_salvas";
    if (!fs::exists(pasta)) {
        fs::create_directory(pasta);
        cout << "Pasta 'placas_salvas' criada." << endl;
    }

    // Abrir a câmera (webcam)
    VideoCapture cap(0);
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
        } else if (key == ' ') { // Pressionou espaço para capturar
            // Pré-processamento
            Mat gray, processed, roi; // Declarar `roi` fora do loop
            cvtColor(frame, gray, COLOR_BGR2GRAY); // Converter para escala de cinza
            
            // Passo 1: Redimensionamento para melhorar a qualidade de leitura
            resize(gray, gray, Size(), 1.2, 1.2, INTER_CUBIC);

            // Passo 2: Aplicar dilatação e erosão para destacar as características principais
            Mat kernel = Mat::ones(1, 1, CV_8U); // Kernel de 1x1
            dilate(gray, processed, kernel, Point(-1, -1), 1); // Dilatação
            erode(processed, processed, kernel, Point(-1, -1), 1); // Erosão

            // Passo 3: Aplicar suavização e limiarização
            GaussianBlur(processed, processed, Size(5, 5), 0);
            threshold(processed, processed, 0, 255, THRESH_BINARY + THRESH_OTSU);

            // Passo 4: Detectar contornos para identificar regiões candidatas a placas
            vector<vector<Point>> contornos;
            findContours(processed, contornos, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

            vector<Rect> candidatos; // Armazenar retângulos candidatos
            for (const auto& contorno : contornos) {
                Rect retangulo = boundingRect(contorno);
                float proporcao = (float)retangulo.width / retangulo.height;

                // Filtrar retângulos com base no tamanho e na proporção (ex.: 4:1 típico de placas)
                if (retangulo.width > 100 && retangulo.height > 25 && proporcao > 2 && proporcao < 5) {
                    candidatos.push_back(retangulo);
                }
            }

            // Passo 5: Iterar sobre as regiões candidatas e aplicar OCR
            string placaDetectada;
            for (const auto& retangulo : candidatos) {
                roi = gray(retangulo); // Extrair região de interesse
q
                // Aplicar processamento à ROI antes de usar OCR e salvar
                Mat roiProcessada;
                resize(roi, roi, Size(), 1.2, 1.2, INTER_CUBIC); // Melhorar detalhes
                dilate(roi, roiProcessada, kernel, Point(-1, -1), 1); // Dilatação
                erode(roiProcessada, roiProcessada, kernel, Point(-1, -1), 1); // Erosão
                GaussianBlur(roiProcessada, roiProcessada, Size(5, 5), 0);
                threshold(roiProcessada, roiProcessada, 0, 255, THRESH_BINARY + THRESH_OTSU);

                // Aplicar OCR na região processada
                ocr.SetImage(roiProcessada.data, roiProcessada.cols, roiProcessada.rows, 1, roiProcessada.step);
                string texto = string(ocr.GetUTF8Text());

                // Limpar o texto detectado
                texto = regex_replace(texto, regex("[^A-Za-z0-9]"), "");
                transform(texto.begin(), texto.end(), texto.begin(), ::toupper);

                // Validar comprimento e evitar leituras inválidas
                if (texto.size() == 7) {
                    placaDetectada = texto;

                    // Salvar imagem processada da ROI
                    auto now = chrono::system_clock::now();
                    time_t now_c = chrono::system_clock::to_time_t(now);
                    stringstream datetime;
                    datetime << put_time(localtime(&now_c), "%H-%M-%S_%d-%m-%Y");

                    string filename = pasta + "/placa_" + datetime.str() + ".jpg";
                    imwrite(filename, roiProcessada); // Salvar imagem processada
                    cout << "Imagem salva como: " << filename << endl;
                    break; // Usar a primeira leitura válida encontrada
                }
            }

            // Verificar se uma placa válida foi detectada
            if (!placaDetectada.empty()) {
                cout << "Placa detectada: " << placaDetectada << endl;

                // Verificar no banco de dados
                if (bancoDeDados.find(placaDetectada) != bancoDeDados.end()) {
                    cout << "Acesso permitido!" << endl;
                    cout << "Motorista: " << bancoDeDados[placaDetectada].first << endl;
                    cout << "Modelo do carro: " << bancoDeDados[placaDetectada].second << endl;
                } else {
                    cout << "Acesso negado! Placa não encontrada no sistema." << endl;
                }
            } else {
                cout << "Nenhuma placa válida detectada." << endl;
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

