#!/usr/bin/env python3
"""Build a two-sheet XLSX with Python's standard library; statuses come from GTest XML."""
import csv
from pathlib import Path
import re
from xml.etree import ElementTree as ET
from xml.sax.saxutils import escape
from zipfile import ZipFile, ZIP_DEFLATED

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / 'member1_deliverables'
HEADER = ['Test ID', 'Component / function', 'Purpose / scenario', 'Key controlled input / state',
          'Expected result', 'Execution result', 'Structural coverage target', 'Test file / evidence reference']
# Inputs and oracles supplement the authoritative executable tests, not duplicate them.
DETAILS = {
 'A01': ('update / init_attitude_q', 'data_good=false then valid level vectors', 'Reject before data; initialize a unit identity quaternion', 'Initialization gate T/F'),
 'A02': ('init_attitude_q', 'roll=.3 pitch=-.2; declination=.15 rad', 'Recover tilt quaternion times declination rotation', 'Initialization numerical statements'),
 'A03': ('init_attitude_q', 'zero acceleration; collinear mag; then valid mag', 'Reject degenerate inputs, then recover', 'Nonfinite initialization branch'),
 'A04': ('update', 'z gyro=1 rad/s; dt=.02; correction gains=0', 'Yaw=2*atan(.01); unit quaternion; rates=gyro', 'Gyro integration and normalization'),
 'A05': ('update', 'mode=1; valid vision; conflicting mag', 'Negative analytic yaw correction from vision', 'Vision selection; skip mag'),
 'A06': ('update', 'mode=2; valid mocap; heading along -Y', 'Positive yaw correction', 'Mocap selection'),
 'A07': ('update', 'mode=1; external heading invalid; mag along +Y', 'Magnetometer supplies negative yaw correction', 'Heading validity fallback'),
 'A08': ('update', 'spin=.873,1.746,20 rad/s', 'Magnetic gain=1,2,10; analytic integrated yaw', 'Spin threshold and gain cap'),
 'A09': ('update', 'acceleration magnitude=.9g,1g,1.1g; compensation off', 'Fuse only the interior 1g case', 'Strict acceleration norm limits'),
 'A10': ('update', 'accel=(4,0,-g); position acceleration=(4,0,0)', 'Compensated gravity preserves identity attitude', 'Compensation bypass and subtraction'),
 'A11': ('update', 'injected bias limit=.02; opposite mag errors; spin=.175', 'Clip bias both signs; do not learn at spin boundary', 'Bias learning gate and saturation'),
 'A12': ('update', 'NaN gyro; previous finite q and nonzero bias', 'False; restore q; clear rates and bias', 'Invalid-update rollback'),
 'A13': ('update', 'position acceleration equals measured acceleration', 'False; zero-vector normalization rolls back', 'Zero division / rollback'),
 'A14': ('update_mag_declination', 'before init; .2 to .20001 to .4 rad', 'Small change preserves q; large change rotates yaw', 'Initial, trivial, nontrivial declination paths'),
 'A15': ('update_parameters', 'SYS_HAS_MAG=0 then ATT_W_MAG=0', 'Disable mag fusion; seed north vector; initialize', 'Missing-mag / zero-weight guards'),
 'A16': ('update_sensors', 'timestamp=123 then 0; invalid relative acceleration timestamp', 'Retain gyro/time and accel when their validity gates fail', 'Independent timestamp gates'),
 'A17': ('update_sensors / update_magnetometer', 'zero vectors then exact .01 magnitude', 'Degenerate vectors stored; subsequent boundary samples read', 'Degenerate input early returns'),
 'A18': ('update_gps_position', 'auto declination off/on; eph=20 then19.99', 'Change declination only when enabled and accurate', 'GPS enablement / accuracy gates'),
 'A19': ('update_visual_odometry / update_motion_capture_odometry', 'NaN q; variance10001,10000,NaN,all negative', 'Reject bad q/high or negative variance; accept boundary/unknown variance', 'Quaternion and variance validation'),
 'A20': ('update_visual_odometry / update_motion_capture_odometry', 'modes1,2,0; ages0,499999,500000; timestamp0', 'Use matching fresh source only; timeout at .5s', 'Source / timestamp / timeout decisions'),
 'A21': ('update_vehicle_local_position', 'first/equal/new timestamps; 90deg yaw; vx increases .002 in .001s', 'Avoid zero division; body acceleration approximately(0,-2,0)', 'Derivative history and body rotation'),
 'A22': ('update_vehicle_local_position', 'disable compensation; stale20ms; invalid XY/Z; eph5; uninitialized', 'Clear position acceleration, previous velocity and timestamp', 'Each position acceptance gate false'),
 'A23': ('update_vehicle_attitude', 'failed init then valid; elapsed0 or100ms; sample time1234', 'Publish only success; clamp dt to10us/20ms; retain sample time', 'Publish gate and dt clamp'),
 'A24': ('Run', 'no new IMU, then one valid sample, then no new sample', 'Process only the new sample and initialize', 'Run sensor gate and routing'),
 'A25': ('update_parameters / update', 'real ATT_BIAS_MAX=.05; low spin and nonzero heading error', 'Characterization: internal limit and learned bias remain zero (defect)', 'Parameter-to-filter wiring defect reproduction'),
 'A26': ('update_parameters', 'ATT_W_MAG=.7; notification after2s; force=false', 'Cached weight becomes .7', 'Parameter notification path'),
 'A27': ('init', 'other attitude publisher fresh then exactly1s old', 'Reject fresh competitor; accept stale publisher', 'Startup freshness guard'),
 'A28': ('Run', 'stop requested; no registered ModuleBase object', 'Bypass sensor processing; cleanup returns safely', 'Stop branch'),
 'A29': ('init', 'no competing attitude publisher', 'Register sensor callback successfully', 'Unadvertised startup path'),
 'H01': ('set_state_and_update / update', 'rise delay11; fall delay7; deadline-1 and deadline', 'Switch exactly at both asymmetric deadlines', 'Delay boundary T/F in both directions'),
 'H02': ('set_state_and_update / update', 'cancel at deadline; restart later; both initial states', 'Cancellation wins; renewed request waits full delay', 'Cancel / restart request paths'),
 'H03': ('set_hysteresis_time_from / update', 'pending delay20 shortened to10; both states', 'Use original request time and switch at100+10', 'Live delay reduction'),
 'H04': ('set_hysteresis_time_from / update', 'pending delay10 extended to20; both states', 'Old deadline cannot switch; new deadline can', 'Live delay extension'),
 'H05': ('set_hysteresis_time_from / update', 'pending50 becomes0 at timestamp0', 'Immediate change on update; immediate reverse', 'Zero time / zero delay'),
 'H06': ('set_state_and_update / update', 'two instances interleaved with delays5/9', 'Independent deadlines and states', 'State isolation and no pending update'),
 'H07': ('update', 'request at uint64_max-10; delay10', 'Switch at uint64_max, not one microsecond before', 'Large timestamp without overflow'),
}


def execution(path):
    result = {}
    for test in ET.parse(path).iter('testcase'):
        status = 'FAIL' if test.find('failure') is not None else 'BLOCKED' if (test.find('skipped') is not None or test.get('status') == 'notrun') else 'PASS'
        result[test.attrib['name']] = status
    return result


def inventory():
    rows = []
    for source, xml in [
        ('src/modules/attitude_estimator_q/AttitudeEstimatorQTest.cpp', 'functional-AttitudeEstimatorQ.xml'),
        ('src/lib/hysteresis/HysteresisAssignmentTest.cpp', 'unit-HysteresisAssignment.xml'),
    ]:
        statuses = execution(OUT / 'coverage/final' / xml)
        code = (ROOT / source).read_text()
        pattern = r'// (TC_M1_(\w+)): ([^\n]+)\nTEST(?:_F)?\([^,]+, (\w+)\)'
        for match in re.finditer(pattern, code):
            test_id, key, scenario, name = match.groups()
            function, inputs, expected, target = DETAILS[key]
            line = code[:match.start()].count('\n') + 2
            rows.append([test_id, function, scenario, inputs, expected, statuses[name], target,
                         f'{source}:{line} ({name}); coverage/final/{xml}'])
        assert len(statuses) == len(re.findall(pattern, code)), source
    source = 'src/modules/land_detector/LandDetectorTest.cpp'
    code = (ROOT / source).read_text()
    statuses = execution(OUT / 'logs/member2_execution.xml')
    member2_inputs = {
        'GroundContactMCDC_ConditionA_Armed': 'armed=false then true; distance10m; thrust1; velocity=(5,5,5)',
        'GroundContactMCDC_ConditionB_CloseToGround': 'armed; thrust0; valid bottom distance .5m then10m; no movement',
        'GroundContactMCDC_ConditionC_LowThrottle': 'armed; no movement; thrust0 then1',
        'GroundContactMCDC_ConditionD_HorizontalMovement': 'armed; thrust0; vz0; vx0 then5m/s',
        'GroundContactMCDC_ConditionE_VerticalMovement': 'armed; thrust0; horizontal velocity0; vz0 then5m/s',
        'MaybeLandedMCDC_ConditionA_Armed': 'thrust1; freefall; rotation(5,5,0); stale position; no ground contact; armed=false then true',
        'MaybeLandedMCDC_ConditionB_MinThrust': 'armed; not freefall; rotation0; ground contact true; thrust0 then1',
        'MaybeLandedMCDC_ConditionC_NotFreefall': 'armed; thrust0; rotation0; ground contact true; freefall false then true',
        'MaybeLandedMCDC_ConditionE_VerticalEstimate': 'controlled clock20s; G=true after8s; F=false; position exactly1s old then fresh',
        'MaybeLandedMCDC_ConditionF_GroundContactHysteresis': 'fresh vertical estimate; G=false; toggle ground contact true then false',
        'MaybeLandedMCDC_ConditionG_MinThrust8sHysteresis': 'position exactly1s old; F=false; G=true after8s then false',
        'GroundEffectMCDC_ConditionD_TakeoffStateFlight': 'not descending; cached horizontal movement true; below height; FLIGHT then DISARMED',
        'MaybeLandedMCDC_ConditionD_NotRotating': 'armed; thrust0; no freefall; ground contact; rotation0 then(5,5,0)',
        'GroundEffectMCDC_ConditionA_InDescend': 'no horizontal movement; height flag false; DISARMED; descend true then false',
        'GroundEffectMCDC_ConditionB_NoHorizontalMovement': 'descending remains true; height flag false; DISARMED; cached horizontal movement false then true; all other D3 operands fixed',
        'GroundEffectMCDC_ConditionC_BelowGndHeight': 'not descending; FLIGHT; height flag true then false',
        'GroundEffectMCDC_ConditionE_TakeoffRampup': 'not descending; height flag false; RAMPUP then DISARMED',
    }
    # Preserve the published IDs of the original fifteen cases; append the four newer cases.
    legacy_names = [
        'GroundContactMCDC_ConditionA_Armed', 'GroundContactMCDC_ConditionB_CloseToGround',
        'GroundContactMCDC_ConditionC_LowThrottle', 'GroundContactMCDC_ConditionD_HorizontalMovement',
        'GroundContactMCDC_ConditionE_VerticalMovement', 'MaybeLandedMCDC_ConditionA_Armed',
        'MaybeLandedMCDC_ConditionB_MinThrust', 'MaybeLandedMCDC_ConditionC_NotFreefall',
        'MaybeLandedMCDC_ConditionD_NotRotating', 'GroundEffectMCDC_ConditionA_InDescend',
        'GroundEffectMCDC_ConditionB_NoHorizontalMovement', 'GroundEffectMCDC_ConditionC_BelowGndHeight',
        'GroundEffectMCDC_ConditionE_TakeoffRampup', 'Freefall_AccelThresholdBoundary', 'Landed_StateTransitions',
        'MaybeLandedMCDC_ConditionE_VerticalEstimate', 'MaybeLandedMCDC_ConditionF_GroundContactHysteresis',
        'MaybeLandedMCDC_ConditionG_MinThrust8sHysteresis', 'GroundEffectMCDC_ConditionD_TakeoffStateFlight',
    ]
    for match in re.finditer(r'TEST_F\(LandDetectorFixture, (\w+)\)', code):
        name = match.group(1)
        line = code[:match.start()].count('\n') + 1
        i = legacy_names.index(name) + 1
        if name.startswith('GroundContact'):
            function = '_get_ground_contact_state'
        elif name.startswith('MaybeLanded'):
            function = '_get_maybe_landed_state'
        elif name.startswith('GroundEffect'):
            function = '_get_ground_effect_state'
        elif name.startswith('Freefall'):
            function = '_get_freefall_state'
        else:
            function = '_get_landed_state'
        inputs = member2_inputs.get(name, '')
        expected = 'Decision true then false; assert all controlled D1-D3 operand values and record runtime vectors'
        if name.startswith('Freefall'):
            inputs = 'Acceleration(1,0,1) then(0,0,9.81)'; expected = 'Freefall true then false'
        if name.startswith('Landed'):
            inputs = 'Disarmed; armed+maybe_landed true; armed+maybe_landed false'; expected = 'Landed true, true, false'
        rows.append([f'TC_M2_{i:02d}', 'MulticopterLandDetector::' + function, name, inputs, expected,
                     statuses[name], 'D1-D3 return-decision pairs verified against runtime vectors; broader governing guards pending',
                     f'{source}:{line} ({name}); logs/member2_execution.xml'])
    return rows


def column(n):
    s = ''
    while n:
        n, r = divmod(n - 1, 26)
        s = chr(65 + r) + s
    return s


def worksheet(rows, widths):
    last = f'{column(len(rows[0]))}{len(rows)}'
    parts = ['<?xml version="1.0" encoding="UTF-8" standalone="yes"?>',
             '<worksheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">',
             f'<dimension ref="A1:{last}"/><sheetViews><sheetView workbookViewId="0">',
             '<pane ySplit="1" topLeftCell="A2" activePane="bottomLeft" state="frozen"/>',
             '</sheetView></sheetViews><sheetFormatPr defaultRowHeight="18"/><cols>']
    for n, width in enumerate(widths, 1):
        parts.append(f'<col min="{n}" max="{n}" width="{width}" customWidth="1"/>')
    parts.append('</cols><sheetData>')
    for r, row in enumerate(rows, 1):
        parts.append(f'<row r="{r}" ht="{36 if r == 1 else 72}" customHeight="1">')
        for c, value in enumerate(row, 1):
            style = 1 if r == 1 else 2
            parts.append(f'<c r="{column(c)}{r}" s="{style}" t="inlineStr"><is><t xml:space="preserve">{escape(str(value))}</t></is></c>')
        parts.append('</row>')
    parts.extend(['</sheetData>', f'<autoFilter ref="A1:{last}"/>',
                  '<pageMargins left="0.25" right="0.25" top="0.5" bottom="0.5" header="0.3" footer="0.3"/>',
                  '<pageSetup paperSize="9" orientation="landscape" fitToWidth="1" fitToHeight="0"/>', '</worksheet>'])
    return ''.join(parts)


def main():
    # Fail rather than publish MC/DC rows that disagree with actual passing execution.
    import subprocess
    import sys
    subprocess.run([sys.executable, str(ROOT / 'member2_deliverables/scripts/validate_mcdc.py'),
                    '--xml', str(OUT / 'logs/member2_execution.xml')], check=True)
    rows = inventory()
    with (OUT / 'sheet1_test_inventory.csv').open('w', newline='') as f:
        writer = csv.writer(f); writer.writerow(HEADER); writer.writerows(rows)
    with (ROOT / 'member2_deliverables/sheet2_mcdc_evidence.csv').open(newline='') as f:
        mcdc = list(csv.reader(f))
    # Excel forbids '/' in worksheet names. Preserve the required meaning with a valid name.
    names = ['Test Inventory', 'MC-DC Evidence']
    mcdc[0].append('Integration review')
    for row in mcdc[1:]:
        row.append('D1-D3: runtime operands/outcomes checked; 17 single-condition pairs validated. Masks follow source short-circuit rules. Other governing guards remain to be analyzed.')
    ns = 'http://schemas.openxmlformats.org/spreadsheetml/2006/main'
    rel = 'http://schemas.openxmlformats.org/officeDocument/2006/relationships'
    content = '<?xml version="1.0"?><Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types"><Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/><Default Extension="xml" ContentType="application/xml"/><Override PartName="/xl/workbook.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"/><Override PartName="/xl/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml"/>'
    for i in (1, 2):
        content += f'<Override PartName="/xl/worksheets/sheet{i}.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>'
    content += '</Types>'
    styles = f'''<?xml version="1.0"?><styleSheet xmlns="{ns}"><fonts count="2"><font><sz val="11"/><name val="Calibri"/></font><font><b/><sz val="11"/><color rgb="FFFFFFFF"/><name val="Calibri"/></font></fonts><fills count="3"><fill><patternFill patternType="none"/></fill><fill><patternFill patternType="gray125"/></fill><fill><patternFill patternType="solid"><fgColor rgb="FF17365D"/><bgColor indexed="64"/></patternFill></fill></fills><borders count="1"><border/></borders><cellStyleXfs count="1"><xf/></cellStyleXfs><cellXfs count="3"><xf xfId="0"/><xf xfId="0" fontId="1" fillId="2" applyFont="1" applyFill="1" applyAlignment="1"><alignment vertical="center" wrapText="1"/></xf><xf xfId="0" applyAlignment="1"><alignment vertical="top" wrapText="1"/></xf></cellXfs><cellStyles count="1"><cellStyle name="Normal" xfId="0" builtinId="0"/></cellStyles></styleSheet>'''
    with ZipFile(OUT / 'Testing_Workbook.xlsx', 'w', ZIP_DEFLATED) as z:
        z.writestr('[Content_Types].xml', content)
        z.writestr('_rels/.rels', f'<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships"><Relationship Id="rId1" Type="{rel}/officeDocument" Target="xl/workbook.xml"/></Relationships>')
        sheets = ''.join(f'<sheet name="{name}" sheetId="{i}" r:id="rId{i}"/>' for i, name in enumerate(names, 1))
        z.writestr('xl/workbook.xml', f'<workbook xmlns="{ns}" xmlns:r="{rel}"><sheets>{sheets}</sheets></workbook>')
        relationships = ''.join(f'<Relationship Id="rId{i}" Type="{rel}/worksheet" Target="worksheets/sheet{i}.xml"/>' for i in (1, 2))
        z.writestr('xl/_rels/workbook.xml.rels', f'<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">{relationships}<Relationship Id="rId3" Type="{rel}/styles" Target="styles.xml"/></Relationships>')
        z.writestr('xl/styles.xml', styles)
        z.writestr('xl/worksheets/sheet1.xml', worksheet([HEADER, *rows], [17, 39, 56, 62, 66, 16, 55, 90]))
        z.writestr('xl/worksheets/sheet2.xml', worksheet(mcdc, [16, 44, 65, 45, 20, 42, 40, 70, 85]))
    with ZipFile(OUT / 'Testing_Workbook.xlsx') as z:
        assert z.testzip() is None
        for file in z.namelist(): ET.fromstring(z.read(file))
        assert len(ET.fromstring(z.read('xl/workbook.xml')).find(f'{{{ns}}}sheets')) == 2
    assert len(rows) == 55 and len(mcdc) == 35
    print(f'Workbook validated: {len(rows)} tests (36 Member 1 + 19 Member 2), 34 runtime-checked MC/DC rows, exactly 2 sheets.')


if __name__ == '__main__':
    main()
