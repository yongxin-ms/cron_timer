// CronTimerTest.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "cron_timer.h"

void on_timer()
{
	printf("timer hit\n");
}

int main()
{
	CronTimer::TimerMgr g_mgr;
	g_mgr.Start();
	g_mgr.AddTimer("* * * * * * *", on_timer);

	while (true)
	{
		g_mgr.Update();
	}

	g_mgr.Stop();
    return 0;
}

