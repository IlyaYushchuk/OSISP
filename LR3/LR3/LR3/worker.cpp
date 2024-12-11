#include <windows.h>
#include <iostream>
#include <vector>
#include <sstream>

using namespace std;

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



int main() {
    HANDLE hPipe = CreateFileA(
        R"(\\.\pipe\MyPipe)",   // Имя канала
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
    DWORD bytesRead;
    ReadFile(hPipe, buffer, sizeof(buffer), &bytesRead, NULL);

    cout << "Received from server: " << buffer << endl;

    string vec(buffer);

    cout << "Received from server in string: " << vec << endl;


    const char* data = "Fuck you Server, not sorry!!!!";
    DWORD bytesWritten;
    WriteFile(hPipe, data, strlen(data) + 1, &bytesWritten, NULL);

    CloseHandle(hPipe);
    int a;
    cin >> a;
    return 0;
}
