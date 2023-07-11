/********************************************************
*@file   Epoll.cpp
*@Author rongweihe
*@Data   2019/03/31
*********************************************************/
#include "Epoll.h"
#include "Util.h"
#include <bits/stdc++.h>
#include <sys/epoll.h>

std::unordered_map<int, std::shared_ptr<HttpData> > Epoll::http_data_map_;
const int Epoll::kMaxEvents = 10000;
epoll_event *Epoll::events_;

/// �ɶ� | ETģ | ��֤һ��socket��������һʱ��ֻ��һ���̴߳���
const __uint32_t Epoll::kDefaultEvents =  (EPOLLIN | EPOLLET | EPOLLONESHOT);

TimerManager Epoll::timer_manager_;

///��ʼ�� epoll_create ��������һ��������ļ�����������Ψһ��ʶ�ں��е��¼���
int Epoll::init(int max_events)
{
    ///max_events �������ں�һ����ʾ���������¼�����Ҫ���
    ///���ص��ļ������� epoll_fd ���������������� epoll ϵͳ���õĵ�һ������
    int epoll_fd = ::epoll_create(max_events);
    if (epoll_fd == -1)
    {
        std::cout << "epoll create error" << std::endl;
        exit(-1);
    }
    events_ = new epoll_event[max_events];
    return epoll_fd;
}

int Epoll::addfd(int epoll_fd, int fd, __uint32_t events, std::shared_ptr<HttpData> httpData)
{
    epoll_event event;
    event.events = (EPOLLIN|EPOLLET);
    event.data.fd = fd;
    /// ����http_data_map_
    http_data_map_[fd] = httpData;
    /// epoll_ctl �����ļ���������������������������
    /// EPOLL_CTL_ADD ���¼�����ע�� fd �ϵ��¼�
    /// EPOLL_CTL_MOD �޸� fd �ϵ�ע���¼�
    /// EPOLL_CTL_DEL ɾ�� fd �ϵ�ע���¼�
    int ret = ::epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);
    if (ret < 0)
    {
        std::cout << "epoll add error" << endl;
        /// �ͷ�httpData
        http_data_map_[fd].reset();
        return -1;
    }
    return 0;
}
///setsockopt : �˿ڸ���
int Epoll::modfd(int epoll_fd, int fd, __uint32_t events, std::shared_ptr<HttpData> httpData)
{
    epoll_event event;
    event.events = events;
    event.data.fd = fd;
    /// ÿ�θ��ĵ�ʱ��Ҳ���� http_data_map_
    http_data_map_[fd] = httpData;
    int ret = ::epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &event);
    if (ret < 0)
    {
        std::cout << "epoll mod error" << endl;
        /// �ͷ�httpData
        http_data_map_[fd].reset();
        return -1;
    }
    return 0;
}

int Epoll::delfd(int epoll_fd, int fd, __uint32_t events)
{
    epoll_event event;
    event.events = events;
    event.data.fd = fd;
    int ret = epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, &event);
    if (ret < 0)
    {
        std::cout << "epoll del error" << endl;
        return -1;
    }
    auto it = http_data_map_.find(fd);
    if (it != http_data_map_.end())
    {
        http_data_map_.erase(it);
    }
    return 0;
}
///�ɻ�
int Epoll::handleConnection(const ServerSocket &serverSocket)
{
    std::shared_ptr<ClientSocket> tempClient(new ClientSocket);
    /// epoll �� ET ģʽ��ѭ����������
    /// ��Ҫ�� listen_fd ����Ϊ non-blocking
    while(serverSocket.accept(*tempClient) > 0)
    {
        /// ���÷�����
        int ret = setnonblocking(tempClient->fd);
        if (ret < 0)
        {
            std::cout << "setnonblocking error" << std::endl;
            tempClient->close();
            continue;
        }
        std::shared_ptr<HttpData> m_shared_http_data(new HttpData);
        m_shared_http_data->request_  = std::shared_ptr<HttpRequest>(new HttpRequest());
        m_shared_http_data->response_ = std::shared_ptr<HttpRequest>(new HttpResponse());

        std::shared_ptr<ClientSocket> m_shared_client_socket(new ClientSocket());

        //???
        m_shared_client_socket.swap(tempClient);
        m_shared_http_data->clientSocket_ = m_shared_client_socket;
        m_shared_http_data->epoll_fd = serverSocket.epoll_fd;

        addfd(serverSocket.epoll_fd,m_shared_client_socket->fd,kDefaultEvents,m_shared_http_data);
    }
}

// ���ػ�Ծ�¼��� + �ַ�������
std::vector<std::shared_ptr<HttpData>> Epoll::poll(const ServerSocket &serverSocket, int max_event, int timeout)
{
    // epoll_wait ������ epoll ϵͳ���õ���Ҫ�ӿڣ�����һ�γ�ʱʱ���ڵȴ�һ���ļ��������ϵ��¼���
    // �ɹ����ؾ����ļ�������������ʧ�ܷ��� -1 ������ errno
    int event_num = epoll_wait(serverSocket.epoll_fd,events_,max_event,timeout);
    if( event_num < 0 )
    {
        std::cout << "epoll_num=" << event_num << std::endl;
        std::cout << "epoll_wait error" << std::endl;
        std::cout << errno << std::endl;
        exit(-1);
    }
    //���� events ����
    std::vector<std::shared_ptr<HttpData>> httpDatas;
    for(int i=0; i<event_num; ++i)
    {
        int fd = events_[i].data.fd;

        //����������
        if(fd == serverSocket.listen_fd)
        {
            handleConnection(serverSocket);
        }
        else
        {
            //��������������Ƴ���ʱ�����ر��ļ�������
            if ((events_[i].events & EPOLLERR) || (events_[i].events & EPOLLRDHUP) || (events_[i].events & EPOLLHUP))
            {
                auto it = http_data_map_.find(fd);
                if( it != http_data_map_.end())
                {
                    //��HttpData�ڵ��TimerNode�Ĺ����ֿ�������HttpData�����������������������ڹر��ļ�����������Դ
                    it->second->closeTime();
                }
                continue;
            }
            //����
            auto it = http_data_map_.find(fd);
            if (it != http_data_map_.end())
            {
                if ((events_[i].events & EPOLLIN) || (events_[i].events & EPOLLPRI))
                {
                    http_data_map_.push_back(it->second);
                    std::cout << "��ʱ�����ҵ�:" << fd << std::endl;
                    // �����ʱ�� HttpData.closeTimer()
                    it->second->closeTime();
                    http_data_map_.erase(it);
                }
            }
            else
            {
                std::cout << "�����ӵڶ�������δ�ҵ�" << std::endl;
                ::close(fd);
                continue;
            }
        }
    }
    return httpDatas;
}


/*
void epoll_run(int port)
{
    // ����һ�� epoll ���ĸ��ڵ�
    int epoll_fd = epoll_create(MAX_THREAD_SIZE);
    if(epoll_fd == -1)
    {
        perror("epoll_create error");
        exit(1);
    }
    // ��ӵ�Ҫ�����Ľڵ�
    // ί���ں˼����ӵ����ϵĽڵ�
    struct epoll_event all[MAX_THREAD_SIZE];
    all[0].events
    while(1)
    {
        int ret = epoll_wait(epoll_fd,all,MAX_THREAD_SIZE,-1);
        if(ret == -1 )
        {
            perror("epoll_wait error");
            exit(1);
        }
        //���������仯�Ľڵ�
        for(int i=0; i<ret; ++i)
        {

        }
    }
}
*/
