# Reproduce Member 1's evidence

Run commands from the PX4 repository root. The local run used the installed PX4 development dependencies, recursive submodules, GCC 13.3, CMake/Ninja, Python 3, LCOV/genhtml and gcov. Fresh CMake configuration may fetch the upstream test dependencies if they are not cached. Workbook generation uses only the Python standard library; no Excel package or online service is needed.

## Apply on a separate clean baseline checkout

The existing working directory already contains these changes: **do not apply the patch to it again**. In a separate checkout containing the required commit and submodules:

```bash
git checkout --detach d6f12ad1c4f70ad3230afd7d86e971421e02fef4
git submodule update --init --recursive
git apply --check /path/to/member1_deliverables/px4_assignment2_tests.patch
git apply /path/to/member1_deliverables/px4_assignment2_tests.patch
```

Copy `member1_deliverables/` and the supplied `member2_deliverables/` into that checkout for report/workbook scripts. The combined patch contains only test-related source/CMake/access changes, not documentation or unrelated existing EKF fixture changes. To add only Member 1's code, use `member1_tests.patch`; omit Member 2 build/workbook integration when that track is unavailable.

## Configure and build the registered PX4 tests

```bash
cmake -S . -B build/px4_sitl_test -G Ninja \
  -DCONFIG=px4_sitl_test -DCMAKE_BUILD_TYPE=Coverage
cmake --build build/px4_sitl_test --target px4 \
  unit-Hysteresis unit-HysteresisAssignment \
  functional-AttitudeEstimatorQ functional-LandDetector -j 4
ctest --test-dir build/px4_sitl_test \
  -R '^(unit-HysteresisAssignment|functional-AttitudeEstimatorQ)$' --timeout 30 -V
```

This produces **36 new test cases passing in 2 CTest targets**. PX4's standard alternative is:

```bash
make tests PX4_CMAKE_BUILD_TYPE=Coverage TESTFILTER='AttitudeEstimatorQ|HysteresisAssignment'
```

The direct CMake/CTest commands above were used to keep the build focused. `make tests` can build additional dependencies even with a test filter. Do not use `make tests_coverage` midway through an existing shared build: PX4's implementation starts with a clean; the scripts below instead isolate coverage counters.

## Generate comparable coverage

```bash
python3 member1_deliverables/scripts/collect_coverage.py baseline
python3 member1_deliverables/scripts/collect_coverage.py student
python3 member1_deliverables/scripts/collect_coverage.py final
python3 member1_deliverables/scripts/analyze_coverage.py
```

`baseline` runs only existing `unit-Hysteresis`; `student` runs the two new suites; `final` runs both plus upstream hysteresis. Initial zero-count notes are included. All commands, GTest cases, LCOV options, and summaries are written to `logs/<phase>_coverage.log`. The estimator is compiled through the same test translation unit for each phase, preventing branch-layout mismatches when merging initial and executed traces. Each phase has `coverage.info`, its input tracefiles, XML test results, and a self-contained `html/index.html`.

`--ignore-errors mismatch` matches PX4's documented local LCOV workaround, while `unused` permits the estimator include pattern to have no executed data in the upstream-only baseline. These options do not supply hit counts. Branch collection is explicitly enabled; no exception edges are filtered from submitted metrics. The comparison is scoped, not the complete upstream PX4 suite.

## Additional checks and packaging

```bash
ctest --test-dir build/px4_sitl_test -R '^sitl-(mathlib|matrix)$' --timeout 30 -V
python3 member1_deliverables/scripts/build_workbook.py
python3 member1_deliverables/scripts/generate_patches.py
git diff --check
```

The two SITL checks need permission to create local Unix sockets. The supplied unrestricted smoke-test log shows both passed. They are separate from the estimator's functional coverage run. Work-queue diagnostics in functional tests are expected because the runner does not launch scheduler threads.

For the complete scripted rebuild, isolated coverage, Member 2 recheck, shuffled estimator test, workbook and patches:

```bash
bash member1_deliverables/scripts/reproduce.sh
```

`MEMBER1_JOBS=2` may be set on memory-constrained systems. Reproduction replaces generated Member 1 evidence; it does not delete the shared build, change production behavior, or overwrite Member 2 documentation. Patch generation works with unstaged/untracked test files and verifies reconstruction in a temporary directory without altering the Git index. Source copies in `source_files/` are generated from the authoritative files under `src/`.
