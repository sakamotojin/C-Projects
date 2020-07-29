#include <iostream>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<stdio.h>
#include <fcntl.h>
#include<unistd.h>
#include<string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include<stdlib.h>
#include<thread>

using namespace std;

void read_serve(int sock){
    int r = 1 ;
    char buffer[100];
    while (r > 0){
        memset(buffer , '\0' , sizeof buffer);
        r = recv(sock, buffer, 100, 0);
        if(r <= 0) {
            cout << "read thread dead" << endl;
            return ;
        }
        cout << buffer << endl;
    }
}

void write_serve(int sock){
    int s = 1;
    while(s > 0){
        string msg ;
        cin.clear();
        fflush(stdin);
        getline(cin , msg);
        s = send(sock , msg.c_str(), msg.length() , 0);

        if(s <= 0 ) {
            cout << "write thread dead " << endl;
            return ;
        }
        cout << "message sent successfully" << endl;
        cin.clear();
    }
}

int register_to_server(int sock){
    char buffer[100];
    memset(buffer , '\0' , sizeof buffer);
    int r = recv(sock, buffer, 100, 0);
    if(r <= 0) return -1;
    cout << buffer << endl ;

    string msg ;
    cin.clear();
    fflush(stdin);
    getline(cin , msg);
    r = send(sock , msg.c_str(), msg.length() , 0);
    string all ;
    if(r <= 0) return -1;
    return 2 ;
}

int main(){
    const int port = 8000;
    int sock;
    struct sockaddr_in client;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock<=0)
        cout<<"socket failed";

    client.sin_family = AF_INET;
    client.sin_addr.s_addr = INADDR_ANY;
    client.sin_port = htons(port);

    if(connect(sock, (struct sockaddr*)&client,sizeof(client))<0)
        cout<<"connection failed";

    if(register_to_server(sock) != -1) {
        cout << "Successfully registered to server " << endl;
        thread read_thread(read_serve, sock);
        thread write_thread(write_serve, sock);

        read_thread.join();
        write_thread.join();
    }
    close(sock);
}