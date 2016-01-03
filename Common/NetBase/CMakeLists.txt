
include_directories(
  ${LIBEVENT_INCLUDE_DIR}
  ${Boost_INCLUDE_DIRS}
  )

set(NETBASE_SOURCES
  connector.h
  connector.cc
  exception.h
  exception.cc
  listener.cc
  listener.h
  netbase.h
  netbase.cc
  tcp_session.h
  tcp_session.cc
  util.h
  util.cc
  )

add_library_ex(netbase STATIC ${NETBASE_SOURCES})
folder(netbase "common")
target_link_libraries(netbase ${LIBEVENT_CORE_LIB})
if(UNIX)
  target_link_libraries(netbase ${LIBEVENT_PTHREADS_LIB})
endif(UNIX)

link_boost_library(netbase thread system chrono)
include_directories(${CMAKE_CURRENT_LIST_DIR})

if(UNIX)
add_subdirectory(tests)
endif(UNIX)

# sdk install
install(TARGETS netbase
  ARCHIVE DESTINATION ../sdk/lib
  )

install(FILES 
  connector.h
  exception.h
  listener.h
  netbase.h
  tcp_session.h
  DESTINATION ../sdk/include/netbase
  )
