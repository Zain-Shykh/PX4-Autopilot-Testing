#!/usr/bin/env python3
"""Validate the documented D1-D3 pairs against passing GTest runtime properties.

The evaluation masks below follow the checked-in C++ expressions. They are a
source-derived short-circuit analysis, not compiler condition instrumentation.
"""
import argparse
import csv
import json
from collections import defaultdict
from pathlib import Path
import re
import xml.etree.ElementTree as ET

OUT = Path(__file__).resolve().parents[1]


def evaluate(decision, values):
    evaluated = set()

    def condition(index):
        evaluated.add(index)
        return values[index]

    if decision == 'D1':
        outcome = condition(0) or (condition(1) and condition(2) and condition(3) and condition(4))
    elif decision == 'D2':
        outcome = condition(0) or (condition(1) and condition(2) and condition(3) and
                                  ((condition(4) and condition(5)) or (not condition(4) and condition(6))))
    elif decision == 'D3':
        outcome = (condition(0) and condition(1)) or (condition(2) and condition(3)) or condition(4)
    else:
        raise ValueError(f'Unknown decision: {decision}')
    return outcome, evaluated


def validate(matrix, xml):
    tree = ET.parse(xml)
    cases = {case.get('name'): case for case in tree.iter('testcase')}
    pairs = defaultdict(list)
    seen_values = defaultdict(set)
    seen_outcomes = defaultdict(set)
    evidence = []
    with matrix.open(newline='') as stream:
        rows = list(csv.DictReader(stream))
    for row in rows:
        decision = row['Decision ID']
        match = re.fullmatch(r'(\w+) \((TP_\w+)\)', row['Student Test Case Reference (LandDetectorTest.cpp)'])
        assert match, row
        name, vector_id = match.groups()
        case = cases[name]
        assert case.find('failure') is None and case.find('skipped') is None, name
        assert case.get('status') == 'run' and case.get('result') == 'completed', name
        properties = {prop.get('name'): prop.get('value') for prop in case.findall('properties/property')}
        vector = row['Atomic Condition Values [A B C D E F G]']
        expected_outcome = row['Decision Outcome (Y)']
        assert properties[vector_id] == f'{vector} -> {expected_outcome}', (name, vector_id, properties)
        tokens = vector.strip('[]').split()
        assert all(token in ('T', 'F') for token in tokens), vector
        values = [token == 'T' for token in tokens]
        outcome, evaluated = evaluate(decision, values)
        assert outcome == (expected_outcome == 'True'), vector_id
        for index in evaluated:
            seen_values[(decision, index)].add(values[index])
        seen_outcomes[decision].add(outcome)
        condition_letter = re.match(r'Condition ([A-G])\b', row['Condition Demonstrated']).group(1)
        pairs[(decision, condition_letter)].append((vector_id, values, outcome))
        evidence.append({'vector_id': vector_id, 'test': name, 'values': tokens, 'outcome': outcome,
                         'evaluated_by_source_short_circuit_rules': ['ABCDEFG'[i] for i in sorted(evaluated)]})

    assert len(rows) == 34 and len(pairs) == 17
    for (decision, letter), pair in pairs.items():
        assert len(pair) == 2, (decision, letter)
        first, second = pair
        changed = [i for i, (a, b) in enumerate(zip(first[1], second[1])) if a != b]
        assert changed == ['ABCDEFG'.index(letter)], (decision, letter, changed)
        assert first[2] != second[2], (decision, letter, 'decision did not change')
    for decision, size in [('D1', 5), ('D2', 7), ('D3', 5)]:
        assert seen_outcomes[decision] == {True, False}, decision
        for index in range(size):
            assert seen_values[(decision, index)] == {True, False}, (decision, index, 'missing evaluated value')
    return {'status': 'PASS', 'scope': 'D1-D3 return decisions only; governing guards require further analysis',
            'test_cases_in_execution': len(cases), 'verified_vectors': len(rows), 'verified_pairs': len(pairs),
            'short_circuit_note': 'Masks are derived from source; runtime properties assert controlled operand values and actual production outcomes.',
            'vectors': evidence}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--xml', type=Path, default=OUT / 'logs/merge_main/functional-LandDetector.xml')
    parser.add_argument('--matrix', type=Path, default=OUT / 'sheet2_mcdc_evidence.csv')
    parser.add_argument('--output', type=Path)
    args = parser.parse_args()
    result = validate(args.matrix, args.xml)
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(json.dumps(result, indent=2) + '\n')
    print(f"PASS: {result['verified_vectors']} runtime vectors, {result['verified_pairs']} independence pairs; D1-D3 only.")


if __name__ == '__main__':
    main()
