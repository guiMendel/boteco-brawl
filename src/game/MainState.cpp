#include "MainState.h"
#include "GameData.h"
#include "Recipes.h"
#include "Camera.h"

using namespace std;

void MainState::InitializeObjects()
{
  // Add a background
  CreateObject("Background", Recipes::Background("./assets/image/boteco.jpg"));

  // Add player
  auto player1 = CreateObject("Player", Recipes::Character(), Camera::GetMain()->GetPosition() + Vector2(-2, 0))->GetComponent<Rigidbody>();

  // auto player2 = CreateObject("Player2", Recipes::Character(), Camera::GetMain()->GetPosition() + Vector2(2, 0))->GetComponent<Rigidbody>();

  CreateObject("Ground", Recipes::Platform({5, 1}), Camera::GetMain()->GetPosition() + Vector2(0, 4));

  // auto thing = CreateObject("Thing", Recipes::Platform({1, 1}, false), Camera::GetMain()->GetPosition() + Vector2(2, 0));

  // auto thingBody = thing->GetComponent<Rigidbody>();
  // thingBody->velocity += Vector2({-1, 0});

  player1->velocity = Vector2(3, 0.3);
  // player2->velocity = Vector2(-1, 0.2);

  // Play music
  music.Play("./assets/music/main.mp3");
}
