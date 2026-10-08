/****************************************************************************
 *
 *   Copyright (c) 2024 PX4 Development Team. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name PX4 nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

#include <gtest/gtest.h>
#include "FlightModeManager.hpp"
#include <uORB/Publication.hpp>
#include <uORB/Subscription.hpp>
#include <uORB/topics/vehicle_status.h>
#include <uORB/topics/vehicle_control_mode.h>
#include <uORB/topics/vehicle_command.h>
#include <uORB/topics/takeoff_status.h>
#include <uORB/topics/vehicle_local_position.h>
#include <uORB/topics/trajectory_setpoint.h>
#include <uORB/topics/vehicle_constraints.h>
#include <uORB/topics/landing_gear.h>
#include <uORB/topics/manual_control_setpoint.h>
#include <uORB/topics/home_position.h>

class TestFlightModeManager : public FlightModeManager
{
public:
	TestFlightModeManager() : FlightModeManager() {}
	~TestFlightModeManager() override = default;

	FlightTaskError switchTask(FlightTaskIndex new_task_index) { return FlightModeManager::switchTask(new_task_index); }
	FlightTaskError switchTask(int new_task_index) { return FlightModeManager::switchTask(new_task_index); }
	void start_flight_task() { FlightModeManager::start_flight_task(); }
	void handleCommand() { FlightModeManager::handleCommand(); }
	void tryApplyCommandIfAny() { FlightModeManager::tryApplyCommandIfAny(); }
	void generateTrajectorySetpoint(const float dt, const vehicle_local_position_s &vehicle_local_position)
	{
		FlightModeManager::generateTrajectorySetpoint(dt, vehicle_local_position);
	}
	const char *errorToString(const FlightTaskError error) { return FlightModeManager::errorToString(error); }
	bool isAnyTaskActive() const { return FlightModeManager::isAnyTaskActive(); }
	FlightTaskIndex getCurrentTaskIndex() const { return _current_task.index; }
	FlightTask *getCurrentTask() const { return _current_task.task; }
	void updateParams() override { FlightModeManager::updateParams(); }
	int print_status() override { return FlightModeManager::print_status(); }
	static int print_usage(const char *reason = nullptr) { return FlightModeManager::print_usage(reason); }
	static int custom_command(int argc, char *argv[]) { return FlightModeManager::custom_command(argc, argv); }

	void updateSubscriptions()
	{
		_vehicle_control_mode_sub.update();
		_vehicle_land_detected_sub.update();
		_vehicle_status_sub.update();
	}

	void setCommand(const vehicle_command_s &cmd) { _current_command = cmd; }
	vehicle_command_s getCommand() const { return _current_command; }
	void setTakeoffState(uint8_t state) { _takeoff_state = state; }
	void setOldLandingGearPosition(int8_t pos) { _old_landing_gear_position = pos; }
};

class FlightModeManagerTest : public ::testing::Test
{
public:
	void SetUp() override
	{
		param_control_autosave(false);
		publishValidEstimates();
		_manager.updateSubscriptions();
	}

	void publishValidEstimates()
	{
		hrt_abstime now = hrt_absolute_time();

		vehicle_local_position_s lpos{};
		lpos.timestamp = now;
		lpos.timestamp_sample = now;
		lpos.xy_valid = true;
		lpos.z_valid = true;
		lpos.v_xy_valid = true;
		lpos.v_z_valid = true;
		lpos.heading_good_for_control = true;
		lpos.heading = 0.0f;
		lpos.x = 0.0f;
		lpos.y = 0.0f;
		lpos.z = -5.0f;
		lpos.vx = 0.0f;
		lpos.vy = 0.0f;
		lpos.vz = 0.0f;
		lpos.dist_bottom = 5.0f;
		lpos.dist_bottom_valid = true;
		_vehicle_local_position_pub.publish(lpos);

		manual_control_setpoint_s manual{};
		manual.timestamp = now;
		manual.timestamp_sample = now;
		manual.valid = true;
		manual.roll = 0.0f;
		manual.pitch = 0.0f;
		manual.yaw = 0.0f;
		manual.throttle = 0.0f;
		manual.data_source = manual_control_setpoint_s::SOURCE_RC;
		_manual_control_setpoint_pub.publish(manual);

		home_position_s home{};
		home.timestamp = now;
		home.valid_alt = true;
		home.valid_hpos = true;
		home.valid_lpos = true;
		home.z = 0.0f;
		_home_position_pub.publish(home);

		vehicle_control_mode_s control_mode{};
		control_mode.timestamp = now;
		control_mode.flag_control_altitude_enabled = true;
		control_mode.flag_control_position_enabled = true;
		control_mode.flag_control_velocity_enabled = true;
		control_mode.flag_control_manual_enabled = true;
		control_mode.flag_armed = true;
		_vehicle_control_mode_pub.publish(control_mode);

		vehicle_status_s status{};
		status.timestamp = now;
		status.vehicle_type = vehicle_status_s::VEHICLE_TYPE_ROTARY_WING;
		status.nav_state = vehicle_status_s::NAVIGATION_STATE_POSCTL;
		_vehicle_status_pub.publish(status);
	}

	uORB::Publication<vehicle_status_s> _vehicle_status_pub{ORB_ID(vehicle_status)};
	uORB::Publication<vehicle_control_mode_s> _vehicle_control_mode_pub{ORB_ID(vehicle_control_mode)};
	uORB::Publication<vehicle_command_s> _vehicle_command_pub{ORB_ID(vehicle_command)};
	uORB::Publication<takeoff_status_s> _takeoff_status_pub{ORB_ID(takeoff_status)};
	uORB::Publication<vehicle_local_position_s> _vehicle_local_position_pub{ORB_ID(vehicle_local_position)};
	uORB::Publication<manual_control_setpoint_s> _manual_control_setpoint_pub{ORB_ID(manual_control_setpoint)};
	uORB::Publication<home_position_s> _home_position_pub{ORB_ID(home_position)};

	uORB::SubscriptionData<trajectory_setpoint_s> _trajectory_setpoint_sub{ORB_ID(trajectory_setpoint)};
	uORB::SubscriptionData<vehicle_constraints_s> _vehicle_constraints_sub{ORB_ID(vehicle_constraints)};
	uORB::SubscriptionData<landing_gear_s> _landing_gear_sub{ORB_ID(landing_gear)};

	TestFlightModeManager _manager;
};

TEST_F(FlightModeManagerTest, TaskSwitchingBasics)
{
	EXPECT_FALSE(_manager.isAnyTaskActive());
	EXPECT_EQ(_manager.getCurrentTaskIndex(), FlightTaskIndex::None);

	// Switch to None explicitly
	EXPECT_EQ(_manager.switchTask(FlightTaskIndex::None), FlightTaskError::NoError);
	EXPECT_FALSE(_manager.isAnyTaskActive());

	// Switch to None again (idempotent branch: new_task_index == _current_task.index)
	EXPECT_EQ(_manager.switchTask(FlightTaskIndex::None), FlightTaskError::NoError);
	EXPECT_FALSE(_manager.isAnyTaskActive());
}

TEST_F(FlightModeManagerTest, TaskSwitchingWithIntegerIndex)
{
	// Valid None index (-1)
	int none_idx = static_cast<int>(FlightTaskIndex::None);
	EXPECT_EQ(_manager.switchTask(none_idx), FlightTaskError::NoError);
	EXPECT_EQ(_manager.getCurrentTaskIndex(), FlightTaskIndex::None);

	// Out-of-bounds integer indices (< None or >= Count)
	EXPECT_EQ(_manager.switchTask(-2), FlightTaskError::InvalidTask);
	EXPECT_FALSE(_manager.isAnyTaskActive());

	EXPECT_EQ(_manager.switchTask(999), FlightTaskError::InvalidTask);
	EXPECT_FALSE(_manager.isAnyTaskActive());
}

TEST_F(FlightModeManagerTest, ErrorToStringMapping)
{
	EXPECT_STREQ(_manager.errorToString(FlightTaskError::NoError), "No Error");
	EXPECT_STREQ(_manager.errorToString(FlightTaskError::InvalidTask), "Invalid Task");
	EXPECT_STREQ(_manager.errorToString(FlightTaskError::ActivationFailed), "Activation Failed");
	EXPECT_STREQ(_manager.errorToString(static_cast<FlightTaskError>(99)), "This error is not mapped to a string or is unknown.");
}

TEST_F(FlightModeManagerTest, StartFlightTaskAutoModes)
{
	vehicle_control_mode_s vcm{};
	vcm.timestamp = hrt_absolute_time();
	vcm.flag_control_auto_enabled = true;
	_vehicle_control_mode_pub.publish(vcm);

	vehicle_status_s status{};
	status.timestamp = hrt_absolute_time();

	// AUTO_TAKEOFF
	status.nav_state = vehicle_status_s::NAVIGATION_STATE_AUTO_TAKEOFF;
	_vehicle_status_pub.publish(status);
	_manager.updateSubscriptions();
	_manager.start_flight_task();
	EXPECT_EQ(_manager.getCurrentTaskIndex(), FlightTaskIndex::Auto);

	// AUTO_LAND
	status.nav_state = vehicle_status_s::NAVIGATION_STATE_AUTO_LAND;
	_vehicle_status_pub.publish(status);
	_manager.updateSubscriptions();
	_manager.start_flight_task();
	EXPECT_EQ(_manager.getCurrentTaskIndex(), FlightTaskIndex::Auto);

	// AUTO_RTL
	status.nav_state = vehicle_status_s::NAVIGATION_STATE_AUTO_RTL;
	_vehicle_status_pub.publish(status);
	_manager.updateSubscriptions();
	_manager.start_flight_task();
	EXPECT_EQ(_manager.getCurrentTaskIndex(), FlightTaskIndex::Auto);

	// AUTO_MISSION
	status.nav_state = vehicle_status_s::NAVIGATION_STATE_AUTO_MISSION;
	_vehicle_status_pub.publish(status);
	_manager.updateSubscriptions();
	_manager.start_flight_task();
	EXPECT_EQ(_manager.getCurrentTaskIndex(), FlightTaskIndex::Auto);

#if !defined(CONSTRAINED_FLASH)
	// Disable auto mode to test follow target and orbit
	vcm.flag_control_auto_enabled = false;
	_vehicle_control_mode_pub.publish(vcm);

	// AUTO_FOLLOW_TARGET
	status.nav_state = vehicle_status_s::NAVIGATION_STATE_AUTO_FOLLOW_TARGET;
	_vehicle_status_pub.publish(status);
	_manager.updateSubscriptions();
	_manager.start_flight_task();
	EXPECT_EQ(_manager.getCurrentTaskIndex(), FlightTaskIndex::AutoFollowTarget);

	// ORBIT
	status.nav_state = vehicle_status_s::NAVIGATION_STATE_ORBIT;
	_vehicle_status_pub.publish(status);
	_manager.updateSubscriptions();
	_manager.start_flight_task();
	EXPECT_EQ(_manager.getCurrentTaskIndex(), FlightTaskIndex::Orbit);
#endif
}

TEST_F(FlightModeManagerTest, StartFlightTaskPositionControlModes)
{
	vehicle_status_s status{};
	status.timestamp = hrt_absolute_time();
	status.nav_state = vehicle_status_s::NAVIGATION_STATE_POSCTL;
	_vehicle_status_pub.publish(status);
	_manager.updateSubscriptions();

	// Case 0: ManualPosition
	int32_t pos_mode = 0;
	param_set(param_find("MPC_POS_MODE"), &pos_mode);
	_manager.updateParams();
	_manager.start_flight_task();
	EXPECT_EQ(_manager.getCurrentTaskIndex(), FlightTaskIndex::ManualPosition);

	// Case 4: ManualAcceleration
	pos_mode = 4;
	param_set(param_find("MPC_POS_MODE"), &pos_mode);
	_manager.updateParams();
	_manager.start_flight_task();
	EXPECT_EQ(_manager.getCurrentTaskIndex(), FlightTaskIndex::ManualAcceleration);

	// Default: resets invalid mode to 4 and selects ManualAcceleration
	pos_mode = 99;
	param_set(param_find("MPC_POS_MODE"), &pos_mode);
	_manager.updateParams();
	_manager.start_flight_task();
	EXPECT_EQ(_manager.getCurrentTaskIndex(), FlightTaskIndex::ManualAcceleration);
	param_get(param_find("MPC_POS_MODE"), &pos_mode);
	EXPECT_EQ(pos_mode, 4);
}

TEST_F(FlightModeManagerTest, StartFlightTaskAltitudeControlModes)
{
	vehicle_status_s status{};
	status.timestamp = hrt_absolute_time();
	status.nav_state = vehicle_status_s::NAVIGATION_STATE_ALTCTL;
	_vehicle_status_pub.publish(status);
	_manager.updateSubscriptions();

	// Case 0: ManualAltitude
	int32_t pos_mode = 0;
	param_set(param_find("MPC_POS_MODE"), &pos_mode);
	_manager.updateParams();
	_manager.start_flight_task();
	EXPECT_EQ(_manager.getCurrentTaskIndex(), FlightTaskIndex::ManualAltitude);

	// Case 3: ManualAltitudeSmoothVel
	pos_mode = 3;
	param_set(param_find("MPC_POS_MODE"), &pos_mode);
	_manager.updateParams();
	_manager.start_flight_task();
	EXPECT_EQ(_manager.getCurrentTaskIndex(), FlightTaskIndex::ManualAltitudeSmoothVel);

	// Default: ManualAltitudeSmoothVel
	pos_mode = 99;
	param_set(param_find("MPC_POS_MODE"), &pos_mode);
	_manager.updateParams();
	_manager.start_flight_task();
	EXPECT_EQ(_manager.getCurrentTaskIndex(), FlightTaskIndex::ManualAltitudeSmoothVel);
}

TEST_F(FlightModeManagerTest, StartFlightTaskAltitudeCruiseAndDescend)
{
	vehicle_status_s status{};
	status.timestamp = hrt_absolute_time();

	// Altitude Cruise
	status.nav_state = vehicle_status_s::NAVIGATION_STATE_ALTITUDE_CRUISE;
	_vehicle_status_pub.publish(status);
	_manager.updateSubscriptions();
	_manager.start_flight_task();
	EXPECT_EQ(_manager.getCurrentTaskIndex(), FlightTaskIndex::AltitudeCruise);

	// Descend
	status.nav_state = vehicle_status_s::NAVIGATION_STATE_DESCEND;
	_vehicle_status_pub.publish(status);
	_manager.updateSubscriptions();
	_manager.start_flight_task();
	EXPECT_EQ(_manager.getCurrentTaskIndex(), FlightTaskIndex::Descend);
}

TEST_F(FlightModeManagerTest, StartFlightTaskUnmatchedTaskFallback)
{
	vehicle_status_s status{};
	status.timestamp = hrt_absolute_time();

	// Fixed Wing mode -> no flight task
	status.vehicle_type = vehicle_status_s::VEHICLE_TYPE_FIXED_WING;
	_vehicle_status_pub.publish(status);
	_manager.updateSubscriptions();
	_manager.start_flight_task();
	EXPECT_EQ(_manager.getCurrentTaskIndex(), FlightTaskIndex::None);

	// External navigation state -> no flight task
	status.vehicle_type = vehicle_status_s::VEHICLE_TYPE_ROTARY_WING;
	status.nav_state = vehicle_status_s::NAVIGATION_STATE_EXTERNAL1;
	_vehicle_status_pub.publish(status);
	_manager.updateSubscriptions();
	_manager.start_flight_task();
	EXPECT_EQ(_manager.getCurrentTaskIndex(), FlightTaskIndex::None);
}

#include "tasks/Auto/FlightTaskAuto.hpp"

class TestFlightTaskAuto : public FlightTaskAuto {
public:
	float getCruiseSpeed() const { return _mc_cruise_speed; }
};

TEST_F(FlightModeManagerTest, HandleCommandOrbitAndSpeedChange)
{
	// Switch to auto task to check actual cruise speed change
	EXPECT_EQ(_manager.switchTask(FlightTaskIndex::Auto), FlightTaskError::NoError);
	EXPECT_TRUE(_manager.isAnyTaskActive());

	vehicle_command_s cmd{};
	cmd.timestamp = hrt_absolute_time();
	cmd.command = vehicle_command_s::VEHICLE_CMD_DO_ORBIT;
	cmd.param1 = 10.0f; // radius
	cmd.param2 = 2.0f;  // velocity
	_vehicle_command_pub.publish(cmd);

	_manager.handleCommand();
	EXPECT_EQ(_manager.getCommand().command, vehicle_command_s::VEHICLE_CMD_DO_ORBIT);

	// Change speed command
	cmd.command = vehicle_command_s::VEHICLE_CMD_DO_CHANGE_SPEED;
	cmd.param1 = vehicle_command_s::SPEED_TYPE_GROUNDSPEED;
	cmd.param2 = 5.0f; // speed
	_vehicle_command_pub.publish(cmd);

	_manager.handleCommand();
	EXPECT_TRUE(_manager.isAnyTaskActive());
	
	// Check the actual cruise-speed change
	auto task = static_cast<TestFlightTaskAuto*>(_manager.getCurrentTask());
	EXPECT_EQ(task->getCruiseSpeed(), 5.0f);
}

TEST_F(FlightModeManagerTest, TryApplyCommandTimeout)
{
	EXPECT_EQ(_manager.switchTask(FlightTaskIndex::Orbit), FlightTaskError::NoError);

	// Case 1: Old command > 200ms -> not applied
	vehicle_command_s cmd{};
	cmd.timestamp = hrt_absolute_time() - 1000000ULL; // 1s old
	cmd.command = vehicle_command_s::VEHICLE_CMD_DO_ORBIT;
	_manager.setCommand(cmd);

	_manager.tryApplyCommandIfAny();
	EXPECT_EQ(_manager.getCommand().command, vehicle_command_s::VEHICLE_CMD_DO_ORBIT);

	// Case 2: Fresh command <= 200ms -> applied and reset to 0
	cmd.timestamp = hrt_absolute_time();
	_manager.setCommand(cmd);
	_manager.tryApplyCommandIfAny();
	EXPECT_EQ(_manager.getCommand().command, 0);
}

TEST_F(FlightModeManagerTest, GenerateTrajectorySetpointAndLandingGear)
{
	EXPECT_EQ(_manager.switchTask(FlightTaskIndex::ManualAltitude), FlightTaskError::NoError);

	vehicle_local_position_s lpos{};
	lpos.timestamp = hrt_absolute_time();
	lpos.z = -10.0f;
	lpos.vz = 0.0f;

	// Landed state -> disarmed gear
	_manager.setTakeoffState(takeoff_status_s::TAKEOFF_STATE_DISARMED);
	_manager.generateTrajectorySetpoint(0.02f, lpos);

	EXPECT_TRUE(_trajectory_setpoint_sub.update());
	EXPECT_TRUE(_vehicle_constraints_sub.update());
	if (_landing_gear_sub.update()) {
		EXPECT_EQ(_landing_gear_sub.get().landing_gear, landing_gear_s::GEAR_DOWN);
	}

	// Flight state -> active setpoint generation
	_manager.setTakeoffState(takeoff_status_s::TAKEOFF_STATE_FLIGHT);
	_manager.generateTrajectorySetpoint(0.02f, lpos);

	EXPECT_TRUE(_trajectory_setpoint_sub.update());
	EXPECT_TRUE(_vehicle_constraints_sub.update());
}

TEST_F(FlightModeManagerTest, StatusAndUsageFunctions)
{
	EXPECT_EQ(_manager.print_status(), 0);

	EXPECT_EQ(_manager.switchTask(FlightTaskIndex::Orbit), FlightTaskError::NoError);
	EXPECT_EQ(_manager.print_status(), 0);

	EXPECT_EQ(TestFlightModeManager::print_usage(), 0);
	EXPECT_EQ(TestFlightModeManager::print_usage("Testing usage error string"), 0);

	char *args[] = { (char *)"flight_mode_manager", (char *)"invalid_subcmd" };
	EXPECT_EQ(TestFlightModeManager::custom_command(2, args), 0);
}