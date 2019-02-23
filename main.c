#include "game_server.h"

int main (int argc, char** argv){
    game_server_t* server = game_server_new(9999, 10, 10, 2.0);

    for (;;)
    {
        game_server_run(server);
    }

    return 1;
}