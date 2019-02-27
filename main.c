#include "game_server.h"

#define SERVER_CMD_HELLO_WORLD 0
#define SERVER_CMD_JOIN 1
#define SERVER_CMD_ACK 2
#define SERVER_CMD_UPDATE 3

#define CLIENT_CMD_WELCOME 0
#define CLIENT_CMD_SPAWN_OBJ 1

void cmd_hello_world(game_server_t *game_server, packet_t *packet);
void cmd_join(game_server_t *game_server, packet_t *packet);
void cmd_ack(game_server_t *game_server, packet_t *packet);
void cmd_update(game_server_t *game_server, packet_t *packet);

int main(int argc, char **argv)
{
    game_server_t *server = game_server_new(9999, 10, 10, 10, 2.0);

    add_command(server, SERVER_CMD_HELLO_WORLD, cmd_hello_world);

    add_command(server, SERVER_CMD_JOIN, cmd_join);

    add_command(server, SERVER_CMD_ACK, cmd_ack);

    add_command(server, SERVER_CMD_UPDATE, cmd_update);

    for (;;)
    {
        game_server_run(server);
    }

    return 1;
}

void cmd_hello_world(game_server_t *game_server, packet_t *packet)
{
    printf("HELLO WORLD!\n");
}

void cmd_join(game_server_t *game_server, packet_t *packet)
{
    game_client_t *new_client = game_client_new(packet->sender_adress);
    printf("adress %s has been created\n", inet_ntoa(new_client->adress.sin_addr));
    game_client_t *bad_client = (game_client_t *)get_value(game_server->connected_clients, (void *)&new_client->adress.sin_addr, sizeof(new_client->adress.sin_addr));

    if (bad_client != NULL)
    {
        bad_client->malus += 1;
        printf("Client already joined! Increased malus to: %d\n", bad_client->malus);
        free(new_client);
    }
    else
    {
        key_value_t *connected_client_kv = new_key_value((void *)&new_client->adress.sin_addr, (void *)new_client, sizeof(new_client->adress.sin_addr));
        register_key_value(game_server->connected_clients, connected_client_kv);
        game_server->number_of_connected_clients += 1;

        game_object_t *avatar = game_object_new(game_server); //NOT SURE IF I SHOULD LET THE USER PUT THE ID
                                                              //MANUALLY OR GIVE IT AUTOMATICALLY IN THE NEW
        avatar->owner = new_client;

        printf("here\n");
        char data[sizeof(char) + sizeof(char) + sizeof(unsigned int) + (sizeof(float) * 3)];
        data[0] = CLIENT_CMD_WELCOME;
        data[1] = 0; // useless
        memcpy(&data[2], &avatar->game_object_id, sizeof(unsigned int));
        memcpy(&data[6], &avatar->x, sizeof(float));
        memcpy(&data[10], &avatar->y, sizeof(float));
        memcpy(&data[14], &avatar->z, sizeof(float));

        unsigned int rec_id;
        float rec_x, rec_y, rec_z;

        memcpy(&rec_id, &data[2], sizeof(unsigned int));
        memcpy(&rec_x, &data[6], sizeof(float));
        memcpy(&rec_y, &data[10], sizeof(float));
        memcpy(&rec_z, &data[14], sizeof(float));
        printf("Created data buffer with the following data:\ndata 0 = %d, data 1 = %d, id = %u, x = %f, y = %f, z = %f\n", (char)data[0], (char)data[1], rec_id, rec_x, rec_y, rec_z);

        packet_t *welcome_pkt = packet_new(game_server, data, new_client->adress, sizeof(data));
        welcome_pkt->need_ack = 1;
        enqueue(new_client->send_queue, welcome_pkt);

        key_value_t *current_game_object_kv = game_server->game_objects->first_entry;
        while (current_game_object_kv != NULL)
        {
            game_object_t *game_obj = current_game_object_kv->value;
            char data_to_spw_obj[sizeof(char) + (sizeof(unsigned int) * 2) + (sizeof(float) * 3)];
            data[0] = CLIENT_CMD_SPAWN_OBJ;
            memcpy(&data_to_spw_obj[2], &game_obj->game_object_id, sizeof(unsigned int));
            memcpy(&data_to_spw_obj[6], &game_obj->game_object_client_type, sizeof(unsigned int));
            memcpy(&data_to_spw_obj[10], &game_obj->x, sizeof(float));
            memcpy(&data_to_spw_obj[14], &game_obj->y, sizeof(float));
            memcpy(&data_to_spw_obj[18], &game_obj->z, sizeof(float));

            packet_t *spawn_pkt = packet_new(game_server, data_to_spw_obj, new_client->adress, sizeof(data_to_spw_obj));
            spawn_pkt->need_ack = 1;
            enqueue(new_client->send_queue, spawn_pkt);

            current_game_object_kv = current_game_object_kv->next_dict_entry;
        }

        key_value_t *game_object_kv = new_key_value((void *)&avatar->game_object_id, avatar, sizeof(avatar->game_object_id));
        register_key_value(game_server->game_objects, game_object_kv);

        key_value_t *current_connected_client_kv = game_server->connected_clients->first_entry;
        while (current_connected_client_kv != NULL)
        {
            if (memcmp((game_client_t *)current_connected_client_kv->value, new_client, sizeof(game_client_t)))
            {
                game_client_t *client_to_spawn_yourself = (game_client_t *)current_connected_client_kv->value;
                char data_to_spw_self[sizeof(char) + (sizeof(unsigned int) * 2) + (sizeof(float) * 3)];
                data[0] = CLIENT_CMD_SPAWN_OBJ;
                memcpy(&data_to_spw_self[2], &avatar->game_object_id, sizeof(unsigned int));
                memcpy(&data_to_spw_self[6], &avatar->game_object_client_type, sizeof(unsigned int));
                memcpy(&data_to_spw_self[10], &avatar->x, sizeof(float));
                memcpy(&data_to_spw_self[14], &avatar->y, sizeof(float));
                memcpy(&data_to_spw_self[18], &avatar->z, sizeof(float));

                packet_t *spawn_yourself_pkt = packet_new(game_server, data_to_spw_self, new_client->adress, sizeof(data_to_spw_self));
                spawn_yourself_pkt->need_ack = 1;
                enqueue(client_to_spawn_yourself->send_queue, spawn_yourself_pkt);
            }

            current_connected_client_kv = current_connected_client_kv->next_dict_entry;
        }
    }
}

void cmd_ack(game_server_t *game_server, packet_t *packet)
{
    IN_ADDR sender_adress = packet->sender_adress.sin_addr;
    game_client_t *retrieved_client = get_value(game_server->connected_clients, (void *)&sender_adress, sizeof(sender_adress));
    if (retrieved_client != NULL)
    {
        if (get_key_value(retrieved_client->ack_table, (void *)&packet->data[1], sizeof(packet->data[1])))
        {
            remove_key_value(retrieved_client->ack_table, (void *)&packet->data[1], sizeof(packet->data[1]));
        }
    }
}

void cmd_update(game_server_t *game_server, packet_t *packet)
{
    IN_ADDR sender_adress = packet->sender_adress.sin_addr;
    game_client_t *retrieved_client = get_value(game_server->connected_clients, (void *)&sender_adress, sizeof(sender_adress));
    if (retrieved_client != NULL)
    {
        printf("\n\n\nFOUND CLIENT\n");

        unsigned int game_object_id = -1;
        memcpy(&game_object_id, &packet->data[2], sizeof(unsigned int));
        game_object_t *retrieved_game_object = get_value(game_server->game_objects, (void *)&game_object_id, sizeof(unsigned int));
        if (retrieved_game_object != NULL && !memcmp(retrieved_game_object->owner, retrieved_client, sizeof(game_client_t)))
        {
            printf("\nFOUND GAMEOBJECT: %d\n", game_object_id);

            float new_x, new_y, new_z;
            memcpy(&new_x, &packet->data[5], sizeof(float));
            memcpy(&new_y, &packet->data[9], sizeof(float));
            memcpy(&new_z, &packet->data[13], sizeof(float));

            retrieved_game_object->x = new_x;
            retrieved_game_object->y = new_y;
            retrieved_game_object->z = new_z;
        }
    }
}
