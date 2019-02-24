#include "game_server.h"

void cmd_hello_world(game_server_t* game_server, packet_t* packet);
void cmd_join(game_server_t* game_server, packet_t* packet);

int main(int argc, char **argv)
{
    game_server_t *server = game_server_new(9999, 10, 10, 10, 2.0);

    add_command(server, 0, cmd_hello_world);
    
    add_command(server, 1, cmd_join);

    for (;;)
    {
        game_server_run(server);
    }

    return 1;
}

void cmd_hello_world(game_server_t* game_server, packet_t* packet){
    printf("HELLO WORLD!\n");
}

void cmd_join(game_server_t* game_server, packet_t* packet)
{
    game_client_t* new_client = game_client_new(packet->sender_adress);
    game_client_t* bad_client = (game_client_t*)get_value(game_server->connected_clients, (void*)&new_client->adress.sin_addr, sizeof(new_client->adress.sin_addr));

    if(bad_client != NULL)
    {
        bad_client->malus += 1;
        printf("Client already joined! Increased malus to: %d\n", bad_client->malus);
        free(new_client);
    } 
    else
    {
        key_value_t *connected_client_id = new_key_value((void *)&new_client->adress.sin_addr, (void *)new_client, sizeof(new_client->adress.sin_addr));
        register_key_value(game_server->connected_clients, connected_client_id);
        game_server->number_of_connected_clients += 1;    
    }

}