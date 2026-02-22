//TCP port scanner Author:Anay Sharma
/*This is a TCP connect()-based port scanner written in C. It uses non-blocking
sockets with explicit timeouts to safely probe a target across a user-defined
port range.Each connection attempt is classified into meaningful TCP states
(open, closed, filtered, unreachable) based on OS-level error behavior.
Scan results are emitted in CSV format with UTC ISO-8601 timestamps, making
the output suitable for automation, log ingestion, and further analysis.
The project focuses on clarity and protocol correctness rather than high-speed
mass scanning, and serves as a solid foundation for future enhancements such as
parallel scanning, configurable timeouts, or alternative probing techniques.
*/
#include <stdio.h> //Output scan results
#include <stdlib.h> //Basic utilities (argument parsing)
#include <unistd.h>  //POSIX functions (close, read/write)
#include <errno.h> //TCP error codes for connection state 
#include <string.h> //Memory utilities 
#include <fcntl.h>  //Non-blocking socket control
#include <time.h>   //UTC timestamp generation
#include <sys/socket.h>//Socket creation and connection APIs
#include <sys/time.h>  //Timeout handling with select()
#include <arpa/inet.h> //IPv4 address conversion
#include <netinet/in.h> //IPv4 socket address structures
#define TIMEOUT_SEC 1     // TCP connection timeout (seconds)


// Example timestamp: 2026-02-09T21:45:30Z
//1st we declares a function that returns a pointer to a string (const char *)
const char *timestamp_utc(){
    static char buf[32];//array buf[32] will store our timestamp text 
    time_t now=time(NULL);// current system time fetched from system in raw seconds
    struct tm *utc=gmtime(&now);// gmtime converts the address of now into broken down utc(year,month,day,hour,etc)
    strftime(buf, sizeof(buf),"%Y-%m-%dT%H:%M:%SZ", utc); //formats a structured time into a readable time string
    return buf;
}


// Maps socket error codes to human-readable TCP port states
const char *port_state(int err){
    if (err==0) return "open"; //connection succeeded
    if (err==ECONNREFUSED) return "closed";//target actively refused the connection
    if (err==ETIMEDOUT) return "filtered"; //no response within timeout period
    if (err==EHOSTUNREACH || err==ENETUNREACH) return "unreachable"; //host or network is not reachable
    return "unknown";//unhandled or unexpected error
}


int main(int argc, char *argv[]) {
    if(argc!=4){
        fprintf(stderr,"Usage: %s <IP> <START_PORT> <END_PORT>\n",argv[0]);
        return 1;
    }
    char *target_ip=argv[1];//IP address is a string argument; target_ip stores a pointer to that string from argv
    int start_port=atoi(argv[2]);//atoi converts ascii based string to integer 
    int end_port=atoi(argv[3]);

    if(start_port<1 || end_port>65535 || start_port>end_port) {
        fprintf(stderr,"Invalid port range\n");
        return 1;
    }
    printf("timestamp,ip,port,state\n");
    for (int port=start_port;port<=end_port;port++)
        
        int sockfd=socket(AF_INET,SOCK_STREAM,0);//socket creation in this line....AF_INET=IPv4....SOCK_STREAM=TCP
        if (sockfd==-1) //-1 means socket creation failure 
            continue;
        if (fcntl(sockfd,F_SETFL,O_NONBLOCK)==-1)//feature for nonblocking by fcntl(file control)..F_SETFL=Set file status flags...O_NONBLOCK=do not block
        {
            close(sockfd);
            continue;
        }
        struct sockaddr_in target={0};//create IPv4 socket address structure and initialize all fields to zero
        target.sin_family=AF_INET;// pecify address family as IPv4
        target.sin_port=htons(port);//Convert port to network byte order and assign it

        if(inet_pton(AF_INET,target_ip, &target.sin_addr)!=1)// Convert human-readable IPv4 string into binary format for socket; 1=success,0=invalid string,-1=error
        {
            fprintf(stderr, "Invalid IP address\n");
            close(sockfd);
            break;
        }
        int err=0;//stores the final connection result....0=success, non-zero=socket error code.
        int rc=connect(sockfd,(struct sockaddr*)&target,sizeof(target));// TCP connection attempt to the target IP and port.
        int saved_errno=errno;// Save the current errno value immediately after connect().

       if (rc==0)//if connect() succeeded
        {
           err=0; //Immediate success → OPEN
        }
        else if (saved_errno == EINPROGRESS) 
           {
            fd_set wfds;//set of file descriptors declared to monitor for writability (used by select())
            FD_ZERO(&wfds);// Clear the descriptor set to remove any previous data (always do this before using)
            FD_SET(sockfd,&wfds);//Add our socket file descriptor to the set-we want to know when it becomes writable
            struct timeval tv={TIMEOUT_SEC,0};//// Set the maximum time select() should wait: TIMEOUT_SEC seconds,
            int sel=select(sockfd + 1, NULL, &wfds, NULL, &tv);// Wait for the socket to become writable or for timeout to expire....return>0 if socket is ready, 0 if timeout,-1 on error
           if (sel>0)
           {
                //Socket became writable before timeout → connection finished (success or failure)
                socklen_t len=sizeof(err);
                //prepare length variable for getsockopt()
                if(getsockopt(sockfd,SOL_SOCKET,SO_ERROR,&err,&len)<0)
                    err=errno;
               //retrieve the final connection status from the socket
             //if getsockopt itself fails,store the error in err
            }
            else if (sel == 0)
            {
                err = ETIMEDOUT;   // Timeout → FILTERED
            }
            else 
            {
              err=errno;       // select() failure
            }
        }
        else {
             err=saved_errno;     // Immediate failure

        }
        printf("%s,%s,%d,%s\n",timestamp_utc(),target_ip,port,port_state(err));
        close(sockfd);
    }

    return 0;
}
/*associated with <netinet/in.h> file 
struct sockaddr_in{
    sa_family_t sin_family;     
     //sa_family:data type....specifies ip address type....IPv4:AF_INET...IPv6:AF_INET6...kernel knows from here how to interpret structure 
    in_port_t sin_port;        
    //in_port_t:data type for TCP/UDP ports...sin_port:the variable of type in_port_t that stores the TCP/UDP port number(use htons() to assign)
    struct in_addr sin_addr;    
     //struct in_addr:data type that stores an IPv4 address in binary form (32 bits)
    //sin_addr is the variable name where it is the storage location inside the struct sockaddr_in where the IPv4 address is kept
    char sin_zero[8];           
     // Ensures size compatibility with struct sockaddr.
};*/
// target.sin_family target.sin_port.....target is container sin_family & sin_port are variables 
