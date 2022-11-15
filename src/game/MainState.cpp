#include "MainState.h"
#include "GameData.h"
#include "EndState.h"
#include "Recipes.h"
#include "Camera.h"
#include "Tilemap.h"

using namespace std;

const int MainState::totalAliens{5};
const float MainState::dieAdvanceTime{2};
const float MainState::edgeSlack{80};


void MainState::AdvanceState(bool victory)
{
  if (victory)
    GameData::GetInstance().playerWon = true;

  Game::GetInstance().PushState(make_unique<EndState>());
  popRequested = true;
}

Vector2 GetPositionDistantFrom(const TileMap &tilemap, Vector2 target, float minDistance)
{
  while (true)
  {
    // Get a position
    Vector2 position = Vector2(
        RandomRange(-tilemap.GetWidth() / 2, tilemap.GetWidth() / 2),
        RandomRange(-tilemap.GetHeight() / 2, tilemap.GetHeight() / 2));

    // Check if it's far enough
    if (Vector2::SqrDistance(position, target) >= minDistance * minDistance)
      return position;
  }
}

void MainState::InitializeObjects()
{
  // Add a background
  CreateObject("Background", Recipes::Background("./assets/image/ocean.jpg"));

  // Add a tilemap
  auto tilemap = CreateObject("Tilemap", Recipes::Tilemap)->GetComponent<TileMap>();
  tilemapWeak = tilemap;

  // Function to count alien death
  auto CountAlienDeath = [this]()
  {
    //  If all aliens are dead, win
    if (--alienCount == 0)
      AdvanceState(true);
  };

  // Add penguins
  auto penguin = CreateObject("Penguin Body", Recipes::PenguinBody);
  penguinWeak = penguin;

  // Add cannon as child
  CreateObject("Penguin Cannon", Recipes::PenguinCannon, penguin->GetPosition(), penguin->GetRotation(), penguin);

  // Add aliens
  for (int i = 0; i < totalAliens; i++)
  {
    // Get coordinates for it
    Vector2 alienPosition = GetPositionDistantFrom(*tilemap, penguin->GetPosition(), 800);

    auto alien = CreateObject("Alien", Recipes::Alien, alienPosition);

    // When it dies, decrement counter
    alien->GetComponent<Health>()->OnDeath.AddListener("stateCounterDecrement", CountAlienDeath);
  }

  // Make camera follow penguin
  Camera::GetInstance()
      .Follow(penguin);

  // Play music
  music.Play("./assets/music/main.mp3");
}

void MainState::Update(float deltaTime)
{
  // Kill player it he exceeds the map's boundaries
  {
    LOCK(tilemapWeak, tilemap);
    // If die timer is up, advance
    if (timer.Get("advanceState") > dieAdvanceTime)
      AdvanceState(false);

    if (penguinWeak.expired() == false)
    {
      LOCK(penguinWeak, penguin);
      if (
          abs(penguin->GetPosition().x) > tilemap->GetWidth() / 2 + edgeSlack ||
          abs(penguin->GetPosition().y) > tilemap->GetHeight() / 2 + edgeSlack)
      {
        // Starts advance timer
        penguin->RequestDestroy();
      }
    }
  }

  // Call base
  GameState::Update(deltaTime);

  // Pop this state on esc key
  if (inputManager.KeyRelease(ESCAPE_KEY))
  {
    popRequested = true;
  }
}