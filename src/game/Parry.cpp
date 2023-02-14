#include "Parry.h"

Parry::Parry(GameObject &associatedObject) : WorldComponent(associatedObject) {}

bool Parry::CanParry(Damage) { return ready; }

void Parry::SetParry(bool value) { ready = value; }

DamageParameters Parry::Riposte(Damage damage)
{
  // Go to render front
  worldObject.RequireComponent<SpriteRenderer>()->SetRenderOrder(Game::currentFrame);

  return GetRiposteDamage(damage);
}
