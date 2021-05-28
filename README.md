# cron_timer

这是一个使用CronTab表达式的定时器，可以在指定时间点触发定时器事件，也可以在一段时间之后触发定时器事件。



## 待办事项：

| 编号 | 内容                                                         | 状态                    |
| ---- | ------------------------------------------------------------ | ----------------------- |
| 1    | 参考一下 https://www.zhihu.com/question/32251997/answer/1899420964?content_id=1379404441725026304&type=zvideo 优化定时器的出发逻辑 |                         |
| 2    | 在Update函数中查找最近的定时器，然后可以等待较长的时间，降低执行次数，提升效率 | 20210525，xinyong已完成 |
|      |                                                              |                         |
|      |                                                              |                         |





## 特点：

1.对时间的表达能力强

CronTab表达式已经在Linux平台上广泛使用，需求久经考验



2.使用方便

一个头文件搞定一切，拷贝过去就可以使用，不依赖第三方库，Windows、Centos、Ubuntu、Mac都可以运行。

一行代码添加一个定时器，可传入成员函数，携带自定义参数



3.精度高、误差不累积



4.性能好

对于定时器内的对象个数，时间判断的时间复杂度做到了O(log(n))



## 使用示例

在主线程中触发定时器事件的例子：

```
	cron_timer::TimerMgr mgr;

	mgr.AddTimer("* * * * * *", [](void) {
		// 每秒钟都会执行一次
		Log("1 second cron timer hit");
	});

	mgr.AddTimer("0/3 * * * * *", [](void) {
		// 从0秒开始，每3秒钟执行一次
		Log("3 second cron timer hit");
	});

	mgr.AddTimer("0 * * * * *", [](void) {
		// 1分钟执行一次（每次都在0秒的时候执行）的定时器
		Log("1 minute cron timer hit");
	});

	mgr.AddTimer("15;30;33 * * * * *", [](void) {
		// 指定时间（15秒、30秒和33秒）点都会执行一次
		Log("cron timer hit at 15s or 30s or 33s");
	});

	mgr.AddTimer("40-50 * * * * *", [](void) {
		// 指定时间段（40到50内的每一秒）执行的定时器
		Log("cron timer hit between 40s to 50s");
	});

	auto timer = mgr.AddDelayTimer(3000, [](void) {
		// 3秒钟之后执行
		Log("3 second delay timer hit");
	});

	// 可以在执行之前取消
	timer->Cancel();

	mgr.AddDelayTimer(
		10000,
		[](void) {
			// 每10秒钟执行一次，总共执行3次
			Log("10 second delay timer hit");
		},
		3);
	Log("10 second delay timer added");

	while (!_shutDown) {
		auto nearest_timer =
			(std::min)(std::chrono::system_clock::now() + std::chrono::milliseconds(500), mgr.GetNearestTime());
		std::this_thread::sleep_until(nearest_timer);
		mgr.Update();
	}

```



## 测试数据

精度有多高，**误差0毫秒**，数据说话：

```
2021-05-28 15:13:45.143 10 second delay timer added
2021-05-28 15:13:45.143 1 second cron timer hit
2021-05-28 15:13:45.143 3 second cron timer hit
2021-05-28 15:13:45.143 cron timer hit between 40s to 50s
2021-05-28 15:13:46.000 1 second cron timer hit
2021-05-28 15:13:46.000 cron timer hit between 40s to 50s
2021-05-28 15:13:47.000 1 second cron timer hit
2021-05-28 15:13:47.000 cron timer hit between 40s to 50s
2021-05-28 15:13:48.000 3 second cron timer hit
2021-05-28 15:13:48.000 1 second cron timer hit
2021-05-28 15:13:48.000 cron timer hit between 40s to 50s
2021-05-28 15:13:49.000 1 second cron timer hit
2021-05-28 15:13:49.000 cron timer hit between 40s to 50s
2021-05-28 15:13:50.000 1 second cron timer hit
2021-05-28 15:13:50.000 cron timer hit between 40s to 50s
2021-05-28 15:13:51.000 3 second cron timer hit
2021-05-28 15:13:51.000 1 second cron timer hit
2021-05-28 15:13:52.000 1 second cron timer hit
2021-05-28 15:13:53.000 1 second cron timer hit
2021-05-28 15:13:54.000 3 second cron timer hit
2021-05-28 15:13:54.000 1 second cron timer hit
2021-05-28 15:13:55.000 1 second cron timer hit
2021-05-28 15:13:55.143 10 second delay timer hit
2021-05-28 15:13:56.000 1 second cron timer hit
2021-05-28 15:13:57.000 3 second cron timer hit
2021-05-28 15:13:57.000 1 second cron timer hit
2021-05-28 15:13:58.000 1 second cron timer hit
2021-05-28 15:13:59.000 1 second cron timer hit
2021-05-28 15:14:00.000 1 minute cron timer hit
2021-05-28 15:14:00.000 3 second cron timer hit
2021-05-28 15:14:00.000 1 second cron timer hit
2021-05-28 15:14:01.000 1 second cron timer hit
2021-05-28 15:14:02.000 1 second cron timer hit
2021-05-28 15:14:03.000 3 second cron timer hit
2021-05-28 15:14:03.000 1 second cron timer hit
2021-05-28 15:14:04.000 1 second cron timer hit
2021-05-28 15:14:05.000 1 second cron timer hit
2021-05-28 15:14:05.143 10 second delay timer hit
2021-05-28 15:14:06.000 3 second cron timer hit
2021-05-28 15:14:06.000 1 second cron timer hit
2021-05-28 15:14:07.000 1 second cron timer hit

```



## 一些思路：

1.为什么没有使用时间轮？

答：时间轮定时器作为延时触发是很合适的，时间复杂度都是O(1)的，但是对于在固定时间点触发的定时器，我觉得会有累积误差，我暂时还没找到解决的方法，所以使用了红黑树，红黑树的时间复杂度是O（log(N)）的，性能也不错。



