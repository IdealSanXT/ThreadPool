#pragma once
#include "CTaskQueue.h"
class CThreadPool{
public:
	// 创建线程池并初始化，传入最大最小线程个数
	CThreadPool(int min, int max);
	// 销毁线程池
	~CThreadPool();

	// 给线程池添加任务
	void pushTask(Task task);

	// 获取线程池中工作的线程的个数
	int getBusyNum();

	// 获取线程池中活着的线程的个数
	int getAliveNum();
	

private:
	// 注意向pthread_create函数传递回调函数参数时，函数本身需要存在，而类内普通函数在类实例化前不存在
	// 所以添加static关键字
	// 
	// 工作的线程的任务函数
	static void* worker(void* arg);
	// 管理者线程的任务函数
	static void* manager(void* arg);
	void threadExit();
	
private:
	// 任务队列
	CTaskQueue* m_taskQ;
	
	// 定义线程的操作
	pthread_t m_managerID;    // 管理者线程ID
	pthread_t* m_threadIDs;   // 工作线程ID，工作线程有多个，定义为数组
	int m_minNum;             // 最小线程数量
	int m_maxNum;             // 最大线程数量
	int m_busyNum;            // 忙的线程的个数
	int m_liveNum;            // 存活的线程的个数
	int m_exitNum;            // 要销毁的线程个数，任务特别少时，需要销毁线程

	// 同步操作
	pthread_mutex_t m_lock;  // 锁整个的线程池
	pthread_cond_t m_notEmpty;    // 信号量，任务队列是不是空了，空了需要阻塞消费者，不能消费任务

	bool m_shutdown;				// 是不是要销毁线程池, 销毁为1, 不销毁为0
};

