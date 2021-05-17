#pragma once
#include <string>
#include <list>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <assert.h>
#include <time.h>
#include <cstring>

/*
	std::unique_ptr<cron_timer::TimerMgr> timer_mgr = std::make_unique<cron_timer::TimerMgr>();

	// 1秒钟执行一次的定时器
	timer_mgr->AddTimer("* * * * * *", [](void) { printf("1 second cron timer hit\n"); });

	//从0秒开始，每3秒钟执行一次的定时器
	timer_mgr->AddTimer("0/3 * * * * *", [](void) { printf("3 second cron timer hit\n"); });

	// 1分钟执行一次（每次都在0秒的时候执行）的定时器
	timer_mgr->AddTimer("0 * * * * *", [](void) { printf("1 minute cron timer hit\n"); });

	//指定时间点执行的定时器
	timer_mgr->AddTimer("15;30;50 * * * * *", [](void) { printf("cron timer hit at 15s or 30s or 50s\n"); });

	//指定时间段执行的定时器
	timer_mgr->AddTimer("30-34 * * * * *", [](void) { printf("cron timer hit at 30s, 31s, 32s, 33s, 34s\n"); });

	// 3秒钟之后执行
	auto timer = timer_mgr->AddTimer(3, [](void) { printf("3 second delay timer hit\n"); });

	// 中途可以取消
	timer->Cancel();

	// 10秒钟之后执行
	timer_mgr->AddTimer(
		10, [](void) { printf("10 second delay timer hit\n"); }, 3);

	while (!_shutDown) {
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		timer_mgr->Update();
	}
*/

namespace cron_timer {
class Text {
public:
	static size_t SplitStr(std::vector<std::string>& os, const std::string& is, const char c) {
		os.clear();
		std::string::size_type pos_find = is.find_first_of(c, 0);
		std::string::size_type pos_last_find = 0;
		while (std::string::npos != pos_find) {
			std::string::size_type num_char = pos_find - pos_last_find;
			os.emplace_back(is.substr(pos_last_find, num_char));

			pos_last_find = pos_find + 1;
			pos_find = is.find_first_of(c, pos_last_find);
		}

		std::string::size_type num_char = is.length() - pos_last_find;
		if (num_char > is.length())
			num_char = is.length();

		std::string sub = is.substr(pos_last_find, num_char);
		os.emplace_back(sub);
		return os.size();
	}

	static size_t SplitInt(std::vector<int>& number_result, const std::string& is, const char c) {
		std::vector<std::string> string_result;
		SplitStr(string_result, is, c);

		number_result.clear();
		for (size_t i = 0; i < string_result.size(); i++) {
			const std::string& value = string_result[i];
			number_result.emplace_back(atoi(value.data()));
		}

		return number_result.size();
	}
};

class CronExpression {
public:
	enum DATA_TYPE {
		DT_SECOND = 0,
		DT_MINUTE = 1,
		DT_HOUR = 2,
		DT_DAY_OF_MONTH = 3,
		DT_MONTH = 4,
		DT_YEAR = 5,
		DT_MAX,
	};

	static bool GetValues(const std::string& input, DATA_TYPE data_type, std::vector<int>& values) {

		//
		// 注意：枚举之前是','，为了能在csv中使用改成了';'
		// 20181226 xinyong
		//

		static const char CRON_SEPERATOR_ENUM = ';';
		static const char CRON_SEPERATOR_RANGE = '-';
		static const char CRON_SEPERATOR_INTERVAL = '/';

		if (input == "*") {
			auto pair_range = GetRangeFromType(data_type);
			for (auto i = pair_range.first; i <= pair_range.second; ++i) {
				values.push_back(i);
			}
		} else if (input.find_first_of(CRON_SEPERATOR_ENUM) != std::string::npos) {
			//枚举
			std::vector<int> v;
			Text::SplitInt(v, input, CRON_SEPERATOR_ENUM);
			std::pair<int, int> pair_range = GetRangeFromType(data_type);
			for (auto value : v) {
				if (value < pair_range.first || value > pair_range.second) {
					return false;
				}
				values.push_back(value);
			}
		} else if (input.find_first_of(CRON_SEPERATOR_RANGE) != std::string::npos) {
			//范围
			std::vector<int> v;
			Text::SplitInt(v, input, CRON_SEPERATOR_RANGE);
			if (v.size() != 2) {
				return false;
			}

			int from = v[0];
			int to = v[1];
			std::pair<int, int> pair_range = GetRangeFromType(data_type);
			if (from < pair_range.first || to > pair_range.second) {
				return false;
			}

			for (int i = from; i <= to; i++) {
				values.push_back(i);
			}
		} else if (input.find_first_of(CRON_SEPERATOR_INTERVAL) != std::string::npos) {
			//间隔
			std::vector<int> v;
			Text::SplitInt(v, input, CRON_SEPERATOR_INTERVAL);
			if (v.size() != 2) {
				return false;
			}

			int from = v[0];
			int interval = v[1];
			std::pair<int, int> pair_range = GetRangeFromType(data_type);
			if (from < pair_range.first || interval < 0) {
				return false;
			}

			for (int i = from; i <= pair_range.second; i += interval) {
				values.push_back(i);
			}
		} else {
			//具体数值
			std::pair<int, int> pair_range = GetRangeFromType(data_type);
			int value = atoi(input.data());
			if (value < pair_range.first || value > pair_range.second) {
				return false;
			}

			values.push_back(value);
		}

		assert(values.size() > 0);
		return values.size() > 0;
	}

private:
	static std::pair<int, int> GetRangeFromType(DATA_TYPE data_type) {
		int from = 0;
		int to = 0;

		switch (data_type) {
		case CronExpression::DT_SECOND:
		case CronExpression::DT_MINUTE:
			from = 0;
			to = 59;
			break;
		case CronExpression::DT_HOUR:
			from = 0;
			to = 23;
			break;
		case CronExpression::DT_DAY_OF_MONTH:
			from = 1;
			to = 31;
			break;
		case CronExpression::DT_MONTH:
			from = 1;
			to = 12;
			break;
		case CronExpression::DT_YEAR:
			from = 1970;
			to = 2099;
			break;
		case CronExpression::DT_MAX:
			assert(false);
			break;
		}

		return std::make_pair(from, to);
	}
};

struct CronWheel {
	CronWheel()
		: cur_index(0) {}

	//返回值：是否有进位
	bool init(int init_value) {
		for (size_t i = cur_index; i < values.size(); ++i) {
			if (values[i] >= init_value) {
				cur_index = i;
				return false;
			}
		}

		cur_index = 0;
		return true;
	}

	size_t cur_index;
	std::vector<int> values;
};

class TimerMgr;
using FUNC_CALLBACK = std::function<void()>;

class BaseTimer : public std::enable_shared_from_this<BaseTimer> {
public:
	BaseTimer(TimerMgr& owner, const FUNC_CALLBACK& func)
		: owner_(owner)
		, func_(func) {}

	void Cancel();
	void SetIt(const std::list<std::shared_ptr<BaseTimer>>::iterator& it) { it_ = it; }
	std::list<std::shared_ptr<BaseTimer>>::iterator& GetIt() { return it_; }

	virtual void DoFunc() = 0;
	virtual time_t GetCurTime() const = 0;

protected:
	TimerMgr& owner_;
	const FUNC_CALLBACK func_;
	std::list<std::shared_ptr<BaseTimer>>::iterator it_;
};

class CronTimer : public BaseTimer {
public:
	CronTimer(TimerMgr& owner, const std::vector<CronWheel>& wheels, const FUNC_CALLBACK& func)
		: BaseTimer(owner, func)
		, wheels_(wheels) {
		tm local_tm;
		time_t time_now = time(nullptr);

#ifdef _WIN32
		localtime_s(&local_tm, &time_now);
#else
		localtime_r(&time_now, &local_tm);
#endif // _WIN32

		std::vector<int> init_values;
		init_values.push_back(local_tm.tm_sec);
		init_values.push_back(local_tm.tm_min);
		init_values.push_back(local_tm.tm_hour);
		init_values.push_back(local_tm.tm_mday);
		init_values.push_back(local_tm.tm_mon + 1);
		init_values.push_back(local_tm.tm_year + 1900);

		bool addup = false;
		for (int i = 0; i < CronExpression::DT_MAX; i++) {
			auto& wheel = wheels_[i];
			auto init_value = addup ? init_values[i] + 1 : init_values[i];
			addup = wheel.init(init_value);
		}
	}

	virtual void DoFunc() override;
	virtual time_t GetCurTime() const override {
		tm next_tm;
		memset(&next_tm, 0, sizeof(next_tm));
		next_tm.tm_sec = GetCurValue(CronExpression::DT_SECOND);
		next_tm.tm_min = GetCurValue(CronExpression::DT_MINUTE);
		next_tm.tm_hour = GetCurValue(CronExpression::DT_HOUR);
		next_tm.tm_mday = GetCurValue(CronExpression::DT_DAY_OF_MONTH);
		next_tm.tm_mon = GetCurValue(CronExpression::DT_MONTH) - 1;
		next_tm.tm_year = GetCurValue(CronExpression::DT_YEAR) - 1900;

		return mktime(&next_tm);
	}

private:
	void Next(int data_type) {
		if (data_type >= CronExpression::DT_MAX)
			return;

		auto& wheel = wheels_[data_type];
		if (wheel.cur_index == wheel.values.size() - 1) {
			wheel.cur_index = 0;
			Next(data_type + 1);
		} else {
			++wheel.cur_index;
		}
	}

	int GetCurValue(int data_type) const {
		const auto& wheel = wheels_[data_type];
		return wheel.values[wheel.cur_index];
	}

private:
	std::vector<CronWheel> wheels_;
};

class LaterTimer : public BaseTimer {
public:
	LaterTimer(TimerMgr& owner, uint32_t seconds, const FUNC_CALLBACK& func, int count)
		: BaseTimer(owner, func)
		, seconds_(seconds)
		, count_left_(count) {
		cur_time_ = time(nullptr);
		Next();
	}

	virtual void DoFunc() override;
	virtual time_t GetCurTime() const override { return cur_time_; }

private:
	void Next() {
		time_t time_now = time(nullptr);
		while (true) {
			cur_time_ += seconds_;
			if (cur_time_ > time_now) {
				break;
			}
		}
	}

private:
	const uint32_t seconds_;
	time_t cur_time_;
	int count_left_;
};

class TimerMgr {
public:
	~TimerMgr() { timers_.clear(); }

	std::shared_ptr<BaseTimer> AddTimer(const std::string& timer_string, const FUNC_CALLBACK& func) {
		std::vector<std::string> v;
		Text::SplitStr(v, timer_string, ' ');
		if (v.size() != CronExpression::DT_MAX)
			return nullptr;

		std::vector<CronWheel> wheels;
		for (int i = 0; i < CronExpression::DT_MAX; i++) {
			const auto& expression = v[i];
			CronExpression::DATA_TYPE data_type = CronExpression::DATA_TYPE(i);
			CronWheel wheel;
			if (!CronExpression::GetValues(expression, data_type, wheel.values)) {
				return nullptr;
			}

			wheels.emplace_back(wheel);
		}

		auto p = std::make_shared<CronTimer>(*this, wheels, func);
		insert(p);
		return p;
	}

	std::shared_ptr<BaseTimer> AddTimer(int seconds, const FUNC_CALLBACK& func, int count = 1) {
		auto p = std::make_shared<LaterTimer>(*this, seconds, func, count);
		insert(p);
		return p;
	}

	size_t Update() {
		time_t time_now = time(nullptr);
		size_t count = 0;
		if (time_now == last_proc_)
			return 0;

		last_proc_ = time_now;
		while (!timers_.empty()) {
			auto& first = *timers_.begin();
			auto expire_time = first.first;
			auto& timer_list = first.second;
			if (expire_time > time_now) {
				break;
			}

			while (!timer_list.empty()) {
				auto p = *timer_list.begin();
				p->DoFunc();
				++count;
			}

			timers_.erase(timers_.begin());
		}

		return count;
	}

	void insert(std::shared_ptr<BaseTimer> p) {
		time_t t = p->GetCurTime();
		auto it = timers_.find(t);
		if (it == timers_.end()) {
			std::list<std::shared_ptr<BaseTimer>> l;
			timers_.insert(std::make_pair(t, l));
			it = timers_.find(t);
		}

		auto& l = it->second;
		p->SetIt(l.insert(l.end(), p));
	}

	bool remove(std::shared_ptr<BaseTimer> p) {
		time_t t = p->GetCurTime();
		auto it = timers_.find(t);
		if (it == timers_.end()) {
			return false;
		}

		auto& l = it->second;
		l.erase(p->GetIt());
		p->SetIt(l.end());
		return true;
	}

private:
	std::map<time_t, std::list<std::shared_ptr<BaseTimer>>> timers_;
	time_t last_proc_ = 0;
};

void BaseTimer::Cancel() {
	auto self = shared_from_this();
	owner_.remove(self);
}

void CronTimer::DoFunc() {
	func_();
	auto self = shared_from_this();
	owner_.remove(self);
	Next(CronExpression::DT_SECOND);
	owner_.insert(self);
}

void LaterTimer::DoFunc() {
	func_();
	auto self = shared_from_this();
	owner_.remove(self);

	if (--count_left_ > 0) {
		Next();
		owner_.insert(self);
	}
}

} // namespace cron_timer
