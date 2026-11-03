cmake_minimum_required(VERSION 3.0.2)
project(ball_chaser)

add_compile_options(-std=c++14 -Wall -Wextra)

find_package(catkin REQUIRED COMPONENTS
  roscpp
  std_msgs
  geometry_msgs
  sensor_msgs
  message_generation
)

add_service_files(
  FILES
  DriveToTarget.srv
)

generate_messages(
  DEPENDENCIES
  std_msgs
)

catkin_package(
  INCLUDE_DIRS include
  CATKIN_DEPENDS roscpp std_msgs geometry_msgs sensor_msgs message_runtime
)

include_directories(
  include
  ${catkin_INCLUDE_DIRS}
)

# Pure image analysis, kept free of ROS so it can be unit tested
add_library(ball_region src/ball_region.cpp)

add_executable(drive_bot src/drive_bot.cpp)
target_link_libraries(drive_bot ${catkin_LIBRARIES})
add_dependencies(drive_bot ball_chaser_generate_messages_cpp)

add_executable(process_image src/process_image.cpp)
target_link_libraries(process_image ball_region ${catkin_LIBRARIES})
add_dependencies(process_image ball_chaser_generate_messages_cpp)

if(CATKIN_ENABLE_TESTING)
  catkin_add_gtest(test_ball_region test/test_ball_region.cpp)
  target_link_libraries(test_ball_region ball_region)
endif()
