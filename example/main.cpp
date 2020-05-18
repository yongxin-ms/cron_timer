#include <stdio.h>
#include <memory>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <signal.h>
#endif

#include "../include/cron_timer_mgr.h"


std::atomic_bool _shutDown = false;

#ifdef _WIN32
BOOL WINAPI ConsoleHandler(DWORD CtrlType) {
	switch (CtrlType) {
	case CTRL_C_EVENT:
	case CTRL_CLOSE_EVENT:
	case CTRL_LOGOFF_EVENT:
	case CTRL_SHUTDOWN_EVENT:
		_shutDown = true;
		break;

	default:
		break;
	}

	return TRUE;
}
#else
void signal_hander(int signo) //自定义一个函数处理信号
{
	printf("catch a signal:%d\n:", signo);
	App::Instance()->ShutDown();
}
#endif


int main() {
#ifdef _WIN32
	SetConsoleCtrlHandler(ConsoleHandler, TRUE);
	EnableMenuItem(GetSystemMenu(GetConsoleWindow(), false), SC_CLOSE, MF_GRAYED | MF_BYCOMMAND);
#else
	signal(SIGINT, signal_hander);
#endif

	std::shared_ptr<CronTimer::TimerMgr> cron_timer_mgr = std::make_shared<CronTimer::TimerMgr>();
	cron_timer_mgr->Start();

	// 1秒钟执行一次的定时器
	auto timer1 =
		cron_timer_mgr->AddTimer("* * * * * * *", [](void) { printf("1 second timer hit\n"); });

	//从0秒开始，每3秒钟执行一次的定时器
	auto timer2 =
		cron_timer_mgr->AddTimer("0/3 * * * * * *", [](void) { printf("3 second timer hit\n"); });

	// 1分钟执行一次（每次都在0秒的时候执行）的定时器
	auto timer3 =
		cron_timer_mgr->AddTimer("0 * * * * * *", [](void) { printf("1 minute timer hit\n"); });

	//指定时间点执行的定时器
	auto timer4 = cron_timer_mgr->AddTimer(
		"15;30;50 * * * * * *", [](void) { printf("timer hit at 15s or 30s or 50s\n"); });

	//指定时间段执行的定时器
	auto timer5 = cron_timer_mgr->AddTimer(
		"30-34 * * * * * *", [](void) { printf("timer hit at 30s, 31s, 32s, 33s, 34s\n"); });

	while (!_shutDown) {
		cron_timer_mgr->Update();
		std::this_thread::sleep_for(std::chrono::milliseconds(10));

		cron_timer_mgr->RemoveTimer(timer3);
	}

	cron_timer_mgr->RemoveTimer(timer1);
	cron_timer_mgr->RemoveTimer(timer2);
	cron_timer_mgr->RemoveTimer(timer3);
	cron_timer_mgr->RemoveTimer(timer4);
	cron_timer_mgr->RemoveTimer(timer5);

	cron_timer_mgr->Stop();
	return 0;
}
