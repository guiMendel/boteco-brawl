#include "Music.h"
#include "Helper.h"
#include "Resources.h"
#include <iostream>

using namespace Helper;
using namespace std;

void Music::Play(const int times)
{
  Assert(musicPath.size() > 0, "Tried playing music without providing it's file path");

  // Get music
  music = Resources::GetMusic(musicPath);

  auto encounteredError = Mix_PlayMusic(music.get(), times);

  // Catch weird errors
  Assert(!encounteredError, "Failed to play a music track");
}

Music::~Music()
{
  // Stops any music being played
  FadeOut();
}