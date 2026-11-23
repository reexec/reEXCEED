# 작업 로그: 화면 셰이더 설정 (issue #20)

* 설계: [`docs/design/20261124-i020-screen-shaders.md`](../design/20261124-i020-screen-shaders.md), 작업 지시: [`docs/work-orders/20261124-i020-screen-shaders.md`](../work-orders/20261124-i020-screen-shaders.md)
* 구현: Antigravity(`91d05d9`). 설계·리뷰·검증: Claude. 기능 자체: rexbox #91(`d2a3b19`, 브랜치 `i091-screen-shaders`).

## 경과

1. 사용자 요청: reDMP1(v0.0.25, #49)에 넣은 화면 셰이더를 같게 넣는다. 게임과 무관한 부분은 rexbox #91, 설정 연결은 이 issue로 나누었다.
2. Antigravity가 구현했다.
   * `Settings::post_shader`와 `[Graphics] PostShader`.
   * `--post-shader <id>`/`=<id>`(빈 값은 사용법 오류), `REEXCEED_POST_SHADER`.
   * `ChoosePostShader`(명령줄 > 환경 변수 > ini > `none`), 시작 로그.
   * F1 Graphics 탭 `Screen shader`(콤보, Reload, 오류, 슬라이더, 기본값).
   * `reexceed_settings_probe`(CTest), `README.md`, `ARCHITECTURE.md`.

## 리뷰

* 명령줄·환경 변수 값은 `settings.post_shader`에 넣지 않으므로 ini에 쓰이지 않는다. 메뉴에서 고른 값만 저장된다.
* 셰이더 디렉터리와 시작 셰이더는 장치가 만들어지기 전에 rexbox에 넘긴다.

## 검증

| 항목 | 결과 |
|---|---|
| Win32 x86 Release, Debug | 오류 0, 경고 0, CTest 9/9 |
| 실행 `--post-shader crt` | 시작 로그 `[app] post shader: crt (directory shaders)`, 타이틀 화면에 CRT 셰이더가 똑바로, 오버레이는 셰이더 밖, 60 fps |
| 실행 `--post-shader none` | 이전과 같은 화면 |
| 사용자 확인 | 창에서 셰이더가 잘 동작한다(2026-11-24) |

---

# Work Log: Screen Shader Setting (issue #20)

* Design: [`docs/design/20261124-i020-screen-shaders.md`](../design/20261124-i020-screen-shaders.md), work order: [`docs/work-orders/20261124-i020-screen-shaders.md`](../work-orders/20261124-i020-screen-shaders.md)
* Implemented by Antigravity (`91d05d9`); designed, reviewed and verified by Claude. The feature itself: rexbox #91 (`d2a3b19`, branch `i091-screen-shaders`).

## Progress

1. User request: the screen shaders reDMP1 has (v0.0.25, #49), the same way. The game-independent part is rexbox #91; the settings are this issue.
2. Antigravity implemented:
   * `Settings::post_shader` and `[Graphics] PostShader`;
   * `--post-shader <id>`/`=<id>` (an empty value is a usage error) and `REEXCEED_POST_SHADER`;
   * `ChoosePostShader` (command line > environment > ini > `none`) and the start log line;
   * the F1 Graphics tab's `Screen shader` section (combo, Reload, error, sliders, defaults);
   * `reexceed_settings_probe` (CTest), `README.md` and `ARCHITECTURE.md`.

## Review

* A command-line or environment value never goes into `settings.post_shader`, so it is never written to the ini; only a menu choice is saved.
* The shader directory and the initial shader reach rexbox before the device is created.

## Verification

* Win32 x86 Release and Debug: 0 errors, 0 warnings, CTest 9/9.
* A run with `--post-shader crt`: the start log says `[app] post shader: crt (directory shaders)`, the CRT shader is upright on the title screen, the overlay is unshaded, and it holds 60 fps.
* A run with `--post-shader none`: the same picture as before.
* User check (2026-11-24): the shaders work well in the window.
