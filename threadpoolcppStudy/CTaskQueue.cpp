#include "CTaskQueue.h"

CTaskQueue::CTaskQueue(){
	// ��ʼ��������
	pthread_mutex_init(&this->m_mutex, NULL);
}


CTaskQueue::~CTaskQueue(){
	// ���ٻ�����
	pthread_mutex_destroy(&this->m_mutex);
}

// �������
void CTaskQueue::pushTask(Task task){
	// ��������������
	pthread_mutex_lock(&this->m_mutex);
	this->m_queue.push(task);
	pthread_mutex_unlock(&this->m_mutex);
}

// �����������
void CTaskQueue::pushTask(callback func, void* arg){
	// ��������������
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

	// ���в�Ϊ��ʱ��ȡ������
	if (!this->m_queue.empty()){
		t = m_queue.front();
		m_queue.pop();
	}
	pthread_mutex_unlock(&m_mutex);
	return t;
}
