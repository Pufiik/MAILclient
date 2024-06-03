#include <stdlib.h> 
#include <sys/socket.h> 
#include <memory.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <iostream> 
#include <unistd.h> 
#include <limits.h>
#include <stdio.h>
#include <regex>

using namespace std;
char HELO_COMMAND[] = "EHLO test\n";
char MAIL_FROM[] = "MAIL FROM:<%s>\t\n"; 
char RCPT_TO[] = "RCPT TO:<%s>\r\n";
char DATA[] = "DATA\r\n";
char SUBJECT[] = "Subject: %s\n";
char FROM[] = "From: %s\n";
char TO[] = "To: %s\n";
char MESSAGE[] = "%s\r\n";
char DOT[] = ".\r\n";
char QUIT[] = "quit\r\n";
char AUTH_LOGIN[] = "AUTH LOGIN\r\n";
char LOGIN[] = "ZGFuaWlsLmdyaXNoYUByYW1ibGVyLnJ1\r\n";
char PASSWORD[] = "RGFuaWxrYTEy\r\n";
const unsigned short p = 2525;
const char * HOSTNAME = "smtp.rambler.ru";
const char * loginFrom = "daniil.grisha@rambler.ru";
const char * loginTo = "daniil.grisha@rambler.ru";
std::regex expression("[a-zA-Z0-9._-]+@rambler.ru");

void runSmtpCommand(char *message, int s) {
    char response[1024];

    memset(response, 0, sizeof(response)); 
    if (send(s, message, strlen(message),0) == -1){
        throw std::runtime_error("send(7) call error");
    } 
    cout << "Sending: " << message << endl; 
    cout << "Waiting for response" << endl;

    if (recv(s, response, sizeof(response), 0) == -1) {
        throw std::runtime_error("recv(5) call error");
    }
    cout << "Got response: " << response << endl << endl;
}
int main(int argc, char* argv[]) {

    hostent* host_name;
    sockaddr_in server_address;

    int _s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); 

    if (_s < 0) {
        throw std::runtime_error("socket(2) call error");
    }

    host_name = gethostbyname(HOSTNAME);
    
    if (host_name == nullptr) {
        throw std::runtime_error("gethostbyname(3) call error");
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(p);

    memcpy(&server_address.sin_addr.s_addr, host_name->h_addr, host_name->h_length);
    // Связывание сокета с заданным сетевым адресом.

    if (connect(_s, (sockaddr *) &server_address, sizeof(server_address)) == -1) {
        cout << "Connecting error" << endl;
        throw std::runtime_error("connect(4) call error");
    }

    char response[1024];

    memset(response, 0, sizeof(response));
    if ((recv(_s, response, sizeof(response), 0)) == -1){
        throw std::runtime_error("recv(5) call error");
    }

    char _login[_POSIX_LOGIN_NAME_MAX];

    if (argc == 2){
        if (std::regex_match(argv[1], expression)){
            loginTo = argv[1];
        } 
        else {
            throw std::runtime_error("invalid mail(8) call error");
        }
    }
    

    strcpy(_login, loginFrom);

    // Посылка EHLO
    // Посылка AUTH_LOGIN
    // Посылка LOGIN
    // Посылка PASSWORD

    runSmtpCommand(HELO_COMMAND, _s);
    runSmtpCommand(AUTH_LOGIN, _s);
    runSmtpCommand(LOGIN, _s);
    runSmtpCommand(PASSWORD, _s);

    // Посылка MAIL FROM

    char *buf = new char[BUFSIZ];
    sprintf(buf, MAIL_FROM, _login);
    runSmtpCommand(buf, _s);

    // Посылка RCPT TO
    buf = new char[BUFSIZ];
    sprintf(buf, RCPT_TO, argv[1]);
    runSmtpCommand(buf, _s);

    // Посылка DATA
    runSmtpCommand(DATA, _s);

    // Подготовка заголовка
    char finalMessage[BUFSIZ] = "";
    char tempFrom[100];

    sprintf(tempFrom, FROM, _login);
    strcat(finalMessage, tempFrom);
    char tempTo[100];
    sprintf(tempTo, TO, loginTo);
    strcat(finalMessage, tempTo);

    cout << "Subject: ";
    char subject[1024];
    memset(subject, 0, sizeof(subject)); 
    int i = 0;
    char n;
    while ((n = getchar()) != '\n') {
        subject[i++] = n;
    }
    char tempSubject[BUFSIZ];
    sprintf(tempSubject, SUBJECT, subject);
    strcat(finalMessage, tempSubject);
    // Подготовка письма
    char message[BUFSIZ];
    memset(message, 0, sizeof(message)); 
    
    i = 0;
    while ((n = getchar()) != EOF) {
        while (n != '\n') {
            message[i++] = n;
            n = getchar();
        }
        
        char tempMessage[BUFSIZ];
        message[i] = '\0';
        sprintf(tempMessage, MESSAGE, message);
        strcat(finalMessage, tempMessage);
    
        i=0;
    }
    strcat(finalMessage, DOT);
    cout << "\n"<< endl;
    // Посылка письма
    runSmtpCommand(finalMessage, _s);
    // Посылка QUIT
    runSmtpCommand(QUIT, _s);

    if (close(_s) == -1){
        throw std::runtime_error("close(6) call error");
    }
    return 0;
}