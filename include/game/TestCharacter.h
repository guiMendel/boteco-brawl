#ifndef __TEST_CHARACTER__
#define __TEST_CHARACTER__

#include "Character.h"

class TestCharacter : public Character
{
public:
  TestCharacter(GameObject &associatedObject);
  virtual ~TestCharacter() {}

protected:
  const transformerMap &GetSequenceIndexTransformer() const override;
};

#endif