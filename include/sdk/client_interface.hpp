#pragma once

#include <cstdint>
#include <cstddef>
#include <type_traits>

#include "types.hpp"

#include "sdk_interface.hpp"
#include "module_interface.hpp"
#include "plugin_interface.hpp"

namespace sdk {

struct ver_info {
  int major;
  int minor;
};

inline const ver_info verion = {
  .major = 1, // Updated when breaking changes are introduced (VF index changes, parameter type changes, API changes, etc...)
  .minor = 0, // Updated when non breaking changes are made (Addition of API, backend changes)
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

  event_action     action;
  managed_string * message; // Message sent
};

/*
 *  Event Listener: Chat log
 *
 *  Triggered when a new entry to the chat log
 *  is added
 */
struct event_chat_log {
  static constexpr const char EVENT_ID[]  = "evn_chat_log";
  using fn_t = void(*)(event_chat_log * msg);

  event_action     action;
  const char     * message;      // The message that was sent to chat
  const char     * sender_name;  // The name of the sender
  const char     * context;      // Context of the message
  managed_string * display_text; // A pointer to a text that is to be displayed in the chat logs
};

/*
 *  Event Listener: Plugin Loaded
 *
 *  Triggered when a plugin is created / loaded
 *  successfuly
 */
struct event_plugin_load {
  static constexpr const char EVENT_ID[]  = "evn_plug_loaded";
  using fn_t = void(*)(event_plugin_load * msg);

  sdk::plugin_intf * instance;
};

/*
 *  Event Listener: Plugin Unload
 *
 *  Triggered when a plugin is unloading
 *  successfuly
 */
struct event_plugin_unload {
  static constexpr const char EVENT_ID[]  = "evn_plug_unload";
  using fn_t = void(*)(event_plugin_unload * msg);

  sdk::plugin_intf * instance;
};

/*
 *  Event Listener: Dynamic Module Loaded
 *
 *  Triggered when a module is registered
 *  successfuly
 */
struct event_module_load {
  static constexpr const char EVENT_ID[]  = "evn_mod_loaded";
  using fn_t = void(*)(event_module_load * msg);

  sdk::plugin_intf * instance;
};

/*
 *  Event Listener: Dynamic Module Unload
 *
 *  Triggered when a module is being unregistered
 *  successfuly
 */
struct event_module_unload {
  static constexpr const char EVENT_ID[]  = "evn_mod_unload";
  using fn_t = void(*)(event_module_unload * msg);

  sdk::plugin_intf * instance;
};

// -- End of EVENTS
// ---------------------------------------------------------------------------------------------------- 

/*
 *  Interface to the Client's API
 *  Allows you to interact with the internal client.
 */
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
  virtual auto register_plugin(plugin_intf * instance) -> bool = 0;

  /*
   *  Unregisters a plugin instance in the client
   *
   *  Importance: This notifies other plugins and modules so they would
   *  know to not use the instance pointer anymore
   */
  virtual auto unregister_plugin(plugin_intf * instance) -> bool = 0;

  /*
   *  Register a module into the client
   *
   *  NOTE: You will be managing the life time of `instance`. Before
   *  deletining the instance make sure to unregister it first using
   *  the `unregister_module`
   */
  virtual auto register_module(plugin_intf * parent, module_intf * instance) -> bool = 0;

  /*
   *  Unregisters a previously registered module
   *
   *  Importance: This notifies other plugins and modules so they would
   *  know to not use the instance pointer anymore
   */
  virtual auto unregister_module(module_intf * instance) -> bool = 0;

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
   *  NOTE: Only logs text, not send! This is client sided.
   */
  virtual auto queue_log_chat(const char * text) -> bool = 0;

  /*
   *  Obtain the C string of a `managed_string`
   */
  virtual auto get_mcstr(managed_string * ms) -> const char * = 0;

  /*
   *  Set the value of a `managed_string` with a C string
   */
  virtual auto set_mcstr(managed_string * ms, const char * str) -> managed_string * = 0;

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

  // -- End of Helpers
  // -------------------------------------------------------------------------------------------
};

struct load_info {
  ver_info client_sdk_version;
  client_intf * instance;
};

} // sdk
