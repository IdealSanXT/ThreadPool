# ThreadPool
线程池实现（C++版）
在VS里用MS编译器不能直接调用pthread库，需要先自行下载该库：http://sourceware.org/pub/pthreads-win32/pthreads-w32-2-9-1-release.zip
解压后用的到的只有Pre-built.2文件夹下的文件。
![image](https://github.com/IdealSanXT/ThreadPool/assets/75885212/0813cfd7-689f-41d3-adf0-12e7eb9952ba)
1. 将文件夹Pre-built.2里面的include和lib文件中的东西合并到VS下载目录下的位置，我的目录是E:\VS2019\Community\VC\Tools\MSVC\14.29.30133
2. 附加依赖项–>pthreadVC2.lib
   ![image](https://github.com/IdealSanXT/ThreadPool/assets/75885212/a027f8b5-3982-4bab-8c50-c82ccac9c9fe)
完成
