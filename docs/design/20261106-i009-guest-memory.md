# 설계: 곡 선택 그림, 본게임 BGA·음악, GAME OVER 영상 — 게스트 메모리 고갈 (issue #9)

상태: **완료**(rexbox #67~#69, #73: 본게임 BGA, STAGE BREAK·GAME OVER 영상, 잠긴 곡 그림). 음악은 재현되지 않아 닫았다(재현되면 다시 연다). 작업 지시: [20261106-i009](../work-orders/20261106-i009-guest-memory.md) · 작업 로그: [20261106-i009](../work-logs/20261106-i009-guest-memory.md)

## 1. 요구사항

v0.0.4 뒤 보고된 증상:
1. 곡 선택에서 잠긴 곡의 그림이 깨진다.
2. 본게임 배경(BGA 동영상)이 나오지 않고, 음악이 나오지 않을 때가 있다.
3. 실패 때의 GAME OVER 영상이 나오지 않는다.

## 2. 원인 (확인됨)

* 화면 몇 개 만에 게스트 메모리가 바닥났다(`PsCreateSystemThreadEx guest memory exhausted creating a thread`). 그래서 동영상 디코더와 스트림을 만들지 못했다.
* 타이틀은 텍스처를 인라인으로 만든다(`0x00076D50`). 헤더와 데이터는 `XMemAlloc`으로 할당하고, 놓을 때는 `D3DResource_Release` → `D3D_DestroyResource` → `XMemFree(data | 0x80000000)`를 거친다.
* rexbox에서는 두 군데가 막혀 있었다:
  * `Release`와 `D3D_DestroyResource`가 아무것도 해제하지 않았다([#67](https://github.com/reexec/rexbox/issues/67)).
  * `0x80000000` 창 주소로 들어온 해제가 블록을 찾지 못했다([#68](https://github.com/reexec/rexbox/issues/68)).
* 곡이 끝난 뒤 저장할 때 `NtQueryFullAttributesFile`이 없어 멈췄다([#69](https://github.com/reexec/rexbox/issues/69)).

## 3. 설계

* 프로파일 `kXdkInternals`에 `{"D3D_DestroyResource", 0x00177E60}`을 더한다. rexbox는 타이틀이 만든 리소스의 마지막 참조가 사라지면 이 함수(라이브러리 본문)를 부른다.
* submodule을 rexbox #67~#69로 올린다.

## 4. 검증

창 표시, 오디오, 키보드로 곡 선택 → 본게임 → GAME OVER → SELECT MODE를 지나며 메모리 고갈이 없는지, BGA와 GAME OVER 영상이 그려지는지 본다. 잠긴 곡 그림은 키 입력 주입으로 곡 목록을 넘기지 못해 사용자 확인이 필요하다.

---

# Design: song-select pictures, the in-game BGA and music, the game-over video — guest memory runs out (issue #9)

Status: **done** (rexbox #67–#69 and #73: the in-game BGA, the STAGE BREAK and GAME OVER videos, the locked-song pictures). The music did not reproduce and the issue is closed (to be reopened if it does). Work order: [20261106-i009](../work-orders/20261106-i009-guest-memory.md) · Work log: [20261106-i009](../work-logs/20261106-i009-guest-memory.md)

**Requirement.** Reported after v0.0.4:
1. Locked songs' pictures on the song select come out broken.
2. The in-game background (the BGA movie) does not show, and the music sometimes does not play.
3. The GAME OVER video does not show on a failure.

**Cause (confirmed).**
- Guest memory ran out after a few screens (`PsCreateSystemThreadEx guest memory exhausted creating a thread`), so movie decoders and streams could not be created.
- The title creates textures inline (`0x00076D50`). It allocates the header and the data with `XMemAlloc` and drops them through `D3DResource_Release` → `D3D_DestroyResource` → `XMemFree(data | 0x80000000)`.
- Two places in rexbox blocked that:
  - `Release` and `D3D_DestroyResource` freed nothing ([#67](https://github.com/reexec/rexbox/issues/67));
  - a free through the `0x80000000` window found no block ([#68](https://github.com/reexec/rexbox/issues/68)).
- The save after a song then stopped at the missing `NtQueryFullAttributesFile` ([#69](https://github.com/reexec/rexbox/issues/69)).

**Design.** The profile's `kXdkInternals` gains `{"D3D_DestroyResource", 0x00177E60}`, which rexbox calls (the library body) when a title-made resource loses its last reference. The submodule moves to rexbox #67–#69.

**Verification.** With the window shown, audio and the keyboard, go through song select → gameplay → GAME OVER → SELECT MODE. Memory must not run out, and the BGA and the GAME OVER video must draw. The locked-song pictures need the user's check, since the injected keys do not scroll the song list.
