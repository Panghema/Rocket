#include <assert.h>
#include "rocket/net/io_thread.h"
#include "rocket/common/log.h"
#include "rocket/common/util.h"

namespace rocket {

IOThread::IOThread() {
    int rt = sem_init(&m_init_semaphore, 0, 0);
    assert(rt == 0);

    rt = sem_init(&m_start_semaphore, 0, 0);
    assert(rt == 0);

    pthread_create(&m_thread, NULL, &IOThread::Main, this);
    // wait 直到Main内部的前置流程走完
    sem_wait(&m_init_semaphore);

    DEBUGLOG("IOThread [%d] create successfully", m_thread_id);

}

IOThread::~IOThread() {
    m_event_loop->stop();
    sem_destroy(&m_init_semaphore);
    sem_destroy(&m_start_semaphore);

    pthread_join(m_thread, NULL);

    if (m_event_loop != NULL) {
        delete m_event_loop;
        m_event_loop = NULL;
    }
}


void* IOThread::Main(void* arg) {
    IOThread* thread = static_cast<IOThread*> (arg);
    
    thread->m_event_loop = new EventLoop();
    thread->m_thread_id = getThreadId();
    
    // wait到这里就可以结束构造函数了
    // 因为loop是死循环，永远到不了loop()后面，除非退出
    // 所以到这里就可以表示创建完了
    sem_post(&thread->m_init_semaphore);
    
    // 让io线程等待，直到我们主动地启动
    DEBUGLOG("IOThread %d created, wait start semaphore", thread->m_thread_id);      
    sem_wait(&thread->m_start_semaphore);
    DEBUGLOG("IOThread %d start loop", thread->m_thread_id);
    thread->m_event_loop->loop();
    DEBUGLOG("IOThread %d end loop", thread->m_thread);

    return NULL;
}

EventLoop* IOThread::getEventLoop() {
    return m_event_loop;
}

void IOThread::start() {
    DEBUGLOG("Now invoke IOThread %d", m_thread_id);
    sem_post(&m_start_semaphore);
}

void IOThread::join() {
    pthread_join(m_thread, NULL);
}

}