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
include CMakeFiles/app.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/app.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/app.dir/flags.make

../App/Enclave_u.c: ../Enclave/Enclave.edl
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/frede/Documents/SEP/18-ibr_ds_0/src/Enclave/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Generating ../App/Enclave_u.c, ../App/Enclave_u.h, ../Enclave/Enclave_t.c, ../Enclave/Enclave_t.h"
	cd /home/frede/Documents/SEP/18-ibr_ds_0/src/Enclave && /home/frede/Programmiertes/SGX/sgxsdk/bin/x64/sgx_edger8r --search-path Enclave --search-path /home/frede/Programmiertes/SGX/sgxsdk/include --untrusted-dir App --trusted-dir Enclave Enclave.edl

../App/Enclave_u.h: ../App/Enclave_u.c
	@$(CMAKE_COMMAND) -E touch_nocreate ../App/Enclave_u.h

../Enclave/Enclave_t.c: ../App/Enclave_u.c
	@$(CMAKE_COMMAND) -E touch_nocreate ../Enclave/Enclave_t.c

../Enclave/Enclave_t.h: ../App/Enclave_u.c
	@$(CMAKE_COMMAND) -E touch_nocreate ../Enclave/Enclave_t.h

CMakeFiles/app.dir/App/App.cpp.o: CMakeFiles/app.dir/flags.make
CMakeFiles/app.dir/App/App.cpp.o: ../App/App.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/frede/Documents/SEP/18-ibr_ds_0/src/Enclave/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/app.dir/App/App.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/app.dir/App/App.cpp.o -c /home/frede/Documents/SEP/18-ibr_ds_0/src/Enclave/App/App.cpp

CMakeFiles/app.dir/App/App.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/app.dir/App/App.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/frede/Documents/SEP/18-ibr_ds_0/src/Enclave/App/App.cpp > CMakeFiles/app.dir/App/App.cpp.i

CMakeFiles/app.dir/App/App.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/app.dir/App/App.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/frede/Documents/SEP/18-ibr_ds_0/src/Enclave/App/App.cpp -o CMakeFiles/app.dir/App/App.cpp.s

CMakeFiles/app.dir/App/App.cpp.o.requires:

.PHONY : CMakeFiles/app.dir/App/App.cpp.o.requires

CMakeFiles/app.dir/App/App.cpp.o.provides: CMakeFiles/app.dir/App/App.cpp.o.requires
	$(MAKE) -f CMakeFiles/app.dir/build.make CMakeFiles/app.dir/App/App.cpp.o.provides.build
.PHONY : CMakeFiles/app.dir/App/App.cpp.o.provides

CMakeFiles/app.dir/App/App.cpp.o.provides.build: CMakeFiles/app.dir/App/App.cpp.o


CMakeFiles/app.dir/App/Enclave_u.c.o: CMakeFiles/app.dir/flags.make
CMakeFiles/app.dir/App/Enclave_u.c.o: ../App/Enclave_u.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/frede/Documents/SEP/18-ibr_ds_0/src/Enclave/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building C object CMakeFiles/app.dir/App/Enclave_u.c.o"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/app.dir/App/Enclave_u.c.o   -c /home/frede/Documents/SEP/18-ibr_ds_0/src/Enclave/App/Enclave_u.c

CMakeFiles/app.dir/App/Enclave_u.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/app.dir/App/Enclave_u.c.i"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/frede/Documents/SEP/18-ibr_ds_0/src/Enclave/App/Enclave_u.c > CMakeFiles/app.dir/App/Enclave_u.c.i

CMakeFiles/app.dir/App/Enclave_u.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/app.dir/App/Enclave_u.c.s"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/frede/Documents/SEP/18-ibr_ds_0/src/Enclave/App/Enclave_u.c -o CMakeFiles/app.dir/App/Enclave_u.c.s

CMakeFiles/app.dir/App/Enclave_u.c.o.requires:

.PHONY : CMakeFiles/app.dir/App/Enclave_u.c.o.requires

CMakeFiles/app.dir/App/Enclave_u.c.o.provides: CMakeFiles/app.dir/App/Enclave_u.c.o.requires
	$(MAKE) -f CMakeFiles/app.dir/build.make CMakeFiles/app.dir/App/Enclave_u.c.o.provides.build
.PHONY : CMakeFiles/app.dir/App/Enclave_u.c.o.provides

CMakeFiles/app.dir/App/Enclave_u.c.o.provides.build: CMakeFiles/app.dir/App/Enclave_u.c.o


# Object files for target app
app_OBJECTS = \
"CMakeFiles/app.dir/App/App.cpp.o" \
"CMakeFiles/app.dir/App/Enclave_u.c.o"

# External object files for target app
app_EXTERNAL_OBJECTS =

app: CMakeFiles/app.dir/App/App.cpp.o
app: CMakeFiles/app.dir/App/Enclave_u.c.o
app: CMakeFiles/app.dir/build.make
app: /opt/intel/sgxsdk/lib64/libsgx_urts_sim.so
app: /opt/intel/sgxsdk/lib64/libsgx_uae_service_sim.so
app: CMakeFiles/app.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/frede/Documents/SEP/18-ibr_ds_0/src/Enclave/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Linking CXX executable app"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/app.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/app.dir/build: app

.PHONY : CMakeFiles/app.dir/build

CMakeFiles/app.dir/requires: CMakeFiles/app.dir/App/App.cpp.o.requires
CMakeFiles/app.dir/requires: CMakeFiles/app.dir/App/Enclave_u.c.o.requires

.PHONY : CMakeFiles/app.dir/requires

CMakeFiles/app.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/app.dir/cmake_clean.cmake
.PHONY : CMakeFiles/app.dir/clean

CMakeFiles/app.dir/depend: ../App/Enclave_u.c
CMakeFiles/app.dir/depend: ../App/Enclave_u.h
CMakeFiles/app.dir/depend: ../Enclave/Enclave_t.c
CMakeFiles/app.dir/depend: ../Enclave/Enclave_t.h
	cd /home/frede/Documents/SEP/18-ibr_ds_0/src/Enclave/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/frede/Documents/SEP/18-ibr_ds_0/src/Enclave /home/frede/Documents/SEP/18-ibr_ds_0/src/Enclave /home/frede/Documents/SEP/18-ibr_ds_0/src/Enclave/build /home/frede/Documents/SEP/18-ibr_ds_0/src/Enclave/build /home/frede/Documents/SEP/18-ibr_ds_0/src/Enclave/build/CMakeFiles/app.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/app.dir/depend
