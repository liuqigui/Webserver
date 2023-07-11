/********************************************************
*@file  ��װHttpServer��
*@Author rongweihe
*@Data 2019/03/31
*********************************************************/
#ifndef SERVER_H_INCLUDED
#define SERVER_H_INCLUDED

#include "Socket.h"
#include "HttpParse.h"
#include "HttpResponse.h"
#include "HttpData.h"
#include <memory>
const int BUFFSIZE = 2048;
class HttpServer
{
public:
    enum FileState
    {
        FILE_OK,
        FILE_NOT_FOUND,
        FILE_FORBIDDEN
    };
public:
    //����ڹ��캯��ǰ���explicit�ؼ���,������ֹ������ʹ��������캯������ʽת��
    explicit HttpServer(int port = 80,const char *ip = nullptr):serverSocket_(port,ip)
    {
        serverSocket_.bind();
        serverSocket_.listen();
    }
    void run(int,int max_queue_size = 10000);
    void do_request(std::shared_ptr<void> arg);
private:
    void header(std::shared_ptr<HttpData>);
    FileState static_file(std::shared_ptr<HttpData>,const char *);
    void send(std::shared_ptr<HttpData>,FileState);
    void get_mime(std::shared_ptr<HttpData>);
    void hanle_index();
    ServerSocket serverSocket_;
};

#endif // SERVER_H_INCLUDED
