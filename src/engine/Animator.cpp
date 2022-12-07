#include "Animator.h"
#include "Animation.h"

using namespace std;

Animator::Animator(GameObject &associatedObject) : Component(associatedObject) {}

void Animator::Start()
{
  // Play initial animation
  if (initialAnimation != "")
    Play(initialAnimation);
}

void Animator::Update(float deltaTime)
{
  // Update the animation being played
  if (currentAnimation == "")
    return;

  animations[currentAnimation]->Update(deltaTime);
}

void Animator::AddAnimation(shared_ptr<Animation> animation, bool makeInitial)
{
  // Check for unique name
  Assert(animations.find(animation->name) == animations.end(), "Tried to add two animations with the same name");

  animations[animation->name] = animation;

  if (makeInitial || initialAnimation == "")
    initialAnimation = animation->name;
}

void Animator::AddAnimation(function<shared_ptr<Animation>(Animator &)> recipe, bool makeInitial)
{
  AddAnimation(recipe(*this), makeInitial);
}

void Animator::Stop()
{
  if (currentAnimation == "")
    return;

  // Stop it
  animations[currentAnimation]->Stop();

  // Forget it
  currentAnimation = "";
}

void Animator::Play(string animation)
{
  // Validate name
  Assert(animations.find(animation) != animations.end(), "Animator couldn't find animation with name \"" + animation + "\"");

  // Remember it
  currentAnimation = animation;

  // Start it
  animations[animation]->Start();
}

Animation &Animator::GetAnimation(string name)
{
  return *animations[name];
}
