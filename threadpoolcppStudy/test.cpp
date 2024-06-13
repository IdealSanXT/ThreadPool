#include<iostream>
#include<string>
#include<windows.h>
#include"CThreadPool.h"
using namespace std;


// @file:test
// @author:IdealSanX_T
// @date:2024/6/12 19:27:58
// @brief:

void taskFunc(void* arg) {
    int num = *(int*)arg;
    cout << "thread " << GetCurrentThreadId() << " is working, number = " << num << endl;
}

int main() {
    // 创建线程池，最少3个线程，最多10个
    CThreadPool pool(3, 10);

    // 给线程池5个任务
    for (int i = 0; i < 5; ++i) {
        int* num = new int(i);
        *num = i;
        pool.pushTask(Task(taskFunc,num));
    }

    system("pause");
    return 0;
}