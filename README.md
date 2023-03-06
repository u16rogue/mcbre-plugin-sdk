# Minecraft: Bedrock Edition Plugin SDK

### SDK Usage
```cmake
add_subdirectory(<path to sdk>)
...
target_link_libraries(
  <your project>
  PRIVATE # up to you
  mcbre_client
)
```

### Implementation
* You can receive a client interface from the lpReserved parameter of your main entrypoint (DllMain)
```c++
#include <Windows.h>
#include <sdk/client_interface.hpp>

class myplugin : public sdk::plugin_intf {
  ...
};

myplugin       * mypluginst = nullptr;
HINSTANCE        hmod       = NULL;
sdk::client_intf client     = nullptr;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
  sdk::load_info * info = reinterpret_cast<decltype(info)>(lpvReserved);
  if (fwdReason == DLL_PROCESS_ATTACH && info && info->client_sdk_version.major == sdk::version.major) {
    if (!mypluginst)
      mypluginst = new myplugin();

    info->instance = mypluginst;
    hmod           = hinstDLL;
    client         = info->client;
  }
  
  return TRUE;
}
```
* This will only work if its loaded by the client.

### Loading your plugin
Using the designated command prefix `'.' by default` enter the command `plug` followed by the path to your plugin.
```
.plug E:\myplugin.dll
```
and to unload
```
.unplug E:\myplugin.dll
```

### Documentation
Comments are provided in the source and header files.
