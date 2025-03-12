find_package(ament_cmake_gmock REQUIRED)

ament_add_gmock(${PROJECT_NAME}_test franka_hardware_interface_test.cpp)
target_include_directories(${PROJECT_NAME}_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_link_libraries(${PROJECT_NAME}_test  ${PROJECT_NAME})

if(${hardware_interface_VERSION} VERSION_GREATER_EQUAL "4.27")
    target_compile_definitions(${PROJECT_NAME}_test PRIVATE HW_HAS_GET_OPT)
endif()
