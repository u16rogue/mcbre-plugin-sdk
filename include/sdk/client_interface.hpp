#pragma once

#include <cstdint>
#include <cstddef>
#include <type_traits>

#include "sdk_interface.hpp"
#include "module_interface.hpp"
#include "plugin_interface.hpp"

namespace sdk {

struct ver_info {
  int major;
  int minor;
};

inline const ver_info verion = {
  .major = 1,
  .minor = 0
};

// ---------------------------------------------------------------------------------------------------- 
// -- EVENTS

enum class event_action : std::uint32_t {
  NOTHING = 0, // Event is continued to be dispatched to other listeners until it reaches the game
  CANCEL  = 1, // Prevents the event from reaching the game and other event listeners
  COMMIT  = 2, // Immediately lets the event to reach the game skipping the rest of the listeners upon the listener's return
};

/*
 *  Event Listener: Chat send
 *
 *  Triggered when the player sends a message into the chat including
 *  normal text and commands
 */
struct event_chat_send {
  static constexpr const char EVENT_ID[] = "evn_chat_send";
  using fn_t = void(*)(event_chat_send * msg);

  event_action action;

  /*
   *  A pointer to the message sent by the player
   *
   *  The value of `message` can be overriden by providing a valid pointer to pointer to a null terminated string. After
   *  the event dispatcher handles the override value the pointer value provided to `message` is set to null.
   *  WARNING: If overriding, the lifetime for the new value of `message` MUST perist after the return of the event handler
   *           which will then be processed by the event dispatcher.
   *  EXAMPLE:
   *  static const char * newmsg = "hello";
   *  ...
   *  +[](eldt_chat_send * m) {
   *    newmsg     = "goodbye";
   *    m->message = &newmsg;
   *    m->action  = event_action::COMMIT;
   *    return;
   *  }
   *  ...
   *  // Sends a message to the ingame chat "goodbye"
   *  // newmsg == nullptr
   */
  const char ** message;
};
using elfn_chat_send = typename event_chat_send::fn_t;

struct event_chat_log {
  static constexpr const char EVENT_ID[]  = "evn_chat_log";
  using fn_t = void(*)(event_chat_log * msg);

  event_action action;

  const char *  message;      // The message that was sent to chat
  const char *  sender_name;  // The name of the sender
  const char *  context;      // Context of the message

  /*
   *  A pointer to a text that is to be displayed in the chat logs
   *
   *  The value of `display_text` can be overriden by providing a valid pointer to pointer to a null terminated string. After
   *  the event dispatcher handles the override value the pointer value provided to `display_text` is set to null.
   *  WARNING: If overriding, the lifetime for the new value of `display_text` MUST perist after the return of the event handler
   *           which will then be processed by the event dispatcher.
   *  EXAMPLE:
   *  static const char * newmsg = "hello";
   *  ...
   *  +[](eldt_chat_log * m) {
   *    newmsg     = "goodbye";
   *    m->display_text = &newmsg;
   *    m->action  = event_action::COMMIT;
   *    return;
   *  }
   *  ...
   *  // Displays the text "goodbye" instead of the original message received into the chat log
   *  // newmsg == nullptr
   */
  const char ** display_text;
};
using elfn_chat_log = typename event_chat_log::fn_t;

// -- End of EVENTS
// ---------------------------------------------------------------------------------------------------- 

class client_intf : public sdk::sdk_intf {
public:
  virtual ~client_intf() = 0;

  /*
   *  Allows you to register your plugin
   *
   *  NOTE: You will be managing the life time of `instance`. Before
   *  deletining the instance make sure to unregister it first using
   *  the `unregister_plugin`
   */
  virtual auto register_plugin(plugin_intf * instance, const char * name) -> bool = 0;

  /*
   *  Unregisters a plugin instance in the client
   *
   *  Importance: This notifies other plugins and modules so they would
   *  know to not use the instance pointer anymore
   */
  virtual auto unregister_plugin(plugin_intf * instance) -> bool = 0;

  /*
   *  Enumerate the loaded plugins on the client.
   *
   *  To get the number of plugins loaded you can pass a nullptr
   *  to the `out` parameter. You can use this to determine what the
   *  size of your `out` array should be.
   */
  virtual auto enumerate_plugins(plugin_intf * out, std::size_t * count) -> bool = 0;

  /*
   *  Enumerate the loaded modules on the client.
   *
   *  To get the number of modules loaded you can pass a nullptr
   *  to the `out` parameter. You can use this to determine what the
   *  size of your `out` array should be.
   */
  virtual auto enumerate_modules(module_intf * out, std::size_t * count) -> bool = 0;

  /*
   *  Register a function listener for a specified event
   */
  virtual auto add_event_listener(const char * ename, void * fnp) -> bool = 0;

  /*
   *  Unregisters a function listener for a specified event
   */
  virtual auto remove_event_listener(void * fnp) -> bool = 0;

  /*
   *  Logs a text into the ingame chat.
   *  NOTE: Only logs text not send! This is client sided.
   */
  virtual auto queue_log_chat(const char * text) -> bool = 0;

  // -------------------------------------------------------------------------------------------
  // -- Helpers

  /*
   *  Helper function to register event listeners easier while having type checks for callbacks.
   *  EXAMPLE:
   *    client->add_event_listener(+[](event_chat_send * e) { ... });
   */
  template <typename T>
  auto add_event_listener(void(*fn)(T *)) -> bool {
    static_assert(requires { T::EVENT_ID;                      }, "Event type parameter T must provide an EVENT_ID.");
    static_assert(requires { typename T::fn_t;                 }, "Event type parameter T must provide an fn_t for a callback type definition.");
    static_assert(std::is_same_v<typename T::fn_t, decltype(fn)>, "Event listener callback did not match the expected function signature.");
    return this->add_event_listener(T::EVENT_ID, fn);
  }
};

} // sdk
