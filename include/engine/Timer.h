#ifndef __TIMER__
#define __TIMER__

#include <unordered_map>
#include <string>

class Timer
{
  struct Entry
  {
    float value{0.0f};
    bool enabled{false};
  };

  std::unordered_map<std::string, Entry> timers;

public:
  void
  Reset(std::string name, float value = 0, bool enable = true)
  {
    timers[name].value = value;
    timers[name].enabled = enable;
  }
  float Get(std::string name) { return timers[name].value; }
  void Start(std::string name) { timers[name].enabled = true; }
  void Stop(std::string name) { timers[name].enabled = false; }
  void Update(float deltaTime)
  {
    for (auto &entry : timers)
      if (entry.second.enabled)
        entry.second.value += deltaTime;
  }
};

#endif