# 설계: 프로젝트 부트스트랩과 공유 rexbox 위의 레이어 구조 (issue #1)

상태: **구현됨**. 작업 지시: [20261103-i001](../work-orders/20261103-i001-project-bootstrap.md) · 작업 로그: [20261103-i001](../work-logs/20261103-i001-project-bootstrap.md)

## 1. 요구사항

* Xbox 게임 Pump It Up: Exceed를 구동하는 프로젝트를 만든다. 원격 저장소는 `reexec/reEXCEED`(`git@github.com:reexec/reEXCEED.git`, `main`에 MIT `LICENSE` 한 커밋이 있음).
* [reCT3](https://github.com/reexec/reCT3)와 [reJSRF](https://github.com/nworkers/reJSRF)가 가진 기능과 개선사항을 모두 가져오고, 작업 규칙도 같게 적용한다.
* xboxrecomp는 upstream 최신 버전으로 한다(사용자 지시, 작업 중 추가). 확인한 최신은 v0.16.0(`f0f8a461`, 2026-10-20)이다.
* 사용자의 덤프가 `roms/`에 있다(`DEFAULT.XBE` 등, 대문자 파일명).

## 2. 두 프로젝트에서 가져오는 것

reCT3는 reJSRF에서 rexbox를 분리한 뒤 같은 규칙으로 만든 두 번째 타이틀이며, 이 저장소 구조의 직접 원형이다. 그래서 **reCT3의 트리를 바탕으로** 하고, reCT3에 없는 reJSRF의 타이틀 독립 개선을 더한다.

| 출처 | 항목 | reEXCEED | 판단 |
|---|---|---|---|
| 공통 | `third_party/rexbox` | rexbox `v0.0.21`(`bc21519`, 최신 태그, reCT3와 같음) | 공유 런타임. 기능 대부분(HLE, 렌더러, 설정 표시, 와이드스크린, MSAA/SMAA/AF, vsync, 진동, 3D 오디오 등)이 여기 있다 |
| 공통 | `third_party/xboxrecomp` | upstream `v0.16.0`(`f0f8a46`) | 사용자 지시. reCT3는 v0.15.0, reJSRF는 v0.16.0 기반 fork(§4) |
| reCT3 | 규칙 문서(`AGENTS.md`, `CLAUDE.md`, `CODING_STYLE.md`), CMake(루트, 프리셋), `.gitignore`·`.gitattributes` | 같은 규칙, 이름만 `reexceed` | |
| reCT3 | `src/app/`(main, settings, settings_ui, build_stamp), 앱 smoke 테스트 | `reexceed` | |
| reCT3 | `src/game/ct3/` 구조(profile, recomp lookup, host 훅, `decomp/`, `recomp/`) | `src/game/exceed/`, `include/reexceed/` | 내용(변형, 시드, XDK 내부 주소, 추가 심볼)은 CT3 주소라 비움 |
| reCT3 | `scripts/recomp/`(lifter, regenerate, register, check_hle_pops, find_pointer_targets), `scripts/build_win32_x86.*`, `scripts/build_linux.sh` | 같음 | |
| reCT3 | `tools/decomp_check/`(GPR·x87·XMM·메모리 비교 틀) | `reexceed_decomp_check` | D3DX 경우는 CT3 주소라 빼고 틀만 |
| reCT3 | `.github/workflows/ci.yml`, `release.yml` | 같음(issue #2) | |
| reCT3 | 문서 골격: guides(Win32, Linux), analysis·kb 색인, XBE_DESIGN | Exceed로 다시 씀 | |
| reCT3 | `decomp/*.c`(D3DX 행렬, CRI ADXM·SJ 잠금), `drive_west_coast.ps1`, 운전 측정 가이드, CT3 분석 문서, 이력 문서(design·work-*·release-notes) | 가져오지 않음 | CT3 주소·CRI 미들웨어·CT3 화면 전용. Exceed XBE에 CRI 문자열이 없다 |
| reJSRF | Info 탭의 rexbox 버전·커밋(#111) | 가져옴 | `build_stamp.cmake`가 `third_party/rexbox/VERSION`과 커밋을 쓴다 |
| reJSRF | Stats 그래프를 절반 높이로(#111) | 가져옴 | |
| reJSRF | Debug 빌드의 미해결 스텁 보고(`RECOMP_STUB_LOG`, #115) | 가져옴 | upstream v0.15.0부터 있는 기능이라 v0.16.0에서 그대로 동작 |
| reJSRF | 언어 설정(Game 탭, `--korean`, 한국어 패치 폴더), 빌드 시 변형 선택(`REJSRF_VARIANT`, 지문), Media 캐시 비우기 | 가져오지 않음 | JSRF의 언어 변수·한국어 패치·두 지역판에 묶임. Exceed에 지역판이나 언어 선택이 확인되면 같은 방식으로 설계한다 |
| reJSRF | `tools/d3d8_check` | 예정 | JSRF 빌드의 D3D8 함수 주소에 묶임. lift 뒤 Exceed 주소로 만든다 |
| reJSRF | `scripts/analysis/{language,text}`, `fetch_references.ps1` | 가져오지 않음 | JSRF 전용 |

## 3. Exceed에 맞춘 것

* **대상 빌드**: 북미판 `default.xbe`, title id `41440002`, 인증서 시각 `42F8EC5C`, 버전 1, 이미지 시각 `42EDCD9F`, MD5 `575f861e…`, XDK 5849([분석](../analysis/exceed-xbe-overview.md)). `register_xbe.py`로 `variants.json`(`exceed_us`)과 `kVariants`에 등록한다. reCT3는 XBE가 생긴 뒤의 일(#3)로 미뤘지만, 덤프가 이미 있으므로 이번에 한다.
* **대소문자**: 디스크 파일명이 대문자(`DEFAULT.XBE`)다. Windows에서는 상관없지만 Linux 호스트는 `default.xbe`를 찾지 못한다. 앱(`FindXbe`)과 스크립트(`lifter.find_case_insensitive`)가 정확한 이름이 없으면 대소문자를 무시하고 찾는다. 게스트의 파일 열기는 rexbox 커널 HLE의 일이라 여기서 다루지 않는다(**미확정**: Linux에서 확인).
* **HLE 대상**: `hle_libraries.json`은 reCT3와 같은 첫 선택(D3D8, D3D8LTCG, DSOUND 전체와 XAPI 장치·입력 11개). `lifted` 세 함수는 reCT3(4531)의 근거를 가져온 것이라 5849에서 다시 확인한다(**추정**). WMADEC, XMV, XACTENG, XONLINE, XNET은 lift된 코드로 둔다.
* **물리 메모리 창**: reCT3처럼 켠다. Exceed가 쓰는지는 lift 뒤 확인한다.

## 4. xboxrecomp v0.16.0

* v0.15.0 → v0.16.0의 변경은 disasm의 함수 발견(#236, #239: 패딩 뒤 함수, 점프 표), 16비트 push/pop의 esp(#238), xboxrecomp 자체 커널의 POSIX 파일 경로(#237, 우리는 쓰지 않음)다([reJSRF 설계 i115](https://github.com/nworkers/reJSRF/blob/main/docs/design/20261023-i115-xboxrecomp-v0.16.md)).
* `templates/runtime/recomp_types.h`(ABI 헤더)와 lifter 출력 형식(`functions.json`, `/* ret N */`, `recomp_dispatch.c`, 이름 규칙)은 v0.15.0과 같다. 그래서 `lifter.py`와 rexbox의 `recomp_abi.c`는 바꿀 필요가 없다.
* reJSRF fork의 C4310 수정(`16a4a0da`)은 upstream에 없으므로 디컴파일 소스와 `guest_fpu.c`의 `/wd4310`은 reCT3처럼 유지한다.
* reJSRF fork에만 있는 성능·SSE2 커밋은 가져오지 않는다(upstream만 쓴다).

## 5. 레이어 구조

```mermaid
flowchart TD
    APP["app (reexceed.exe)<br/>orchestration, settings"]
    subgraph TITLE["game/exceed (title-specific)"]
        PROFILE["exceed_profile<br/>XBE identity, HLE bindings"]
        DECOMP["decomp/<br/>hand-written functions (none yet)"]
        RECOMP["gen/exceed_us/<br/>lifted C (gitignored)"]
        CFG["recomp/<br/>variants, seeds, hle_libraries"]
    end
    subgraph REXBOX["third_party/rexbox v0.0.21 (shared, title-agnostic)"]
        RUNTIME["runtime, recomp ABI adapter"]
        XBE["xbe"]
        HLE["hle: kernel, xapi, d3d8, dsound, xinput"]
        PLATFORM["platform: win32, linux (SDL3 + OpenGL)"]
    end
    XR["third_party/xboxrecomp (upstream v0.16.0)<br/>pipeline tools, lifter ABI header"]

    APP --> TITLE
    APP --> HLE
    TITLE --> RUNTIME
    HLE --> RUNTIME
    RUNTIME --> XBE
    RUNTIME --> PLATFORM
    CFG -. regenerate.py .-> RECOMP
    XR -. tools, recomp_types.h .-> RECOMP
```

이름 규칙: 저장소 `reEXCEED`, 실행 파일·CMake 접두사·네임스페이스 `reexceed`, 매크로 `REEXCEED_*`, 타이틀 디렉터리 `exceed`, 변형 `exceed_us`, include guard `REEXCEED_EXCEED_*_H`.

## 6. 범위 밖

* 재컴파일 코드 생성, 첫 부팅, rexbox HLE 확장(XDK 5849, 없는 커널 import 42개): 다음 task.
* reJSRF·reCT3 저장소와 rexbox는 바꾸지 않는다.
* `settings*.cpp`를 rexbox로 올리는 일: reCT3 설계 i001의 후속 후보와 같다(**미확정**).

## 7. 검증

| 검증 | 기대 |
|---|---|
| Win32 x86 Debug·Release 빌드 | 경고 0, 오류 0 |
| CTest | rexbox probe 4개(recomp, kernel, dsound, d3d8)와 앱 smoke 2개 통과 |
| `reexceed.exe roms` | `xbe: North America (default.xbe)`, 섹션 19개 배치, 커널 import 바인딩 수 출력, `this build has no recompiled code ...; stopping after load`, 종료 코드 0 |
| `register_xbe.py` | MD5 `575f861e…`와 인증서 값 출력, `variants.json` 갱신 |
| `git status` | `roms/`, `build/`, `gen/`이 staging되지 않음 |

---

# Design: project bootstrap and the layers on the shared rexbox (issue #1)

Status: **implemented**. Work order: [20261103-i001](../work-orders/20261103-i001-project-bootstrap.md) · Work log: [20261103-i001](../work-logs/20261103-i001-project-bootstrap.md)

## 1. Requirements

* Create a project that runs the Xbox game Pump It Up: Exceed. The remote is `reexec/reEXCEED` (`git@github.com:reexec/reEXCEED.git`; `main` holds one commit with the MIT `LICENSE`).
* Bring over every feature and improvement of [reCT3](https://github.com/reexec/reCT3) and [reJSRF](https://github.com/nworkers/reJSRF), and apply the same working rules.
* xboxrecomp is upstream's latest version (the user's instruction, added during the work); the latest found is v0.16.0 (`f0f8a461`, 2026-10-20).
* The user's dump is under `roms/` (`DEFAULT.XBE` and the rest, upper-case names).

## 2. What comes from the two projects

reCT3 is the second title built under the same rules after rexbox was split out of reJSRF, and the direct model for this repository's structure. So **reCT3's tree is the base**, and reJSRF's title-neutral improvements that reCT3 lacks are added. The table above lists each item: the rexbox submodule at `v0.0.21` (`bc21519`, the latest tag, as reCT3 pins it), which carries most features (HLE, renderer, display settings, widescreen, MSAA/SMAA/AF, vsync, vibration, 3D audio and so on); xboxrecomp at upstream `v0.16.0` (§4); reCT3's rules, CMake, app, title-layer structure, recomp scripts, build scripts, decompile check framework and workflows, renamed; reCT3's CT3-bound content (decompiled functions, the CRI lock adapters, seeds, the drive script and guide, CT3 analysis and history documents) left out, since Exceed links no CRI (no CRI strings in the XBE); from reJSRF, the Info tab's rexbox version and commit (#111), the half-height Stats graphs (#111) and the Debug builds' unresolved-stub report (`RECOMP_STUB_LOG`, #115, an upstream feature since v0.15.0); reJSRF's language settings, build-time variant selection and cache clearing are bound to JSRF's language variable, its Korean patch and its two regional builds and are not taken (designed the same way if Exceed shows regional builds or a language choice); `tools/d3d8_check` is bound to JSRF's D3D8 addresses and comes after the lift; JSRF's analysis scripts and reference fetcher are not taken.

## 3. Fitted to Exceed

* **Target build**: the North America `default.xbe`, title id `41440002`, certificate time `42F8EC5C`, version 1, image time `42EDCD9F`, MD5 `575f861e…`, XDK 5849 ([analysis](../analysis/exceed-xbe-overview.md)), registered with `register_xbe.py` into `variants.json` (`exceed_us`) and `kVariants`. reCT3 left this to its #3 because it had no XBE; the dump is here, so it is done now.
* **Case**: the disc's names are upper case (`DEFAULT.XBE`). Windows does not care, but a Linux host would miss `default.xbe`. The app (`FindXbe`) and the scripts (`lifter.find_case_insensitive`) fall back to a case-insensitive match when the exact name is absent. The guest's own file opens are rexbox's kernel HLE's business and are not handled here (**unresolved**: to be checked on Linux).
* **HLE targets**: `hle_libraries.json` makes reCT3's first choice (all of D3D8, D3D8LTCG and DSOUND, and XAPI's eleven device and input functions). The three `lifted` functions carry reCT3's (4531) reasons and are re-checked on 5849 (**inferred**). WMADEC, XMV, XACTENG, XONLINE and XNET stay lifted.
* **Physical window**: on, as in reCT3; whether Exceed needs it is checked after the lift.

## 4. xboxrecomp v0.16.0

* From v0.15.0 to v0.16.0: function discovery in disasm (#236, #239: functions after padding, jump tables), esp for 16-bit push/pop (#238), and POSIX file paths in xboxrecomp's own kernel (#237, unused here) ([reJSRF design i115](https://github.com/nworkers/reJSRF/blob/main/docs/design/20261023-i115-xboxrecomp-v0.16.md)).
* The ABI header `templates/runtime/recomp_types.h` and the lifter's output formats (`functions.json`, `/* ret N */`, `recomp_dispatch.c`, the naming) are the same as v0.15.0's, so neither `lifter.py` nor rexbox's `recomp_abi.c` needs changes.
* reJSRF's fork fix for C4310 (`16a4a0da`) is not upstream, so the decompiled sources and `guest_fpu.c` keep `/wd4310` as in reCT3.
* The performance and SSE2 commits only in reJSRF's fork are not taken (upstream only).

## 5. Layers

The diagram above applies. Naming: repository `reEXCEED`; executable, CMake prefix and namespace `reexceed`; macros `REEXCEED_*`; title directory `exceed`; variant `exceed_us`; include guards `REEXCEED_EXCEED_*_H`.

## 6. Out of scope

* Generating the recompiled code, the first boot, widening rexbox's HLE (XDK 5849, the 42 kernel imports without an export): the next task.
* reJSRF, reCT3 and rexbox are not changed.
* Moving `settings*.cpp` into rexbox: the same follow-up candidate as in reCT3's design i001 (**unresolved**).

## 7. Verification

As in the table above: Win32 x86 Debug and Release build with 0 warnings and 0 errors; CTest passes the four rexbox probes (recomp, kernel, dsound, d3d8) and the two app smoke tests; `reexceed.exe roms` prints `xbe: North America (default.xbe)`, places 19 sections, prints the kernel import binding count, ends with `this build has no recompiled code ...; stopping after load` and exit code 0; `register_xbe.py` prints the MD5 `575f861e…` and the certificate values and updates `variants.json`; `git status` shows `roms/`, `build/` and `gen/` unstaged.
