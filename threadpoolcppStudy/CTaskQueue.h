#pragma once
#include<queue>
#include<pthread.h>

// 定义任务结构体
using callback = void(*)(void*);
struct Task{
    Task(){
        function = nullptr;
        arg = nullptr;
    }
    Task(callback f, void* arg){
        function = f;
        this->arg = arg;
    }
    callback function;
    void* arg;
};

class CTaskQueue{
public:
    CTaskQueue();
    ~CTaskQueue();

    // 向队列里添加任务
    void pushTask(Task task);
    void pushTask(callback func, void* arg);
    // 从队列里取出任务
    Task popTask();
    // 获取当前队列中任务个数
    inline size_t taskNumber(){
        return m_queue.size();
    }

private:
    pthread_mutex_t m_mutex;    // 互斥锁
	std::queue<Task> m_queue;
};

