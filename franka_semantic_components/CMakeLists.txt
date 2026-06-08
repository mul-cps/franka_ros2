cmake_minimum_required(VERSION 3.22)
project(franka_semantic_components)

# Default to C++17
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

option(CHECK_TIDY "Adds clang-tidy tests" OFF)

set(THIS_PACKAGE_INCLUDE_DEPENDS Franka franka_hardware franka_msgs hardware_interface controller_interface rclcpp_lifecycle)

# find dependencies
find_package(ament_cmake REQUIRED)
foreach(Dependency IN ITEMS ${THIS_PACKAGE_INCLUDE_DEPENDS})
  find_package(${Dependency} REQUIRED)
endforeach()

add_library(franka_semantic_components SHARED src/franka_robot_state.cpp src/franka_robot_model.cpp)
target_include_directories(franka_semantic_components PRIVATE include)
target_link_libraries(franka_semantic_components PUBLIC
    Franka::Franka
    franka_hardware::franka_hardware
    ${franka_msgs_TARGETS}
    controller_interface::controller_interface
    hardware_interface::hardware_interface
    hardware_interface::mock_components
    rclcpp_lifecycle::rclcpp_lifecycle
)
if(${hardware_interface_VERSION} VERSION_GREATER_EQUAL "4.27")
    target_compile_definitions(franka_semantic_components PRIVATE HW_HAS_GET_OPT)
endif()

install(TARGETS franka_semantic_components
  EXPORT franka_semantic_componentsTargets
  DESTINATION lib
  INCLUDES
  DESTINATION include
)
install(DIRECTORY include/ DESTINATION include)

if(BUILD_TESTING)
  find_package(ament_cmake_clang_format REQUIRED)
  find_package(ament_cmake_copyright REQUIRED)
  find_package(ament_cmake_cppcheck REQUIRED)
  find_package(ament_cmake_flake8 REQUIRED)
  find_package(ament_cmake_lint_cmake REQUIRED)
  find_package(ament_cmake_pep257 REQUIRED)
  find_package(ament_cmake_xmllint REQUIRED)
  find_package(ament_cmake_gmock REQUIRED)

  set(CPP_DIRECTORIES src include)
  ament_clang_format(CONFIG_FILE ../.clang-format ${CPP_DIRECTORIES})
  ament_copyright(${CPP_DIRECTORIES} package.xml)
  ament_cppcheck(${CPP_DIRECTORIES})
  ament_lint_cmake(CMakeLists.txt)
  ament_flake8()
  ament_pep257()
  ament_xmllint()
  if(CHECK_TIDY)
    find_package(ament_cmake_clang_tidy REQUIRED)
    set(ament_cmake_clang_tidy_CONFIG_FILE ../.clang-tidy)
    ament_clang_tidy(${CMAKE_BINARY_DIR})
  endif()

  ament_add_gmock(test_franka_robot_state test/franka_robot_state_test.cpp)
  target_include_directories(test_franka_robot_state PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
  target_link_libraries(test_franka_robot_state
    franka_semantic_components
    ${franka_msgs_TARGETS}
    controller_interface::controller_interface
    hardware_interface::hardware_interface
    hardware_interface::mock_components
    Franka::Franka
  )
  ament_add_gmock(test_franka_robot_model test/franka_robot_model_test.cpp)
  target_include_directories(test_franka_robot_model PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
  target_link_libraries(test_franka_robot_model
    franka_semantic_components
    ${franka_msgs_TARGETS}
    controller_interface::controller_interface
    hardware_interface::hardware_interface
    hardware_interface::mock_components
    Franka::Franka
  )
endif()

ament_export_include_directories(include)
ament_export_libraries(franka_semantic_components)
ament_export_targets(franka_semantic_componentsTargets HAS_LIBRARY_TARGET)
ament_export_dependencies(${THIS_PACKAGE_INCLUDE_DEPENDS})
ament_package()
