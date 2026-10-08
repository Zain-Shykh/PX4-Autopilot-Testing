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
#include "battery.h"
#include <analog_battery.h>
#include <uORB/Publication.hpp>
#include <uORB/Subscription.hpp>
#include <uORB/topics/vehicle_status.h>
#include <uORB/topics/flight_phase_estimation.h>
#include <uORB/topics/battery_status.h>

class TestBattery : public Battery
{
public:
	TestBattery(int index = 1, ModuleParams *parent = nullptr, const int sample_interval_us = 100000, const uint8_t source = 0)
		: Battery(index, parent, sample_interval_us, source)
	{
	}

	using Battery::calculateStateOfChargeVoltageBased;
	using Battery::estimateStateOfCharge;
	using Battery::determineWarning;
	using Battery::determineFaults;
	using Battery::computeScale;
	using Battery::updateInternalResistanceEstimation;
	using Battery::resetInternalResistanceEstimation;
	using Battery::updateParams;
	using Battery::_params;
	using Battery::_connected;
	using Battery::_voltage_v;
	using Battery::_current_a;
	using Battery::_state_of_charge;
	using Battery::_state_of_charge_volt_based;
	using Battery::_warning;
	using Battery::_scale;
	using Battery::_discharged_mah;
	using Battery::_discharged_mah_loop;
	using Battery::_capacity_mah;
	using Battery::_internal_resistance_estimate;
	using Battery::_internal_resistance_initialized;
	using Battery::_estimation_covariance_norm;
	using Battery::_battery_initialized;
	using Battery::_armed;
	using Battery::_vehicle_status_is_fw;
	using Battery::_cell_voltage_filter_v;
};

class TestAnalogBattery : public AnalogBattery
{
public:
	TestAnalogBattery(int index = 1, ModuleParams *parent = nullptr, const int sample_interval_us = 100000, const uint8_t source = 0, const uint8_t priority = 0)
		: AnalogBattery(index, parent, sample_interval_us, source, priority)
	{
	}

	using AnalogBattery::updateParams;
	using AnalogBattery::_analog_params;
};

class BatteryStatusTest : public ::testing::Test
{
public:
	void SetUp() override
	{
		param_control_autosave(false);
		param_reset_all();
		// New subscribers must not inherit another test's armed/fixed-wing messages.
		vehicle_status_s status{};
		status.timestamp = hrt_absolute_time();
		status.arming_state = vehicle_status_s::ARMING_STATE_DISARMED;
		status.vehicle_type = vehicle_status_s::VEHICLE_TYPE_ROTARY_WING;
		ASSERT_TRUE(_vehicle_status_pub.publish(status));
		flight_phase_estimation_s phase{};
		phase.timestamp = hrt_absolute_time();
		ASSERT_TRUE(_flight_phase_estimation_pub.publish(phase));
	}

	void TearDown() override
	{
		param_reset_all();
	}

	uORB::Publication<vehicle_status_s> _vehicle_status_pub{ORB_ID(vehicle_status)};
	uORB::Publication<flight_phase_estimation_s> _flight_phase_estimation_pub{ORB_ID(flight_phase_estimation)};
};

TEST_F(BatteryStatusTest, ParameterInitializationAndCellCount)
{
	int32_t n_cells = 3;
	param_set(param_find("BAT1_N_CELLS"), &n_cells);

	TestBattery battery{1, nullptr, 100000, 0};
	battery.updateParams();

	EXPECT_EQ(battery.cell_count(), 3);
	EXPECT_GT(battery.full_cell_voltage(), battery.empty_cell_voltage());
	EXPECT_GT(battery.empty_cell_voltage(), 2.0f);
}

TEST_F(BatteryStatusTest, DisconnectedBatteryState)
{
	// Unknown capacity is required for the expected NaN remaining-time result.
	float capacity = 0.f;
	ASSERT_EQ(param_set(param_find("BAT1_CAPACITY"), &capacity), 0);
	TestBattery battery{1, nullptr, 100000, 0};
	battery.updateParams();
	ASSERT_FLOAT_EQ(battery._capacity_mah, 0.f);

	// Voltage below LITHIUM_BATTERY_RECOGNITION_VOLTAGE (2.1V)
	battery.updateVoltage(1.5f);
	battery.updateBatteryStatus(hrt_absolute_time());

	battery_status_s status = battery.getBatteryStatus();
	EXPECT_FALSE(status.connected);
	EXPECT_EQ(status.warning, battery_status_s::WARNING_NONE);
	EXPECT_TRUE(std::isnan(status.time_remaining_s));
}

TEST_F(BatteryStatusTest, ConnectedBatteryInitialTransition)
{
	int32_t n_cells = 3;
	param_set(param_find("BAT1_N_CELLS"), &n_cells);

	TestBattery battery{1, nullptr, 100000, 0};
	battery.updateParams();
	battery.setConnected(true);

	// Transition to 12.0V (approx 4.0V/cell for 3S)
	battery.updateVoltage(12.0f);
	battery.updateCurrent(2.0f);
	battery.updateTemperature(25.0f);
	battery.updateBatteryStatus(hrt_absolute_time());

	battery_status_s status = battery.getBatteryStatus();
	EXPECT_TRUE(status.connected);
	EXPECT_GT(status.voltage_v, 11.0f);
	EXPECT_GT(status.remaining, 0.5f);
	EXPECT_LE(status.remaining, 1.0f);
	EXPECT_EQ(status.warning, battery_status_s::WARNING_NONE);
}

TEST_F(BatteryStatusTest, DetermineWarningLadder)
{
	TestBattery battery{1, nullptr, 100000, 0};
	battery.updateParams();

	// Emergency: < emergen_thr (0.05 default)
	EXPECT_EQ(battery.determineWarning(0.02f), battery_status_s::WARNING_EMERGENCY);

	// Critical: < crit_thr (0.07 default)
	EXPECT_EQ(battery.determineWarning(0.06f), battery_status_s::WARNING_CRITICAL);

	// Low: < low_thr (0.15 default)
	EXPECT_EQ(battery.determineWarning(0.10f), battery_status_s::WARNING_LOW);

	// Normal: >= low_thr
	EXPECT_EQ(battery.determineWarning(0.50f), battery_status_s::WARNING_NONE);
}

TEST_F(BatteryStatusTest, DetermineFaultsOvervoltage)
{
	int32_t n_cells = 3;
	param_set(param_find("BAT1_N_CELLS"), &n_cells);

	TestBattery battery{1, nullptr, 100000, 0};
	battery.updateParams();

	// Normal voltage -> no faults
	battery.updateVoltage(12.0f);
	EXPECT_EQ(battery.determineFaults(), 0);

	// Overvoltage spike condition: > n_cells * v_charged * 1.05
	float overvoltage = battery.cell_count() * battery.full_cell_voltage() * 1.10f;
	battery.updateVoltage(overvoltage);
	EXPECT_NE(battery.determineFaults() & (1 << battery_status_s::FAULT_SPIKES), 0);
}

TEST_F(BatteryStatusTest, ComputeScaleThrustCompensation)
{
	int32_t n_cells = 3;
	param_set(param_find("BAT1_N_CELLS"), &n_cells);

	TestBattery battery{1, nullptr, 100000, 0};
	battery.updateParams();
	battery.setConnected(true);

	// 12.6V on 3S -> 4.2V/cell -> scale ~1.0
	hrt_abstime now = hrt_absolute_time();
	battery.updateVoltage(12.6f);
	battery.updateBatteryStatus(now);

	now += 2500000ULL; // > 2s for filter initialization
	battery.updateBatteryStatus(now);

	EXPECT_NEAR(battery.getBatteryStatus().scale, 1.0f, 0.05f);

	// Low voltage -> scale increases towards 1.3
	battery.updateVoltage(10.0f);
	now += 200000ULL;
	battery.updateBatteryStatus(now);
	EXPECT_GE(battery.getBatteryStatus().scale, 1.0f);
	EXPECT_LE(battery.getBatteryStatus().scale, 1.3f);
}

TEST_F(BatteryStatusTest, StateOfChargeVoltageBasedAndLoadDropCorrection)
{
	int32_t n_cells = 3;
	param_set(param_find("BAT1_N_CELLS"), &n_cells);

	TestBattery battery{1, nullptr, 100000, 0};
	battery.updateParams();

	// Zero current voltage based SoC
	battery._cell_voltage_filter_v.reset(11.1f / 3.0f);
	float soc_no_load = battery.calculateStateOfChargeVoltageBased(11.1f, 0.0f);
	EXPECT_GE(soc_no_load, 0.0f);
	EXPECT_LE(soc_no_load, 1.0f);

	// High load current -> voltage drop compensation increases estimated cell voltage
	battery._cell_voltage_filter_v.reset((11.1f / 3.0f) + 0.005f * 15.0f);
	float soc_with_load = battery.calculateStateOfChargeVoltageBased(11.1f, 15.0f);
	EXPECT_GT(soc_with_load, soc_no_load);
}

TEST_F(BatteryStatusTest, StateOfChargeEstimationCoulombFusion)
{
	int32_t n_cells = 3;
	param_set(param_find("BAT1_N_CELLS"), &n_cells);

	TestBattery battery{1, nullptr, 100000, 0};
	battery.setCapacityMah(2200.0f);
	battery.updateParams();
	battery.setConnected(true);

	// Initialize battery at high voltage
	hrt_abstime t = hrt_absolute_time();
	battery.updateVoltage(12.6f);
	battery.updateCurrent(10.0f);
	battery.updateBatteryStatus(t);

	t += 2500000ULL;
	battery.updateBatteryStatus(t);

	float initial_soc = battery.getBatteryStatus().remaining;
	EXPECT_GT(initial_soc, 0.8f);

	// Discharged over 10 seconds at 10A = ~27.78 mAh
	t += 10000000ULL;
	battery.updateVoltage(12.0f);
	battery.updateCurrent(10.0f);
	battery.updateBatteryStatus(t);

	float discharged_soc = battery.getBatteryStatus().remaining;
	EXPECT_LE(discharged_soc, initial_soc);
	EXPECT_GT(battery.getBatteryStatus().discharged_mah, 0.0f);
}

TEST_F(BatteryStatusTest, ComputeRemainingTimeArmedAndFixedWing)
{
	int32_t n_cells = 3;
	param_set(param_find("BAT1_N_CELLS"), &n_cells);
	float capacity = 2200.0f;
	param_set(param_find("BAT1_CAPACITY"), &capacity);

	TestBattery battery{1, nullptr, 100000, 0};
	battery.updateParams();
	battery.setCapacityMah(2200.0f);
	battery.setConnected(true);
	battery.setStateOfCharge(0.80f);

	// Publish armed multicopter status
	hrt_abstime now = hrt_absolute_time();
	vehicle_status_s vstatus{};
	vstatus.timestamp = now;
	vstatus.arming_state = vehicle_status_s::ARMING_STATE_ARMED;
	vstatus.vehicle_type = vehicle_status_s::VEHICLE_TYPE_ROTARY_WING;
	_vehicle_status_pub.publish(vstatus);

	battery.updateVoltage(11.5f);
	battery.updateCurrent(10.0f);
	battery.updateBatteryStatus(now);

	now += 100000;
	battery.updateDt(now);

	float remaining_time = battery.computeRemainingTime(10.0f);
	EXPECT_TRUE(PX4_ISFINITE(remaining_time));
	EXPECT_GT(remaining_time, 0.0f);

	// Switch to Fixed-Wing level flight
	now += 100000;
	vstatus.timestamp = now;
	vstatus.vehicle_type = vehicle_status_s::VEHICLE_TYPE_FIXED_WING;
	_vehicle_status_pub.publish(vstatus);

	flight_phase_estimation_s fpe{};
	fpe.timestamp = now;
	fpe.flight_phase = flight_phase_estimation_s::FLIGHT_PHASE_LEVEL;
	_flight_phase_estimation_pub.publish(fpe);

	battery.updateDt(now + 100000);
	remaining_time = battery.computeRemainingTime(8.0f);
	EXPECT_TRUE(PX4_ISFINITE(remaining_time));
	EXPECT_GT(remaining_time, 0.0f);
}

TEST_F(BatteryStatusTest, ExternalStateOfChargeOverride)
{
	TestBattery battery{1, nullptr, 100000, 0};
	battery.updateParams();

	battery.setStateOfCharge(0.68f);
	battery.updateVoltage(11.0f);
	battery.updateBatteryStatus(hrt_absolute_time());

	EXPECT_NEAR(battery.getBatteryStatus().remaining, 0.68f, 0.01f);
}

TEST_F(BatteryStatusTest, AnalogBatteryADCConversionAndChannels)
{
	float v_div = 10.0f;
	param_set(param_find("BAT1_V_DIV"), &v_div);
	float a_per_v = 20.0f;
	param_set(param_find("BAT1_A_PER_V"), &a_per_v);

	TestAnalogBattery analog_battery{1, nullptr, 100000, 0, 0};
	analog_battery.updateParams();

	// Test ADC conversion with raw voltage and current
	hrt_abstime now = hrt_absolute_time();
	analog_battery.updateBatteryStatusADC(now, 1.5f, 0.5f);

	battery_status_s status = analog_battery.getBatteryStatus();
	EXPECT_TRUE(status.connected);
	EXPECT_GT(status.voltage_v, 0.0f);

	// Channel verification
	EXPECT_TRUE(analog_battery.is_valid());
	EXPECT_GE(analog_battery.get_voltage_channel(), -1);
}
