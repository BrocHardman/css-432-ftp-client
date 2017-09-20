//Conard James B. Faraon
//CSS 432 Final Project: Network Application
//May 13, 2017

//Purpose- An Unix-like FTP client program based on the Internet Engineering Task Force RFC 959 protocol.

//The program may be started using the following:
//1) ./ftp <hostname>
//2) ./ftp <hostname> <port>
//3) ./ftp

#include "Input.h"
#include "FTP.h"

using namespace std;

const static int DEFAULT_PORT = 21;
const static int IN_BUFFER = 256;
const static string PROMPT = "ftp> ";
const static string HEADER = "================================================================";
const static string EXIT_MSG = "FTP Program Exit!";
const static string USAGE = "usage: open host-name [port]";
const static string EMPTY = "";

int main(int argc, char *argv[])
{
    char *hostname = NULL;

    //buffer for user input
    char cmd[IN_BUFFER];
    int port = DEFAULT_PORT;

    //for user input processing
    Input input;
    FTP *client = NULL;

    if (argc > 3)
    {
        cerr << "usage: ./ftp <hostname>" << endl;
        cerr << "usage: ./ftp <hostname> <port>" << endl;
        cerr << "usage: ./ftp" << endl;
        return (EXIT_FAILURE);
    }

    if (argc == 2)
    {
        hostname = argv[1];
        port = DEFAULT_PORT;
        client = new FTP(hostname, port);
        client->setOnline();
        client->login();

    } else if (argc == 3)
    {
        hostname = argv[1];
        port = atoi(argv[2]);
        string portString(argv[2]);

        if (!input.isDigit(portString) || (port <= 0 || port > 65535))
        {
            cerr << "usage: ./ftp <hostname> <port>" << endl;
            cerr << "Error: Invalid port number" << endl;
            return (EXIT_FAILURE);
        }
        client = new FTP(hostname, port);
        client->setOnline();
        client->login();

    } else
    {
        //no argument given, goes to ftp> prompt
        client = new FTP();
    }

    while (true)
    {
        //prompt for user input.
        bzero(cmd, sizeof(cmd));
        cout << PROMPT;
        cin.getline(cmd, IN_BUFFER);

        //check for non-input
        if (cmd[0] == '\0')
        {
            continue;
        }

        //check for all spaces input
        int i = 0;
        while (cmd[i])
        {
            char c = cmd[i];
            if (isspace(c))
            {
                i++;
            }
            else
            {
                break;
            }

        }
        string test1(cmd);
        if (test1.length() == i)
        {
            continue;
        }

        vector<string> instruction = input.stringToArgs(cmd);
        //check for valid input or instruction
        if (!input.validate(instruction))
        {
            continue;
        }

        if (instruction.at(0) == "quit" || instruction.at(0) == "exit")
        {
            if (client->isConnected())
            {
                client->disconnect();
            }
            cout << HEADER << endl;
            cout << EXIT_MSG << endl;
            cout << HEADER << endl;
            exit(EXIT_SUCCESS);

        } else if (instruction.at(0) == "open" && input.open(instruction))
        {

            if (client->isConnected())
            {
                cerr << "Already connected to " << hostname << ", use close first." << endl;
                continue;
            }

            if (instruction.size() == 1)
            {
                bzero(cmd, sizeof(cmd));
                cout << "(to) ";
                cin.getline(cmd, IN_BUFFER);

                //check for non-input
                if (cmd[0] == '\0')
                {
                    cerr << USAGE << endl;
                    continue;
                }

                //check for all spaces input
                int y = 0;
                while (cmd[y])
                {
                    char c = cmd[y];
                    if (isspace(c))
                    {
                        y++;
                    }
                    else
                    {
                        break;
                    }

                }
                string test2(cmd);
                if (test2.length() == y)
                {
                    cerr << USAGE << endl;
                    continue;
                }

                vector<string> openTemp = input.stringToArgs(cmd);
                if (openTemp.size() > 1)
                {
                    cerr << "ftp: connect: Connection refused" << endl;
                    continue;
                }

                hostname = cmd;

            } else if (instruction.size() == 2)
            {
                port = DEFAULT_PORT;
                hostname = input.toCharArr(instruction.at(1));

            }
            else
            {
                port = atoi(instruction.at(2).c_str());
                hostname = input.toCharArr(instruction.at(1));
            }

            if (client->setOnline(hostname, port))
            {
                client->login();
            } else
            {
                cerr << "ftp: " << hostname << ": Name or service not known" << endl;
            }

        } else if (instruction.at(0) == "close")
        {
            client->disconnect();

        } else if (instruction.at(0) == "ls")
        {
            client->ls();

        } else if (instruction.at(0) == "pwd")
        {
            client->pwd();

        } else if (instruction.at(0) == "cd")
        {
            vector<string> args;
            if (instruction.size() > 1)
            {
                args = input.processArgs(cmd);
                client->cd(args.at(1));
            }
            else
            {
                client->cd(EMPTY);
            }

        } else if (instruction.at(0) == "get")
        {

            if (instruction.size() == 3)
            {
                client->get(instruction.at(1), instruction.at(2));
            }
            else if (instruction.size() == 2)
            {
                client->get(instruction.at(1), EMPTY);
            }
            else
            {
                client->get(EMPTY, EMPTY);
            }

        } else if (instruction.at(0) == "put")
        {
            if (instruction.size() == 3)
            {
                client->put(instruction.at(1), instruction.at(2));
            }
            else if (instruction.size() == 2)
            {
                client->put(instruction.at(1), EMPTY);
            }
            else
            {
                client->put(EMPTY, EMPTY);
            }
        } else if (instruction.at(0) == "del")
        {
            vector<string> args;
            if (instruction.size() > 1)
            {
                args = input.processArgs(cmd);
                client->del(args.at(1));
            }
            else
            {
                client->del(EMPTY);
            }
        } else if (instruction.at(0) == "user")
        {
            if (instruction.size() == 3)
            {
                client->user(instruction.at(1), instruction.at(2));
            }
            else if (instruction.size() == 2)
            {
                client->user(instruction.at(1), EMPTY);
            }
            else
            {
                client->user(EMPTY, EMPTY);
            }
        }
    }
}