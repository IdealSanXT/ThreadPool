#include "CTaskQueue.h"

CTaskQueue::CTaskQueue(){
	// 初始化互斥锁
	pthread_mutex_init(&this->m_mutex, NULL);
}


CTaskQueue::~CTaskQueue(){
	// 销毁互斥锁
	pthread_mutex_destroy(&this->m_mutex);
}

// 添加任务
void CTaskQueue::pushTask(Task task){
	// 互斥访问任务队列
	pthread_mutex_lock(&this->m_mutex);
	this->m_queue.push(task);
	pthread_mutex_unlock(&this->m_mutex);
}

// 重载添加任务
void CTaskQueue::pushTask(callback func, void* arg){
	// 互斥访问任务队列
	pthread_mutex_lock(&m_mutex);
	Task task;
	task.function = func;
	task.arg = arg;
	m_queue.push(task);
	pthread_mutex_unlock(&m_mutex);
}

Task CTaskQueue::popTask(){
	Task t;
	pthread_mutex_lock(&m_mutex);

	// 队列不为空时，取出任务
	if (!this->m_queue.empty()){
		t = m_queue.front();
		m_queue.pop();
	}
	pthread_mutex_unlock(&m_mutex);
	return t;
}
