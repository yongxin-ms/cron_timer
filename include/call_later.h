#pragma once
#include <memory>
#include <string>
#include <list>
#include <vector>
#include <map>
#include <functional>
#include <assert.h>
#include <time.h>

namespace call_later {

class TimerMgr;
using FUNC_CALLBACK = std::function<void()>;

class Timer : public std::enable_shared_from_this<Timer> {
	friend class TimerMgr;
public:
	Timer(TimerMgr& owner, uint32_t seconds, const FUNC_CALLBACK& func)
		: owner_(owner)
		, seconds_(seconds)
		, func_(func) {
		cur_time_ = time(nullptr);
		Next();
	}

	void Cancel();

private:
	void SetIt(const std::list<std::shared_ptr<Timer>>::iterator& it) { it_ = it; }
	std::list<std::shared_ptr<Timer>>::iterator& GetIt() { return it_; }

	void DoFunc();
	time_t GetCurTime() const { return cur_time_; }
	void Next() {
		time_t time_now = time(nullptr);
		while (true) {
			cur_time_ += seconds_;
			if (cur_time_ >= time_now) {
				break;
			}
		}
	}

private:
	TimerMgr& owner_;
	const uint32_t seconds_;
	const FUNC_CALLBACK func_;
	std::list<std::shared_ptr<Timer>>::iterator it_;
	time_t cur_time_;
};

class TimerMgr {
	friend class Timer;

public:
	~TimerMgr() { Stop(); }
	void Stop() { timers_.clear(); }

	std::shared_ptr<Timer> AddTimer(int seconds, const FUNC_CALLBACK& func) {
		auto p = std::make_shared<Timer>(*this, seconds, func);
		return insert(p);
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

private:
	std::shared_ptr<Timer> insert(std::shared_ptr<Timer> p) {
		time_t t = p->GetCurTime();
		auto it = timers_.find(t);
		if (it == timers_.end()) {
			std::list<std::shared_ptr<Timer>> l;
			timers_.insert(std::make_pair(t, l));
			it = timers_.find(t);
		}

		auto& l = it->second;
		p->SetIt(l.insert(l.end(), p));
		return p;
	}

	bool remove(std::shared_ptr<Timer> p) {
		time_t t = p->GetCurTime();
		auto it = timers_.find(t);
		if (it == timers_.end()) {
			assert(false);
			return false;
		}

		auto& l = it->second;
		l.erase(p->GetIt());
		p->SetIt(l.end());
		return true;
	}

private:
	std::map<time_t, std::list<std::shared_ptr<Timer>>> timers_;
	time_t last_proc_ = 0;
};

void Timer::Cancel() {
	auto self = shared_from_this();
	owner_.remove(self);
}

void Timer::DoFunc() {
	func_();
	auto self = shared_from_this();
	owner_.remove(self);
}

} // namespace cron_timer
