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
  if (currentAnimation == nullptr)
    return;

  currentAnimation->Update(deltaTime);
}

void Animator::Stop()
{
  if (currentAnimation == nullptr)
    return;

  // Stop it
  currentAnimation->Stop();

  // Forget it
  currentAnimation = nullptr;

  // Transition to default
  Play(defaultAnimation);
}

void Animator::Stop(string animation)
{
  if (currentAnimation != nullptr && currentAnimation->Name() == animation)
    Stop();
}

shared_ptr<Animation> Animator::BuildAnimation(string name)
{
  // Validate name
  Assert(HasAnimation(name), "Animator didn't have name \"" + name + "\" registered to build");

  // Call it's builder
  return animations[name]();
}

void Animator::Play(string animationName, bool forceReset)
{
  // Skip if already playing this
  if (currentAnimation != nullptr && currentAnimation->Name() == animationName)
    return;

  // Build it and play it
  Play(BuildAnimation(animationName), forceReset);
}

void Animator::Play(shared_ptr<Animation> animation, bool forceReset)
{
  // Skip if already playing this
  if (forceReset == false && currentAnimation != nullptr && currentAnimation->Name() == animation->Name())
    return;

  // Ensure it's type has been previously registered
  Assert(HasAnimation(animation->Name()), "Animator is forbidden to play an animation whose type hasn't been registered yet. Animation name: " + animation->Name());

  // Stop current animation
  Stop();

  // Remember it
  currentAnimation = animation;

  // Start it
  animation->Start();
}

bool Animator::HasAnimation(string name) const
{
  return animations.count(name) > 0;
}

shared_ptr<Animation> Animator::GetCurrentAnimation() const { return currentAnimation; }
