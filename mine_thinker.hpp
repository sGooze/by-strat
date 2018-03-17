// mine_thinker.hpp

#pragma once

#include "common.hpp"

class Thinker {
protected:
	int32_t lastCall = 0;
	int32_t per;
	virtual bool ThinkInt() = 0;
public:
	Thinker(int32_t period, int32_t delay = 0) : per(period) { lastCall = SDL_GetTicks() + delay; }
	bool Think() { 
		auto i = SDL_GetTicks(); 
		int j = (i - lastCall);
		if (j < per) return true; 
		lastCall = i; bool k;
		while (j >= per) {
			k = ThinkInt();
			if (!k) return k;
			j -= per;
		}
		return true; 
	};
	virtual ~Thinker() {}
};

/// ThinkerSet: sets a specified var to a specified value after a delay
template <class V>
class ThinkerSet : public Thinker {
private:
	V& it; V targ;
	bool ThinkInt() { it = targ; return false; }
public:
	ThinkerSet(V& item, V target, int32_t delay)
		: it(item), targ(target), Thinker(1, delay) {}
};

/// ThinkerAdd: increments a specified var until it reaches a specified maximum
template <class V>
class ThinkerAdd : public Thinker {
private:
	V& it; V inc; V max; 
	bool ThinkInt() { it += inc; return /*(it < max)*/(max - it > 0.00001f); }
public:
	ThinkerAdd(V& item, V increment, V maximum, int32_t period, int32_t delay = 0)
		: it(item), inc(increment), max(maximum), Thinker(period, delay) {}
};

/// ThinkerAdd: increments a specified var until it reaches a specified maximum
template <class V>
class ThinkerSub : public Thinker {
private:
	V& it; V inc; V min;
	bool ThinkInt() { it -= inc; return /*(it > min)*/ (it - min > 0.00001f); }
public:
	ThinkerSub(V& item, V decrement, V minimum, int32_t period, int32_t delay = 0)
		: it(item), inc(decrement), min(minimum), Thinker(period, delay) {}
};

/// ThinkerFunc: calls a specified void function with a specified interval
class ThinkerFunc : public Thinker {
private:
	bool(*func)(void);
	bool ThinkInt() {  return func(); }
public:
	ThinkerFunc(bool(*function)(void), int32_t period = 1, int32_t delay = 0) : func(function), Thinker(period, delay) {}
};

/// ThinkerDelayedEvent: fires an event after a delay. can trigger once or a specified amount of times
class ThinkerDelayedEvent : public Thinker {
private:
	Event* ev;
	uint16_t times_left;
	bool ThinkInt() { EventQueue::AddEvent(ev); return (--times_left > 0); }
public:
	ThinkerDelayedEvent(Event* eventToFire, int32_t repeats = 1, int32_t delay = 1, uint32_t delay_first = 0)
		: ev(eventToFire), times_left(repeats), Thinker(delay, delay_first){}
};

// TODO: New thinker types:
//  increment/decrement a value X times

//*****//

/// ThinkerList: list of thinkers

class ThinkerList {
private:
	std::list<Thinker*> thinkers;
public:
	void Add(Thinker* think) { thinkers.push_back(think); }
	void Think() {
		for (auto it = thinkers.begin(); it != thinkers.end(); it++) {
			if (!(*it)->Think()) { 
				delete (*it);
				it = thinkers.erase(it); 
				if (it == thinkers.end()) return;
			}
		}
	}
	void Clean() {
		for (auto& x : thinkers) {
			delete x;
		}
		thinkers.clear();
	}
};