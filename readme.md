EE450 Socket Programming Project, Fall 2022

a. Full Name:Ruoyu Chen

b. Student ID: 4635203583

c. What I have done in the assignment: 
        I completed all the required phases in the Description except the extra credit section.
    
d. What my code files are and what each one of them does:
    client.cpp: 
        client can communicate with serverM by TCP, send the input of clients and receive request from serverM by using TCP.

    serverM.cpp: 
        serverM communicate with client to get messages about user login and query for course by TCP, send login message to serverC to verify the authentication, and send query for class message to the serverCS or serverEE according to the class. It also sends back the result of authentication and course information to the client.In addition, the serverM encrypt the username and password from client and send them to serverC.

    serverC.cpp: 
        serverC read the encrypted txt file in and store these information in an unordered map.When there is information from serverM,it will compare it with map and return the authentication result to serverM over UDP.    

    serverCS.cpp: 
        serverCS read the CS.txt file storing the course information and store these information on several map.When there is query sent from serverM, it will return the result of query by  looking up in dictionaries to the serverM over UDP.

    serverEE.cpp: 
        It functions like serverCS.cpp, but it read the EE.txt and is responsible for EE course query.

e. The format of all the messages exchanged:
    
    client: 
        The input data like username and password are in authentication_send string like（“username,password”）and is sent in a string like "username,password" to serverM.It will receive string like "0","1","2" to represent no user,wrong password,success. Later, it will send query string like "EE450,Days" to serverM to query course information.It will receive "0" for no such course and "Tue,Thu" for your query. 

    serverM:        
        The authentication data sent from client will be in buffer.And it looks like "james,2kAnsa7s)".
        The authentication data sending to serverC is in sendSerC. It looks like "james,2kAnsa7s)".
        The authentication result sent from serverC is in  udp_from_serverC.It is "0" for no user,"1" for wrong password and "2" for success.
        The authentication result sending to client is in feedback.
        The feedback to client for authentification represent this: 0 for no user, 1 for wrong password , 2 for success.
        The query sent from the client is in buffer_query. It likes "EE450,Days".
        The query sending to serverEE or serverCS is in buffer_query.It likes "EE450,Days".
        The query result sent from the serverEE or severCS is in udp_from_serverEE or udp_from_serverCS.It likes "Tue,Thr".If it is "0", it means there is no such course.
        The query result sending to client is in udp_from_serverEE or udp_from_serverCS.It likes "Tue,Thr".If it is "0", it means there is no such course.

    serverC:
        The authentication data from the serverM is in buf like "james,2kAnsa7s)"
        The authentication result to the serverM is in send.
        We send "0" to the serverM to inply that there is no username.
        We send "1"  to imply that password is wrong and "2" to imply success.

    serverCS:
        The query information from the serverM is in buf like "CS100,Credit"
        The query result to serverM is in send.
        If we cannot find the course, we will return "0" to serverM.
        If we find, we will return the information like "4".

    serverEE:
        The query information from the serverM is in buf like "EE450,Days"
        The query result to serverM is in send.
        If we cannot find the course, we will return "0" to serverM.
        If we find, we will return the information like "Tue,Thu".

g. Any idiosyncrasy of my project:

        I don't find fail in my test. To run my project, you need to do these thing.
        1. open 5 terminal and run "make all" in one of them
        2. typing "./serverM" using one terminal to run serverM.
        3. typing "./serverC" using one terminal to run serverC.
        4. typing "./serverCS" using one terminal to run serverCS.
        5. typing "./serverEE" using one terminal to run serverEE.
        6. typing "./client" using one terminal to run client and input data.
h. reuse of the code:

        1. some code for setting TCP and UDP are from the "Beej's Guide to Network Programming".
        2. some function to deal with string like split string into several strings are from github.
        These code are marked in the program.