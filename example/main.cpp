#include "../include/cron_timer_mgr.h"

void on_1_second_timer() {
	printf("1 second timer hit\n");
}

CronTimer::TimerMgr g_timer_mgr;

int main() {
	g_timer_mgr.Start();

	//使用全局函数的例子
	g_timer_mgr.AddTimer("* * * * * * *", on_1_second_timer);
	
	//使用lamda表达式的例子
	g_timer_mgr.AddTimer("0/3 * * * * * *", [](void){
		printf("3 second timer hit\n");
	});

	while (true) {
		g_timer_mgr.Update();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	g_timer_mgr.Stop();
    return 0;
}

