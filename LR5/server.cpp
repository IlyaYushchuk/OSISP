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

// ��������� ��� ��������� � ����
struct ChatMessage 
{
    char author[50];  // ����� ���������
    char text[256];   // ����� ���������
};

// ��������� ��� ���������� � �����������
struct ClientConnection 
{
    SOCKET socket;
    string username;
};

mutex clients_mutex;                     // ������� ��� ������������� ������� � ��������


vector<ClientConnection> active_clients; // ������ �������� ��������

// �������� ��������� ���� ��������, ����� ����������
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

// ��������� �������
void HandleClient(SOCKET client_socket) 
{
    ChatMessage received_message;

    // ��������� ������� ��������� (��� ������������)
    int received_bytes = recv(client_socket, reinterpret_cast<char*>(&received_message), sizeof(received_message), 0);

    if (received_bytes <= 0) 
    {
        closesocket(client_socket);
        return;
    }

    string username = received_message.author;

    // ���������� ������� � ������ ��������
    {
        lock_guard<mutex> lock(clients_mutex);
        active_clients.push_back({ client_socket, username });
        cout << "������������ " << username << " ������������� � ����.\n";
    }

    // �������� ���� ��������� ���������
    while (true) {
        received_bytes = recv(client_socket, reinterpret_cast<char*>(&received_message), sizeof(received_message), 0);
        if (received_bytes <= 0) 
        {
            break;
        }

        // ����������� � �������� ��������� ������ ��������
        cout << received_message.author << ": " << received_message.text << endl;
        BroadcastMessage(received_message, username);
    }

    // �������� ���������� �������
    closesocket(client_socket);

    // �������� ������� �� ������ ��������
    {
        lock_guard<mutex> lock(clients_mutex);

        auto it = remove_if(active_clients.begin(), active_clients.end(),
            [&](const ClientConnection& client) 
            {
                return client.socket == client_socket;
            });

        active_clients.erase(it, active_clients.end());
        cout << "������������ " << username << " ������� ���.\n";
    }
}

int main() {
    // ������������� WinSock
    WSADATA wsa_data;

    WSAStartup(MAKEWORD(2, 2), &wsa_data);

    // �������� ������ ��� �������������
    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);

    // ��������� ������ �������
    sockaddr_in server_address;

    server_address.sin_family = AF_INET;        // IPv4
    server_address.sin_port = htons(SERVER_PORT); // ���� �������
    server_address.sin_addr.s_addr = INADDR_ANY; // ����� ������� � ����� IP

    // �������� ������
    bind(server_socket, reinterpret_cast<sockaddr*>(&server_address), sizeof(server_address));



    // ������������� �������� ����������
    listen(server_socket, SOMAXCONN);

    cout << "������ ���� �������. �������� �����������...\n";

    // �������� ���� �������� ��������
    while (true) 
    {
        SOCKET client_socket = accept(server_socket, nullptr, nullptr);


        thread(HandleClient, client_socket).detach(); // �������� ������ ��� ��������� �������
    }

    // ���������� ������
    closesocket(server_socket);
    WSACleanup();

    return 0;
}
