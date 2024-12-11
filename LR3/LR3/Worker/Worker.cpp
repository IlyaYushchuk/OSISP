#include <windows.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <string> 

using namespace std;



int main(int argc, char* argv[]) {

    
    string namedPipeEncoder = "\\\\.\\pipe\\encoderPipe";
    
    HANDLE hPipeEncoder = CreateFileA(
        namedPipeEncoder.c_str(),   // Имя канала
        GENERIC_READ | GENERIC_WRITE, // Чтение и запись
        0,                      // Нет совместного доступа
        NULL,                   // Без защиты
        OPEN_EXISTING,          // Открыть существующий канал
        0,                      // Атрибуты по умолчанию
        NULL                    // Нет шаблона
    );

    if (hPipeEncoder == INVALID_HANDLE_VALUE) {
        cerr << "CreateFile failed. Error: " << GetLastError() << endl;
        return 1;
    }

    DWORD bytesRead;
    
    string message = "Test message to encode and decode";

    ReadFile(hPipeEncoder, &message, message.size() + 1, &bytesRead, NULL);

    cout << "Received from server: " << buffer << endl;

    cout << '\n';


    //DWORD bytesWritten;
    //WriteFile(hPipe, vec.data(), vec.size() * sizeof(int) + 1, &bytesWritten, NULL);

    CloseHandle(hPipeEncoder);
    int a;
    cin >> a;
    return 0;
}
