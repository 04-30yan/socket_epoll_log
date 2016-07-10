/*************************************************************************
> File Name: socket.cpp
> Author: 
> Mail: 
> Created Time: Sun 29 May 2016 08:59:09 PM CST
************************************************************************/

#include <iostream>
#include <fstream>
#include "mul_socket.h"
#include <string>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <errno.h>

using namespace std;

#define LISTEN_SIZE 20

cSocket::cSocket() 
{
    memset(&addr, 0, sizeof(addr));
}

cSocket::~cSocket() 
{
}

int  cSocket::create() 
{
    socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd == -1) 
    {
        return -1;
    }
    return 0;
}

int cSocket::mul_bind(int port, char* ip) 
{
    this->addr.sin_family = AF_INET;
    int ret = inet_pton(AF_INET, ip, &(addr.sin_addr.s_addr));
    if (ret < 0)
    {
        cout << "inet_pton failed!" << strerror(errno) << endl;
        return -1;
    }
    else if (ret == 0)
    {
        cout << "inet_pton failed!(ip wrong format)" << endl;
        return -1;
    }
    this->addr.sin_port = htons(port);
    if ( (bind(this->socketfd, (struct sockaddr *)(&(this->addr)), sizeof(struct sockaddr))) == -1) 
    {
        return -1;
    }
    return 0;
}

int cSocket::mul_listen() 
{
    if ( (listen(this->socketfd, LISTEN_SIZE)) == -1) 
    {
        return -1;
    }
    return 0;
}

int cSocket::mul_accept() 
{
    unsigned int length = sizeof(struct sockaddr_in);
    cout << "accepting ..." << endl;
    int i = 0;
    int fd = 0;
    while ( (fd = accept(this->socketfd, (struct sockaddr *)(&(this->addr)), &length)) == -1) 
    {
        //由于逻辑错误，重试超过五次退出
        if ( (errno == EINTR) && (i < 5))
        {
            i++;
            continue;
        }
        else
        {
            return -1;
        }

    }
    cout << inet_ntoa(this->addr.sin_addr) << endl;
    return fd;
}

int cSocket::mul_connect(char* ip, int port) 
{
    addr.sin_family = AF_INET;
    int ret = inet_pton(AF_INET, ip, &(this->addr.sin_addr.s_addr));
    if (ret < 0)
    {
        cout << "inet_pton failed!" << strerror(errno) << endl;
        return -1;
    }
    else if (ret == 0)
    {
        cout << "inet_pton failed!(ip wrong format)" << endl;
        return -1;
    }
    addr.sin_port = htons(port);
    //connnection maybe timeout -> alarm
    int i = 0, j = 0;
    while ( (connect(this->socketfd, (struct sockaddr *)(&(this->addr)), sizeof(struct sockaddr))) == -1) 
    {
        if ( (errno == ECONNREFUSED) && (i < 5))
        {
            i++;
            continue;
        }
        if ( (errno == EINTR) && (j < 5))
        {
            j++;
            continue;
        }
        return -1;
    }
    return 0;
}

int cSocket::mul_write(int fd, log_type* buf, int len) 
{
    int i = 0;
    char *ptr;
    ptr = (char*)buf;
    int len_total;
    int len_now = len;
    size_t ret = write(fd, ptr, len_now);
    len_total = ret;
    while (ret <= 0) 
    {
        if ( (errno == EINTR) && (i < 5)) 
        {
            i++;
            ret = write(fd, ptr, len_now);
            continue;
        }
        return ret;
    }
    while (len != len_total && len_total < len) 
    {
        len_now = len_now - ret;
        ptr = ptr + ret;
        ret = write(fd, ptr, len_now);
        if (ret <= 0)
        {
            return ret;
        }
        len_total = len_total + ret;
    }
    return len_total;
}

int cSocket::mul_read(int fd, log_type* buf, int len) 
{
    memset(buf, 0, len);
    int len_now = len;
    char * ptr = (char *)buf;
    memset(ptr, 0, sizeof(ptr));
    size_t ret = read(fd, ptr, len_now);
    int len_total = ret;
    if (ret <= 0) 
    {
        return ret; 
    }

    while (len != len_total && len_total < len)
    {   
        len_now = len_now - ret;
        ptr = ptr + ret;
        ret = read(fd, ptr, len_now);
        if (ret <= 0) 
        {
            return ret;
        }
        len_total = len_total + ret;
    }
    return len_total;
}

