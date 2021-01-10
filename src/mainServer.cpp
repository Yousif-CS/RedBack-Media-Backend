#include "config.h"
#include "server.h"

int main()
{
	readConfig(CONFIG_PATH);
	
	std::shared_ptr<Server> server = std::make_shared<Server>(port);
	server->start();

	while (true)
	{
		server->update(5, true);
	}

	return 0;
}
