#include "CThreadPool.h"
#include<iostream>
#include<string>
#include<windows.h>
const int NUMBER = 1;
using namespace std;

// 创建线程池并初始化，传入最大最小线程个数
CThreadPool::CThreadPool(int min, int max){
    // 实例化任务队列
    m_taskQ = new CTaskQueue;

    do {
        // 初始化线程池参数
        m_minNum = min;
        m_maxNum = max;
        m_busyNum = 0;
        m_liveNum = min;
        m_exitNum = 0;

        // 根据线程的最大上限给线程数组分配内存
        m_threadIDs = new pthread_t[max];
        if (m_threadIDs == nullptr){
            cout << "new pthread_t[] 失败...." << endl;;
            break;
        }
        // 初始化
        memset(m_threadIDs, 0, sizeof(pthread_t) * max);
        
        // 初始化互斥锁,条件变量
        if (pthread_mutex_init(&m_lock, NULL) != 0 ||pthread_cond_init(&m_notEmpty, NULL) != 0){
            cout << "init mutex or condition fail..." << endl;
            break;
        }

        /////////////////// 创建线程 //////////////////
        // 根据最小线程个数, 创建线程
        for (int i = 0; i < min; ++i){
            pthread_create(&m_threadIDs[i], NULL, worker, this); 
            HANDLE win_thread_handle = pthread_getw32threadhandle_np(m_threadIDs[i]);
            cout << "创建子线程, ID: " << win_thread_handle << endl;
        }

        // 创建管理者线程, 1个
        cout << "创建管理者子线程" << endl;
        pthread_create(&m_managerID, NULL, manager, this);
    } while (0);
}

// 销毁线程池
CThreadPool::~CThreadPool(){
    m_shutdown = true;

    // 销毁管理者线程
    pthread_join(m_managerID, NULL);
    
    // 唤醒所有消费者线程，使其自杀
    for (int i = 0; i < m_liveNum; ++i){
        pthread_cond_signal(&m_notEmpty);
    }
    // 等待所有消费者线程自杀完毕
    Sleep(3000);

    if (m_taskQ) delete m_taskQ;
    if (m_threadIDs) delete[]m_threadIDs;
    pthread_mutex_destroy(&m_lock);
    pthread_cond_destroy(&m_notEmpty);
    cout<<"任务完毕，线程池销毁"<<endl;
}

// 给线程池添加任务
void CThreadPool::pushTask(Task task){
    if (m_shutdown){
        return;
    }
    // 添加任务，不需要加锁，任务队列中有锁
    m_taskQ->pushTask(task);
    // 唤醒工作的线程
    pthread_cond_signal(&m_notEmpty);
}

// 获取线程池中工作的线程的个数
int CThreadPool::getBusyNum(){
    pthread_mutex_lock(&m_lock);
    int busyNum = m_busyNum;
    pthread_mutex_unlock(&m_lock);
    return busyNum;
}

// 获取线程池中活着的线程的个数
int CThreadPool::getAliveNum(){
    pthread_mutex_lock(&m_lock);
    int threadNum = m_liveNum;
    pthread_mutex_unlock(&m_lock);
    return threadNum;
}

// 工作线程
void* CThreadPool::worker(void* arg){
    // 强制类型转换
    CThreadPool* pool = static_cast<CThreadPool*>(arg);

    // 一直不停的添加任务
    while (true){
        // 访问任务队列(共享资源)加锁
        pthread_mutex_lock(&pool->m_lock);

        // 判断任务队列是否为空, 如果为空则工作线程阻塞
        while (pool->m_taskQ->taskNumber() == 0 && !pool->m_shutdown){
            cout << "thread " << GetCurrentThreadId() << " waiting..." << endl;
            // 阻塞线程
            // 当m_notEmpty信号量没有时，则为空，就阻塞工作线程，并且释放m_lock
            // 不为空时，会被唤醒，消费m_notEmpty信号量，并重新对m_lock加锁
            pthread_cond_wait(&pool->m_notEmpty, &pool->m_lock);

            // 判断是不是要销毁线程，m_exitNum会在管理者进程中需要销毁线程时赋值
            if (pool->m_exitNum > 0){
                pool->m_exitNum--;


                if (pool->m_liveNum > pool->m_minNum){
                    pool->m_liveNum--;
                    pthread_mutex_unlock(&pool->m_lock); //解锁
                    // 自定义线程退出函数，而没有直接用pthread_exit()，
                    // 需要将退出的线程所在线程队列位置进行置空
                    pool->threadExit();
                }
            }
        }
        // 判断线程池是否被关闭了
        if (pool->m_shutdown){
            pthread_mutex_unlock(&pool->m_lock);
            pool->threadExit();
        }

        // 从任务队列中取出一个任务
        Task task = pool->m_taskQ->popTask();
        // 工作的线程+1
        pool->m_busyNum++;
        // 线程池解锁
        pthread_mutex_unlock(&pool->m_lock);
        // 执行任务
        cout << "thread " << GetCurrentThreadId() << " start working..." << endl;
        // 防止打印过快，造成线程交叉
        Sleep(500);
        task.function(task.arg);
        
        //释放函数参数的堆内存
        delete task.arg;
        task.arg = nullptr;

        // 任务处理结束
        cout << "thread " << GetCurrentThreadId() << " end working...";
        Sleep(500);
        pthread_mutex_lock(&pool->m_lock);
        pool->m_busyNum--;
        pthread_mutex_unlock(&pool->m_lock);
    }
    return nullptr;
}

void* CThreadPool::manager(void* arg){
    CThreadPool* pool = static_cast<CThreadPool*>(arg);
    
    // 如果线程池没有关闭, 就一直检测
    while (!pool->m_shutdown){
        // 每隔2s检测一次
        Sleep(2000);
        //  取出线程池中的任务数和线程数量
        //  取出工作的线程池数量
        pthread_mutex_lock(&pool->m_lock);
        int queueSize = pool->m_taskQ->taskNumber();
        int liveNum = pool->m_liveNum;
        int busyNum = pool->m_busyNum;
        pthread_mutex_unlock(&pool->m_lock);

        // 创建线程

        // 添加线程，每次最多添加两个线程，也可以修改添加多个
        // 任务的个数>存活的线程个数 && 存活的线程数<最大线程数，这个条件是自定义，可以修改
        if (queueSize > liveNum && liveNum < pool->m_maxNum){
            // 线程池加锁
            pthread_mutex_lock(&pool->m_lock);
            int num = 0;
            for (int i = 0; i < pool->m_maxNum && num < NUMBER
                && pool->m_liveNum < pool->m_maxNum; ++i){
                if (pool->m_threadIDs[i].x == 0){ // 将任务放到线程队列中空闲位置
                    pthread_create(&pool->m_threadIDs[i], NULL, worker, pool);
                    num++;
                    pool->m_liveNum++;
                }
            }
            pthread_mutex_unlock(&pool->m_lock);
        }

        // 销毁线程，当空闲的存活线程过多，就需要销毁一部分
        // 忙的线程*2 < 存活的线程数 && 存活的线程>最小线程数，这个条件是自定义，可以修改
        if (busyNum * 2 < liveNum && liveNum > pool->m_minNum){
            // 互斥访问线程池
            pthread_mutex_lock(&pool->m_lock);
            pool->m_exitNum = NUMBER;
            pthread_mutex_unlock(&pool->m_lock);

            // 让空闲工作的线程自杀
            for (int i = 0; i < NUMBER; ++i){
                // 唤醒在任务队列为空时工作的线程，使其向下执行自杀操作
                pthread_cond_signal(&pool->m_notEmpty);
            }
        }
    }
    return nullptr;
}

void CThreadPool::threadExit(){
    pthread_t tid = pthread_self();
    for (int i = 0; i < m_maxNum; ++i){
        // 寻找自杀的线程在线程队列里的位置
        if (pthread_equal(m_threadIDs[i], tid)){
            cout << "threadExit() function: thread "
                << GetCurrentThreadId() << " exiting..." << endl;
            memset(&m_threadIDs[i], 0, sizeof(pthread_t));
            break;
        }
    }
    pthread_exit(NULL);
}
