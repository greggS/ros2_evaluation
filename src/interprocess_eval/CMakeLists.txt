cmake_minimum_required(VERSION 3.3)

project(interprocess_eval)

if(NOT WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Wextra")
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rmw_implementation_cmake REQUIRED)
find_package(std_msgs REQUIRED)

function(custom_executable target)
  add_executable(${target}${target_suffix}
    src/${target}.cpp)

  ament_target_dependencies(${target}${target_suffix}
    "rclcpp"
    "std_msgs"
    "rt")

  install(TARGETS ${target}${target_suffix}
    DESTINATION lib/${PROJECT_NAME})

endfunction()

macro(targets)
  custom_executable(talker_interprocess)
  custom_executable(listener_interprocess)
  custom_executable(talker_interprocess_single)
  custom_executable(listener_interprocess_single)

endmacro()

call_for_each_rmw_implementation(targets GENERATE_DEFAULT)

ament_package()
