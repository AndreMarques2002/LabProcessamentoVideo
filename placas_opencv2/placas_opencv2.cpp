#include <opencv2/opencv.hpp>
#include <tesseract/baseapi.h>
#include <iostream>
#include <chrono>
#include <ctime>
#include <sstream>
#include <filesystem>

using namespace cv;
using namespace std;
namespace fs = std::filesystem; // Usado para trabalhar com arquivos e diretórios

int main() {
    // Verificar e criar a pasta "placas_salvas" se não existir
    string pasta = "placas_salvas";
    if (!fs::exists(pasta)) {
        fs::create_directory(pasta); // Cria a pasta se ela não existir
        cout << "Pasta 'placas_salvas' criada." << endl;
    }

    // Abrir a câmera (webcam)
    VideoCapture cap(0); // Usar a câmera padrão (0 é o índice da webcam principal)
    if (!cap.isOpened()) {
        cerr << "Erro ao abrir a webcam!" << endl;
        return -1;
    }

    // Inicializar o OCR (Tesseract)
    tesseract::TessBaseAPI ocr;
    ocr.Init(NULL, "eng");  // Inicializa o OCR com o idioma inglês

    string resposta;

    while (true) {
        Mat frame;
        cap >> frame; // Captura um frame da webcam

        if (frame.empty()) {
            cerr << "Erro ao capturar o frame!" << endl;
            break;
        }

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

        // Limpar a string do texto da placa (remover caracteres indesejados, como "?")
        placaTexto = placaTexto.erase(placaTexto.find_last_not_of(" ?\n\r\t") + 1);

        // Se o texto da placa foi detectado
        if (!placaTexto.empty()) {
            cout << "Texto detectado: " << placaTexto << endl;

            // Obter a data e hora atual
            auto now = chrono::system_clock::now();
            time_t now_c = chrono::system_clock::to_time_t(now);
            stringstream datetime;
            datetime << put_time(localtime(&now_c), "%H-%M-%S_%d-%m-%Y"); // Formato: hh-mm-ss_dd-mm-yyyy

            // Melhorar a qualidade da imagem
            // 1. Equalizar o histograma para melhorar o contraste
            Mat equalized;
            equalizeHist(gray, equalized);

            // 2. Aumentar a resolução da imagem para melhorar a nitidez
            Mat highResImage;
            Size newSize(frame.cols * 5, frame.rows * 5); // Aumentando a imagem em 2x
            resize(equalized, highResImage, newSize, 0, 0, INTER_CUBIC); // Usando interpolação cúbica

            // 3. Aplicar filtro de nitidez
            Mat sharpened;
            Mat kernel = (Mat_<float>(3, 3) << 0, -1, 0, -1, 5,-1, 0, -1, 0); // Kernel de nitidez
            filter2D(highResImage, sharpened, -1, kernel);

           // **Criar uma moldura mais fina e adicionar a data/hora dentro dela**
            // Definir a área da moldura (mais próxima dos limites)
            Rect moldura(10, 10, sharpened.cols - 20, sharpened.rows - 20); // Margens de 10px em cada lado
            rectangle(sharpened, moldura, Scalar(0, 0, 255), 2); // Moldura vermelha, espessura 2

            // Adicionar a data/hora no canto superior direito da moldura
            stringstream datetimeImage;
            datetimeImage << put_time(localtime(&now_c), "%H:%M:%S %d/%m/%Y"); // Formato: hh:mm:ss dd/mm/yyyy
            Size textSize = getTextSize(datetimeImage.str(), FONT_HERSHEY_SIMPLEX, 1.0, 3, 0);
            Point textOrigin(sharpened.cols - textSize.width - 20, 40); // Canto superior direito
            putText(sharpened, datetimeImage.str(), textOrigin, FONT_HERSHEY_SIMPLEX, 1.0, Scalar(255, 255, 255), 3); // Texto branco, mais gros

            // Salvar a imagem na pasta "placas_salvas"
            string filename = pasta + "/placa_" + datetime.str() + ".jpg";

            // Verificar se o arquivo já existe e evitar sobrescrita
            int counter = 1;
            while (fs::exists(filename)) {
                filename = pasta + "/placa_" + datetime.str() + "_" + to_string(counter++) + ".jpg";
            }

            // Salvar a imagem com a moldura e a data/hora
            if (!imwrite(filename, sharpened)) {
                cerr << "Erro ao salvar a imagem!" << endl;
            } else {
                cout << "Imagem salva como: " << filename << endl;
            }
        } else {
            cout << "Nenhuma placa detectada." << endl;
        }

        // Perguntar se o usuário deseja tentar ler outra placa
        cout << "Deseja tentar ler outra placa? (sim/nao): ";
        cin >> resposta;

        if (resposta == "nao" || resposta == "Nao") {
            break; // Se a resposta for "não", sair do loop e encerrar o programa
        }

        // Se a resposta for "sim", continuar o loop e capturar um novo frame
        cout << "Tentando novamente..." << endl;
    }

    // Finalizar o OCR e liberar recursos
    ocr.End();
    cap.release(); // Libera a câmera
    destroyAllWindows(); // Fecha as janelas abertas

    cout << "Programa encerrado." << endl;

    return 0;
}

