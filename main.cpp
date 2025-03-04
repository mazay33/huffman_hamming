#include <iostream>
#include <fstream>
#include <queue>
#include <unordered_map>
#include <vector>
#include <bitset>
#include <string>
#include <sstream>
#include <algorithm>
#include <sys/stat.h>

using namespace std;

// Проверка существования файла с использованием sys/stat.h
bool fileExists(const string& fileName) {
    struct stat buffer;
    return (stat(fileName.c_str(), &buffer) == 0);
}

// Структура узла дерева Хаффмана
struct HuffmanNode {
    char ch;        // Символ
    int freq;       // Частота символа
    HuffmanNode* left, * right; // Указатели на левого и правого потомков

    // Конструктор для инициализации узла
    HuffmanNode(char c, int f) : ch(c), freq(f), left(nullptr), right(nullptr) {}
};

// Функтор для сравнения узлов в приоритетной очереди
struct compare {
    bool operator()(HuffmanNode* left, HuffmanNode* right) {
        // При равных частотах учитываем тип узла (лист/внутренний) и символы
        if (left->freq == right->freq) {
            bool isLeftLeaf = (left->left == nullptr && left->right == nullptr);
            bool isRightLeaf = (right->left == nullptr && right->right == nullptr);

            // Листья сравниваются по символам, внутренние узлы — по первому символу поддерева
            if (isLeftLeaf && isRightLeaf) return left->ch > right->ch;
            else if (isLeftLeaf) return false; // Лист имеет приоритет над внутренним узлом
            else if (isRightLeaf) return true;
            else return left->left->ch > right->left->ch;
        }
        // Узел с меньшей частотой имеет высший приоритет
        return left->freq > right->freq;
    }
};

// Кодирование 4-битного блока в 7-битный код Хэмминга
string hammingEncodeBlock(const string& block) {
    // Извлечение битов данных
    char d1 = block[0], d2 = block[1], d3 = block[2], d4 = block[3];
    // Вычисление контрольных битов
    char p1 = ((d1 - '0') ^ (d2 - '0') ^ (d4 - '0')) ? '1' : '0';
    char p2 = ((d1 - '0') ^ (d3 - '0') ^ (d4 - '0')) ? '1' : '0';
    char p3 = ((d2 - '0') ^ (d3 - '0') ^ (d4 - '0')) ? '1' : '0';
    // Формирование закодированного блока: p1, p2, d1, p3, d2, d3, d4
    return { p1, p2, d1, p3, d2, d3, d4 };
}

// Декодирование 7-битного блока Хэмминга с исправлением ошибок
string hammingDecodeBlock(const string& block) {
    // Извлечение битов из блока
    char p1 = block[0], p2 = block[1], d1 = block[2],
        p3 = block[3], d2 = block[4], d3 = block[5], d4 = block[6];

    // Вычисление синдромов для обнаружения ошибок
    int s1 = (p1 - '0') ^ (d1 - '0') ^ (d2 - '0') ^ (d4 - '0');
    int s2 = (p2 - '0') ^ (d1 - '0') ^ (d3 - '0') ^ (d4 - '0');
    int s3 = (p3 - '0') ^ (d2 - '0') ^ (d3 - '0') ^ (d4 - '0');
    int errorPos = s1 * 1 + s2 * 2 + s3 * 4; // Позиция ошибки (0 - нет ошибки)

    // Исправление ошибки в соответствующем бите
    if (errorPos > 0) {
        switch (errorPos) {
        case 1: p1 ^= 1; break; // Ошибка в p1
        case 2: p2 ^= 1; break; // Ошибка в p2
        case 3: d1 ^= 1; break; // Ошибка в d1
        case 4: p3 ^= 1; break; // Ошибка в p3
        case 5: d2 ^= 1; break; // Ошибка в d2
        case 6: d3 ^= 1; break; // Ошибка в d3
        case 7: d4 ^= 1; break; // Ошибка в d4
        }
    }

    // Возврат исправленных битов данных
    return { d1, d2, d3, d4 };
}

// Кодирование всей битовой строки с использованием кода Хэмминга
string hammingEncode(const string& data, int& originalBits) {
    originalBits = data.size(); // Сохранение исходной длины
    size_t padded_len = ((data.size() + 3) / 4) * 4; // Дополнение до длины, кратной 4
    string padded = data + string(padded_len - data.size(), '0'); // Дополнение нулями
    string encoded;

    // Кодирование каждого 4-битного блока
    for (size_t i = 0; i < padded_len; i += 4) {
        encoded += hammingEncodeBlock(padded.substr(i, 4));
    }
    return encoded;
}

// Декодирование битовой строки с исправлением ошибок
string hammingDecode(const string& encoded, int originalBits) {
    string decoded;
    // Обработка каждого 7-битного блока
    for (size_t i = 0; i < encoded.size(); i += 7) {
        if (i + 7 > encoded.size()) break; // Пропуск неполного блока в конце
        decoded += hammingDecodeBlock(encoded.substr(i, 7));
    }
    // Обрезка до исходной длины (удаление дополняющих нулей)
    return decoded.substr(0, originalBits);
}

// Рекурсивное построение таблицы кодов Хаффмана
void buildCodes(HuffmanNode* root, const string& str, unordered_map<char, string>& huffCodes) {
    if (!root) return;
    // Если узел - лист, сохраняем код символа
    if (root->ch != '\0') huffCodes[root->ch] = str;
    // Рекурсивный обход левого и правого поддеревьев
    buildCodes(root->left, str + "0", huffCodes);
    buildCodes(root->right, str + "1", huffCodes);
}

// Функция сжатия файла
void compress(const string& inputFile, const string& outputFile, bool useHamming) {
    // Проверка существования входного файла
    if (!fileExists(inputFile)) {
        cerr << "Error: Input file does not exist." << endl;
        return;
    }

    // Чтение файла и подсчет частот символов
    ifstream in(inputFile, ios::binary);
    unordered_map<char, int> freqMap;
    char ch;
    while (in.get(ch)) freqMap[ch]++;
    in.close();

    // Построение приоритетной очереди узлов
    priority_queue<HuffmanNode*, vector<HuffmanNode*>, compare> pq;
    for (const auto& entry : freqMap)
        pq.push(new HuffmanNode(entry.first, entry.second));

    // Построение дерева Хаффмана объединением узлов
    while (pq.size() > 1) {
        HuffmanNode* left = pq.top(); pq.pop();
        HuffmanNode* right = pq.top(); pq.pop();
        HuffmanNode* newNode = new HuffmanNode('\0', left->freq + right->freq);
        newNode->left = left;
        newNode->right = right;
        pq.push(newNode);
    }

    // Генерация кодов Хаффмана
    unordered_map<char, string> huffCodes;
    if (!pq.empty()) buildCodes(pq.top(), "", huffCodes);

    // Кодирование исходного файла
    in.open(inputFile, ios::binary);
    string compressedData;
    while (in.get(ch)) compressedData += huffCodes[ch];
    in.close();

    // Применение кодирования Хэмминга при необходимости
    int originalBits = compressedData.size();
    string finalData = compressedData;
    if (useHamming)
        finalData = hammingEncode(compressedData, originalBits);

    // Запись сжатых данных в выходной файл
    ofstream out(outputFile, ios::binary);
    // Заголовок файла с указанием использования Хэмминга
    out << (useHamming ? "HUFFMAN_HAMMING\n" : "HUFFMAN\n");
    // Запись таблицы частот
    for (const auto& entry : freqMap) {
        if (entry.first == '\n') out << "'\\n' " << entry.second << "\n";
        else if (entry.first == ' ') out << "' ' " << entry.second << "\n";
        else out << entry.first << " " << entry.second << "\n";
    }
    out << "\n" << originalBits << "\n"; // Исходное количество бит
    out << finalData.size() << "\n";      // Закодированное количество бит

    // Упаковка битов в байты
    bitset<8> byte;
    int bitCount = 0;
    for (char bit : finalData) {
        byte[7 - bitCount] = (bit == '1'); // Заполнение битов справа налево
        if (++bitCount == 8) {
            out.put(static_cast<char>(byte.to_ulong())); // Запись байта
            byte.reset();
            bitCount = 0;
        }
    }
    // Запись последнего неполного байта
    if (bitCount > 0)
        out.put(static_cast<char>(byte.to_ulong()));
    out.close();

    cout << "Compression successful. "
        << (useHamming ? "With Hamming encoding." : "No error protection.") << endl;
}

// Функция распаковки файла
void decompress(const string& inputFile, const string& outputFile) {
    // Проверка существования входного файла
    if (!fileExists(inputFile)) {
        cerr << "Error: Input file does not exist." << endl;
        return;
    }

    ifstream in(inputFile, ios::binary);
    string line;
    getline(in, line);
    bool useHamming = (line == "HUFFMAN_HAMMING"); // Проверка заголовка

    // Чтение таблицы частот
    unordered_map<char, int> freqMap;
    while (getline(in, line)) {
        if (line.empty()) break; // Конец таблицы
        // Обработка специальных символов
        if (line.find("'\\n'") == 0) {
            freqMap['\n'] = stoi(line.substr(5));
            continue;
        }
        if (line.find("' '") == 0) {
            freqMap[' '] = stoi(line.substr(3));
            continue;
        }
        // Обработка обычных символов
        size_t pos = line.find(' ');
        freqMap[line[0]] = stoi(line.substr(pos + 1));
    }

    // Чтение исходного и закодированного количества бит
    int originalBits, encodedBits;
    getline(in, line); originalBits = stoi(line);
    getline(in, line); encodedBits = stoi(line);

    // Чтение закодированных данных
    string encodedData;
    char byte;
    while (in.get(byte))
        encodedData += bitset<8>(byte).to_string(); // Распаковка байтов в биты
    encodedData = encodedData.substr(0, encodedBits); // Обрезка до нужной длины

    // Декодирование Хэмминга при необходимости
    string compressedData = useHamming ?
        hammingDecode(encodedData, originalBits) :
        encodedData.substr(0, originalBits);

    // Восстановление дерева Хаффмана
    priority_queue<HuffmanNode*, vector<HuffmanNode*>, compare> pq;
    for (const auto& entry : freqMap)
        pq.push(new HuffmanNode(entry.first, entry.second));

    while (pq.size() > 1) {
        HuffmanNode* left = pq.top(); pq.pop();
        HuffmanNode* right = pq.top(); pq.pop();
        HuffmanNode* newNode = new HuffmanNode('\0', left->freq + right->freq);
        newNode->left = left;
        newNode->right = right;
        pq.push(newNode);
    }

    // Получение корня дерева Хаффмана
    HuffmanNode* root = pq.empty() ? nullptr : pq.top();
    ofstream out(outputFile, ios::binary);
    HuffmanNode* curr = root;

    // Декодирование битовой строки
    for (char bit : compressedData) {
        // Перемещение по дереву в зависимости от бита (0 - влево, 1 - вправо)
        curr = (bit == '0') ? curr->left : curr->right;
        // Если достигнут лист, записываем символ и возвращаемся к корню
        if (!curr->left && !curr->right) {
            out.put(curr->ch);
            curr = root;
        }
    }
    out.close();
    cout << "Decompression successful. "
        << (useHamming ? "Hamming decoding applied." : "") << endl;
}

// Вспомогательная функция для сжатия файла (интерфейс пользователя)
void compressFile() {
    string inputFile, outputFile;
    char choice;
    bool useHamming = false;

    // Запрос пути к входному файлу
    cout << "Enter the path of the input file to compress: ";
    cin >> inputFile;
    if (!fileExists(inputFile)) {
        cout << "Error: The input file does not exist." << endl;
        return;
    }

    // Запрос использования кода Хэмминга
    cout << "Use Hamming error protection? (y/n): ";
    cin >> choice;
    if (tolower(choice) == 'y') useHamming = true;

    // Запрос пути к выходному файлу
    cout << "Enter the path of the output file: ";
    cin >> outputFile;

    // Вызов функции сжатия
    compress(inputFile, outputFile, useHamming);
}

// Вспомогательная функция для распаковки файла (интерфейс пользователя)
void decompressFile() {
    string inputFile, outputFile;

    // Запрос пути к входному файлу
    cout << "Enter the path of the input file to decompress: ";
    cin >> inputFile;
    if (!fileExists(inputFile)) {
        cout << "Error: The input file does not exist." << endl;
        return;
    }

    // Запрос пути к выходному файлу
    cout << "Enter the path of the output file: ";
    cin >> outputFile;

    // Вызов функции распаковки
    decompress(inputFile, outputFile);
}

// Основная функция с консольным меню
int main() {
    while (true) {
        cout << "-------------------\n";
        cout << "Huffman Compression Tool with Hamming\n";
        cout << "1. Compress a file\n";
        cout << "2. Decompress a file\n";
        cout << "3. Exit\n";
        cout << "Enter your choice (1-3): ";

        int choice;
        cin >> choice;

        switch (choice) {
        case 1:
            compressFile(); // Обработка сжатия
            break;
        case 2:
            decompressFile(); // Обработка распаковки
            break;
        case 3:
            cout << "Exiting...\n";
            return 0; // Выход из программы
        default:
            cout << "Invalid choice. Try again.\n"; // Некорректный ввод
        }
    }
}
