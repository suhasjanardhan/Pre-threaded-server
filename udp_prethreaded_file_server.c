#include <stdio.h>
#define _USE_BSD 1
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <math.h>
#include <signal.h>
#include <pthread.h>
#include <fcntl.h>

#define MAX_SEND_BUF 1000
#define MAX_RECV_BUF 1000
#define MAX_DATA 1000

extern int errno;

int errexit(const char *format,...);

pthread_mutex_t mtx=PTHREAD_MUTEX_INITIALIZER;

int connectUDP(const char *service,int portnum);
int connectsock(const char *service,int portnum,const char *transport);
void handler(int);

/*------------------------------------------------------------------------------------
 * connectsock-Allocate and connect socket for UDP
 *------------------------------------------------------------------------------------
*/

int connectsock(const char *service,int portnum,const char *transport)
{
/*
Arguments:
*service   - service associated with desired port
*transport - name of the transport protocol to use
*/
struct sockaddr_in server;                                                //an internet endpoint address

int server_socket,type,b,l,accept_socket,num;                             //two socket descriptors for listening and accepting 

memset(&server,0,sizeof(server));

server.sin_addr.s_addr=htons(INADDR_ANY);                                 //INADDR_ANY to match any IP address
server.sin_family=AF_INET;                                                //family name
server.sin_port=htons(portnum);                                              //port number

 
/*
 * to determine the type of socket
 */

if(strcmp(transport,"udp")==0)
{
type=SOCK_DGRAM;
}
else
{
type=SOCK_STREAM;
}


server_socket=socket(AF_INET,type,0);                                    //allocate a socket

if(server_socket<0)
{
printf("Socket can't be created\n");
exit(0);
}

/* to set the socket options- to reuse the given port multiple times */
num=1;

if(setsockopt(server_socket,SOL_SOCKET,SO_REUSEPORT,(const char*)&num,sizeof(num))<0)
{
printf("setsockopt(SO_REUSEPORT) failed\n");
exit(0);
}


/* bind the socket to known port */
b=bind(server_socket,(struct sockaddr*)&server,sizeof(server));

if(b<0)
{
printf("Error in binding\n");
exit(0);
}

return server_socket;

}



/*------------------------------------------------------------------------
 * connectUCP-connect to a specified UDP service on specified host
 -------------------------------------------------------------------------*/
int connectUDP(const char *service,int portnum)
{
/*
 Arguments:
 *service-service associated with desired port
 */
 return connectsock(service,portnum,"udp");
}




int errexit(const char* format,...)
{
va_list args;

va_start(args,format);
vfprintf(stderr,format,args);
va_end(args);
exit(1);
}


void *handle(void *msock)
{
        int sock=*(int*)msock;
	 char msg[1000];
         int data_len,alen;
         //int ssock;
	   
          while(1)
           {
            struct sockaddr_in fsin;
	    alen=sizeof(struct sockaddr_in);
             
            

	               char send_buf[MAX_SEND_BUF];



		
			data_len = recvfrom(sock,msg,MAX_DATA,0,(struct sockaddr *)&fsin,&alen);
			
			
			if(data_len)
			{
				printf("Connected to prethreaded connectionless oriented server\n");
				printf("File name recieved: %s\n", msg);
				
			}
						
			
			int file;             //for reading local file(server file)
			if((file = open(msg,O_RDONLY))<0)
			{       
				
				printf("File not found\n");
				 printf("Client disconnected\n");
			}
			else
			{	
				
				printf("File opened successfully\n");
					
				
				
				ssize_t read_bytes;
 				ssize_t sent_bytes;
				 
				char send_buf[MAX_SEND_BUF]; 
				
	
				 
				 while( (read_bytes = read(file, send_buf, MAX_RECV_BUF)) > 0 )
				 {
					 printf("%s",send_buf);
					 if( (sent_bytes = sendto(sock, send_buf, read_bytes, 0,(struct sockaddr *)&fsin,sizeof(fsin))) < read_bytes )
					 {
					   printf("send error");
					 
					 }
                                        pthread_self();
                                          
					//printf("Thread terminated:%lu",pthread_self());
                                         //close(msock);
					
				 }
				 close(file);	
                                 printf("\nClient disconnected\n");                           		 
			}

            
         }
         
  

}

/*
 main - connectionless prethreaded server
 */

int main(char argc,char *argv[])
{
       
       int nthreads=atoi(argv[1]);
       int portnum=atoi(argv[2]);
        
             

	char *service="echo";
        int alen;
        pthread_t thread[nthreads];

	int msock,ssock;
	

	/* call connectTCP to create a socket, bind it and place it in passive mode
	   once the call returns call accept on listening socket to accept the incoming requests
	 */

	msock=connectUDP(service,portnum);
        printf("Listening to client\n");
        

	    
           

	    if(msock<0)
		{
		
                   errexit("Accept: %s\n",strerror(errno));
		}

	int *thread_arg=malloc(sizeof(int));
                *thread_arg=msock;
        for(int i=0;i<nthreads;i++)
          {
	     if(pthread_create(&thread[i],NULL,handle,thread_arg)!=0)
                    {
                       printf("error in creating the thread\n");
                       return 1;
                     }
				            
          }
	  //printf("Thread created: %lu\n",pthread_self());
        for(int i=0;i<nthreads;i++)
	  {
	     pthread_join(thread[i],NULL);
                         
             
           }
 		
               
	 
	close(msock);
	return 0;

}












