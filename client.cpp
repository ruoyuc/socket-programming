#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <ctype.h> 
#include <arpa/inet.h>
#include <iostream>
#define PORT "25583"//It is the port that the serverM use
#define MAXDATASIZE 1000 // The largest number of bite get at one time
#define localhost "127.0.0.1"

//Get the get_in_addr function from the Beej's book

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int getPort(int socket_fd)
{
    struct sockaddr_in sin;
    socklen_t sinlen = sizeof(sin);
    getsockname(socket_fd, (struct sockaddr *)&sin, &sinlen);
    return ntohs(sin.sin_port);
}
//The main function

int main()
{   char username[110];
    char password[110];
    char authentication_send[110];
    char coursecode[110];
    char category[110];
    char query_send[110];
    int attempsRemian = 3;
    // Below code is frome the Beej's book

    int sockfd = 0, numbytes;  
    char buf[MAXDATASIZE];
    char buf_query[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(localhost, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and connect to the first we can ---- Beej
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
            s, sizeof s);

    freeaddrinfo(servinfo); // all done with this structure


    printf("The client is up and running.\n");

    while (attempsRemian>0){
        attempsRemian--;
        printf("Please enter the username:");
        scanf("%109s",username);
        printf("Please enter the password:");
        scanf("%109s",password);
        strcpy(authentication_send,username);
        strcat(authentication_send,",");
        strcat(authentication_send,password);//Now the  authentication_send contain the "username,password"
        int len = strlen(authentication_send);
        int clientSendResult=send(sockfd, authentication_send, len, 0);// BECAREFUL SEND NAME
        if(clientSendResult == -1) {
            perror("ERROR: client fails to send\n");
        }
        printf("%s sent an authentication request to the main server.\n",username);

        //above code are from Beej's code
        if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
            perror("recv");
            exit(1);
        }

        buf[numbytes] = '\0';
        int TCPport = getPort(sockfd);
        
        if (strcmp(buf, "0")==0) {
            printf("%s received the result of authentication using TCP over port %d.Authentication failed: Username Does not exist\n",username,TCPport);
            printf("Attemps remaining: %d\n",attempsRemian);
            
        } else if (strcmp(buf,  "1")==0) {
            printf("%s received the result of authentication using TCP over port %d.Authentication failed: Password Does not match\n",username,TCPport);
            
            printf("Attemps remaining: %d\n",attempsRemian);
            
        } else if (strcmp(buf,  "2")==0) {
            printf("%s received the result of authentication using TCP over port %d.Authentication is successful\n",username,TCPport);
            //This is the query part
            while(1){
                printf("Please enter the course code to query:");
                scanf("%109s",coursecode);
                printf("Please enter the category (Credit / Professor / Days / CourseName):");
                scanf("%109s",category);
                strcpy(query_send,coursecode);
                strcat(query_send,",");
                strcat(query_send,category);//Now the query_send contain the "coursecode,category"
                int len_query = strlen(query_send);
                int query_send_result = send(sockfd, query_send, len_query, 0);
                if( query_send_result == -1) {
                    perror("ERROR: client fails to send\n");
                }
                std::cout<<username<<" sent a request to main server."<<std::endl;
                int query_recv_result = recv(sockfd, buf_query, MAXDATASIZE-1, 0);
                if (query_recv_result == -1) {
                    perror("recv");
                    exit(1);
                }
                buf_query[query_recv_result] = '\0';
                //above code are from Beej's code
                std::cout<<"The client received the response from the Main server using TCP over port "<<TCPport<<std::endl;
                if(strcmp(buf_query,  "0")==0){
                    std::cout<<"Didn’t find the course: "<< coursecode <<std::endl;
                }else{
                    std::cout<<"The "<<category<<" of "<<coursecode<<" is "<< buf_query<<std::endl;
                }
                std::cout<<"\n\n-----Start a new request-----"<<std::endl;
                
            }
        }
        if (attempsRemian==0){
            close(sockfd);
            printf("Authentication Failed for 3 attempts. Client will shut down.\n");
            exit(1);
        }                  
    
    }
    
    return 0;        
    
}