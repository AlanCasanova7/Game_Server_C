#include "dictionary.h"
#include "queue.h"
#include <winsock2.h>
#include <windows.h>

#include <stdio.h>

#define BUFFER_SIZE 4096

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
    
    LARGE_INTEGER time_frequency;
    LARGE_INTEGER current_time;

    dictionary_t* command_table;

    unsigned int number_of_connected_clients; //Unsure if needed
    dictionary_t* connected_clients;

    struct timeval update_frequency;
} game_server_t;

game_client_t* game_client_new();
game_server_t* game_server_new(int port, int max_connected_sockets, int max_command_table, float update_frequency);
int game_server_run(game_server_t* game_server);

static int inline server_internal_select(game_server_t *game_server)
{
    int total_sockets = select(0, &game_server->read_set, &game_server->write_set, NULL, &game_server->update_frequency);
    if (total_sockets > 0)
    {
        printf("SELECT is OK!\n");
        char current_data[BUFFER_SIZE];
        memset(current_data, 0, BUFFER_SIZE);
        SOCKADDR_IN current_adress;
        int adress_len = sizeof(struct sockaddr);
        for (int i = 0; i < total_sockets; i++)
        {
            int byte_received = recvfrom(game_server->server_socket, current_data, BUFFER_SIZE, 0, (SOCKADDR *)&current_adress, &adress_len);
            if (byte_received != -1)
            {
                printf("RECEIVE is OK!\n");
                if (byte_received > 0)
                {
                    char command = current_data[0];
                    void *func_ptr = get_value(game_server->command_table, (void*)&command, sizeof(char));
                    printf("%d\n", command);               
                    if (func_ptr != NULL)
                    {
                        void (*func_ptr)(game_server_t* game_server) = NULL;
                        (*func_ptr)(game_server);
                    }
                    else
                    {
                        printf("RECEIVED UNKOWN COMMAND\n");
                    }
                }
                else
                {
                    printf("RECEIVE is OK! BUT the is EMPTY!\n");
                }
                memset(current_data, 0, BUFFER_SIZE);
            }
        }
    }
    else if (total_sockets == 0)
    {
        printf("No Sockets connected!\t");
    }
    else
    {
        printf("SELECT() returned with error %d\n", WSAGetLastError());
        return 1;
    }
    return 0;
}

static int inline server_internal_process_client(game_server_t* game_server, game_client_t* current_client){
    for (int i = 0; i < current_client->send_queue->elements; i++)
    {
        char *data = dequeue(current_client->send_queue);
        //TODO ADD CHECK IF PACKET TIMESTAMP IS OLDER THAN CURRENT TIME
        printf("data %s\n", inet_ntoa(current_client->current_adress.sin_addr));

        int data_sent = sendto(game_server->server_socket, data, strlen(data), 0, (SOCKADDR *)&current_client->current_adress, sizeof(current_client->current_adress));
        printf("Sent %d bytes\n", data_sent);
        if (data_sent == -1)
        {
            printf("Last error: %d\n", WSAGetLastError());
            return 1;
        }
    }
    return 0;
}

static int inline server_internal_process_client_ack_package(game_server_t* game_server, game_client_t* current_client){
    printf("Processing ACK");
    return 0;
    //TO DO: ACK IMPLEMENTATION.
}