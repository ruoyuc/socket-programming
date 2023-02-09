#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ctype.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
using namespace std;

#define myPort "21583"
#define maxBuflen 4000
#define localhost "127.0.0.1"
// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

// split messages 
vector<string> split_cooma(string &str)
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
//Below are functions used to readin txt file and turn it into a unordered map
void readInFile (unordered_map<string, string> &Umap){
    ifstream infile;
    infile.open("./cred.txt", ios::in);//"../cred.txt"
    if (!infile.is_open()) {
        cout << "fail to open the txt" << endl;
        return ;
    }
    string buf;
    while (getline(infile, buf)) {
        string s;
        vector<string> result = split_cooma(buf);
        if (infile.peek()!=EOF){
            s = result[1].substr(0,result[1].size()-1);
        }else{
            s = result[1];
        } 
        Umap.emplace(result[0],s);
        
        // string s = result[1].substr(0,result[1].size()-1);        
        // Umap.emplace(result[0],s);
    }
}

void trim(string &s) 
{
    if (s.empty()) 
    {
        return ;
    }
    s.erase(0,s.find_first_not_of(" "));
    s.erase(s.find_last_not_of(" ") + 1);
}
string isInFile(string username,string password,unordered_map <string, string> map){
    auto iter = map.find(username);
    if(iter == map.end()){
        return "0";//cannot find username
    }else{

        string x = iter->second;
        trim(x);
        trim(password);
        int cmpresult = x.compare(password);
        if(cmpresult==0){
            return "2"; //find username and password
        }else{
            return "1"; // find username but wrong password
        }
    }
}
void split_args2(char *args[], char *message) {
    char *p = strtok (message, ",");
    int i = 0;
    while (p != NULL)
    {
        args[i++] = p;
        p = strtok (NULL, ",");
    }
}

int main() {
    //readin the file cred.txt
    unordered_map <string, string> map;
    readInFile(map);
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
    struct sockaddr_storage their_addr;
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];

    char buf[maxBuflen];
    char data[maxBuflen];
    char *args[3];
    vector<string> saved;//this vector store the username and password received from serverM

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    // get information of backend server itself
    if((rv = getaddrinfo(localhost, myPort, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    // loop through all the results and make a socket
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if(bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }
        break;
    }
    if(p == NULL) {
        fprintf(stderr, "talker: failed to bind socket\n");
        return 2;
    }

    freeaddrinfo(servinfo);

    addr_len = sizeof their_addr;
    
    printf("The ServerC is up and running using UDP on port %s.\n", myPort);

    while(true){
        if((numbytes = recvfrom(sockfd, buf, maxBuflen - 1, 0,
                                (struct sockaddr *)&their_addr, &addr_len)) == -1) {
            perror("recvfrom");
            exit(1);
        }
        buf[numbytes] = '\0';        
        split_args2(args, buf);
        string vname = args[0];
        string vpass = args[1];
        printf("The ServerC received an authentication request from the Main Server.\n");
        string authStatus = isInFile(vname,vpass,map);
        char send[maxBuflen];
        strcpy(send, authStatus.c_str());
        numbytes = sendto(sockfd, send, strlen(send), 0,
            (struct sockaddr *)&their_addr, addr_len);
        
        if(numbytes == -1) {
            perror("listener: sendto");
            exit(1);
        }
        printf("The ServerC finished sending the response to the Main Server.\n"); 
    }
    return 0;
}
