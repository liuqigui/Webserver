/********************************************************
*@file  ��װHttpParse��
*@Author rongweihe
*@Data 2019/04/18
*********************************************************/
#ifndef HTTPPARSE_H_INCLUDED
#define HTTPPARSE_H_INCLUDED

#include <unordered_map>
#include <string>
#include <iostream>

const char CR =  '\r' ;
const char LF =  '\n';
const char LINE_END = '\0';
#define PASS

class HttpRequest;
std::ostream &operator<<(std::ostream &, const HttpRequest &);

class HttpRequestParser
{
public:
    //�еĶ�ȡ״̬����ȡ��һ���������С��г�������������Ҳ�����
    enum LINE_STATE
    {
        LINE_OK=0,
        LINE_BAD,
        LINE_MORE
    };
    //����״̬
    enum PARSE_STATE
    {
        PARSE_REQUESTLINE = 0,
        PARSE_HEADER,
        PARSE_BODY
    };
    //���������� HTTP ����Ŀ��ܽ��
    enum HTTP_CODE
    {
        NO_REQUEST,/*NO_REQUEST��ʾ������������Ҫ��ȡ�ͻ�����*/
        GET_REQUEST,/*GET_REQUEST��ʾ�����һ�������Ŀͻ�����*/
        BAD_REQUEST,/*BAD_REQUEST��ʾ�ͻ��������﷨����*/
        FORBIDDEN_REQUEST,/*��ʾ�ͻ�����Դû���㹻�ķ���Ȩ��*/
        INTERNAL_ERROR,/*��ʾ�������ڲ�����*/
        CLOSED_CONNECTION/*��ʾ�ͻ����Ѿ��ر�����*/
    };

    //��״̬�������ڽ���һ������
    static LINE_STATE parse_line(char *buffer, int &checked_index, int &read_index);

    //����������
    static HTTP_CODE parse_requestline(char *line, PARSE_STATE &parse_state, HttpRequest &request);

    //����ͷ��
    static HTTP_CODE parse_headers(char *line, PARSE_STATE &parse_state, HttpRequest &request);

    static HTTP_CODE parse_body(char *body, HttpRequest &request);
    //����HTTP�������ں���
    static HTTP_CODE
    parse_content(char *buffer, int &check_index, int &read_index, PARSE_STATE &parse_state, int &start_line,
                  HttpRequest &request);
};
#endif // HTTPPARSE_H_INCLUDED
