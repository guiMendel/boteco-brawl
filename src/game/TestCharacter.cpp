#include "TestCharacter.h"

static const float baseDamage{1};

TestCharacter::TestCharacter(WorldObject &associatedObject)
    : Character(associatedObject) {}

static const TestCharacter::transformerMap sequenceIndexTransformer{
    {"neutral", SequenceIndexTransformer::Repeat(2)}};

const TestCharacter::transformerMap &TestCharacter::GetSequenceIndexTransformer() const
{
  return sequenceIndexTransformer;
}

float TestCharacter::GetBaseDamage() const { return baseDamage; }
