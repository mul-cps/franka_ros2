// Copyright (c) 2021 Franka Emika GmbH
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <franka_hardware/robot.hpp>

#include <cassert>
#include <mutex>

#include <franka/control_tools.h>
#include <franka/exception.h>
#include <rclcpp/logging.hpp>

namespace franka_hardware {

Robot::Robot(const std::string& robot_ip, const rclcpp::Logger& logger) {
  tau_command_.fill(0.);
  franka::RealtimeConfig rt_config = franka::RealtimeConfig::kEnforce;
  if (!franka::hasRealtimeKernel()) {
    rt_config = franka::RealtimeConfig::kIgnore;
    RCLCPP_WARN(
        logger,
        "You are not using a real-time kernel. Using a real-time kernel is strongly recommended!");
  }
  robot_ = std::make_unique<franka::Robot>(robot_ip, rt_config);
  model_ = std::make_unique<franka::Model>(robot_->loadModel());
  franka_hardware_model_ = std::make_unique<Model>(model_.get());
}

void Robot::write(const std::array<double, 7>& efforts) {
  std::lock_guard<std::mutex> lock(write_mutex_);
  tau_command_ = efforts;
}

franka::RobotState Robot::read() {
  std::lock_guard<std::mutex> lock(read_mutex_);
  return {current_state_};
}

franka_hardware::Model* Robot::getModel() {
  return franka_hardware_model_.get();
}

void Robot::stopRobot() {
  if (!stopped_) {
    finish_ = true;
    control_thread_->join();
    finish_ = false;
    stopped_ = true;
  }
}

void Robot::initializeTorqueControl() {
  assert(isStopped());
  stopped_ = false;
  const auto kTorqueControl = [this]() {
    try {
      robot_->control(
          [this](const franka::RobotState& state, const franka::Duration& /*period*/) {
            {
              std::lock_guard<std::mutex> lock(read_mutex_);
              current_state_ = state;
            }
            std::lock_guard<std::mutex> lock(write_mutex_);
            franka::Torques out(tau_command_);
            out.motion_finished = finish_;
            return out;
          },
          true, franka::kMaxCutoffFrequency);
    } catch (const franka::ControlException& e) {
      // libfranka: Move command rejected: command not possible in the current mode ("User
      // stopped")!
      RCLCPP_INFO_STREAM(rclcpp::get_logger("FrankaHardwareInterface"),
                         "Exception from control thread: " << std::string(e.what())
                                                           << " (logs: " << e.log.size() << ")");
      for (const franka::Record& log : e.log) {
        RCLCPP_WARN_STREAM(rclcpp::get_logger("FrankaHardwareInterface"),
                           "mode " << log.state.robot_mode);
      }
    }
  };
  control_thread_ = std::make_unique<std::thread>(kTorqueControl);
}

void Robot::initializeContinuousReading() {
  assert(isStopped());
  stopped_ = false;
  const auto kReading = [this]() {
    robot_->read([this](const franka::RobotState& state) {
      {
        std::lock_guard<std::mutex> lock(read_mutex_);
        current_state_ = state;
      }
      return !finish_;
    });
  };
  control_thread_ = std::make_unique<std::thread>(kReading);
}

Robot::~Robot() {
  stopRobot();
}

bool Robot::isStopped() const {
  return stopped_;
}
}  // namespace franka_hardware
