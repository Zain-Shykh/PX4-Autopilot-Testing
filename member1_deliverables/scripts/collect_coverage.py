#!/usr/bin/env python3
"""Capture focused coverage without deleting or mixing existing PX4 profiles."""
import argparse
import os
from pathlib import Path
import shlex
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[2]
BUILD = ROOT / 'build/px4_sitl_test'
OUT = ROOT / 'member1_deliverables'
SOURCES = ['src/lib/hysteresis/hysteresis.cpp',
           'src/modules/attitude_estimator_q/attitude_estimator_q_main.cpp']


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('phase', choices=['baseline', 'student', 'final'])
    args = parser.parse_args()
    phase = args.phase
    profile = BUILD / 'member1_profiles' / phase
    if profile.exists():
        shutil.rmtree(profile)  # Only this script's isolated profile directory.
    profile.mkdir(parents=True)
    report = OUT / 'coverage' / phase
    if report.exists():
        shutil.rmtree(report)  # Avoid GTest's _1.xml suffix and stale HTML/results.
    report.mkdir(parents=True, exist_ok=True)
    env = os.environ.copy()
    env.update(GCOV_PREFIX=str(profile), GCOV_PREFIX_STRIP=str(len(BUILD.parts) - 1))
    with (OUT / 'logs' / f'{phase}_coverage.log').open('w') as log:
        def run(command, environment=None):
            log.write('$ ' + shlex.join(map(str, command)) + '\n')
            log.flush()
            subprocess.run(list(map(str, command)), cwd=ROOT, env=environment,
                           stdout=log, stderr=subprocess.STDOUT, check=True)

        pattern = {'baseline': '^unit-Hysteresis$',
                   'student': '^(unit-HysteresisAssignment|functional-AttitudeEstimatorQ)$',
                   'final': '^(unit-Hysteresis|unit-HysteresisAssignment|functional-AttitudeEstimatorQ)$'}[phase]
        env['GTEST_OUTPUT'] = 'xml:' + str(report) + '/'
        run(['ctest', '--test-dir', BUILD, '-R', pattern, '--timeout', '30', '-V'], env)
        for data in profile.rglob('*.gcda'):
            note = BUILD / data.relative_to(profile).with_suffix('.gcno')
            if not note.exists():
                raise RuntimeError(f'Missing note for {data}')
            shutil.copy2(note, data.with_suffix('.gcno'))
        common = ['--branch-coverage', '--ignore-errors', 'mismatch,unused',
                  '--base-directory', BUILD]
        for source in SOURCES:
            common += ['--include', str(ROOT / source)]
        # Zero-count instrumented files remain in the denominator.
        estimator_notes = BUILD / 'src/modules/attitude_estimator_q/CMakeFiles/functional-AttitudeEstimatorQ.dir'
        if not estimator_notes.exists():
            estimator_notes = BUILD / 'src/modules/attitude_estimator_q/CMakeFiles/modules__attitude_estimator_q.dir'
        initial_dirs = ['--directory', BUILD / 'src/lib/hysteresis/CMakeFiles/hysteresis.dir',
                        '--directory', estimator_notes]
        run(['lcov', '--capture', '--initial', *initial_dirs, *common, '-o', report / 'initial.info'])
        executed_dirs = ['--directory', profile / 'src/lib/hysteresis']
        if phase != 'baseline':
            executed_dirs += ['--directory', profile / 'src/modules/attitude_estimator_q']
        run(['lcov', '--capture', *executed_dirs, *common, '-o', report / 'executed.info'])
        run(['lcov', '--branch-coverage', '-a', report / 'initial.info', '-a', report / 'executed.info',
             '-o', report / 'coverage.info'])
        run(['genhtml', '--branch-coverage', '--legend', '--prefix', ROOT,
             report / 'coverage.info', '--output-directory', report / 'html'])
        run(['lcov', '--branch-coverage', '--summary', report / 'coverage.info'])
    print(f'{phase}: {report / "html/index.html"}')


if __name__ == '__main__':
    main()
