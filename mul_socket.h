/*************************************************************************
	> File Name: socket.h
	> Author: 
	> Mail: 
	> Created Time: Sun 29 May 2016 08:52:10 PM CST
 ************************************************************************/

#ifndef _SOCKET_H
#define _SOCKET_H
#include <netinet/in.h>

using namespace std;

typedef struct {
    unsigned int task_id;
    unsigned int oc_id;
    unsigned int ocgroup_id;
    unsigned int oc_type;
    char domain[32];
    char file_name[64];
    unsigned long long file_size;
    char hash_type[4];
    char hash_value[64];
    unsigned int dst_zone_id;
    char dispatch_type[8];
    int operate;
    unsigned int main_create_time;
    unsigned int main_start_time;
    unsigned int start_time;
    unsigned int end_time;
    unsigned int status;
    unsigned int errcode;
    char oc_ip[32];
    unsigned int success_rate;
    unsigned int complete_rate;
}log_type;


class cSocket
{
public:
    cSocket();
    virtual  ~cSocket();
    virtual int create();
    virtual int mul_bind(int port, char *ip);
    virtual int mul_listen();
    virtual int mul_accept();
    virtual int mul_connect(char* ip, int port);
    virtual int mul_write(int fd, log_type* logtype, int len);
    virtual int mul_read(int fd, log_type* logtype, int len);
    int getsocketfd()
    {
        return this->socketfd;
    }

private:
    struct sockaddr_in addr;
    int socketfd;

};

#endif
