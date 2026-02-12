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
#include <errno.h> //TCP error codes for connection state detection
#include <string.h> //Memory utilities (memset)
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
    static char buf[32];//persistent output buffer
    time_t now=time(NULL);// current system time fetched from system in raw seconds
    struct tm *utc=gmtime(&now);// gmtime converts the address of now into broken down utc
    strftime(buf, sizeof(buf),"%Y-%m-%dT%H:%M:%SZ", utc); //convert a struct tm into a human readable  ip
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
        fprintf(stderr,"Usage:%s <IP> <START_PORT> <END_PORT>\n", argv[0]);
        return 1; // exit with error
    }
    char *target_ip=argv[1];// Read target IP address from command line
    int start_port=atoi(argv[2]); // Convert starting and ending ports from string to integer
    int end_port=atoi(argv[3]);
    if (start_port<1 || end_port>65535 || start_port>end_port){
        fprintf(stderr, "Invalid port range\n");
        return 1; // exit on invalid input
    }
    printf("timestamp,ip,port,state\n");//Print CSV(comma seperated values)header for structured and machine-readable output
   for(int port=start_port;port<=end_port;port++) {
        int sockfd = socket(AF_INET, SOCK_STREAM, 0); //Create a TCP socket...AF_INET= Address family SOCK_STREAM=Socket Type
        if (sockfd < 0)
            continue; // skip this port if socket creation fails
        if (fcntl(sockfd, F_SETFL,O_NONBLOCK) < 0)// Set socket to non-blocking mode so connect() does not stall the scan
        {
            close(sockfd); // discard socket if configuration fails
            continue;
        }
      struct sockaddr_in target={0}; //Create and zero initialize an IPv4 socket address structure
      target.sin_family=AF_INET; //Specify that we use IPv4 address
      target.sin_port=htons(port);//Convert port number from host byte order to network byte order
      //Convert human readable IP string to binary form and store in the target.sin_addr
      // inet_pton returns:
      //1 mean success 0 mean invalid address string -1 means error 
      if(inet_pton(AF_INET, target_ip,&target.sin_addr)!=1){
      fprintf(stderr, "Invalid IP address\n");
      close(sockfd);
      break;
   }
   int err = 0;//Variable to store final connection error/status
   //Attempt tcp connection to target address
   if (connect(sockfd,(struct sockaddr*)&target,sizeof(target))<0){
      if (errno==EINPROGRESS) {
        fd_set wfds;//structure used by select() to monitor sockets
        FD_ZERO(&wfds);//clear the set (always before using)
        FD_SET(sockfd, &wfds);//add our socket to the "write " monitoring set
        struct timeval tv={TIMEOUT_SEC,0};
        //Wait Until:
       //Socket becomes writable(connection finished)
       //timeout occurs
        if(select(sockfd + 1,NULL,&wfds,NULL,&tv)>0){
            //length variable required by getsockopt()
            socklen_t len=sizeof(err);
            //retrieve final connection from socket
            //SO_ERROR tells us whether connection succeded or failed
            getsockopt(sockfd,SOL_SOCKET,SO_ERROR,&err,&len);
        }else{
            err = ETIMEDOUT;//select() timed out which is treated as connection timeout
        }

    }else{
        err=errno;//connect() failed immediately(eg:connection refused)
    }
}
 printf("%s,%s,%d,%s\n",timestamp_utc(),target_ip,port,port_state(err));

close(sockfd);
}
    // Successful program termination
    return 0;
}

