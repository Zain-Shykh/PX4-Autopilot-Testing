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

/**
 * @file LandDetectorTest.cpp
 * Student-authored GTest unit & functional test suite for MulticopterLandDetector.
 * Exercises Statement, Decision/Branch, and MC/DC Condition Independence pairs.
 */

#include <gtest/gtest.h>
#include "MulticopterLandDetector.h"

namespace land_detector
{

class MulticopterLandDetectorTest : public MulticopterLandDetector
{
public:
	MulticopterLandDetectorTest() : MulticopterLandDetector() {}
	~MulticopterLandDetectorTest() override = default;

	// Expose protected methods for direct unit testing
	bool test_get_ground_contact_state() { return _get_ground_contact_state(); }
	bool test_get_maybe_landed_state() { return _get_maybe_landed_state(); }
	bool test_get_freefall_state() { return _get_freefall_state(); }
	bool test_get_ground_effect_state() { return _get_ground_effect_state(); }
	bool test_get_landed_state() { return _get_landed_state(); }

	// Setters to control internal state variables for deterministic MC/DC testing
	void set_armed(bool armed) { _armed = armed; }
	void set_acceleration(const matrix::Vector3f &accel) { _acceleration = accel; }
	void set_angular_velocity(const matrix::Vector3f &angular_vel) { _angular_velocity = angular_vel; }
	void set_vehicle_thrust_setpoint_throttle(float throttle) { _vehicle_thrust_setpoint_throttle = throttle; }
	void set_horizontal_velocity(float vx, float vy)
	{
		_vehicle_local_position.v_xy_valid = true;
		_vehicle_local_position.vx = vx;
		_vehicle_local_position.vy = vy;
	}
	void set_vertical_velocity(float vz)
	{
		_vehicle_local_position.v_z_valid = true;
		_vehicle_local_position.vz = vz;
	}
	void set_distance_bottom(bool valid, float dist)
	{
		_dist_bottom_is_observable = true;
		_vehicle_local_position.dist_bottom_valid = valid;
		_vehicle_local_position.dist_bottom = dist;
	}
	void set_in_descend(bool in_descend) { _in_descend = in_descend; }
	void set_horizontal_movement(bool moving) { _horizontal_movement = moving; }
	void set_flag_control_climb_rate_enabled(bool enabled) { _flag_control_climb_rate_enabled = enabled; }
	void set_takeoff_state(uint8_t state) { _takeoff_state = state; }
	void set_below_gnd_effect_hgt(bool below) { _below_gnd_effect_hgt = below; }
	// Use timestamp=1 so that when _get_maybe_landed_state() later calls set_state_and_update(x, now),
	// the condition (now >= 1 + hysteresis_time) is guaranteed true for any reasonable hysteresis duration.
	// This ensures the forced hysteresis state is not reset by internal timing logic.
	void set_ground_contact_hysteresis_state(bool state) { _ground_contact_hysteresis.set_state_and_update(state, 1); }
	void set_maybe_landed_hysteresis_state(bool state) { _maybe_landed_hysteresis.set_state_and_update(state, 1); }
	void set_landed_hysteresis_state(bool state) { _landed_hysteresis.set_state_and_update(state, 1); }
	void set_freefall_hysteresis_state(bool state) { _freefall_hysteresis.set_state_and_update(state, 1); }
	void set_minimum_thrust_8s_hysteresis_state(bool state) { _minimum_thrust_8s_hysteresis.set_state_and_update(state, 1); }
	void set_local_position_timestamp(hrt_abstime time) { _vehicle_local_position.timestamp = time; }
	void set_v_z_valid(bool valid) { _vehicle_local_position.v_z_valid = valid; }
};

class LandDetectorFixture : public ::testing::Test
{
protected:
	MulticopterLandDetectorTest detector;

	void SetUp() override
	{
		detector.set_local_position_timestamp(hrt_absolute_time());
		detector.set_horizontal_velocity(0.0f, 0.0f);
		detector.set_vertical_velocity(0.0f);
		detector.set_angular_velocity(matrix::Vector3f(0.0f, 0.0f, 0.0f));
	}
};

/* ============================================================================
 * DECISION D1: Ground Contact State Logic (_get_ground_contact_state)
 * D1 = !_armed || (_close_to_ground_or_skipped_check && ground_contact && !_horizontal_movement && !_vertical_movement)
 * ============================================================================ */

TEST_F(LandDetectorFixture, GroundContactMCDC_ConditionA_Armed)
{
	// Pair: (A=True -> !_armed=True) vs (A=False -> !_armed=False)
	// TP_D1_A1: !_armed = true -> D1 must be True regardless of other conditions
	detector.set_armed(false);
	detector.set_horizontal_velocity(5.0f, 5.0f); // Movement present
	EXPECT_TRUE(detector.test_get_ground_contact_state());

	// TP_D1_A2: !_armed = false, and movement present -> D1 must be False
	detector.set_armed(true);
	EXPECT_FALSE(detector.test_get_ground_contact_state());
}

TEST_F(LandDetectorFixture, GroundContactMCDC_ConditionB_CloseToGround)
{
	// Pair: B=True vs B=False while A=F, C=T, D=T, E=T
	detector.set_armed(true);
	detector.set_vehicle_thrust_setpoint_throttle(0.0f);
	detector.set_horizontal_velocity(0.0f, 0.0f);
	detector.set_vertical_velocity(0.0f);

	// TP_D1_B1: B=True (Distance to ground < 1.0m threshold) -> D1 = True
	detector.set_distance_bottom(true, 0.5f);
	EXPECT_TRUE(detector.test_get_ground_contact_state());

	// TP_D1_B2: B=False (Distance to ground > 1.0m threshold, e.g. 10m) -> D1 = False
	detector.set_distance_bottom(true, 10.0f);
	EXPECT_FALSE(detector.test_get_ground_contact_state());
}

TEST_F(LandDetectorFixture, GroundContactMCDC_ConditionC_LowThrottle)
{
	// Pair: C=True vs C=False while A=F, B=T, D=T, E=T
	detector.set_armed(true);
	detector.set_horizontal_velocity(0.0f, 0.0f);
	detector.set_vertical_velocity(0.0f);

	// TP_D1_C1: C=True (Low throttle setpoint = 0.0) -> D1 = True
	detector.set_vehicle_thrust_setpoint_throttle(0.0f);
	EXPECT_TRUE(detector.test_get_ground_contact_state());

	// TP_D1_C2: C=False (High throttle setpoint = 1.0) -> D1 = False
	detector.set_vehicle_thrust_setpoint_throttle(1.0f);
	EXPECT_FALSE(detector.test_get_ground_contact_state());
}

TEST_F(LandDetectorFixture, GroundContactMCDC_ConditionD_HorizontalMovement)
{
	// Pair: D=True (!horizontal_movement) vs D=False (horizontal_movement present)
	detector.set_armed(true);
	detector.set_vehicle_thrust_setpoint_throttle(0.0f);
	detector.set_vertical_velocity(0.0f);

	// TP_D1_D1: D=True (Vx = 0.0, Vy = 0.0 -> no horizontal movement) -> D1 = True
	detector.set_horizontal_velocity(0.0f, 0.0f);
	EXPECT_TRUE(detector.test_get_ground_contact_state());

	// TP_D1_D2: D=False (Vx = 5.0m/s -> horizontal movement present) -> D1 = False
	detector.set_horizontal_velocity(5.0f, 0.0f);
	EXPECT_FALSE(detector.test_get_ground_contact_state());
}

TEST_F(LandDetectorFixture, GroundContactMCDC_ConditionE_VerticalMovement)
{
	// Pair: E=True (!vertical_movement) vs E=False (vertical_movement present)
	detector.set_armed(true);
	detector.set_vehicle_thrust_setpoint_throttle(0.0f);
	detector.set_horizontal_velocity(0.0f, 0.0f);

	// TP_D1_E1: E=True (Vz = 0.0 -> no vertical movement) -> D1 = True
	detector.set_vertical_velocity(0.0f);
	EXPECT_TRUE(detector.test_get_ground_contact_state());

	// TP_D1_E2: E=False (Vz = 5.0m/s -> vertical movement present) -> D1 = False
	detector.set_vertical_velocity(5.0f);
	EXPECT_FALSE(detector.test_get_ground_contact_state());
}

/* ============================================================================
 * DECISION D2: Maybe Landed State Logic (_get_maybe_landed_state)
 * D2 = !_armed || (minimum_thrust_now && !_freefall_hysteresis && !_rotational_movement
 *                  && ((vertical_estimate && _ground_contact_hysteresis) || (!vertical_estimate && _minimum_thrust_8s)))
 * ============================================================================ */

TEST_F(LandDetectorFixture, MaybeLandedMCDC_ConditionA_Armed)
{
	// TP_D2_A1: !_armed = true -> D2 = True
	detector.set_armed(false);
	detector.set_vehicle_thrust_setpoint_throttle(1.0f);
	EXPECT_TRUE(detector.test_get_maybe_landed_state());

	// TP_D2_A2: !_armed = false, and remaining conditions false -> D2 = False
	detector.set_armed(true);
	detector.set_vehicle_thrust_setpoint_throttle(1.0f);
	EXPECT_FALSE(detector.test_get_maybe_landed_state());
}

TEST_F(LandDetectorFixture, MaybeLandedMCDC_ConditionB_MinThrust)
{
	detector.set_armed(true);
	detector.set_freefall_hysteresis_state(false);
	detector.set_angular_velocity(matrix::Vector3f(0.0f, 0.0f, 0.0f));
	detector.set_ground_contact_hysteresis_state(true);

	// TP_D2_B1: B=True (min throttle setpoint = 0.0) -> D2 = True
	detector.set_vehicle_thrust_setpoint_throttle(0.0f);
	EXPECT_TRUE(detector.test_get_maybe_landed_state());

	// TP_D2_B2: B=False (high throttle setpoint = 1.0) -> D2 = False
	detector.set_vehicle_thrust_setpoint_throttle(1.0f);
	EXPECT_FALSE(detector.test_get_maybe_landed_state());
}

TEST_F(LandDetectorFixture, MaybeLandedMCDC_ConditionC_NotFreefall)
{
	detector.set_armed(true);
	detector.set_vehicle_thrust_setpoint_throttle(0.0f);
	detector.set_angular_velocity(matrix::Vector3f(0.0f, 0.0f, 0.0f));
	detector.set_ground_contact_hysteresis_state(true);

	// TP_D2_C1: C=True (!freefall) -> D2 = True
	detector.set_freefall_hysteresis_state(false);
	EXPECT_TRUE(detector.test_get_maybe_landed_state());

	// TP_D2_C2: C=False (freefall active) -> D2 = False
	detector.set_freefall_hysteresis_state(true);
	EXPECT_FALSE(detector.test_get_maybe_landed_state());
}

TEST_F(LandDetectorFixture, MaybeLandedMCDC_ConditionE_VerticalEstimate)
{
	// Pair: TP_D2_E1 vs TP_D2_E2
	// Independence proof: flip E (vertical_estimate) while A=F, B=T, C=T, D=T, F=F, G=T
	// When E=False(no vertical estimate), path is (!E && G) -> with G=True -> D2 = True
	// When E=True (valid estimate), path is  (E && F)  -> with F=False -> D2 = False
	detector.set_armed(true);
	detector.set_vehicle_thrust_setpoint_throttle(0.0f);   // B=True
	detector.set_freefall_hysteresis_state(false);         // C=True  (!freefall)
	detector.set_angular_velocity(matrix::Vector3f(0.0f, 0.0f, 0.0f)); // D=True (!rotation)
	detector.set_ground_contact_hysteresis_state(false);   // F=False
	detector.set_minimum_thrust_8s_hysteresis_state(true); // G=True

	// TP_D2_E1: E=False (stale timestamp -> vertical_estimate=false), G=True -> D2 = True
	detector.set_local_position_timestamp(0); // Make timestamp stale (> 1s old)
	detector.set_v_z_valid(true);
	EXPECT_TRUE(detector.test_get_maybe_landed_state());

	// TP_D2_E2: E=True (fresh timestamp + v_z_valid=true), F=False -> (!E&&G) arm broken, (E&&F)=F -> D2 = False
	detector.set_local_position_timestamp(hrt_absolute_time());
	detector.set_v_z_valid(true);
	EXPECT_FALSE(detector.test_get_maybe_landed_state());
}

TEST_F(LandDetectorFixture, MaybeLandedMCDC_ConditionF_GroundContactHysteresis)
{
	// Pair: TP_D2_F1 vs TP_D2_F2
	// Independence proof: flip F (_ground_contact_hysteresis) while A=F, B=T, C=T, D=T, E=T, G=F
	// With E=True, path is (E&&F): flipping F flips D2.
	detector.set_armed(true);
	detector.set_vehicle_thrust_setpoint_throttle(0.0f);   // B=True
	detector.set_freefall_hysteresis_state(false);         // C=True  (!freefall)
	detector.set_angular_velocity(matrix::Vector3f(0.0f, 0.0f, 0.0f)); // D=True
	detector.set_local_position_timestamp(hrt_absolute_time()); // E=True (local_position_updated)
	detector.set_v_z_valid(true);                          // E=True (v_z_valid)
	detector.set_minimum_thrust_8s_hysteresis_state(false); // G=False (not relevant when E=True)

	// TP_D2_F1: F=True (_ground_contact_hysteresis active) -> (E&&F) = True -> D2 = True
	detector.set_ground_contact_hysteresis_state(true);
	EXPECT_TRUE(detector.test_get_maybe_landed_state());

	// TP_D2_F2: F=False (_ground_contact_hysteresis inactive) -> (E&&F) = False AND (!E&&G)=False -> D2 = False
	detector.set_ground_contact_hysteresis_state(false);
	EXPECT_FALSE(detector.test_get_maybe_landed_state());
}

TEST_F(LandDetectorFixture, MaybeLandedMCDC_ConditionG_MinThrust8sHysteresis)
{
	// Pair: TP_D2_G1 vs TP_D2_G2
	// Independence proof: flip G (_minimum_thrust_8s_hysteresis) while A=F, B=T, C=T, D=T, E=F, F=F
	// With E=False (stale timestamp), path is (!E&&G): flipping G flips D2.
	detector.set_armed(true);
	detector.set_vehicle_thrust_setpoint_throttle(0.0f);   // B=True
	detector.set_freefall_hysteresis_state(false);         // C=True (!freefall)
	detector.set_angular_velocity(matrix::Vector3f(0.0f, 0.0f, 0.0f)); // D=True
	detector.set_local_position_timestamp(0); // E=False (stale timestamp)
	detector.set_v_z_valid(true);
	detector.set_ground_contact_hysteresis_state(false);   // F=False

	// TP_D2_G1: G=True (8s low-thrust hysteresis active) -> (!E&&G) = True -> D2 = True
	detector.set_minimum_thrust_8s_hysteresis_state(true);
	EXPECT_TRUE(detector.test_get_maybe_landed_state());

	// TP_D2_G2: G=False (8s low-thrust hysteresis inactive) -> (!E&&G)=False AND (E&&F)=False -> D2 = False
	detector.set_minimum_thrust_8s_hysteresis_state(false);
	EXPECT_FALSE(detector.test_get_maybe_landed_state());
}

TEST_F(LandDetectorFixture, MaybeLandedMCDC_ConditionD_NotRotating)
{
	detector.set_armed(true);
	detector.set_vehicle_thrust_setpoint_throttle(0.0f);
	detector.set_freefall_hysteresis_state(false);
	detector.set_ground_contact_hysteresis_state(true);

	// TP_D2_D1: D=True (angular velocity = 0.0 -> no rotational movement) -> D2 = True
	detector.set_angular_velocity(matrix::Vector3f(0.0f, 0.0f, 0.0f));
	EXPECT_TRUE(detector.test_get_maybe_landed_state());

	// TP_D2_D2: D=False (angular velocity = 5.0 rad/s -> rotational movement present) -> D2 = False
	detector.set_angular_velocity(matrix::Vector3f(5.0f, 5.0f, 0.0f));
	EXPECT_FALSE(detector.test_get_maybe_landed_state());
}

/* ============================================================================
 * DECISION D3: Ground Effect State Logic (_get_ground_effect_state)
 * D3 = (_in_descend && !_horizontal_movement) || (_below_gnd_effect_hgt && TAKEOFF_STATE_FLIGHT) || TAKEOFF_STATE_RAMPUP
 * ============================================================================ */

TEST_F(LandDetectorFixture, GroundEffectMCDC_ConditionA_InDescend)
{
	detector.set_horizontal_velocity(0.0f, 0.0f); // B=True
	detector.set_below_gnd_effect_hgt(false);     // C=False
	detector.set_takeoff_state(takeoff_status_s::TAKEOFF_STATE_DISARMED); // D=F, E=F

	// TP_D3_A1: A=True (_in_descend) -> D3 = True
	detector.set_in_descend(true);
	EXPECT_TRUE(detector.test_get_ground_effect_state());

	// TP_D3_A2: A=False (!_in_descend) -> D3 = False
	detector.set_in_descend(false);
	EXPECT_FALSE(detector.test_get_ground_effect_state());
}

TEST_F(LandDetectorFixture, GroundEffectMCDC_ConditionB_NoHorizontalMovement)
{
	detector.set_in_descend(true);            // A=True
	detector.set_below_gnd_effect_hgt(false); // C=False
	detector.set_takeoff_state(takeoff_status_s::TAKEOFF_STATE_DISARMED);

	// TP_D3_B1: B=True (Vx=0, Vy=0 -> no horizontal movement) -> D3 = True
	detector.set_horizontal_movement(false);
	EXPECT_TRUE(detector.test_get_ground_effect_state());

	// TP_D3_B2: B=False (Vx=5m/s -> horizontal movement) -> D3 = False
	detector.set_horizontal_movement(true);
	EXPECT_FALSE(detector.test_get_ground_effect_state());
}

TEST_F(LandDetectorFixture, GroundEffectMCDC_ConditionC_BelowGndHeight)
{
	detector.set_in_descend(false);           // A=False
	detector.set_horizontal_velocity(5.0f, 0.0f); // B=False
	detector.set_takeoff_state(takeoff_status_s::TAKEOFF_STATE_FLIGHT); // D=True, E=False

	// TP_D3_C1: C=True (below ground effect height) -> D3 = True
	detector.set_below_gnd_effect_hgt(true);
	EXPECT_TRUE(detector.test_get_ground_effect_state());

	// TP_D3_C2: C=False (above ground effect height) -> D3 = False
	detector.set_below_gnd_effect_hgt(false);
	EXPECT_FALSE(detector.test_get_ground_effect_state());
}

TEST_F(LandDetectorFixture, GroundEffectMCDC_ConditionD_TakeoffStateFlight)
{
	// Pair: TP_D3_D1 vs TP_D3_D2
	// Independence proof: flip D (TAKEOFF_STATE_FLIGHT) while A=F, B=any, C=T, E=F
	// With C=True (_below_gnd_effect_hgt) and E=False (not RAMPUP), flipping D flips D3.
	detector.set_in_descend(false);           // A=False
	detector.set_below_gnd_effect_hgt(true);  // C=True
	detector.set_takeoff_state(takeoff_status_s::TAKEOFF_STATE_DISARMED); // baseline: E=False

	// TP_D3_D1: D=True (TAKEOFF_STATE_FLIGHT) -> (C&&D) = True -> D3 = True
	detector.set_takeoff_state(takeoff_status_s::TAKEOFF_STATE_FLIGHT);
	EXPECT_TRUE(detector.test_get_ground_effect_state());

	// TP_D3_D2: D=False (TAKEOFF_STATE_DISARMED, not FLIGHT not RAMPUP) -> (C&&D)=False AND E=False -> D3 = False
	detector.set_takeoff_state(takeoff_status_s::TAKEOFF_STATE_DISARMED);
	EXPECT_FALSE(detector.test_get_ground_effect_state());
}

TEST_F(LandDetectorFixture, GroundEffectMCDC_ConditionE_TakeoffRampup)
{
	detector.set_in_descend(false);
	detector.set_horizontal_velocity(5.0f, 0.0f);
	detector.set_below_gnd_effect_hgt(false);

	// TP_D3_E1: E=True (TAKEOFF_STATE_RAMPUP) -> D3 = True
	detector.set_takeoff_state(takeoff_status_s::TAKEOFF_STATE_RAMPUP);
	EXPECT_TRUE(detector.test_get_ground_effect_state());

	// TP_D3_E2: E=False (TAKEOFF_STATE_DISARMED) -> D3 = False
	detector.set_takeoff_state(takeoff_status_s::TAKEOFF_STATE_DISARMED);
	EXPECT_FALSE(detector.test_get_ground_effect_state());
}

/* ============================================================================
 * FREEFALL AND LANDED STATE TESTS
 * ============================================================================ */

TEST_F(LandDetectorFixture, Freefall_AccelThresholdBoundary)
{
	// Norm < 2.0 m/s^2 triggers freefall
	detector.set_acceleration(matrix::Vector3f(1.0f, 0.0f, 1.0f)); // Norm = 1.414 < 2.0
	EXPECT_TRUE(detector.test_get_freefall_state());

	// Norm >= 2.0 m/s^2 (e.g. 9.8 m/s^2 gravity) does NOT trigger freefall
	detector.set_acceleration(matrix::Vector3f(0.0f, 0.0f, 9.81f)); // Norm = 9.81 >= 2.0
	EXPECT_FALSE(detector.test_get_freefall_state());
}

TEST_F(LandDetectorFixture, Landed_StateTransitions)
{
	// Unarmed vehicle is always considered landed
	detector.set_armed(false);
	detector.set_maybe_landed_hysteresis_state(false);
	EXPECT_TRUE(detector.test_get_landed_state());

	// Armed vehicle requires maybe_landed_hysteresis = true
	detector.set_armed(true);
	detector.set_maybe_landed_hysteresis_state(true);
	EXPECT_TRUE(detector.test_get_landed_state());

	detector.set_maybe_landed_hysteresis_state(false);
	EXPECT_FALSE(detector.test_get_landed_state());
}

} // namespace land_detector
