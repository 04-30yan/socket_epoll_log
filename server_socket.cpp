/*************************************************************************
	> File Name: server_socket.cpp
	> Author: 
	> Mail: 
	> Created Time: Sun 31 May 2016 10:41:52 AM CST
 ************************************************************************/

#include <iostream>
#include "mul_socket.h"
#include <string>
#include <sys/select.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/epoll.h>
#include <string.h>
#include <string>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <map>

using namespace std;
#define EPOLLFD_SIZE 30
#define EVENTS_SIZE 30

int read_file(map<int, int>*operate_files, map<string, int> *file_lines, map<string,FILE*>*file_fds, log_type *logtype);

int main(int argc, char* argv[])
{
    if(argc < 3)
    {
        cout << "argv error,please cin portnumber!" << endl;
        return -1;
    }

    int portnumber;
    if((portnumber = atoi(argv[2])) < 0)
    {
        cout << "portnumber error,please cin correct portnumber!" << endl;
		return -1;
    }
    cSocket *socket = new cSocket();
    int ret = socket->create();
	if (ret == -1)
	{
		return -1;
	}
	int socketfd = socket->getsocketfd();
    ret = socket->mul_bind(portnumber, argv[1]);
    if (ret == -1)
	{
        printf("bind:%s\n", strerror(errno));
	    close(socketfd);
		delete socket;
		return -1;
	}
	
    log_type logtype;
    bzero(&logtype, sizeof(logtype));

    int epollfd;
    struct epoll_event events[EVENTS_SIZE], event;
    memset(events, 0, sizeof(events));

    epollfd = epoll_create(EPOLLFD_SIZE);
    int cir_i = 0;
	while ((epollfd == -1) && (cir_i < 5))
	{
	    cout << "epoll create failed!" << strerror(errno) << endl;
        epollfd = epoll_create(EPOLLFD_SIZE);
        cir_i++;
	}
	
    event.events =  EPOLLIN;
    event.data.fd = socketfd;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, socketfd, &event);
    
    ret = socket->mul_listen();
    if (ret == -1)
	{
        printf("listen err:%s\n",strerror(errno));
	}

    map<int, int> operate_files;
    map<string, int> file_lines;
    map<string, FILE*> file_fds;
    while(true)
    {
        int num =  epoll_wait(epollfd, events, EVENTS_SIZE, -1);
        for(int i = 0; i < num; i++)
        { 
            if(events[i].data.fd == socketfd)
            {
                int fd = socket->mul_accept();
				if (fd == -1)
                {
                    printf("accept error%s\n", strerror(errno));
                }
                else
                {
                    int flags = fcntl(fd, F_GETFL, 0);
                    if (flags == -1)
                    {
                        printf("fcnl faild!\n");
                    }
                    else
                    {
                        if ( (fcntl(fd, F_SETFL, flags|O_NONBLOCK)) == -1)
                        {
                            printf("fcntl NONBLOCK failed!");
                        }
                    }
                    event.data.fd = fd;
                    event.events = EPOLLIN;
                    ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
                    if (ret == -1)
                    {
                        printf("epoll_ctl error\n");
                        close(fd);
                    }
                }
            }

            else if(events[i].events & EPOLLIN)
            {
                int fd = events[i].data.fd;
                struct epoll_event ev;
                ev.events = 0;
                ret =  socket->mul_read(fd, &logtype, sizeof(logtype));
                if (ret <= 0) 
                {
                    printf("read:%s\n", strerror(errno));
                    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &ev);
                    close(fd);
                    events[i].data.fd = -1;
                }else
                {
                    read_file(&operate_files, &file_lines,&file_fds, &logtype);
                }
                
            } 
            else if(events[i].events & EPOLLOUT)
            {
                int fd;
                struct epoll_event ev;
                ev.data.fd = fd;
                ev.events = EPOLLIN;
                fd = events[i].data.fd;
                ret = socket->mul_write(fd, &logtype, sizeof(logtype));
                if (ret <= 0) 
                {
                    printf("write:%s\n", strerror(errno));
                    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &ev);
                    close(fd);
                    events[i].data.fd= -1;
                }
                event.data.fd = fd;
                event.events = EPOLLIN;
                epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
            }
            else if (events[i].events & EPOLLERR || events[i].events & EPOLLHUP)
            {
                printf("close %d\n", events[i].data.fd );
                close(events[i].data.fd);
            }
        } 
    }

    close(epollfd);
    close(socketfd);
    delete socket;

    return 0;
}
int read_file(map<int, int>*operate_files, map<string, int> *file_lines, map<string,FILE*>*file_fds, log_type *logtype)
{

    int ocgroup_id = logtype->ocgroup_id;
    int file_now = 0;
    char file_name[64] = {0};
    FILE *file_fd = NULL;
    while (true)
    {
        file_now = (*operate_files)[ocgroup_id];
        bzero(file_name,64);
        snprintf(file_name, 64, "%d_%d.log", ocgroup_id, file_now);
        map<string, FILE*>::iterator it = (*file_fds).find(file_name);
        if (it != (*file_fds).end())
        {
        file_fd = (*it).second;
        }else
        {
            int i = 0;
            while ( ((file_fd = fopen(file_name, "wb+")) == NULL) && (i < 5))
            {
                printf("%s open failed!%s\n",file_name, strerror(errno));
                i++;
                return -1;
            }
            (*file_fds).insert(map<string, FILE*>::value_type(file_name, file_fd));
        }

        int lines_start = (*file_lines)[file_name];

        if ( lines_start < 100 )
        {
            fprintf(file_fd, "%u\t%u\t%u\t%u\t%s\t%s\t%lld\t%s\t%s\t%u\t%s\t%d\t%u\t%u\t%u\t%u\t%u\t%u\t%s\t%u\t%u\n", 
                    logtype->task_id, logtype->oc_id, logtype->ocgroup_id, logtype->oc_type, logtype->domain, logtype->file_name, 
                    logtype->file_size, logtype->hash_type, logtype->hash_value, logtype->dst_zone_id, logtype->dispatch_type, 
                    logtype->operate, logtype->main_create_time, logtype->main_start_time, logtype->start_time, logtype->end_time, 
                    logtype->status, logtype->errcode, logtype->oc_ip, logtype->success_rate, logtype->complete_rate);
            fseek(file_fd,0,SEEK_END);
            printf("%u\t%u\t%u\t%u\t%s\t%s\t%lld\t%s\t%s\t%u\t%s\t%d\t%u\t%u\t%u\t%u\t%u\t%u\t%s\t%u\t%u\n", 
                    logtype->task_id, logtype->oc_id, logtype->ocgroup_id, logtype->oc_type, logtype->domain, logtype->file_name, 
                    logtype->file_size, logtype->hash_type, logtype->hash_value, logtype->dst_zone_id, logtype->dispatch_type, 
                    logtype->operate, logtype->main_create_time, logtype->main_start_time, logtype->start_time, logtype->end_time, 
                    logtype->status, logtype->errcode, logtype->oc_ip, logtype->success_rate, logtype->complete_rate);
            (*file_lines)[file_name] = (*file_lines)[file_name] + 1;
            break;
        }
        else
        {
            (*operate_files)[ocgroup_id] = (*operate_files)[ocgroup_id] + 1;
            fclose(file_fd);
        }
    } 

return 0;
}
