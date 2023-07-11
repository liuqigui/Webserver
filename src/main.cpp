/********************************************************
*@file   main.cpp
*@Author herongwei
*@Data   2019/03/31
*********************************************************/
#include "Server.h"
#include "Util.h"

#include <string>
#include <iostream>
#include <dirent.h>
#include <stdio.h>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>  //for signal


//����Ĭ�ϵ�ǰĿ¼
std::string basePath = ".";

//�������ػ����̵ķ�ʽ����
void daemon_run()
{
    //�����ӽ��̣��رո����̣���������ʹ�����ں�̨����
    //1���ڸ������У�fork�����´����ӽ��̵Ľ���ID��
    //2�����ӽ����У�fork����0��
    //3��������ִ���fork����һ����ֵ��

    signal(SIGCHLD,SIG_IGN);
    pid_t pid = fork();
    if(pid<0)
    {
        std::cout<<"fork eror"<<std::endl;
        exit(-1);
    }
    //�������˳����ӽ��̶�������
    else if(pid>0)
    {
        exit(0);
    }
    //֮ǰparent��child������ͬһ��session��,parent�ǻỰ��session������ͷ����,
    //parent������Ϊ�Ự����ͷ���̣����exit����ִ�еĻ�����ô�ӽ��̻��Ϊ�¶����̣�����init������
    //ִ��setsid()֮��,child�����»��һ���µĻỰ(session)id��
    //��ʱparent�˳�֮��,������Ӱ�쵽child�ˡ�
    setsid();
    //�رձ�׼���룬���������
    int fd = open("dev/null",O_RDWR,0);
    if(fd!=-1)
    {
        dup2(fd, STDIN_FILENO);
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
    }
    if (fd > 2)
    {
        close(fd);
    }
    /*
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    open("dev/null",O_RDONLY,0);
    open("dev/null",O_RDWR,0);
    */
}
int main(int argc, const char *argv[])
{
    //Ĭ���߳���
    int thread_number = 4;
    int port = 6666;
    char tp_path[256];
    int opt;
    const char *str ="t:p:r:d";
    bool daemon = false;

    //getopt ���������в�������
    while((opt=getopt(argc,argv,str))!=-1)
    {
        switch(opt)
        {
        case 't':
        {
            thread_number = atoi(optarg);
            break;
        }
        case 'r':
        {
            //����
            int ret = check_base_path(optarg);
            if(ret==-1)
            {
                printf("Warning: \"%s\" �����ڻ򲻿ɷ���, ��ʹ�õ�ǰĿ¼��Ϊ��վ��Ŀ¼\n", optarg);
                //����ǰ����Ŀ¼�ľ���·�����Ƶ�����buffer��ָ���ڴ�ռ���
                if(getcwd(tp_path,256) == NULL)
                {
                    perror("getcwd error");
                    basePath = ".";
                }
                else
                {
                    basePath = tp_path;
                }
                break;
            }
            //����
            if (optarg[strlen(optarg)-1] == '/')
            {
                optarg[strlen(optarg)-1] = '\0';
            }
            basePath = optarg;
            break;
        }
        case 'p':
        {
            // FIXME �˿ںϷ���У��
            port = atoi(optarg);
            break;
        }
        case 'd':
        {
            daemon = true;
            break;
        }
        default:
            break;
        }
    }
    //�ػ�����
    if(daemon)
    {
        daemon_run();
    }
    //  ���������Ϣ
    {
      printf("*******WebServer ������Ϣ*******\n");
      printf("�˿�:\t%d\n", port);
      printf("�߳���:\t%d\n", threadNumber);
      printf("��Ŀ¼:\t%s\n", basePath.c_str());
    }
    //����
    handle_for_sigpipe();

   // �󶨶˿ڣ�����
    HttpServer httpServer(port);
    httpServer.run(threadNumber);
    return 0;
    /*
    http_request
    HTTP ����
    ���ļ���ʱ����Ҫ�ж�һ�£��򿪵����ļ�����Ŀ¼
    ��ȡ�ļ�������
    stat() ����
    struct stat st;
    //�ڶ������Դ�������
    int ret = stat(file, &st);
    if(ret == -1)
    {
     show(404);
    }
    */
}

