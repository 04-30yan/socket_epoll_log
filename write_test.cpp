/*************************************************************************
	> File Name: write_tt.cpp
	> Author: 
	> Mail: 
	> Created Time: Wed 06 Jul 2016 12:44:19 PM CST
 ************************************************************************/

#include<iostream>
#include <fstream>
#include<unistd.h>
using namespace std;
int main(int argv, char *argc[])
{
    time_t now = time(0);
    time_t end;
    struct tm *p,*p1;
    p = localtime(&now);
    char tmp[64];
    snprintf(tmp, 64, "dispatch_log0.%d%02d%02d%02d%d.log", (1900 + p->tm_year), (1 + p->tm_mon), p->tm_mday, p->tm_hour, (p->tm_min / 10));
    ofstream ofs(tmp, fstream::app | fstream::binary);
    int i = 47;
    while (true)
    {
        end = time(0);
        p1 = localtime(&end);
        if (p1->tm_min != p->tm_min)
        {
            ofs.close();
            snprintf(tmp, 64, "dispatch_log0.%d%02d%02d%02d%d.log", (1900 + p->tm_year), (1 + p->tm_mon), p->tm_mday, p->tm_hour, (p->tm_min / 10));
            ofstream ofs(tmp, fstream::app | fstream::binary);
        }
        ofs << "[2016-06-06 08:49:59] 163109342	2112	102	2	vhot2.qqvideo.tc.qq.com	x0304p83c8m.p712.1.mp4	19922785	1	9b4f5fc40b251cdc93e8b3c1949ddf6b08208d28	0	ADD_DIS	1	1465174184	1465174197	1465174199	30	0	163.177.158.27	47" << flush;
        i++;
        sleep(5);
        ofs << "\t" << i << endl;
    }
}
