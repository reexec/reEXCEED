# 작업 로그: 음악 로더 스레드 순서 어댑터 (issue #9 후속)

* 설계: [`docs/design/20261123-i009-music-loader-race.md`](../design/20261123-i009-music-loader-race.md)
* 작업 지시: [`docs/work-orders/20261123-i009-music-loader-race.md`](../work-orders/20261123-i009-music-loader-race.md)
* 구현: Antigravity(`05864dd`). 조사·설계·리뷰·검증: Claude.

## 경과

1. 곡 음악 무음이 다시 재현되었다. `REXBOX_DSOUND_TRACE`/`REXBOX_AUDIO_DUMP`와 로그 파일 쓰기를 켜면 재현되지 않았다. 로그를 콘솔로만 내보내자 재현되었다.
2. 재현된 실행의 DSOUND 기록: 세 번째 `FlushEx` 뒤로 스트림 `Process`가 없고 효과음 `Play`도 없다. `music.wav`는 50초 중 6초만 소리가 있다. 정상 기록(v0.0.7)은 flush 뒤에도 계속 약 22패킷을 쌓아 둔다.
3. 무음일 때 받을 진단으로 rexbox #89(F10 스레드 보고)를 만들었다. 보고에서 막힌 스레드는 없었다. 로더 스레드 5, 6은 끝났고, 정상 실행과 달리 로더 7번 이후가 생기지 않았다.
4. 게임 코드에서 원인을 찾았다. 음악 시작 함수 세 곳이 `CreateThread(sub_0007DFD0)` **뒤에** "로딩 중" `[0x843550] = 1`을 쓰고, 로더는 끝에서 0을 쓴다. 호스트에서 로더가 먼저 끝나면 1이 남는다(설계 문서).
5. 설계와 작업 지시를 쓰고 Antigravity가 구현했다(`05864dd`). 어댑터 `decomp/music_loader_order.c`, 훅 `reexceed_host_yield`·`reexceed_host_milliseconds`, 검사 `TestMusicLoaderOrder`, 문서를 바꾸었다.

## 리뷰

* 설계와 일치한다. 플래그가 1이면 바로 진행하고, 아니면 양보하며 100 ms까지 기다린 뒤 lift 본문을 부른다. 시계 훅이 없으면 양보 100,000회를 상한으로 한다. 대기 중에는 게스트 레지스터와 메모리를 건드리지 않는다.
* 로더 `sub_0007DFD0`은 세 시작 함수의 `CreateThread` 인자로만 쓰이고 직접 호출은 없다(디스어셈블 확인). 어댑터 한 곳으로 세 경로를 모두 덮는다.
* 직전 로더가 실패해 1이 남은 경우에는 바로 진행한다. 원본과 같다.

## 검증

| 항목 | 결과 |
|---|---|
| Win32 x86 Release, Debug | 오류 0, 경고 0, CTest 7/7 (Claude가 다시 실행) |
| `reexceed_decomp_check` | 3 case 0 difference. `TestUnlockAll`, `TestAutoplay`, `TestFullSizePicture`, `TestMusicLoaderOrder` 통과 |
| 실행(프레임 3600) | 타이틀 화면, 로더 스레드 5가 타이틀 음악을 엶 |
| 사용자 재현 시험 | **남음**: 진단 옵션 없이 곡을 여러 번 플레이해 무음이 사라졌는지 |

---

# Work Log: The music loader thread ordering adapter (issue #9 follow-up)

* Design: [`docs/design/20261123-i009-music-loader-race.md`](../design/20261123-i009-music-loader-race.md)
* Work order: [`docs/work-orders/20261123-i009-music-loader-race.md`](../work-orders/20261123-i009-music-loader-race.md)
* Implementation: Antigravity (`05864dd`). Investigation, design, review and verification: Claude.

## Progress

1. Silent song music came back. It did not happen with `REXBOX_DSOUND_TRACE`/`REXBOX_AUDIO_DUMP` and the log written to a file, and did with the log on the console.
2. The reproducing run's DSOUND record had no stream `Process` and no sound-effect `Play` after the third `FlushEx`. Its `music.wav` had sound in 6 of 50 s. A good record (v0.0.7) keeps about 22 packets queued after flushes.
3. rexbox #89 (the F10 thread report) was made to look at the silence. The report showed no blocked thread. Loader threads 5 and 6 had ended, and unlike a good run no loader 7 or later appeared.
4. The cause is in the title's code. The three music start functions write "loading" `[0x843550] = 1` **after** `CreateThread(sub_0007DFD0)`, and the loader writes 0 at its end. On the host the loader can finish first, leaving 1 (see the design).
5. Design and work order written; Antigravity implemented them (`05864dd`): the adapter `decomp/music_loader_order.c`, the hooks `reexceed_host_yield` and `reexceed_host_milliseconds`, the check `TestMusicLoaderOrder`, and the documents.

## Review

* It matches the design. With the flag at 1 it goes on at once; otherwise it yields for up to 100 ms, then calls the lifted body. Without a clock hook it caps at 100,000 yields. The wait touches neither guest registers nor memory.
* The loader `sub_0007DFD0` is used only as the `CreateThread` argument of the three start functions, never called directly (checked in the disassembly), so one adapter covers all three paths.
* A 1 left by a failed earlier loader lets it go on at once, as the original does.

## Verification

| Item | Result |
|---|---|
| Win32 x86 Release, Debug | 0 errors, 0 warnings, CTest 7/7 (rerun by Claude) |
| `reexceed_decomp_check` | 3 cases, 0 differences; `TestUnlockAll`, `TestAutoplay`, `TestFullSizePicture`, `TestMusicLoaderOrder` pass |
| Run (frame 3600) | the title screen; loader thread 5 opened the title music |
| User reproduction runs | **open**: play songs several times without diagnostic options and check the silence is gone |
