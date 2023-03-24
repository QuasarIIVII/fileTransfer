project(
	A
	LANGUAGES CXX
	)

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)
set(CMAKE_FIND_ROOT_PATH /usr/aarch64-linux-gnu)
set(CMAKE_CROSSCOMPILING ON)

# Add the library search path for aarch64
link_directories(/usr/aarch64-linux-gnu/lib64)

# Set the target architecture and linker flags
set(CMAKE_EXE_LINKER_FLAGS "-march=armv8-a")
set(CMAKE_SHARED_LINKER_FLAGS "-march=armv8-a")

# Add your source files and compile your program
add_compile_options(--static --std=c++2a)
add_executable(a a.cpp)

# Link against the appropriate libraries
target_link_libraries(a stdc++ pthread)

