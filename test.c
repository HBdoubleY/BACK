#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


int main(int argc, char* argv[]){
#if 0
    int pipefd[2] = {0};
    if (-1 == pipe(pipefd)){
        printf("init pipe error\n");
    }
    const char *str_write = {"123456789"};
    if (-1 == write(pipefd[1], str_write, strlen(str_write))){
        printf("write error\n");
    }
    char str_read[10] = {0}; 
    if (-1 == read(pipefd[0], str_read, sizeof(str_read))){
        printf("read error\n");
    }
    printf("read : %s\n", str_read)
#elif 1
    unlink("test_pipe");
    if (-1 == mkfifo("test_pipe", 0644)){
        perror("init mkfifo error\n");
    }
    int pipefd = open("test_pipe", O_RDWR);
    const char *str_write = {"123456"};
    char str_read[7] = {0};
    if (-1 == write(pipefd, str_write, strlen(str_write))){
        perror("write error\n");
    }

    int len = read(pipefd, str_read, sizeof(str_read) - 1);
    if (-1 == len){
        perror("read error\n");
    }
    str_read[len] = '\0';

    printf("%s\n", str_read);
    close(pipefd);
#elif 0

#endif

    return 0;
}
