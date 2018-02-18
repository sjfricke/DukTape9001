set(EXAMPLE_FILES 
  main.cpp
  )

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -g -std=c++11 -lGL -lglut -lGLU")
SET( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -g -std=c++11 -lGL -lglut -lGLU")

foreach(EXAMPLE_FILE ${EXAMPLE_FILES})

  get_filename_component(SOURCE_NAME ${EXAMPLE_FILE} NAME_WE)
  
  if (UNIX AND NOT APPLE)
    set(EXE_NAME "${SOURCE_NAME}.bin")
  elseif (WIN32)
    set(EXE_NAME "${SOURCE_NAME}")
  endif ()

  message(STATUS "Adding Example ${EXE_NAME}")
  add_executable(${EXE_NAME} ${EXAMPLE_FILE} userdimensions.cpp)
  include_directories(library ${OPENGL_INCLUDE_DIRS} ${GLUT_INCLUDE_DIRS})
  target_link_libraries(${EXE_NAME} openpose ${GLOG_LIBRARY} ${GFLAGS_LIBRARY} ${Caffe_LIBS} ${MKL_LIBS} ${GLUT_LIBRARY} ${SPINNAKER_LIB} ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})
  
  if (WIN32)
    set_property(TARGET ${EXE_NAME} PROPERTY FOLDER "Examples/Tutorial/Pose")
    configure_file(${CMAKE_SOURCE_DIR}/cmake/OpenPose.vcxproj.user
        ${CMAKE_CURRENT_BINARY_DIR}/${EXE_NAME}.vcxproj.user @ONLY)
    # Properties->General->Output Directory
    set_property(TARGET ${EXE_NAME} PROPERTY RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_BINARY_DIR}/$(Platform)/$(Configuration))
    set_property(TARGET ${EXE_NAME} PROPERTY RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_BINARY_DIR}/$(Platform)/$(Configuration))
  endif (WIN32)

endforeach()
