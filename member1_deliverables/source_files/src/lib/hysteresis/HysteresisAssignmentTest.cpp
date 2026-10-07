// Assignment 02, Member 1. New boundary and interrupted-transition tests.
#include <gtest/gtest.h>
#include <limits>
#include "hysteresis.h"

// TC_M1_H01: asymmetric delays include the deadline, in both directions.
TEST(HysteresisAssignment, ExactDeadlinesInBothDirections)
{
	systemlib::Hysteresis h(false);
	h.set_hysteresis_time_from(false, 11);
	h.set_hysteresis_time_from(true, 7);
	h.set_state_and_update(true, 100);
	h.update(110);
	EXPECT_FALSE(h.get_state());
	h.set_state_and_update(true, 111);
	ASSERT_TRUE(h.get_state());
	h.set_state_and_update(false, 120);
	h.update(126);
	EXPECT_TRUE(h.get_state());
	h.set_state_and_update(false, 127);
	EXPECT_FALSE(h.get_state());
}

// TC_M1_H02: cancellation at the deadline must beat the pending update.
TEST(HysteresisAssignment, CancelAtDeadlineAndRestartFullDelay)
{
	for (bool initial : {false, true}) {
		systemlib::Hysteresis h(initial);
		h.set_hysteresis_time_from(initial, 10);
		h.set_state_and_update(!initial, 100);
		h.set_state_and_update(initial, 110);
		EXPECT_EQ(h.get_state(), initial);
		h.update(200);
		EXPECT_EQ(h.get_state(), initial);
		h.set_state_and_update(!initial, 200);
		h.update(209);
		EXPECT_EQ(h.get_state(), initial);
		h.update(210);
		EXPECT_EQ(h.get_state(), !initial);
	}
}

// TC_M1_H03: duration changes while pending retain the original request time.
TEST(HysteresisAssignment, ShortenPendingDelay)
{
	for (bool initial : {false, true}) {
		systemlib::Hysteresis h(initial);
		h.set_hysteresis_time_from(initial, 20);
		h.set_state_and_update(!initial, 100);
		h.update(109);
		h.set_hysteresis_time_from(initial, 10);
		h.update(110);
		EXPECT_EQ(h.get_state(), !initial);
	}
}

// TC_M1_H04: extending the delay invalidates the old deadline.
TEST(HysteresisAssignment, ExtendPendingDelay)
{
	for (bool initial : {false, true}) {
		systemlib::Hysteresis h(initial);
		h.set_hysteresis_time_from(initial, 10);
		h.set_state_and_update(!initial, 100);
		h.set_hysteresis_time_from(initial, 20);
		h.update(110);
		EXPECT_EQ(h.get_state(), initial);
		h.update(119);
		EXPECT_EQ(h.get_state(), initial);
		h.update(120);
		EXPECT_EQ(h.get_state(), !initial);
	}
}

// TC_M1_H05: zero-delay change during a pending request, same timestamp.
TEST(HysteresisAssignment, PendingDelayCanBecomeImmediate)
{
	systemlib::Hysteresis h(true);
	h.set_hysteresis_time_from(true, 50);
	h.set_state_and_update(false, 0);
	ASSERT_TRUE(h.get_state());
	h.set_hysteresis_time_from(true, 0);
	h.update(0);
	EXPECT_FALSE(h.get_state());
	h.set_state_and_update(true, 0);
	EXPECT_TRUE(h.get_state());
}

// TC_M1_H06: updates of an unrelated object cannot share pending state.
TEST(HysteresisAssignment, InterleavedInstancesRemainIndependent)
{
	systemlib::Hysteresis a(false), b(true);
	a.set_hysteresis_time_from(false, 5);
	b.set_hysteresis_time_from(true, 9);
	a.set_state_and_update(true, 100);
	b.set_state_and_update(false, 100);
	a.update(105);
	b.update(105);
	EXPECT_TRUE(a.get_state());
	EXPECT_TRUE(b.get_state());
	b.update(109);
	EXPECT_FALSE(b.get_state());
	a.update(109);
	EXPECT_TRUE(a.get_state());
}

// TC_M1_H07: exact deadline near the representable limit, without overflow.
TEST(HysteresisAssignment, LargeTimestampDeadline)
{
	const hrt_abstime end = std::numeric_limits<hrt_abstime>::max();
	systemlib::Hysteresis h(false);
	h.set_hysteresis_time_from(false, 10);
	h.set_state_and_update(true, end - 10);
	h.update(end - 1);
	EXPECT_FALSE(h.get_state());
	h.update(end);
	EXPECT_TRUE(h.get_state());
}
