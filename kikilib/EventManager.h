//@Author Liu Yukang 
#pragma once

#include "EventEpoller.h"
#include "Time.h"
#include "utils.h"

#include <set>
#include <vector>
#include <thread>
#include <mutex>
#include <functional>
#include <future>

namespace kikilib
{
	class EventService;
	class Timer;
	class ThreadPool;

	//事件优先级
	enum EventPriority
	{
		NORMAL_EVENT = 0,  //一般事件
		IMMEDIATE_EVENT,   //紧急事件
		EVENT_PRIORITY_TYPE_COUNT   //事件优先级种类个数
	};

	//事件管理器
	//职责：
	//1、提供向其中插入事件，移除，修改事件的接口
	//2、提供定时器的使用接口
	//3、循环扫描其管理的事件中被激活的对象，然后根据事件类型调用其相关函数
	//需要管理以下类的生命：
	//1、循环器线程looper
	//2、事件监视器epoller
	//3、定时器timer
	//4、所有的事件EventService
	class EventManager
	{
	public:
		EventManager(int idx, ThreadPool* threadPool);
		~EventManager();

		DISALLOW_COPY_MOVE_AND_ASSIGN(EventManager);

		//创建一个线程，然后线程中循环扫描事件
		bool Loop();

		//向事件管理器中插入一个事件,这是线程安全的
		void Insert(EventService* ev);

		//向事件管理器中移除一个事件,这是线程安全的
		void Remove(EventService* ev);

		//向事件管理器中修改一个事件服务所关注的事件类型,这是线程安全的
		void Motify(EventService* ev);

		//time时间后执行timerCb函数
		void RunAfter(Time time, std::function<void()> timerCb);

		//每过time时间执行一次timerCb函数
		void RunEvery(Time time, std::function<void()> timerCb);

		//将任务放在线程池中以达到异步执行的效果
        void RunInThreadPool(std::function<void()>&& func);

	private:
		//当前manager的索引号，有些场景需要某个manager专门处理某种事件
		const int _idx;

		//退出循环的标志
		bool _quit;

		//线程池，可将函数放入其中异步执行
		ThreadPool* _pThreadPool;

		//循环器，在单独的一个线程中
		std::thread* _pLooper;

		//定时器，进行定时事件处理
		Timer* _pTimer;

		//保证eventSet的线程安全
		std::mutex _eventSetMutex;

		//保证移除事件时的线程安全
		std::mutex _removedEvMutex;

		//被移除的事件列表，要移除某一个事件会先放在该列表中，一次循环结束才会真正放入其中
		std::vector<EventService*> _removedEv;

		//EventEpoller发现的活跃事件所放的列表
		std::vector<EventService*> _actEvServs;

		//活跃事件按照优先级所放的列表
		std::vector<EventService*> _priorityEvQue[EVENT_PRIORITY_TYPE_COUNT];

		//事件监视器
		EventEpoller _epoller;

		//事件集合
		std::set<EventService*> _eventSet;

	};

}
