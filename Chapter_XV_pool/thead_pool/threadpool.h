//
// Created by 13345 on 2023/7/2.
// <<Linux高性能服务器编程>>第十五章 进程池部分
// 使用pthread实现的简易线程池，不支持被执行函数的异步返回，仅仅做执行事件的功能
//

#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <list>
#include <cstdio>
#include <exception>
#include <pthread.h>

#include "locker.h"

/**线程池类*/
template<typename T>
class threadpool
{
public:
    /**参数thread_number是线程池中线程的数量，max_requests是请求队列中最多允许的、等待处理的请求的数量*/
    threadpool(int thread_number = 8, int max_reuqests = 10000);
    ~threadpool();
    bool append(T* request);

private:
    static void* worker(void* arg);
    void run();

private:
    int m_thread_number;  /**线程池中的线程数*/
    int m_max_requests;  /**请求队列中允许的最大请求数*/
    pthread_t* m_threads;  /**描述线程池的数组，其大小为m_thread_number*/
    std::list<T*> m_workqueue;  /**请求队列*/
    locker m_queuelocker;  /**保护请求队列的互斥锁*/
    sem m_queuestat;  /**是否有任务需要处理*/
    bool m_stop;  /**是否结束线程*/
};

template<typename T>
threadpool<T>::threadpool(int thread_number, int max_requests) :
    m_thread_number(thread_number), m_max_requests(max_requests), m_stop(false), m_threads(NULL)
{
    if((thread_number <= 0) || (max_requests <= 0))
    {
        throw std::exception();
    }

    m_threads = new pthread_t [m_thread_number];
    if (!m_threads)
    {
        throw std::exception();
    }

    /**创建thread_number个线程，并将它们都设置为脱离线程*/
    for (int i = 0; i < thread_number; ++i)
    {
        printf("create the %dth thread\n", i);
        if (pthread_create(m_threads + i, NULL, worker, this) != 0)
        {
            delete [] m_threads;
            throw std::exception();
        }
        if (pthread_detach(m_threads[i]))
        {
            delete [] m_threads;
            throw std::exception();
        }
    }
}

template<typename T>
threadpool<T>::~threadpool()
{
    delete [] m_threads;
    m_stop = true;
}

template<typename T>
bool threadpool<T>::append(T *request)
{
    /**操作工作队列一定要加锁，因为它被线程共享*/
    m_queuelocker.lock();
    if (m_workqueue.size() > m_max_requests)
    {
        m_queuelocker.unlock();
        return false;
    }
    m_workqueue.push_back(request);
    m_queuelocker.unlock();
    m_queuestat.post();
    return true;
}

//TODO worker的意义是什么喃
template<typename T>
void threadpool<T>::worker(void *arg)
{
    threadpool* pool = (threadpool*) arg;
    pool->run();
    return pool;
}

template<typename T>
void threadpool<T>::run()
{
    while (!m_stop)
    {
        m_queuestat.wait();
        m_queuelocker.lock();
        if (m_workqueue.empty())
        {
            m_queuelocker.unlock();
            continue;
        }
        T* request = m_workqueue.front();
        m_workqueue.pop_front();
        m_queuelocker.unlocker();
        if (!request)
        {
            continue;
        }
        request->process();
    }
}

#endif //THREADPOOL_H
