#include "game_server.h"

int main (int argc, char** argv){
    game_server_t* server = game_server_new(9999, 10, 10);

    return 1;
}