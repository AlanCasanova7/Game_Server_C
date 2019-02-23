#include "game_server.h"
#include <stdio.h>

game_server_t* game_server_new(int port, int max_connected_sockets, int max_command_table){
    game_server_t* to_return = malloc(sizeof(game_server_t));
    memset(to_return, 0, sizeof(game_server_t));
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)
    {
        printf("WSAStartup() failed with error %d\n", WSAGetLastError());
        WSACleanup();
        return NULL;
    }
    else
    {
        printf("WSAStartup() is OK!\n");
    }

    to_return->server_socket = WSASocketW(AF_INET, SOCK_DGRAM,
                                     IPPROTO_UDP, NULL, 0,
                                     WSA_FLAG_OVERLAPPED);
    if (to_return->server_socket == INVALID_SOCKET)
    {
        printf("WSASocketW() failed with error %d\n", WSAGetLastError());
        return NULL;
    }
    else
    {
        printf("WSASocketW() is OK!\n");
    }

    unsigned long non_blocking = 1;
    if (ioctlsocket(to_return->server_socket, FIONBIO, &non_blocking) == SOCKET_ERROR)
    {
        printf("ioctlsocket() failed with error %d\n", WSAGetLastError());
        return NULL;
    }
    else
    {
        printf("ioctlsocket() is OK!\n");
    }

    to_return->server_adress.sin_family = AF_INET;
    to_return->server_adress.sin_addr.s_addr = htonl(INADDR_ANY);
    to_return->server_adress.sin_port = htons(port);

    if (bind(to_return->server_socket, (PSOCKADDR)&to_return->server_adress, sizeof(to_return->server_adress)) == SOCKET_ERROR)
    {
        printf("bind() failed with error %d\n", WSAGetLastError());
        return NULL;
    }
    else
    {
        printf("bind() is OK!\n");
    }

    to_return->connected_clients = new_dictionary(max_connected_sockets);
    to_return->command_table = new_dictionary(max_command_table);

    QueryPerformanceFrequency(&to_return->frequency);

    return to_return;
}