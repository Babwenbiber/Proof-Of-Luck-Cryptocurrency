# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.5

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/frede/Documents/SEP/18-ibr_ds_0/src/Enclave

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/frede/Documents/SEP/18-ibr_ds_0/src/Enclave/build

# Include any dependencies generated for this target.
include CMakeFiles/enclave.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/enclave.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/enclave.dir/flags.make

../App/Enclave_u.c: ../Enclave/Enclave.edl
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/frede/Documents/SEP/18-ibr_ds_0/src/Enclave/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Generating ../App/Enclave_u.c, ../App/Enclave_u.h, ../Enclave/Enclave_t.c, ../Enclave/Enclave_t.h"
	cd /home/frede/Documents/SEP/18-ibr_ds_0/src/Enclave && /home/frede/Programmiertes/SGX/sgxsdk/bin/x64/sgx_edger8r --search-path Enclave --search-path /home/frede/Programmiertes/SGX/sgxsdk/include --untrusted-dir App --trusted-dir Enclave Enclave.edl

../App/Enclave_u.h: ../App/Enclave_u.c
	@$(CMAKE_COMMAND) -E touch_nocreate ../App/Enclave_u.h

../Enclave/Enclave_t.c: ../App/Enclave_u.c
	@$(CMAKE_COMMAND) -E touch_nocreate ../Enclave/Enclave_t.c

../Enclave/Enclave_t.h: ../App/Enclave_u.c
	@$(CMAKE_COMMAND) -E touch_nocreate ../Enclave/Enclave_t.h

CMakeFiles/enclave.dir/Enclave/Enclave.cpp.o: CMakeFiles/enclave.dir/flags.make
CMakeFiles/enclave.dir/Enclave/Enclave.cpp.o: ../Enclave/Enclave.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/frede/Documents/SEP/18-ibr_ds_0/src/Enclave/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/enclave.dir/Enclave/Enclave.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/enclave.dir/Enclave/Enclave.cpp.o -c /home/frede/Documents/SEP/18-ibr_ds_0/src/Enclave/Enclave/Enclave.cpp

CMakeFiles/enclave.dir/Enclave/Enclave.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/enclave.dir/Enclave/Enclave.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/frede/Documents/SEP/18-ibr_ds_0/src/Enclave/Enclave/Enclave.cpp > CMakeFiles/enclave.dir/Enclave/Enclave.cpp.i

CMakeFiles/enclave.dir/Enclave/Enclave.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/enclave.dir/Enclave/Enclave.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/frede/Documents/SEP/18-ibr_ds_0/src/Enclave/Enclave/Enclave.cpp -o CMakeFiles/enclave.dir/Enclave/Enclave.cpp.s

CMakeFiles/enclave.dir/Enclave/Enclave.cpp.o.requires:

.PHONY : CMakeFiles/enclave.dir/Enclave/Enclave.cpp.o.requires

CMakeFiles/enclave.dir/Enclave/Enclave.cpp.o.provides: CMakeFiles/enclave.dir/Enclave/Enclave.cpp.o.requires
	$(MAKE) -f CMakeFiles/enclave.dir/build.make CMakeFiles/enclave.dir/Enclave/Enclave.cpp.o.provides.build
.PHONY : CMakeFiles/enclave.dir/Enclave/Enclave.cpp.o.provides

CMakeFiles/enclave.dir/Enclave/Enclave.cpp.o.provides.build: CMakeFiles/enclave.dir/Enclave/Enclave.cpp.o


CMakeFiles/enclave.dir/Enclave/Enclave_t.c.o: CMakeFiles/enclave.dir/flags.make
CMakeFiles/enclave.dir/Enclave/Enclave_t.c.o: ../Enclave/Enclave_t.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/frede/Documents/SEP/18-ibr_ds_0/src/Enclave/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building C object CMakeFiles/enclave.dir/Enclave/Enclave_t.c.o"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/enclave.dir/Enclave/Enclave_t.c.o   -c /home/frede/Documents/SEP/18-ibr_ds_0/src/Enclave/Enclave/Enclave_t.c

CMakeFiles/enclave.dir/Enclave/Enclave_t.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/enclave.dir/Enclave/Enclave_t.c.i"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/frede/Documents/SEP/18-ibr_ds_0/src/Enclave/Enclave/Enclave_t.c > CMakeFiles/enclave.dir/Enclave/Enclave_t.c.i

CMakeFiles/enclave.dir/Enclave/Enclave_t.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/enclave.dir/Enclave/Enclave_t.c.s"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/frede/Documents/SEP/18-ibr_ds_0/src/Enclave/Enclave/Enclave_t.c -o CMakeFiles/enclave.dir/Enclave/Enclave_t.c.s

CMakeFiles/enclave.dir/Enclave/Enclave_t.c.o.requires:

.PHONY : CMakeFiles/enclave.dir/Enclave/Enclave_t.c.o.requires

CMakeFiles/enclave.dir/Enclave/Enclave_t.c.o.provides: CMakeFiles/enclave.dir/Enclave/Enclave_t.c.o.requires
	$(MAKE) -f CMakeFiles/enclave.dir/build.make CMakeFiles/enclave.dir/Enclave/Enclave_t.c.o.provides.build
.PHONY : CMakeFiles/enclave.dir/Enclave/Enclave_t.c.o.provides

CMakeFiles/enclave.dir/Enclave/Enclave_t.c.o.provides.build: CMakeFiles/enclave.dir/Enclave/Enclave_t.c.o


# Object files for target enclave
enclave_OBJECTS = \
"CMakeFiles/enclave.dir/Enclave/Enclave.cpp.o" \
"CMakeFiles/enclave.dir/Enclave/Enclave_t.c.o"

# External object files for target enclave
enclave_EXTERNAL_OBJECTS =

libenclave.so: CMakeFiles/enclave.dir/Enclave/Enclave.cpp.o
libenclave.so: CMakeFiles/enclave.dir/Enclave/Enclave_t.c.o
libenclave.so: CMakeFiles/enclave.dir/build.make
libenclave.so: CMakeFiles/enclave.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/frede/Documents/SEP/18-ibr_ds_0/src/Enclave/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Linking CXX shared library libenclave.so"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/enclave.dir/link.txt --verbose=$(VERBOSE)
	cd /home/frede/Documents/SEP/18-ibr_ds_0/src/Enclave && /home/frede/Programmiertes/SGX/sgxsdk/bin/x64/sgx_sign sign -key Enclave/Enclave.pem -enclave /home/frede/Documents/SEP/18-ibr_ds_0/src/Enclave/build/libenclave.so -out /home/frede/Documents/SEP/18-ibr_ds_0/src/Enclave/build/libenclave.signed.so -config Enclave/Enclave.config.xml

# Rule to build all files generated by this target.
CMakeFiles/enclave.dir/build: libenclave.so

.PHONY : CMakeFiles/enclave.dir/build

CMakeFiles/enclave.dir/requires: CMakeFiles/enclave.dir/Enclave/Enclave.cpp.o.requires
CMakeFiles/enclave.dir/requires: CMakeFiles/enclave.dir/Enclave/Enclave_t.c.o.requires

.PHONY : CMakeFiles/enclave.dir/requires

CMakeFiles/enclave.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/enclave.dir/cmake_clean.cmake
.PHONY : CMakeFiles/enclave.dir/clean

CMakeFiles/enclave.dir/depend: ../App/Enclave_u.c
CMakeFiles/enclave.dir/depend: ../App/Enclave_u.h
CMakeFiles/enclave.dir/depend: ../Enclave/Enclave_t.c
CMakeFiles/enclave.dir/depend: ../Enclave/Enclave_t.h
	cd /home/frede/Documents/SEP/18-ibr_ds_0/src/Enclave/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/frede/Documents/SEP/18-ibr_ds_0/src/Enclave /home/frede/Documents/SEP/18-ibr_ds_0/src/Enclave /home/frede/Documents/SEP/18-ibr_ds_0/src/Enclave/build /home/frede/Documents/SEP/18-ibr_ds_0/src/Enclave/build /home/frede/Documents/SEP/18-ibr_ds_0/src/Enclave/build/CMakeFiles/enclave.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/enclave.dir/depend

