#include "timer.h"

void TimerProc(union sigval sgval)
{
	Timer timer = TimerManager::GetInstance().GetTimer(sgval.sival_int);
	ITimerProc *proc = timer.GetTimerProc();
	if(proc)
	{
		proc->OnTimer();
	}
}

/************************************************************************************************/
/*                                           Timer                                              */
/************************************************************************************************/
Timer::Timer()
: t_id(0), m_proc(0), m_id(0)
{
}

Timer::Timer(int ms, ITimerProc* proc)
: t_id(0), m_proc(proc), m_id(0)
{
	Create(ms);
}

Timer::Timer(const Timer& timer)
: m_proc(timer.m_proc), t_id(timer.t_id), m_id(timer.m_id)
{
}
	
Timer::~Timer()
{
}

void Timer::operator = (const Timer& timer)
{
	m_proc = timer.m_proc;
	t_id = timer.t_id;
	m_id = timer.m_id;
}

bool Timer::operator == (const Timer& timer)
{
	return t_id == timer.t_id;
}
	
int Timer::Create(int ms)
{
	if(t_id)
		return m_id;
	
	sigevent se;
	se.sigev_notify = SIGEV_THREAD;
	se.sigev_value.sival_int = m_id = TimerManager::GetInstance().GetNextTimerId();
	se.sigev_notify_function = TimerProc;
	se.sigev_notify_attributes = 0;
	if (-1 == timer_create(CLOCK_REALTIME, &se, &t_id))
		return 0;
	
	struct itimerspec itimer;
	itimer.it_interval.tv_sec = ms/1000;
	itimer.it_interval.tv_nsec = (ms % 1000) * 1000000;
	itimer.it_value.tv_sec = ms/1000;
	itimer.it_value.tv_nsec = (ms % 1000) * 1000000;
	if (timer_settime (t_id, 0, &itimer, NULL) < 0)
		return 0;
	
	TimerManager::GetInstance().AddTimer(m_id, *this);
	return m_id;
}

void Timer::Destroy()
{
	if(!t_id)
		return;
	timer_delete(t_id);
	TimerManager::GetInstance().RemoveTimer(m_id);
}

ITimerProc* Timer::GetTimerProc()
{
	return m_proc;
}

int Timer::GetId()
{
	return m_id;
}

/************************************************************************************************/
/*                                       TimerManager                                           */
/************************************************************************************************/
TimerManager::TimerManager()
{
}

TimerManager::~TimerManager()
{
}

int TimerManager::CreateTimer(int ms, ITimerProc* proc)
{
	Timer timer(ms, proc);
	return timer.GetId();
}

void TimerManager::DeleteTimer(int id)
{
	Timer t;
	{
		CSLocker lock(&m_cs);
		typename std::map<int, Timer>::iterator it = m_timers.find(id);
		if(it != m_timers.end())
		{
			t = it->second;
		}
	}
	
	t.Destroy();
}

Timer TimerManager::GetTimer(int id)
{
	CSLocker lock(&m_cs);
	typename std::map<int, Timer>::iterator it = m_timers.find(id);
	if(it != m_timers.end())
	{
		return it->second;
	}
	
	return Timer();
}
	
void TimerManager::RemoveTimer(int id)
{
	CSLocker lock(&m_cs);
	typename std::map<int, Timer>::iterator it = m_timers.find(id);
	if(it != m_timers.end())
	{
		m_timers.erase(it);
	}
}

void TimerManager::AddTimer(int id, Timer timer)
{
	CSLocker lock(&m_cs);
	m_timers.insert(std::make_pair(id, timer));
}

int TimerManager::GetNextTimerId()
{
	return 0;
}

void TimerManager::ManagerStop()
{
}
