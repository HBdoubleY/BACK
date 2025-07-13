#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sqlite3.h>
#include <signal.h>
#include <time.h>
#include <poll.h>
#include <fcntl.h>
#include <sys/types.h>    
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <linux/videodev2.h>
#include <sys/mman.h>
#include <poll.h>      
#include <netinet/ip.h>
#include <termios.h>
#include <dirent.h> 
#include <pthread.h>
#include "../include/cJSON.h"

sqlite3 *db;
int login_server_fd;																//192.168.17.83 : 8888
int login_client_fd;

int video_fd;
int video_server_fd;
int video_client_fd;
typedef struct __video_buffer
{
    void *start;
    size_t length;
}req_op;
req_op image_buffer[4];

int serial_fd;
int serial_server_fd;
int serial_client_fd;

#define BUFLEN			100
#define SQLBUFLEN		100

#define IMAGE_WIDTH 	640
#define IMAGE_HIGHT 	480
#define REQBUFLENGTH	10
#define REQBUF      	1843200

#define SERIALBUF 		36

void recycle_child(){
	wait(NULL);
}

int socket_init(){
	/*create socket*/
	if(0 > (login_server_fd = socket(AF_INET, SOCK_STREAM, 0)))											return -1;
	/*bind*/
	struct sockaddr_in login_server_addr;
	login_server_addr.sin_family = AF_INET;
	login_server_addr.sin_port = htons(7777);
	login_server_addr.sin_addr.s_addr = htons(INADDR_ANY);
	if(0 > bind(login_server_fd, (struct sockaddr *)&login_server_addr, sizeof(login_server_addr)))		return -2;
	/*listen*/	
	if(0 > listen(login_server_fd, 5))																	return -3;

	/*create socket*/
	if(0 > (video_server_fd = socket(AF_INET, SOCK_STREAM, 0)))											return -4;
	/*bind*/
	struct sockaddr_in video_server_addr;
	video_server_addr.sin_family = AF_INET;
	video_server_addr.sin_port = htons(8888);
	video_server_addr.sin_addr.s_addr = htons(INADDR_ANY);
	if(0 > bind(video_server_fd, (struct sockaddr *)&video_server_addr, sizeof(video_server_addr)))		return -5;
	/*listen*/	
	if(0 > listen(video_server_fd, 5))																	return -6;

	/*create socket*/
	if(0 > (serial_server_fd = socket(AF_INET, SOCK_STREAM, 0)))										return -7;
	/*bind*/
	struct sockaddr_in serial_server_addr;
	serial_server_addr.sin_family = AF_INET;
	serial_server_addr.sin_port = htons(9999);
	serial_server_addr.sin_addr.s_addr = htons(INADDR_ANY);
	if(0 > bind(serial_server_fd, (struct sockaddr *)&serial_server_addr, sizeof(serial_server_addr)))	return -8;
	/*listen*/	
	if(0 > listen(serial_server_fd, 5))																	return -9;

    return 0;
}

int sqlite_init(){
	/*create/open database*/
	if(0 != sqlite3_open("../database/member.db", &db))									return -1;
	/*create info table*/
	char *errmsg=NULL;
	if(0 != sqlite3_exec(db, "create table if not exists member(username text, password text);", NULL, NULL, &errmsg)){
		sqlite3_close(db);
		return -2;
	}
	return 0;
}

int register_login(){
	int nrow;
	int ncolumn;
	char** dbresult;
	char buf[BUFLEN] = {0};
	printf("in register login\n");

	if(0 == read(login_client_fd, buf, sizeof(buf))){
		exit(0);
	}
	printf("%s\n", buf);
	cJSON *json_str = cJSON_Parse(buf);
	
	if(json_str == NULL)	return -1;
	else	puts(cJSON_Print(json_str));

	char *type = cJSON_GetObjectItem(json_str, "type")->valuestring;
	printf("%s\n", type);
	char *username = cJSON_GetObjectItem(json_str, "username")->valuestring;
	printf("%s\n", username);
	char *password = cJSON_GetObjectItem(json_str, "password")->valuestring;
	printf("%s\n", password);

	char sql[SQLBUFLEN]={0};
	if(0 == strncmp(type, "login", 5)){
		sprintf(sql, "select *from member where username='%s';", username);
		sqlite3_get_table(db, sql, &dbresult, &nrow, &ncolumn, NULL);
		if(0 != nrow){
			if(0 == strcmp(dbresult[1+ncolumn], password)){
				sprintf(buf, "login successful!");
				write(login_client_fd, buf, sizeof(buf));
				return 0;
			}
			else{
				sprintf(buf, "passwd mistake!\nplese input your name and passwd again");
				write(login_client_fd, buf, sizeof(buf));
				return 1;
			}
		}
		else{
			sprintf(buf, "username no exsit!\nplese input your name and passwd again");
			write(login_client_fd, buf, sizeof(buf));
			return 2;
		}
	}
	if(0 == strncmp(type, "register", 8)){
		sprintf(sql, "select *from member where username='%s';", username);
		sqlite3_get_table(db, sql, &dbresult, &nrow, &ncolumn, NULL);
		if(0 == nrow){
                sprintf(sql, "insert into member values('%s', '%s');", username, password);
                sqlite3_get_table(db, sql, &dbresult, &nrow, &ncolumn, NULL);
                sprintf(buf, "register successful!");
                write(login_client_fd, buf, sizeof(buf));
				return 3;
            }
		else{
			sprintf(buf, "this username already exsited!\n");
			write(login_client_fd, buf, sizeof(buf));
			return 4;
		}
	}
}

int serial_init(){
	serial_fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NDELAY);
    if(0 > serial_fd)    return -1;
    fcntl(serial_fd, F_SETFL, O_NONBLOCK);
    struct termios serial_info = {
        .c_cflag = CS8 | (~PARENB) | (~CSTOPB) | (~CRTSCTS) | (~INPCK) | CREAD | CLOCAL,
        .c_cc[VTIME] = 0,
        .c_cc[VMIN]  = 0
    };
    cfsetispeed(&serial_info, B115200);
    cfsetospeed(&serial_info, B115200);
    tcflush(serial_fd, TCIFLUSH);
    tcsetattr(serial_fd, TCSANOW, &serial_info);
    return 0;
}

void *serial_send_data_to_client(){
	unsigned char buf[36] = {0};
    int len = 36;
    int num = 0;
    int m = 0;
    unsigned char *p = buf; 

	while(1){
		while(len){
			num = read(serial_fd, p, 36);
			len -= num;
			p   += num;
		}

		char send_buf[BUFLEN] = {0};
		sprintf(send_buf, "{\"temperature\":\"%d\",\"illumination\":\"%d\",\"humidity\":\"%d\"}", buf[5], buf[21]*100+buf[20], buf[7]);
		write(serial_client_fd, send_buf, sizeof(send_buf));
		printf("%s\n", send_buf);
	}
}

int camera_init(){
	/*打开摄像头设备*/
	video_fd = open("/dev/video0", O_RDWR | O_NONBLOCK);
	if(video_fd < 0)  return -1;
    /*当前视频设备支持的视频图像格式*/
	struct v4l2_fmtdesc fmt;
    memset(&fmt,0,sizeof(fmt));
	fmt.index = 0;
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	/*设置摄像头的属性*/
	struct v4l2_format set_format;
    memset(&set_format, 0, sizeof(set_format));
    set_format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;          /*表示视频捕获设备*/
	set_format.fmt.pix.width = IMAGE_WIDTH;                 /*预设的宽度*/
	set_format.fmt.pix.height = IMAGE_HIGHT;                /*预设的高度*/
    set_format.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;    /*预设的格式    JPEG    */ 
    set_format.fmt.pix.field = V4L2_FIELD_ANY;              
	if(ioctl(video_fd, VIDIOC_S_FMT, &set_format))  return -2;
    //得到图片格式
    struct v4l2_format get_format;
    memset(&get_format, 0, sizeof(get_format));
    get_format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ioctl(video_fd, VIDIOC_G_FMT, &get_format);

	/*设置摄像头采集的帧率*/
	struct v4l2_streamparm set_streamparm;
    memset(&set_streamparm, 0, sizeof(set_streamparm));
	set_streamparm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;      /*表示视频捕获设备*/
	set_streamparm.parm.capture.timeperframe.numerator = 1;
	set_streamparm.parm.capture.timeperframe.denominator = 10;

	/*设置摄像头的帧率*/
    if(ioctl(video_fd, VIDIOC_S_PARM, &set_streamparm))   return -3;
    /*获取摄像头的帧率*/
    struct v4l2_streamparm get_streamparm;
    memset(&get_streamparm, 0, sizeof(get_streamparm));
    get_streamparm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if(ioctl(video_fd, VIDIOC_G_PARM, &get_streamparm))   return -3;

	/*请求缓冲区: 申请摄像头数据采集的缓冲区*/
	struct v4l2_requestbuffers set_req;
	memset(&set_req, 0, sizeof(set_req));
	set_req.count = 4;                              /*预设要申请4个缓冲区*/
	set_req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;     /*视频捕获设备*/
	set_req.memory = V4L2_MEMORY_MMAP;              /*支持mmap内存映射*/
    /*申请缓冲区*/
	if(ioctl(video_fd, VIDIOC_REQBUFS, &set_req))       	return -4;

	/*获取缓冲区的详细信息: 地址,编号*/
	struct v4l2_buffer req_info;
    for(int i = 0; i < set_req.count; i++){
        memset(&req_info, 0, sizeof(req_info));
		req_info.index = i;                             // 这里需要解释一下，因为在调用ioctl-VIDIOC_REQBUFS时，建立了count个Buffer。所以，这里index的有效范围是：0到count-1.
        req_info.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;    /*视频捕获设备*/
		req_info.memory = V4L2_MEMORY_MMAP;             /*支持mmap内存映射*/
        /*获取缓冲区的详细信息*/
		if(ioctl(video_fd, VIDIOC_QUERYBUF, &req_info))    return -5;
		
		/*根据摄像头申请缓冲区信息: 使用mmap函数将内核的地址映射到进程空间*/
        image_buffer[i].length = req_info.length;
		image_buffer[i].start = mmap(NULL, 
                                req_info.length, 
                                PROT_READ | PROT_WRITE,
                                MAP_SHARED, 
                                video_fd, 
                                req_info.m.offset); 
		if(image_buffer[i].start == NULL)            return -6;

        /*根据节点编号将缓冲区放入队列*/
        memset(&req_info, 0, sizeof(req_info));
        req_info.index = i;                             /*缓冲区的节点编号*/
		req_info.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;    /*视频捕获设备*/
		req_info.memory = V4L2_MEMORY_MMAP;             /*支持mmap内存映射*/
        if(ioctl(video_fd, VIDIOC_QBUF, &req_info))  return -7;
	}
	/*6. 启动摄像头数据采集*/
	enum v4l2_buf_type Type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if(ioctl(video_fd, VIDIOC_STREAMON, &Type))     return -8;
    
	return 0;
}

void *camera_send_jpeg_to_client(){  
	struct v4l2_buffer video_buffer;
    int number = 0;
    char buf_data[REQBUF] = {0};
    char buf_size[REQBUFLENGTH] = {0};

	struct pollfd poll_list[getdtablesize()];
    int intersting_postion = -1;				        //初始长度为-1 才能从起点添加文件设备
    poll_list[++intersting_postion].fd = video_fd;	    //加入需要监视的文件设备
    poll_list[intersting_postion].events = POLLIN;	    //表示感兴趣

	while(1){
		poll(poll_list, intersting_postion+1, -1);
		memset(&video_buffer, 0, sizeof(video_buffer));

		/*取出队列里采集完毕的缓冲区*/
		video_buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;    /*视频捕获设备*/
		video_buffer.memory = V4L2_MEMORY_MMAP;
		ioctl(video_fd, VIDIOC_DQBUF, &video_buffer);
		printf("DQBUF\n");

		printf("image_buffer[%d]=%p\n",
				video_buffer.index,
				image_buffer[video_buffer.index].start);
		number = video_buffer.index;

		memset(buf_size, 0, sizeof(buf_size));
		sprintf(buf_size, "%d", video_buffer.bytesused);
		write(video_client_fd, buf_size, sizeof(buf_size));
		write(video_client_fd, image_buffer[video_buffer.index].start, video_buffer.bytesused);
		printf("write over\n");

		/*将缓冲区再放入队列*/
		memset(&video_buffer, 0, sizeof(video_buffer));
		video_buffer.index = number;
		video_buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		video_buffer.memory = V4L2_MEMORY_MMAP;
		ioctl(video_fd, VIDIOC_QBUF, &video_buffer);                  
		printf("QBUF\n");
	}
}

int recv_client_cmd(){
	char buf[BUFLEN] = {0};
	read(serial_client_fd, buf, sizeof(buf));
	printf("%s\n", buf);
	if(0 == strncmp(buf, "open_led", 8))
	{
		unsigned char open_led_buf[SERIALBUF] = {0xDD, 0X03, 0X24, 0X00, 0X00};
		write(serial_fd, open_led_buf, sizeof(open_led_buf));
		//printf("write led on over\n");
	}
	if(0 == strncmp(buf, "close_led", 9)){
		unsigned char close_led_buf[SERIALBUF] = {0xDD, 0X03, 0X24, 0X00, 0X01};
		write(serial_fd, close_led_buf, sizeof(close_led_buf));
	}
	if(0 == strncmp(buf, "open_beep", 9)){
		unsigned char open_beep_buf[SERIALBUF] = {0xDD, 0X03, 0X24, 0X00, 0X02};
		write(serial_fd, open_beep_buf, sizeof(open_beep_buf));
	}
	if(0 == strncmp(buf, "close_beep", 10)){
		unsigned char close_beep_buf[SERIALBUF] = {0xDD, 0X03, 0X24, 0X00, 0X03};
		write(serial_fd, close_beep_buf, sizeof(close_beep_buf));
	}
	if(0 == strncmp(buf, "open_fan_s", 10)){
		unsigned char open_fan_s_buf[SERIALBUF] = {0xDD, 0X03, 0X24, 0X00, 0X04};
		write(serial_fd, open_fan_s_buf, sizeof(open_fan_s_buf));
	}
	if(0 == strncmp(buf, "close_fan", 9)){
		unsigned char close_fan_buf[SERIALBUF] = {0xDD, 0X03, 0X24, 0X00, 0X08};
		write(serial_fd, close_fan_buf, sizeof(close_fan_buf));
	}
	return 0;
}

int main(int argc, char *argv[]){
	int ret = -1;
	printf("login_socket_init = %d\n", ret = socket_init());
	if(0 != ret)	return 0;
	printf("sqlite_init = %d\n", ret = sqlite_init());
	if(0 != ret)	return 0;
	printf("serial_init = %d\n", ret = serial_init());
	if(0 != ret)	return 0;
	printf("camera_init = %d\n", ret = camera_init());
	if(0 != ret)	return 0;

	signal(SIGCHLD, recycle_child);
	while(1){

		login_client_fd = accept(login_server_fd, NULL, NULL);
		printf("login client link\n");
		pid_t pid = fork();
		if(0 == pid){
			while(1){
				if(0 == (ret = register_login()))	break;
				printf("%d\n", ret);
			}
			printf("login successful\n");

			serial_client_fd = accept(serial_server_fd, NULL, NULL);
			printf("serial client link\n");

			video_client_fd = accept(video_server_fd, NULL, NULL);
			printf("video client link\n");

			pthread_t camera_thread;
        	pthread_create(&camera_thread, NULL, camera_send_jpeg_to_client, NULL);
        	pthread_detach(camera_thread);

			pthread_t serial_thread;
        	pthread_create(&serial_thread, NULL, serial_send_data_to_client, NULL);
        	pthread_detach(serial_thread);
#if 0
			struct pollfd poll_list[getdtablesize()];
			int intersting_postion = -1;
			poll_list[++intersting_postion].fd = serial_client_fd;	
			poll_list[intersting_postion].events = POLLIN;	

			while(1){
				poll(poll_list, intersting_postion+1, 0);
				for(int i = 0; i < intersting_postion+1; i++){
					if(poll_list[i].revents & POLLIN){
						if(poll_list[i].fd = serial_client_fd){
							recv_client_cmd();
						}
					}
				}
			}
#endif		
			while(1){
				recv_client_cmd();
			}
		}
	}
	
	close(login_server_fd);
    return 0;
}

