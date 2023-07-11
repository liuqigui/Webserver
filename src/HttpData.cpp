/********************************************************
*@file  ��װHttpData.cpp
*@Author rongweihe
*@Data 2019/03/31
*********************************************************/
#include "HttpData.h"

void HttpData::closeTime()
{
    //�����ж�Timer�Ƿ񻹴��ڣ��п��ܳ�ʱ�ͷ�
    if(weak_ptr_timer_.lock())
    {
        std::shared_ptr<TimerNode> tempTimer(weak_ptr_timer_.lock());
        tempTimer->deleted();
        weak_ptr_timer_.reset();// �Ͽ�weak_ptr
    }
}
void HttpData::setTimer(std::shared_ptr<TimerNode> timer)
{
    weak_ptr_timer_  = timer;
}
