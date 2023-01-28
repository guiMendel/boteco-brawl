#ifndef __SOUND__
#define __SOUND__

#include <SDL_mixer.h>
#include <iostream>
#include <functional>
#include "GameObject.h"
#include "Component.h"
#include "Helper.h"

class Sound : public Component
{
public:
  // Constructor with sound file name
  Sound(GameObject &associatedObject, const std::string fileName, bool playOnStart = true);

  // Ensure sound stops if destroyed
  virtual ~Sound() { Stop(); }

  // Plays audio
  void Play(const int times = 1);

  // Stops playing
  void Stop();

  void Start() override;

private:
  // The chunk path
  std::string chunkPath;

  // Chunk resource
  std::shared_ptr<Mix_Chunk> chunk;

  // Whther to play on Start callback
  bool playOnStart;

  // The channel where to play
  int channel{-1};
};

#endif