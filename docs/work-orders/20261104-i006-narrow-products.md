# 작업 지시: 8·16비트 단일 피연산자 곱셈의 lift 우회 (issue #6)

설계: [20261104-i006](../design/20261104-i006-narrow-products.md)

1. `scripts/recomp/lifter_patches.py`: `Lifter._lift_imul`/`_lift_muldiv`의 8·16비트 단일 피연산자 형식을 바꾸고, 인자로 받은 모듈을 실행한다.
2. `scripts/recomp/lifter.py`: `run_step(..., patched=True)`로 lift 단계만 패치를 거쳐 실행한다.
3. `scripts/recomp/test_lifter_patches.py`: 바꾸지 않는 형식의 글자 비교, 좁은 형식의 컴파일 실행 비교.
4. upstream `pytest tools/`를 패치 없이와 패치를 적용하고 각각 실행해 비교한다.
5. 다시 lift하고 Win32 x86 Debug·Release를 빌드한 뒤, 프레임 덤프로 ESRB 공지와 로고를 확인한다.
6. sp00nznet/xboxrecomp#277에 이 타이틀의 사례를 남긴다.
7. 작업 로그, 분석 문서(`docs/analysis/exceed-xbe-overview.md`)를 갱신하고 커밋한다(`(#6)`).

---

# Work order: working around the lift of 8- and 16-bit one-operand products (issue #6)

Design: [20261104-i006](../design/20261104-i006-narrow-products.md)

1. `scripts/recomp/lifter_patches.py`: replace the 8- and 16-bit one-operand forms of `Lifter._lift_imul`/`_lift_muldiv` and run the module given as the argument.
2. `scripts/recomp/lifter.py`: `run_step(..., patched=True)` runs only the lift step through the patches.
3. `scripts/recomp/test_lifter_patches.py`: a text comparison of the untouched forms and a compiled comparison of the narrow forms.
4. Run upstream's `pytest tools/` without and with the patches and compare.
5. Re-lift, build Win32 x86 Debug and Release, and check the ESRB notice and the logos in frame dumps.
6. Leave this title's case on sp00nznet/xboxrecomp#277.
7. Update the work log and the analysis document (`docs/analysis/exceed-xbe-overview.md`) and commit (`(#6)`).
