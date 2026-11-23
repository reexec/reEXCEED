# 작업 지시서: 음악 로더 스레드 순서 어댑터 (issue #9 후속)

* 설계: [`docs/design/20261123-i009-music-loader-race.md`](../design/20261123-i009-music-loader-race.md)
* 구현: Antigravity. 리뷰·검증: Claude.
* 브랜치: 현재 작업 브랜치(`i018-xboxrecomp-v0.18`)에서 이어서 한다. 커밋 제목 끝에 `(#9)`.

## 구현 단계

1. **호스트 훅** (`include/reexceed/exceed_host.h`, `src/game/exceed/exceed_host.c`)
   * `extern void (*reexceed_host_yield)(void);`: null이면 아무것도 하지 않는다.
   * 경과 시간: `extern uint32_t (*reexceed_host_milliseconds)(void);`을 두거나, 양보 횟수 상한으로 대신한다. 100 ms 상한이 대략 지켜지면 된다.
   * 기존 `reexceed_host_apu_sample_count`와 같은 모양(정의는 `exceed_host.c`, 기본 0)과 주석 형식을 따른다.
2. **앱 연결** (`src/app/main.cpp`): 다른 훅을 연결하는 곳(`reexceed_host_apu_sample_count = ...` 근처)에서 `std::this_thread::yield`를 부르는 함수와 시간 함수를 연결한다.
3. **어댑터** (`src/game/exceed/decomp/music_loader_order.c`, 새 파일)
   * `extern void sub_0007DFD0_gen(void);`을 선언하고 `void sub_0007DFD0(void)`을 정의한다.
   * 동작:
     1. `MEM8(0x00843550u) != 1`이면 1이 될 때까지 `reexceed_host_yield()`로 기다린다. 상한(100 ms)을 넘으면 그만 기다린다.
     2. `sub_0007DFD0_gen()`을 부른다(레지스터·스택은 건드리지 않는다. 진입 상태 그대로 lift 본문에 넘긴다).
   * 대기 중에는 게스트 레지스터(`g_esp` 등)와 게스트 메모리를 바꾸지 않는다.
   * 검사용으로 본문 호출을 정적 함수 포인터로 빼고(기본 `sub_0007DFD0_gen`), 검사 도구에서 바꿀 수 있게 setter를 둔다. 기존 `full_size_picture.c`, `unlock_all.c`의 주석 형식(무엇·왜·근거 문서)을 따른다. 주석은 영어로만 쓴다.
   * `docs/CODING_STYLE.md`의 디컴파일 규약을 따른다. 본문을 마지막에 부르는 래퍼이므로 EBP·플래그 게시는 본문에 맡긴다.
4. `python scripts/recomp/regenerate.py`로 `sub_0007DFD0_gen`을 만든다.
5. **검사** (`tools/decomp_check/main.cpp`): `TestMusicLoaderOrder`
   * 본문 대신 호출 여부만 기록하는 검사 함수를 넣는다.
   * 플래그 1이면 바로 호출되는지 확인한다.
   * 플래그 0이면 상한 근처까지 기다린 뒤 호출되는지 확인한다. 시간이나 양보 횟수로 본다.
   * 끝나면 원래 본문 포인터로 되돌린다.
6. **문서**: `src/game/exceed/decomp/README.md` 표에 한 줄을 넣는다. `ARCHITECTURE.md`의 `decomp/` 목록과 호스트 훅 목록, `docs/analysis/exceed-xbe-overview.md`에 음악 시작·로더·`[0x843550]` 항목(확인됨/추정 구분)을 넣는다.
7. Win32 x86 Release·Debug 빌드(경고 0), `ctest -C Release`, `ctest -C Debug`.

## Claude가 할 일

* 위 결과를 리뷰한다(설계와의 일치, 원본 동작 보존, 경계 조건).
* 빌드·테스트를 다시 돌리고, 사용자 재현 시험 결과를 작업 로그(`docs/work-logs/20261123-i009-music-loader-race.md`)에 적는다.

---

# Work Order: The music loader thread ordering adapter (issue #9 follow-up)

* Design: [`docs/design/20261123-i009-music-loader-race.md`](../design/20261123-i009-music-loader-race.md)
* Implementation: Antigravity. Review and verification: Claude.
* Branch: continue on the current task branch (`i018-xboxrecomp-v0.18`); commit titles end with `(#9)`.

## Implementation steps

1. **Host hooks** (`include/reexceed/exceed_host.h`, `src/game/exceed/exceed_host.c`)
   * `extern void (*reexceed_host_yield)(void);`: null does nothing.
   * Elapsed time: either `extern uint32_t (*reexceed_host_milliseconds)(void);` or a cap on the yield count; the 100 ms cap only needs to hold roughly.
   * Follow the shape of the existing `reexceed_host_apu_sample_count` (defined in `exceed_host.c`, default 0) and its comment style.
2. **App wiring** (`src/app/main.cpp`): where the other hooks are set (near `reexceed_host_apu_sample_count = ...`), set a function calling `std::this_thread::yield` and the time function.
3. **Adapter** (`src/game/exceed/decomp/music_loader_order.c`, new)
   * Declare `extern void sub_0007DFD0_gen(void);` and define `void sub_0007DFD0(void)`.
   * Behaviour:
     1. While `MEM8(0x00843550u) != 1`, wait with `reexceed_host_yield()`; stop waiting past the cap (100 ms).
     2. Call `sub_0007DFD0_gen()`, leaving registers and stack untouched (the lifted body gets the entry state as is).
   * The wait changes neither guest registers (`g_esp`, ...) nor guest memory.
   * For the check, put the body call behind a static function pointer (default `sub_0007DFD0_gen`) with a setter the check tool can use.
   * Follow the comment style of `full_size_picture.c` and `unlock_all.c` (what, why, the documents behind it). Comments are English only.
   * Follow the decompiled-code convention in `docs/CODING_STYLE.md`: as a wrapper calling the body last, it leaves the EBP and flags publishing to the body.
4. Run `python scripts/recomp/regenerate.py` to emit `sub_0007DFD0_gen`.
5. **Check** (`tools/decomp_check/main.cpp`): `TestMusicLoaderOrder`
   * Install a stand-in body that only records the call.
   * Check that with the flag at 1 it is called at once.
   * Check that with the flag at 0 it is called after waiting about the cap (by time or yield count).
   * Restore the real body pointer afterwards.
6. **Documents**:
   * Add a row to the table in `src/game/exceed/decomp/README.md`.
   * Add the adapter and the new hooks to the `decomp/` and host hook lists in `ARCHITECTURE.md`.
   * Add the music start functions, the loader and `[0x843550]` to `docs/analysis/exceed-xbe-overview.md`, separating confirmed from inferred.
7. Win32 x86 Release and Debug builds (0 warnings), `ctest -C Release`, `ctest -C Debug`.

## Claude's part

* Review the result: agreement with the design, preservation of the original behaviour, edge cases.
* Rerun the build and tests, and record the user's reproduction runs in the work log (`docs/work-logs/20261123-i009-music-loader-race.md`).
