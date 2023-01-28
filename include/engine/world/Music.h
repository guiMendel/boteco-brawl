#ifndef __MUSIC__
#define __MUSIC__

#include <SDL_mixer.h>
#include <string>
#include "Helper.h"

class Music
{
public:
  Music() {}

  // Construct from music filename
  Music(const std::string filename) : musicPath(filename) {}

  // Plays the given music (-1 loops forever)
  void Play(const int times = -1);

  void Play(std::string filename, const int times = -1)
  {
    musicPath = filename;
    Play(times);
  }

  // Fades out the currently playing music. Thw fade out window is in ms
  void FadeOut([[maybe_unused]] const int fadeWindow = 1500) { Mix_HaltMusic(); }

  // Declare custom destructor
  ~Music();

private:
  // The currently loaded music track
  std::string musicPath{""};

  // Music resource
  std::shared_ptr<Mix_Music> music;
};

#endif