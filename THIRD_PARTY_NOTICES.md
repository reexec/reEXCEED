# 서드파티 고지 / Third-Party Notices

reEXCEED는 MIT 라이선스다. 아래 서드파티 코드가 저장소에 포함되거나 옮겨져 있다.

reEXCEED is MIT licensed. The third-party code below is included in, or ported into, the repository.

## rexbox

* 출처 / Source: https://github.com/reexec/rexbox
* 라이선스 / License: MIT (Copyright (c) 2026 rexbox contributors (reJSRF contributors)). 전문은 `third_party/rexbox/LICENSE`. / Full text in `third_party/rexbox/LICENSE`.
* 포함 방식 / How included: git submodule `third_party/rexbox`(`v0.0.21`, `bc21519`). 공유 Xbox 런타임 전체(플랫폼, XBE, 런타임, 재컴파일 ABI 어댑터, HLE, 도구)와 그 안의 submodule(SDL3 zlib, Dear ImGui MIT, SMAA MIT, XbSymbolDatabase MIT)을 `add_subdirectory`로 빌드한다. 각 라이브러리의 고지는 `third_party/rexbox/THIRD_PARTY_NOTICES.md`에 있다. / Git submodule (`v0.0.21`, `bc21519`): the whole shared Xbox runtime and its own submodules (SDL3 zlib, Dear ImGui MIT, SMAA MIT, XbSymbolDatabase MIT), built with `add_subdirectory`; each library's notice is in `third_party/rexbox/THIRD_PARTY_NOTICES.md`.
* 옮긴 코드 / Ported code: `src/app/settings.h`, `settings.cpp`, `settings_ui.h`, `settings_ui.cpp`, `main.cpp`, `build_stamp.cmake`와 `src/game/exceed/exceed_recomp.cpp`, `exceed_host.c`, `include/reexceed/*.h`는 reCT3(MIT)와 reJSRF(MIT)의 같은 파일을 이름만 바꿔 가져온 것이다(reCT3가 바탕이고, Info 탭의 rexbox 버전·커밋, Stats 그래프 높이, Debug 스텁 로그는 reJSRF에서 왔다). `scripts/`, `tools/decomp_check/`, CMake와 워크플로 파일도 reCT3(MIT)에서 왔다. / Those app and title files are reCT3's and reJSRF's (MIT) under the reEXCEED names (reCT3's as the base; the Info tab's rexbox version and commit, the Stats graph height and the Debug stub log from reJSRF). `scripts/`, `tools/decomp_check/`, the CMake and workflow files also come from reCT3 (MIT).

## xboxrecomp

* 출처 / Source: https://github.com/sp00nznet/xboxrecomp
* 라이선스 / License: MIT (Copyright (c) 2026 sp00nz). 전문은 `third_party/xboxrecomp/LICENSE`. / Full text in `third_party/xboxrecomp/LICENSE`.
* 포함 방식 / How included: git submodule `third_party/xboxrecomp`, upstream `v0.16.0` (`f0f8a461496a4f23d9b0aeaa284b8d70ba7ab82b`). `tools/`(Python 파이프라인)를 `scripts/recomp/lifter.py`가 실행한다. 빌드에 들어가는 파일은 lifter가 생성 코드 옆에 쓴 ABI 헤더(`gen/<variant>/gen/recomp_types.h`, 원본 `templates/runtime/recomp_types.h`)뿐이며, 생성 코드와 rexbox의 `recomp_abi.c`가 이것으로 컴파일된다. / The Python pipeline under `tools/` is run by `scripts/recomp/lifter.py`. The only file that enters the build is the ABI header the lifter writes next to the generated code (`gen/<variant>/gen/recomp_types.h`, from `templates/runtime/recomp_types.h`); the generated code and rexbox's `recomp_abi.c` compile against it.
* 제외 / Excluded: xemu에서 가져온 LGPL-2.1-or-later 파일(`src/apu/*`, `src/nv2a/nv2a_core.c`, `src/nv2a/nv2a_regs.h`, `src/nv2a/nv2a_state.h`)과 `include/xbox/xboxrecomp.h`는 컴파일·링크하지 않는다. 검사 코드 없이 AGENTS.md 라이선스 규칙(어느 타깃의 소스 목록에도 넣지 않는다)으로 지킨다. / The LGPL-2.1-or-later files taken from xemu and `include/xbox/xboxrecomp.h` are never compiled or linked; this is kept by the license rule in AGENTS.md (never added to any target's source list), not by guard code.

```text
MIT License

Copyright (c) 2026 sp00nz

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```
