#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

#include <bits/stdc++.h>

void daemon_run()
{
    int pid;
    signal(SIGCHLD,SIG_IGN);
    /*
    [1] �ڸ������У�fork ���ش����ӽ��̵Ľ��� ID
    [2] ���ӽ����У�fork ���� 0
    [3] ��� fork ��������һ����ֵ
    */
    pid = fork();
    if(pid<0) {
        std::cout<<"fork error"<<endl;
        exit(-1);
    } else if(pid>0) {
        exit(0);
    }
    //�������˳����ӽ��̶�������

    /*
    [1]֮ǰ���ӽ�����������ͬһ��session�Ự��������ǻỰ����ͷ���̡�
    [2]��������Ϊ�Ự����ͷ���̣����exit����ִ�еĻ�����ô�ӽ��̻��Ϊ�¶����̣�����init��������
    [3]setsid֮���ӽ��̽����»��һ���µĻỰid����ʱ�������˳��Ժ󣬽�����Ӱ�쵽�ӽ���
    */
    setsid();

    /*
    open ����������һ���豸
    ���ص���һ�����ͱ���
    [1]������ֵ����-1��˵�����ļ����ִ���
    [2]���Ϊ����0��ֵ����ô���ֵ����ľ����ļ���������
    O_RDONLY ֻ���򿪡�
    O_WRONLY ֻд�򿪡�
    O_RDWR ����д�򿪡�
    O_APPEND ÿ��дʱ���ӵ��ļ���β�ˡ�
    */
    int fd;
    fd = open("dev/null",O_RDWR,0);

    /*
    ����׼���룬����������ض��� /dev/null �ļ�
    */
    if(fd!=-1)
    {
        dup2(fd,STDIN_FILENO);
        dup2(fd,STDOUT_FILENO);
        dup2(fd,STDERR_FILENO);
    }


    /*�رձ�׼���룬����������豸*/
    if(fd>2)
        close(fd);
}

#endif // MAIN_H_INCLUDED
