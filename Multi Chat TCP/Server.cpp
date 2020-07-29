


#include<iostream>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<stdio.h>
#include <fcntl.h>
#include<unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include<thread>
#include <signal.h>
#include<map>
#include <queue>
#include <unordered_set>
using namespace  std;

map<int , string> sock_to_name ;
unordered_set <int>all_sock_ids;
queue<pair<int , string>> messages_recieved;

void send_to_all(string &msg , int ign){
    for(auto i : all_sock_ids){
        if(i != ign){
            int r = send(i, msg.c_str(),msg.length() ,0);
            if(r < 0){
                all_sock_ids.erase(i);
            }
        }
    }
}

void send_the_queue(){
    while(true) {
        while (!messages_recieved.empty()) {
            string msg =
                    sock_to_name[messages_recieved.front().first] + " : " + messages_recieved.front().second + "\n";
            send_to_all(msg, messages_recieved.front().first);
            messages_recieved.pop();
        }
        sleep(2);
    }
}

int make_accept_sock (int port) {

    int sock;
    struct sockaddr_in server_in;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock<0)
        cout<<"socket failed";

    const int one  = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

    server_in.sin_family = AF_INET;
    server_in.sin_addr.s_addr = INADDR_ANY;
    server_in.sin_port = htons(port);

    if(bind(sock, (struct sockaddr*)&server_in, sizeof(server_in))<0)
        cout<<"bind failed";

    if(listen(sock,25)<0)
        cout<<"listen failed";
    sock_to_name[-1]  = "Server";
    return sock;
}

void new_connection_send () {

    while (true) {
        string msg ;
        getline(cin , msg);
        messages_recieved.push({-1,msg});
        sleep(3);
    }
}
void new_connection_recv (int sock) {
    char buffer[100];
    while (true) {
        memset(buffer , '\0' , sizeof buffer);
        int r = recv(sock, buffer, 100, 0);
        if (r <= 0) break;
        cout << "Message recieved From " << sock_to_name[sock] << " : " << buffer << endl;
        messages_recieved.push({sock,string(buffer)});
        sleep(1);
    }
    cout << sock_to_name[sock] << "'s recieve thread is dead " << endl;
    close(sock);
    messages_recieved.push({-1,sock_to_name[sock] + " left\n"});
    sleep(5);
    sock_to_name.erase(sock);
    all_sock_ids.erase(sock);
}

int register_client(int sock){

    const string reg_m = " Welcome to the server : Please Register Yourself . Enter your name \n";
    int r = send(sock, reg_m.c_str(),reg_m.length() ,0);
    if(r <= 0){
        return -1;
    }
    char buffer[25] ;
    memset(buffer , '\0' , sizeof buffer);
    r = recv(sock, buffer, 25, 0);
    if (r <= 0) {
        return -1;
    }
    cout << (string(buffer) + " joined \n") << endl;
    messages_recieved.push({sock , string(buffer) + " joined\n"});

    {
        string msg;
        for (auto i : sock_to_name) msg.append(i.second + " , ");
        msg.append(" are online currently \n");

        r = send(sock, msg.c_str(), msg.length(), 0);
        if (r <= 0) {
            return -1;
        }
    }

    sleep(1);
    sock_to_name[sock] = string(buffer);
    all_sock_ids.insert(sock);
    new_connection_recv (sock);
}


void accept_loop (int port) {
    int sock = make_accept_sock(port);
    thread read_in(new_connection_send);
    thread send_all(send_the_queue);

    for (;;) {
        int new_sock = accept(sock, 0, 0);
        if(new_sock != -1){
        std::thread r(register_client, new_sock);
        r.detach();
    }
        else{
            cout << "Registration failed \n" << endl;
        }
    }

}


int main ()
{
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);
    signal(SIGPIPE, SIG_IGN);
    accept_loop(8000);

    return 0;
}



