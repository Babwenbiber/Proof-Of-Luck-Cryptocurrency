cmake_minimum_required(VERSION 3.5)

#cmake for SIM MODE

include(FindSGXSDK.cmake REQUIRED)
set(LDS_NAME Enclave.lds)

add_custom_command(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/App/Enclave_u.c ${CMAKE_CURRENT_SOURCE_DIR}/App/Enclave_u.h ${CMAKE_CURRENT_SOURCE_DIR}/Enclave/Enclave_t.c ${CMAKE_CURRENT_SOURCE_DIR}/Enclave/Enclave_t.h
        COMMAND ${SGXSDK_EDGER8R} --search-path Enclave --search-path ${SGXSDK_INCLUDE_DIRS} --untrusted-dir App --trusted-dir Enclave Enclave.edl
        DEPENDS Enclave/Enclave.edl
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )

add_custom_command(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/Enclave/Enclave.pem
        COMMAND openssl genrsa -out Enclave/Enclave.pem -3 3072
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_custom_target(generate_key DEPENDS Enclave/Enclave.pem)

set(SAMPLE_APP_SOURCE_FILES
        App/App.cpp
        App/Enclave_u.c)

set(SAMPLE_ENC_SOURCE_FILES
        Enclave/Enclave.cpp
        Enclave/Enclave_t.c)

set(ENCL_FLAGS "-Wl,--no-undefined -nostdlib -nodefaultlibs -nostartfiles -L${SGXSDK_ENCL_LIB_DIR} -Wl,--whole-archive \
        -l${SGXSDK_TRTS_SIM} -Wl,--no-whole-archive -Wl,--start-group -l${SGXSDK_TSTDC} -l${SGXSDK_TSTDCXX} -l${SGXSDK_TCRYPTO} \
        -l${SGXSDK_TSERVICE_SIM} -Wl,--end-group -Wl,-Bstatic -Wl,-Bsymbolic -Wl,--no-undefined -Wl,-pie,-eenclave_entry \
        -Wl,--export-dynamic -Wl,--defsym,__ImageBase=0 -Wl,--gc-sections -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/Enclave/${LDS_NAME}")

add_library(enclave SHARED EXCLUDE_FROM_ALL ${SAMPLE_ENC_SOURCE_FILES})
target_include_directories(enclave PUBLIC ${SGXSDK_ENCL_CXX_INCLUDE_DIRS})
target_compile_options(enclave PUBLIC -nostdinc -nostdinc++ -fvisibility=hidden -fpie -fstack-protector -std=c++03)
target_link_libraries(enclave PUBLIC ${ENCL_FLAGS})
add_dependencies(enclave generate_key)
#set_target_properties(enclave PROPERTIES LINK_FLAGS ${ENCL_FLAGS})

add_custom_command(TARGET enclave POST_BUILD
        COMMAND ${SGXSDK_SIGN} sign -key Enclave/Enclave.pem -enclave ${CMAKE_CURRENT_SOURCE_DIR}/build/libenclave.so -out ${CMAKE_CURRENT_SOURCE_DIR}/build/libenclave.signed.so -config Enclave/Enclave.config.xml
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(app EXCLUDE_FROM_ALL ${SAMPLE_APP_SOURCE_FILES})
target_include_directories(app PUBLIC ${SGXSDK_INCLUDE_DIRS})
target_link_libraries(app PUBLIC ${SGXSDK_URTS_SIM} pthread ${SGXSDK_UAE_SERVICE_SIM})
