# 작업 로그: 게스트 메모리 고갈 (issue #9)

설계: [20261106-i009](../design/20261106-i009-guest-memory.md) · 작업 지시: [20261106-i009](../work-orders/20261106-i009-guest-memory.md)

## 한 일

1. 재현: Enter 8번(START → 메뉴 → 곡 → 난이도 → 본게임). 본게임 무렵 `PsCreateSystemThreadEx guest memory exhausted creating a thread`가 나왔다. 그다음 저장 단계에서 `NtQueryFullAttributesFile`(import 210)로 멈췄다.
2. 할당 기록(진단용 임시 코드, 커밋하지 않음):
   * 64 KiB 이상 블록 183개(41.6 MiB)가 해제되지 않았다. 모두 `XMemAlloc`(`0x00086824`) → `MmAllocateContiguousMemoryEx`에서 왔다.
   * 가장 큰 몫은 인라인 텍스처 생성(`0x00076D50`, 데이터 `0x00076F3D`)이었다.
3. rexbox #67: `D3D_DestroyResource`와 `Release`가 라이브러리 본문으로 해제한다. 해제 횟수가 5번에서 173번으로 늘었지만 남는 메모리는 그대로였다.
4. rexbox #68: 173번 가운데 168번이 `0x8xxxxxxx`(연속 메모리 창) 주소였다. 창 주소를 접게 하자 고갈이 사라졌다.
5. rexbox #69: `NtQueryFullAttributesFile`(세이브 폴더 `U:\33A446FC39CC\`).
6. `src/game/exceed/exceed_profile.cpp`: `kXdkInternals`에 `D3D_DestroyResource` = `0x00177E60`.

## 그 밖에 찾은 것 (이 issue에서 고치지 않음)

* UIX(Xbox Live UI, `d:\media\UIX.uix`) 객체 `[obj+0x808]`가 0인 채로 그 닫기 함수(`0x000C6705`)가 불린다. `[recomp] indirect call to 8298BC05 is outside code`가 그 흔적이다. 네트워크가 없어 Live 로그인이 실패한 상태로 보인다. `REXBOX_TRACE_FILES`를 켜면 I/O가 느려져 같은 자리에서 fault가 난다(`read at guest C2700008`).

## submodule

* `third_party/rexbox`: rexbox `v0.0.24`(`eec649c`, reexec/rexbox#70의 squash 머지). 브랜치에서의 커밋은 #67 `2286b13`, #68 `ce95a96`, #69 `a514e79`이었다.

## 검증

| 검증 | 결과 |
|---|---|
| Win32 x86 Debug, Release | 오류 0, 경고 0, CTest 7/7(두 구성) |
| `reexceed roms`(창 표시, 오디오, 키보드, Enter 8번) | 본게임 BGA(프레임 5700), GAME OVER 영상(5900), 저장 뒤 SELECT MODE. 181초 동안 고갈과 멈춤 없음 |
| 사용자 확인(2026-11-06) | 본게임 배경(BGA), STAGE BREAK·GAME OVER 영상은 해결. 곡 선택의 잠긴 곡 그림(가운데 미리보기도 검정)과 음악은 그대로 |

## 남은 것

* 잠긴 곡 그림, 가운데 미리보기, 음악: 이 issue에서 이어서 본다.

---

# Work log: guest memory runs out (issue #9)

Design: [20261106-i009](../design/20261106-i009-guest-memory.md) · Work order: [20261106-i009](../work-orders/20261106-i009-guest-memory.md)

Done:
1. Reproduction: Enter eight times (START → menus → song → difficulty → gameplay). Around gameplay `PsCreateSystemThreadEx guest memory exhausted creating a thread` appeared, and the save step then stopped at `NtQueryFullAttributesFile` (import 210).
2. An allocation log (temporary diagnostic code, not committed):
   - 183 blocks of 64 KiB and more (41.6 MiB) were never freed, all from `XMemAlloc` (`0x00086824`) → `MmAllocateContiguousMemoryEx`;
   - most of it was inline texture creation (`0x00076D50`, data at `0x00076F3D`).
3. rexbox #67: `D3D_DestroyResource` and `Release` free through the library body. Frees rose from 5 to 173, but the memory left did not change.
4. rexbox #68: 168 of the 173 frees came in as `0x8xxxxxxx` (the contiguous-memory window). Folding the window removed the exhaustion.
5. rexbox #69: `NtQueryFullAttributesFile` (the save folder `U:\33A446FC39CC\`).
6. `src/game/exceed/exceed_profile.cpp`: `kXdkInternals` gains `D3D_DestroyResource` = `0x00177E60`.

Also found, not fixed here: the UIX (Xbox Live UI, `d:\media\UIX.uix`) object `[obj+0x808]` is 0 when its close (`0x000C6705`) is called; `[recomp] indirect call to 8298BC05 is outside code` is its trace. This looks like the Live sign-in having failed for want of a network. With `REXBOX_TRACE_FILES` the slower I/O makes the same spot fault (`read at guest C2700008`).

Submodule: `third_party/rexbox` is at rexbox `v0.0.24` (`eec649c`, the squash merge of reexec/rexbox#70); on the branch the commits were #67 `2286b13`, #68 `ce95a96` and #69 `a514e79`.

Verification:
- Win32 x86 Debug and Release: 0 errors and 0 warnings, CTest 7/7 in both.
- `reexceed roms` (window shown, audio, keyboard, Enter eight times): the in-game BGA (frame 5700), the GAME OVER video (5900) and SELECT MODE after the save, with no exhaustion and no stop in 181 s.
- The user's check (2026-11-06): the in-game background (BGA) and the STAGE BREAK and GAME OVER videos are fixed. The locked-song pictures (the centre preview is black too) and the music are unchanged.

Remaining: the locked-song pictures, the centre preview and the music, to be continued under this issue.
