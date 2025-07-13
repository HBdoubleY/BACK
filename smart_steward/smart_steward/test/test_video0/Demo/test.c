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
#include <stdlib.h>
#include "yuv_to_jpeg.h"
 
#define IMAGE_WIDTH 320
#define IMAGE_HIGHT 240

typedef struct __video_buffer
{
    void *start;
    size_t length;
}req_op;
req_op image_buffer[4];

int video_fd;
 
/*
函数功能: 摄像头设备初始化
*/
int Video_Device_Init(char *DEVICE_NAME)
{
	/*1. 打开摄像头设备*/
	video_fd=open(DEVICE_NAME, O_RDWR | O_NONBLOCK);
	if(video_fd < 0)  return -1;
	
    //1.1当前视频设备支持的视频图像格式
	struct v4l2_fmtdesc fmt;
    memset(&fmt,0,sizeof(fmt));
	fmt.index = 0;
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	printf("当前摄像头支持输出的图像格式如下:\n");
	while(ioctl(video_fd, VIDIOC_ENUM_FMT, &fmt) != -1){
        printf("\t%d.%s\n", fmt.index+1, fmt.description);  // index是查询的格式序号, description是格式名称
        fmt.index++;
    }

	/*2 设置摄像头的属性*/
	struct v4l2_format set_format;
    memset(&set_format, 0, sizeof(set_format));
    set_format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;          /*表示视频捕获设备*/
	set_format.fmt.pix.width = IMAGE_WIDTH;                 /*预设的宽度*/
	set_format.fmt.pix.height = IMAGE_HIGHT;                /*预设的高度*/
#if 0   
	set_format.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;     /*预设的格式    YUYV    */
//    set_format.fmt.pix.field = V4L2_FIELD_ANY;            /*系统自动设置: 帧属性*/
//    set_format.fmt.pix.field = V4L2_FIELD_INTERLACED;     /*系统自动设置: 帧属性*/
    set_format.fmt.pix.field = V4L2_FIELD_NONE; 
#else
	set_format.fmt.pix.pixelformat = V4L2_PIX_FMT_JPEG;     /*预设的格式    JPEG    */
//    set_format.fmt.pix.field = V4L2_FIELD_INTERLACED;     /*系统自动设置: 帧属性*/
    set_format.fmt.pix.field = V4L2_FIELD_ANY;              
#endif

	if(ioctl(video_fd, VIDIOC_S_FMT, &set_format))  return -2;

    //得到图片格式
    struct v4l2_format get_format;
    memset(&get_format, 0, sizeof(get_format));
    get_format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ioctl(video_fd, VIDIOC_G_FMT, &get_format);
    if(get_format.fmt.pix.pixelformat == V4L2_PIX_FMT_YUYV)   printf("当前摄像头支持YUV格式图像输出!\n");
    printf("fmt.type:\t\t%d\n", get_format.type);
    printf("pix.pixelformat:\t%c%c%c%c\n",
        get_format.fmt.pix.pixelformat & 0xFF,
        (get_format.fmt.pix.pixelformat >> 8) & 0xFF, 
        (get_format.fmt.pix.pixelformat >> 16) & 0xFF,
        (get_format.fmt.pix.pixelformat >> 24) & 0xFF);
    printf("pix.width:\t\t%d\n", get_format.fmt.pix.width);
    printf("pix.height:\t\t%d\n", get_format.fmt.pix.height);
    printf("pix.field:\t\t%d\n", get_format.fmt.pix.field);

	
	/*2.3 设置摄像头采集的帧率*/
	struct v4l2_streamparm set_streamparm;
    memset(&set_streamparm, 0, sizeof(set_streamparm));
	set_streamparm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;      /*表示视频捕获设备*/
	set_streamparm.parm.capture.timeperframe.numerator = 1;
	set_streamparm.parm.capture.timeperframe.denominator = 30;
	printf("设置当前摄像头采集帧率: %d秒%d帧\n", 
                set_streamparm.parm.capture.timeperframe.numerator,
                set_streamparm.parm.capture.timeperframe.denominator);
	/*设置摄像头的帧率*/
    if(ioctl(video_fd, VIDIOC_S_PARM, &set_streamparm))   return -3;
    /*获取摄像头的帧率*/

    struct v4l2_streamparm get_streamparm;
    memset(&get_streamparm, 0, sizeof(get_streamparm));
    get_streamparm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if(ioctl(video_fd, VIDIOC_G_PARM, &get_streamparm))   return -3;
	printf("当前摄像头实际采集帧率: %d秒%d帧\n",
                get_streamparm.parm.capture.timeperframe.numerator,
                get_streamparm.parm.capture.timeperframe.denominator);
	

    //利用VIDIOC_G_CTRL得到一些设置 如亮度，饱和度，曝光时间，帧数，增益，白平衡等



	/*3. 请求缓冲区: 申请摄像头数据采集的缓冲区*/
	struct v4l2_requestbuffers set_req;
	memset(&set_req, 0, sizeof(set_req));
	set_req.count = 4;                              /*预设要申请4个缓冲区*/
	set_req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;     /*视频捕获设备*/
	set_req.memory = V4L2_MEMORY_MMAP;              /*支持mmap内存映射*/
    /*申请缓冲区*/
	if(ioctl(video_fd, VIDIOC_REQBUFS, &set_req))       return -4;

	printf("摄像头缓冲区申请的数量: %d\n", set_req.count);

	/*4. 获取缓冲区的详细信息: 地址,编号*/
	struct v4l2_buffer req_info;
    //get req ip index length
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
        //printf("%x\n", req_info.m.offset);
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
 
int main(int argc,char argv[]){
	
	/*1. 初始化摄像头设备*/
	int ret = Video_Device_Init("/dev/video0");
	printf("Video_Device_Init = %d\n", ret);

#if 1
	/*2. 循环读取摄像头采集的数据*/
	struct pollfd poll_list[getdtablesize()];
    int intersting_postion = -1;				        //初始长度为-1 才能从起点添加文件设备
    poll_list[++intersting_postion].fd = video_fd;	    //加入需要监视的文件设备
    poll_list[intersting_postion].events = POLLIN;	    //表示感兴趣
#endif

	/*3. 申请存放JPG的数据空间*/
	unsigned char jpg_p[ IMAGE_WIDTH * IMAGE_HIGHT * 4 ] = {0};
	//char *jpg_p = NULL;

	struct v4l2_buffer video_buffer;

    char jpg_file_name[20] = {0};                       /*存放JPG图片名称*/
	int jpg_cnt = 0;
	int jpg_size = 0;
    int jpg_fd = -1;
    int temp = 0;
	while(1){
		/*(1)等待摄像头采集数据*/
#if 1
		poll(poll_list, intersting_postion+1, -1);
#endif
        memset(&video_buffer, 0, sizeof(video_buffer));

        /*(2)取出队列里采集完毕的缓冲区*/
        video_buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; /*视频捕获设备*/
        video_buffer.memory = V4L2_MEMORY_MMAP;
        ioctl(video_fd, VIDIOC_DQBUF, &video_buffer);
    
        printf("image_buffer[%d]=%p\n",
                video_buffer.index,
                image_buffer[video_buffer.index].start);
        temp = video_buffer.index;
    
        /*YUV数据转JPEG格式*/
        sprintf(jpg_file_name,"./%d.jpeg", jpg_cnt++);
        jpg_fd = open(jpg_file_name, O_WRONLY | O_CREAT, 0777);
#if 0
        memset(&jpg_p, 0, sizeof(IMAGE_WIDTH * IMAGE_HIGHT * 4));
        jpg_size = yuv_to_jpeg(IMAGE_WIDTH,
                            IMAGE_HIGHT,
                            strlen(image_buffer[video_buffer.index].start),
                            image_buffer[temp].start,
                            jpg_p,
                            80);
        /*(3)处理图像数据*/

        printf("图片名称:%s,字节大小:%d\n", jpg_file_name, jpg_size);
    

        write(jpg_fd, jpg_p, jpg_size);
#else

        write(jpg_fd, image_buffer[temp].start, video_buffer.bytesused);
        printf("%ld\n", image_buffer[temp].length);
#endif
        close(jpg_fd);

        /*(4)将缓冲区再放入队列*/
        memset(&video_buffer, 0, sizeof(video_buffer));
        video_buffer.index = temp;
        video_buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        video_buffer.memory = V4L2_MEMORY_MMAP;
        ioctl(video_fd, VIDIOC_QBUF, &video_buffer);
                                        
//      }
        sleep(1);
	}

    enum v4l2_buf_type Type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ioctl(video_fd, VIDIOC_STREAMOFF, &Type);
    
    for(int i = 0; i < 4; i++){
        munmap(image_buffer[i].start, image_buffer[i].length);
    }
    close(video_fd);
	return 0;
}