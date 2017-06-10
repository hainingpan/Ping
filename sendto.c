#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<netinet/ip_icmp.h>

#define BUFFER_SIZE 1024
//对ICMP包的验证
u_short cal_cksum(const u_short*addr,register int len,u_short csum)
{
register int nleft=len;
const u_short * w =addr;
register int sum = csum;
//1,16位累加
while(nleft>1)
{
sum+= *w++;
nleft-=2;
}
//2,字节长度为奇数，补零成偶数，并再次累加
if(nleft==1)
{
sum+=htons(*(u_char*)w<<8);
}
//3，字节长度为偶数
sum=(sum>>16)+(sum&0xffff);
sum+=(sum>>16);
//4，取反，判断
return ~sum;
}
int main(int argc,char*argv[])
{
int fd,tmp;
struct sockaddr_in sin;

int buf_size;
int buf2_size;
char * buf;
char * buf2;
int len;
struct iphdr *ip_header;
struct icmphdr *icmp_header;
//创建套接字
fd = socket(PF_INET,SOCK_RAW,IPPROTO_ICMP);
if(fd<0)
{
printf("create socket error\n");
return -1;
}
//绑定套接字
memset(&sin,0,sizeof(sin));
sin.sin_port = htons(1111);
if( (bind(fd,(struct sockaddr*)&sin,sizeof(sin) ))<0)
{
printf("bind fail\n");
close(fd);
return -1;
}
//设置IP
tmp=1;
setsockopt(fd,0,IP_HDRINCL,&tmp,sizeof(tmp));
//设置IP过程
//1,申请一块内存放Ip数据=IP头+ICMP包头长度+时间搓长度
buf_size = sizeof(struct iphdr)+sizeof(struct icmphdr)+sizeof(struct timeval);
buf2_size=buf_size;
buf = (char*)malloc(buf_size);
buf2 = (char*)malloc(buf2_size);
//2，填充IP头信息
memset(buf,0,buf_size);

memset(&sin,0,sizeof(sin));
sin.sin_family = AF_INET;
sin.sin_addr.s_addr= inet_addr("192.168.1.102");

ip_header =(struct iphdr*)buf;
ip_header->ihl = 5;//ip包头长度
ip_header->version=4;//版本
ip_header->tos=0;
ip_header->tot_len=htons(buf_size);
ip_header->id=rand();
ip_header->frag_off=0x40;
ip_header->ttl=64;
ip_header->protocol=IPPROTO_ICMP;
ip_header->check=0;
ip_header->saddr=0;
ip_header->daddr=inet_addr("192.168.0.2");

//设置ICMP
icmp_header=(struct icmphdr*)(ip_header+1);
icmp_header->type=ICMP_ECHO;
icmp_header->code=0;
icmp_header->un.echo.id=htons(1111);
icmp_header->un.echo.sequence=0;

icmp_header->checksum=
cal_cksum( (const u_short*)icmp_header,\
sizeof(struct icmphdr)+sizeof(struct timeval),(u_short)0) ;

//发送
if(sendto(fd,buf,buf_size,0,( struct sockaddr* )&sin,sizeof(struct sockaddr_in) )<0)
{
return -1;
}
//接收
if(recvfrom(fd,buf2,buf2_size,0,(struct sockaddr*)&sin,&len)<0)
{
return -1;
}
printf("received\n");
return 0;
}