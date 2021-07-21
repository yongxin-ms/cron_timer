# cron_timer

这是一个使用CronTab表达式的定时器，可以在指定时间点触发定时器事件，也可以在一段时间之后触发定时器事件。



## 待办事项：

| 编号 | 内容                                                         | 状态                    |
| ---- | ------------------------------------------------------------ | ----------------------- |
| 1    | 参考一下 https://www.zhihu.com/question/32251997/answer/1899420964?content_id=1379404441725026304&type=zvideo 优化定时器的出发逻辑 | 20210525，xinyong已完成 |
| 2    | 在Update函数中查找最近的定时器，然后可以等待较长的时间，降低执行次数，提升效率 | 20210525，xinyong已完成 |
| 3    | 日志的时间表示从毫秒改成微秒，让误差更有说服力               | 20210611, xinyong已完成 |
| 4    | AddTimer的返回值应该由一个weak_ptr来接，当执行完毕之后其引用自动变为null_ptr | 20210628, xinyong已完成 |
| 5    | 支持在定时器处理函数中取消自己                               | 20210721, xinyong已完成 |
|      |                                                              |                         |
|      |                                                              |                         |





## 特点：

1. 对时间的表达能力强，CronTab表达式已经在Linux平台上广泛使用，可以参考 https://www.bejson.com/othertools/cron/?ivk_sa=1024320u 生成表达式，本组件对这个表达式少有修改，具体可以参考示例代码。
2. 使用方便，一个头文件搞定一切，拷贝过去就可以使用，不依赖第三方库，Windows、Centos、Ubuntu、Mac都可以运行。一行代码添加一个定时器，可传入成员函数，携带自定义参数。
3. 精度高、误差不累积，实测误差小于1毫秒。
4. 性能好，对于定时器内的对象个数，时间判断的时间复杂度做到了O(log(n))，可以支持海量的定时器对象。



## 使用示例

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

	std::weak_ptr<cron_timer::BaseTimer> timer = mgr.AddDelayTimer(3000, [](void) {
		// 3秒钟之后执行
		Log("3 second delay timer hit");
	});

	// 可以在执行之前取消
	if (auto ptr = timer.lock(); ptr != nullptr) {
		ptr->Cancel();
	}

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

精度有多高，**误差小于1毫秒**，数据说话：

```
2021-06-11 22:25:31.017870 10 second delay timer added
2021-06-11 22:25:31.017937 1 second cron timer hit
2021-06-11 22:25:32.000337 1 second cron timer hit
2021-06-11 22:25:33.000450 3 second cron timer hit
2021-06-11 22:25:33.000508 cron timer hit at 15s or 30s or 33s
2021-06-11 22:25:33.000534 1 second cron timer hit
2021-06-11 22:25:34.000368 1 second cron timer hit
2021-06-11 22:25:35.000398 1 second cron timer hit
2021-06-11 22:25:36.000455 3 second cron timer hit
2021-06-11 22:25:36.000512 1 second cron timer hit
2021-06-11 22:25:37.000385 1 second cron timer hit
2021-06-11 22:25:38.000349 1 second cron timer hit
2021-06-11 22:25:39.000380 3 second cron timer hit
2021-06-11 22:25:39.000449 1 second cron timer hit
2021-06-11 22:25:40.000544 cron timer hit between 40s to 50s
2021-06-11 22:25:40.000603 1 second cron timer hit
2021-06-11 22:25:41.000307 cron timer hit between 40s to 50s
2021-06-11 22:25:41.000361 1 second cron timer hit
2021-06-11 22:25:41.017942 10 second delay timer hit
2021-06-11 22:25:42.000431 3 second cron timer hit
2021-06-11 22:25:42.000488 cron timer hit between 40s to 50s
2021-06-11 22:25:42.000525 1 second cron timer hit
2021-06-11 22:25:43.000369 cron timer hit between 40s to 50s
2021-06-11 22:25:43.000425 1 second cron timer hit
2021-06-11 22:25:44.000370 cron timer hit between 40s to 50s
2021-06-11 22:25:44.000424 1 second cron timer hit
2021-06-11 22:25:45.000447 3 second cron timer hit
2021-06-11 22:25:45.000505 cron timer hit between 40s to 50s
2021-06-11 22:25:45.000532 1 second cron timer hit
2021-06-11 22:25:46.000357 cron timer hit between 40s to 50s
2021-06-11 22:25:46.000411 1 second cron timer hit
2021-06-11 22:25:47.000355 cron timer hit between 40s to 50s
2021-06-11 22:25:47.000408 1 second cron timer hit
2021-06-11 22:25:48.000438 3 second cron timer hit
2021-06-11 22:25:48.000496 cron timer hit between 40s to 50s
2021-06-11 22:25:48.000522 1 second cron timer hit

```



## 一些思路：

1.为什么没有使用时间轮？

答：时间轮定时器作为延时触发是很合适的，时间复杂度都是O(1)的，但是对于在固定时间点触发的定时器，我觉得会有累积误差，我暂时还没找到解决的方法，所以使用了红黑树，红黑树的时间复杂度是O（log(N)）的，性能也不错。

2.为什么误差能做到1毫秒以下？

答：对于定时器组件，需要提供一个“获取最近即将触发的时间”这样一个接口，有了这样一个接口，在消息队列中就可以做到堆这个时间的精确等待，这个等待操作不是简单的sleep几毫秒，是使用了CPU硬件提供的计时器功能，超时的时候触发一个中断。当然重要的是红黑树LogN的时间复杂度在发挥作用。

