//Conard James B. Faraon
//CSS 432 Final Project: Network Application
//May 13, 2017

//Purpose- An Unix-like FTP client program based on the Internet Engineering Task Force RFC 959 protocol.

#include "Input.h"

const string OPEN_C = "open";
const string CLOSE_C = "close";
const string LS_C = "ls";
const string QUIT_C = "quit";
const string CD_C = "cd";
const string GET_C = "get";
const string PUT_C = "put";
const string EXIT_C = "exit";
const string PWD_C = "pwd";
const string DELE_C = "del";
const string USER_C = "user";

Input::Input()
{
}

//Break char into multiple strings.
vector<string> Input::stringToArgs(const char in[])
{
    stringstream strStream(in);
    vector<string> cmd;
    string word;
    while (getline(strStream, word, ' '))
    {
        cmd.push_back(word);
    }

    return cmd;
}

//Break the command and the args.
vector<string> Input::processArgs(char in[])
{

    char *test = NULL;
    char *next_token = NULL;
    vector<string> args;

    test = strtok_r(in, " ", &next_token);

    args.push_back(string(test));  //grab first input
    args.push_back(string(next_token));

    return args;
}

char *Input::toCharArr(const string command, const string arg, unsigned long &finalLength)
{
    finalLength = command.length() + arg.length() + 2;
    char *converted = new char[finalLength];
    strcpy(converted, command.c_str());
    strcat(converted, arg.c_str());
    strcat(converted, "\r\n");

    return converted;
}

char *Input::toCharArr(const string command, unsigned long &finalLength)
{
    finalLength = command.length() + 2;
    char *converted = new char[finalLength];
    strcpy(converted, command.c_str());
    strcat(converted, "\r\n");

    return converted;
}

char *Input::toCharArr(const string in)
{
    char *converted = new char[in.length()];
    strcpy(converted, in.c_str());

    return converted;
}

bool Input::isDigit(const string s) const
{
    int i = 0;
    while (i < s.length())
    {
        //checks if the character is a digit
        if (!isdigit(s[i]))
        {
            return false;
        }

        i++;
    }

    return true;
}

//Validate the instruction to ensure compliance.
bool Input::validate(const vector<string> in) const
{
    if (in.size() == 1 && in.at(0) == CLOSE_C)
    {
        return true;

    } else if (in.size() == 1 && in.at(0) == QUIT_C)
    {
        return true;

    } else if (in.size() == 1 && in.at(0) == EXIT_C)
    {
        return true;

    } else if (in.size() == 1 && in.at(0) == LS_C)
    {
        return true;

    } else if (in.size() >= 1 && in.at(0) == CD_C)
    {
        return true;

    } else if (in.size() >= 1 && in.size() < 4 && in.at(0) == GET_C)
    {
        return true;

    } else if (in.size() >= 1 && in.size() < 4 && in.at(0) == PUT_C)
    {
        return true;

    } else if (in.size() == 1 && in.at(0) == PWD_C)
    {
        return true;

    } else if (in.size() >= 1 && in.at(0) == DELE_C)
    {
        return true;

    }
    else if (in.size() >= 1 && in.at(0) == OPEN_C)
    {
        return true;

    }
    else if (in.size() >= 1 && in.size() < 4 && in.at(0) == USER_C)
    {
        return true;

    }
    else if (in.size() > 3 && in.at(0) == USER_C)
    {
        cerr << "usage: user username [password]" << endl;
        return false;

    }
    else if (in.size() > 3 && in.at(0) == GET_C)
    {
        cerr << "usage: get remote-file [ local-file ]" << endl;
        return false;

    }
    else if (in.size() > 3 && in.at(0) == PUT_C)
    {
        cerr << "usage: put local-file remote-file" << endl;
        return false;

    }
    else if (in.at(0).length() == 1)
    {
        cerr << "?Invalid command" << endl;

    } else if (in.at(0).length() > 1)
    {
        cerr << "?Ambiguous command" << endl;
    } else
    {
        cerr << "?Invalid command" << endl;
    }

    return false;
}

//Process the connection and login process
bool Input::open(const vector<string> in) const
{
    if (in.size() == 3 && in.at(0) == OPEN_C)
    {
        if (isDigit(in.at(2)))
        {
            int temp = atoi(in.at(2).c_str());
            if (temp > 0 && temp <= 65535)
            {
                return true;
            }
            else
            {
                cerr << "ftp: " << in.at(1) << ": Name or service not known" << endl;
                return false;
            }
        }
        else
        {
            cerr << in.at(1) << ": bad port number-- " << in.at(2) << endl;
            cerr << "usage: open host-name [port]" << endl;
            return false;
        }

    } else if (in.size() == 2 && in.at(0) == OPEN_C)
    {
        return true;

    } else if (in.size() == 1 && in.at(0) == OPEN_C)
    {
        return true;
    }

    cerr << "usage: open host-name [port]" << endl;
    return false;
}

//Get the port for passive mode.
int Input::getPort(string in)
{
    char *str = new char[in.length()];
    strcpy(str, in.c_str());
    char *test = NULL;
    char *next_token = NULL;
    int counter = 0;
    int port1 = 0;
    int port2 = 0;
    test = strtok_r(str, " ,()\n", &next_token);
    while (test != NULL)
    {
        if (counter == 8)
        {
            port1 = atoi(test);
        }
        if (counter == 9)
        {
            port2 = atoi(test);
        }
        test = strtok_r(NULL, " ,()\n", &next_token);
        counter++;
    }

    int result = (port1 * MULTIPLIER) + port2;
    return result;
}

//Helper method for the get password.
int Input::getch()
{
    int ch;
    struct termios t_old, t_new;

    tcgetattr(STDIN_FILENO, &t_old);
    t_new = t_old;
    t_new.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &t_new);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &t_old);
    return ch;
}

//Masked the password with asterisks.
string Input::getpass(const char *prompt, bool show_asterisk = true)
{
    const char BACKSPACE = 127;
    const char RETURN = 10;

    string password;
    unsigned char ch = 0;

    cout << prompt;

    while ((ch = getch()) != RETURN)
    {
        if (ch == BACKSPACE)
        {
            if (password.length() != 0)
            {
                if (show_asterisk)
                    cout << "\b \b";
                password.resize(password.length() - 1);
            }
        }
        else
        {
            password += ch;
            if (show_asterisk)
                cout << '*';
        }
    }
    cout << endl;
    return password;
}