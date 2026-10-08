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
#include <array>
#include <string>
#include "MulticopterLandDetector.h"

// This clock substitution is linked only into functional-LandDetector.
static hrt_abstime land_detector_test_now = 20_s;
extern "C" hrt_abstime __wrap_hrt_absolute_time() { return land_detector_test_now; }

namespace land_detector
{

class MulticopterLandDetectorTest : public MulticopterLandDetector
{
public:
	float test_get_minManThrottle() { return _params.minManThrottle; }
	MulticopterLandDetectorTest() : MulticopterLandDetector() {}
	~MulticopterLandDetectorTest() override = default;

	// Expose protected methods for direct unit testing
	bool test_get_ground_contact_state() { return _get_ground_contact_state(); }
	bool test_get_maybe_landed_state() { return _get_maybe_landed_state(); }
	float test_get_minManThrottle() { return _params.minManThrottle; }
	bool test_get_freefall_state() { return _get_freefall_state(); }
	bool test_get_ground_effect_state() { return _get_ground_effect_state(); }
	bool test_get_landed_state() { return _get_landed_state(); }

	bool test_get_vertical_movement() { return _vertical_movement; }
	bool test_get_horizontal_movement() { return _horizontal_movement; }
	bool test_get_below_gnd_effect_hgt() { return _below_gnd_effect_hgt; }
	bool test_get_close_to_ground_or_skipped_check() { return _close_to_ground_or_skipped_check; }
	bool test_get_hover_thrust_estimate_valid() { return _hover_thrust_estimate_valid; }
	bool test_get_in_descend() { return _in_descend; }
	bool test_get_has_low_throttle() { return _has_low_throttle; }
	bool test_get_rotational_movement() { return _rotational_movement; }

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
		_vehicle_local_position.z_valid = false;
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
	void set_ground_contact_hysteresis_state(bool state) { _ground_contact_hysteresis.set_state_and_update(state, land_detector_test_now); }
	void set_maybe_landed_hysteresis_state(bool state) { _maybe_landed_hysteresis.set_state_and_update(state, land_detector_test_now); }
	void set_landed_hysteresis_state(bool state) { _landed_hysteresis.set_state_and_update(state, land_detector_test_now); }
	void set_freefall_hysteresis_state(bool state) { _freefall_hysteresis.set_state_and_update(state, land_detector_test_now); }
	void set_minimum_thrust_8s_hysteresis_state(bool state)
	{
		// Exercise the real 8-second delay using controlled time, without sleeping.
		_minimum_thrust_8s_hysteresis.set_state_and_update(state, state ? land_detector_test_now - 8_s : land_detector_test_now);
		_minimum_thrust_8s_hysteresis.update(land_detector_test_now);
		ASSERT_EQ(_minimum_thrust_8s_hysteresis.get_state(), state);
	}
	void set_local_position_timestamp(hrt_abstime time) { _vehicle_local_position.timestamp = time; }
	void set_v_z_valid(bool valid) { _vehicle_local_position.v_z_valid = valid; }


	void set_hover_thrust_estimate_last_valid(hrt_abstime t) { _hover_thrust_estimate_last_valid = t; }
	uORB::Publication<trajectory_setpoint_s> _traj_pub{ORB_ID(trajectory_setpoint)};
	void publish_trajectory_setpoint(float vz) {
		trajectory_setpoint_s sp{};
		sp.velocity[0] = NAN;
		sp.velocity[1] = NAN;
		sp.velocity[2] = vz;
		sp.timestamp = hrt_absolute_time();
		_trajectory_setpoint_pub.publish(sp);
		sp.timestamp = hrt_absolute_time() + 1;
		_trajectory_setpoint_pub.publish(sp);
	}
	void set_dist_bottom_is_observable(bool observable) { _dist_bottom_is_observable = observable; }

	void set_z_derivative(bool valid, float deriv) {
		_vehicle_local_position.z_valid = valid;
		_vehicle_local_position.z_deriv = deriv;
	}
	void set_v_xy_valid(bool valid) {
		_vehicle_local_position.v_xy_valid = valid;
	}
	void set_hover_thrust_estimate_valid(bool valid) {
		_hover_thrust_estimate_valid = valid;
	}
	void set_trajectory_setpoint_vz(float vz) {
		_flag_control_climb_rate_enabled = true; // To enter the block
		// Note: We'd need to publish or somehow set _trajectory_setpoint_sub.
		// Since we cannot easily publish without setting up uORB, maybe we can mock it?
		// Wait, MulticopterLandDetector checks _trajectory_setpoint_sub.update(&trajectory_setpoint).
		// We might need to just rely on _in_descend setter for other things, but for "commanded descent"
		// we might need to publish.
	}


	void configure_thresholds()
	{
		_params.minThrottle = 0.1f;
		_params.hoverThrottle = 0.5f;
		_params.minManThrottle = 0.08f;
		_params.landSpeed = 0.0f;
		_params.crawlSpeed = 0.0f;
		_param_lndmc_z_vel_max.set(0.5f);
		_param_lndmc_xy_vel_max.set(1.5f);
		_param_lndmc_rot_max.set(20.f);
		_param_lndmc_alt_gnd_effect.set(2.f);
	}

	std::array<bool, 5> ground_contact_conditions() const
	{
		// For these D1 pairs, manual-thrust mode makes local ground_contact equal _has_low_throttle.
		EXPECT_FALSE(_flag_control_climb_rate_enabled);
		return {!_armed, _close_to_ground_or_skipped_check, _has_low_throttle, !_horizontal_movement, !_vertical_movement};
	}

	std::array<bool, 7> maybe_landed_conditions() const
	{
		EXPECT_FALSE(_flag_control_climb_rate_enabled);
		const bool minimum_thrust = _vehicle_thrust_setpoint_throttle <= _params.minManThrottle + 0.01f;
		const bool vertical_estimate = (land_detector_test_now - _vehicle_local_position.timestamp) < 1_s
					       && _vehicle_local_position.v_z_valid;
		return {!_armed, minimum_thrust, !_freefall_hysteresis.get_state(), !_rotational_movement,
			vertical_estimate, _ground_contact_hysteresis.get_state(), _minimum_thrust_8s_hysteresis.get_state()};
	}

	std::array<bool, 5> ground_effect_conditions() const
	{
		return {_in_descend, !_horizontal_movement, _below_gnd_effect_hgt,
			_takeoff_state == takeoff_status_s::TAKEOFF_STATE_FLIGHT,
			_takeoff_state == takeoff_status_s::TAKEOFF_STATE_RAMPUP};
	}

	uORB::Publication<trajectory_setpoint_s> _trajectory_setpoint_pub{ORB_ID(trajectory_setpoint)};
};

class LandDetectorFixture : public ::testing::Test
{
protected:
	MulticopterLandDetectorTest detector;

	void SetUp() override
	{
		land_detector_test_now = 20_s;
		param_control_autosave(false);
		detector.configure_thresholds();
		detector.set_local_position_timestamp(hrt_absolute_time());
		detector.set_horizontal_velocity(0.0f, 0.0f);
		detector.set_vertical_velocity(0.0f);
		detector.set_angular_velocity(matrix::Vector3f(0.0f, 0.0f, 0.0f));
	}

	template<size_t N>
	void record_vector(const char *id, const std::array<bool, N> &actual, const std::array<bool, N> &expected,
			   bool outcome, bool expected_outcome)
	{
		SCOPED_TRACE(id);
		EXPECT_EQ(actual, expected);
		EXPECT_EQ(outcome, expected_outcome);
		std::string value = "[";

		for (size_t i = 0; i < N; ++i) {
			if (i > 0) { value += ' '; }

			value += actual[i] ? 'T' : 'F';
		}

		value += outcome ? "] -> True" : "] -> False";
		RecordProperty(id, value);
	}

	void check_ground_contact(const char *id, const std::array<bool, 5> &expected, bool expected_outcome)
	{
		const bool outcome = detector.test_get_ground_contact_state();
		record_vector(id, detector.ground_contact_conditions(), expected, outcome, expected_outcome);
	}

	void check_maybe_landed(const char *id, const std::array<bool, 7> &expected, bool expected_outcome)
	{
		const bool outcome = detector.test_get_maybe_landed_state();
		record_vector(id, detector.maybe_landed_conditions(), expected, outcome, expected_outcome);
	}

	void check_ground_effect(const char *id, const std::array<bool, 5> &expected, bool expected_outcome)
	{
		const bool outcome = detector.test_get_ground_effect_state();
		record_vector(id, detector.ground_effect_conditions(), expected, outcome, expected_outcome);
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
	detector.set_horizontal_velocity(5.0f, 5.0f); // D=False
	detector.set_vertical_velocity(5.0f); // E=False
	detector.set_distance_bottom(true, 10.0f); // B=False
	detector.set_vehicle_thrust_setpoint_throttle(1.0f); // C=False
	check_ground_contact("TP_D1_A1", {true, false, false, false, false}, true);

	// TP_D1_A2: !_armed = false, and movement present -> D1 must be False
	detector.set_armed(true);
	check_ground_contact("TP_D1_A2", {false, false, false, false, false}, false);
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
	check_ground_contact("TP_D1_B1", {false, true, true, true, true}, true);

	// TP_D1_B2: B=False (Distance to ground > 1.0m threshold, e.g. 10m) -> D1 = False
	detector.set_distance_bottom(true, 10.0f);
	check_ground_contact("TP_D1_B2", {false, false, true, true, true}, false);
}

TEST_F(LandDetectorFixture, GroundContactMCDC_ConditionC_LowThrottle)
{
	// Pair: C=True vs C=False while A=F, B=T, D=T, E=T
	detector.set_armed(true);
	detector.set_horizontal_velocity(0.0f, 0.0f);
	detector.set_vertical_velocity(0.0f);

	// TP_D1_C1: C=True (Low throttle setpoint = 0.0) -> D1 = True
	detector.set_vehicle_thrust_setpoint_throttle(0.0f);
	check_ground_contact("TP_D1_C1", {false, true, true, true, true}, true);

	// TP_D1_C2: C=False (High throttle setpoint = 1.0) -> D1 = False
	detector.set_vehicle_thrust_setpoint_throttle(1.0f);
	check_ground_contact("TP_D1_C2", {false, true, false, true, true}, false);
}

TEST_F(LandDetectorFixture, GroundContactMCDC_ConditionD_HorizontalMovement)
{
	// Pair: D=True (!horizontal_movement) vs D=False (horizontal_movement present)
	detector.set_armed(true);
	detector.set_vehicle_thrust_setpoint_throttle(0.0f);
	detector.set_vertical_velocity(0.0f);

	// TP_D1_D1: D=True (Vx = 0.0, Vy = 0.0 -> no horizontal movement) -> D1 = True
	detector.set_horizontal_velocity(0.0f, 0.0f);
	check_ground_contact("TP_D1_D1", {false, true, true, true, true}, true);

	// TP_D1_D2: D=False (Vx = 5.0m/s -> horizontal movement present) -> D1 = False
	detector.set_horizontal_velocity(5.0f, 0.0f);
	check_ground_contact("TP_D1_D2", {false, true, true, false, true}, false);
}

TEST_F(LandDetectorFixture, GroundContactMCDC_ConditionE_VerticalMovement)
{
	// Pair: E=True (!vertical_movement) vs E=False (vertical_movement present)
	detector.set_armed(true);
	detector.set_vehicle_thrust_setpoint_throttle(0.0f);
	detector.set_horizontal_velocity(0.0f, 0.0f);

	// TP_D1_E1: E=True (Vz = 0.0 -> no vertical movement) -> D1 = True
	detector.set_vertical_velocity(0.0f);
	check_ground_contact("TP_D1_E1", {false, true, true, true, true}, true);

	// TP_D1_E2: E=False (Vz = 5.0m/s -> vertical movement present) -> D1 = False
	detector.set_vertical_velocity(5.0f);
	check_ground_contact("TP_D1_E2", {false, true, true, true, false}, false);
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
	detector.set_freefall_hysteresis_state(true);
	detector.set_angular_velocity(matrix::Vector3f(5.0f, 5.0f, 0.0f));
	detector.set_local_position_timestamp(land_detector_test_now - 1_s);
	detector.set_ground_contact_hysteresis_state(false);
	detector.set_vehicle_thrust_setpoint_throttle(1.0f);
	check_maybe_landed("TP_D2_A1", {true, false, false, false, false, false, false}, true);

	// TP_D2_A2: !_armed = false, and remaining conditions false -> D2 = False
	detector.set_armed(true);
	detector.set_vehicle_thrust_setpoint_throttle(1.0f);
	check_maybe_landed("TP_D2_A2", {false, false, false, false, false, false, false}, false);
}

TEST_F(LandDetectorFixture, MaybeLandedMCDC_ConditionB_MinThrust)
{
	detector.set_armed(true);
	detector.set_freefall_hysteresis_state(false);
	detector.set_angular_velocity(matrix::Vector3f(0.0f, 0.0f, 0.0f));
	detector.set_ground_contact_hysteresis_state(true);

	// TP_D2_B1: B=True (min throttle setpoint = 0.0) -> D2 = True
	detector.set_vehicle_thrust_setpoint_throttle(0.0f);
	check_maybe_landed("TP_D2_B1", {false, true, true, true, true, true, false}, true);

	// TP_D2_B2: B=False (high throttle setpoint = 1.0) -> D2 = False
	detector.set_vehicle_thrust_setpoint_throttle(1.0f);
	check_maybe_landed("TP_D2_B2", {false, false, true, true, true, true, false}, false);
}

TEST_F(LandDetectorFixture, MaybeLandedMCDC_ConditionC_NotFreefall)
{
	detector.set_armed(true);
	detector.set_vehicle_thrust_setpoint_throttle(0.0f);
	detector.set_angular_velocity(matrix::Vector3f(0.0f, 0.0f, 0.0f));
	detector.set_ground_contact_hysteresis_state(true);

	// TP_D2_C1: C=True (!freefall) -> D2 = True
	detector.set_freefall_hysteresis_state(false);
	check_maybe_landed("TP_D2_C1", {false, true, true, true, true, true, false}, true);

	// TP_D2_C2: C=False (freefall active) -> D2 = False
	detector.set_freefall_hysteresis_state(true);
	check_maybe_landed("TP_D2_C2", {false, true, false, true, true, true, false}, false);
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
	detector.set_local_position_timestamp(land_detector_test_now - 1_s); // Exactly stale
	detector.set_v_z_valid(true);
	check_maybe_landed("TP_D2_E1", {false, true, true, true, false, false, true}, true);

	// TP_D2_E2: E=True (fresh timestamp + v_z_valid=true), F=False -> (!E&&G) arm broken, (E&&F)=F -> D2 = False
	detector.set_local_position_timestamp(hrt_absolute_time());
	detector.set_v_z_valid(true);
	check_maybe_landed("TP_D2_E2", {false, true, true, true, true, false, true}, false);
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
	check_maybe_landed("TP_D2_F1", {false, true, true, true, true, true, false}, true);

	// TP_D2_F2: F=False (_ground_contact_hysteresis inactive) -> (E&&F) = False AND (!E&&G)=False -> D2 = False
	detector.set_ground_contact_hysteresis_state(false);
	check_maybe_landed("TP_D2_F2", {false, true, true, true, true, false, false}, false);
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
	detector.set_local_position_timestamp(land_detector_test_now - 1_s); // E=False (exact timeout)
	detector.set_v_z_valid(true);
	detector.set_ground_contact_hysteresis_state(false);   // F=False

	// TP_D2_G1: G=True (8s low-thrust hysteresis active) -> (!E&&G) = True -> D2 = True
	detector.set_minimum_thrust_8s_hysteresis_state(true);
	check_maybe_landed("TP_D2_G1", {false, true, true, true, false, false, true}, true);

	// TP_D2_G2: G=False (8s low-thrust hysteresis inactive) -> (!E&&G)=False AND (E&&F)=False -> D2 = False
	detector.set_minimum_thrust_8s_hysteresis_state(false);
	check_maybe_landed("TP_D2_G2", {false, true, true, true, false, false, false}, false);
}

TEST_F(LandDetectorFixture, MaybeLandedMCDC_ConditionD_NotRotating)
{
	detector.set_armed(true);
	detector.set_vehicle_thrust_setpoint_throttle(0.0f);
	detector.set_freefall_hysteresis_state(false);
	detector.set_ground_contact_hysteresis_state(true);

	// TP_D2_D1: D=True (angular velocity = 0.0 -> no rotational movement) -> D2 = True
	detector.set_angular_velocity(matrix::Vector3f(0.0f, 0.0f, 0.0f));
	check_maybe_landed("TP_D2_D1", {false, true, true, true, true, true, false}, true);

	// TP_D2_D2: D=False (angular velocity = 5.0 rad/s -> rotational movement present) -> D2 = False
	detector.set_angular_velocity(matrix::Vector3f(5.0f, 5.0f, 0.0f));
	check_maybe_landed("TP_D2_D2", {false, true, true, false, true, true, false}, false);
}

/* ============================================================================
 * DECISION D3: Ground Effect State Logic (_get_ground_effect_state)
 * D3 = (_in_descend && !_horizontal_movement) || (_below_gnd_effect_hgt && TAKEOFF_STATE_FLIGHT) || TAKEOFF_STATE_RAMPUP
 * ============================================================================ */

TEST_F(LandDetectorFixture, GroundEffectMCDC_ConditionA_InDescend)
{
	detector.set_horizontal_movement(false); // B=True
	detector.set_below_gnd_effect_hgt(false);     // C=False
	detector.set_takeoff_state(takeoff_status_s::TAKEOFF_STATE_DISARMED); // D=F, E=F

	// TP_D3_A1: A=True (_in_descend) -> D3 = True
	detector.set_in_descend(true);
	check_ground_effect("TP_D3_A1", {true, true, false, false, false}, true);

	// TP_D3_A2: A=False (!_in_descend) -> D3 = False
	detector.set_in_descend(false);
	check_ground_effect("TP_D3_A2", {false, true, false, false, false}, false);
}

TEST_F(LandDetectorFixture, GroundEffectMCDC_ConditionB_NoHorizontalMovement)
{
	detector.set_in_descend(true);            // A=True
	detector.set_below_gnd_effect_hgt(false); // C=False
	detector.set_takeoff_state(takeoff_status_s::TAKEOFF_STATE_DISARMED);

	// TP_D3_B1: B=True (cached horizontal movement false) -> D3 = True
	detector.set_horizontal_movement(false);
	check_ground_effect("TP_D3_B1", {true, true, false, false, false}, true);

	// TP_D3_B2: change only the cached movement flag, keeping A=True.
	detector.set_horizontal_movement(true);
	check_ground_effect("TP_D3_B2", {true, false, false, false, false}, false);
}

TEST_F(LandDetectorFixture, GroundEffectMCDC_ConditionC_BelowGndHeight)
{
	detector.set_in_descend(false);           // A=False
	detector.set_horizontal_movement(true); // B=False
	detector.set_takeoff_state(takeoff_status_s::TAKEOFF_STATE_FLIGHT); // D=True, E=False

	// TP_D3_C1: C=True (below ground effect height) -> D3 = True
	detector.set_below_gnd_effect_hgt(true);
	check_ground_effect("TP_D3_C1", {false, false, true, true, false}, true);

	// TP_D3_C2: C=False (above ground effect height) -> D3 = False
	detector.set_below_gnd_effect_hgt(false);
	check_ground_effect("TP_D3_C2", {false, false, false, true, false}, false);
}

TEST_F(LandDetectorFixture, GroundEffectMCDC_ConditionD_TakeoffStateFlight)
{
	// Pair: TP_D3_D1 vs TP_D3_D2
	// Independence proof: flip D (TAKEOFF_STATE_FLIGHT) while A=F, B=any, C=T, E=F
	// With C=True (_below_gnd_effect_hgt) and E=False (not RAMPUP), flipping D flips D3.
	detector.set_in_descend(false);           // A=False
	detector.set_below_gnd_effect_hgt(true);  // C=True
	detector.set_horizontal_movement(true); // B=False
	detector.set_takeoff_state(takeoff_status_s::TAKEOFF_STATE_DISARMED); // baseline: E=False

	// TP_D3_D1: D=True (TAKEOFF_STATE_FLIGHT) -> (C&&D) = True -> D3 = True
	detector.set_takeoff_state(takeoff_status_s::TAKEOFF_STATE_FLIGHT);
	check_ground_effect("TP_D3_D1", {false, false, true, true, false}, true);

	// TP_D3_D2: D=False (TAKEOFF_STATE_DISARMED, not FLIGHT not RAMPUP) -> (C&&D)=False AND E=False -> D3 = False
	detector.set_takeoff_state(takeoff_status_s::TAKEOFF_STATE_DISARMED);
	check_ground_effect("TP_D3_D2", {false, false, true, false, false}, false);
}

TEST_F(LandDetectorFixture, GroundEffectMCDC_ConditionE_TakeoffRampup)
{
	detector.set_in_descend(false);
	detector.set_horizontal_movement(true);
	detector.set_below_gnd_effect_hgt(false);

	// TP_D3_E1: E=True (TAKEOFF_STATE_RAMPUP) -> D3 = True
	detector.set_takeoff_state(takeoff_status_s::TAKEOFF_STATE_RAMPUP);
	check_ground_effect("TP_D3_E1", {false, false, false, false, true}, true);

	// TP_D3_E2: E=False (TAKEOFF_STATE_DISARMED) -> D3 = False
	detector.set_takeoff_state(takeoff_status_s::TAKEOFF_STATE_DISARMED);
	check_ground_effect("TP_D3_E2", {false, false, false, false, false}, false);
}


/* ============================================================================
 * DECISION D4: Vertical Velocity Validity / Fallback
 * D4 = (v_z_valid && |vz| < threshold) || (z_valid && |z_deriv| < threshold)
 * Evaluated inside _get_ground_contact_state() to set !_vertical_movement
 * ============================================================================ */

TEST_F(LandDetectorFixture, MCDC_D4_VerticalVelocityFallback_A_VzValid)
{
	detector.set_local_position_timestamp(hrt_absolute_time());
	detector.set_z_derivative(false, 10.0f); // C=False, D=False
	detector.set_vertical_velocity(0.0f); // B=True

	// TP_D4_A1: A=True -> !vertical_movement = True
	detector.set_v_z_valid(true);
	detector.test_get_ground_contact_state();
	EXPECT_FALSE(detector.test_get_vertical_movement());

	// TP_D4_A2: A=False -> !vertical_movement = False
	detector.set_v_z_valid(false);
	detector.test_get_ground_contact_state();
	EXPECT_TRUE(detector.test_get_vertical_movement());
}

TEST_F(LandDetectorFixture, MCDC_D4_VerticalVelocityFallback_B_VzLow)
{
	detector.set_local_position_timestamp(hrt_absolute_time());
	detector.set_z_derivative(false, 10.0f); // C=False, D=False
	detector.set_v_z_valid(true); // A=True

	// TP_D4_B1: B=True -> !vertical_movement = True
	detector.set_vertical_velocity(0.0f);
	detector.test_get_ground_contact_state();
	EXPECT_FALSE(detector.test_get_vertical_movement());

	// TP_D4_B2: B=False -> !vertical_movement = False
	detector.set_vertical_velocity(10.0f);
	detector.test_get_ground_contact_state();
	EXPECT_TRUE(detector.test_get_vertical_movement());
}

TEST_F(LandDetectorFixture, MCDC_D4_VerticalVelocityFallback_C_ZValid)
{
	detector.set_local_position_timestamp(hrt_absolute_time());
	detector.set_v_z_valid(false); // A=False
	detector.set_vertical_velocity(10.0f); // B=False

	// TP_D4_C1: C=True, D=True -> !vertical_movement = True
	detector.set_z_derivative(true, 0.0f);
	detector.test_get_ground_contact_state();
	EXPECT_FALSE(detector.test_get_vertical_movement());

	// TP_D4_C2: C=False, D=True -> !vertical_movement = False
	detector.set_z_derivative(false, 0.0f);
	detector.test_get_ground_contact_state();
	EXPECT_TRUE(detector.test_get_vertical_movement());
}

TEST_F(LandDetectorFixture, MCDC_D4_VerticalVelocityFallback_D_ZDerivLow)
{
	detector.set_local_position_timestamp(hrt_absolute_time());
	detector.set_v_z_valid(false); // A=False
	detector.set_vertical_velocity(10.0f); // B=False
	detector.set_z_derivative(true, 0.0f); // C=True

	// TP_D4_D1: D=True -> !vertical_movement = True
	detector.set_z_derivative(true, 0.0f);
	detector.test_get_ground_contact_state();
	EXPECT_FALSE(detector.test_get_vertical_movement());

	// TP_D4_D2: D=False -> !vertical_movement = False
	detector.set_z_derivative(true, 10.0f);
	detector.test_get_ground_contact_state();
	EXPECT_TRUE(detector.test_get_vertical_movement());
}

/* ============================================================================
 * DECISION D5: Horizontal Position Availability
 * D5 = lpos_available && v_xy_valid && (v_xy.longerThan(threshold))
 * Evaluated inside _get_ground_contact_state() to set _horizontal_movement
 * ============================================================================ */

TEST_F(LandDetectorFixture, MCDC_D5_HorizontalPosition_A_LposAvailable)
{
	detector.set_v_xy_valid(true); // B=True
	detector.set_horizontal_velocity(5.0f, 0.0f); // C=True

	// TP_D5_A1: A=True -> _horizontal_movement = True
	detector.set_local_position_timestamp(hrt_absolute_time());
	detector.test_get_ground_contact_state();
	EXPECT_TRUE(detector.test_get_horizontal_movement());

	// TP_D5_A2: A=False -> _horizontal_movement = False
	detector.set_local_position_timestamp(land_detector_test_now - 2_s);
	detector.test_get_ground_contact_state();
	EXPECT_FALSE(detector.test_get_horizontal_movement());
}

TEST_F(LandDetectorFixture, MCDC_D5_HorizontalPosition_B_VxyValid)
{
	detector.set_local_position_timestamp(hrt_absolute_time()); // A=True
	detector.set_horizontal_velocity(5.0f, 0.0f); // C=True

	// TP_D5_B1: B=True -> _horizontal_movement = True
	detector.set_v_xy_valid(true);
	detector.test_get_ground_contact_state();
	EXPECT_TRUE(detector.test_get_horizontal_movement());

	// TP_D5_B2: B=False -> _horizontal_movement = False
	detector.set_v_xy_valid(false);
	detector.test_get_ground_contact_state();
	EXPECT_FALSE(detector.test_get_horizontal_movement());
}

TEST_F(LandDetectorFixture, MCDC_D5_HorizontalPosition_C_VxyHigh)
{
	detector.set_local_position_timestamp(hrt_absolute_time()); // A=True
	detector.set_v_xy_valid(true); // B=True

	// TP_D5_C1: C=True -> _horizontal_movement = True
	detector.set_horizontal_velocity(5.0f, 0.0f);
	detector.test_get_ground_contact_state();
	EXPECT_TRUE(detector.test_get_horizontal_movement());

	// TP_D5_C2: C=False -> _horizontal_movement = False
	detector.set_horizontal_velocity(0.0f, 0.0f);
	detector.test_get_ground_contact_state();
	EXPECT_FALSE(detector.test_get_horizontal_movement());
}

/* ============================================================================
 * DECISION D6: Ground-Effect Eligibility
 * D6 = lpos_available && dist_bottom_valid && alt_gnd_effect > 0 && dist_bottom < alt_gnd_effect
 * Evaluated inside _get_ground_contact_state() to set _below_gnd_effect_hgt
 * ============================================================================ */
TEST_F(LandDetectorFixture, MCDC_D6_GroundEffectEligibility_A_LposAvailable)
{
	detector.set_distance_bottom(true, 1.0f); // B=True, D=True
	// C is true because configure_thresholds sets _param_lndmc_alt_gnd_effect to 2.f

	// TP_D6_A1: A=True -> True
	detector.set_local_position_timestamp(hrt_absolute_time());
	detector.test_get_ground_contact_state();
	EXPECT_TRUE(detector.test_get_below_gnd_effect_hgt());

	// TP_D6_A2: A=False -> False
	detector.set_local_position_timestamp(land_detector_test_now - 2_s);
	detector.test_get_ground_contact_state();
	EXPECT_FALSE(detector.test_get_below_gnd_effect_hgt());
}

TEST_F(LandDetectorFixture, MCDC_D6_GroundEffectEligibility_B_DistBottomValid)
{
	detector.set_local_position_timestamp(hrt_absolute_time()); // A=True

	// TP_D6_B1: B=True, D=True -> True
	detector.set_distance_bottom(true, 1.0f);
	detector.test_get_ground_contact_state();
	EXPECT_TRUE(detector.test_get_below_gnd_effect_hgt());

	// TP_D6_B2: B=False, D=True -> False
	detector.set_distance_bottom(false, 1.0f);
	detector.test_get_ground_contact_state();
	EXPECT_FALSE(detector.test_get_below_gnd_effect_hgt());
}

TEST_F(LandDetectorFixture, MCDC_D6_GroundEffectEligibility_D_DistBelowThresh)
{
	detector.set_local_position_timestamp(hrt_absolute_time()); // A=True

	// TP_D6_D1: D=True -> True
	detector.set_distance_bottom(true, 1.0f);
	detector.test_get_ground_contact_state();
	EXPECT_TRUE(detector.test_get_below_gnd_effect_hgt());

	// TP_D6_D2: D=False -> False
	detector.set_distance_bottom(true, 5.0f);
	detector.test_get_ground_contact_state();
	EXPECT_FALSE(detector.test_get_below_gnd_effect_hgt());
}

/* ============================================================================
 * DECISION D7: Hover-Thrust Retention
 * D7 = !_in_descend || hover_thrust_estimate_valid
 * Evaluated inside _get_ground_contact_state() to update _hover_thrust_estimate_valid
 * ============================================================================ */
TEST_F(LandDetectorFixture, MCDC_D7_HoverThrustRetention_A_NotInDescend)
{
	// A = !_in_descend, B = hover_thrust_estimate_valid

	// TP_D7_A1: A=True, B=False -> Result is False (because it assigns B to the state)
	detector.set_in_descend(false); // A=True
	detector.set_hover_thrust_estimate_last_valid(land_detector_test_now - 2_s); // B=False
	detector.set_hover_thrust_estimate_valid(true); // Pre-set to true to see if it remains true
	detector.test_get_ground_contact_state();
	EXPECT_FALSE(detector.test_get_hover_thrust_estimate_valid());

	// TP_D7_A2: A=False, B=False -> Result is True (because it skips assignment, keeps old True value)
	detector.set_in_descend(true); // A=False
	detector.set_hover_thrust_estimate_last_valid(land_detector_test_now - 2_s); // B=False
	detector.set_hover_thrust_estimate_valid(true);
	detector.test_get_ground_contact_state();
	EXPECT_TRUE(detector.test_get_hover_thrust_estimate_valid());
}

TEST_F(LandDetectorFixture, MCDC_D7_HoverThrustRetention_B_HoverThrustValid)
{
	// TP_D7_B1: A=False, B=True -> Result is True
	detector.set_in_descend(true); // A=False
	detector.set_hover_thrust_estimate_last_valid(hrt_absolute_time()); // B=True
	detector.set_hover_thrust_estimate_valid(false);
	detector.test_get_ground_contact_state();
	EXPECT_TRUE(detector.test_get_hover_thrust_estimate_valid());

	// TP_D7_B2: A=False, B=False -> Result is False
	detector.set_in_descend(true); // A=False
	detector.set_hover_thrust_estimate_last_valid(land_detector_test_now - 2_s); // B=False
	detector.set_hover_thrust_estimate_valid(false);
	detector.test_get_ground_contact_state();
	EXPECT_FALSE(detector.test_get_hover_thrust_estimate_valid());
}

/* ============================================================================
 * DECISION D8: Commanded Descent
 * D8 = ISFINITE(vel[2]) && (vel[2] >= 1.1 * z_vel_max)
 * Evaluated inside _get_ground_contact_state() when _flag_control_climb_rate_enabled
 * ============================================================================ */
TEST_F(LandDetectorFixture, MCDC_D8_CommandedDescent_A_IsFinite)
{
	detector.set_flag_control_climb_rate_enabled(true);
	float thresh = 1.1f * 0.5f; // z_vel_max is 0.5

	// TP_D8_A1: A=True, B=True -> True
	detector.publish_trajectory_setpoint(thresh + 0.1f);
	detector.test_get_ground_contact_state();
	EXPECT_TRUE(detector.test_get_in_descend());

	// TP_D8_A2: A=False, B=True (if it were possible, NAN is not >= thresh, so C++ short-circuits. We test NAN)
	detector.publish_trajectory_setpoint(NAN);
	detector.test_get_ground_contact_state();
	EXPECT_FALSE(detector.test_get_in_descend());
}

TEST_F(LandDetectorFixture, MCDC_D8_CommandedDescent_B_AboveThresh)
{
	detector.set_flag_control_climb_rate_enabled(true);
	float thresh = 1.1f * 0.5f;

	// TP_D8_B1: A=True, B=True -> True
	detector.publish_trajectory_setpoint(thresh + 0.1f);
	detector.test_get_ground_contact_state();
	EXPECT_TRUE(detector.test_get_in_descend());

	// TP_D8_B2: A=True, B=False -> False
	detector.publish_trajectory_setpoint(thresh - 0.1f);
	detector.test_get_ground_contact_state();
	EXPECT_FALSE(detector.test_get_in_descend());
}

/* ============================================================================
 * DECISION D9: Landed-State Gating
 * D9 = !_maybe_landed_hysteresis && !_landed_hysteresis
 * Evaluated inside _get_ground_contact_state() to apply in_descend to ground_contact
 * ============================================================================ */
TEST_F(LandDetectorFixture, MCDC_D9_LandedStateGating_A_NotMaybeLanded)
{
	detector.set_flag_control_climb_rate_enabled(true);
	// We make ground_contact true initially by setting low throttle
	detector.set_vehicle_thrust_setpoint_throttle(0.0f);
	// We make in_descend false. If D9 is true, ground_contact becomes false (true &= false).
	// If D9 is false, ground_contact remains true.
	detector.publish_trajectory_setpoint(0.0f); // in_descend = false

	// TP_D9_A1: A=True (!maybe_landed=T), B=True (!landed=T) -> D9=True -> ground_contact &= false -> False
	detector.set_maybe_landed_hysteresis_state(false);
	detector.set_landed_hysteresis_state(false);
	detector.test_get_ground_contact_state();
	EXPECT_FALSE(detector.test_get_has_low_throttle() && detector.test_get_in_descend());
	// Wait, test_get_has_low_throttle() just returns _has_low_throttle, which is not updated by the &= in_descend.
	// We need to check if the overall _get_ground_contact_state() returns differently?
	// The overall ground contact requires ground_contact to be true.
	// We'll set conditions such that overall would be true if ground_contact is true.
	detector.set_horizontal_velocity(0.0f, 0.0f);
	detector.set_vertical_velocity(0.0f);
	detector.set_local_position_timestamp(hrt_absolute_time());
	detector.set_armed(true);
	// skip_close_to_ground_check = true by default if not observable
	EXPECT_FALSE(detector.test_get_ground_contact_state()); // Because ground_contact became false

	// TP_D9_A2: A=False (!maybe_landed=F), B=True -> D9=False -> ground_contact remains true -> overall True
	detector.set_maybe_landed_hysteresis_state(true);
	detector.set_landed_hysteresis_state(false);
	EXPECT_TRUE(detector.test_get_ground_contact_state());
}

TEST_F(LandDetectorFixture, MCDC_D9_LandedStateGating_B_NotLanded)
{
	detector.set_flag_control_climb_rate_enabled(true);
	detector.set_vehicle_thrust_setpoint_throttle(0.0f);
	detector.publish_trajectory_setpoint(0.0f); // in_descend = false
	detector.set_horizontal_velocity(0.0f, 0.0f);
	detector.set_vertical_velocity(0.0f);
	detector.set_local_position_timestamp(hrt_absolute_time());
	detector.set_armed(true);

	// TP_D9_B1: A=True (!maybe_landed=T), B=True (!landed=T) -> D9=True -> overall False
	detector.set_maybe_landed_hysteresis_state(false);
	detector.set_landed_hysteresis_state(false);
	EXPECT_FALSE(detector.test_get_ground_contact_state());

	// TP_D9_B2: A=True (!maybe_landed=T), B=False (!landed=F) -> D9=False -> overall True
	detector.set_maybe_landed_hysteresis_state(false);
	detector.set_landed_hysteresis_state(true);
	EXPECT_TRUE(detector.test_get_ground_contact_state());
}

/* ============================================================================
 * DECISION D10: Distance-Check Alternatives
 * D10 = (dist_bottom_valid && dist_bottom < thresh) || !_dist_bottom_is_observable || !dist_bottom_valid
 * Evaluated in _get_ground_contact_state() to set _close_to_ground_or_skipped_check
 * ============================================================================ */
TEST_F(LandDetectorFixture, MCDC_D10_DistanceCheck_A_CloseToGround)
{
	// D10 = A || B || C
	// A = _is_close_to_ground() -> valid && dist < 1.0
	// B = !_dist_bottom_is_observable
	// C = !dist_bottom_valid
	detector.set_dist_bottom_is_observable(true); // B=False

	// TP_D10_A1: A=True (valid=T, dist < 1.0), C=False (valid=T) -> True
	detector.set_distance_bottom(true, 0.5f);
	detector.test_get_ground_contact_state();
	EXPECT_TRUE(detector.test_get_close_to_ground_or_skipped_check());

	// TP_D10_A2: A=False (valid=T, dist > 1.0), C=False (valid=T) -> False
	detector.set_distance_bottom(true, 1.5f);
	detector.test_get_ground_contact_state();
	EXPECT_FALSE(detector.test_get_close_to_ground_or_skipped_check());
}

TEST_F(LandDetectorFixture, MCDC_D10_DistanceCheck_B_NotObservable)
{
	// TP_D10_B1: B=True, A=False, C=False -> True
	detector.set_dist_bottom_is_observable(false);
	detector.set_distance_bottom(true, 1.5f); // A=False, C=False
	// wait, set_distance_bottom forces _dist_bottom_is_observable = true. We must overwrite it.
	detector.set_dist_bottom_is_observable(false);
	detector.test_get_ground_contact_state();
	EXPECT_TRUE(detector.test_get_close_to_ground_or_skipped_check());

	// TP_D10_B2: B=False, A=False, C=False -> False
	detector.set_dist_bottom_is_observable(true);
	detector.test_get_ground_contact_state();
	EXPECT_FALSE(detector.test_get_close_to_ground_or_skipped_check());
}

TEST_F(LandDetectorFixture, MCDC_D10_DistanceCheck_C_NotValid)
{
	// TP_D10_C1: C=True, A=False, B=False -> True
	detector.set_dist_bottom_is_observable(true); // B=False
	detector.set_distance_bottom(false, 1.5f); // C=True, A=False
	detector.test_get_ground_contact_state();
	EXPECT_TRUE(detector.test_get_close_to_ground_or_skipped_check());

	// TP_D10_C2: C=False, A=False, B=False -> False
	detector.set_distance_bottom(true, 1.5f); // C=False, A=False
	detector.test_get_ground_contact_state();
	EXPECT_FALSE(detector.test_get_close_to_ground_or_skipped_check());
}

/* ============================================================================
 * DECISION D11: Vertical-Estimate Availability
 * D11 = local_position_updated && vertical_velocity_valid
 * Evaluated in _get_maybe_landed_state()
 * ============================================================================ */
TEST_F(LandDetectorFixture, MCDC_D11_VerticalEstimate_A_LocalPosUpdated)
{
	// D11 is evaluated as vertical_estimate. If vertical_estimate is true,
	// it uses _ground_contact_hysteresis. If false, it uses _minimum_thrust_8s_hysteresis.
	// We'll set _ground_contact_hysteresis=true and _minimum_thrust_8s_hysteresis=false.
	// So overall D2 is true ONLY if D11 is true.
	detector.set_armed(true);
	detector.set_vehicle_thrust_setpoint_throttle(0.0f);
	detector.set_freefall_hysteresis_state(false);
	detector.set_angular_velocity(matrix::Vector3f(0.0f, 0.0f, 0.0f));
	detector.set_ground_contact_hysteresis_state(true);
	detector.set_minimum_thrust_8s_hysteresis_state(false);

	detector.set_v_z_valid(true); // B=True

	// TP_D11_A1: A=True -> D11=True -> overall True
	detector.set_local_position_timestamp(hrt_absolute_time());
	EXPECT_TRUE(detector.test_get_maybe_landed_state());

	// TP_D11_A2: A=False -> D11=False -> overall False
	detector.set_local_position_timestamp(land_detector_test_now - 2_s);
	EXPECT_FALSE(detector.test_get_maybe_landed_state());
}

TEST_F(LandDetectorFixture, MCDC_D11_VerticalEstimate_B_VzValid)
{
	detector.set_armed(true);
	detector.set_vehicle_thrust_setpoint_throttle(0.0f);
	detector.set_freefall_hysteresis_state(false);
	detector.set_angular_velocity(matrix::Vector3f(0.0f, 0.0f, 0.0f));
	detector.set_ground_contact_hysteresis_state(true);
	detector.set_minimum_thrust_8s_hysteresis_state(false);

	detector.set_local_position_timestamp(hrt_absolute_time()); // A=True

	// TP_D11_B1: B=True -> D11=True -> overall True
	detector.set_v_z_valid(true);
	EXPECT_TRUE(detector.test_get_maybe_landed_state());

	// TP_D11_B2: B=False -> D11=False -> overall False
	detector.set_v_z_valid(false);
	EXPECT_FALSE(detector.test_get_maybe_landed_state());
}

/* ============================================================================
 * FREEFALL AND LANDED STATE TESTS
 * ============================================================================ */


TEST_F(LandDetectorFixture, Boundary_FreefallAcceleration)
{
	// Threshold is strictly less than 2.0 m/s^2.
	detector.set_acceleration(matrix::Vector3f(0.0f, 0.0f, 1.99f));
	EXPECT_TRUE(detector.test_get_freefall_state());

	detector.set_acceleration(matrix::Vector3f(0.0f, 0.0f, 2.00f));
	EXPECT_FALSE(detector.test_get_freefall_state());

	detector.set_acceleration(matrix::Vector3f(0.0f, 0.0f, 2.01f));
	EXPECT_FALSE(detector.test_get_freefall_state());
}

TEST_F(LandDetectorFixture, Boundary_VerticalVelocity)
{
	detector.set_local_position_timestamp(hrt_absolute_time());
	detector.set_z_derivative(false, 0.0f);
	detector.set_landed_hysteresis_state(false);

	detector.test_get_ground_contact_state();
	float z_vel_max = 0.5f; // we set it to 0.5f in configure_thresholds and we set landSpeed to 0.0f

	detector.set_vertical_velocity(z_vel_max - 0.01f);
	detector.test_get_ground_contact_state();
	EXPECT_FALSE(detector.test_get_vertical_movement());

	detector.set_vertical_velocity(z_vel_max);
	detector.test_get_ground_contact_state();
	EXPECT_TRUE(detector.test_get_vertical_movement());

	detector.set_vertical_velocity(z_vel_max + 0.01f);
	detector.test_get_ground_contact_state();
	EXPECT_TRUE(detector.test_get_vertical_movement());
}

TEST_F(LandDetectorFixture, Boundary_HorizontalVelocity)
{
	// Threshold is norm(v_xy) > 1.5
	detector.set_local_position_timestamp(hrt_absolute_time());

	detector.set_horizontal_velocity(1.49f, 0.0f);
	detector.test_get_ground_contact_state();
	EXPECT_FALSE(detector.test_get_horizontal_movement());

	detector.set_horizontal_velocity(1.50f, 0.0f);
	detector.test_get_ground_contact_state();
	EXPECT_FALSE(detector.test_get_horizontal_movement());

	detector.set_horizontal_velocity(1.51f, 0.0f);
	detector.test_get_ground_contact_state();
	EXPECT_TRUE(detector.test_get_horizontal_movement());
}

TEST_F(LandDetectorFixture, Boundary_RotationRate)
{
	// Threshold is > 20.0 deg/s (0.349066 rad/s)
	float rot_max_rad = 20.0f * (static_cast<float>(M_PI) / 180.0f);

	detector.set_angular_velocity(matrix::Vector3f(rot_max_rad - 0.01f, 0.0f, 0.0f));
	detector.test_get_maybe_landed_state();
	EXPECT_FALSE(detector.test_get_rotational_movement());

	detector.set_angular_velocity(matrix::Vector3f(rot_max_rad, 0.0f, 0.0f));
	detector.test_get_maybe_landed_state();
	EXPECT_FALSE(detector.test_get_rotational_movement());

	detector.set_angular_velocity(matrix::Vector3f(rot_max_rad + 1.0f, 0.0f, 0.0f));
	detector.test_get_maybe_landed_state();
	EXPECT_TRUE(detector.test_get_rotational_movement());
}

TEST_F(LandDetectorFixture, Boundary_GroundEffectDistance)
{
	// Threshold is < 2.0m
	detector.set_local_position_timestamp(hrt_absolute_time());

	detector.set_distance_bottom(true, 1.99f);
	detector.test_get_ground_contact_state();
	EXPECT_TRUE(detector.test_get_below_gnd_effect_hgt());

	detector.set_distance_bottom(true, 2.00f);
	detector.test_get_ground_contact_state();
	EXPECT_FALSE(detector.test_get_below_gnd_effect_hgt());

	detector.set_distance_bottom(true, 2.01f);
	detector.test_get_ground_contact_state();
	EXPECT_FALSE(detector.test_get_below_gnd_effect_hgt());
}

TEST_F(LandDetectorFixture, Boundary_CloseToGroundDistance)
{
	// Threshold is < 1.0m (DIST_FROM_GROUND_THRESHOLD)
	detector.set_local_position_timestamp(hrt_absolute_time());

	detector.set_distance_bottom(true, 0.99f);
	detector.test_get_ground_contact_state();
	EXPECT_TRUE(detector.test_get_close_to_ground_or_skipped_check());

	detector.set_distance_bottom(true, 1.00f);
	detector.test_get_ground_contact_state();
	EXPECT_FALSE(detector.test_get_close_to_ground_or_skipped_check());

	detector.set_distance_bottom(true, 1.01f);
	detector.test_get_ground_contact_state();
	EXPECT_FALSE(detector.test_get_close_to_ground_or_skipped_check());
}

TEST_F(LandDetectorFixture, Boundary_PositionStaleness)
{
	// Threshold is < 1.0s (1,000,000 us)

	// Just below: 999999 us
	detector.set_local_position_timestamp(land_detector_test_now - 999999_us);
	detector.set_vertical_velocity(0.0f);
	detector.test_get_ground_contact_state();
	EXPECT_FALSE(detector.test_get_vertical_movement()); // because lpos_available is true, vz < threshold

	// Exactly at threshold: 1000000 us
	detector.set_local_position_timestamp(land_detector_test_now - 1_s);
	detector.test_get_ground_contact_state();
	EXPECT_TRUE(detector.test_get_vertical_movement()); // because lpos_available is false

	// Just above: 1000001 us
	detector.set_local_position_timestamp(land_detector_test_now - 1000001_us);
	detector.test_get_ground_contact_state();
	EXPECT_TRUE(detector.test_get_vertical_movement());
}

TEST_F(LandDetectorFixture, Boundary_Thrust)
{
	// Threshold is <= (_params.minManThrottle + 0.01f) which is 0.08 + 0.01 = 0.09f
	detector.set_armed(true);
	detector.set_flag_control_climb_rate_enabled(false);
	detector.set_freefall_hysteresis_state(false);
	detector.set_angular_velocity(matrix::Vector3f(0.0f, 0.0f, 0.0f));
	detector.set_ground_contact_hysteresis_state(true);
	detector.set_local_position_timestamp(hrt_absolute_time());
	detector.set_v_z_valid(true);

	// Below threshold (0.089f) -> thrust condition met -> maybe_landed = true
	detector.set_vehicle_thrust_setpoint_throttle(0.0f);
	EXPECT_TRUE(detector.test_get_maybe_landed_state());

	// Equal to threshold (0.090f) -> thrust condition met -> maybe_landed = true
	detector.set_vehicle_thrust_setpoint_throttle(0.08999999f);
	EXPECT_TRUE(detector.test_get_maybe_landed_state());

	// Above threshold (0.091f) -> thrust condition not met -> maybe_landed = false
	detector.set_vehicle_thrust_setpoint_throttle(1.0f);
	EXPECT_FALSE(detector.test_get_maybe_landed_state());
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
