#include <windows.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <chrono>
#include <fstream>
#include "tools.cpp"

using namespace std;
const int chunk_sizes[] = {25, 296, 3897};
const DWORD CHUNK_SIZE = 296;
const int string_number = 1000;

#define THREADS 4

struct ThreadInfo
{
    OVERLAPPED overlapped; // Structure for asynchronous I/O
    HANDLE file_handle;    // Handle for the file being read/written
    char* buffer;          // Buffer to store data
    size_t size;           // Size of the data to read/write
    size_t offset;         // Offset for reading/writing
};


int bubblesort(vector<int>* mass, int n)
{
    //Sleep(1);
    int opCount = 0;
    for (int i = 1; i < n; ++i)
    {
        for (int r = 0; r < n - i; r++)
        {
            if ((*mass)[r] > (*mass)[r + 1])
            {
                // Обмен местами
                int temp = (*mass)[r];
                (*mass)[r] = (*mass)[r + 1];
                (*mass)[r + 1] = temp;
            }
            opCount++;
        }
    }
    return opCount;
}
vector<int> CharArrayToIntVector(string str) {
    vector<int> intVector;   // Вектор для хранения целых чисел
    stringstream ss(str);  // Преобразуем массив char в строковый поток
    string temp;  // Временная строка для хранения каждого числа

    // Проходим по строковому потоку, разбивая его на части (по пробелам)
    while (ss >> temp) {
        try {
            // Преобразуем каждую подстроку в int и добавляем в вектор
            int number = stoi(temp);
            intVector.push_back(number);
        }
        catch (const invalid_argument& e) {
            cerr << "Error: incorrect number format '" << temp << "'" << endl;
        }
    }

    return intVector;
}
string VectorToString(const vector<int>& vec) {
    ostringstream oss;
    for (size_t i = 0; i < vec.size(); ++i) {
        oss << vec[i];
        if (i < vec.size() - 1) {
            oss << " ";  // Добавляем пробел между числами
        }
    }
    oss << "\n";  // Завершаем строку символом новой строки
    //cout << "-----oss str" << oss.str() << "----\n";
    return oss.str();
}
vector<string> SplitBufferIntoLines(char* buffer, size_t bufferSize) {
    vector<string> lines;
    string currentLine;

    for (size_t i = 0; i < bufferSize; ++i) {
        if (buffer[i] == '\n') {
            lines.push_back(currentLine);  // Добавляем строку в вектор
            currentLine.clear();           // Очищаем для следующей строки
        }
        else if (buffer[i] != '\r') {    // Пропускаем символ возврата каретки (если есть)
            currentLine += buffer[i];
        }
    }

    // Добавляем последнюю строку, если она не пуста
    if (!currentLine.empty()) {
        lines.push_back(currentLine);
    }
    return lines;
}


DWORD WINAPI ReadFileAsync(LPVOID args)
{
    auto start = chrono::steady_clock::now();

    ThreadInfo* io_data = (ThreadInfo*)args; // Cast parameter to AsyncIOData
    DWORD bytes_read;

    // Set the offset for the read operation
    io_data->overlapped.Offset = (DWORD)io_data->offset;
    io_data->overlapped.OffsetHigh = (DWORD)(io_data->offset >> 32);

    // Start the read operation
    if (!ReadFile(io_data->file_handle, io_data->buffer, io_data->size, &bytes_read, &io_data->overlapped))
    {
        // Check if the read operation is pending
        if (GetLastError() != ERROR_IO_PENDING)
        {
            perror("Failed to initiate read");
            return 1;
        }
    }

    // Wait for the read operation to complete
    GetOverlappedResult(io_data->file_handle, &io_data->overlapped, &bytes_read, TRUE);
    auto end = chrono::steady_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);
    //io_data->time = duration.count();
    return bytes_read;
}

void Processing(char* buffer, int size)
{
    vector<string> lines = SplitBufferIntoLines(buffer, size);

    vector<int> f;
    string line;
    size_t currentPos = 0;

    
    for (int i = 0; i < lines.size(); i++)
    {
        f = CharArrayToIntVector(lines[i]);
        bubblesort(&f, f.size());
        line = VectorToString(f);
        size_t lineSize = line.size();
 
        memcpy(buffer + currentPos, line.c_str(), lineSize);
        currentPos += lineSize;
    }

}

DWORD WINAPI WriteFileAsync(LPVOID args)
{
    ThreadInfo* io_data = (ThreadInfo*)args; 
    DWORD bytes_written;

    // Set the offset for the write operation
    io_data->overlapped.Offset = (DWORD)io_data->offset;
    io_data->overlapped.OffsetHigh = (DWORD)(io_data->offset >> 32);

    // Start the write operation
    if (!WriteFile(io_data->file_handle, io_data->buffer, io_data->size, &bytes_written, &io_data->overlapped))
    {
        // Check if the write operation is pending
        if (GetLastError() != ERROR_IO_PENDING)
        {
            perror("Failed to initiate write");
            return 1;
        }
    }

    // Wait for the write operation to complete
    GetOverlappedResult(io_data->file_handle, &io_data->overlapped, &bytes_written, TRUE);
    return bytes_written; // Return number of bytes written
}

int ProcessFileAsync(const wstring& filenameRead, const wstring& filenameWrite) {
    auto start = chrono::steady_clock::now();
   
    HANDLE hFileR = CreateFile(
        filenameRead.c_str(),               // Имя файла
        GENERIC_READ,           // Доступ на чтение
        0,                      // Нет совместного доступа
        NULL,                   // Атрибуты безопасности
        OPEN_EXISTING,          // Открыть существующий файл
        FILE_FLAG_OVERLAPPED && FILE_ATTRIBUTE_NORMAL,   // Асинхронный ввод-вывод
        NULL                    // Шаблон файла
    );

    HANDLE hFileW = CreateFile(
        filenameWrite.c_str(),       // Имя файла
        GENERIC_WRITE,          // Доступ на запись
        0,                      // Нет совместного доступа
        NULL,                   // Атрибуты безопасности
        OPEN_ALWAYS,            // Открыть существующий или создать новый файл
        FILE_FLAG_OVERLAPPED && FILE_ATTRIBUTE_NORMAL,   // Асинхронный ввод-вывод
        NULL                    // Шаблон файла
    );

    LARGE_INTEGER fileSize;
    GetFileSizeEx(hFileR, &fileSize);
    //cout << fileSize.QuadPart << '\n';
    SetFilePointerEx(hFileW, fileSize, NULL, FILE_BEGIN);
    SetEndOfFile(hFileW);

    // Используем SetFileValidData для установки валидных данных
    SetFileValidData(hFileW, fileSize.QuadPart);
    //LARGE_INTEGER fileSize;
    //GetFileSizeEx(hFileR, &fileSize);
    //SetFilePointerEx(hFileW, fileSize, NULL, FILE_BEGIN);
    //

   /* int s;
    cin >> s;*/
    /*if (hFile == INVALID_HANDLE_VALUE) {
        cerr << "Не удалось открыть файл, ошибка: " << GetLastError() << endl;
        return;
    }*/

    OVERLAPPED overlappedR = { 0 };
    overlappedR.Offset = 0;  // Начало чтения с начала файла
    overlappedR.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    OVERLAPPED overlappedW = { 0 };
    overlappedW.Offset = 0;  // Пишем с начала файла
    overlappedW.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    /*if (overlapped.hEvent == NULL) {
        cerr << "Не удалось создать событие, ошибка: " << GetLastError() << endl;
        CloseHandle(hFile);
        return;
    }*/

    char buffer[CHUNK_SIZE] = { 0 };
    DWORD bytesRead = 0;
    DWORD bytesWritten = 0;


    while (true) {
        BOOL readResult = ReadFile(
            hFileR,
            buffer,
            CHUNK_SIZE,
            &bytesRead,
            &overlappedR
        );

        if (!readResult) {
            if (GetLastError() == ERROR_IO_PENDING) {
                // Ожидаем завершения асинхронного чтения
                WaitForSingleObject(overlappedR.hEvent, INFINITE);
                GetOverlappedResult(hFileR, &overlappedR, &bytesRead, FALSE);
            }
            else {
                cerr << "Ошибка чтения файла, код: " << GetLastError() << endl;
                break;
            }
        }

        if (bytesRead == 0) {
            // Конец файла
            break;
        }

        //cout << "BUFFER: " << buffer << '\n';

        string str(buffer);
        str = str.substr(0, CHUNK_SIZE - 3);

        //cout << "BUFFER-2: " << str << '\n';

        ZeroMemory(buffer, sizeof(buffer));
        
        vector<int> arr = CharArrayToIntVector(str);

        /*cout << "BEFORE BUBBLE: ";
        for (int i = 0; i < arr.size(); i++)
            cout << arr[i] << ' ';
        cout << endl;*/


        bubblesort(&arr, arr.size());
        
     /*   cout << "AFTER BUBBLE: ";
        for (int i = 0; i < arr.size(); i++)
            cout << arr[i] << ' ';
        cout << endl;*/


        string res = VectorToString(arr);
        //cout << "after: " << res;
        //arr.clear();

        // Обновляем смещение для следующего чтения
        overlappedR.Offset += CHUNK_SIZE;
        ResetEvent(overlappedR.hEvent);

        BOOL writeResult = WriteFile(
            hFileW,                          // Дескриптор файла
            res.c_str(),                   // Данные для записи
            CHUNK_SIZE-2, // Размер данных для записи
            &bytesWritten,                  // Фактически записанные байты
            &overlappedW                     // OVERLAPPED структура
        );

        if (!writeResult) {
            if (GetLastError() == ERROR_IO_PENDING) {
                // Ожидаем завершения асинхронной записи
                WaitForSingleObject(overlappedW.hEvent, INFINITE);

                // Получаем результат асинхронной записи
                if (!GetOverlappedResult(hFileW, &overlappedW, &bytesWritten, FALSE)) {
                    /*cerr << "Ошибка записи в файл, код: " << GetLastError() << endl;*/
                    break;
                }
            }
            else {
                /*cerr << "Ошибка записи в файл, код: " << GetLastError() << endl;*/
                break;
            }
        }

        overlappedW.Offset += bytesWritten;
        ResetEvent(overlappedW.hEvent);
   }

    // Закрытие дескрипторов

    CloseHandle(overlappedW.hEvent);
    CloseHandle(hFileW);
    CloseHandle(overlappedR.hEvent);
    CloseHandle(hFileR);
    auto end = chrono::steady_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);
    cout << "Async DONE : " << duration.count() << '\n';
    return duration.count();
}

int ProcessFile(const wstring& filenameRead, const wstring& filenameWrite) {
    auto start = chrono::steady_clock::now();

    HANDLE hFileR = CreateFile(
        filenameRead.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    HANDLE hFileW = CreateFile(
        filenameWrite.c_str(),       // Имя файла
        GENERIC_WRITE,          // Доступ на запись
        0,                      // Нет совместного доступа
        NULL,                   // Атрибуты безопасности
        CREATE_ALWAYS,          // Создать файл, если не существует, или перезаписать, если существует
        FILE_ATTRIBUTE_NORMAL,  // Обычный файл
        NULL                    // Шаблон файла
    );

    if (hFileR == INVALID_HANDLE_VALUE) {
        cerr << "Ошибка открытия файла: " << GetLastError() << endl;
        return 0;
    }

    const DWORD bufferSize = CHUNK_SIZE; // Размер буфера для чтения
    char buffer[bufferSize];
    DWORD bytesRead = 0;
    DWORD bytesWritten = 0;


    while (true) {
        BOOL result = ReadFile(hFileR, buffer, bufferSize, &bytesRead, NULL);
        if (!result || bytesRead == 0) {
            break; // Выход из цикла, если произошла ошибка или достигнут конец файла
        }

        
        //cout << "BUFFER: " << buffer << '\n';

        string str(buffer);
        str = str.substr(0, CHUNK_SIZE - 3);

        //cout << "BUFFER-2: " << str << '\n';

        ZeroMemory(buffer, sizeof(buffer));

        vector<int> arr = CharArrayToIntVector(str);

        /*cout << "BEFORE BUBBLE: ";
        for (int i = 0; i < arr.size(); i++)
            cout << arr[i] << ' ';
        cout << endl;
*/

        bubblesort(&arr, arr.size());

        /*cout << "AFTER BUBBLE: ";
        for (int i = 0; i < arr.size(); i++)
            cout << arr[i] << ' ';
        cout << endl;*/


        //cout << arr.size() << "ttt";
        string res = VectorToString(arr);
        /*res += buffer[22];
        res += buffer[23];
        res += buffer[24];
        cout << res.size() << "yyy";*/
        BOOL writeResult = WriteFile(
            hFileW,                       // Дескриптор файла
            res.c_str(),                // Данные для записи
            CHUNK_SIZE -2,  // Размер данных для записи
            &bytesWritten,               // Фактически записанные байты
            NULL                         // OVERLAPPED не используется для синхронной записи
        );
        if (!writeResult || bytesWritten == 0) {
            break;
        }
    }

    CloseHandle(hFileR); // Закрываем дескриптор файла
    CloseHandle(hFileW);
    auto end = chrono::steady_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);
    //cout << "DONE : " << duration.count() << '\n';
    return duration.count();
}

char* ReadFromFile(const wstring& filename) {
    // Открываем файл для чтения
    HANDLE hFile = CreateFile(
        filename.c_str(),       // Имя файла
        GENERIC_READ,           // Доступ на чтение
        0,                      // Нет совместного доступа
        NULL,                   // Атрибуты безопасности
        OPEN_EXISTING,          // Открыть существующий файл
        FILE_ATTRIBUTE_NORMAL,  // Обычный файл
        NULL                    // Шаблон файла
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        cerr << "Не удалось открыть файл, ошибка: " << GetLastError() << endl;
        return nullptr;
    }

    // Получаем размер файла
    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(hFile, &fileSize)) {
        cerr << "Не удалось получить размер файла, ошибка: " << GetLastError() << endl;
        CloseHandle(hFile);
        return nullptr;
    }

    // Проверяем, не превышает ли размер файл допустимый диапазон для чтения в память
    if (fileSize.QuadPart > MAXDWORD) {
        cerr << "Файл слишком большой для чтения целиком!" << endl;
        CloseHandle(hFile);
        return nullptr;
    }

    // Буфер для чтения данных из файла
    DWORD bytesRead;
    char* buffer = new char[static_cast<DWORD>(fileSize.QuadPart) + 1];  // Плюс 1 для завершающего нуля

    // Чтение данных из файла
    if (!ReadFile(hFile, buffer, static_cast<DWORD>(fileSize.QuadPart), &bytesRead, NULL)) 
    {
        cerr << "Ошибка чтения файла, код: " << GetLastError() << endl;
        delete[] buffer;
        CloseHandle(hFile);
        return nullptr;
    }

    // Завершаем строку нулевым символом
    buffer[bytesRead] = '\0';

    // Выводим данные на экран
    //cout << "Содержимое файла:\n" << buffer << endl;

    CloseHandle(hFile);
    return buffer;
}

string ProcessData(const char* charPtr, size_t n) {
    // Приводим указатель char* к string
    string str(charPtr);
    vector<string> substrings;

    // Цикл, который проходит по строке и разбивает её на части длиной n
    for (size_t i = 0; i < str.size(); i += n) {
        substrings.push_back(str.substr(i, n));
    }

    string processedData= "";
    for (size_t i = 0; i < substrings.size(); i ++) {

        vector<int> arr = CharArrayToIntVector(substrings[i]);
        /*for (int j = 0; j < arr.size(); j++)
            cout << arr[j] << " ";
        cout << endl;*/
        bubblesort(&arr, arr.size());
        string res = VectorToString(arr);
     //   cout << "res " << res;
        processedData += res;
    }
   // cout << "processedData " << processedData<<"\n";
    return processedData;
}

bool WriteToFile(const wstring& filename, string& data) {
    // Открываем файл для записи, создаем новый если он не существует
    HANDLE hFile = CreateFile(
        filename.c_str(),       // Имя файла
        GENERIC_WRITE,          // Доступ на запись
        0,                      // Нет совместного доступа
        NULL,                   // Атрибуты безопасности
        CREATE_ALWAYS,          // Создать файл, если не существует, или перезаписать, если существует
        FILE_ATTRIBUTE_NORMAL,  // Обычный файл
        NULL                    // Шаблон файла
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        cerr << "Не удалось открыть файл, ошибка: " << GetLastError() << endl;
        return false;
    }

    DWORD bytesWritten = 0;

    // Записываем данные в файл
    BOOL writeResult = WriteFile(
        hFile,                       // Дескриптор файла
        data.c_str(),                // Данные для записи
        static_cast<DWORD>(data.size()),  // Размер данных для записи
        &bytesWritten,               // Фактически записанные байты
        NULL                         // OVERLAPPED не используется для синхронной записи
    );

    if (!writeResult) {
        cerr << "Ошибка записи в файл, код: " << GetLastError() << endl;
        CloseHandle(hFile);
        return false;
    }

    //cout << "Записано байт: " << bytesWritten << endl;

    // Закрываем файл
    CloseHandle(hFile);
    return true;
}

int ProcessFile2(const wstring& filenameRead, const wstring& filenameWrite)
{
    auto start = chrono::steady_clock::now();

    char* buffer = ReadFromFile(filenameRead);

    string data = ProcessData(buffer, CHUNK_SIZE);

   
    WriteToFile(filenameWrite, data);
    {
        Sleep(string_number * 0.05);
    }
    auto end = chrono::steady_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);
    //cout << "DONE : " << duration.count() << '\n';
    
    return duration.count();
}



void writeSequenceInReverseToFile(const wstring& filename, const vector<int>& sequence)
{
    ofstream outFile(filename);
    if (!outFile.is_open()) {
        cerr << "Ошибка открытия файла: " << filename.c_str() << endl;
        return;
    }

    for (int i = 0; i < string_number; i++) 
    {
    // Записываем в файл
    for (const int& number : sequence) {
        outFile << number << ' ';
    }
    outFile << '\n';

    }

    outFile.close(); // Закрываем файл
}

int main() {
    
    wstring fileNameR = L"start.txt";
    wstring fileNameW = L"end.txt";
    wstring fileNameR1 = L"start1.txt";
    wstring fileNameW1 = L"end1.txt";

    vector<int> a;
    for (int i = 100; i >= 0; i--)
    {
        a.push_back(i);
    }
    writeSequenceInReverseToFile(fileNameR, a);
    writeSequenceInReverseToFile(fileNameR1, a);

   
    HANDLE fileReadHandle = CreateFile(fileNameR.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
    if (fileReadHandle == INVALID_HANDLE_VALUE)
    {
        perror("Failed to open input file");
        return EXIT_FAILURE;
    }

    LARGE_INTEGER file_size;

    // Get the size of the input file
    if (!GetFileSizeEx(fileReadHandle, &file_size))
    {
        CloseHandle(fileReadHandle);
        perror("Failed to get file size");
        return EXIT_FAILURE;
    }

    //cout<<"File size: B "<<file_size.QuadPart<<'\n';

    size_t chunk_size = file_size.QuadPart / THREADS; // Calculate chunk size for each thread
    HANDLE read_threads[THREADS];                     // Array to hold read thread handles
    HANDLE write_threads[THREADS];                    // Array to hold write thread handles
    ThreadInfo read_data[THREADS];                   // Array to hold read data structures
    ThreadInfo write_data[THREADS];                  // Array to hold write data structures
    char* buffers[THREADS];                           // Array of buffers for each thread

    auto start = std::chrono::high_resolution_clock::now();


    for (int i = 0; i < THREADS; ++i)
    {
        buffers[i] = (char*)malloc(chunk_size); // Allocate memory for buffer
        if (!buffers[i])
        {
            perror("Failed to allocate memory for buffer");
            return EXIT_FAILURE;
        }

        // Initialize read data for each thread
        read_data[i].overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        read_data[i].file_handle = fileReadHandle;
        read_data[i].buffer = buffers[i];
        read_data[i].size = chunk_size;
        read_data[i].offset = i * chunk_size;

        // Create a thread for reading
        read_threads[i] = CreateThread(NULL, 0, ReadFileAsync, &read_data[i], 0, NULL);
        if (!read_threads[i])
        {
            perror("Failed to create read thread");
            return EXIT_FAILURE;
        }
    }

    // Wait for all read operations to complete
    WaitForMultipleObjects(THREADS, read_threads, TRUE, INFINITE);
    for (int i = 0; i < THREADS; ++i)
    {
        CloseHandle(read_threads[i]); // Close thread handles
    }


    /*for (int i = 0; i < THREADS; ++i)
    {
        cout<< i << ") ";
        for (int j = 0; j < read_data[i].size; ++j)
        {

            cout<< read_data[i].buffer[j];
        }
        cout << '\n';
    }*/

    for (int i = 0; i < THREADS; ++i)
    {
        Processing(read_data[i].buffer, read_data[i].size);
    }

    //cout << read_data[0].size << '\n';

    //for (int i = 0; i < THREADS; ++i)
    //{
    //    cout << i << ") ";
    //    for (int j = 0; j < read_data[i].size - string_number * 2 / THREADS; ++j)
    //    {

    //        cout << read_data[i].buffer[j];
    //    }
    //    cout << '\n';
    //}

    // Create output file handle for writing
    HANDLE fileWriteHandle = CreateFile(fileNameW.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_FLAG_OVERLAPPED, NULL);
    if (fileWriteHandle == INVALID_HANDLE_VALUE)
    {
        CloseHandle(fileReadHandle);
        perror("Failed to open output file");
        return EXIT_FAILURE;
    }

    // Write operations in parallel
    for (int i = 0; i < THREADS; ++i)
    {
        // Initialize write data for each thread
        write_data[i].overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        write_data[i].file_handle = fileWriteHandle;
        write_data[i].buffer = buffers[i];
        write_data[i].size = chunk_size - string_number*2/THREADS;
        write_data[i].offset = i * (chunk_size - string_number * 2 / THREADS);

        // Create a thread for writing
        write_threads[i] = CreateThread(NULL, 0, WriteFileAsync, &write_data[i], 0, NULL);
        if (!write_threads[i])
        {
            perror("Failed to create write thread");
            return EXIT_FAILURE;
        }
    }

    // Wait for all write operations to complete
    WaitForMultipleObjects(THREADS, write_threads, TRUE, INFINITE);
    for (int i = 0; i < THREADS; ++i)
    {
        CloseHandle(write_threads[i]); // Close thread handles
        free(buffers[i]);              // Free allocated buffers
    }

    CloseHandle(fileReadHandle);
    CloseHandle(fileWriteHandle);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    int async = duration_ms.count();
    int sync2 = ProcessFile2(fileNameR1, fileNameW1);
    //int sync = ProcessFile(fileNameR1, fileNameW1);
    cout << "Async - " << async << '\n';
    //cout << "Sync - " << sync << '\n';
    cout << "Sync - " << sync2 << '\n';
    cout << double (sync2 - async) / async * 100.;

    return 0;
}