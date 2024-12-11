#include <windows.h>
#include <iostream>
#include <string>
#include <vector>

// Флаг для включения поиска значений внутри ключей
bool searchValuesInKeys = false;

// Функция для вывода значения реестра
void PrintRegistryValue(HKEY hKey, const std::string& subKey, const std::string& valueName, DWORD valueType, const BYTE* valueData, DWORD dataSize) {
    std::cout << subKey << "\n";
    std::cout << "\"" << valueName << "\"=";

    // Обработка различных типов данных
    if (valueType == REG_SZ) {
        std::cout << "\"" << reinterpret_cast<const char*>(valueData) << "\"";
    }
    else if (valueType == REG_DWORD) {
        DWORD dwordValue = *reinterpret_cast<const DWORD*>(valueData);
        std::cout << "dword:" << std::hex << dwordValue;
    }
    else if (valueType == REG_BINARY) {
        std::cout << "hex:";
        for (DWORD i = 0; i < dataSize; ++i) {
            printf("%02x", valueData[i]);
            if (i < dataSize - 1) {
                std::cout << ",";
            }
        }
    }
    else {
        std::cout << "Unsupported value type";
    }

    std::cout << "\n\n";
}

// Функция для поиска значения в реестре
void SearchRegistryValue(HKEY rootKey, const std::string& subKey, const std::string& searchValueName, const std::string& searchValueData = "", DWORD searchValueDWORD = 0) {
    HKEY hKey;

    // Открытие ключа реестра
    if (RegOpenKeyExA(rootKey, subKey.c_str(), 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
        return;
    }

    DWORD index = 0;
    char valueName[256];
    DWORD valueNameSize;
    BYTE valueData[512];
    DWORD dataSize;
    DWORD valueType;

    // Перебор всех значений в указанном ключе
    while (true) {
        valueNameSize = sizeof(valueName);
        dataSize = sizeof(valueData);

        LONG result = RegEnumValueA(hKey, index, valueName, &valueNameSize, nullptr, &valueType, valueData, &dataSize);

        if (result == ERROR_NO_MORE_ITEMS) {
            break;
        }
        else if (result == ERROR_SUCCESS) {
            std::string currentValueName(valueName);

            // Проверка совпадения имени значения
            bool isNameMatch = searchValueName.empty() || currentValueName.find(searchValueName) != std::string::npos;

            // Проверка совпадения данных значения
            bool isDataMatch = true;

            if (valueType == REG_SZ && !searchValueData.empty()) {
                std::string stringValue(reinterpret_cast<const char*>(valueData), dataSize - 1);
                isDataMatch = stringValue.find(searchValueData) != std::string::npos;
            }
            else if (valueType == REG_DWORD && searchValueDWORD != 0) {
                DWORD dwordValue = *reinterpret_cast<const DWORD*>(valueData);
                isDataMatch = (dwordValue == searchValueDWORD);
            }
            else if (valueType == REG_BINARY && !searchValueData.empty()) {
                std::string hexData;
                for (DWORD i = 0; i < dataSize; ++i) {
                    char hexByte[3];
                    sprintf_s(hexByte, "%02x", valueData[i]);
                    hexData += hexByte;
                }
                isDataMatch = hexData.find(searchValueData) != std::string::npos;
            }

            if (isNameMatch && isDataMatch) {
                PrintRegistryValue(hKey, subKey, currentValueName, valueType, valueData, dataSize);
            }
        }

        ++index;
    }

    RegCloseKey(hKey);
}

// Функция для поиска ключей в реестре
void SearchRegistryKey(HKEY rootKey, const std::string& subKey, const std::string& searchKeyName) {
    HKEY hKey;

    // Открытие указанного ключа
    if (RegOpenKeyExA(rootKey, subKey.c_str(), 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
        return;
    }

    char keyName[256];
    DWORD keyNameSize;
    DWORD index = 0;

    // Поиск значений внутри ключа, если включён флаг
    if (searchValuesInKeys) {
        SearchRegistryValue(rootKey, subKey, "");
    }

    // Перебор всех дочерних ключей
    while (true) {
        keyNameSize = sizeof(keyName);
        LONG result = RegEnumKeyExA(hKey, index, keyName, &keyNameSize, nullptr, nullptr, nullptr, nullptr);

        if (result == ERROR_NO_MORE_ITEMS) {
            break;
        }
        else if (result == ERROR_SUCCESS) {
            std::string currentKeyPath = subKey + "\\" + keyName;

            // Проверка совпадения имени ключа
            if (searchKeyName.empty() || currentKeyPath.find(searchKeyName) != std::string::npos) {
                std::cout  << currentKeyPath << "\n\n";
            }

            // Рекурсивный вызов для дочерних ключей
            SearchRegistryKey(rootKey, currentKeyPath, searchKeyName);
        }

        ++index;
    }

    RegCloseKey(hKey);
}

int main() {
    // Флаг для ввода через GUI
    bool useGui = false;

    // Ключ для поиска
    std::string searchKey = "Control Panel\\Desktop";
    if (useGui) {
        std::cout << "Enter key";
        std::getline(std::cin, searchKey);
    }

    // Имя значения для поиска
    std::string searchValueName = "WallPaper";
    if (useGui) {
        std::cout << "Enter value name";
        std::getline(std::cin, searchValueName);
    }

    // Данные значения для поиска
    std::string searchValueData = "";
    if (useGui) {
        std::cout << "Enter value";
        std::getline(std::cin, searchValueData);
    }

    DWORD searchValueDWORD = 0;

    std::cout << "Ключ\n";
    SearchRegistryKey(HKEY_CURRENT_USER, searchKey, searchKey);

    std::cout << "Value\n";
    SearchRegistryValue(HKEY_CURRENT_USER, searchKey, searchValueName, searchValueData, searchValueDWORD);

    return 0;
}
