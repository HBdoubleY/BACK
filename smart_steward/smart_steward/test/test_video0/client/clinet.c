#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <poll.h>
//#include "yuv_to_jpeg.h"
        
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <sys/wait.h>

#define REQBUFLENGTH    10
#define REQBUF      1843200

int main(int argc, char argv[]){
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(8888);
	saddr.sin_addr.s_addr = inet_addr("192.168.17.83");
	int s_len = sizeof(saddr);	

    connect(sockfd, (struct sockaddr*)&saddr, s_len);

    char buf_size[REQBUFLENGTH] = {0};
    char buf_data[REQBUF] = {0};
    int size = 0;
    int file_name_count = 0;
    char file_name[20] = {0};
    int jpeg_fd = -1;
    while(1){
        memset(buf_size, 0, sizeof(buf_size));
        read(sockfd, buf_size, sizeof(buf_size));
        
        size = atoi(buf_size);
        printf("%d\n", size);

        memset(buf_data, 0, sizeof(buf_data));
        read(sockfd, buf_data, size);

        memset(file_name, 0, sizeof(file_name));
        sprintf(file_name, "./%d.jpeg", file_name_count++);
        jpeg_fd = open(file_name, O_WRONLY | O_CREAT, 0777);
        write(jpeg_fd, buf_data, size);
        close(jpeg_fd);
    }


    close(sockfd);
    return 0;
}