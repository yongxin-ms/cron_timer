#include <stdio.h>
#include <memory>
#include <atomic>
#include <thread>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <signal.h>
#endif

#include "../include/cron_timer.h"
#include "../include/call_later.h"

std::atomic_bool _shutDown;

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
	_shutDown = true;
}
#endif

void TestCronTimer() {
	std::unique_ptr<cron_timer::TimerMgr> timer_mgr = std::make_unique<cron_timer::TimerMgr>();

	// 1秒钟执行一次的定时器
	auto timer1 = timer_mgr->AddTimer("* * * * * *", [](void) { printf("1 second timer hit\n"); });

	//从0秒开始，每3秒钟执行一次的定时器
	auto timer2 = timer_mgr->AddTimer("0/3 * * * * *", [](void) { printf("3 second timer hit\n"); });

	// 1分钟执行一次（每次都在0秒的时候执行）的定时器
	auto timer3 = timer_mgr->AddTimer("0 * * * * *", [](void) { printf("1 minute timer hit\n"); });

	//指定时间点执行的定时器
	auto timer4 = timer_mgr->AddTimer("15;30;50 * * * * *", [](void) { printf("timer hit at 15s or 30s or 50s\n"); });

	//指定时间段执行的定时器
	auto timer5 =
		timer_mgr->AddTimer("30-34 * * * * *", [](void) { printf("timer hit at 30s, 31s, 32s, 33s, 34s\n"); });

	while (!_shutDown) {
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		timer_mgr->Update();
	}

	timer1->Cancel();
	timer2->Cancel();
	timer3->Cancel();
	timer4->Cancel();
	timer5->Cancel();
}

void TestLaterTimer() {
	std::unique_ptr<call_later::TimerMgr> timer_mgr = std::make_unique<call_later::TimerMgr>();

	// 3秒钟之后执行
	auto timer1 = timer_mgr->AddTimer(3, [](void) { printf("3 second timer hit\n"); });

	// 中途可以取消
	timer1->Cancel();

	// 10秒钟之后执行
	auto timer2 = timer_mgr->AddTimer(10, [](void) { printf("10 second timer hit\n"); }, 3);

	while (!_shutDown) {
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		timer_mgr->Update();
	}
}

int main() {
#ifdef _WIN32
	SetConsoleCtrlHandler(ConsoleHandler, TRUE);
	EnableMenuItem(GetSystemMenu(GetConsoleWindow(), false), SC_CLOSE, MF_GRAYED | MF_BYCOMMAND);
#else
	signal(SIGINT, signal_hander);
#endif

	// TestCronTimer();
	TestLaterTimer();
	return 0;
}
