# Building

## Using autotools/Make
TBD

## Using CMake
CMake is a cross platform build configuration tool.

### Configuring
To build, you need to first configure for your target platform, be it MSVC, Unix Makefiles, Ninja etc.

First create a build directory
```
mkdir build && cd build
```

#### Windows:

You have different generators depending on your environment. Here we use Visual Studio 2019:
```
cmake -G "Visual Studio 16 2019" ..
```

#### Linux:
On Linux, the default generator is often Unix Makefiles:

```
cmake ..
```

#### Cross-compile using toolchain
To cross compile, use a toolchain.

Example
```
cmake -DCMAKE_TOOLCHAIN_FILE=./cmake/toolchains/arm-none-eabi.cmake" ..
```

### Building

```
cmake --build .
```

### Installing

System wide:
```
cmake --build . --target install
```

Local:
```
cmake -DCMAKE_INSTALL_PREFIX=install_path ..
&& cmake --build . --target install
```

### Testing

Configure for testing
```
cmake -DENABLE_TESTS=ON ..
```

If you can run the resulting test binary on the host:
```
ctest -C Debug|Release
```

## Linking with other targets
CMake can find installed targets by using the `find_package` macro.

Example:
`find_package(fsmlite)`

You can the link with the target like so:

```
target_link_libraries(${NAME}
    PRIVATE
        fsmlite::fsmlite
)
```

If you compiled fsmlite outside of the build tree, you can now access fsmlite by calling

```
include <fsmlite/fsm.h>
```

See `tests` for full example.