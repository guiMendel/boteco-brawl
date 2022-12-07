#ifndef __CHARACTER__
#define __CHARACTER__

#include "GameObject.h"
#include "Component.h"

class Character : public Component
{
public:
  Character(GameObject &associatedObject);
  virtual ~Character() {}

  
};

#endif