/*************************************************************************
	> File Name: client_socket.cpp
	> Author: 
	> Mail: 
	> Created Time: Sun 31 May 2016 10:41:52 AM CST
 ************************************************************************/

#include <iostream>
#include <fstream>
#include "mul_socket.h"
#include <string>
#include <sys/select.h>
#include <sys/time.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <vector>

using namespace std;
#define EPOLLFD_SIZE 30
#define EVENTS_SIZE 30

int to_struct(log_type *logtype, char *tmp_log[]);
int file_open(ifstream *ifs, int i, struct tm *p);
int getline(ifstream *ifs, char *buf, int len);
int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        cout << "argv error,please cin ip or portnumber or filename!!!" << endl;
        return -1;
    }
    
    int portnumber;
    if ( (portnumber = atoi(argv[2])) < 0)
    {
        cout << "portnumber error,please cin correct portnumber!" << endl;
		return -1;
    }
    cSocket *socket = new cSocket();
    int i = socket->create();
	
	if (i == -1) 
	{
        printf("socket open failed:%s\n", strerror(errno));
		return -1;
	}
	
	int socketfd = socket->getsocketfd();

    int j = socket->mul_connect(argv[1], portnumber);
	if (j == -1)
	{
        printf("connect:%s\n",strerror(errno));
		close(socketfd);
		delete socket;
	    return -1;
	}
    

    ifstream ifs;
    time_t now = time(0);
    struct tm *p = localtime(&now);
    if(file_open(&ifs, fstream::in, p) == -1)
    {
        printf("connot open file!\n");
        return -1;
    }

    log_type logtype;
    bzero(&logtype, sizeof(logtype));
    char buf[512] = {0};
    size_t seek = 0;
    while (true)
    {
        if(ifs.peek() == EOF)
        {
            now  = time(0);
            struct tm *p1 = localtime(&now);
            if(p1->tm_min != p->tm_min)
            {
                ifs.close();
                if (file_open(&ifs, fstream::in, p1) == -1)
                {
                    printf("connot open file!\n");
                    return -1;
                }
                continue;
            }
            else{
                ifs.clear();
                sleep(1);
                ifs.seekg(seek, ios::beg);
                continue;
            }
        }
        else{
            getline(&ifs, buf, sizeof(buf));  
            cout << buf << endl;
            if (buf  != NULL)
            {
            char* tmp = NULL;
            char * tmp_log[22];
            memset(tmp_log, 0, sizeof(tmp_log));
            char *str = buf;
            int i = 0;
            while ((tmp = strsep(&str, "'\t'|' '")) != NULL)
            {
                tmp_log[i++] = tmp;
            }
            bzero(&logtype, sizeof(logtype)); 
            to_struct(&logtype, tmp_log);
            int ret = socket->mul_write(socketfd, &logtype, sizeof(logtype));
            if (ret == -1) 
            {
                printf("write error %s\n", strerror(errno));
                close(socketfd);
                delete socket;
                return -1;
            }
            }
            seek = ifs.tellg();
        }
    }
    
    ifs.clear();
    

    close(socketfd);
    delete socket;
    return 0;
}
int to_struct(log_type *logtype, char *tmp_log[])
{

    logtype->task_id = atoi(tmp_log[2]);
    logtype->ocgroup_id = atoi(tmp_log[4]);
    logtype->oc_type = atoi(tmp_log[5]);
    strncpy(logtype->domain, tmp_log[6], strlen(tmp_log[6]));
    strncpy(logtype->file_name, tmp_log[7], strlen(tmp_log[7]));
    logtype->file_size = atoi(tmp_log[8]);
    strncpy(logtype->hash_type, tmp_log[9], strlen(tmp_log[9]));
    strncpy(logtype->hash_value, tmp_log[10], strlen(tmp_log[10]));
    logtype->dst_zone_id = atoi(tmp_log[11]);
    strncpy(logtype->dispatch_type, tmp_log[12], strlen(tmp_log[12]));
    logtype->operate = atoi(tmp_log[13]);
    logtype->main_create_time = 0;
    logtype->main_start_time = atoi(tmp_log[14]);
    logtype->start_time = atoi(tmp_log[15]);
    logtype->end_time = atoi(tmp_log[16]);
    logtype->status = atoi(tmp_log[17]);
    logtype->errcode = atoi(tmp_log[18]);
    strncpy(logtype->oc_ip, tmp_log[19], strlen(tmp_log[19]));
    logtype->success_rate = atoi(tmp_log[20]);
    logtype->complete_rate = atoi(tmp_log[21]);
}
int file_open(ifstream *ifs, int i, struct tm *p)
{
    char tmp[64];
    snprintf(tmp, 64, "dispatch_log0.%d%02d%02d%02d%d.log", (1900 + p->tm_year), (1 + p->tm_mon), p->tm_mday, p->tm_hour, (p->tm_min / 10));
    if (i & fstream::in)
    {
        ifs->open(tmp, fstream::in | fstream::binary);
    }
    else{
        ifs->open(tmp, fstream::out | fstream::binary);
    }
    while (!ifs->is_open())
    {
        if (errno & EACCES)
        {
            printf("%s\t%s\n", tmp, strerror(errno));
            return -1;
        }
        if (errno & EEXIST)
        { 
            printf("%s\t%s\tcreated file!\n", tmp, strerror(errno));
            file_open(ifs, fstream::out, p);
        }
        if (errno & EAGAIN)
        {
            continue;
        }
        printf("%s\t%s\n", tmp, strerror(errno));
        return -1;
    }
    return 0;
}
int getline(ifstream *ifs, char *buf, int len)
{
    char c = '0';
    int i = 0;
    bzero(buf, len);
    while(true)
    {
        if (c == '\n')
        {
            break;
        }
        else
        {
            if (ifs->get(c) != NULL)
            {
                buf[i++] = c;
            }
            if(ifs->peek() == EOF && ifs->get(c) == NULL)
            {
                ifs->clear();
                sleep(1);
                continue;
            }
        }
    }
}
