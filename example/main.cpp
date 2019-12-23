#include <stdio.h>
#include <memory>
#include "../include/cron_timer_mgr.h"

int main() {
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

	while (true) {
		cron_timer_mgr->Update();
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	cron_timer_mgr->RemoveTimer(timer1);
	cron_timer_mgr->RemoveTimer(timer2);
	cron_timer_mgr->RemoveTimer(timer3);
	cron_timer_mgr->RemoveTimer(timer4);
	cron_timer_mgr->RemoveTimer(timer5);

	cron_timer_mgr->Stop();
	return 0;
}
