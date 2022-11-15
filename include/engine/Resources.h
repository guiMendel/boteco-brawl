#ifndef __RESOURCES__
#define __RESOURCES__

#include <functional>
#include <unordered_map>
#include <memory>
#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include "Helper.h"

class Resources
{
public:
  // Type of resource table
  template <class T>
  using table = std::unordered_map<std::string, std::shared_ptr<T>>;

  // Get an image texture
  static std::shared_ptr<SDL_Texture> GetTexture(std::string filename);

  // Get a music
  static std::shared_ptr<Mix_Music> GetMusic(std::string filename);

  // Get an sfx
  static std::shared_ptr<Mix_Chunk> GetSound(std::string filename);

  // Get a font
  static std::shared_ptr<TTF_Font> GetFont(std::string filename, int size);

  // Clear everything
  static void ClearAll()
  {
    ClearTable(musicTable);
    ClearTable(textureTable);
    ClearTable(soundTable);
  }

private:
  // Get a resource
  template <class Resource>
  static std::shared_ptr<Resource> GetResource(
      std::string resourceType,
      std::string resourceKey,
      table<Resource> &table,
      std::function<Resource *(std::string)> resourceLoader,
      void (*resourceDestructor)(Resource *))
  {
    // Check if it's already loaded
    auto resourceIterator = table.find(resourceKey);

    // If so, return the loaded asset
    if (resourceIterator != table.end())
      return resourceIterator->second;

    // At this point, we know the asset isn't loaded yet

    // Load it
    Resource *resourcePointer = resourceLoader(resourceKey);

    // Catch any errors
    Assert(resourcePointer != nullptr, "Failed to load " + resourceType + " at " + resourceKey);

    // Store the texture (create the pointer with the destructor)
    table.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(resourceKey),
        std::forward_as_tuple(resourcePointer, resourceDestructor));

    // Now that it's loaded, return it
    return table[resourceKey];
  }

  template <class Resource>
  static void ClearTable(table<Resource> &table)
  {
    // Iterate through it
    auto entryIterator = table.begin();
    while (entryIterator != table.end())
    {
      // Check if the table's is the only reference left
      if (entryIterator->second.unique())
      {
        // If so, delete it
        entryIterator = table.erase(entryIterator);
      }

      // Otherwise, keep it around
      else
        entryIterator++;
    }
  }

  // Store textures
  static table<SDL_Texture> textureTable;

  // Store music
  static table<Mix_Music> musicTable;

  // Store sfx
  static table<Mix_Chunk> soundTable;

  // Store fonts
  static table<TTF_Font> fontTable;
};

#endif