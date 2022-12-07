#ifndef __ACTION__
#define __ACTION__

#include <memory>

// Defines the data of a game action, such as a standing attack or a dash
struct Action {
  // Unique identifier of this action
  int id;

  // Action that may follow this one up
  std::weak_ptr<Action> next;
};

#endif