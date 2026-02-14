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
        fprintf(stderr,"Usage:%s <IP> <START_PORT> <END_PORT>\n", argv[0]);
        return 1; // exit with error code 1 
    }
    char *target_ip=argv[1];//IP address is stored as string in the memory we used a pointer at the variable target_ip access to that memory location
    int start_port=atoi(argv[2]); //atoi helps to convert starting and ending ports from ASCII numeric string to integer
    int end_port=atoi(argv[3]);
    if (start_port<1 || end_port>65535 || start_port>end_port){
        fprintf(stderr,"Invalid port range\n");
        return 1; //exit on invalid input
    }
    printf("timestamp,ip,port,state\n");//Print CSV(comma seperated values)header for structured and machine-readable output
   for(int port=start_port;port<=end_port;port++){
        int sockfd=socket(AF_INET,SOCK_STREAM,0); //Create a TCP socket...AF_INET= Address family here is IPv4 SOCK_STREAM=Socket Type here it is TCP 
        if(sockfd==-1)//-1 is the code returned for if socket creation fails 
            continue; // skip this port if socket creation fails
        if(fcntl(sockfd, F_SETFL,O_NONBLOCK)==-1)//Set socket to non-blocking mode so connect() does not stall the scan
        {
            close(sockfd); //discard socket if configuration fails
            continue;
        }
      struct sockaddr_in target={0}; //Create and zero initialize an IPv4 socket address structure
      target.sin_family=AF_INET;//Specify that we use IPv4 address
      target.sin_port=htons(port);//Convert port number from host byte order to network byte order
      //inet_pton:convert human readable IP string to binary form and store in the target.sin_addr
      //1=success 0=invalid address string -1=error 
      if(inet_pton(AF_INET, target_ip,&target.sin_addr)!=1){
      fprintf(stderr, "Invalid IP address\n");
      close(sockfd);
      break;
   }
   int err = 0;//Variable to store final connection error/status
   //Attempt tcp connection to target address
   if (connect(sockfd,(struct sockaddr*)&target,sizeof(target))<0){
      if(errno==EINPROGRESS){
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
/*associated with <netinet/in.h> file 
struct sockaddr_in{
    sa_family_t sin_family;     
     // Address family identifier (e.g., AF_INET for IPv4).
     // Tells the kernel how to interpret the structure.
    in_port_t sin_port;         
     // Transport-layer port number in NETWORK BYTE ORDER.
     // Must use htons() when assigning.
    struct in_addr sin_addr;    
     // IPv4 address stored in binary form (32-bit).
     // Typically filled via inet_pton().
    char sin_zero[8];           
     // Unused padding bytes.
     // Ensures size compatibility with struct sockaddr.
};*/
