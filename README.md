# cron_timer

这是一个使用CronTab表达式的定时器，可以在指定时间点触发定时器事件，也可以在一段时间之后触发定时器事件。



待办事项：

| 编号 | 内容                                                         | 状态 |
| ---- | ------------------------------------------------------------ | ---- |
| 1    | 参考一下 https://www.zhihu.com/question/32251997/answer/1899420964?content_id=1379404441725026304&type=zvideo 优化定时器的出发逻辑 |      |
|      |                                                              |      |



特点：

1.对时间的表达能力强

CronTab表达式已经在Linux平台上广泛使用，需求久经考验



2.使用方便

一个头文件搞定一切，拷贝过去就可以使用，不依赖第三方库，Windows、Centos、Ubuntu、Mac都可以运行。

一行代码添加一个定时器，可传入成员函数，携带自定义参数



3.精度高、误差不累积



4.性能好

对于定时器内的对象个数，时间判断的时间复杂度做到了O(log(n))



5.可以选择在主线程中也可以选择在子线程中触发定时器。



在主线程中触发定时器事件的例子：

```
	// 如果你想在主线程中触发定时器事件
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

	auto timer = mgr->AddTimer(3, [](void) {
		// 3秒钟之后执行
		Log("3 second delay timer hit");
	});

	// 可以在执行之前取消
	timer->Cancel();

	mgr->AddTimer(
		10,
		[](void) {
			// 每10秒钟执行一次，总共执行3次
			Log("10 second delay timer hit");
		},
		3);

	while (!_shutDown) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		mgr->Update(); // 在主线程中触发定时器事件
	}
```



在子线程中触发定时器事件的例子：

```
	// 如果你想在子线程中触发定时器事件
	std::shared_ptr<cron_timer::TimerMgr> mgr = std::make_shared<cron_timer::TimerMgr>([&mgr]() {
		if (mgr != nullptr) {
			// 在子线程中触发定时器事件
			mgr->Update();
		}
	});

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

	auto timer = mgr->AddTimer(3, [](void) {
		// 3秒钟之后执行
		Log("3 second delay timer hit");
	});

	// 可以在执行之前取消
	timer->Cancel();

	mgr->AddTimer(
		10,
		[](void) {
			// 每10秒钟执行一次，总共执行3次
			Log("10 second delay timer hit");
		},
		3);

	while (!_shutDown) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
```



