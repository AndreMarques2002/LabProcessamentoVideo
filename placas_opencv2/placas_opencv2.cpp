#include <opencv2/opencv.hpp>
#include <tesseract/baseapi.h>
#include <iostream>
#include <chrono>
#include <ctime>
#include <sstream>
#include <filesystem>

using namespace cv;
using namespace std;
namespace fs = std::filesystem;

int main() {
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

    cout << "Pressione a barra de espaço para salvar a imagem com a placa detectada." << endl;
    cout << "Pressione 'q' para sair." << endl;

    while (true) {
        Mat frame;
        cap >> frame; // Captura um frame da webcam

        if (frame.empty()) {
            cerr << "Erro ao capturar o frame!" << endl;
            break;
        }

        // Mostrar o feed da câmera ao vivo
        imshow("Webcam", frame);

        // Verificar se o usuário pressionou uma tecla
        char key = (char)waitKey(30); // Aguarda 30ms por entrada do teclado
        if (key == 'q' || key == 'Q') {
            break; // Sair do loop se 'q' for pressionado
        } else if (key == ' ') { // Se a barra de espaço for pressionada
            // Converter para escala de cinza
            Mat gray;
            cvtColor(frame, gray, COLOR_BGR2GRAY);

            // Aplicar filtro de suavização para reduzir ruído
            Mat blurred;
            GaussianBlur(gray, blurred, Size(5, 5), 0);

            // Aplicar limiarização (valor fixo)
            Mat thresh;
            threshold(blurred, thresh, 0, 255, THRESH_BINARY_INV + THRESH_OTSU);

            // Configurar o Tesseract para usar a imagem limiarizada
            ocr.SetImage(thresh.data, thresh.cols, thresh.rows, 1, thresh.step);

            // Realizar o OCR e obter o texto
            string placaTexto = string(ocr.GetUTF8Text());

            // Limpar a string do texto da placa
            placaTexto = placaTexto.erase(placaTexto.find_last_not_of(" ?\n\r\t") + 1);

            if (!placaTexto.empty()) {
                cout << "Texto detectado: " << placaTexto << endl;

                // Obter a data e hora atual
                auto now = chrono::system_clock::now();
                time_t now_c = chrono::system_clock::to_time_t(now);
                stringstream datetime;
                datetime << put_time(localtime(&now_c), "%H-%M-%S_%d-%m-%Y");

                // Melhorar a qualidade da imagem
                Mat equalized;
                equalizeHist(gray, equalized);

                Mat highResImage;
                Size newSize(frame.cols * 2, frame.rows * 2); // Aumentar a imagem em 2x
                resize(equalized, highResImage, newSize, 0, 0, INTER_CUBIC);

                Mat sharpened;
                Mat kernel = (Mat_<float>(3, 3) << 0, -1, 0, -1, 5, -1, 0, -1, 0);
                filter2D(highResImage, sharpened, -1, kernel);

                // Adicionar a data/hora na imagem
                stringstream datetimeImage;
                datetimeImage << put_time(localtime(&now_c), "%H:%M:%S %d/%m/%Y");
                putText(sharpened, datetimeImage.str(), Point(10, 50), FONT_HERSHEY_SIMPLEX, 1.0, Scalar(255, 255, 255), 2);

                // Salvar a imagem na pasta "placas_salvas"
                string filename = pasta + "/placa_" + datetime.str() + ".jpg";
                int counter = 1;
                while (fs::exists(filename)) {
                    filename = pasta + "/placa_" + datetime.str() + "_" + to_string(counter++) + ".jpg";
                }

                if (!imwrite(filename, sharpened)) {
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
