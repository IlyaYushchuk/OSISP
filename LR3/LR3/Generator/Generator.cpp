#include <windows.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <string> 

using namespace std;



int main() {


        string namedPipeName = "\\\\.\\pipe\\MyPipe";

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
    DWORD bytesRead;

    vector<int> vec(11);

    ReadFile(hPipe, buffer, bufferSize - 1, &bytesRead, NULL);

    cout << "Received from server: " << buffer << endl;

    cout << '\n';


    //DWORD bytesWritten;
    //WriteFile(hPipe, vec.data(), vec.size() * sizeof(int) + 1, &bytesWritten, NULL);

    CloseHandle(hPipe);
    int a;
    cin >> a;
    return 0;
}
