/********************************************************
*@file  ��װHttpParse.cpp
*@Author rongweihe
*@Data 2019/03/31
*********************************************************/
#include <bits/stdc++.h>
#include <string.h>
#include <iostream>
#include <algorithm>
#include "HttpParse.h"
#include "HttpRequest.h"
#include "Util.h"
using namespace std;

std::unordered_map<std::string,HttpRequest::HTTP_HEADER> HttpRequest::header_map =
{
    {"HOST",                      HttpRequest::Host},
    {"USER-AGENT",                HttpRequest::User_Agent},
    {"CONNECTION",                HttpRequest::Connection},
    {"ACCEPT-ENCODING",           HttpRequest::Accept_Encoding},
    {"ACCEPT-LANGUAGE",           HttpRequest::Accept_Language},
    {"ACCEPT",                    HttpRequest::Accept},
    {"CACHE-CONTROL",             HttpRequest::Cache_Control},
    {"UPGRADE-INSECURE-REQUESTS", HttpRequest::Upgrade_Insecure_Requests}
};

// ����������
HttpRequestParser::HTTP_CODE
HttpRequestParser::parse_requestline(char *line,PARSE_STATE &parse_state,HttpRequest &request)
{
    //strpbrk����Դ�ַ���(s1)���ҳ����Ⱥ��������ַ���(s2)����һ�ַ���λ�ò�����,
    //�����������û�пհ��ַ��� '\t' ��������������
    char *url = strpbrk(line," \t");
    if(!url)
    {
        return BAD_REQUEST;
    }

    //�ָ� method �� url
    *url++ = '\0';
    char *method = line;
    if (strcasecmp(method, "GET") == 0)
    {
        request.mMethod = HttpRequest::GET;
    }
    else if (strcasecmp(method, "POST") == 0)
    {
        request.mMethod = HttpRequest::POST;
    }
    else if (strcasecmp(method, "PUT") == 0)
    {
        request.mMethod = HttpRequest::PUT;
    }
    else
    {
        return BAD_REQUEST;
    }

    //�����ַ��� str1 �е�һ�������ַ��� str2 �г��ֵ��ַ��±ꡣ
    url += strspn(url, " \t");
    char *version = strpbrk(url, " \t");
    if (!version)
    {
        return BAD_REQUEST;
    }
    *version++ = '\0';
    version += strspn(version, " \t");

    // HTTP/1.1 ������ܻ����ڿհ��ַ�
    if (strncasecmp("HTTP/1.1", version, 8) == 0)
    {
        request.mVersion = HttpRequest::HTTP_11;
    }
    else if (strncasecmp("HTTP/1.0", version, 8) == 0)
    {
        request.mVersion = HttpRequest::HTTP_10;
    }
    else
    {
        return BAD_REQUEST;
    }

    if(strncasecmp(url,"http://",7) ==0)
    {
        url+=7;
        //�ڲ��� str ��ָ����ַ�����������һ�γ����ַ� c��һ���޷����ַ�����λ�á�
        url = strchr(url,'/');
    }
    else if (strncasecmp(url, "/", 1) == 0)
    {
        //PASS;
    }
    else
    {
        return BAD_REQUEST;
    }
    if(!url || *url !='/')
    {
        return BAD_REQUEST;
    }

    request.mUrl = std::string(url);
    parse_state = PARSE_HEADER;
    return NO_REQUEST;
}

//����ͷ���ֶ�
HttpRequestParser::HTTP_CODE
HttpRequestParser::parse_headers(char *line, PARSE_STATE &parse_state, HttpRequest &request)
{

    if (*line == '\0')
    {
        if (request.mMethod == HttpRequest::GET)
        {
            return GET_REQUEST;
        }
        parse_state = PARSE_BODY;
        return NO_REQUEST;
    }

    char key[100], value[300];

    sscanf(line, "%[^:]:%[^:]", key, value);

    decltype(HttpRequest::header_map)::iterator it;
    std::string key_s(key);
    std::transform(key_s.begin(), key_s.end(), key_s.begin(), ::toupper);
    std::string value_s(value);

    if ((it = HttpRequest::header_map.find(trim(key_s))) != (HttpRequest::header_map.end()))
    {
        request.mHeaders.insert(std::make_pair(it->second, trim(value_s)));
    }
    else
    {
        std::cout << "Header no support: " << key << " : " << value << std::endl;
    }

    return NO_REQUEST;
}


// http �������
HttpRequestParser::HTTP_CODE
HttpRequestParser::parse_content(char *buffer, int &check_index, int &read_index,
                                 HttpRequestParser::PARSE_STATE &parse_state, int &start_line,
                                 HttpRequest &request)
{

    LINE_STATE line_state = LINE_OK;
    HTTP_CODE retcode = NO_REQUEST;
    while ((line_state = parse_line(buffer, check_index, read_index)) == LINE_OK)
    {
        char *temp = buffer + start_line;      // ��һ����buffer�е���ʼλ��
        start_line = check_index;              // ��һ����ʼλ��

        switch (parse_state)
        {
        case PARSE_REQUESTLINE:
        {
            retcode = parse_requestline(temp, parse_state, request);
            if (retcode == BAD_REQUEST)
                return BAD_REQUEST;

            break;
        }

        case PARSE_HEADER:
        {
            retcode = parse_headers(temp, parse_state, request);
            if (retcode == BAD_REQUEST)
            {
                return BAD_REQUEST;
            }
            else if (retcode == GET_REQUEST)
            {
                return GET_REQUEST;
            }
            break;
        }

        case PARSE_BODY:
        {
            retcode = parse_body(temp, request);
            if (retcode == GET_REQUEST)
            {
                return GET_REQUEST;
            }
            return BAD_REQUEST;
        }
        default:
            return INTERNAL_ERROR;
        }
    }
    if (line_state == LINE_MORE)
    {
        return NO_REQUEST;
    }
    else
    {
        return BAD_REQUEST;
    }
}
















