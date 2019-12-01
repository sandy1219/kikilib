#include "EventEpoller.h"
#include "LogManager.h"
#include "EventService.h"

#include <string.h>
#include <errno.h>
#include <poll.h>
#include <sys/epoll.h>
#include <unistd.h>

using namespace kikilib;

EventEpoller::EventEpoller()
	: _epollFd(::epoll_create1(EPOLL_CLOEXEC)), _activeEpollEvents(Parameter::epollEventListFirstSize)
{
	if (_epollFd < 0)
	{
		RecordLog(ERROR_DATA_INFORMATION, "epoll fd create failed!");
	}
}

EventEpoller::~EventEpoller() 
{
	::close(_epollFd);
};


//�޸�EventEpoller�е��¼�
void EventEpoller::MotifyEv(EventService* evServ)
{
	if (!evServ)
	{
		return;
	}
	struct epoll_event event;
	memset(&event, 0, sizeof(event));
	event.events = evServ->GetInteresEv();
	event.data.ptr = evServ;
	int fd = evServ->fd();
	if (::epoll_ctl(_epollFd, EPOLL_CTL_MOD, fd, &event) < 0)
	{
		RecordLog(ERROR_DATA_INFORMATION, "event motify err!");
	}
}

//��EventEpoller�������¼�
void EventEpoller::AddEv(EventService* evServ)
{
	if (!evServ)
	{
		return;
	}
	struct epoll_event event;
	memset(&event, 0, sizeof(event));
	event.events = evServ->GetInteresEv();
	event.data.ptr = evServ;
	int fd = evServ->fd();
	if (::epoll_ctl(_epollFd, EPOLL_CTL_ADD, fd, &event) < 0)
	{
		RecordLog(ERROR_DATA_INFORMATION, "event add err!");
	}
}

//��EventEpoller���Ƴ��¼�
void EventEpoller::RemoveEv(EventService* evServ)
{
	if (!evServ)
	{
		return;
	}
	struct epoll_event event;
	memset(&event, 0, sizeof(event));
	event.events = evServ->GetInteresEv();
	event.data.ptr = evServ;
	int fd = evServ->fd();
	if (::epoll_ctl(_epollFd, EPOLL_CTL_DEL, fd, &event) < 0)
	{
		RecordLog(ERROR_DATA_INFORMATION, "event add err!");
	}
}

void EventEpoller::GetActEvServ(int timeOutMs, std::vector<EventService*>& activeEvServs)
{
	int actEvNum = ::epoll_wait(_epollFd, &*_activeEpollEvents.begin(), static_cast<int>(_activeEpollEvents.size()), timeOutMs);
	int savedErrno = errno;
	if (actEvNum > 0)
	{
		if (actEvNum > _activeEpollEvents.size())
		{
			RecordLog(ERROR_DATA_INFORMATION, "unknown err in GetActServ()!");
			return;
		}
		for (int i = 0; i < actEvNum; ++i)
		{
			//�����¼����ͣ��Ž���Ծ�¼��б���
			EventService* evServ = static_cast<EventService*>(_activeEpollEvents[i].data.ptr);
			evServ->SetEventState(_activeEpollEvents[i].events);
			activeEvServs.push_back(evServ);
		}
		if (actEvNum == _activeEpollEvents.size())
		{
			//����epoll�л�ȡ�¼����������ˣ�˵���������Ĵ�С���ܲ�������չһ��
			_activeEpollEvents.resize(_activeEpollEvents.size() * 2);
		}
	}
	else if (actEvNum == 0)
	{
		//RecordLog(DEBUG_DATA_INFORMATION, "nothing happened in GetActEvServ()");
	}
	else
	{
		if (savedErrno != EINTR)
		{
			errno = savedErrno;
			RecordLog(ERROR_DATA_INFORMATION, "err happened in GetActEvServ()");
		}
	}
}