/********************************************************
*@file   Util.cpp
*@Author herongwei
*@Data   2019/03/31
*********************************************************/
#include "Util.h"
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <cstring>
#include <sys/stat.h>

std::string& ltrim(std::string &str) {
    if(str.empty()) {
        return str;
    }
    //Searches the string for the first character that does not match any of the characters specified in its arguments.
    str.erase(0,str.find_first_not_of(" \t"));
    return str;
}
std::string& rtrim(std::string &str) {
    if(str.empty()) {
        return str;
    }
    str.erase(0,str.find_last_not_of(" \t")+1);
    return str;
}
//��ȥ�Ʊ��
std::string& trim(std::string& str) {
    if (str.empty()) {
        return str;
    }
    ltrim(str);
    rtrim(str);
    return str;
}
//���� fd ������
int setnonblocking(int fd) {
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}
//�źŴ�����
void handle_for_sigpipe()
{
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = SIG_IGN;//����Ŀ���ź�
    sa.sa_flags = 0;
    //sigָ��Ҫ������ź����ͣ�act����ָ���µ��źŴ���ʽ��oact�����������ǰ�Ĵ���ʽ
    if(sigaction(SIGPIPE, &sa, NULL))
        return;
}

int check_base_path(char *basePath) {
    struct stat file;
    if (stat(basePath, &file) == -1) {
        return -1;
    }
    // ����Ŀ¼ ���߲��ɷ���
    if (!S_ISDIR(file.st_mode) || access(basePath, R_OK) == -1) {
        return -1;
    }
    return 0;
}
