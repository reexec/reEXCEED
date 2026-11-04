# 작업 로그: 8·16비트 단일 피연산자 곱셈의 lift 우회 (issue #6)

설계: [20261104-i006](../design/20261104-i006-narrow-products.md) · 작업 지시: [20261104-i006](../work-orders/20261104-i006-narrow-products.md)

## 한 일

1. 추적: 프레임 덤프에서 ESRB 구간(프레임 약 60~250)에 검은 테두리만 보였다. 프레임 캡처에는 높이 0인 텍스처 사각형이 하나 있었고, 그 사각형은 원근 투영으로 그려졌다. RAM 덤프(진단용 임시 코드, 커밋하지 않음)에서 ESRB 상태 객체(vtable `0x001B72A4`)를 찾았다. 그 BGA 핸들의 레이어와 키프레임은 정상이었다(위치 (320, 240), 배율 1.25×1, 알파 0→1→1→0). 이미지 항목의 사각형은 (0, 0)–(512, 0)이었고, 텍스처 표(`[0x1D1B44]`)의 `esrb.tga`는 512×0이었다. `REXBOX_WATCH32`로 쓰는 시점을 잡아 libpng 경로(`sub_00022840` → `png_get_IHDR` `0x000DC7B5` → `png_handle_IHDR` `0x000E0737`)까지 따라갔다. RAM의 PNG IHDR는 512×64였다.
2. 원인: `png_handle_IHDR`의 `imul bl`(`0x000E08F8`). upstream v0.16.0은 이것을 `edx:eax = eax × LO8(ebx)`로 옮겨, EDX에 있던 높이를 0으로 덮어쓴다. upstream에 같은 수정을 담은 PR [#277](https://github.com/sp00nznet/xboxrecomp/pull/277)(OPEN)이 있다.
3. `scripts/recomp/lifter_patches.py`(새 파일): 8·16비트 단일 피연산자 `imul`/`mul`을 제 폭으로 옮긴다. 나머지 형식은 upstream 메서드가 그대로 처리한다. `scripts/recomp/lifter.py`: `run_step(..., patched=True)`로 lift 단계만 이것을 거친다.
4. `scripts/recomp/test_lifter_patches.py`(새 파일): 자체 검사.
5. 다시 lift했다(lift 64초, 패치된 형식은 예를 들어 `0x000E08EF`의 `int32_t _r = (int32_t)(int8_t)eax * (int32_t)(int8_t)(LO8(ebx)); SET_LO16(eax, _r);`). 그리고 Release로 빌드했다.

## 검증

| 검증 | 결과 |
|---|---|
| `py -3 scripts/recomp/test_lifter_patches.py` | 바꾸지 않는 형식 11/11이 upstream과 같음, 좁은 형식 7개 5,110/5,110 벡터가 x86과 같음(MSVC). 패치가 없으면 실패 |
| upstream `pytest tools/`(`--continue-on-collection-errors`) | 패치 없이와 패치를 적용하고 둘 다 820 passed, 112 skipped, 1 failed(`test_runtime_template_exports_shared_fpu_state`), 1 error(`pefile` 모듈 없음). 실패 목록이 같다 |
| `reexceed roms`(Release, hidden, no audio), 프레임 덤프 150/400/600/850 | ESRB 공지, AM 로고, Andamiro, Mastiff 로고가 화면 가운데에 그려진다 |

## 남은 것

* upstream이 #277을 담은 릴리스를 내면 고정 버전을 올리고 우회를 지운다.
* 타이틀에서 START를 누르면 멈추는 것은 이 issue와 별개다(후속 issue).

---

# Work log: working around the lift of 8- and 16-bit one-operand products (issue #6)

Design: [20261104-i006](../design/20261104-i006-narrow-products.md) · Work order: [20261104-i006](../work-orders/20261104-i006-narrow-products.md)

Done: 1) Tracing: the frame dumps showed only the black borders during the ESRB stretch (frames about 60–250); the frame capture had one zero-height textured quad, drawn through a perspective projection; a RAM dump (temporary diagnostic code, not committed) gave the ESRB state object (vtable `0x001B72A4`), whose BGA handle's layer and keyframes were sane (position (320, 240), scale 1.25x1, alpha 0→1→1→0), the image entry's rectangle (0, 0)–(512, 0) and the texture table's (`[0x1D1B44]`) `esrb.tga` at 512x0; `REXBOX_WATCH32` caught the write, and the libpng path (`sub_00022840` → `png_get_IHDR` `0x000DC7B5` → `png_handle_IHDR` `0x000E0737`) followed; the PNG's IHDR in RAM says 512x64. 2) Cause: `imul bl` in `png_handle_IHDR` (`0x000E08F8`), lifted by upstream v0.16.0 as `edx:eax = eax x LO8(ebx)`, which overwrites the height held in EDX with 0; upstream PR [#277](https://github.com/sp00nznet/xboxrecomp/pull/277) (open) carries the same fix. 3) `scripts/recomp/lifter_patches.py` (new) lifts the 8- and 16-bit one-operand `imul`/`mul` at their own width and leaves every other form to the upstream methods; `scripts/recomp/lifter.py` runs only the lift step through it (`run_step(..., patched=True)`). 4) `scripts/recomp/test_lifter_patches.py` (new), the self-check. 5) Re-lifted (lift 64 s; a patched form, at `0x000E08EF` for example, is `int32_t _r = (int32_t)(int8_t)eax * (int32_t)(int8_t)(LO8(ebx)); SET_LO16(eax, _r);`) and built Release.

Verification, as in the table above: the self-check finds the 11 untouched forms identical to upstream and the 7 narrow forms matching x86 on 5,110/5,110 vectors (MSVC), and fails without the patches; upstream's `pytest tools/` (`--continue-on-collection-errors`) gives the same result without and with the patches: 820 passed, 112 skipped, 1 failed (`test_runtime_template_exports_shared_fpu_state`), 1 error (no `pefile` module); `reexceed roms` (Release, hidden, no audio) draws the ESRB notice and the AM, Andamiro and Mastiff logos in the centre of the screen in the frame dumps at 150/400/600/850.

Remaining: when upstream releases #277, move the pin and drop the workaround. The stop after pressing START at the title is a separate problem (a follow-up issue).
