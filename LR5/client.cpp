#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <iostream>
#include <thread>
#include <string>

#pragma comment(lib, "ws2_32.lib")

#define SERVER_IP "127.0.0.1"    // IP-адрес сервера
#define SERVER_PORT 8000         // Порт сервера

// Структура для представления сообщений чата
struct ChatMessage 
{
    char username[50];        // Имя пользователя
    char message_text[256];   // Текст сообщения
};

// Функция для прослушивания входящих сообщений
void ReceiveMessages(SOCKET connection_socket) 
{
    ChatMessage received_message;

    while (true) 
    {
        // Получение сообщения от сервера
        int received_bytes = recv(connection_socket, reinterpret_cast<char*>(&received_message), sizeof(received_message), 0);
        if (received_bytes > 0) 
        {
            std::cout << "\n" << received_message.username << ": " << received_message.message_text
                << std::endl << "Введите сообщение: ";
        }
        else 
        {
            // Завершение при ошибке получения
            break;
        }
    }
}

int main() {
    // Инициализация WinSock
    WSADATA wsa_data;
    WSAStartup(MAKEWORD(2, 2), &wsa_data);

    // Создание клиентского сокета
    SOCKET client_socket = socket(AF_INET, SOCK_STREAM, 0);

    // Настройка адреса сервера
    sockaddr_in server_address;
    server_address.sin_family = AF_INET;          // IPv4
    server_address.sin_port = htons(SERVER_PORT); // Порт сервера

    // Преобразование IP-адреса из строки в двоичный формат
    wchar_t wide_server_ip[16];
    MultiByteToWideChar(CP_UTF8, 0, SERVER_IP, -1, wide_server_ip, 16);
    InetPton(AF_INET, wide_server_ip, &server_address.sin_addr);

    // Подключение к серверу
    connect(client_socket, reinterpret_cast<sockaddr*>(&server_address), sizeof(server_address));

    // Ввод имени пользователя
    ChatMessage chat_message;
    std::cout << "Введите имя: ";
    std::cin.getline(chat_message.username, 50);

    // Отправка имени пользователя серверу
    send(client_socket, reinterpret_cast<char*>(&chat_message), sizeof(chat_message), 0);

    // Запуск потока для получения сообщений
    std::thread message_receiver(ReceiveMessages, client_socket);
    message_receiver.detach();

    // Основной цикл отправки сообщений
    while (true) 
    {
        std::cout << "Введите сообщение: ";
        std::cin.getline(chat_message.message_text, 256);

        // Отправка сообщения на сервер
        send(client_socket, reinterpret_cast<char*>(&chat_message), sizeof(chat_message), 0);
    }

    // Закрытие сокета
    closesocket(client_socket);

    // Завершение работы WinSock
    WSACleanup();

    return 0;
}
