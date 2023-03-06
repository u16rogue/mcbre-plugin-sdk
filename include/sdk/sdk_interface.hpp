#pragma once

#include <cstdint>
#include <cstddef>

namespace sdk {

class sdk_intf {
public:
  /*
   *   Query is user implemented! Implementer can define what ID values it can accept
   *   and what ptr output values are given. This can be used to further extend functionality
   *   of other interfaces without creating an extended interface.
   *
   *   There is no specification for this on what constitues for a valid ptr arg and size
   *   so check the interface you're using to see how to use it.
   *
   *   Use case can be if your plugin loads a dynamic module that does chat parsing and another
   *   module would like to interact with that functionality. The chat parsing module developer
   *   can override query and respond to an ID of "register_callback" while receiving a function
   *   pointer for the ptr arg.
   */
  virtual auto query(const char * id,  void * ptr, std::uint64_t size) -> bool = 0;

  template <typename T>
  auto query(const char * id, T * ptr) -> bool {
    return this->query(id, ptr, sizeof(T));
  }
};

}
