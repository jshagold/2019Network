/*
 A simple server in the internet domain using TCP
 Usage:./server port (E.g. ./server 10000 )
 */
#include <stdio.h>
#include <sys/types.h>   // definitions of a number of data types used in socket.h and netinet/in.h
#include <sys/socket.h>  // definitions of structures needed for sockets, e.g. sockaddr
#include <netinet/in.h>  // constants and structures needed for internet domain addresses, e.g. sockaddr_in
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#define BUF_NUM 1024

void error(char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    int sockfd, clisockfd; //descriptors rturn from socket and accept system calls
    int portno; // port number
    socklen_t clilen;
    
    char buffer[BUF_NUM];
    
    
    /*sockaddr_in: Structure Containing an Internet Address*/
    struct sockaddr_in serv_addr, cli_addr;
    
    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }
    
    /*Create a new socket
     AF_INET: Address Domain is Internet
     SOCK_STREAM: Socket Type is STREAM Socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]); //atoi converts from String to Integer
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY; //for the server the IP address is always the address that the server is running on
    serv_addr.sin_port = htons(portno); //convert from host to network byte order
    
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) //Bind the socket to the server address
        error("ERROR on binding");
    
    listen(sockfd,5); // Listen for socket connections. Backlog queue (connections to wait) is 5
    
    clilen = sizeof(cli_addr);
    
    
    
    /* 반복문으로 브라우저입력 계속받기 */
    while(1)
    {
        /*accept function:
         1) Block until a new connection is established
         2) the new socket descriptor will be used for subsequent communication with the newly connected client.
         */
        clisockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (clisockfd < 0)
            error("ERROR on accept");
        
        
        
        /* http error code 식별자 */
        int http_error = 0;
        
        
        int n,m;
        
        while(1){
            
            /* buffer초기화 */
            bzero(buffer,BUF_NUM);
            
            n = read(clisockfd,buffer,BUF_NUM); //Read is a block function. It will read at most 255 bytes
            
            if (n < 0) error("ERROR reading from socket");
            
            printf("Here is the message: \n%s\n",buffer);
            
            
            if(n < BUF_NUM)
                break;
            /* 더이상 받을 메세지가 없으면 반복종료 */
        }
        
        
        
        /* request message parsing */
        char *f_cut_buf = strtok(buffer,"\r\n");
        char *s_cut_buf = strtok(f_cut_buf," ");
        
        char *file_buf = strtok(NULL," ");
        printf("need file=====  %s\n",file_buf);
        
        /* 요청받은 파일이름 = file_name*/
        char *file_name = strtok(file_buf,"/");
        
        printf("file name = %s\n", file_name);

        
        
        
        /* 요청받은 파일을 open */
        FILE *fp;
        
        
        if((fp = fopen((const char*)file_name,"r")) == NULL){
            
            
            bzero(buffer, sizeof(buffer));
            sprintf(buffer, "HTTP/1.1 404 Not Found\r\nContent-Length: 140\r\nContent-Type: text/html\r\n\r\n");
            int a;
            
            
            
            n = write(clisockfd,buffer,strlen(buffer));
            if(n < 0)
                error("Error writing to socket");
            
            
            
            FILE *notfound_fp = fopen("notfound.html","r");
            
            
            
            char n_read_file[BUF_NUM];
            /* 파일 read */
            n = fread(n_read_file, 1, BUF_NUM, notfound_fp);
            
            m = write(clisockfd, n_read_file, n);
            
            if(m < 0)
                error("Error writing to socket");
            
            
            fclose(notfound_fp);
            
            fclose(fp);
        }
        else {
            printf("%s\n", "hi");
            /* 파일크기 구하기 */
            int file_size;
            FILE *sfp = fopen((const char*)file_name,"r");
            
            fseek(sfp, 0, SEEK_END);
            file_size = ftell(sfp);
            printf("fp의 크기 :%d\n",file_size);
            fclose(sfp);
            
            
            /* 요청받은 파일의 확장자 = file_type */
            char *file_type_before = strtok(file_name,".");
            char *file_type = strtok(NULL, ".");
            printf("file type: %s\n", file_type);
            
            
            
            /* 요청받은 파일의 확장자 결정 */
            char *input_ftype;
            
            if (strcmp(file_type,"html") == 0)
                input_ftype = "text/html";
            else if (strcmp(file_type,"mp3") == 0)
                input_ftype = "audio/mpeg3";
            else if (strcmp(file_type,"gif") == 0)
                input_ftype = "image/gif";
            else if (strcmp(file_type,"jpeg") == 0)
                input_ftype = "image/jpeg";
            else if (strcmp(file_type,"pdf") == 0)
                input_ftype = "application/pdf";
            else if (strcmp(file_type,"ico") == 0)
                input_ftype = "image/x-icon";
            else
                error("There's no extension");
            
            
            /* response를 위한 buffer초기화 */
            bzero(buffer,1024);
            
            
            
            /* 보내야하는 response message */
            sprintf(buffer, "HTTP/1.1 200 OK\r\nContent-Length: %d\r\nContent-Type: %s\r\n\r\n", file_size, input_ftype);
            /* response message에 파일추가 */
            
            
            
            /* write실행 */
            n = write(clisockfd,buffer,strlen(buffer)); //NOTE: write function returns the number of bytes actually sent out —> this might be less than the number you told it to send
            if (n < 0) error("ERROR writing to socket");
            
            /* 읽을파일을 저장할 배열설정 */
            char read_file[BUF_NUM];
            
            
            
            
            while(1)
            {
                bzero(read_file,BUF_NUM);
                
                /* 파일 read */
                n = fread(read_file, 1, BUF_NUM, fp);
                
                m = write(clisockfd, read_file, n);
                if(m < 0)
                    error("Error writing to socket");
                if(n < BUF_NUM)
                {
                    break;
                }
            }
            
            
            
            fclose(fp);
        }
        close(clisockfd);
        printf("finish!!!!!!!!!!!\n\n\n\n");
    }
    
    
    close(sockfd);
    
    return 0;
}
