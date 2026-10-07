import openpyxl
from openpyxl.styles import Font, PatternFill, Alignment, Border, Side
from openpyxl.utils import get_column_letter

wb = openpyxl.Workbook()

# Sheet 1: Test Inventory
ws1 = wb.active
ws1.title = 'Test Inventory'

headers_ws1 = [
    'Test ID',
    'Component / Function',
    'Purpose / Scenario',
    'Key Controlled Input / State',
    'Expected Result',
    'Execution Result',
    'Structural-Coverage Target',
    'Test-File / Evidence Reference'
]

rows_ws1 = [
    [
        'TC-M3-FMM-01',
        'FlightModeManager::switchTask',
        'Verify initial inactive task state, explicit switch to None, and idempotent repeated task switching.',
        '_manager.switchTask(FlightTaskIndex::None)',
        'Returns FlightTaskError::NoError, isAnyTaskActive() == false, getCurrentTaskIndex() == None',
        'PASSED',
        'FlightModeManager.cpp (lines 379-414: switchTask idempotent check, None initialization)',
        'src/modules/flight_mode_manager/FlightModeManagerTest.cpp#L125'
    ],
    [
        'TC-M3-FMM-02',
        'FlightModeManager::switchTask(int)',
        'Verify integer-to-enum casting boundaries and out-of-bounds index rejection.',
        'Index values: -1 (valid None), -2 (underflow), 999 (overflow >= Count)',
        '-1 returns NoError; -2 and 999 return InvalidTask with task reset to None',
        'PASSED',
        'FlightModeManager.cpp (lines 416-425: integer range validation)',
        'src/modules/flight_mode_manager/FlightModeManagerTest.cpp#L138'
    ],
    [
        'TC-M3-FMM-03',
        'FlightModeManager::errorToString',
        'Verify human-readable diagnostic error string mappings for task switching statuses.',
        'FlightTaskError enum values: NoError, InvalidTask, ActivationFailed, and invalid code 99',
        'Exact string matches: \"No Error\", \"Invalid Task\", \"Activation Failed\", and fallback unmapped message',
        'PASSED',
        'FlightModeManager.cpp (lines 427-438: errorToString switch cases)',
        'src/modules/flight_mode_manager/FlightModeManagerTest.cpp#L152'
    ],
    [
        'TC-M3-FMM-04',
        'FlightModeManager::start_flight_task (Auto Modes)',
        'Verify autonomous mode dispatching to FlightTaskAuto, AutoFollowTarget, and Orbit.',
        'flag_control_auto_enabled=true; nav_state in {AUTO_TAKEOFF, AUTO_LAND, AUTO_RTL, AUTO_MISSION, AUTO_FOLLOW_TARGET, ORBIT}',
        'Task switched to FlightTaskIndex::Auto, AutoFollowTarget, and Orbit respectively',
        'PASSED',
        'FlightModeManager.cpp (lines 157-196: auto modes and follow/orbit task switches)',
        'src/modules/flight_mode_manager/FlightModeManagerTest.cpp#L160'
    ],
    [
        'TC-M3-FMM-05',
        'FlightModeManager::start_flight_task (Position Control)',
        'Verify MPC_POS_MODE parameter routing between manual position control algorithms.',
        'nav_state=POSCTL; MPC_POS_MODE in {0 (ManualPosition), 4 (ManualAcceleration), 99 (invalid reset)}',
        'Mode 0 starts ManualPosition; Mode 4 and 99 start ManualAcceleration with parameter reset',
        'PASSED',
        'FlightModeManager.cpp (lines 207-230: posctl mode switch and param reset)',
        'src/modules/flight_mode_manager/FlightModeManagerTest.cpp#L209'
    ],
    [
        'TC-M3-FMM-06',
        'FlightModeManager::start_flight_task (Altitude Control)',
        'Verify MPC_POS_MODE parameter routing for manual altitude control modes.',
        'nav_state=ALTCTL; MPC_POS_MODE in {0 (ManualAltitude), 3 (ManualAltitudeSmoothVel), 99 (default fallback)}',
        'Mode 0 starts ManualAltitude; Mode 3 and 99 start ManualAltitudeSmoothVel',
        'PASSED',
        'FlightModeManager.cpp (lines 232-251: altctl mode switch)',
        'src/modules/flight_mode_manager/FlightModeManagerTest.cpp#L236'
    ],
    [
        'TC-M3-FMM-07',
        'FlightModeManager::start_flight_task (Cruise & Descend)',
        'Verify autonomous altitude cruise and emergency descent nav_state handling.',
        'nav_state in {NAVIGATION_STATE_ALTITUDE_CRUISE, NAVIGATION_STATE_DESCEND}',
        'AltitudeCruise task and Descend task activated respectively',
        'PASSED',
        'FlightModeManager.cpp (lines 253-273: cruise and emergency descend dispatch)',
        'src/modules/flight_mode_manager/FlightModeManagerTest.cpp#L263'
    ],
    [
        'TC-M3-FMM-08',
        'FlightModeManager::start_flight_task (Fallbacks)',
        'Verify fixed-wing vehicle type and external navigation states disable multicopter flight tasks.',
        'vehicle_type=VEHICLE_TYPE_FIXED_WING or nav_state=NAVIGATION_STATE_EXTERNAL1',
        'Task cleanly switches to FlightTaskIndex::None; no active task running',
        'PASSED',
        'FlightModeManager.cpp (lines 139-145: fixed wing and external nav exclusion)',
        'src/modules/flight_mode_manager/FlightModeManagerTest.cpp#L282'
    ],
    [
        'TC-M3-FMM-09',
        'FlightModeManager::handleCommand',
        'Verify vehicle command buffering for DO_ORBIT and in-flight DO_CHANGE_SPEED execution.',
        'Published vehicle_command_s with VEHICLE_CMD_DO_ORBIT and VEHICLE_CMD_DO_CHANGE_SPEED',
        'DO_ORBIT buffered in _current_command; DO_CHANGE_SPEED overrides cruise speed without latching',
        'PASSED',
        'FlightModeManager.cpp (lines 310-336: command handler loop and task parameter override)',
        'src/modules/flight_mode_manager/FlightModeManagerTest.cpp#L302'
    ],
    [
        'TC-M3-FMM-10',
        'FlightModeManager::tryApplyCommandIfAny',
        'Verify 200ms timeout rejection on stale commands and successful application of fresh commands.',
        'Command timestamps: t - 1.0s (> 200ms timeout) vs t (fresh <= 200ms)',
        'Stale command ignored; fresh command applied to active task and cleared to 0',
        'PASSED',
        'FlightModeManager.cpp (lines 289-305: command validity timeout and application)',
        'src/modules/flight_mode_manager/FlightModeManagerTest.cpp#L328'
    ],
    [
        'TC-M3-FMM-11',
        'FlightModeManager::generateTrajectorySetpoint',
        'Verify trajectory setpoints and vehicle constraints publication during landed vs flight states.',
        'Active ManualAltitude task; takeoff_state in {TAKEOFF_STATE_DISARMED, TAKEOFF_STATE_FLIGHT}',
        'uORB topics trajectory_setpoint and vehicle_constraints successfully updated',
        'PASSED',
        'FlightModeManager.cpp (lines 338-377: trajectory setpoint and constraints generation)',
        'src/modules/flight_mode_manager/FlightModeManagerTest.cpp#L347'
    ],
    [
        'TC-M3-FMM-12',
        'FlightModeManager::print_status / usage / custom_command',
        'Verify module diagnostic status reporting, CLI usage printer, and custom command entrypoint.',
        'Invocation of print_status(), print_usage(), and custom_command(\"status\", \"invalid\")',
        'Returns 0 exit codes, outputs formatted CLI usage string and diagnostic logs',
        'PASSED',
        'FlightModeManager.cpp (lines 440-520: status, print_usage, and CLI trampoline)',
        'src/modules/flight_mode_manager/FlightModeManagerTest.cpp#L371'
    ],
    [
        'TC-M3-BAT-01',
        'Battery::updateParams',
        'Verify multi-cell parameter synchronization, cell count parsing, and voltage thresholds.',
        'BAT1_N_CELLS=3, BAT1_V_CHARGED=4.05V, BAT1_V_EMPTY=3.60V',
        'cell_count()==3, full_cell_voltage()==4.05V, empty_cell_voltage()==3.60V',
        'PASSED',
        'battery.cpp (lines 50-100: constructor parameter finding and updateParams)',
        'src/lib/battery/BatteryTest.cpp#L113'
    ],
    [
        'TC-M3-BAT-02',
        'Battery::updateVoltage (Disconnect Threshold)',
        'Verify sub-recognition voltage (< 2.1V) transitions battery to disconnected state.',
        'Voltage input: 1.5V (< LITHIUM_BATTERY_RECOGNITION_VOLTAGE)',
        'status.connected==false, warning==WARNING_NONE, time_remaining_s is NaN',
        'PASSED',
        'battery.cpp (lines 115-125: recognition voltage boundary and disconnect latch)',
        'src/lib/battery/BatteryTest.cpp#L125'
    ],
    [
        'TC-M3-BAT-03',
        'Battery::updateBatteryStatus (Connection & SoC)',
        'Verify valid voltage connection (12.0V) initializes state of charge and filter pipeline.',
        '3S battery, voltage=12.0V (4.0V/cell), current=2.0A, temp=25.0C',
        'status.connected==true, voltage_v > 11.0V, remaining in [0.5, 1.0], warning==WARNING_NONE',
        'PASSED',
        'battery.cpp (lines 126-170: connection filter initialization and SoC estimation)',
        'src/lib/battery/BatteryTest.cpp#L139'
    ],
    [
        'TC-M3-BAT-04',
        'Battery::determineWarning',
        'Verify multi-tier warning ladder escalation based on remaining state of charge.',
        'SoC values: 0.02 (emergency < 5%), 0.06 (critical < 7%), 0.10 (low < 15%), 0.50 (normal >= 15%)',
        'Exact warning codes: WARNING_EMERGENCY, WARNING_CRITICAL, WARNING_LOW, WARNING_NONE',
        'PASSED',
        'battery.cpp (lines 305-325: determineWarning threshold comparator)',
        'src/lib/battery/BatteryTest.cpp#L160'
    ],
    [
        'TC-M3-BAT-05',
        'Battery::determineFaults',
        'Verify overvoltage spike fault detection under dangerous pack voltage conditions.',
        '3S pack; normal 12.0V vs overvoltage 14.0V (> 3 * 4.05 * 1.05 = 12.7575V)',
        '12.0V returns faults==0; 14.0V flags bit FAULT_SPIKES in status.faults',
        'PASSED',
        'battery.cpp (lines 327-340: determineFaults overvoltage spike check)',
        'src/lib/battery/BatteryTest.cpp#L176'
    ],
    [
        'TC-M3-BAT-06',
        'Battery::computeScale',
        'Verify dynamic thrust compensation scaling across battery discharge envelope.',
        'Full pack 12.6V (scale ~1.0) vs discharged pack 10.0V (> 2s filter settling)',
        'Scale constrained in [1.0, 1.3] (maximum 30% motor compensation gain)',
        'PASSED',
        'battery.cpp (lines 342-355: computeScale alpha filter and clamp)',
        'src/lib/battery/BatteryTest.cpp#L194'
    ],
    [
        'TC-M3-BAT-07',
        'Battery::calculateStateOfChargeVoltageBased',
        'Verify recursive least squares load drop compensation on cell voltage estimation.',
        '3S pack, 11.1V with no load (0A) vs heavy load (15A)',
        'soc_with_load > soc_no_load due to internal resistance * current offset addition',
        'PASSED',
        'battery.cpp (lines 212-250: load drop compensation and RLS update)',
        'src/lib/battery/BatteryTest.cpp#L217'
    ],
    [
        'TC-M3-BAT-08',
        'Battery::sumDischarged & estimateStateOfCharge',
        'Verify coulomb counting integration reducing state of charge over sustained discharge.',
        'Capacity=2200mAh, 10A current draw over 10 seconds (~27.78 mAh integrated)',
        'status.discharged_mah > 0, status.remaining drops proportionally below initial value',
        'PASSED',
        'battery.cpp (lines 195-210: sumDischarged dt integration and SoC fusion)',
        'src/lib/battery/BatteryTest.cpp#L235'
    ],
    [
        'TC-M3-BAT-09',
        'Battery::computeRemainingTime',
        'Verify flight time remaining computation for rotary-wing armed flight and fixed-wing level cruise.',
        'Armed rotary-wing at 10A draw, followed by transition to fixed-wing level flight at 8A',
        'time_remaining_s is finite and positive in both flight regimes',
        'PASSED',
        'battery.cpp (lines 380-425: remaining time filter reset and capacity projection)',
        'src/lib/battery/BatteryTest.cpp#L261'
    ],
    [
        'TC-M3-BAT-10',
        'Battery::setStateOfCharge (Override)',
        'Verify external state of charge injection overriding internal voltage/coulomb estimators.',
        'setStateOfCharge(0.68f) with voltage 11.0V update',
        'status.remaining reflects exactly 0.68 with _external_state_of_charge latch',
        'PASSED',
        'battery.cpp (lines 140-150: external SoC flag bypass in updateBatteryStatus)',
        'src/lib/battery/BatteryTest.cpp#L302'
    ],
    [
        'TC-M3-BAT-11',
        'AnalogBattery::updateBatteryStatusADC',
        'Verify analog ADC pin sampling, divider scaling, and voltage channel validation.',
        'BAT1_V_DIV=10.0, BAT1_A_PER_V=20.0; ADC inputs: raw_v=1.5V (15.0V), raw_i=0.5V (10.0A)',
        'status.connected==true, status.voltage_v > 0, is_valid()==true',
        'PASSED',
        'analog_battery.cpp (lines 60-120: ADC scaling, filter update, and channel query)',
        'src/lib/battery/BatteryTest.cpp#L313'
    ]
]

# Sheet 2: MC/DC Evidence (Structure preserved for Member 2)
ws2 = wb.create_sheet(title='MC-DC Evidence')
headers_ws2 = [
    'Decision ID',
    'Component / Decision Description',
    'Compound Decision Boolean Expression',
    'Conditions',
    'Truth Table Vector (Conditions -> Outcome)',
    'Independence Pair Selected',
    'Demonstrated Condition',
    'Test Reference / Method'
]

# Style Definitions
header_fill = PatternFill(start_color='1F497D', end_color='1F497D', fill_type='solid')
header_font = Font(name='Calibri', size=11, bold=True, color='FFFFFF')
data_font = Font(name='Calibri', size=10)
pass_fill = PatternFill(start_color='C6EFCE', end_color='C6EFCE', fill_type='solid')
pass_font = Font(name='Calibri', size=10, color='006100', bold=True)
thin_border = Border(
    left=Side(style='thin', color='D9D9D9'),
    right=Side(style='thin', color='D9D9D9'),
    top=Side(style='thin', color='D9D9D9'),
    bottom=Side(style='thin', color='D9D9D9')
)

# Populate Sheet 1
ws1.append(headers_ws1)
for col_num in range(1, len(headers_ws1) + 1):
    cell = ws1.cell(row=1, column=col_num)
    cell.fill = header_fill
    cell.font = header_font
    cell.alignment = Alignment(horizontal='center', vertical='center', wrap_text=True)

for row_data in rows_ws1:
    ws1.append(row_data)
    row_num = ws1.max_row
    for col_num in range(1, len(row_data) + 1):
        cell = ws1.cell(row=row_num, column=col_num)
        cell.font = data_font
        cell.border = thin_border
        if col_num == 1:
            cell.alignment = Alignment(horizontal='center', vertical='center')
        elif col_num == 6:
            cell.fill = pass_fill
            cell.font = pass_font
            cell.alignment = Alignment(horizontal='center', vertical='center')
        else:
            cell.alignment = Alignment(horizontal='left', vertical='center', wrap_text=True)

# Auto-adjust column widths for Sheet 1
col_widths_ws1 = [16, 26, 38, 36, 36, 14, 38, 42]
for idx, width in enumerate(col_widths_ws1, start=1):
    ws1.column_dimensions[get_column_letter(idx)].width = width

# Populate Sheet 2 Header
ws2.append(headers_ws2)
for col_num in range(1, len(headers_ws2) + 1):
    cell = ws2.cell(row=1, column=col_num)
    cell.fill = header_fill
    cell.font = header_font
    cell.alignment = Alignment(horizontal='center', vertical='center', wrap_text=True)

col_widths_ws2 = [14, 28, 40, 24, 34, 22, 22, 36]
for idx, width in enumerate(col_widths_ws2, start=1):
    ws2.column_dimensions[get_column_letter(idx)].width = width

wb.save('Testing_Workbook.xlsx')
print('Successfully generated Testing_Workbook.xlsx with exactly 2 sheets.')
