#include <iostream>
#include <map>
#include <vector>
#include <poll.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <errno.h> 
#include <string.h> 
#include <string>
#include <signal.h>
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <algorithm>
using namespace std;
#define maxBuf  1000
#define localhost "127.0.0.1" 
#define PortOfServerC "21583"    
#define PortOfServerCS "22583"
#define PortOfServerEE "23583"
#define PortOfUDP "24583"
#define PortOfClient "25583"
#define BACKLOG 20

void sigchld_handler(int s){
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;
    
    while(waitpid(-1, NULL, WNOHANG) > 0);
    
    errno = saved_errno;
}
//The function below is from Beej's book
int intialTCP(const char* port){
    int rv;
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    struct sigaction sa;
    int yes = 1;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;//Use my IP

    if((rv = getaddrinfo(localhost, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
    }

    for(p = servinfo; p != NULL; p = p->ai_next) {
        if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }
        if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
            perror("setsockopt");
            exit(1);
        }
        if(bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
                close(sockfd);
                perror("server: bind");
                continue;
            }
            break;

    }
    freeaddrinfo(servinfo);//

    if(p == NULL) {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

	if(listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

	sa.sa_handler = sigchld_handler; // reap all dead processes sigemptyset(&sa.sa_mask);
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1); 
    }

    return sockfd;

}


int intialUDP(const char* port){
    int sockfd;
	int rv;
	struct addrinfo hints, *servinfo, *p;
	socklen_t addr_len;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;

	if ((rv = getaddrinfo(localhost, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

    for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("talker: socket");
			continue;
		}
        if(bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }
			break;
	}
       if (p == NULL) {
		fprintf(stderr, "talker: failed to create socket\n");
		return 2;
	}
	freeaddrinfo(servinfo); // done with servinfo
	return sockfd;
}
string encrypt(string str){
    string ans;
    for(auto c:str){

        int num = char(c);
        if(num>47&&num<58){
            //char c is a digit
            if (num <  54){
                num = num +4;
                ans+= (char) num;
                continue;
            } else{
                num = num-6;
                ans += (char) num;
                continue;
            }
        }
        if (num>64 && num<91){
            // char c is a upper char
            if (num <  87){
                num = num +4;
                ans+= (char) num;
                continue;
            } else{
                num = num-22;
                ans += (char) num;
                continue;
            }
        }
        if (num>96&&num<123){
            if (num <  119){
                num = num +4;
                ans+= (char) num;
                continue;
            } else{
                num = num-22;
                ans += (char) num;
                continue;
            }
        }
        ans+=c;

    }
    return ans;
}
// use udp to query with port info
//Below function is from the Beej's book
//code for serverC is 1, serverCS is 2, serverEE is 3
void send_backend(int sockfd, const char *port, char *data_sent, char *data_recv,int code) {
    
    int rv,numbytes;
    struct addrinfo hints, *servinfo, *p;
	socklen_t addr_len;
	memset(&hints, 0, sizeof hints);
    char recv_data[maxBuf]; // dataReci received from backend server
    if ((rv = getaddrinfo(localhost, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return;
	}
    for(p = servinfo; p != NULL; p = p->ai_next) {
            if ((socket(p->ai_family, p->ai_socktype,
                    p->ai_protocol)) == -1) {
                perror("talker: socket");
                continue;
            }
        break;
    }
    if (p == NULL) {
            fprintf(stderr, "talker: failed to create socket\n");
            return;
	}
	if ((numbytes = sendto(sockfd, data_sent, strlen(data_sent), 0,
                           p->ai_addr, p->ai_addrlen)) == -1) {
		perror("talker: sendto");
		exit(1);
	}

    
    
    if(code==1){
        cout<<"The main server sent an authentication request to serverC."<<endl;
    }
    if(code==2){
        cout<<"The main server sent a request to serverCS."<<endl;
    }
    if(code==3){
        cout<<"The main server sent a request to serverEE."<<endl;
    }
    int recv_bytes;

    recv_bytes = recvfrom(sockfd, recv_data, sizeof recv_data, 0, NULL, NULL);
    if(recv_bytes == -1) {
        perror("recvfrom");
        exit(1);
    }
    
    recv_data[recv_bytes] = '\0';
    strcpy(data_recv, recv_data);
    
    if(code==1){
        cout<<"The main server received the result of the authentication request from ServerC using UDP over port 24583."<<endl;
    }
    if(code==2){
        cout<<"The main server received the response from serverCS using UDP over port 24583."<<endl;
    }
    if(code==3){
        cout<<"The main server received the response from serverEE using UDP over port 24583."<<endl;
    }
    
}

// getPort function is from the code on the instruction of project on d2l
int getPort(int socket_fd)
{
    struct sockaddr_in sin;
    socklen_t sinlen = sizeof(sin);
    getsockname(socket_fd, (struct sockaddr *)&sin, &sinlen);
    return ntohs(sin.sin_port);
}

vector<string> split_cooma(string str)
{   string pattern = ",";
    vector<string> res;
    if(str == "")
        return res;
    string strs = str + pattern;
    size_t pos = strs.find(pattern);

    while(pos != strs.npos)
    {
        string temp = strs.substr(0, pos);
        res.push_back(temp);
        strs = strs.substr(pos+1, strs.size());
        pos = strs.find(pattern);
    }

    return res;
}

int main(){
    int socketTCP = intialTCP(PortOfClient);//listen
    int socketUDP = intialUDP(PortOfUDP);//bind
    int new_fd;
    int error_count = 0;
    int numbytes;// The length of message received from the client
    string feedback;//Store the status for authentication, 0 no user, 1 wrong password, 2 success
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char ip4[INET_ADDRSTRLEN];		   // space to hold the IPv4 string
    string clientSend; // Store the username and pass word the client send.
    char udp_from_serverC[maxBuf];
    bool query = false;
    printf("The main server is up and running.\n");
    new_fd = accept(socketTCP,(struct sockaddr*)&client_addr, &addr_len);
    if (new_fd == -1) { 
            perror("accept"); 
        }
    while(1){
        char buffer[110];
        int numbytes;
        if ((numbytes = recv(new_fd, buffer, sizeof buffer, 0)) == -1)
        {
            perror("ServerM: recv");
            exit(1);
        }
        buffer[numbytes] = '\0';
        vector<string> a = split_cooma(buffer);
        string username = a[0];
        printf("The main server received the authentication for %s using TCP over port 25583.\n",username.c_str());        
        string TCP_received = buffer;
        
        string encryptString = encrypt(TCP_received);
        char sendSerC[110];
        strcpy (sendSerC,encryptString.c_str());
        //send the username and password to the serverC and receive info from serverC
        send_backend(socketUDP, PortOfServerC,sendSerC,udp_from_serverC,1);
        if((udp_from_serverC[0]=='\0')||(udp_from_serverC[0] == '0')){
            feedback = "0";
            error_count+=1;
        }else if(udp_from_serverC[0] == '1'){
            feedback = '1';
            error_count+=1;
        }else if(udp_from_serverC[0] == '2'){
            feedback = '2';
            query = true;
        }
        if(send(new_fd,feedback.c_str(),feedback.size(),0)==-1)
        {   
            perror("severM: send");                                       
            exit(1);
        }
        cout<<"The main server sent the authentication result to the client."<<endl;
        
        //This is for the query section
        while(query){
            char buffer_query[110];
            int numbytes_query;
            char udp_from_serverEE[110];
            char udp_from_serverCS[110];
            if ((numbytes_query = recv(new_fd, buffer_query, sizeof buffer_query, 0)) == -1)
            {
                perror("ServerM: recv");
                exit(1);
            }
            buffer_query[numbytes_query] = '\0';
            vector<string> q = split_cooma(buffer_query);
            transform(q[1].begin(),q[1].end(),q[1].begin(),::tolower);
            cout << "The main server received from " <<username<< " to query course "<< q[0] << " about " << q[1] << "." << endl;
            if(buffer_query[0] == 'E'|| buffer_query[1] == 'E'){
                send_backend(socketUDP, PortOfServerEE,buffer_query,udp_from_serverEE,3);
                
                if(send(new_fd,udp_from_serverEE,sizeof(udp_from_serverEE),0)==-1){   
                    perror("severM: send");                                       
                    exit(1);
                }
                cout << "The main server sent the query information to the client." << endl;

            }else{
                send_backend(socketUDP, PortOfServerCS,buffer_query,udp_from_serverCS,2);

                
                if(send(new_fd,udp_from_serverCS,sizeof(udp_from_serverCS),0)==-1){   
                    perror("severM: send");                                       
                    exit(1);
                }
                cout << "The main server sent the query information to the client." << endl;
                
            }


        }
        if(error_count>2){
            cout<< "The client type in three error, SeverM exit"<<endl;
            exit(1);
        }    
    }
    return 0;
}