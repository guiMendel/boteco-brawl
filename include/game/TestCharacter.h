#ifndef __TEST_CHARACTER__
#define __TEST_CHARACTER__

#include "Character.h"

class TestCharacter : public Character
{
public:
  TestCharacter(GameObject &associatedObject);
  virtual ~TestCharacter() {}

  float GetBaseDamage() const override;

protected:
  const transformerMap &GetSequenceIndexTransformer() const override;
};

#endif