/********************************************************
*@file   Socket.cpp
*@Author rongweihe
*@Data   2019/03/31
*********************************************************/
#include "Socket.h"
#include "Util.h"
#include <bits/stdc++.h>
#include <sys/types.h>
#include <sys/socket.h>
void setReusePort(int fd)
{
    // getsockopt / setsockopt ��ȡ/������ĳ���׽��ֹ�����ѡ�� setsockopt : �˿ڸ���
    int opt = -1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&opt, sizeof(opt));
}

/*
ͨ���׶���� �������ĸ�����
����һ�� socket �൱����һ���ֻ�
bind            �൱�ڸ��ֻ��绰��
listen          �൱�ڵȴ�����
accept          �൱�ڽ���֮��
*/


/*
UNIX/Linux ��һ����ѧ��:���ж��������ļ���
socketҲ������,�����ǿɶ�����д���ɿ��ơ��ɹرյ��ļ���������
socket ϵͳ���ÿɴ���һ�� socket:
#include <sys/types.h>
#include <sys/socket.h>
int socket( int domain, int type, int protocol );

domain��������ϵͳʹ���ĸ��ײ�Э���塣
��TCP/IPЭ�������,�ò���Ӧ������ΪPF INET (Protocol Family of Internet,����IPv4)��PF-INET6 (����IPv6) ;
����UNIX������Э�������,�ò���Ӧ������ΪPF UNIX,����socketϵͳ����֧�ֵ�����Э����,������Լ��ο���man�ֲᡣ
type����ָ���������͡�
����������Ҫ��SOCK STREAM����(������)��SOCKUGRAM (���ݱ�)����
��TCP/IPЭ�������,��ֵȡSOCK STREAM��ʾ�����ʹ��TCPЭ��,ȡSOCK DGRAM��ʾ�����ʹ��UDPЭ�顣

ֵ��ָ������, ��Linux�ں˰汾2.6.17��, type�������Խ���������������������������Ҫ�ı�־�����ֵ:
SOCK-NONBLOCK��SOCKCLOEXEC,���Ƿֱ��ʾ���´�����socket��Ϊ��������,
�Լ��� fork ���ô����ӽ���ʱ���ӽ����йرո�socket,���ں˰汾2.6.17֮ǰ��Linux��,�ļ������������������Զ���Ҫʹ�ö����ϵͳ����(����fcntl)�����á�
protocol ��������ǰ�����������ɵ�Э�鼯����,��ѡ��һ�������Э�顣
�������ֵͨ������Ψһ��(ǰ���������Ѿ���ȫ����������ֵ)�����������������,���Ƕ�Ӧ�ð�������Ϊ0,��ʾʹ��Ĭ��Э�顣
socket ϵͳ���óɹ�ʱ����һ�� socket �ļ�������,ʧ���򷵻� -1 ������ errno
*/
ServerSocket::ServerSocket(int port, const char *ip) : mPort(m_port), mIp(m_ip)
{
    //����һ�� Ipv4 socket ��ַ
    bzero(&m_addr,sizeof(m_addr));
    //sin_family = ��ַ�壬AF_INET = TCP/Ipv4 Э����
    m_addr.sin_family = AF_INET;
    m_addr.sin_port   = htons(port);
    if(m_ip != nullptr)
    {
        //���ַ�����ʾ��IP��ַת��Ϊ�����ֽ���������ʾ��IP��ַ
        ::inet_pton(AF_INET,m_ip,&m_addr.sin_addr);
    }
    else
    {
        m_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1)
    {
        std::cout << "creat socket error in file <" << __FILE__ << "> "<< "at " << __LINE__ << std::endl;
        exit(0);
    }
}
//------------------------------------------���� socket
/*
���� socket ʱ,���Ǹ���ָ���˵�ַ��,���ǲ�δָ��ʹ�øõ�ַ���е��ĸ����� socket ��ַ��
��һ��socket��socket��ַ�󶨳�Ϊ��socket�������ڷ�����������,����ͨ��Ҫ����socket,��Ϊֻ��������ͻ��˲���֪����������������ͻ�����ͨ������Ҫ����socket,���ǲ���������ʽ,��ʹ�ò���ϵͳ�Զ������socket��ַ������socket��ϵͳ������bind,�䶨������:
#include <sys/types.h>
#include <sys/socket.h>
int bind( int sockfd, const struct sockaddr* my addr, socklen t addrlen );
bind��my addr��ָ��socket��ַ�����δ������sockfd�ļ�������, addrlen����ָ����socket��ַ�ĳ���,
bind�ɹ�ʱ����0,ʧ���򷵻�-1������errno��
�������ֳ�����errno��EACCES��EADDRINUSE,���ǵĺ���ֱ���:
OEACCES,���󶨵ĵ�ַ���ܱ����ĵ�ַ,�������û��ܹ����ʡ�������ͨ�û���socket�󶨵�֪������˿�(�˿ں�Ϊ0-1023)��ʱ, bind������EACCES����
�� EADDRINUSE,���󶨵ĵ�ַ����ʹ���С����罫socket�󶨵�һ������TIME.WAIT״̬��socket��ַ��
*/
void ServerSocket::bind()
{
    int ret = ::bind(listen_fd, (struct sockaddr*)&m_addr, sizeof(m_addr));
    if (ret == -1)
    {
        std::cout << "bind error in file <" << __FILE__ << "> "<< "at " << __LINE__ << std::endl;
        exit(0);
    }
}
//------------------------------------------ ���� socket
/*
socket������֮��,���������Ͻ��ܿͻ�����,������Ҫʹ������ϵͳ����������һ�����������Դ�Ŵ�����Ŀͻ�����:
#include <sys/socket.h>
int listen( int sockfd, int backlog ).
sockfa����ָ����������socket, backlog������ʾ�ں˼������е���󳤶ȡ�
�������еĳ����������backlog,���������������µĿͻ�����,�ͻ���Ҳ���յ�ECONNREFUSED������Ϣ��
���ں˰汾2.2֮ǰ��Linux��, backlog������ָ���д��ڰ�����״̬(SYN RCVD)����ȫ����״̬(ESTABLISHED)��socket�����ޡ�
�����ں˰汾2.2֮��,��ֻ��ʾ������ȫ����״̬��socket������,
���ڰ�����״̬��socket����������/proc/sys/net/ipv4/tcp-maxsyn-backlog�ں˲������塣backlog�����ĵ���ֵ��5.
listen�ɹ�ʱ����0,ʧ���򷵻�-1������errno.
*/
void ServerSocket::listen()
{
    int ret = ::listen(listen_fd, 1024);
    if (ret == -1)
    {
        std::cout << "listen error in file <" << __FILE__ << "> "<< "at " << __LINE__ << std::endl;
        exit(0);
    }
}


/*------------------------------------------accept ϵͳ����
�� listen ���������н���һ������:
#include <sys/types.h>
#include <sys/socket.h>
int accept( int sockfd, struct sockaddr addr, socklen t *addrlen );
sockfd������ִ�й�listenϵͳ���õļ���sockete,
addr����������ȡ���������ӵ�Զ��socket��ַ,
��socket��ַ�ĳ�����addrlen����ָ��
accept�ɹ�ʱ����һ���µ�����socket,��socketΨһ�ر�ʶ�˱����ܵ��������,
��������ͨ����д��socket���뱻�������Ӷ�Ӧ�Ŀͻ���ͨ��. acceptʧ��ʱ����-1������errno.
*/
//�ɻ�
void ServerSocket::accept(ClientSocket &clientSocket) const
{
    int clientfd = ::accept(listen_fd, NULL, NULL);
    if (clientfd < 0)
    {
        if ((errno == EWOULDBLOCK) || (errno == EAGAIN))
            return clientfd;
        std::cout << "accept error in file <" << __FILE__ << "> "<< "at " << __LINE__ << std::endl;
        std::cout << "clientfd:" << clientfd << std::endl;
        perror("accpet error");
        //exit(0);
    }
    //std::cout << "accept a client�� " << clientfd << std::endl;
    clientSocket.fd = clientfd;
    return clientfd;
}

void ServerSocket::close()
{
    if (listen_fd >= 0)
    {
        ::close(listen_fd);
        //std::cout << "��ʱ����ʱ�ر�, �ļ�������:" << listen_fd << std::endl;
        listen_fd = -1;
    }


}
ServerSocket::~ServerSocket()
{
    close();
}

void ClientSocket::close()
{
    if (fd >= 0)
    {
        //std::cout << "�ļ��������ر�: " << fd <<std::endl;
        ::close(fd);
        fd = -1;
    }
}
ClientSocket::~ClientSocket()
{
    close();
}
