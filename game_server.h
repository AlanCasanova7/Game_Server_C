#include "dictionary.h"
#include "queue.h"
#include <winsock2.h>
#include <windows.h>

#include <stdio.h>

#define BUFFER_SIZE 4096

#pragma comment(lib, "ws2_32.lib")

typedef struct packet
{
    unsigned int packet_id;
    unsigned int attempts;
    unsigned char try_once;
    float send_after;
    float expires_after;
    char data[BUFFER_SIZE];
    SOCKADDR_IN sender_adress;
} packet_t;

typedef struct game_client
{
    unsigned int malus;
    dictionary_t *ack_table;
    queue_t *send_queue;
    SOCKADDR_IN adress;
} game_client_t;

typedef struct game_server
{
    SOCKET server_socket;
    SOCKADDR_IN server_adress;

    FD_SET write_set;
    FD_SET read_set;
    unsigned int total_sockets;

    LARGE_INTEGER time_frequency;
    LARGE_INTEGER current_time;

    unsigned int number_of_connected_clients; //Unsure if needed
    dictionary_t *connected_clients;

    struct timeval update_frequency;

    unsigned int packet_counter;
    
    dictionary_t* game_objects;
    unsigned int game_object_counter;

    char max_command_table;
    void (**command_table)(struct game_server *game_server, packet_t *packet);
} game_server_t;

typedef struct game_object
{
    float x, y, z;
    game_client_t *owner;
    unsigned int game_object_id;
    //unsigned int interna_type_id;
} game_object_t;

game_client_t *game_client_new(SOCKADDR_IN adress);
game_server_t *game_server_new(int port, int max_connected_sockets, char max_command_table, int max_game_objects, float update_frequency);
void add_command(game_server_t* game_server, char command, void (*func_ptr)(game_server_t * game_server, packet_t *packet));
packet_t *packet_new(game_server_t *game_server, char *data, SOCKADDR_IN current_adress);

int game_server_run(game_server_t *game_server);

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
                    void (*func_ptr)(game_server_t * game_server, packet_t *packet) = game_server->command_table[(int)command];

                    packet_t* current_packet = packet_new(game_server, current_data, current_adress);
                    printf("received command = %d\n", command);
                    if (func_ptr != NULL)
                    {
                        (*game_server->command_table[(int)command])(game_server, current_packet);
                    }
                    else
                    {
                        printf("RECEIVED UNKOWN COMMAND\n");
                    }
                    free(current_packet);
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

static int inline server_internal_process_client(game_server_t *game_server, game_client_t *current_client)
{   
    printf("Processing %d clients\t", game_server->number_of_connected_clients);
    for (int i = 0; i < current_client->send_queue->elements; i++)
    {
        char *data = dequeue(current_client->send_queue);
        //TODO ADD CHECK IF PACKET TIMESTAMP IS OLDER THAN CURRENT TIME
        printf("data %s\n", inet_ntoa(current_client->adress.sin_addr));

        int data_sent = sendto(game_server->server_socket, data, strlen(data), 0, (SOCKADDR *)&current_client->adress, sizeof(current_client->adress));
        printf("Sent %d bytes\n", data_sent);
        if (data_sent == -1)
        {
            printf("Last error: %d\n", WSAGetLastError());
            return 1;
        }
    }
    return 0;
}

static int inline server_internal_process_client_ack_package(game_server_t *game_server, game_client_t *current_client)
{
    printf("Processing ACK\t");
    return 0;
    //TO DO: ACK IMPLEMENTATION.
}