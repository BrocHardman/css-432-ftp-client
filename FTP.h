//Conard James B. Faraon
//CSS 432 Final Project: Network Application
//May 13, 2017

//Purpose- An Unix-like FTP client program based on the Internet Engineering Task Force RFC 959 protocol.
//         This is the header file for FTP client that supports several commands.

#ifndef FTP_FTP_H
#define FTP_FTP_H

#include "Socket.h"
#include "Input.h"
#include <cstring>
#include <vector>
#include <sys/poll.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>

using namespace std;

class FTP
{
public:

    FTP();

    FTP(const char[], const int);

    bool setOnline();

    bool setOnline(const char[], const int);

    bool isConnected();

    void login();

    int sendServerCommand(const char[], const unsigned long) const;

    //for long stream of strings
    int receiveStringStream(string &);

    //for long stream of strings
    int receiveStringStream(string &, const int);

    //for short server reply
    int receiveServerReply(string &);

    //extracting server's reply
    int getReplyCode(const string);

    int getPasv();

    void ls();

    void cd(const string);

    void get(const string, const string);

    void user(const string, const string);

    void download(const int, const int);

    void put(const string, const string);

    void upload(const int, const int);

    void pwd();

    void del(const string);

    void disconnect();

private:

    char *serverIp;
    Socket *server;
    bool isOnline;
    bool loggedIn;
    int port;
    int serverDescriptor;
    Input input;

    const static int FAIL = -1;
    const static int SUCCESS = 0;
    const static int BUFF_IN = 256;
    const static int BUFF_STREAM = 8192;
    const static int BUFF_SEND = 1500;

};


#endif //FTP_FTP_H
