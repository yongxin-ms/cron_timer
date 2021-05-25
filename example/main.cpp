#include <stdio.h>
#include <memory>
#include <atomic>
#include <thread>
#include <cstdarg>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <signal.h>
#endif

#include "../include/cron_timer.h"

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

std::string FormatDateTime(time_t t) {
	tm this_tm;
#ifdef _WIN32
	localtime_s(&this_tm, &t);
#else
	localtime_r(&t, &this_tm);
#endif // _WIN32
	char data[64] = {0};
	snprintf(data, sizeof(data) - 1, "%d-%02d-%02d %02d:%02d:%02d", this_tm.tm_year + 1900, this_tm.tm_mon + 1,
		this_tm.tm_mday, this_tm.tm_hour, this_tm.tm_min, this_tm.tm_sec);
	return std::string(data);
}

void Log(const char* fmt, ...) {
	char buf[4096];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf) - 1, fmt, args);
	va_end(args);

	std::string time_now = FormatDateTime(time(nullptr));
	printf("%s %s\n", time_now.c_str(), buf);
}

void TestCronTimerInMainThread() {
	auto mgr = std::make_shared<cron_timer::TimerMgr>();

	mgr->AddTimer("* * * * * *", [](void) {
		// 每秒钟都会执行一次
		Log("1 second cron timer hit");
	});

	mgr->AddTimer("0/3 * * * * *", [](void) {
		// 从0秒开始，每3秒钟执行一次
		Log("3 second cron timer hit");
	});

	mgr->AddTimer("0 * * * * *", [](void) {
		// 1分钟执行一次（每次都在0秒的时候执行）的定时器
		Log("1 minute cron timer hit");
	});

	mgr->AddTimer("15;30;33 * * * * *", [](void) {
		// 指定时间（15秒、30秒和33秒）点都会执行一次
		Log("cron timer hit at 15s or 30s or 33s");
	});

	mgr->AddTimer("40-50 * * * * *", [](void) {
		// 指定时间段（40到50内的每一秒）执行的定时器
		Log("cron timer hit between 40s to 50s");
	});

	auto timer = mgr->AddDelayTimer(3000, [](void) {
		// 3秒钟之后执行
		Log("3 second delay timer hit");
	});

	// 可以在执行之前取消
	timer->Cancel();

	mgr->AddDelayTimer(
		10000,
		[](void) {
			// 每10秒钟执行一次，总共执行3次
			Log("10 second delay timer hit");
		},
		3);

	while (!_shutDown) {
		auto nearest_timer =
			(std::min)(std::chrono::system_clock::now() + std::chrono::milliseconds(500), mgr->GetNearestTime());

		std::this_thread::sleep_until(nearest_timer);
		mgr->Update();
	}
}

int main() {
#ifdef _WIN32
	SetConsoleCtrlHandler(ConsoleHandler, TRUE);
	EnableMenuItem(GetSystemMenu(GetConsoleWindow(), false), SC_CLOSE, MF_GRAYED | MF_BYCOMMAND);
#else
	signal(SIGINT, signal_hander);
#endif

	TestCronTimerInMainThread();
	return 0;
}
