#include "game_server.h"

void hello_world(game_server_t* game_server);

int main(int argc, char **argv)
{
    game_server_t *server = game_server_new(9999, 10, 10, 10, 2.0);

    char hello_world_cmd = 1;
    add_command(server, hello_world_cmd, hello_world);

    for (;;)
    {
        game_server_run(server);
    }

    return 1;
}

void hello_world(game_server_t* game_server){
    printf("HELLO WORLD!\n");
}