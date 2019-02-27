#include "game_server.h"

game_server_t *game_server_new(int port, int max_connected_sockets, char max_command_table, int max_game_objects, float update_frequency)
{
    game_server_t *to_return = malloc(sizeof(game_server_t));
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
    to_return->game_objects = new_dictionary(max_game_objects);

    to_return->max_command_table = max_command_table;
    to_return->command_table = malloc(sizeof(void (*)(game_server_t *)) * max_command_table);
    for (int i = 0; i < max_command_table; i++)
    {
        to_return->command_table[i] = NULL;
    }

    QueryPerformanceFrequency(&to_return->time_frequency);

    to_return->update_frequency.tv_sec = update_frequency;
    return to_return;
}

game_client_t *game_client_new(SOCKADDR_IN adress)
{
    game_client_t *to_return = malloc(sizeof(game_client_t));
    memset(to_return, 0, sizeof(game_client_t));

    to_return->ack_table = new_dictionary(100);
    to_return->send_queue = new_queue();

    to_return->adress = adress;

    return to_return;
}

game_object_t *game_object_new(game_server_t *game_server)
{
    game_object_t *to_return = malloc(sizeof(game_object_t));
    memset(to_return, 0, sizeof(game_object_t));

    to_return->game_object_id = game_server->game_object_counter; //game_server->game_object_counter;
    game_server->game_object_counter += 1;
    return to_return;
}

int game_server_run(game_server_t *game_server)
{
    QueryPerformanceCounter(&game_server->current_time);
    float time_stamp = game_server->current_time.QuadPart / game_server->time_frequency.QuadPart;
    printf("tick, timestamp: %f\n", time_stamp);

    FD_ZERO(&game_server->write_set);
    FD_ZERO(&game_server->read_set);

    FD_SET(game_server->server_socket, &game_server->read_set);

    int error = 0;

    error = server_internal_select(game_server);

    key_value_t *current_entry = (key_value_t *)game_server->connected_clients->first_entry;
    while (current_entry != NULL)
    {
        error = server_internal_process_client(game_server, (game_client_t *)current_entry->value);

        error = server_internal_process_client_ack_package(game_server, (game_client_t *)current_entry->value);

        current_entry = current_entry->next_dict_entry;
    }

    current_entry = (key_value_t*)game_server->game_objects->first_entry;
    while (current_entry != NULL)
    {
        error = server_internal_tick_game_object(game_server, (game_object_t *)current_entry->value);

        current_entry = current_entry->next_dict_entry;
    }

    return error;
}

packet_t *packet_new(game_server_t *game_server, char *data, SOCKADDR_IN sender_adress, size_t packet_size)
{
    //TODO DUE TO THIS IMPLEMENTATION THE SERVER WILL ALWAYS SEND 4096 Bytes

    packet_t *to_return = malloc(sizeof(packet_t));
    memset(to_return, 0, sizeof(packet_t));

    memcpy(to_return->data, data, packet_size); 

    to_return->sender_adress = sender_adress;
    to_return->packet_id = game_server->packet_counter;
    game_server->packet_counter += 1;

    return to_return;
}

void add_command(game_server_t *game_server, char command, void (*func_ptr)(game_server_t *game_server, packet_t *packet))
{
    game_server->command_table[(int)command] = func_ptr;
}