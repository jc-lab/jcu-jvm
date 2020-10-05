cmake_minimum_required(VERSION 3.8)
project(commons-daemon-native)

set(JAVA_AWT_LIBRARY NotNeeded)
set(JAVA_AWT_INCLUDE_PATH NotNeeded)
find_package(JNI REQUIRED)

set(JCU_JVM_USE_JSIG yes)
if (JCU_JVM_USE_JSIG)
    find_library(JAVA_JSIG_LIBRARY
            NAMES
                jsig
            PATHS
                ${JAVA_JVM_LIBRARY_DIRECTORIES}
            )
    if (JAVA_JSIG_LIBRARY)
        list(APPEND JNI_LIBRARIES ${JAVA_JSIG_LIBRARY})
    endif()
endif(JCU_JVM_USE_JSIG)

message("JNI_LIBRARIES = ${JNI_LIBRARIES}")

string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" temp_arch)
if (temp_arch MATCHES "(x86_64|AMD64)")
    set(HOST_CPU "i386")
elseif(temp_arch MATCHES "(i.?86|x86)")
    set(HOST_CPU "amd64")
elseif(temp_arch MATCHES "(mips.*el)")
    set(HOST_CPU "mipsel")
elseif(temp_arch MATCHES "(mips.*)")
    set(HOST_CPU "mips")
elseif(temp_arch MATCHES "powerpc64")
    set(HOST_CPU "ppc64")
elseif(temp_arch MATCHES "powerpc64le")
    set(HOST_CPU "ppc64le")
elseif(temp_arch MATCHES "powerpc.*")
    set(HOST_CPU "${temp_arch}")
elseif(temp_arch MATCHES "sparc.*")
    set(HOST_CPU "${temp_arch}")
elseif(temp_arch MATCHES "arm.*")
    set(HOST_CPU "arm")
elseif(temp_arch MATCHES "aarch64")
    set(HOST_CPU "aarch64")
endif()

message( STATUS "Architecture: ${CMAKE_SYSTEM_PROCESSOR}" )

list(REMOVE_ITEM JNI_LIBRARIES "NotNeeded")
list(REMOVE_ITEM JNI_INCLUDE_DIRS "NotNeeded")

set(INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/inc/jcu-jvm)
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(SRC_FILES
        ${INC_DIR}/os_handler.h
        ${INC_DIR}/jvm_library.h
        ${INC_DIR}/memory_pool.h
        ${SRC_DIR}/intl_utils.h
        ${SRC_DIR}/intl_utils.cc
        ${INC_DIR}/vm.h
        ${SRC_DIR}/vm.cc
        ${SRC_DIR}/simple_memory_pool.h
        ${SRC_DIR}/simple_memory_pool.cc
        ${SRC_DIR}/jvm_library_base.cc
        )

if (MSVC)
    set(PLAT_SRC_DIR ${SRC_DIR}/plat-win)

    set(PLAT_SRC_FILES
            ${PLAT_SRC_DIR}/os_handler_win.cc
#            ${PLAT_SRC_DIR}/jvm_library_win.cc
            )
    set(PLAT_LIBRARIES)
else()
    set(PLAT_SRC_DIR ${SRC_DIR}/plat-unix)
    set(PLAT_SRC_FILES
            ${PLAT_SRC_DIR}/os_handler_unix.cc
#            ${PLAT_SRC_DIR}/jvm_library_unix.cc
            ${PLAT_SRC_DIR}/dso.h
            ${PLAT_SRC_DIR}/dso-dlfcn.c
            ${PLAT_SRC_DIR}/dso-dyld.c
            ${PLAT_SRC_DIR}/location.h
            ${PLAT_SRC_DIR}/location.c
            )
    set(PLAT_LIBRARIES dl)
endif()

add_library(jcu_jvm STATIC ${SRC_FILES} ${PLAT_SRC_FILES})
target_compile_definitions(jcu_jvm
        PRIVATE
        -DCPU=\"${HOST_CPU}\"
        )
target_link_libraries(jcu_jvm
        PUBLIC
        ${PLAT_LIBRARIES}
        ${JNI_LIBRARIES}
        )
target_include_directories(jcu_jvm
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        PUBLIC
        ${JNI_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
        )

if (NOT MSVC)
    target_compile_definitions(jcu_jvm
            PRIVATE
            -DOS_LINUX
            -DDSO_DLFCN
            )
endif()
