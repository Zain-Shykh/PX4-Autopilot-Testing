#!/usr/bin/env bash
set -euo pipefail
task_root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd)"
cd -- "$task_root"
task_build="$task_root/build/px4_sitl_test"
task_out="$task_root/member1_deliverables"
mkdir -p "$task_out/logs"
cmake -S . -B "$task_build" -G Ninja -DCONFIG=px4_sitl_test -DCMAKE_BUILD_TYPE=Coverage \
    > "$task_out/logs/reproduce_configure.log" 2>&1
cmake --build "$task_build" --target px4 unit-Hysteresis unit-HysteresisAssignment \
    functional-AttitudeEstimatorQ functional-LandDetector -j "${MEMBER1_JOBS:-4}" \
    > "$task_out/logs/reproduce_build.log" 2>&1
for phase in baseline student final; do
    python3 "$task_out/scripts/collect_coverage.py" "$phase"
done
task_strip="$(python3 -c 'from pathlib import Path; import sys; print(len(Path(sys.argv[1]).parts)-1)' "$task_build")"
GCOV_PREFIX="$task_build/member1_profiles/member2" GCOV_PREFIX_STRIP="$task_strip" \
    GTEST_OUTPUT="xml:$task_out/logs/member2_execution.xml" \
    ctest --test-dir "$task_build" -R '^functional-LandDetector$' --timeout 30 -V \
    > "$task_out/logs/member2_execution.log" 2>&1
GCOV_PREFIX="$task_build/member1_profiles/shuffle" GCOV_PREFIX_STRIP="$task_strip" \
    "$task_build/functional-AttitudeEstimatorQ" --gtest_shuffle --gtest_random_seed=2026 --gtest_repeat=3 \
    > "$task_out/logs/shuffle_execution.log" 2>&1
python3 "$task_out/scripts/analyze_coverage.py" > "$task_out/logs/coverage_summary.log"
python3 "$task_out/scripts/build_workbook.py"
python3 "$task_out/scripts/generate_patches.py"
