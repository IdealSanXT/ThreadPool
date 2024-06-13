#pragma once
#include "CTaskQueue.h"
class CThreadPool{
public:
	// �����̳߳ز���ʼ�������������С�̸߳���
	CThreadPool(int min, int max);
	// �����̳߳�
	~CThreadPool();

	// ���̳߳��������
	void pushTask(Task task);

	// ��ȡ�̳߳��й������̵߳ĸ���
	int getBusyNum();

	// ��ȡ�̳߳��л��ŵ��̵߳ĸ���
	int getAliveNum();
	

private:
	// ע����pthread_create�������ݻص���������ʱ������������Ҫ���ڣ���������ͨ��������ʵ����ǰ������
	// �������static�ؼ���
	// 
	// �������̵߳�������
	static void* worker(void* arg);
	// �������̵߳�������
	static void* manager(void* arg);
	void threadExit();
	
private:
	// �������
	CTaskQueue* m_taskQ;
	
	// �����̵߳Ĳ���
	pthread_t m_managerID;    // �������߳�ID
	pthread_t* m_threadIDs;   // �����߳�ID�������߳��ж��������Ϊ����
	int m_minNum;             // ��С�߳�����
	int m_maxNum;             // ����߳�����
	int m_busyNum;            // æ���̵߳ĸ���
	int m_liveNum;            // �����̵߳ĸ���
	int m_exitNum;            // Ҫ���ٵ��̸߳����������ر���ʱ����Ҫ�����߳�

	// ͬ������
	pthread_mutex_t m_lock;  // ���������̳߳�
	pthread_cond_t m_notEmpty;    // �ź�������������ǲ��ǿ��ˣ�������Ҫ���������ߣ�������������

	bool m_shutdown;				// �ǲ���Ҫ�����̳߳�, ����Ϊ1, ������Ϊ0
};

