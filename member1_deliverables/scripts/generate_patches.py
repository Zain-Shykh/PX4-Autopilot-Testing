#!/usr/bin/env python3
"""Package scoped changes, including untracked tests; reconstruct each patch for verification."""
from hashlib import sha256
from pathlib import Path
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / 'member1_deliverables'
BASE = 'd6f12ad1c4f70ad3230afd7d86e971421e02fef4'
MEMBER1 = [
    'src/lib/hysteresis/CMakeLists.txt',
    'src/lib/hysteresis/HysteresisAssignmentTest.cpp',
    'src/modules/attitude_estimator_q/CMakeLists.txt',
    'src/modules/attitude_estimator_q/attitude_estimator_q_main.cpp',
    'src/modules/attitude_estimator_q/AttitudeEstimatorQTest.cpp',
]
MEMBER2 = [
    'src/modules/land_detector/CMakeLists.txt',
    'src/modules/land_detector/MulticopterLandDetector.h',
    'src/modules/land_detector/LandDetectorTest.cpp',
]


def git(*args, cwd=ROOT):
    return subprocess.run(['git', *args], cwd=cwd, capture_output=True)


def main():
    with (OUT / 'logs/patch_validation.log').open('w') as log:
        for name, paths in [('member1_tests.patch', MEMBER1), ('px4_assignment2_tests.patch', MEMBER1 + MEMBER2)]:
            patch = bytearray()
            for path in paths:
                tracked = git('ls-files', '--error-unmatch', path).returncode == 0
                diff = git('diff', '--no-ext-diff', BASE, '--', path) if tracked else git('diff', '--no-ext-diff', '--no-index', '--', '/dev/null', path)
                if diff.returncode not in (0, 1):
                    raise RuntimeError(diff.stderr.decode())
                patch.extend(diff.stdout)
            destination = OUT / name
            destination.write_bytes(patch)
            with tempfile.TemporaryDirectory(prefix='px4-member1-patch-') as temp:
                clean = Path(temp)
                for path in paths:
                    original = git('show', f'{BASE}:{path}')
                    if original.returncode == 0:
                        file = clean / path
                        file.parent.mkdir(parents=True, exist_ok=True)
                        file.write_bytes(original.stdout)
                for args in [('apply', '--check', str(destination)), ('apply', str(destination))]:
                    result = git(*args, cwd=clean)
                    log.write('$ git ' + ' '.join(args) + f'\nexit={result.returncode}\n')
                    log.write(result.stdout.decode() + result.stderr.decode())
                    if result.returncode:
                        raise RuntimeError('Patch reconstruction failed')
                for path in paths:
                    assert (clean / path).read_bytes() == (ROOT / path).read_bytes(), path
                log.write(f'{name}: reconstructed {len(paths)} files byte-for-byte against {BASE}\n')
                log.write('sha256=' + sha256(patch).hexdigest() + '\n\n')
    for path in MEMBER1:
        destination = OUT / 'source_files' / path
        destination.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(ROOT / path, destination)
    print('Both patches applied cleanly and reconstructed the submitted files exactly.')


if __name__ == '__main__':
    main()
