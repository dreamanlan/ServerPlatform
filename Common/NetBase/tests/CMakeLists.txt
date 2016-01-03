
include_directories(${Boost_INCLUDE_DIRS})

foreach(target echo massive_svr massive_cli)
  add_executable(${target} ${target}.cc)
  if(UNIX)
    set_target_properties(${target} PROPERTIES
      COMPILE_FLAGS "-std=c++0x -Wall"
      )
  endif(UNIX)
  target_link_libraries(${target} netbase)
endforeach(target echo massive_svr massive_cli)

install(TARGETS echo massive_cli massive_svr RUNTIME DESTINATION ${CMAKE_SOURCE_DIR}/bin)
install(FILES ${CMAKE_CURRENT_LIST_DIR}/data DESTINATION ${CMAKE_SOURCE_DIR}/bin)
