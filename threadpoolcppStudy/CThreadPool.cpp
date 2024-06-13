#include "CThreadPool.h"
#include<iostream>
#include<string>
#include<windows.h>
const int NUMBER = 1;
using namespace std;

// �����̳߳ز���ʼ�������������С�̸߳���
CThreadPool::CThreadPool(int min, int max){
    // ʵ�����������
    m_taskQ = new CTaskQueue;

    do {
        // ��ʼ���̳߳ز���
        m_minNum = min;
        m_maxNum = max;
        m_busyNum = 0;
        m_liveNum = min;
        m_exitNum = 0;

        // �����̵߳�������޸��߳���������ڴ�
        m_threadIDs = new pthread_t[max];
        if (m_threadIDs == nullptr){
            cout << "new pthread_t[] ʧ��...." << endl;;
            break;
        }
        // ��ʼ��
        memset(m_threadIDs, 0, sizeof(pthread_t) * max);
        
        // ��ʼ��������,��������
        if (pthread_mutex_init(&m_lock, NULL) != 0 ||pthread_cond_init(&m_notEmpty, NULL) != 0){
            cout << "init mutex or condition fail..." << endl;
            break;
        }

        /////////////////// �����߳� //////////////////
        // ������С�̸߳���, �����߳�
        for (int i = 0; i < min; ++i){
            pthread_create(&m_threadIDs[i], NULL, worker, this); 
            HANDLE win_thread_handle = pthread_getw32threadhandle_np(m_threadIDs[i]);
            cout << "�������߳�, ID: " << win_thread_handle << endl;
        }

        // �����������߳�, 1��
        cout << "�������������߳�" << endl;
        pthread_create(&m_managerID, NULL, manager, this);
    } while (0);
}

// �����̳߳�
CThreadPool::~CThreadPool(){
    m_shutdown = true;

    // ���ٹ������߳�
    pthread_join(m_managerID, NULL);
    
    // ���������������̣߳�ʹ����ɱ
    for (int i = 0; i < m_liveNum; ++i){
        pthread_cond_signal(&m_notEmpty);
    }
    // �ȴ������������߳���ɱ���
    Sleep(3000);

    if (m_taskQ) delete m_taskQ;
    if (m_threadIDs) delete[]m_threadIDs;
    pthread_mutex_destroy(&m_lock);
    pthread_cond_destroy(&m_notEmpty);
    cout<<"������ϣ��̳߳�����"<<endl;
}

// ���̳߳��������
void CThreadPool::pushTask(Task task){
    if (m_shutdown){
        return;
    }
    // ������񣬲���Ҫ�������������������
    m_taskQ->pushTask(task);
    // ���ѹ������߳�
    pthread_cond_signal(&m_notEmpty);
}

// ��ȡ�̳߳��й������̵߳ĸ���
int CThreadPool::getBusyNum(){
    pthread_mutex_lock(&m_lock);
    int busyNum = m_busyNum;
    pthread_mutex_unlock(&m_lock);
    return busyNum;
}

// ��ȡ�̳߳��л��ŵ��̵߳ĸ���
int CThreadPool::getAliveNum(){
    pthread_mutex_lock(&m_lock);
    int threadNum = m_liveNum;
    pthread_mutex_unlock(&m_lock);
    return threadNum;
}

// �����߳�
void* CThreadPool::worker(void* arg){
    // ǿ������ת��
    CThreadPool* pool = static_cast<CThreadPool*>(arg);

    // һֱ��ͣ���������
    while (true){
        // �����������(������Դ)����
        pthread_mutex_lock(&pool->m_lock);

        // �ж���������Ƿ�Ϊ��, ���Ϊ�������߳�����
        while (pool->m_taskQ->taskNumber() == 0 && !pool->m_shutdown){
            cout << "thread " << GetCurrentThreadId() << " waiting..." << endl;
            // �����߳�
            // ��m_notEmpty�ź���û��ʱ����Ϊ�գ������������̣߳������ͷ�m_lock
            // ��Ϊ��ʱ���ᱻ���ѣ�����m_notEmpty�ź����������¶�m_lock����
            pthread_cond_wait(&pool->m_notEmpty, &pool->m_lock);

            // �ж��ǲ���Ҫ�����̣߳�m_exitNum���ڹ����߽�������Ҫ�����߳�ʱ��ֵ
            if (pool->m_exitNum > 0){
                pool->m_exitNum--;


                if (pool->m_liveNum > pool->m_minNum){
                    pool->m_liveNum--;
                    pthread_mutex_unlock(&pool->m_lock); //����
                    // �Զ����߳��˳���������û��ֱ����pthread_exit()��
                    // ��Ҫ���˳����߳������̶߳���λ�ý����ÿ�
                    pool->threadExit();
                }
            }
        }
        // �ж��̳߳��Ƿ񱻹ر���
        if (pool->m_shutdown){
            pthread_mutex_unlock(&pool->m_lock);
            pool->threadExit();
        }

        // �����������ȡ��һ������
        Task task = pool->m_taskQ->popTask();
        // �������߳�+1
        pool->m_busyNum++;
        // �̳߳ؽ���
        pthread_mutex_unlock(&pool->m_lock);
        // ִ������
        cout << "thread " << GetCurrentThreadId() << " start working..." << endl;
        // ��ֹ��ӡ���죬����߳̽���
        Sleep(500);
        task.function(task.arg);
        
        //�ͷź��������Ķ��ڴ�
        delete task.arg;
        task.arg = nullptr;

        // ���������
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
    
    // ����̳߳�û�йر�, ��һֱ���
    while (!pool->m_shutdown){
        // ÿ��2s���һ��
        Sleep(2000);
        //  ȡ���̳߳��е����������߳�����
        //  ȡ���������̳߳�����
        pthread_mutex_lock(&pool->m_lock);
        int queueSize = pool->m_taskQ->taskNumber();
        int liveNum = pool->m_liveNum;
        int busyNum = pool->m_busyNum;
        pthread_mutex_unlock(&pool->m_lock);

        // �����߳�

        // ����̣߳�ÿ�������������̣߳�Ҳ�����޸���Ӷ��
        // ����ĸ���>�����̸߳��� && �����߳���<����߳���������������Զ��壬�����޸�
        if (queueSize > liveNum && liveNum < pool->m_maxNum){
            // �̳߳ؼ���
            pthread_mutex_lock(&pool->m_lock);
            int num = 0;
            for (int i = 0; i < pool->m_maxNum && num < NUMBER
                && pool->m_liveNum < pool->m_maxNum; ++i){
                if (pool->m_threadIDs[i].x == 0){ // ������ŵ��̶߳����п���λ��
                    pthread_create(&pool->m_threadIDs[i], NULL, worker, pool);
                    num++;
                    pool->m_liveNum++;
                }
            }
            pthread_mutex_unlock(&pool->m_lock);
        }

        // �����̣߳������еĴ���̹߳��࣬����Ҫ����һ����
        // æ���߳�*2 < �����߳��� && �����߳�>��С�߳���������������Զ��壬�����޸�
        if (busyNum * 2 < liveNum && liveNum > pool->m_minNum){
            // ��������̳߳�
            pthread_mutex_lock(&pool->m_lock);
            pool->m_exitNum = NUMBER;
            pthread_mutex_unlock(&pool->m_lock);

            // �ÿ��й������߳���ɱ
            for (int i = 0; i < NUMBER; ++i){
                // �������������Ϊ��ʱ�������̣߳�ʹ������ִ����ɱ����
                pthread_cond_signal(&pool->m_notEmpty);
            }
        }
    }
    return nullptr;
}

void CThreadPool::threadExit(){
    pthread_t tid = pthread_self();
    for (int i = 0; i < m_maxNum; ++i){
        // Ѱ����ɱ���߳����̶߳������λ��
        if (pthread_equal(m_threadIDs[i], tid)){
            cout << "threadExit() function: thread "
                << GetCurrentThreadId() << " exiting..." << endl;
            memset(&m_threadIDs[i], 0, sizeof(pthread_t));
            break;
        }
    }
    pthread_exit(NULL);
}
