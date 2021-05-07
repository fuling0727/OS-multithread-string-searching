#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h> 
#include <fcntl.h>
#include <netinet/in.h>
//#define PORT 8080 
#include <pthread.h>


char test[10];
//void * clientThread(char const *argv[]);
void* clientThread(void *arg)
{
    int a = 1;
    
    char msg[128];
    char buffer[300]; 
    char buffer2[300]; 
    int valread;
    int sock = (int* )arg;
    
    //printf("%d\n",sock);
    
    while(a == 1){
        memset(msg,0,128);
        memset(buffer,0,300);
        
        fgets(msg,sizeof(msg),stdin);
        if(msg[strlen(msg)-1] == '\n')
            msg[strlen(msg)-1] = '\0';
        //printf("%s\n",msg);
        send(sock , msg , strlen(msg) , 0 ); 
        
        while(strcmp(buffer,"finish") != 0){
            memset(buffer,0,300); 
            valread = read( sock , buffer, 1024);
            if(strcmp(buffer,"finish") == 0)break;
            else
                printf("%s\n",buffer ); 
            //memset(buffer,0,300); 
        }
        memset(msg,0,128);
        memset(buffer,0,300);
       
    }
        
        
        /*valread = read( sock , buffer, 1024); 
        valread = read( sock , test, 1024); 
        printf("%s\n",buffer ); 
        printf("%s\n",test ); 
    */
    
    pthread_exit(NULL);
    //return 0;
}
int main(int argc, char const *argv[]) 
{ 
    
    int i=0;
    //pthread_t tid;
    //pthread_create(&tid,NULL,clientThread,NULL);
    int t = 2;
    char *addr = argv[2];
    int PORT = atoi(argv[4]);
    struct sockaddr_in serv_addr; 
    int sock = 0, valread;
    
    
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n Socket creation error \n"); 
        return -1; 
    } 
   
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(PORT); 
    
    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, addr, &serv_addr.sin_addr)<=0)  
    { 
        printf("\nInvalid address/ Address not supported \n"); 
        return -1; 
    } 
    
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("\nConnection Failed \n"); 
        return -1; 
    } 
    pthread_t tid;
    void *ret;
    int a = 1;
    //while(a == 1){
        
        if(pthread_create(&tid,NULL,clientThread,(void*)sock) != 0)
        {
            printf("failed to create thread!\n");
        }
        pthread_join(tid,&ret);
        //printf("return:%d\n",ret);
        i++;
        //sleep(5);
        
        //
    //}
    close(sock);
    return 0; 
} 
