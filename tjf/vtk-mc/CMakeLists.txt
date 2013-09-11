cmake_minimum_required(VERSION 2.8)
 
PROJECT(tjfMC)
 
find_package(VTK REQUIRED)
include(${VTK_USE_FILE})
 
add_executable(mc MACOSX_BUNDLE mc)
 
if(VTK_LIBRARIES)
  target_link_libraries(mc ${VTK_LIBRARIES})
else()
  target_link_libraries(mc vtkHybrid)
endif()
