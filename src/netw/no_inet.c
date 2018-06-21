#include "sock.h"

struct player_id *playerlist = (void*)0;
int playerlist_n = 0;

void request_playerlist() {}
void connect_to_player(struct player_id *p) {}
int reconnect_server() { return 0; }
int handle_server_message() { return 0; }

int reconnect_inet() { return 0; }
int is_inet() { return 0; }
