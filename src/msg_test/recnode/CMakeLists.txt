cmake_minimum_required(VERSION 2.8.3)
project(recnode)


find_package(catkin REQUIRED COMPONENTS
  roscpp
  rospy
  std_msgs
  cv_bridge
  image_transport
)

find_package(OpenCV REQUIRED)

catkin_package(
#  INCLUDE_DIRS include
#  LIBRARIES recnode
#  CATKIN_DEPENDS roscpp rospy std_msgs
#  DEPENDS system_lib
)

include_directories(
# include
  ${catkin_INCLUDE_DIRS}
)

add_executable(listener src/rec_msg.cpp)
target_link_libraries(listener ${catkin_LIBRARIES} ${OpenCV_LIBS})
