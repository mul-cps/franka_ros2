cmake_minimum_required(VERSION 3.22)
project(franka_robot_state_broadcaster)

# Default to C++20
if(NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 20)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
    add_compile_options(-Werror=return-type)
endif()

option(CHECK_TIDY "Adds clang-tidy tests" OFF)

set(THIS_PACKAGE_INCLUDE_DEPENDS
    builtin_interfaces
    control_msgs
    controller_interface
    controller_manager
    Franka
    franka_msgs
    franka_semantic_components
    generate_parameter_library
    pluginlib
    rclcpp_lifecycle
    rcutils
    realtime_tools
    sensor_msgs)


# find dependencies
find_package(ament_cmake REQUIRED)

foreach(Dependency IN ITEMS ${THIS_PACKAGE_INCLUDE_DEPENDS})
  find_package(${Dependency} REQUIRED)
endforeach()

add_library(franka_robot_state_broadcaster
        SHARED
        src/franka_robot_state_broadcaster.cpp)
target_include_directories(
  franka_robot_state_broadcaster
        PRIVATE
        include
)


generate_parameter_library(franka_robot_state_broadcaster_parameters src/franka_robot_state_broadcaster_parameters.yaml)

target_link_libraries(franka_robot_state_broadcaster PUBLIC
                      franka_semantic_components::franka_semantic_components
                      franka_robot_state_broadcaster_parameters
                      Franka::Franka
                      ${builtin_interfaces_TARGETS}
                      ${control_msgs_TARGETS}
                      ${franka_msgs_TARGETS}
                      ${sensor_msgs_TARGETS}
                      controller_interface::controller_interface
                      controller_manager::controller_manager
                      pluginlib::pluginlib
                      rclcpp_lifecycle::rclcpp_lifecycle
                      rcutils::rcutils
                      realtime_tools::realtime_tools
                      realtime_tools::thread_priority
                      sensor_msgs::sensor_msgs_library
)

pluginlib_export_plugin_description_file(controller_interface franka_robot_state_broadcaster.xml)


install(
        TARGETS franka_robot_state_broadcaster
        DESTINATION lib
)
install(
        DIRECTORY include/
        DESTINATION include
)


if(BUILD_TESTING)
    find_package(ament_cmake_gmock REQUIRED)
    find_package(ament_cmake_clang_format REQUIRED)
    find_package(ament_cmake_copyright REQUIRED)
    find_package(ament_cmake_cppcheck REQUIRED)
    find_package(ament_cmake_flake8 REQUIRED)
    find_package(ament_cmake_lint_cmake REQUIRED)
    find_package(ament_cmake_pep257 REQUIRED)
    find_package(ament_cmake_xmllint REQUIRED)
    find_package(ros2_control_test_assets REQUIRED)
    find_package(controller_manager REQUIRED)

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

    add_rostest_with_parameters_gmock(${PROJECT_NAME}_test
        test/test_franka_robot_state_broadcaster.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/franka_robot_state_broadcaster_parameters.yaml)
    target_include_directories(${PROJECT_NAME}_test PRIVATE include)
    target_link_libraries(${PROJECT_NAME}_test
        ${PROJECT_NAME}
        controller_manager::controller_manager
        hardware_interface::hardware_interface
        hardware_interface::mock_components
        ros2_control_test_assets::ros2_control_test_assets
    )
    target_compile_definitions(
        ${PROJECT_NAME}_test
        PUBLIC controller_interface_VERSION_MAJOR=${controller_interface_VERSION_MAJOR}
    )

    ament_add_gmock(test_load_franka_robot_state_broadcaster test/test_load_franka_robot_state_broadcaster.cpp)
    target_include_directories(test_load_franka_robot_state_broadcaster PRIVATE include)
    target_link_libraries(test_load_franka_robot_state_broadcaster
        ${PROJECT_NAME}
        controller_manager::controller_manager
    )
endif()

ament_export_include_directories(
        include
)
ament_export_libraries(
        franka_robot_state_broadcaster
)
ament_export_dependencies(
        hardware_interface
        pluginlib
        rclcpp
)
ament_package()
