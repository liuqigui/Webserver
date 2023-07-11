#ifndef THREADPOOL_H_INCLUDED
#define THREADPOOL_H_INCLUDED


#include <bits/stdc++.h>
#include <functional>
#include <pthread.h>
#include <memory>
#include "MutexLock.h"
#include "cond.h"

const int MAX_THREAD_SIZE = 1024;
const int MAX_QUEUE_SIZE = 10000;


/*���Źر�����*/
typedef enum {
    immediate_mode = 1,
    graceful_mode = 2
}ShutdownMode;

//����ָ�� �� ��������
struct ThreadTask {
    std::function< void(std::shared_ptr<void>) > process;// ʵ�ʴ������Server::do_request;
    std::shared_ptr<void> arg;// ʵ��Ӧ����HttpData����
};

class ThreadPool {
public:
    ThreadPool(int thread_s,int max_queue_s);
    ~ThreadPool();
    bool append(std::shared_ptr<void>arg,std::function<void(std::shared_ptr<void>)> fun);
    void shutdown(bool graceful);
private:
    ///�����߳����еĺ����������ϴӹ�������ȡ������ִ��
    static void *worker(void *args);
    void run();
private:
    //�߳�ͬ������
    MutexLock m_mutex;
    cond m_cond;

    //�̳߳�����
    int m_thread_size;
    int max_queue_size;
    int started;

    ///�̳߳��Ƿ����
    int shutdown_;
    std::vector<pthread_t> threads;
    std::list<ThreadTask> request_queue;
};
#endif // THREADPOOL_H_INCLUDED
