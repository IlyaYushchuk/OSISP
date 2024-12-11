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
                // ����� �������
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
    vector<int> intVector;   // ������ ��� �������� ����� �����
    stringstream ss(str);  // ����������� ������ char � ��������� �����
    string temp;  // ��������� ������ ��� �������� ������� �����

    // �������� �� ���������� ������, �������� ��� �� ����� (�� ��������)
    while (ss >> temp) {
        try {
            // ����������� ������ ��������� � int � ��������� � ������
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
            oss << " ";  // ��������� ������ ����� �������
        }
    }
    oss << "\n";  // ��������� ������ �������� ����� ������
    //cout << "-----oss str" << oss.str() << "----\n";
    return oss.str();
}



int main() {
    HANDLE hPipe = CreateFileA(
        R"(\\.\pipe\MyPipe)",   // ��� ������
        GENERIC_READ | GENERIC_WRITE, // ������ � ������
        0,                      // ��� ����������� �������
        NULL,                   // ��� ������
        OPEN_EXISTING,          // ������� ������������ �����
        0,                      // �������� �� ���������
        NULL                    // ��� �������
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
