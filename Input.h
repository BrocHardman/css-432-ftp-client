//Conard James B. Faraon
//CSS 432 Final Project: Network Application
//May 13, 2017

//Purpose- An Unix-like FTP client program based on the Internet Engineering Task Force RFC 959 protocol.
//         This is the header file for Input client that supports input validations.

#ifndef FTP_INPUT_H
#define FTP_INPUT_H

#include <iostream>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <sstream>
#include <cctype>
#include <map>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <string>

using namespace std;

class Input
{
public:
    Input();

    //split the input into multiple strings using space as a delimiter
    vector<string> stringToArgs(const char[]);

    //separate the command from the args
    vector<string> processArgs(char[]);

    char *toCharArr(const string, const string, unsigned long &);

    char *toCharArr(const string, unsigned long &);

    char *toCharArr(const string);

    bool isDigit(const string) const;

    bool validate(const vector<string>) const;

    bool open(const vector<string>) const;

    int getPort(string);

    //masked the password
    string getpass(const char *, bool);

private:

    //helper function for getpass
    int getch();

    const static int MULTIPLIER = 256;

};


#endif //FTP_INPUT_H
