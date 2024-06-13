#pragma once
#include<queue>
#include<pthread.h>

// ��������ṹ��
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

    // ��������������
    void pushTask(Task task);
    void pushTask(callback func, void* arg);
    // �Ӷ�����ȡ������
    Task popTask();
    // ��ȡ��ǰ�������������
    inline size_t taskNumber(){
        return m_queue.size();
    }

private:
    pthread_mutex_t m_mutex;    // ������
	std::queue<Task> m_queue;
};

