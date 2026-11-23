# 작업 지시서: 화면 셰이더 설정 (issue #20)

* 설계: [`docs/design/20261124-i020-screen-shaders.md`](../design/20261124-i020-screen-shaders.md)
* 구현: Antigravity. 리뷰·검증: Claude.
* 순서: rexbox #91(`third_party/rexbox`, 브랜치 `i091-screen-shaders`)을 먼저 끝낸 뒤 시작한다.
* 브랜치: reEXCEED `i020-screen-shaders`(이미 있음). 커밋 제목 끝에 `(#20)`. push, VERSION, submodule 포인터 커밋은 하지 않는다.
* 참고(읽기만): `E:\MYWORK\Projects\reexec\reDMP1` 커밋 `b328e9a`의 앱 변경(`git -C E:\MYWORK\Projects\reexec\reDMP1 show b328e9a -- src/app`).

## 단계

1. `src/app/settings.{h,cpp}`: `Settings::post_shader`(기본 `"none"`), `[Graphics] PostShader` 읽기·쓰기. 주석 한 줄(`; PostShader: none, crt, scanline, fsr, scalefx, anime4k, or a file in shaders/.`).
2. `src/app/main.cpp`:
   * `--post-shader <id>`와 `--post-shader=<id>`를 해석한다. 값이 없거나 비면 사용법 오류로 끝낸다.
   * `REEXCEED_POST_SHADER`를 읽는다.
   * `ChoosePostShader`로 고르고 `SetPostShaderDirectory(ResolvePostShaderDirectory(실행 파일 디렉터리))`, `SetInitialPostShader(id)`를 장치가 만들어지기 전에 부른다.
   * 시작 로그 `[app] post shader: <id> (directory <경로>)`를 남기고, 사용법 출력에 옵션을 더한다.
   * 명령줄·환경 변수 값이 ini를 덮어쓰지 않게 한다. 메뉴에서 고른 값만 저장한다.
3. `src/app/settings_ui.{h,cpp}`: Graphics 탭에 `Screen shader` 구역을 둔다. reDMP1 `DrawPostShaderMenu` 구성대로 만들고 기존 탭 코드 스타일(`SeparatorText`, `(?)` 툴팁)을 따른다. 고르면 `settings_.post_shader`를 갱신하고 `Save()`.
4. 설정 검사: 우선순위와 ini 왕복을 CTest로 확인한다(reDMP1 `settings_probe.cpp` 참고, 기존 `src/app/CMakeLists.txt`의 smoke 시험 옆).
5. 문서: `README.md`(Screen shader 메뉴, `--post-shader`, `REEXCEED_POST_SHADER`, `shaders/`, `PostShader`)와 `ARCHITECTURE.md`(앱 설정). 한국어 먼저, 영어. 소스 주석은 영어만.
6. 빌드·시험: Win32 x86 Release·Debug 경고 0, `ctest -C Release`, `ctest -C Debug` 통과.

## 보고

`%TEMP%\reexceed-coder\i020-screen-shaders.txt`에 바꾼 파일, 빌드·시험 요약, 커밋 id, 설계와 다르게 한 점과 질문을 쓰고 턴을 끝낸다.

---

# Work Order: Screen Shader Setting (issue #20)

* Design: [`docs/design/20261124-i020-screen-shaders.md`](../design/20261124-i020-screen-shaders.md)
* Implementation: Antigravity. Review and verification: Claude.
* Order: start after rexbox #91 (`third_party/rexbox`, branch `i091-screen-shaders`) is done.
* Branch: reEXCEED `i020-screen-shaders` (exists). Commit titles end with `(#20)`. No push, VERSION or submodule pointer commit.
* Reference (read only): the app changes of reDMP1 commit `b328e9a` (`git -C E:\MYWORK\Projects\reexec\reDMP1 show b328e9a -- src/app`).

## Steps

1. `src/app/settings.{h,cpp}`: `Settings::post_shader` (default `"none"`); read and write `[Graphics] PostShader` with a one-line comment (`; PostShader: none, crt, scanline, fsr, scalefx, anime4k, or a file in shaders/.`).
2. `src/app/main.cpp`:
   * Parse `--post-shader <id>` and `--post-shader=<id>`; a missing or empty value is a usage error.
   * Read `REEXCEED_POST_SHADER`.
   * Choose with `ChoosePostShader`, then call `SetPostShaderDirectory(ResolvePostShaderDirectory(executable directory))` and `SetInitialPostShader(id)` before the device is created.
   * Log `[app] post shader: <id> (directory <path>)` and add the option to the usage text.
   * A command-line or environment value never overwrites the ini; only a menu choice is saved.
3. `src/app/settings_ui.{h,cpp}`: a `Screen shader` section in the Graphics tab, built like reDMP1's `DrawPostShaderMenu` and in the existing tab style (`SeparatorText`, `(?)` tooltips). A choice updates `settings_.post_shader` and calls `Save()`.
4. Settings check: precedence and an ini round trip under CTest (see reDMP1's `settings_probe.cpp`), next to the existing smoke tests in `src/app/CMakeLists.txt`.
5. Documents: `README.md` (the Screen shader menu, `--post-shader`, `REEXCEED_POST_SHADER`, `shaders/`, `PostShader`) and `ARCHITECTURE.md` (app settings), Korean first, then English. Source comments in English only.
6. Build and test: Win32 x86 Release and Debug with 0 warnings; `ctest -C Release` and `ctest -C Debug` pass.

## Report

Write the changed files, a build and test summary, the commit id, anything done differently from the design, and questions to `%TEMP%\reexceed-coder\i020-screen-shaders.txt`, then end the turn.
