# Member 2 deliverables — MulticopterLandDetector

The current suite contains **19 functional GTest cases**. All pass in normal order and 20 shuffled repetitions. **17 D1-D3 return-decision independence pairs / 34 runtime vectors** are checked against the production result and recorded in XML. Complete MC/DC of the additional governing guards remains unfinished.

- [Scope record](part1_scope_record.md)
- [Corrected MC/DC analysis and remaining guard obligations](part2_mcdc_analysis.md)
- [Current MC/DC matrix](sheet2_mcdc_evidence.csv)
- [Measured coverage and limits](part3_coverage_evidence.md)
- [Scoped quality judgment](part4_final_quality_judgment.md)
- [Current execution/XML/validation evidence](logs/merge_main)
- [Current coverage HTML](coverage/merge_main/html/index.html)
- [Merge decisions and fresh verification](../assignment_audit/MAIN_MERGE.md)

```bash
cmake --build build/px4_sitl_test --target functional-LandDetector -j 4
ctest --test-dir build/px4_sitl_test -R '^functional-LandDetector$' --timeout 30 -V
python3 member2_deliverables/scripts/validate_mcdc.py
```

The validator's default XML is the saved current evidence. When checking a new execution, pass its path with `--xml`. The CMake registration includes the functional test and its private link-time clock replacement. The friend declaration is test access only; the production algorithms have not been changed.

The old top-level execution log is historical. Current evidence is under `logs/merge_main/`. Generated baseline coverage, remaining guard tests/gap investigation, and final group consolidation still need completion.
