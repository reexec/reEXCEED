# Win32 개발 환경 준비와 빌드

근거: [설계 i001](../design/20261103-i001-project-bootstrap.md), [작업 로그 i001](../work-logs/20261103-i001-project-bootstrap.md)

## 1. 필요한 것

* Windows 10/11
* Visual Studio 2022 또는 2026, "C++를 사용한 데스크톱 개발" 워크로드(x86/x64 MSVC 도구 포함)
* OpenGL 3.3 이상을 지원하는 그래픽 드라이버(창 표시용; `REXBOX_DISPLAY=none`으로 창 없이 실행 가능)
* CMake 3.20 이상(Visual Studio 내장 CMake도 가능)
* Git
* 재컴파일 파이프라인용: Python 3.10 이상과 `pip install capstone`
* 직접 소유한 Pump It Up: Exceed(북미판) 디스크의 덤프

## 2. 저장소 받기

```powershell
git clone --recurse-submodules https://github.com/reexec/reEXCEED.git
cd reEXCEED
# submodule 없이 받았다면(rexbox와 그 안의 SDL·imgui·smaa·XbSymbolDatabase, 그리고 xboxrecomp):
git submodule update --init --recursive
```

## 3. 게임 파일 두기

디스크에서 추출한 파일을 `roms/`에 둔다(`roms/`는 gitignore 대상이며 절대 커밋하지 않는다). ISO만 있다면 [extract-xiso](https://github.com/XboxDev/extract-xiso)나 [xdvdfs](https://github.com/antangelo/xdvdfs)로 추출한다.

```text
roms/
  DEFAULT.XBE        the game's XBE: the North America build (MD5 575f861eb700e86a82c0ad530dc4a310) is the primary target
  AUDIO/ BGA/ MEDIA/ TITLE/ WAVE/ STEP.DAT ...   the disc's assets (songs, movies, step charts)
```

## 4. XBE 등록 (처음 한 번)

북미판 XBE의 신원은 이미 등록되어 있다(issue #1). 다른 덤프(다른 지역판 등)를 쓸 때 등록하면 `variants.json`에 MD5가 들어가고, 프로파일에 붙여 넣을 인증서 값이 출력된다. 디스크의 대문자 파일명(`DEFAULT.XBE`)은 그대로 둬도 된다(스크립트와 앱이 대소문자 없이 찾는다).

```powershell
python scripts\recomp\register_xbe.py                 # roms\default.xbe 를 exceed_us 로 (이미 등록됨; 다른 덤프를 쓸 때)
# 출력된 kVariants 항목을 src\game\exceed\exceed_profile.cpp 에 붙여 넣고,
# docs\analysis\exceed-xbe-overview.md 의 표도 출력된 행으로 채운다.
.\build\win32_x86\third_party\rexbox\tools\Debug\rexbox_xbe_info.exe roms\default.xbe   # 헤더·섹션·라이브러리·커널 import
```

## 5. 재컴파일 코드 생성

```powershell
pip install capstone
# 처음 한 번: XDK 함수 식별 도구를 먼저 빌드한다
.\scripts\build_win32_x86.ps1 -Configuration Release -Target rexbox_xdk_symbols
python scripts\recomp\regenerate.py      # gen/exceed_us/ 에 생성
```

`regenerate.py`는 기본으로 `roms\default.xbe`를 읽는다. 다른 경로는 `--xbe <경로>`로 준다. 읽기 전에 MD5가 등록된 북미판(`src/game/exceed/recomp/variants.json`)과 같은지 확인하고, 다르거나 등록되지 않았으면 생성하지 않고 멈춘다. 파이프라인(`scripts/recomp/lifter.py`)은 이 저장소가 고정한 upstream xboxrecomp(`third_party/xboxrecomp`, v0.16.0)를 쓴다.

생성 코드는 커밋하지 않는다. `src/game/exceed/decomp/`를 바꾸거나 submodule을 갱신한 뒤에는 다시 생성한다. 생성하지 않으면 앱은 로드 단계에서 멈춘다.

## 6. 빌드

```powershell
.\scripts\build_win32_x86.ps1                       # Debug
.\scripts\build_win32_x86.ps1 -Configuration Release
```

PowerShell 실행 정책이 `.ps1`을 막으면 같은 인자로 `scripts\build_win32_x86.bat`을 쓴다. 결과는 `build\win32_x86\` 아래 각 타깃 디렉터리에 생긴다(rexbox의 도구는 `build\win32_x86\third_party\rexbox\tools\<구성>\`).

## 7. 확인

```powershell
# 게임 데이터 없이 도는 rexbox probe
.\build\win32_x86\third_party\rexbox\tools\Debug\rexbox_recomp_probe.exe   # "8 check(s), 0 failure(s)"
.\build\win32_x86\third_party\rexbox\tools\Debug\rexbox_kernel_probe.exe   # "94 check(s), 0 failure(s)"
.\build\win32_x86\third_party\rexbox\tools\Debug\rexbox_dsound_probe.exe   # "73 check(s), 0 failure(s)"
.\build\win32_x86\third_party\rexbox\tools\Debug\rexbox_d3d8_probe.exe     # "223 check(s), 0 failure(s)"

# the four probes and the two app smoke tests in one command (the one CI uses);
# with generated code, also reexceed_decomp_check
ctest --test-dir build\win32_x86 -C Debug --output-on-failure

# 생성 코드가 있으면: 디컴파일 함수마다 lift 원본과 비교한다(첫 디컴파일 함수 전까지 "0 case(s), 0 difference(s)")
.\build\win32_x86\tools\Debug\reexceed_decomp_check.exe

# 앱: 게임 디렉터리를 D:로 쓰고 default.xbe를 읽어 인증서로 식별한다.
.\build\win32_x86\src\app\Debug\reexceed.exe roms
```

등록되지 않은 XBE는 `error: default.xbe is not a supported Pump It Up: Exceed build (title id ..., certificate version ...)`로 멈춘다(종료 코드 1). 등록된 북미판을 재컴파일 코드 없이 빌드했으면 `xbe: North America (default.xbe)`, `hle: ... kernel imports 104/146 bound`를 보인 뒤 `this build has no recompiled code (run scripts/recomp/regenerate.py); stopping after load`로 끝난다(종료 코드 0). 재컴파일 코드가 있으면 진입점부터 실행하고 멈춘 지점을 보고한다. 창 조작, 설정 메뉴(Tab), 환경 변수(`REXBOX_*`, `REEXCEED_HDD_ROOT`), 진단은 reJSRF와 같다: [rexbox ARCHITECTURE §5](../../third_party/rexbox/ARCHITECTURE.md)와 [reJSRF의 가이드](https://github.com/nworkers/reJSRF/blob/main/docs/guides/win32-setup-and-build.md).

게임의 HDD는 현재 폴더의 `hdd\`(gitignore)에 만들어지고, `REEXCEED_HDD_ROOT`로 바꾼다. 설정은 실행 파일 옆 `reexceed.ini`에 저장된다.

---

# Win32 development setup and build

Based on: [design i001](../design/20261103-i001-project-bootstrap.md), [work log i001](../work-logs/20261103-i001-project-bootstrap.md)

1. **Requirements**: Windows 10/11; Visual Studio 2022 or 2026 with the "Desktop development with C++" workload; a graphics driver with OpenGL 3.3 or newer for the window (`REXBOX_DISPLAY=none` runs without one); CMake 3.20+; Git; for the recompilation pipeline, Python 3.10+ with `pip install capstone`; a dump of a Pump It Up: Exceed (North America) disc you own.
2. **Get the repository**: `git clone --recurse-submodules https://github.com/reexec/reEXCEED.git` (or `git submodule update --init --recursive` after a plain clone; the submodules are rexbox with its own SDL, imgui, smaa and XbSymbolDatabase, plus this project's xboxrecomp pin).
3. **Place the game files** under `roms/` (gitignored; never commit them): `roms/DEFAULT.XBE` and the disc's folders (`AUDIO/`, `BGA/`, `MEDIA/`, `TITLE/`, `WAVE/`, ...); the upper-case names may stay as they are. Extract an ISO with [extract-xiso](https://github.com/XboxDev/extract-xiso) or [xdvdfs](https://github.com/antangelo/xdvdfs).
4. **Register the XBE** (done for the North America build in issue #1; again only for another dump): `python scripts\recomp\register_xbe.py` writes the build's MD5 into `variants.json` and prints the certificate values to paste into `kVariants` in `src\game\exceed\exceed_profile.cpp` and the row for `docs\analysis\exceed-xbe-overview.md`; `rexbox_xbe_info.exe` prints the header, sections, libraries and kernel imports.
5. **Generate the recompiled code**: `pip install capstone`; build `rexbox_xdk_symbols` first (`.\scripts\build_win32_x86.ps1 -Configuration Release -Target rexbox_xdk_symbols`); then `python scripts\recomp\regenerate.py` (into `gen/exceed_us/`). It reads `roms\default.xbe` unless `--xbe <path>` names another, stops when the MD5 is not the registered North America build's, and the pipeline (`scripts/recomp/lifter.py`) uses this project's upstream xboxrecomp pin (`third_party/xboxrecomp`, v0.16.0). Generated code is never committed; regenerate after changing `src/game/exceed/decomp/` or moving a submodule.
6. **Build**: `.\scripts\build_win32_x86.ps1` for Debug, `-Configuration Release` for Release (`scripts\build_win32_x86.bat` when the execution policy blocks `.ps1` files). Output goes under `build\win32_x86\`; rexbox's tools under `build\win32_x86\third_party\rexbox\tools\<config>\`.
7. **Check**: `ctest --test-dir build\win32_x86 -C Debug --output-on-failure` runs the four rexbox probes and the two app smoke tests (the command CI uses), plus `reexceed_decomp_check` when generated code exists (each decompiled function against its lifted original; "0 case(s), 0 difference(s)" until the first decompiled function); by hand, the four rexbox probes (recomp, kernel, dsound, d3d8) report 8, 94, 73 and 223 checks with 0 failures (rexbox v0.0.21); `reexceed.exe roms` serves the folder as the game's `D:` and identifies `default.xbe` by its certificate. An unregistered build stops with `not a supported Pump It Up: Exceed build` (exit code 1); the registered North America build without recompiled code ends with `this build has no recompiled code ...; stopping after load` (exit code 0); with recompiled code it runs from the entry point and reports where it stopped. Window controls, the settings menu (Tab), the environment variables (`REXBOX_*`, `REEXCEED_HDD_ROOT`) and the diagnostics are as in reJSRF: see [rexbox ARCHITECTURE §5](../../third_party/rexbox/ARCHITECTURE.md) and [reJSRF's guide](https://github.com/nworkers/reJSRF/blob/main/docs/guides/win32-setup-and-build.md). The game's hard disk is created under `hdd\` in the current folder (`REEXCEED_HDD_ROOT` moves it); settings are saved in `reexceed.ini` next to the executable.
