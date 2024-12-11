#include <windows.h>
#include <iostream>
#include <string> 

using namespace std;



int main() {


    string namedPipeName = "\\\\.\\pipe\\coutPipe";
    

    HANDLE hPipe = CreateFileA(
        namedPipeName.c_str(),   // Имя канала
        GENERIC_READ | GENERIC_WRITE, // Чтение и запись
        0,                      // Нет совместного доступа
        NULL,                   // Без защиты
        OPEN_EXISTING,          // Открыть существующий канал
        0,                      // Атрибуты по умолчанию
        NULL                    // Нет шаблона
    );

    if (hPipe == INVALID_HANDLE_VALUE) {
        cerr << "CreateFile failed. Error: " << GetLastError() << endl;
        return 1;
    }


    char buffer[512];
    DWORD bufferSize = 512;
    DWORD bytesRead, butesWritten;

    
    ReadFile(hPipe, buffer, bufferSize - 1, &bytesRead, NULL);

    cout << "----------------------------------------\n";
    cout << "Received from pipe: " << buffer << endl;
    cout << "----------------------------------------\n";


    CloseHandle(hPipe);

    return 0;
}
