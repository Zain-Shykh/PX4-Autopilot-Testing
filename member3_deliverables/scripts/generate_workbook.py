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

# Keep the spreadsheet synchronized with the editable inventory and current source references.
from pathlib import Path
import csv

OUT = Path(__file__).resolve().parents[1]
with (OUT / 'sheet1_test_inventory.csv').open(newline='') as stream:
    inventory = csv.DictReader(stream)
    assert inventory.fieldnames == headers_ws1
    rows_ws1 = [[row[field] for field in headers_ws1] for row in inventory]
assert len(rows_ws1) == 23

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

wb.save(OUT / 'Testing_Workbook.xlsx')
print('Successfully generated Testing_Workbook.xlsx with exactly 2 sheets.')
