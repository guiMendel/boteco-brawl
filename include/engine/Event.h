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
  void AddOneShotListener(const std::string &id, functionType callback) { listeners[id] = callback; }

  void RemoveListener(const std::string &id) { listeners.erase(id); }

  void Invoke()
  {
    for (auto const &entry : listeners)
      entry.second();
    for (auto const &entry : oneShotListeners)
      entry.second();
    oneShotListeners.clear();
  }

  int Count() { return listeners.size(); }

private:
  // All listener callbacks subscribed to this event
  std::unordered_map<std::string, functionType> listeners;
  std::unordered_map<std::string, functionType> oneShotListeners;
};

template <typename T>
class EventI
{
  typedef std::function<void(T)> functionType;

public:
  void AddListener(const std::string &id, functionType callback) { listeners[id] = callback; }

  void RemoveListener(const std::string &id) { listeners.erase(id); }

  void Invoke(T value)
  {
    for (auto const &entry : listeners)
      entry.second(value);
  }

  int Count() { return listeners.size(); }

private:
  // All listener callbacks subscribed to this event
  std::unordered_map<std::string, functionType> listeners;
};

template <typename T1, typename T2>
class EventII
{
  typedef std::function<void(T1, T2)> functionType;

public:
  void AddListener(const std::string &id, functionType callback) { listeners[id] = callback; }

  void RemoveListener(const std::string &id) { listeners.erase(id); }

  void Invoke(T1 value1, T2 value2)
  {
    for (auto const &entry : listeners)
      entry.second(value1, value2);
  }

  int Count() { return listeners.size(); }

private:
  // All listener callbacks subscribed to this event
  std::unordered_map<std::string, functionType> listeners;
};

#endif