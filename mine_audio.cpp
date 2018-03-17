// mine_audio.cpp

#include "mine_game.hpp"

void MusicManager::Think() {
	if ((delayedMusic == nullptr) || (delayedMusic->Think()))
		return;
	delete delayedMusic; delayedMusic = nullptr;
}

void MusicManager::Play(const std::string& name, int fadeout, int fadein, int loops, double pos) {
	auto x = loaded.find(name);
	if (x == loaded.end()) {
		std::cout << name << ": attempting to play unprecached music track\n";
		return;
	}
	if (IsPlaying()) {
		if (fadeout == 0) {
			Halt(); if (delayedMusic != nullptr) { delete delayedMusic; delayedMusic = nullptr; }
		}
		else {
			if (delayedMusic != nullptr) {
				delete delayedMusic;
			}
			delayedMusic = new ThinkerPlayMusic((*x).second, fadeout, fadein, loops, pos);
			FadeOut(fadeout);
			return;
		}
	}
	std::cout << "Track change, no fadeout\n";
	(*x).second.Play(fadein, loops, pos);
}

void MineGame::PollAudio() {
	//TODO: Audio on/off
	AudioEvent* aev;
	while (EventQueue::PollEvent(EVENT_AUDIO, (Event**)&aev)) {
		switch (aev->purpose){
		case SOUND_LOAD:
			gSounds.Load(((AudioEvent_PrecacheSound*)aev)->name);
			break;
		case SOUND_PLAY:
			gSounds.Play(((AudioEvent_PlaySound*)aev)->name, ((AudioEvent_PlaySound*)aev)->loops, ((AudioEvent_PlaySound*)aev)->channel);
			break;
		case MUSIC_LOAD:
			gMusic.Load(((AudioEvent_PrecacheMusic*)aev)->name);
			break;
		case MUSIC_PLAY:
			gMusic.Play(((AudioEvent_PlayMusic*)aev)->name, ((AudioEvent_PlayMusic*)aev)->fadeout, ((AudioEvent_PlayMusic*)aev)->fadein, ((AudioEvent_PlayMusic*)aev)->loop, ((AudioEvent_PlayMusic*)aev)->pos);
			break;
		case MUSIC_STOP:
			gMusic.FadeOut(((AudioEvent_StopMusic*)aev)->fadeout);
			break;
		default:
			break;
		}
		delete aev;
	}
}