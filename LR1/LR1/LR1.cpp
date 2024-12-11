#include <windows.h>
#include <commdlg.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <ctime>
#include <algorithm>
#include "Parsing.h"

using namespace std;


void logError(DWORD errorCode) {

    LPVOID errorMessageBuffer = nullptr;

    // Получаем сообщение об ошибке, соответствующее коду ошибки
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        errorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  // Язык по умолчанию
        (LPWSTR)&errorMessageBuffer,  // Указатель на буфер, который будет заполнен строкой
        0,  // Размер буфера (0 для автоматического определения)
        NULL
    );
    std::wcout << L"Error code: " << errorCode << L" - " << (LPWSTR)errorMessageBuffer << std::endl;
}

bool runExternalProgram(Task& task) {


    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;

  /*  si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    si.hStdError = GetStdHandle(STD_ERROR_HANDLE);*/

    string command = task.program + " "+ task.args;

    wstring commandW(command.begin(), command.end());
    
    if (!CreateProcess(
        NULL,               // Имя исполняемого файла
        &commandW[0],            // Командная строка
        NULL,               // Атрибуты защиты процесса
        NULL,               // Атрибуты защиты потока
        TRUE,              // Наследование дескрипторов
        0,                  // Флаги создания
        NULL,               // Указатель на блок переменных окружения
        NULL,               // Текущий каталог
        &si,                // Структура STARTUPINFO
        &pi)) {             // Структура PROCESS_INFORMATION
        DWORD errorCode = GetLastError();
        cerr << "Error in task starting\n";
        logError(errorCode);

        //logError("Error in task start. Error code: ");
        return false;
    }

    

    cout << "Started task ID: " << task.taskId << std::endl;

    
        WaitForSingleObject(pi.hProcess, INFINITE);

        
        DWORD exitCode;
        if (GetExitCodeProcess(pi.hProcess, &exitCode)) {
            if (exitCode == 0) {
                cout << "Task ID " << task.taskId << " complited" << endl<<endl;
                
            }
            else {
                cerr << "Task ID " << task.taskId << " ended with error code: " << exitCode << endl;
                try {
                    logError(exitCode);
                }
                catch (...)
                {

                }
            }
        }

    
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

    
    return true;
}





//int compareTasks(const SYSTEMTIME& currentTime, const Task& task1, const Task& task2) {
//    
//    int monthDiff1 = task1.month == -1 ? 0 : (currentTime.wMonth <= task1.month ? task1.month - currentTime.wMonth  : 12 - currentTime.wMonth + task1.month);
//    int monthDiff2 = task2.month == -1 ? 0 : (currentTime.wMonth <= task2.month ? task2.month - currentTime.wMonth : 12 - currentTime.wMonth + task2.month);
//    if (monthDiff1 != monthDiff2)
//    {
//        return monthDiff1 < monthDiff2 ? 1 : -1;
//    }
//
//    int dayDiff1 = task1.dayOfMonth== -1 ? 0: (abs(currentTime.wDay - task1.dayOfMonth));
//    int dayDiff2 = task2.dayOfMonth == -1 ? 0 : (abs(currentTime.wDay - task2.dayOfMonth));
//    if (dayDiff1 != dayDiff2)
//    {
//        return dayDiff1 < dayDiff2 ? 1 : -1;
//    }
//
//    int hourDiff1 = task1.hour == -1 ? 0 : (hourDiff1 = abs(currentTime.wHour - task1.hour));
//    int hourDiff2 = task2.hour == -1 ? 0 : (abs(currentTime.wHour - task2.hour));
//    if (hourDiff1 != hourDiff2)
//    {
//        return hourDiff1 < hourDiff2 ? 1 : -1;
//    }
//
//    int minuteDiff1 = task1.minute == -1 ? 0 : (abs(currentTime.wMinute - task1.minute));
//    int minuteDiff2 = task2.minute == -1 ? 0 : (abs(currentTime.wMinute - task2.minute));
//    if (minuteDiff1 != minuteDiff2)
//    {
//        return minuteDiff1 < minuteDiff2 ? 1 : -1;
//    }
//    return 0;
//}
bool shouldRunTask(const SYSTEMTIME& currentTime, const Task& task)
{
    if (task.minute != -1 && task.minute != currentTime.wMinute)
        return false;
    if (task.hour != -1 && task.hour != currentTime.wHour)
        return false;
    if (task.dayOfMonth != -1 && task.dayOfMonth != currentTime.wDay)
        return false;
    if (task.month != -1 && task.month != currentTime.wMonth)
        return false;
    //TODO dayOfWeek not work yet
    /*if (task.dayOfWeek != -1 && task.dayOfWeek != currentTime.wDayOfWeek)
        return false;*/

    return true;
}

bool isOneTimeTask(const Task& task) 
{
    if (task.minute == -1) 
        return false;
    if (task.hour == -1) 
        return false;
    if (task.dayOfMonth == -1) 
        return false;
    if (task.month == -1)
        return false;
    //TODO
    //if (task.dayOfWeek == -1) return false;

    return true;
}

void checkAndRunTasks(vector<Task>& tasks) 
{
    SYSTEMTIME currentTime;
    GetLocalTime(&currentTime);

    for (auto& task : tasks)
    {
        if (shouldRunTask(currentTime, task)) 
        {
           /* thread thread(runExternalProgram, task);
            thread.join();*/
            runExternalProgram(task);

            if(isOneTimeTask(task))
            {
                task.shouldRemove = true;
            }
            
        }
    }

    tasks.erase(remove_if(tasks.begin(), tasks.end(), [](const Task& task) 
    {
    return task.shouldRemove;
    }), tasks.end());
}


void infiniteCicle(vector<Task>& tasks)
{
    SYSTEMTIME t1, t2;

    while (true)
    {
        checkAndRunTasks(tasks);
        Sleep(1000 * 5);
    }
}

int main() 
{
    string configFilePath = "schedule.txt";
    vector<Task> tasks = parseConfigFile(configFilePath);
    printTasks(tasks);
    infiniteCicle(tasks);
    //runExternalProgram(tasks[0]);
    return 0;
}