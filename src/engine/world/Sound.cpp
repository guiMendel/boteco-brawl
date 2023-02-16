#include "Sound.h"
#include "Resources.h"

using namespace Helper;
using namespace std;

Sound::Sound(GameObject &associatedObject, unordered_map<string, string> sounds)
    : WorldComponent(associatedObject), sounds(sounds) {}

void Sound::Play(string sound, int times)
{
  Assert(sounds.count(sound) > 0, "Sound " + sound + " was never registered");

  // Get chunk
  std::shared_ptr<Mix_Chunk> chunk;

  // If it hasn't been loaded before
  if (chunks.count(sound) == 0)
    chunks[sound] = Resources::GetSound(sounds.at(sound));

  chunk = chunks.at(sound);

  // Play and memorize channel
  channels[sound] = Mix_PlayChannel(-1, chunk.get(), times - 1);
}

void Sound::Stop()
{
  // Stop all channels
  for (auto [sound, channel] : channels)
    Stop(sound);
}

void Sound::Stop(string sound)
{
  // If no channel for this sound, ignore
  if (channels.count(sound) == 0)
    return;

  // Stop it
  Mix_HaltChannel(channels.at(sound));

  // Reset it
  channels.erase(sound);
}

void Sound::AddAudio(std::string sound, std::string path)
{
  sounds[sound] = path;
}
