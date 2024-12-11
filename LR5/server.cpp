#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <algorithm>

#pragma comment(lib, "ws2_32.lib")

#define SERVER_PORT 8000

using namespace std;

// Структура для сообщения в чате
struct ChatMessage 
{
    char author[50];  // Автор сообщения
    char text[256];   // Текст сообщения
};

// Структура для информации о подключении
struct ClientConnection 
{
    SOCKET socket;
    string username;
};

mutex clients_mutex;                     // Мьютекс для синхронизации доступа к клиентам


vector<ClientConnection> active_clients; // Список активных клиентов

// Отправка сообщения всем клиентам, кроме указанного
void BroadcastMessage(const ChatMessage& message, const string& excluded_username) 
{
    lock_guard<mutex> lock(clients_mutex);

    for (const auto& client : active_clients) 
    {
        if (client.username != excluded_username) 
        {
            send(client.socket, reinterpret_cast<const char*>(&message), sizeof(message), 0);
        }
    }
}

// Обработка клиента
void HandleClient(SOCKET client_socket) 
{
    ChatMessage received_message;

    // Получение первого сообщения (имя пользователя)
    int received_bytes = recv(client_socket, reinterpret_cast<char*>(&received_message), sizeof(received_message), 0);

    if (received_bytes <= 0) 
    {
        closesocket(client_socket);
        return;
    }

    string username = received_message.author;

    // Добавление клиента в список активных
    {
        lock_guard<mutex> lock(clients_mutex);
        active_clients.push_back({ client_socket, username });
        cout << "Пользователь " << username << " присоединился к чату.\n";
    }

    // Основной цикл обработки сообщений
    while (true) {
        received_bytes = recv(client_socket, reinterpret_cast<char*>(&received_message), sizeof(received_message), 0);
        if (received_bytes <= 0) 
        {
            break;
        }

        // Логирование и отправка сообщения другим клиентам
        cout << received_message.author << ": " << received_message.text << endl;
        BroadcastMessage(received_message, username);
    }

    // Закрытие соединения клиента
    closesocket(client_socket);

    // Удаление клиента из списка активных
    {
        lock_guard<mutex> lock(clients_mutex);

        auto it = remove_if(active_clients.begin(), active_clients.end(),
            [&](const ClientConnection& client) 
            {
                return client.socket == client_socket;
            });

        active_clients.erase(it, active_clients.end());
        cout << "Пользователь " << username << " покинул чат.\n";
    }
}

int main() {
    // Инициализация WinSock
    WSADATA wsa_data;

    WSAStartup(MAKEWORD(2, 2), &wsa_data);

    // Создание сокета для прослушивания
    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);

    // Настройка адреса сервера
    sockaddr_in server_address;

    server_address.sin_family = AF_INET;        // IPv4
    server_address.sin_port = htons(SERVER_PORT); // Порт сервера
    server_address.sin_addr.s_addr = INADDR_ANY; // Прием трафика с любых IP

    // Привязка сокета
    bind(server_socket, reinterpret_cast<sockaddr*>(&server_address), sizeof(server_address));



    // Прослушивание входящих соединений
    listen(server_socket, SOMAXCONN);

    cout << "Сервер чата запущен. Ожидание подключений...\n";

    // Основной цикл ожидания клиентов
    while (true) 
    {
        SOCKET client_socket = accept(server_socket, nullptr, nullptr);


        thread(HandleClient, client_socket).detach(); // Создание потока для обработки клиента
    }

    // Завершение работы
    closesocket(server_socket);
    WSACleanup();

    return 0;
}
