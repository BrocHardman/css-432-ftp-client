//Conard James B. Faraon
//CSS 432 Final Project: Network Application
//May 13, 2017

//Purpose- An Unix-like FTP client program based on the Internet Engineering Task Force RFC 959 protocol.

#include "FTP.h"

const string USER = "USER ";
const string PASS = "PASS ";
const string SYST = "SYST";
const string PASV = "PASV";
const string LIST = "LIST";
const string CWD = "CWD ";
const string QUIT = "QUIT";
const string TYPE_I = "TYPE I";
const string RETR = "RETR ";
const string STOR = "STOR ";
const string PWD = "PWD";
const string DELE = "DELE ";
const string ABOR = "ABOR";
const string NAME = "Name (";
const string COLON = ":";
const string CLOSING = "): ";
const string NOT_CONNECT = "Not Connected";
const double USEC = 1000000;

//set the elapsed time for get cmd to ON|OFF
bool isRecording = true;

//The constructor used when args were given when invoking the program.
FTP::FTP(const char host[], const int p)
{
    //deep copy server ip
    size_t len = strlen(host);
    serverIp = new char[len];
    strcpy(serverIp, host);

    //port number
    port = p;

    //make a socket for this port
    server = new Socket(port);

    //online status
    isOnline = false;

    //login status
    loggedIn = false;

    //uninitialized server socket
    serverDescriptor = -1;

}

FTP::FTP()
{
    //port number
    port = FAIL;

    //make a socket for this port
    server = NULL;

    //online status
    isOnline = false;

    //login status
    loggedIn = false;

    //uninitialized server socket
    serverDescriptor = -1;

}

bool FTP::setOnline()
{

    //establish a connection with the ftp server
    serverDescriptor = server->getClientSocket(serverIp);
    if (serverDescriptor != FAIL)
    {
        //set online status
        isOnline = true;

        //get reply from the server
        string reply;
        if (receiveServerReply(reply) > 0)
        {
            cout << reply;
        }

        return true;
    }

    return false;
}

//The function overloaded for establishing new connection.
bool FTP::setOnline(const char host[], const int p)
{
    //deep copy server ip
    size_t len = strlen(host);
    serverIp = new char[len];
    strcpy(serverIp, host);

    //port number
    port = p;

    //make a socket for this port
    server = new Socket(port);

    // establish a connection with the ftp server
    serverDescriptor = server->getClientSocket(serverIp);
    if (serverDescriptor != FAIL)
    {
        //set online status
        isOnline = true;

        //get reply from the server
        string reply;
        if (receiveServerReply(reply) > 0)
        {
            cout << reply;
        }

        return true;
    }

    return false;
}

void FTP::login()
{
    if (!isConnected())
    {
        cerr << NOT_CONNECT << endl;
        return;
    }

    //get local host name
    string userString(getlogin());
    cout << NAME << serverIp << COLON << userString << CLOSING;

    //get username
    vector<string> instruction;
    char in[BUFF_IN];
    cin.getline(in, BUFF_IN);

    instruction = input.stringToArgs(in);

    if (instruction.size() != 0)
    {
        userString = instruction[0];
    }

    int replyCode = 0;
    string reply;
    unsigned long length;
    char *username = input.toCharArr(USER, userString, length);
    //send USER command to the ftp server
    if (sendServerCommand(username, length) > 0)
    {
        if (receiveServerReply(reply) > 0)
        {
            cout << reply;
            replyCode = getReplyCode(reply);
        }
    }

    //server crashes
    if (replyCode == 421)
    {
        close(serverDescriptor);
        isOnline = false;
        loggedIn = false;
        return;
    }

    //331 User name okay, need password.
    if (replyCode == 331)
    {
        string pw = input.getpass("Password: ", true);

        // Send password to server
        char *password = input.toCharArr(PASS, pw, length);
        string stringStream;
        if (sendServerCommand(password, length) > 0)
        {
            if (receiveStringStream(stringStream) > 0)
            {
                replyCode = getReplyCode(stringStream);

                if (replyCode == 230)
                {
                    //login success
                    cout << stringStream;
                    loggedIn = true;

                } else if (replyCode == 421) //server crashes
                {
                    cout << stringStream;
                    cerr << "Login failed." << endl;
                    close(serverDescriptor);
                    isOnline = false;
                    loggedIn = false;

                    return;
                }
                else
                {
                    //fail to login
                    cerr << stringStream;
                    cerr << "Login failed." << endl;
                }
            }
        }
    }

    //ask server for system type
    reply.clear();
    char *syst = input.toCharArr(SYST, length);
    if (sendServerCommand(syst, length) > 0)
    {
        if (receiveServerReply(reply) > 0)
        {
            cout << reply;
        }
    }

}

bool FTP::isConnected()
{
    return isOnline;
}

//enables passive mode
int FTP::getPasv()
{
    unsigned long length;
    char *command = input.toCharArr(PASV, length);
    string stringStream;
    int replyCode;
    //sends passive command to server
    if (sendServerCommand(command, length) > 0)
    {
        //read server's respond
        if (receiveStringStream(stringStream) > 0)
        {
            replyCode = getReplyCode(stringStream);
            if (replyCode == 530)
            {
                //error code from the server
                cout << stringStream;
                return FAIL;

            } else if (replyCode == 421)
            {
                //server crashes
                cout << stringStream;
                close(serverDescriptor);
                isOnline = false;
                loggedIn = false;
                return FAIL;
            }

            //calculate the port
            int thePort = input.getPort(stringStream);

            //read server's response
            cout << stringStream;
            return thePort;
        }
    }

    return FAIL;
}

void FTP::ls()
{
    if (!isConnected())
    {
        cerr << NOT_CONNECT << endl;
        return;
    }

    //establish data channel
    int dataPort = getPasv();

    if (dataPort == FAIL)
    {
        cerr << "ftp: Unable to establish PASV data connection." << endl;
        return;
    }

    //establish connection with the server
    Socket dataSocket(dataPort);
    int dataDescriptor = dataSocket.getClientSocket(serverIp);
    if (dataDescriptor == FAIL)
    {
        cerr << "ftp: Unable to establish PASV data connection." << endl;
        return;
    }

    pid_t pid;
    if ((pid = fork()) < SUCCESS)
    {
        //needs to exit due to memory issue
        cerr << "Client: Error in forking!" << endl;
        exit(EXIT_FAILURE);
    }

    //child
    if (pid == 0)
    {
        string stringstream;
        int replyCode;
        if (receiveStringStream(stringstream, dataDescriptor) > 0)
        {
            //just for formatting, data channel can be faster
            //50000 usec
            usleep(50000);

            //read server's reply
            cout << stringstream;
            replyCode = getReplyCode(stringstream);

            //server crashes
            if (replyCode == 421)
            {
                close(serverDescriptor);
                isOnline = false;
                loggedIn = false;
            }
        }

        exit(EXIT_SUCCESS);

    } else //parent
    {
        //int code;
        string reply;
        unsigned long length;
        char *command = input.toCharArr(LIST, length);
        //send LIST command to the server
        if (sendServerCommand(command, length) > 0)
        {
            //read header message from the server.
            if (receiveServerReply(reply) > 0)
            {
                cout << reply;
            }
        }

        //wait for child process to finish
        wait(NULL);

        if (receiveServerReply(reply) > 0)
        {
            //read the closing message from the server.
            cout << reply;
        }

        //close data channel
        close(dataDescriptor);
    }
}

void FTP::cd(const string args)
{
    if (!isConnected())
    {
        cerr << NOT_CONNECT << endl;
        return;
    }

    string remotedirectory = args;

    if (remotedirectory == "")
    {
        cout << "(remote - directory) ";
        getline(cin, remotedirectory);
    }
    if (remotedirectory == "")
    {
        cerr << "usage: cd remote-directory" << endl;
        return;
    }

    string reply;
    unsigned long length;
    //send CWD command to the server
    char *command = input.toCharArr(CWD, remotedirectory, length);
    int replyCode;
    if (sendServerCommand(command, length) > 0)
    {
        //read reply from the server
        if (receiveServerReply(reply) > 0)
        {
            replyCode = getReplyCode(reply);
            cout << reply;

            //server crashes
            if (replyCode == 421)
            {
                close(serverDescriptor);
                isOnline = false;
                loggedIn = false;
            }
        }
    }
}

void FTP::pwd()
{
    if (!isConnected())
    {
        cerr << NOT_CONNECT << endl;
        return;
    }

    string reply;
    unsigned long length;
    //send pwd command to the server
    char *command = input.toCharArr(PWD, length);
    int replyCode;
    if (sendServerCommand(command, length) > 0)
    {
        //read server's reply
        if (receiveServerReply(reply) > 0)
        {
            replyCode = getReplyCode(reply);
            cout << reply;

            //server crashes
            if (replyCode == 421)
            {
                close(serverDescriptor);
                isOnline = false;
                loggedIn = false;
            }
        }
    }
}

void FTP::disconnect()
{
    if (!isConnected())
    {
        cerr << NOT_CONNECT << endl;
        return;
    }

    string reply;
    unsigned long length;
    //send QUIT command to the server
    char *command = input.toCharArr(QUIT, length);
    if (sendServerCommand(command, length) > 0)
    {
        //disconnect the client from the server
        if (receiveServerReply(reply) > 0)
        {
            cout << reply;
            close(serverDescriptor);
            isOnline = false;
            loggedIn = false;
        }
    }
}

int FTP::sendServerCommand(const char message[], const unsigned long length) const
{
    return write(serverDescriptor, message, length);
}

//Receives a stream of string from the server.
int FTP::receiveStringStream(string &stringStream)
{
    char buffer[BUFF_STREAM];
    bzero(buffer, sizeof(buffer));

    //block
    int length = read(serverDescriptor, buffer, BUFF_STREAM);

    stringStream.clear();
    stringStream.append(buffer);

    //start reading stream from the server
    struct pollfd ufds;
    ufds.fd = serverDescriptor;
    ufds.events = POLLIN;
    ufds.revents = 0;
    while (poll(&ufds, 1, 1000) > 0)
    {
        bzero(buffer, sizeof(buffer));
        int value = read(serverDescriptor, buffer, BUFF_STREAM);
        if (value == 0)
        {
            break;
        }
        length += value;
        stringStream.append(buffer);
    }

    return length;
}

//Receives a stream of string from the server.
int FTP::receiveStringStream(string &stringStream, const int dataDescriptor)
{
    char buffer[BUFF_STREAM];
    bzero(buffer, sizeof(buffer));

    //block
    int length = read(dataDescriptor, buffer, BUFF_STREAM);

    stringStream.clear();
    stringStream.append(buffer);

    //start reading stream from the server
    struct pollfd ufds;
    ufds.fd = dataDescriptor;
    ufds.events = POLLIN;
    ufds.revents = 0;
    while (poll(&ufds, 1, 1000) > 0)
    {
        bzero(buffer, sizeof(buffer));
        int value = read(dataDescriptor, buffer, BUFF_STREAM);
        if (value == 0)
        {
            shutdown(dataDescriptor, SHUT_RD);
            break;
        }
        length += value;
        stringStream.append(buffer);

    }

    return length;
}

//Receives short reply from the server.
int FTP::receiveServerReply(string &reply)
{
    char buffer[BUFF_STREAM];
    bzero(buffer, sizeof(buffer));
    int length = read(serverDescriptor, buffer, sizeof(buffer));
    reply.clear();
    if (length != 0)
    {
        reply.append(buffer);
    }

    return length;
}

//Get the reply code from the server.
int FTP::getReplyCode(const string reply)
{
    if (reply.length() > 0)
    {
        char *message = input.toCharArr(reply);
        vector<string> temp;
        temp = input.stringToArgs(message);
        return atoi(temp[0].c_str());
    }
    return FAIL;
}

//Download a file from the server.
void FTP::get(const string remote, const string local)
{
    if (!isConnected())
    {
        cerr << NOT_CONNECT << endl;
        return;
    }

    string remotefilename = remote;
    string localfilename = local;

    if (remotefilename == "")
    {
        cout << "(remote-file) ";
        getline(cin, remotefilename);
        cout << "(local-file) ";
        getline(cin, localfilename);
    }

    if (remotefilename == "")
    {
        cerr << "usage: get remote-file [ local-file ]" << endl;
    }

    if (localfilename == "")
    {
        localfilename = remotefilename;
    }

    if (!loggedIn)
    {
        cerr << "Please login with your username and password." << endl;
    }

    //flag to check if it is a valid server file
    bool validFile = false;

    //establish data channel
    int dataPort = getPasv();
    if (dataPort == FAIL)
    {
        cerr << "ftp: Unable to establish PASV data connection." << endl;
        return;
    }
    Socket dataSocket(dataPort);
    int dataDescriptor = dataSocket.getClientSocket(serverIp);
    if (dataDescriptor == FAIL)
    {
        cerr << "ftp: Unable to establish PASV data connection." << endl;
        return;
    }

    //send TYPE I command to the server
    string reply;
    unsigned long length;
    char *command = input.toCharArr(TYPE_I, length);
    int replyCode;
    if (sendServerCommand(command, length) > 0)
    {
        //read server's response
        if (receiveServerReply(reply) > 0)
        {
            replyCode = getReplyCode(reply);
            cout << reply;

            //server crashes
            if (replyCode == 421)
            {
                close(serverDescriptor);
                isOnline = false;
                loggedIn = false;
                return;
            }
        }
    }

    //send retrieve command
    reply.clear();
    command = input.toCharArr(RETR, remotefilename, length);
    if (sendServerCommand(command, length) > 0)
    {
        if (receiveStringStream(reply) > 0)
        {
            replyCode = getReplyCode(reply);
            cout << reply;
            if (replyCode == 550)
            {
                cerr << "ftp server: Invalid file format or file does not exist. " << endl;

            } else if (replyCode == 421) //server crashes
            {
                remove(localfilename.c_str());
                close(serverDescriptor);
                isOnline = false;
                loggedIn = false;
            } else
            {
                validFile = true;
            }
        }
    }

    if (validFile)
    {
        //let the child read data from the server using the data channel
        pid_t pid;
        if ((pid = fork()) < SUCCESS)
        {
            //needs to exit due to memory issue
            cerr << "Client: Error in forking!" << endl;
            exit(EXIT_FAILURE);
        }

        //child
        if (pid == 0)
        {
            int fd;
            mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
            char *filename = new char[localfilename.length() + 1];
            strcpy(filename, localfilename.c_str());
            fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, mode);

            //read and download files
            download(dataDescriptor, fd);

            //child closes file descriptors
            close(dataDescriptor);
            close(fd);
            exit(EXIT_SUCCESS);

        }
        else //parent waits for the child process to finish
        {

            wait(NULL);
            close(dataDescriptor);
            return;
        }
    }

    //parents section here
    //close data channel
    close(dataDescriptor);

    //send ABOR command (ABORT) due to file does not exist in the server
    reply.clear();
    command = input.toCharArr(ABOR, length);
    if (sendServerCommand(command, length) > 0)
    {
        if (receiveServerReply(reply) > 0)
        {
            replyCode = getReplyCode(reply);
            cout << reply;
            //server crashes
            if (replyCode == 421)
            {
                remove(localfilename.c_str());
                close(serverDescriptor);
                isOnline = false;
                loggedIn = false;
            }
        }
    }
}

void FTP::download(const int dataDescriptor, const int fd)
{
    char message[BUFF_STREAM];
    struct pollfd ufds;
    ufds.fd = dataDescriptor;
    ufds.events = POLLIN;
    ufds.revents = 0;

    // used for measuring totalTime
    struct timeval startTime, endTime;

    if (isRecording)
    {
        // start the timer
        gettimeofday(&startTime, NULL);
    }

    //keep downloading and shutdown read when nothing to read
    while (poll(&ufds, 1, 1000) > 0)
    {
        bzero(message, sizeof(message));
        int length = read(dataDescriptor, message, BUFF_STREAM);
        if (length == 0)
        {
            shutdown(dataDescriptor, SHUT_RD);
            break;
        }

        write(fd, message, (size_t) length);
    }

    if (isRecording)
    {
        // stop the timer
        gettimeofday(&endTime, NULL);

        // print out the statistics
        double totalTime = (endTime.tv_sec - startTime.tv_sec) * USEC;
        totalTime += endTime.tv_usec - startTime.tv_usec;
        totalTime /= USEC;
        //cout << "data-receiving time = " << totalTime << " sec " << endl;
        printf("data-receiving time = %.10f sec\n", totalTime);
    }
}

//Put a file in the server.
void FTP::put(const string local, const string remote)
{
    if (!isConnected())
    {
        cerr << NOT_CONNECT << endl;
        return;
    }

    string localfilename = local;
    string remotefilename = remote;

    if (localfilename == "")
    {
        cout << "(local-file) ";
        getline(cin, localfilename);
        cout << "(remote-file) ";
        getline(cin, remotefilename);
    }

    if (localfilename == "")
    {
        cerr << "usage: put local-file remote-file" << endl;
    }

    if (remotefilename == "")
    {
        remotefilename = localfilename;
    }

    if (!loggedIn)
    {
        cerr << "Please login with your username and password." << endl;
    }

    //open a file
    int fd;
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    char *filename = new char[localfilename.length() + 1];
    strcpy(filename, localfilename.c_str());
    fd = open(filename, O_RDONLY, mode);
    delete[] filename;

    //check if the file exists
    if (fd < SUCCESS)
    {
        cerr << "local: " << localfilename << ": No such file or directory" << endl;
        return;
    }

    //establish data channel
    int dataPort = getPasv();
    if (dataPort == FAIL)
    {
        cerr << "ftp: Unable to establish PASV data connection." << endl;
        close(fd);
        return;
    }
    Socket dataSocket(dataPort);
    int dataDescriptor = dataSocket.getClientSocket(serverIp);
    if (dataDescriptor == FAIL)
    {
        cerr << "ftp: Unable to establish PASV data connection." << endl;
        return;
    }

    string reply;
    unsigned long length;
    char *command = input.toCharArr(TYPE_I, length);
    //send TYPE I command to server
    if (sendServerCommand(command, length) > 0)
    {
        if (receiveServerReply(reply) > 0)
        {
            cout << reply;
        }
    }

    reply.clear();
    command = input.toCharArr(STOR, remotefilename, length);
    int replyCode;
    //send STOR command to server
    if (sendServerCommand(command, length) > 0)
    {
        if (receiveStringStream(reply) > 0)
        {
            replyCode = getReplyCode(reply);
            cout << reply;

            //server crashes
            if (replyCode == 421)
            {
                close(serverDescriptor);
                isOnline = false;
                loggedIn = false;
                close(dataDescriptor);
                close(fd);
                return;
            }
        }
    }

    //start uploading file to the server
    upload(dataDescriptor, fd);

    close(dataDescriptor);
    close(fd);

    reply.clear();
    if (receiveStringStream(reply) > 0)
    {
        cout << reply;
    }
}

void FTP::upload(const int dataDescriptor, const int fd)
{
    char message[BUFF_SEND];
    //keep writing the file to the server
    while (true)
    {
        bzero(message, sizeof(message));
        int length = read(fd, message, BUFF_SEND);
        if (length == 0)
        {
            //let the server knows we are done writing
            shutdown(dataDescriptor, SHUT_WR);
            break;
        }
        write(dataDescriptor, message, (size_t) length);
    }
}

void FTP::del(const string in)
{
    if (!isConnected())
    {
        cerr << NOT_CONNECT << endl;
        return;
    }

    string remotefilename = in;

    if (remotefilename == "")
    {
        cout << "(remote-file) ";
        getline(cin, remotefilename);
    }

    if (remotefilename == "")
    {
        cerr << "usage: del remote-file" << endl;
        return;
    }

    string reply;
    unsigned long length;
    char *command = input.toCharArr(DELE, remotefilename, length);
    int replyCode;
    //send delete command to the server
    if (sendServerCommand(command, length) > 0)
    {
        if (receiveServerReply(reply) > 0)
        {
            replyCode = getReplyCode(reply);
            cout << reply;

            //server crashes
            if (replyCode == 421)
            {
                close(serverDescriptor);
                isOnline = false;
                loggedIn = false;
            }
        }
    }
}

void FTP::user(const string usr, const string psw)
{
    if (!isConnected())
    {
        cerr << NOT_CONNECT << endl;
        return;
    }

    string username = usr;
    string password = psw;

    if (username == "")
    {
        cout << "(username) ";
        getline(cin, username);
    }

    if (username == "")
    {
        cerr << "usage: user username [password]" << endl;
        return;
    }

    string reply;
    int replyCode;
    unsigned long length;
    char *userLog = input.toCharArr(USER, username, length);
    //send USER command to the server
    if (sendServerCommand(userLog, length) > 0)
    {
        if (receiveServerReply(reply) > 0)
        {
            replyCode = getReplyCode(reply);
            cout << reply;

            //already logged in
            if (replyCode == 530)
            {
                //just for formatting, control channel can be faster
                //50000 usec
                usleep(50000);
                cerr << "Login failed." << endl;
                return;

            } else if (replyCode == 421) //server crashes
            {
                cerr << "Login failed." << endl;
                close(serverDescriptor);
                isOnline = false;
                loggedIn = false;
                return;
            }
        }
    }

    if (password == "")
    {
        password = input.getpass("Password: ", true);
    }

    //Send password to server
    reply.clear();
    char *passwordLog = input.toCharArr(PASS, password, length);
    string stringStream;
    if (sendServerCommand(passwordLog, length) > 0)
    {
        if (receiveStringStream(stringStream) > 0)
        {
            replyCode = getReplyCode(stringStream);
            cout << stringStream;
            if (replyCode == 230)
            {
                //login success
                loggedIn = true;

            } else if (replyCode == 421)
            {
                cerr << "Login failed." << endl;
                close(serverDescriptor);
                isOnline = false;
                loggedIn = false;
            }
            else
            {
                //fail to login
                cerr << "Login failed." << endl;
            }
        }
    }

    //display system type
    reply.clear();
    char *syst = input.toCharArr(SYST, length);
    if (sendServerCommand(syst, length) > 0)
    {
        if (receiveServerReply(reply) > 0)
        {
            cout << reply;
        }
    }
}