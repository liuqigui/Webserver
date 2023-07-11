/********************************************************
*@file   Timer.cpp
*@Author rongweihe
*@Data   2019/03/31
*********************************************************/
#include <sys/time.h>
#include <unistd.h>


#include "Timer.h"
#include "Epoll.h"

size_t TimerNode::m_current_msec = 0;  // ��ǰʱ��
const size_t TimerManager::kDefaultTimeOut = 20*1000; // 20s

//��ʱʱ��=��ǰʱ��+��ǰ��ʱʱ��
TimerNode::TimerNode(std::shared_ptr<HttpData> httpData, size_t timeout) : deleted_(false), httpData_(httpData)
{
    current_time();
    expiredTime_ = m_current_msec + timeout;
}
//����?
//�����ر���Դ��ʱ��ͬʱ����httpDataMap�е�����
TimerNode::~TimerNode()
{
    if(m_http_data)
    {
        auto it = Epoll::http_data_map_.find(m_http_data->clientSocket_->fd);
        if(it != Epoll::http_data_map_.end())
        {
            Epoll::http_data_map_.erase(it);
        }
    }
}
//gettimeofday����,�����Է�����1970-01-01 00:00:00�����ھ���������
void inline TimerNode::current_time()
{
    struct timeval cur;
    gettimeofday(&cur,NULL);
    m_current_msec = (cur.tv_sec*1000) + (cur.tv_usec/1000);
}

//���ʣ�
void TimerNode::deleted()
{
    // ɾ�����ñ��ɾ��������ʱ���� HttpData ���ر�������
    // �رն�ʱ��ʱ httpDataMap ���HttpData һ��erase
    m_http_data.reset();
    m_deleted = true;
}

//����
void TimerManager::add_timer(std::shared_ptr<HttpData> httpData,size_t timeout)
{
    m_shared_timer_node mSTimerNode(new TimerNode(httpData,timeout));
    {
        MutexLockGuard guard(m_lock);
        m_timer_priority_queue.push(mSTimerNode);
        httpData->setTimer(mSTimerNode);
        // ��TimerNode��HttpData��������
    }
}
// ����ʱ�¼�
void TimerManager::handle_expired_event()
{
    MutexLockGuard guard(lock_);
    // ���µ�ǰʱ��
    std::cout << "��ʼ����ʱ�¼�" << std::endl;
    TimerNode::current_time();
    while(!m_timer_priority_queue.empty())
    {
        m_shared_timer_node m_node = m_timer_priority_queue.top();
        if(m_node->isDeleted())
        {
            // ɾ���ڵ�
            m_timer_priority_queue.pop();
        }
        else if (m_node->isExpire())
        {
            // ���� ɾ��
            m_timer_priority_queue.pop();
        }
        else
        {
            break;
        }
    }
}
