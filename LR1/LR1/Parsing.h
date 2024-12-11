#pragma once
#include <windows.h>
#include <commdlg.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <ctime>
#include <algorithm>

using namespace std;

struct Task {
    int taskId;
    int minute;
    int hour;
    int dayOfMonth;
    int month;
    int dayOfWeek;
    bool shouldRemove = false;
    string program;
    string args;
};

Task parseLine(const string& line);
vector<Task> parseConfigFile(const string& filePath);
void printTasks(const vector<Task>& tasks);