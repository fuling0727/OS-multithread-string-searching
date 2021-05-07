#include <unistd.h> 
#include <stdio.h>
#include <string.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <sys/types.h>
#include <dirent.h>
#include <pthread.h> 
#include <fcntl.h>

void  GetWords(char *buffer);
void *WorkThread(void *arg);
void listFilesRecursively(char *basePath,char *text);
void FindFileWord(char *FileName,char *text);
int countOccurrences(FILE *fptr,char *text);
void pop();
char hello[100] ; 
char hello2[100];
int count = 0;
int total_count = 0;
int word_num = 0;
int server_fd, new_socket, valread; 
 
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
struct Query{
    char word[128];
    struct Query *next;
    
};
struct Query *head;
struct Query *tail;
int main(int argc, char const *argv[]) 
{ 
    
    
    struct sockaddr_in address;
    int opt = 1; 
    int addrlen = sizeof(address); 
    char buffer[1024] = {0};
    
    int PORT = atoi(argv[4]);
    int thread_num = atoi(argv[6]);
    char *base_path = argv[2];
    
    
    // Creating socket file descriptor 
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    { 
        perror("socket failed"); 
        exit(EXIT_FAILURE); 
    } 
       
    // Forcefully attaching socket to the port 8080 
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 
                                                  &opt, sizeof(opt))) 
    { 
        perror("setsockopt"); 
        exit(EXIT_FAILURE); 
    } 
    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons( PORT ); 
       
    // Forcefully attaching socket to the port 8080 
    if (bind(server_fd, (struct sockaddr *)&address,  
                                 sizeof(address))<0) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
    if (listen(server_fd, 3) < 0) 
    { 
        perror("listen"); 
        exit(EXIT_FAILURE); 
    } 
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,  
                       (socklen_t*)&addrlen))<0) 
    { 
        perror("accept"); 
        exit(EXIT_FAILURE); 
    } 
    pthread_t maint,pool[thread_num];
    
    for(int i = 0;i<thread_num;i++)
    {
        if(pthread_create(&pool[i],NULL,WorkThread,(void*)base_path) != 0)
        {
            printf("failed to create threads!\n");
        }
    }
    while(valread = read( new_socket , buffer, 1024)){
        printf("%s\n",buffer);
        GetWords(buffer);
        memset(buffer,0,1024);
    }
   
    return 0; 
} 
int j = 0;
char finish[10] = "finish";
void * WorkThread(void *arg)
{
    int a = 1;
    char text[128];
    char *base_path = malloc(sizeof(char)*1024);
    base_path = (char*)arg;
    while(a == 1){
        pthread_mutex_lock(&mutex2);
        if(word_num>0){
            memset(text,0,128);
            strncpy(text,head->word,strlen(head->word));
            memset(hello,0,100);
            sprintf(hello,"String: \"%s\"",text);
            total_count = 0;
            sleep(0.2);
            send(new_socket , hello , strlen(hello) , 0 ); 
            pop();
            listFilesRecursively(base_path,text);
            
             if(total_count == 0)
             {
                 sprintf(hello2,"Not Found");
                 send(new_socket , hello2 , strlen(hello2) , 0 ); 
             }
             if(word_num == 0){
                sleep(0.1);
                send(new_socket , finish , strlen(finish) , 0 ); 
             }
            
        }
        pthread_mutex_unlock(&mutex2);
    }
    pthread_exit(NULL);
}
void pop(){
    if(word_num == 1)
    {
        free(head);
        word_num--;
    }
    else{
        struct Query *tmp = (struct Query*)malloc(sizeof(struct Query));
        tmp = head;
        head = head->next;
        free(tmp);
        word_num--;
    }
}
void  GetWords(char *buffer)
{
    char *delim1 = " \" " ;
    char *sepstr = buffer;
    char *substr;
    char *first;
    int i=word_num;
    char *delim = "\"";
    char *ptr;
    ptr = strtok(sepstr, delim1);
    while(ptr != NULL){
        ptr = strtok(NULL,delim);
        if(ptr == NULL) break;
        if(strcmp(ptr," ") == 0)continue;
        if(strlen(ptr) > 128) {
            strcpy(hello,"too long");
            return 0;
        }
        if(word_num == 0)
        {
            head = (struct Query*)malloc(sizeof(struct Query));
            strncpy(head->word,ptr,strlen(ptr));
            tail = head;
            word_num++;
        }
        else{
            struct Query *tmp = (struct Query*)malloc(sizeof(struct Query));
            strncpy(tmp->word,ptr,strlen(ptr));
            tail->next = tmp;
            tail = tmp;
            word_num++;
            
        }
        i++;
    }
}
void listFilesRecursively(char *basePath,char *text)
{
    char path[1000];
    char *fname;
    struct dirent *dp;
    char *loc;
    DIR *dir = opendir(basePath);
    // Unable to open directory stream
    if (!dir)
        return;

    while ((dp = readdir(dir)) != NULL)
    {
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
        {
            
            fname = dp->d_name;
            loc = strstr(fname,".txt");
            // Construct new path from our base path
            
            strcpy(path, basePath);
            strcat(path, "/");
            strcat(path, dp->d_name);
            if(loc != NULL)
                FindFileWord(path,text);
            listFilesRecursively(path,text);
        }
    }

    closedir(dir);
}
void FindFileWord(char *fname,char *text)
{
    
    FILE *f;
    char str[100];
    memset(str,0,sizeof(str));
    //printf("%s\n",fname);
    f = fopen(fname,"r");
    if(!f)
    {
        printf("file error\n");
        return 1;
    }
    else
    {
        count = countOccurrences(f,text);
        total_count += count;
        memset(hello2,0,100);
        if(count > 0){
            sprintf(hello2,"File: ./%s, Count: %d",fname,count);
            sleep(0.2);
            send(new_socket , hello2 , strlen(hello2) , 0 ); 
        }
    }
    fclose(f);
    return 0;
}
int countOccurrences(FILE *fptr,char *text)
{
    char str[1000];
    char *pos;

    int index, count;
    count = 0;
    // Read line from file till end of file.
   
    while ((fgets(str, 1000, fptr)) != NULL)
    {
        index = 0;
        if(str[strlen(str)-1] == '\n')
                str[strlen(str)-1] = '\0';
        // Find next occurrence of word in str
        while ((pos = strstr(str + index, text)) != NULL)
        {
            // Index of word in str is
            // Memory address of pos - memory
            // address of str.
            index = (pos - str) + 1;

            count++;
        }
    }

    return count;
}