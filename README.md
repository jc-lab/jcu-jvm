# jcu-jvm

# example

```c++
#include <jcu-jvm/jvm_library.h>
#include <jcu-jvm/vm.h>


int main() {
  std::shared_ptr<jcu::jvm::OsHandler> os_handler(jcu::jvm::OsHandler::create());
  std::shared_ptr<jcu::jvm::JvmLibrary> jvm_library(jcu::jvm::JvmLibrary::create(os_handler));

  int rc = jvm_library->load(nullptr, nullptr);
  const char* msg = jvm_library->getLoadError();

  printf("rc/msg = %d / %s\n", rc, msg);
  printf("path = %s\n", jvm_library->getJvmPath());

  auto java = jcu::jvm::VM::create(jvm_library);
  jint jrc = java->init();
  printf("java init : %d\n", jrc);

  // Your code

  jrc = java->destroy();
  printf("java destroy : %d\n", jrc);

  return 0;
}
```

# License
Apache License Version 2.0