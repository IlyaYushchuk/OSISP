#include "Parsing.h"

Task parseLine(const string& line) {
    Task task;
    istringstream ss(line);

    string minute, hour, dayOfMonth, month, dayOfWeek;

    ss >> minute >> hour >> dayOfMonth >> month >> dayOfWeek;

   
    task.minute = (minute == "*") ? -1 :  stoi(minute);
    task.hour = (hour == "*") ? -1 :  stoi(hour);
    task.dayOfMonth = (dayOfMonth == "*") ? -1 :  stoi(dayOfMonth);
    task.month = (month == "*") ? -1 :  stoi(month);
    task.dayOfWeek = (dayOfWeek == "*") ? -1 :  stoi(dayOfWeek);

   
    ss >> task.program;
     getline(ss, task.args);

    return task;
}


vector<Task> parseConfigFile(const string& filePath) {
    vector<Task> tasks;
    ifstream configFile(filePath);
    string line;

    if (!configFile) {
         cerr << "Не удалось открыть файл: " << filePath <<  endl;
        return tasks;
    }

    int id = 1;
    while ( getline(configFile, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }
        Task task = parseLine(line);
        task.taskId = id++;
        tasks.push_back(task);
    }

    return tasks;
}

void printTasks(const vector<Task>& tasks)
{
    for (const Task& task : tasks) {
        cout << "Task: " << task.program << " " << task.args << endl;
        cout << "Task ID: " << task.taskId << endl;
        cout << "Minute: " << (task.minute == -1 ? "*" : to_string(task.minute)) << endl;
        cout << "Hour: " << (task.hour == -1 ? "*" : to_string(task.hour)) << endl;
        cout << "Day of month: " << (task.dayOfMonth == -1 ? "*" : to_string(task.dayOfMonth)) << endl;
        cout << "Month: " << (task.month == -1 ? "*" : to_string(task.month)) << endl;
        cout << "Day of week : " << (task.dayOfWeek == -1 ? "*" : to_string(task.dayOfWeek)) << endl;
        cout << "-------------------------------" << endl;
    }
}