#include <iostream>
#include <fstream>
#include <sstream>


// Global variables: I know its bad but it is what it is
uint16_t port = 60000;
std::string host = "";


std::string replace(std::string s, char old, char n)
{
        //replace '=' with ' ' to read using standard notation
        for (auto it = s.begin(); it != s.end(); it++)
        {
            if ((*it) == old)
            {
                s.replace(it, it+1, std::string(1, n));
            }
        }

        return s;
}

/**
 * Get the port number from the config file
*/
void readConfig(std::string path)
{
    std::ifstream input{path};

    if (input.good())
    {
        std::string line;
        std::string discard;

        // get the port
        std::getline(input, line);
        line = replace(line, '=', ' ');
        std::stringstream ss{line};
        ss >> discard >> port;

        std::getline(input, line);
        line = replace(line, '=', ' ');
        ss = std::stringstream(line);

        ss >> discard >> host;
    }
}