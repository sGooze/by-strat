// mine_audio.hpp

#pragma once

//#include "common.hpp"
#include "c_resource.hpp"

class SoundByte {
private:
	Mix_Chunk *sound = NULL;
public:
	SoundByte() {}
	SoundByte(const std::string& path) { 
		sound = Mix_LoadWAV(path.c_str()); 
		if (sound == NULL) std::cout << path << ": sound loading failed: " << Mix_GetError() << std::endl; 
		std::cout << "Loaded sound: " << path << " = " << sound << std::endl;
	};
	// If channel == -1, first free channel is selected. If loops == -1, the sound is looped inifinitely
	void Play(int channel = -1, int loops = 0) { if (sound != NULL) Mix_PlayChannel(channel, sound, loops); };
	void Free() { std::cout << "Warning: obsolete method called: " << "SoundByte::Free" << std::endl; }
	int Volume(int vol_set = -1) { return Mix_VolumeChunk(sound, vol_set); }
	~SoundByte() { 
		std::cout << "Deleting sound: " << sound << std::endl;
		if (sound != NULL) Mix_FreeChunk(sound); 
		sound = NULL; 
	}
};



class MusicTrack {
private:
	Mix_Music *music = NULL;
public:
	MusicTrack() {}
	MusicTrack(const std::string& path) {
		music = Mix_LoadMUS(path.c_str());
		if (music == NULL) std::cout << path << ": music loading failed: " << Mix_GetError() << std::endl;
		std::cout << music << ": loading done\n";
	}
	void Play(int fadein = 0, int loops = -1, double pos = 0) { if (music != NULL) Mix_FadeInMusicPos(music, loops, fadein, pos); }
	void Free() { 
		std::cout << "Warning: obsolete method called: " << "MusicTrack::Free" << std::endl;
	}
	~MusicTrack() {
		std::cout << "Deleting musick: " << music << std::endl;
		if (music != NULL)
			Mix_FreeMusic(music);
		music = NULL;
	}
};

// TODO: for managed resources 

/*
== Audio events
*/

typedef enum{SOUND_PLAY, SOUND_LOAD, MUSIC_PLAY, MUSIC_LOAD, MUSIC_PAUSE, MUSIC_STOP} AudioEventType;

class AudioEvent : public Event {
public:
	const AudioEventType purpose;
	AudioEvent(AudioEventType eventPurpose, void* args = nullptr)
		: Event(EVENT_AUDIO, args), purpose(eventPurpose) {};
	virtual ~AudioEvent() {}
};

class AudioEvent_PlaySound : public AudioEvent {
public:
	const std::string name;
	const int loops, channel;
	AudioEvent_PlaySound(const std::string& sndName, int loops_amnt = 0, int channel_num = -1) :
		name(sndName), loops(loops_amnt), channel(channel_num), AudioEvent(SOUND_PLAY) {}
};

class AudioEvent_PrecacheSound : public AudioEvent {
public:
	const std::string name;
	AudioEvent_PrecacheSound(const std::string& sndName) : name(sndName), AudioEvent(SOUND_LOAD) {}
};

//== music

class AudioEvent_PlayMusic : public AudioEvent {
public:
	const std::string name;
	int fadeout, fadein, loop;
	double pos;
	AudioEvent_PlayMusic(const std::string& songName, int fade_out = 0, int fade_in = 0, int loops = -1, double position = 0.0)
		: name(songName), fadeout(fade_out), fadein(fadein), loop(loops), pos(position), AudioEvent(MUSIC_PLAY) {}
};

class AudioEvent_StopMusic : public AudioEvent {
public:
	int fadeout;
	AudioEvent_StopMusic(int fade_out) : fadeout(fade_out), AudioEvent(MUSIC_STOP) {}
};

class AudioEvent_PrecacheMusic : public AudioEvent {
public:
	const std::string name;
	AudioEvent_PrecacheMusic(const std::string& songName) : name(songName), AudioEvent(MUSIC_LOAD) {}
};

/*
== Audio thinkers
*/

#include "mine_thinker.hpp"

class ThinkerPlaySound : public Thinker {
private:
	SoundByte& snd;
	int ch, loop;
	bool ThinkInt() { snd.Play(ch, loop); return false; };
public:
	ThinkerPlaySound(SoundByte& sound, int loops = 0, int32_t delay = 0, int channel = -1) :
		snd(sound), ch(channel), loop(loops), Thinker(1, delay) {}
};

class ThinkerPlayMusic : public Thinker {
private:
	MusicTrack& name;
	int fadein, loops; 
	double pos;
	bool ThinkInt() { name.Play(fadein, loops, pos); std::cout << "Thinker: Track changed\n"; return false; }
public:
	ThinkerPlayMusic(MusicTrack& track, int delay = 0, int fade_in = 0, int loop_count = -1, double position = 0):
		name(track),  fadein(fade_in), loops(loop_count), pos(position), Thinker(1, delay){}
};

/*
== Audio manager
*/
// TODO: derive from generic resource manager class

// TODO: Positional and volume settings for individual sounds
class SoundManager : public Manager<SoundByte> {
private:
	bool auto_precache = false;
public:
	SoundManager() : Manager("sounds\\") {};
	void Play(const std::string& name, int loops, int chan) {
		auto x = loaded.find(name);
		if (x == loaded.end()) {
			std::cout << name << ": attempting to play unprecached sound\n";
			if (auto_precache)
				Load(name);
			return;
		}
		(*x).second.Play(chan, loops);
	}
	int ChVolume(int channel = -1, int vol_new = -1) { return Mix_Volume(channel, vol_new); }
	void Halt(int channel = -1, int delay = -1) { Mix_ExpireChannel(channel, delay); }
	~SoundManager() {}
};

class MusicManager : public Manager<MusicTrack> {
private:
	ThinkerPlayMusic* delayedMusic = nullptr;
public:
	MusicManager() : Manager("sounds\\music\\") {}
	int Volume(int vol = -1) { return Mix_VolumeMusic(vol); }
	void Pause() { Mix_PauseMusic(); }
	void Resume() { Mix_ResumeMusic(); }
	bool IsPlaying() { return (Mix_PlayingMusic()); }
	bool IsPaused() { return (Mix_PlayingMusic()) ? Mix_PausedMusic() : false; }
	void Halt() { Mix_HaltMusic(); }
	void FadeOut(int length) { Mix_FadeOutMusic(length); }
	void Think();

	void Play(const std::string& name, int fadeout = 0, int fadein = 0, int loops = -1, double pos = 0);
	~MusicManager() { 
		Halt(); 
		for (auto& x : loaded) {
			std::cout << x.first << std::endl;
			//x.second.Free();
		}
	}
};