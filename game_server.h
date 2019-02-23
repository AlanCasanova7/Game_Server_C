#include "dictionary.h"
#include "queue.h"
#include <winsock2.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

typedef struct game_client{
    unsigned int malus;
    dictionary_t* ack_table;
    queue_t* send_queue;
    SOCKADDR_IN current_adress;
} game_client_t;

typedef struct game_server{
    SOCKET server_socket;
    SOCKADDR_IN server_adress;
    
    FD_SET write_set;
    FD_SET read_set;
    unsigned int total_sockets;
    
    LARGE_INTEGER frequency;
    LARGE_INTEGER current_time;

    dictionary_t* command_table;

    unsigned int number_of_connected_clients; //Unsure if needed
    dictionary_t* connected_clients;
} game_server_t;

game_client_t* game_client_new();
game_server_t* game_server_new(int port, int max_connected_sockets, int max_command_table);

