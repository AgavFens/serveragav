#define WIN32_LEAN_AND_MEAN // Исключает редко используемые компоненты из заголовков Windows

#include <Windows.h>
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>

using namespace std;

int main()
{
    WSADATA wsaData;  // Структура для хранения данных о реализации Windows Sockets
    ADDRINFO hints;   // Подсказки для типа адреса сокета, который мы ищем
    ADDRINFO* addrResult;  // Указатель для хранения результата функции getaddrinfo
    SOCKET ConnectSocket = INVALID_SOCKET;  // Сокет для подключения к клиенту
    SOCKET ListenSocket = INVALID_SOCKET;  // Сокет для прослушивания входящих соединений
    char recvBuffer[512];  // Буфер для получения данных

    const char* sendBuffer = "Hello from server";  // Сообщение для отправки клиенту

    // Инициализация библиотеки Windows Sockets
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        cout << "WSAStartup не удалось, результат: " << result << endl;
        return 1;
    }

    // Обнуление структуры hints
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET; // Семейство адресов IPv4
    hints.ai_socktype = SOCK_STREAM; // Тип сокета - потоковый (TCP)
    hints.ai_protocol = IPPROTO_TCP; // Протокол - TCP
    hints.ai_flags = AI_PASSIVE; // Сервер - пассивный, ожидает входящих соединений

    // Получение информации о сетевом адресе и порте
    result = getaddrinfo(NULL, "666", &hints, &addrResult);
    if (result != 0) {
        cout << "getaddrinfo не удалось, ошибка: " << result << endl;
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    // Создание сокета для прослушивания
    ListenSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        cout << "Ошибка создания сокета" << endl;
        freeaddrinfo(addrResult);
        return 1;
    }

    // Привязка сокета к адресу
    result = bind(ListenSocket, addrResult->ai_addr, (int)addrResult->ai_addrlen);
    if (result == SOCKET_ERROR) {
        cout << "Не удалось привязать сокет к адресу" << endl;
        closesocket(ListenSocket);
        ListenSocket = INVALID_SOCKET;
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    // Прослушивание входящих соединений
    result = listen(ListenSocket, SOMAXCONN); // Максимальное количество подключений
    if (result == SOCKET_ERROR) {
        cout << "Ошибка прослушивания сокета: " << result << endl;
        closesocket(ListenSocket);
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    // Ожидание и принятие входящего соединения
    ConnectSocket = accept(ListenSocket, NULL, NULL);
    if (ConnectSocket == SOCKET_ERROR) {
        cout << "Ошибка принятия соединения: " << result << endl;
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    // Цикл для приема и отправки данных
    do {
        ZeroMemory(recvBuffer, 512);  // Обнуление буфера для приема данных
        result = recv(ConnectSocket, recvBuffer, 512, 0);  // Получение данных от клиента

        if (result > 0) {
            cout << "Получено: " << result << " байт" << endl;
            cout << "Полученные данные: " << recvBuffer << endl;

            // Отправка данных клиенту
            result = send(ConnectSocket, sendBuffer, (int)strlen(sendBuffer), 0);
            if (result == SOCKET_ERROR) {
                cout << "Ошибка отправки данных: " << result << endl;
                closesocket(ListenSocket);
                freeaddrinfo(addrResult);
                WSACleanup();
                return 1;
            }
        }
        else if (result == 0) {
            cout << "Соединение закрыто" << endl;
        }
        else {
            cout << "Ошибка получения данных" << endl;
            closesocket(ConnectSocket);
        }

    } while (result > 0);

    // Завершение работы сокета для отправки данных
    result = shutdown(ConnectSocket, SD_SEND);
    if (result == SOCKET_ERROR) {
        cout << "Ошибка завершения работы сокета: " << result << endl;
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(addrResult);  // Освобождение памяти, выделенной для addrResult
    WSACleanup();  // Освобождение ресурсов, используемых Winsock
    return 0;
}
