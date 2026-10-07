#!/usr/bin/env python3
"""Summarize raw LCOV; enumerate every remaining line/branch gap without hiding it."""
import csv
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / 'member1_deliverables/coverage'


def records(path):
    current = None
    for row in path.read_text().splitlines():
        if row.startswith('SF:'):
            current = {'source': str(Path(row[3:]).relative_to(ROOT)), 'lines': {}, 'branches': []}
        elif row.startswith('DA:'):
            line, count, *_ = row[3:].split(',')
            current['lines'][int(line)] = int(count)
        elif row.startswith('BRDA:'):
            line, block, branch, count = row[5:].split(',')
            current['branches'].append((int(line), block, int(branch), count))
        elif row == 'end_of_record' and current:
            yield current
            current = None


def reason(source, line, block, kind):
    if source.endswith('hysteresis.cpp'):
        return ('Boolean invariant: requested!=state permits only (T,F) or (F,T). After '
                'not(true->false), false->true is necessary; the three short-circuit edges '
                'at lines 77 and 83 are infeasible for bool values.')
    if line >= 589:
        return ('Module command/allocation wrapper, outside filter/message business logic. '
                'Further coverage needs command lifecycle tests and controlled allocation failure.')
    if block.startswith('e'):
        return ('Compiler exception-unwind edge at a C++ call. No dependency throws in these cases; '
                'requires deliberate exception injection. Retained in raw denominator.')
    if line in (193, 194, 195):
        return ('Callback registration failure: initialized local uORB successfully allocates the callback; '
                'requires targeted uORB registration/allocation fault injection.')
    if line in (229, 245, 263, 339, 372):
        return ('Inner update(false) after updated(true): subscription generation is unchanged by the preceding '
                'check and there is no competing consumer in synchronous tests. Needs concurrent topic '
                'teardown or a subscription fault-injection seam.')
    if line == 455:
        return ('Finite normalized quaternion length is approximately one. False >0.95 and <1.05 '
                'outcomes require normalization fault injection; degenerate vectors already exercise '
                'the nonfinite rejection outcome.')
    return 'Investigate before reporting completion.'


def main():
    summary = {}
    for phase in ('baseline', 'student', 'final'):
        summary[phase] = {}
        for record in records(OUT / phase / 'coverage.info'):
            lines, branches = record['lines'], record['branches']
            summary[phase][record['source']] = {
                'lines_hit': sum(n > 0 for n in lines.values()), 'lines_total': len(lines),
                'branches_hit': sum(c not in ('0', '-') for _, _, _, c in branches),
                'branches_total': len(branches),
            }
    gaps = []
    for record in records(OUT / 'final/coverage.info'):
        source = record['source']
        text = (ROOT / source).read_text().splitlines()
        for line, count in record['lines'].items():
            if count == 0:
                gaps.append([source, line, 'line', '', '', text[line - 1].strip(), reason(source, line, '', 'line')])
        for line, block, branch, count in record['branches']:
            if count in ('0', '-'):
                gaps.append([source, line, 'branch', block, branch, text[line - 1].strip(), reason(source, line, block, 'branch')])
        if source.endswith('attitude_estimator_q_main.cpp'):
            core = {n: c for n, c in record['lines'].items() if 201 <= n <= 587}
            summary['core_filter_and_message_methods'] = {
                'definition': 'Run through update_mag_declination, inclusive; source lines 201-587',
                'lines_hit': sum(c > 0 for c in core.values()), 'lines_total': len(core),
            }
    with (OUT / 'uncovered_obligations.csv').open('w', newline='') as out:
        writer = csv.writer(out)
        writer.writerow(['Production source', 'Line', 'Kind', 'LCOV block', 'LCOV edge', 'Source expression', 'Investigation and additional strategy'])
        writer.writerows(gaps)
    assert not any('Investigate before' in row[-1] for row in gaps), 'Uninvestigated coverage gap'
    (OUT / 'summary.json').write_text(json.dumps(summary, indent=2) + '\n')
    print(json.dumps(summary, indent=2))


if __name__ == '__main__':
    main()
