#include <windows.h>
#include <iostream>
#include <vector>
#include <string>

using namespace std;

string xorCipher(string& text, string key) {
    string result = text;

    for (size_t i = 0; i < text.size(); ++i) {
        result[i] ^= key[i % key.size()];
    }

    return result;
}


int main() {
  
        string namedPipeCoutManager = "\\\\.\\pipe\\coutPipe";
        string namedPipeEncoder = "\\\\.\\pipe\\encoderPipe";
        string namedPipeDecoder = "\\\\.\\pipe\\decoderPipe";

         HANDLE hPipeEncoder = CreateNamedPipeA(
             namedPipeEncoder.c_str(),   // Имя канала
            PIPE_ACCESS_DUPLEX,     // Чтение и запись
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, // Тип сообщения
            PIPE_UNLIMITED_INSTANCES, // Максимальное количество инстанций
            512,                    // Выходной буфер
            512,                    // Входной буфер
            0,                      // Тайм-аут
            NULL                    // Без защиты
        );

        if (hPipeEncoder == INVALID_HANDLE_VALUE) {
            cerr << "CreateFile failed. Error: " << GetLastError() << endl;
            return 1;
        }

        HANDLE hPipeDecoder = CreateNamedPipeA(
            namedPipeDecoder.c_str(),   // Имя канала
            PIPE_ACCESS_DUPLEX,     // Чтение и запись
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, // Тип сообщения
            PIPE_UNLIMITED_INSTANCES, // Максимальное количество инстанций
            512,                    // Выходной буфер
            512,                    // Входной буфер
            0,                      // Тайм-аут
            NULL                    // Без защиты
        );

        if (hPipeDecoder == INVALID_HANDLE_VALUE) {
            cerr << "CreateFile failed. Error: " << GetLastError() << endl;
            return 1;
        }

         HANDLE hPipeCoutManager = CreateNamedPipeA(
             namedPipeCoutManager.c_str(),   // Имя канала
            PIPE_ACCESS_DUPLEX,     // Чтение и запись
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, // Тип сообщения
            PIPE_UNLIMITED_INSTANCES, // Максимальное количество инстанций
            512,                    // Выходной буфер
            512,                    // Входной буфер
            0,                      // Тайм-аут
            NULL                    // Без защиты
        );

        if (hPipeCoutManager == INVALID_HANDLE_VALUE) {
            cerr << "CreateNamedPipe failed. Error: " << GetLastError() << endl;
            return 1;
        }

        string encoderPath = "D:\\253505\\5sem\\OSiSP\\LR3\\LR3\\x64\\Debug\\Encoder.exe";
   
        PROCESS_INFORMATION pi;
        STARTUPINFO si;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));
        
        wstring encoderPathModefied(encoderPath.begin(), encoderPath.end());

        if (!CreateProcess(
            &encoderPathModefied[0],  // Путь к исполняемому файлу
            NULL,             // Командная строка
            NULL,             // Указатель на процесс
            NULL,             // Указатель на поток
            FALSE,            // Наследовать дескрипторы
            NULL,                // Флаги создания
            NULL,             // Переменные окружения
            NULL,             // Рабочий каталог
            &si,              // Структура информации о запуске
            &pi))             // Структура информации о процессе
        {
            cerr << "CreateProcess failed (" << GetLastError() << ").\n";
            return -1;
        }




        cout << "Waiting for encoder to connect..." << endl;

        BOOL result = ConnectNamedPipe(hPipeEncoder, NULL);
        if (!result) {
            cerr << "ConnectNamedPipe failed. Error: " << GetLastError() << endl;
            CloseHandle(hPipeEncoder);
            return 1;
        }

        cout << "Encoder connected." << endl;

        
        DWORD bytesWritten;

        string buffer("TEST MESSAGE TO ENCODE AND DECODE");
      /*  string key("osisp");
        cout << "Message: " << buffer<<'\n';
        string encoded = xorCipher(buffer, key);
        cout << "Encoded message: " << encoded << '\n';
        cout << "Message: " << xorCipher(encoded, key) << '\n';*/

        
        WriteFile(hPipeEncoder, &buffer[0], buffer.size()+1, &bytesWritten, NULL);

        DWORD bytesRead;
       
        ReadFile(hPipeEncoder, &buffer[0], buffer.size() + 1, &bytesRead, NULL);

        //cout << "Received from encoder: "<< buffer << endl;

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);


        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));

        string coutManagerPath = "D:\\253505\\5sem\\OSiSP\\LR3\\LR3\\x64\\Debug\\CoutManager.exe";
        wstring coutManagerPathModefied(coutManagerPath.begin(), coutManagerPath.end());

        if (!CreateProcess(
            &coutManagerPathModefied[0],  // Путь к исполняемому файлу
            NULL,             // Командная строка
            NULL,             // Указатель на процесс
            NULL,             // Указатель на поток
            FALSE,            // Наследовать дескрипторы
            NULL,                // Флаги создания
            NULL,             // Переменные окружения
            NULL,             // Рабочий каталог
            &si,              // Структура информации о запуске
            &pi))             // Структура информации о процессе
        {
            cerr << "CreateProcess failed (" << GetLastError() << ").\n";
            return -1;
        }


        cout << "Waiting for cout manager to connect..." << endl;

        result = ConnectNamedPipe(hPipeCoutManager, NULL);
        if (!result) {
            cerr << "ConnectNamedPipe failed. Error: " << GetLastError() << endl;
            CloseHandle(hPipeCoutManager);
            return 1;
        }

        cout << "Cout manager connected." << endl;


        WriteFile(hPipeCoutManager, &buffer[0], buffer.size() + 1, &bytesWritten, NULL);

        cout << "Message has been encoded." << endl;


        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        CloseHandle(hPipeCoutManager);

    
        string decoderPath = "D:\\253505\\5sem\\OSiSP\\LR3\\LR3\\x64\\Debug\\Decoder.exe";
        wstring decoderPathModified(decoderPath.begin(), decoderPath.end());

        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));

        
        if (!CreateProcess(
            &decoderPathModified[0],  // Путь к исполняемому файлу
            NULL,             // Командная строка
            NULL,             // Указатель на процесс
            NULL,             // Указатель на поток
            FALSE,            // Наследовать дескрипторы
            NULL,                // Флаги создания
            NULL,             // Переменные окружения
            NULL,             // Рабочий каталог
            &si,              // Структура информации о запуске
            &pi))             // Структура информации о процессе
        {
            cerr << "CreateProcess failed (" << GetLastError() << ").\n";
            return -1;
        }



        cout << "Waiting for decoder to connect..." << endl;

        result = ConnectNamedPipe(hPipeDecoder, NULL);
        if (!result) {
            cerr << "ConnectNamedPipe failed. Error: " << GetLastError() << endl;
            CloseHandle(hPipeDecoder);
            return 1;
        }

        cout << "Decoder connected." << endl;

       
        WriteFile(hPipeDecoder, &buffer[0], buffer.size() + 1, &bytesWritten, NULL);


        ReadFile(hPipeDecoder, &buffer[0], buffer.size() + 1, &bytesRead, NULL);

       // cout << "Received from decoder: " << buffer << endl;

        cout << "Message has been decoded." << endl;

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);


        hPipeCoutManager = CreateNamedPipeA(
            namedPipeCoutManager.c_str(),   // Имя канала
            PIPE_ACCESS_DUPLEX,     // Чтение и запись
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, // Тип сообщения
            PIPE_UNLIMITED_INSTANCES, // Максимальное количество инстанций
            512,                    // Выходной буфер
            512,                    // Входной буфер
            0,                      // Тайм-аут
            NULL                    // Без защиты
        );

        if (hPipeCoutManager == INVALID_HANDLE_VALUE) {
            cerr << "CreateNamedPipe failed. Error: " << GetLastError() << endl;
            return 1;
        }


        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));

        
        if (!CreateProcess(
            &coutManagerPathModefied[0],  // Путь к исполняемому файлу
            NULL,             // Командная строка
            NULL,             // Указатель на процесс
            NULL,             // Указатель на поток
            FALSE,            // Наследовать дескрипторы
            NULL,                // Флаги создания
            NULL,             // Переменные окружения
            NULL,             // Рабочий каталог
            &si,              // Структура информации о запуске
            &pi))             // Структура информации о процессе
        {
            cerr << "CreateProcess failed (" << GetLastError() << ").\n";
            return -1;
        }


        cout << "Waiting for cout manager to connect..." << endl;

        result = ConnectNamedPipe(hPipeCoutManager, NULL);
        if (!result) {
            cerr << "ConnectNamedPipe failed. Error: " << GetLastError() << endl;
            CloseHandle(hPipeCoutManager);
            return 1;
        }

        cout << "Cout manager connected." << endl;


        WriteFile(hPipeCoutManager, &buffer[0], buffer.size() + 1, &bytesWritten, NULL);

        cout << "Message has been encoded." << endl;


        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        CloseHandle(hPipeCoutManager);
        CloseHandle(hPipeDecoder);
        CloseHandle(hPipeEncoder);
    
    return 0;
}


