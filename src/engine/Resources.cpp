#include "Game.h"
#include "Resources.h"
#include "Helper.h"
#include <utility>
#include <tuple>
#include <memory>

using namespace std;
using namespace Helper;

Resources::table<SDL_Texture> Resources::textureTable;

Resources::table<Mix_Music> Resources::musicTable;

Resources::table<Mix_Chunk> Resources::soundTable;

Resources::table<TTF_Font> Resources::fontTable;

shared_ptr<SDL_Texture> Resources::GetTexture(string filename)
{
  function<SDL_Texture *(string)> textureLoader = [](string filename)
  {
    // Get the game renderer
    SDL_Renderer *renderer = Game::GetInstance().GetRenderer();

    // Gets the texture pointer
    return IMG_LoadTexture(renderer, filename.c_str());
  };

  return GetResource<SDL_Texture>("texture", filename, textureTable, textureLoader, SDL_DestroyTexture);
}

shared_ptr<Mix_Music> Resources::GetMusic(string filename)
{
  function<Mix_Music *(string)> musicLoader = [](string filename)
  {
    return Mix_LoadMUS(filename.c_str());
  };

  return GetResource<Mix_Music>("music", filename, musicTable, musicLoader, Mix_FreeMusic);
}

shared_ptr<Mix_Chunk> Resources::GetSound(string filename)
{
  function<Mix_Chunk *(string)> chunkLoader = [](string filename)
  {
    return Mix_LoadWAV(filename.c_str());
  };

  return GetResource<Mix_Chunk>("sound chunk", filename, soundTable, chunkLoader, Mix_FreeChunk);
}

shared_ptr<TTF_Font> Resources::GetFont(string filename, int size)
{
  function<TTF_Font *(string)> fontLoader = [](string fontKey)
  {
    // Get the filename and the size
    size_t delimiter = fontKey.find_first_of("$");

    // Get the size from the key
    int size = stoi(string(fontKey, 0, delimiter));

    // Get the file name
    string filename = string(fontKey, delimiter + 1);

    return TTF_OpenFont(filename.c_str(), size);
  };

  // Build the table key
  string fontKey = to_string(size) + "$" + filename;

  return GetResource<TTF_Font>("font", fontKey, fontTable, fontLoader, TTF_CloseFont);
}
