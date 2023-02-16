#ifndef __SOUND__
#define __SOUND__

#include <SDL_mixer.h>
#include <iostream>
#include <functional>
#include "WorldObject.h"
#include "WorldComponent.h"
#include "Helper.h"

class Sound : public WorldComponent
{
public:
  // Constructor with sound file name
  Sound(GameObject &associatedObject, std::unordered_map<std::string, std::string> sounds = {});

  // Ensure sound stops if destroyed
  virtual ~Sound() { Stop(); }

  // Adds an audio
  void AddAudio(std::string sound, std::string path);

  // Plays audio
  void Play(std::string sound, int times = 1);

  // Stops a single sound
  void Stop(std::string sound);

  // Stops playing all sounds
  void Stop();

private:
  std::unordered_map<std::string, std::string> sounds;
  std::unordered_map<std::string, std::shared_ptr<Mix_Chunk>> chunks;
  std::unordered_map<std::string, int> channels;
};

#endif