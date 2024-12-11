#include <windows.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <string> 

using namespace std;

string xorCipher(string& text, string key) {
    string result = text;

    for (size_t i = 0; i < text.size(); ++i) {
        result[i] ^= key[i % key.size()];
    }

    return result;
}


int main(int argc, char* argv[]) {


    string namedPipeGenerator = "\\\\.\\pipe\\encoderPipe";

    HANDLE hPipeEncoder = CreateFileA(
        namedPipeGenerator.c_str(),   // Имя канала
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


    char buffer[512];
    DWORD bufferSize = 512;
    DWORD bytesRead;

    ReadFile(hPipeEncoder, buffer, bufferSize - 1, &bytesRead, NULL);

    string text(buffer), key = "osisp";
    //cout << "text: " << text << '\n';

    string encodedText = xorCipher(text, key);
    //cout << "encodedText: " << encodedText << '\n';

    DWORD bytesWritten;
    WriteFile(hPipeEncoder, &encodedText[0], encodedText.size() + 1, &bytesWritten, NULL);


    CloseHandle(hPipeEncoder);

    return 0;
}
