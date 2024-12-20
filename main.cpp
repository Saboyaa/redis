#include <iostream>
#include <string.h>
#include <vector>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "resp.cpp"

#include <signal.h>

using namespace std;

#define MYPORT "6379" 
#define BACKLOG 10
#define MAXDATASIZE 100

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argv,char *argc[]){
    struct sockaddr_storage their_addr;
    socklen_t addr_size;
    char buff[MAXDATASIZE];
    struct addrinfo hints,*res,*p;
    int sockfd,new_fd;
    int yes =1;
    char s[INET6_ADDRSTRLEN];
    int rv,num_bytes;
    vector<resp> *x=new vector<resp>();
    //getaddrinfo()

    void* shared_mem = mmap(NULL, sizeof(std::vector<resp>),
                            PROT_READ | PROT_WRITE,
                            MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    if (shared_mem == MAP_FAILED) {
        perror("mmap failed");
        return 1;
    }

    x = new (shared_mem) std::vector<resp>();

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // AF_INET or AF_INET6 to force version
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if((rv = getaddrinfo(NULL,MYPORT,&hints,&res))!=0){ 
        cerr<<"getaddrinfo: "<<gai_strerror(rv);
    }

    //socket()
    for(p = res;p!=NULL;p=p->ai_next){
        //socket
        if((sockfd = socket(p->ai_family,p->ai_socktype,p->ai_protocol))==-1){
            cerr<<"server:socket";
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        //bind
        if(bind(sockfd,p->ai_addr,p->ai_addrlen)==-1){
            close(sockfd);
            cerr<<"server:bind";
            continue;
        }
        break;
    }
    freeaddrinfo(res);

    if(p == NULL){
        cerr<<"server:failed to bind";
        exit(1);
    }

    //listen
    if(listen(sockfd,BACKLOG)){
        cerr<<"listen";
        exit(1);
    }

    cout<<"server: waiting for connections...\n";
    
    while(1){
    addr_size = sizeof their_addr;
    if((new_fd = accept(sockfd, (struct sockaddr *) &their_addr,&addr_size))==-1){
        cerr<<"accept";
        continue;
    };

    inet_ntop(their_addr.ss_family,get_in_addr((struct sockaddr *)&their_addr),s, sizeof s);
    if(!fork()){
    close(sockfd);
    cout<<"server: got connection from "<<s<<"\n";
    
    if((num_bytes = recv(new_fd,buff,MAXDATASIZE-1,0))==-1){
        cerr<<"recv";
    }
    buff[num_bytes]='\0';
    vector<resp> var_vec_resp = read_all(buff);
    x->push_back(var_vec_resp[0]);
    for(resp a:*x){
        for(resp b : a.vec){
            cout<<b.bulk<<" ";
        }
    }
    cout<<sizeof(buff[0]);
    if (send(new_fd, "+Hello, world!\r\n", 16, 0) == -1){
        cerr<<"send";
    }

    close(new_fd);
    exit(0);

    }else{wait(NULL);}
    close(new_fd);
    }
    munmap(shared_mem, sizeof(std::vector<resp>));
    return 0;
}