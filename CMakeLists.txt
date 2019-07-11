cmake_minimum_required(VERSION 3.10.2)
project(VitCoin)

set(CMAKE_C_FLAGS "-Wall -Wextra -pedantic -o3 -g -pthread")
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

#set libs location
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/src/lib/dynamic)
link_directories(${CMAKE_SOURCE_DIR}/src/lib)

#set header file of libs location
set(INCLUDE_LOCATION ${CMAKE_SOURCE_DIR}/src/header)
include_directories(SYSTEM ${INCLUDE_LOCATION})

#set bin dir
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})


#search for OpenSSL lib required
find_package(OpenSSL REQUIRED)
if( OpenSSL_FOUND )
  set(OPENSSL_USE_STATIC_LIBS TRUE)
  #message(STATUS "Using OpenSSL ${OPENSSL_VERSION}")
  #message("OpenSSL include dir: ${OPENSSL_INCLUDE_DIR}")
  #message("OpenSSL libraries: ${OPENSSL_LIBRARIES}")
endif()

# Collects the names of all the source files in lib directory
# and stores the list in the LIB_FILES.
AUX_SOURCE_DIRECTORY(src/lib LIB_FILES)
# add the libs indicated by LIB_FILES.
# SHARED option indicates that libraries are linked dynamically and loaded at runtime
add_library(libraries SHARED ${LIB_FILES})

# add specific lib for server
set(SERVER_LIBS src/server/server.c)
add_executable(server src/server/main.c ${SERVER_LIBS})
target_link_libraries(server libraries OpenSSL::SSL)

# add specific lib for peer
set(PEER_LIBS src/peer/peer.c)
# TODO: controlla se attivare questo
#set(PEER_LIBS src/peer/peer.c src/peer/peer_structure.c)
add_executable(peer src/peer/main.c ${PEER_LIBS})
target_link_libraries(peer libraries OpenSSL::SSL)

# add specific lib for wallet
set(WALLET_LIBS src/wallet/wallet.c)
add_executable(wallet src/wallet/main.c ${WALLET_LIBS})
target_link_libraries(wallet libraries OpenSSL::SSL)
