//
// Created by 13345 on 2023/6/30.
//

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include "processpool.h"

/**用于处理客户CGI请求的类，它可以作为processpool类的模板参数*/
class cgi_conn
{
public:
    cgi_conn() {}
    ~cgi_conn() {}
    /**初始化客户连接，清空该缓冲区*/
    void init(int epollfd, int sockfd, const sockaddr_in& cliend_addr)
    {
        m_epollfd = epollfd;
        m_sockfd = sockfd;
        m_address = cliend_addr;
        memset(m_buf, '\0', BUFFER_SIZE);
        m_read_max = 0;
    }
    void process()
    {

    }

private:
    /**读该缓冲区的大小*/
    static const int BUFFER_SIZE = 1024;
    static int m_epollfd;
    int m_sockfd;
    sockaddr_in m_address;
    char m_buf[BUFFER_SIZE];
    /**标记该缓冲区中已经读入的客户数据的最后一个字节的下一个位置*/
    int m_read_max;
};

int cgi_conn::m_epollfd = -1;