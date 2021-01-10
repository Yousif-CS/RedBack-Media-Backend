#include "config.h"
#include "client.h"



int main()
{
    Client client;

    client.connect(host, port);
    
    // Keep the client running, updating messages
    while(true)
    {
        client.update(5, true);
    }

    return 0;
}