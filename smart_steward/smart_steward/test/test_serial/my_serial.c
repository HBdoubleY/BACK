#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

int serialfd;

int uart_init(){

    serialfd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NDELAY);
    if(0 > serialfd)    return -1;

    fcntl(serialfd, F_SETFL, 0);

    // struct termios serial_info;
    // memset(&serial_info, 0, sizeof(serial_info));
    // tcgetattr(serialfd, &serial_info);

    struct termios serial_info = {
        .c_cflag = CS8 | (~PARENB) | (~CSTOPB) | (~CRTSCTS) | (~INPCK) | CREAD | CLOCAL,
        //.c_cc |= INIT_C_CC 
        .c_cc[VTIME] = 0,
        .c_cc[VMIN]  = 0
    };

    cfsetispeed(&serial_info, B115200);
    cfsetospeed(&serial_info, B115200);

    tcflush(serialfd, TCIFLUSH);

    tcsetattr(serialfd, TCSANOW, &serial_info);

    return 0;
}

int main(int argc, char argv[]){

    int ret = uart_init();
    printf("ret = %d\n", ret);



#if 0
    unsigned char buf[36] = {0};
    int len = 36;
    int num = 0;
    int m = 0;
    unsigned char *p = buf; 

    if(ret == -1)   return 0;
    while(len){
        num = read(serialfd, p, 36);
        len -= num;
        p   += num;
        printf("%d\n", num);
    }

    printf("over while\n");
    if(buf[0] == 0xBB)
    {
        for(int i = 0; i < 36; i++){
            printf("%d = %x \n", i, buf[i]);
        }
    }
#endif
#if 1
    printf("input cmd\n");

    unsigned char send[36] = {0XDD, 0X07, 0X24, 0X00, 0X00};
    if( 0 > write(serialfd, send, 36))
    {
        printf("Write is Error");
        return 0;
    }
#endif
    printf("over\n");
    return 0;
}