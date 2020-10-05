#include <jcu-jvm/jvm_library.h>
#include <jcu-jvm/vm.h>

#include <thread>

int wrapped() {
  setbuf(stdout, nullptr);
  setbuf(stderr, nullptr);

  std::shared_ptr<jcu::jvm::OsHandler> os_handler(jcu::jvm::OsHandler::create());
  std::shared_ptr<jcu::jvm::JvmLibrary> jvm_library(jcu::jvm::JvmLibrary::create(os_handler));

  auto path_info = os_handler->findJvmLibrary("/usr/lib/jvm/java-8-openjdk-amd64/jre/lib/amd64/server/libjvm.so", nullptr);

  int rc = jvm_library->load(path_info, true);
  const char* msg = jvm_library->getLoadError();

  printf("rc/msg = %d / %s\n", rc, msg);
  printf("path = %s\n", jvm_library->getJvmPath());

  auto java = jcu::jvm::VM::create(jvm_library);
//  jint jrc = java->init("D:\\workspace\\zeroback\\zbdevd\\build\\libs\\zbdevd-1.0-SNAPSHOT.jar");
  jint jrc = java->init("/mnt/d/workspace/zeroback/zbdevd/build/libs/zbdevd-1.0-SNAPSHOT.jar");
  printf("java init : %d\n", jrc);

  //

  auto env = java->env();

  jboolean res_bool;

  jclass loader_clazz = env->FindClass("org/apache/commons/daemon/support/DaemonLoader");
  printf("clazz = %p\n", loader_clazz);

  jmethodID method_load = env->GetStaticMethodID(loader_clazz, "load", "(Ljava/lang/String;[Ljava/lang/String;)Z");
  jmethodID method_start = env->GetStaticMethodID(loader_clazz, "start", "()Z");
  jmethodID method_stop = env->GetStaticMethodID(loader_clazz, "stop", "()Z");
  jstring daemon_class_name = env->NewStringUTF("com.zeronsoftn.client.zbdevd.ZbdevDaemon");

  jclass class_string = env->FindClass("java/lang/String");

  jobject daemon_args_array = env->NewObjectArray(0, class_string, nullptr);

  res_bool = env->CallStaticBooleanMethod(loader_clazz, method_load, daemon_class_name, daemon_args_array);
  printf("load result = %d\n", res_bool);

  res_bool = env->CallStaticBooleanMethod(loader_clazz, method_start);
  printf("start result = %d\n", res_bool);

  std::this_thread::sleep_for(std::chrono::milliseconds { 5000 });
  printf("after sleep\n");

  res_bool = env->CallStaticBooleanMethod(loader_clazz, method_stop);
  printf("stop result = %d\n", res_bool);

//  std::thread th = std::thread([&java, loader_clazz, method_start]() -> void {
//    JNIEnv* env = nullptr;
//    bool attached = false;
//    jint rc = java->attachThreadEnv(&env, &attached);
//    printf("attachThreadEnv result = %d\n", rc);
//
//    jboolean res_bool = env->CallStaticBooleanMethod(loader_clazz, method_start);
//    printf("start result = %d\n", res_bool);
//
//    if (attached) {
//      java->detachThread();
//    }
//  });
//
//  std::this_thread::sleep_for(std::chrono::milliseconds { 5000 });
//
//  res_bool = env->CallStaticBooleanMethod(loader_clazz, method_stop);
//  printf("stop result = %d\n", res_bool);
//
//  if (th.joinable()) {
//    th.join();
//  }

  printf("go destroy\n");
  jrc = java->destroy();
  printf("java destroy : %d\n", jrc);

  return 0;
}

int main() {
  return wrapped();
}

