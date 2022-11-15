#ifndef __EVENT__
#define __EVENT__

#include <unordered_map>
#include <functional>
#include <string>

class Event
{
  typedef std::function<void(void)> functionType;

public:
  void AddListener(const std::string &id, functionType callback) { listeners[id] = callback; }

  void RemoveListener(const std::string &id) { listeners.erase(id); }

  void Invoke()
  {
    for (auto const& entry : listeners)
      entry.second();
  }

  int Count() { return listeners.size(); }

private:
  // All listener callbacks subscribed to this event
  std::unordered_map<std::string, functionType> listeners;
};

#endif