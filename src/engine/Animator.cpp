#include "Animator.h"
#include "Animation.h"

using namespace std;

Animator::Animator(GameObject &associatedObject) : Component(associatedObject) {}

void Animator::Start()
{
  // Play initial animation
  if (defaultAnimation != "")
    Play(defaultAnimation);
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

  if (makeInitial || defaultAnimation == "")
    defaultAnimation = animation->name;
}

void Animator::AddAnimation(function<shared_ptr<Animation>(Animator &)> recipe, bool makeInitial)
{
  AddAnimation(recipe(*this), makeInitial);
}

void Animator::Stop()
{
  if (currentAnimation == "")
    return;

  auto stoppedAnimation = currentAnimation;

  // Forget it
  currentAnimation = "";

  // Stop it
  animations[stoppedAnimation]->Stop();

  // Erase all callbacks
  currentAnimationCallbacks.clear();
}

void Animator::Stop(string animation)
{
  if (currentAnimation == animation)
    Stop();
}

void Animator::Play(string animation, frame_callbacks callbacks, function<void()> stopCallback)
{
  // Skip if already playing this
  if (animation == currentAnimation)
    return;

  // Validate name
  Assert(animations.find(animation) != animations.end(), "Animator couldn't find animation with name \"" + animation + "\"");

  // Stop current animation
  Stop();

  // Remember it
  currentAnimation = animation;

  // Start it
  animations[animation]->Start();

  // Add it's stop callback
  if (stopCallback)
    animations[animation]->OnStop.AddOneShotListener("play-callback", stopCallback);

  // Add it's frame callbacks
  currentAnimationCallbacks = callbacks;
}

void Animator::Play(string animation, function<void()> stopCallback)
{
  Play(animation, {}, stopCallback);
}

Animation &Animator::GetAnimation(string name)
{
  return *animations[name];
}

string Animator::GetCurrentAnimation() const { return currentAnimation; }

void Animator::IndicateCurrentFrame(int frame)
{
  // Trigger this frames callback if it's there
  if (currentAnimationCallbacks.count(frame) > 0)
    currentAnimationCallbacks[frame]();
}