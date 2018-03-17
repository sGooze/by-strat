// mine_events.hpp
//	PURPOSE: Events subsystem

#pragma once

#include <string>

typedef enum{EVENT_ANY, EVENT_GAME, EVENT_AUDIO, EVENT_SENK, EVENT_SHUT} EventType;
const std::string defaultSender = "anonymous";

extern int totalEvents;
extern int destroyedEvents;

class Event {
protected:
	//std::string sender;
public:
	void* arguments;
	const EventType type;
	Event(EventType eType, void* args = nullptr) : type(eType), arguments(args) { totalEvents++; }
	virtual ~Event() { destroyedEvents++; }
};

// Event queue
// TODO: ensure that extracted events are _deleted_, but not before their contents are used
class EventQueue {
private:
	static std::list<Event*> eventList;
public:
	static void AddEvent(Event* event) { eventList.push_back(event); }
	static bool PollEvent(EventType type, Event** returned);
	static void TestContents() {
		int i = 0;
		for (auto it = eventList.begin(); it != eventList.end(); it++) {
			std::cout << i++ << ": Type=" << (((*it)->type == EVENT_GAME) ? "GAME\n" : "NotGAme\n");
		}
		std::cout << "Total: " << eventList.size() << std::endl;
	}
};

// GameEvent
// Used to control game flow (start/stop/pause/exit game, etc.)

typedef enum{GAME_START, GAME_STOP, GAME_EXIT, GAME_LOST, GAME_WON, GAME_TOGGLE_MOUSE} GameEventType;

class GameEvent : public Event{
private:
public:
	const GameEventType purpose;
	GameEvent(GameEventType eventPurpose, void* args = nullptr) 
		: Event(EVENT_GAME, args), purpose(eventPurpose) {};
	virtual ~GameEvent() {};
};

typedef enum{ G_NONE, G_MINESWEEP, G_SNEKE, G_SHUT, G_CREDS} GameLabel;

class GameChangeEvent : public GameEvent {
public:
	const GameLabel label;
	GameChangeEvent(GameLabel gameToStart, void* args = nullptr)
		: GameEvent(GAME_START, args), label(gameToStart){}
};