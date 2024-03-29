#pragma once
#include <string>
#include <set>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <assert.h>
#include <time.h>
#include <cstring>

/*
	cron_timer::TimerMgr mgr;

	mgr.AddTimer("* * * * * *", [](void) {
		Log("1 second cron timer hit");
	});

	mgr.AddTimer("0/3 * * * * *", [](void) {
		Log("3 second cron timer hit");
	});

	mgr.AddTimer("0 * * * * *", [](void) {
		Log("1 minute cron timer hit");
	});

	mgr.AddTimer("15;30;33 * * * * *", [](void) {
		Log("cron timer hit at 15s or 30s or 33s");
	});

	mgr.AddTimer("40-50 * * * * *", [](void) {
		Log("cron timer hit between 40s to 50s");
	});

	std::weak_ptr<cron_timer::BaseTimer> timer = mgr.AddDelayTimer(3000, [](void) {
		Log("3 second delay timer hit");
	});

	if (auto ptr = timer.lock(); ptr != nullptr) {
		ptr->Cancel();
	}

	mgr.AddDelayTimer(
		10000,
		[](void) {
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
*/

namespace cron_timer {
class Text {
public:
	// Used to split strings separated by spaces, consecutive spaces are counted as a separator
	static size_t SplitStr(std::vector<std::string>& os, const std::string& is, char c) {
		os.clear();
		auto start = is.find_first_not_of(c, 0);
		while (start != std::string::npos) {
			auto end = is.find_first_of(c, start);
			if (end == std::string::npos) {
				os.emplace_back(is.substr(start));
				break;
			} else {
				os.emplace_back(is.substr(start, end - start));
				start = is.find_first_not_of(c, end + 1);
			}
		}
		return os.size();
	}

	static size_t SplitInt(std::vector<int>& number_result, const std::string& is, char c) {
		std::vector<std::string> string_result;
		SplitStr(string_result, is, c);

		number_result.clear();
		for (size_t i = 0; i < string_result.size(); i++) {
			const std::string& value = string_result[i];
			number_result.emplace_back(atoi(value.data()));
		}

		return number_result.size();
	}

	static std::vector<std::string> ParseParam(const std::string& is, char c) {
		std::vector<std::string> result;
		ParseParam(result, is, c);
		return result;
	}

	// Used to segment strings separated by commas, consecutive commas are counted as multiple delimiters
	static size_t ParseParam(std::vector<std::string>& result, const std::string& is, char c) {
		result.clear();
		size_t start = 0;
		while (start < is.size()) {
			auto end = is.find_first_of(c, start);
			if (end != std::string::npos) {
				result.emplace_back(is.substr(start, end - start));
				start = end + 1;
			} else {
				result.emplace_back(is.substr(start));
				break;
			}
		}

		if (start == is.size()) {
			result.emplace_back(std::string());
		}
		return result.size();
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
		// attention: enum seperater is ';' not ',' for using it in csv
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
			// enum
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
			// range
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
			// interval
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
			// specific value
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

class TimerMgr;
class BaseTimer;
using FUNC_CALLBACK = std::function<void()>;
using TimerPtr = std::shared_ptr<BaseTimer>;

class BaseTimer : public std::enable_shared_from_this<BaseTimer> {
	friend class TimerMgr;

public:
	BaseTimer(TimerMgr& owner, FUNC_CALLBACK&& func, int count)
		: m_owner(owner)
		, m_func(std::move(func))
		, m_triggerTime(std::chrono::system_clock::now())
		, m_countLeft(count)
		, m_canceled(false) {}
	virtual ~BaseTimer() {}
	inline void Cancel();

	// trigger time of the timer
	std::chrono::system_clock::time_point GetTriggerTime() const { return m_triggerTime; }

private:
	virtual void CreateTriggerTime(bool next) = 0;
	inline void DoFunc();

protected:
	TimerMgr& m_owner;
	FUNC_CALLBACK m_func;
	std::chrono::system_clock::time_point m_triggerTime;
	int m_countLeft;
	bool m_canceled;
};

struct CronWheel {
	CronWheel()
		: cur_index(0) {}

	size_t cur_index;
	std::vector<int> values;
};

class CronTimer : public BaseTimer {
	friend class TimerMgr;

public:
	CronTimer(TimerMgr& owner, std::vector<CronWheel>&& wheels, FUNC_CALLBACK&& func, int count)
		: BaseTimer(owner, std::move(func), count)
		, m_wheels(std::move(wheels)) {
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

		std::pair<size_t, bool> pairValue = std::make_pair(0, false);
		for (int i = CronExpression::DT_YEAR; i >= 0; i--) {
			pairValue = GetMinValid(i, init_values[i], pairValue.second);
			m_wheels[i].cur_index = pairValue.first;
		}
	}

private:
	virtual void CreateTriggerTime(bool next) {
		if (next) {
			Next(CronExpression::DT_SECOND);
		}

		tm next_tm;
		memset(&next_tm, 0, sizeof(next_tm));
		next_tm.tm_sec = GetCurValue(CronExpression::DT_SECOND);
		next_tm.tm_min = GetCurValue(CronExpression::DT_MINUTE);
		next_tm.tm_hour = GetCurValue(CronExpression::DT_HOUR);
		next_tm.tm_mday = GetCurValue(CronExpression::DT_DAY_OF_MONTH);
		next_tm.tm_mon = GetCurValue(CronExpression::DT_MONTH) - 1;
		next_tm.tm_year = GetCurValue(CronExpression::DT_YEAR) - 1900;

		m_triggerTime = std::chrono::system_clock::from_time_t(mktime(&next_tm));
	}

	// move to the next trigger time
	void Next(int data_type) {
		if (data_type >= CronExpression::DT_MAX) {
			// overflowed, this timer is invalid, should be removed
			m_canceled = true;
			return;
		}

		auto& wheel = m_wheels[data_type];
		if (wheel.cur_index == wheel.values.size() - 1) {
			wheel.cur_index = 0;
			Next(data_type + 1);
		} else {
			++wheel.cur_index;
		}
	}

	// return index, is changed
	std::pair<size_t, bool> GetMinValid(int data_type, int value, bool changed) const {
		auto& wheel = m_wheels[data_type];
		if (changed) {
			return std::make_pair(0, true);
		}

		for (size_t i = 0; i < wheel.values.size(); i++) {
			if (wheel.values[i] < value) {
				continue;
			} else if (wheel.values[i] == value) {
				return std::make_pair(i, false);
			} else {
				return std::make_pair(i, true);
			}
		}

		return std::make_pair(0, true);
	}

	int GetCurValue(int data_type) const {
		const auto& wheel = m_wheels[data_type];
		return wheel.values[wheel.cur_index];
	}

private:
	std::vector<CronWheel> m_wheels;
};

class LaterTimer : public BaseTimer {
	friend class TimerMgr;

public:
	LaterTimer(TimerMgr& owner, int milliseconds, FUNC_CALLBACK&& func, int count)
		: BaseTimer(owner, std::move(func), count)
		, m_milliSeconds(milliseconds) {}

private:
	virtual void CreateTriggerTime(bool next) { m_triggerTime += std::chrono::milliseconds(m_milliSeconds); }

private:
	const int m_milliSeconds;
};

class TimerMgr {
	friend class BaseTimer;
	friend class CronTimer;
	friend class LaterTimer;

public:
	TimerMgr() {}
	TimerMgr(const TimerMgr&) = delete;
	const TimerMgr& operator=(const TimerMgr&) = delete;

	void Stop() { m_timers.clear(); }

	enum {
		RUN_FOREVER = 0,
	};

	TimerPtr AddTimer(const std::string& timer_string, FUNC_CALLBACK&& func, int count = RUN_FOREVER) {
		std::vector<std::string> v;
		Text::SplitStr(v, timer_string, ' ');
		if (v.size() != CronExpression::DT_MAX) {
			assert(false);
			return nullptr;
		}

		std::vector<CronWheel> wheels;
		for (int i = 0; i < CronExpression::DT_MAX; i++) {
			const auto& expression = v[i];
			CronExpression::DATA_TYPE data_type = CronExpression::DATA_TYPE(i);
			CronWheel wheel;
			if (!CronExpression::GetValues(expression, data_type, wheel.values)) {
				assert(false);
				return nullptr;
			}

			wheels.emplace_back(wheel);
		}

		auto p = std::make_shared<CronTimer>(*this, std::move(wheels), std::move(func), count);
		p->CreateTriggerTime(false);
		insert(p);
		return p;
	}

	TimerPtr AddDelayTimer(int milliseconds, FUNC_CALLBACK&& func, int count = 1) {
		assert(milliseconds > 0);
		milliseconds = (std::max)(milliseconds, 1);
		auto p = std::make_shared<LaterTimer>(*this, milliseconds, std::move(func), count);
		p->CreateTriggerTime(true);
		insert(p);
		return p;
	}

	std::chrono::system_clock::time_point GetNearestTime() {
		auto it = m_timers.begin();
		if (it == m_timers.end()) {
			return (std::chrono::system_clock::time_point::max)();
		} else {
			return it->first;
		}
	}

	size_t Update() {
		auto time_now = std::chrono::system_clock::now();
		size_t count = 0;

		for (auto it = m_timers.begin(); it != m_timers.end();) {
			auto expire_time = it->first;
			if (expire_time > time_now) {
				break;
			}

			// attention: this is a copy, not a reference
			auto timer_set = it->second;
			it = m_timers.erase(it);

			for (auto p : timer_set) {
				p->DoFunc();
				++count;
			}
		}

		return count;
	}

private:
	void insert(const TimerPtr& p) {
		auto t = p->GetTriggerTime();
		auto it = m_timers.find(t);
		if (it == m_timers.end()) {
			std::set<TimerPtr> s;
			s.insert(p);
			m_timers[t] = s;
		} else {
			std::set<TimerPtr>& s = it->second;
			s.insert(p);
		}
	}

	void remove(const TimerPtr& p) {
		auto t = p->GetTriggerTime();
		auto it = m_timers.find(t);
		if (it == m_timers.end()) {
			return;
		}

		std::set<TimerPtr>& s = it->second;
		s.erase(p);
	}

private:
	std::map<std::chrono::system_clock::time_point, std::set<TimerPtr>> m_timers;
};

void BaseTimer::Cancel() {
	auto self = shared_from_this();
	m_owner.remove(self);
	m_canceled = true;
}

void BaseTimer::DoFunc() {
	m_func();
	CreateTriggerTime(true);

	// the timer can be cancelled in m_func()
	if (!m_canceled) {
		if (m_countLeft == TimerMgr::RUN_FOREVER || m_countLeft > 1) {
			if (m_countLeft > 1) {
				m_countLeft--;
			}

			auto self = shared_from_this();
			m_owner.insert(self);
		}
	}
}

} // namespace cron_timer
