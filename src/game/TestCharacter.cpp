#include "TestCharacter.h"

TestCharacter::TestCharacter(GameObject &associatedObject) : Character(associatedObject) {}

static const TestCharacter::transformerMap sequenceIndexTransformer{
  {"neutral", SequenceIndexTransformer::Repeat(2)}
};

const TestCharacter::transformerMap &TestCharacter::GetSequenceIndexTransformer() const
{
  return sequenceIndexTransformer;
}
