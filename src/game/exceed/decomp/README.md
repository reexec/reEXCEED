# decomp/

손으로 디컴파일한 함수가 들어가는 자리다. 각 파일은 대체하는 lift 함수와 같은 심볼 `void sub_XXXXXXXX(void)`을 정의하고, `scripts/recomp/regenerate.py`(파이프라인은 `scripts/recomp/lifter.py`)가 그 정의를 lifter의 `--exclude-manual`로 넘긴다. 파일이 `extern void sub_XXXXXXXX_gen(void);`을 선언하면 lift 본문이 그 이름으로 남고, `reexceed_decomp_check`(`tools/decomp_check/`)가 둘을 같은 게스트 상태에서 돌려 비교한다. 규칙은 [docs/CODING_STYLE.md](../../../../docs/CODING_STYLE.md)의 "재컴파일 코드와 디컴파일 코드" 항목이다.

| 파일 | 함수 | 근거 |
|---|---|---|
| `xapi_code_selector.c` | `00087E12` XAPI 시작 코드의 커널 코드 셀렉터 한계 설정 → 아무것도 하지 않음. 디컴파일이 아니라 호스트 어댑터 | issue #4, [분석 §7](../../../../docs/analysis/exceed-xbe-overview.md) |
| `xact_sample_clock.c` | `0016F12A` XACT 시간 함수, APU 샘플 카운터 대신 호스트 훅. 디컴파일이 아니라 호스트 어댑터 | issue #4, [분석 §11](../../../../docs/analysis/exceed-xbe-overview.md), rexbox #55 |
| `full_size_picture.c` | `00075F10` 2D 설정 앞에서 가상 화면 크기(TV 오버스캔 배율 1.2)를 설정에 따라 백 버퍼 크기로 바꿈. 디컴파일이 아니라 호스트 어댑터 | issue #14, [분석 §12](../../../../docs/analysis/exceed-xbe-overview.md), [설계](../../../../docs/design/20261107-i014-full-size-picture.md) |

reCT3의 `d3dx_matrix.c`, `adxm_lock.c`, `sj_lock.c`는 그 빌드의 주소와 CRI 미들웨어에 묶여 있어 가져오지 않았다(Exceed는 CRI를 링크하지 않는다). 같은 종류의 문제가 이 빌드에서 확인되면 그 방식(디컴파일, 또는 호스트 동시성 어댑터)을 따른다.

이 디렉터리의 파일을 더하거나 바꾸면 `regenerate.py`를 다시 돌리고, `reexceed_decomp_check`에 경우를 더한다.

Hand-decompiled functions go here. Each file defines the same symbol as the lifted function it replaces, `void sub_XXXXXXXX(void)`, and `scripts/recomp/regenerate.py` (pipeline in `scripts/recomp/lifter.py`) hands those definitions to the lifter as `--exclude-manual`. A file that declares `extern void sub_XXXXXXXX_gen(void);` keeps the lifted body under that name, and `reexceed_decomp_check` (`tools/decomp_check/`) runs the two from the same guest state and compares them. The rules are the "Recompiled and Decompiled Code" section of [docs/CODING_STYLE.md](../../../../docs/CODING_STYLE.md).

Files: `xapi_code_selector.c`, XAPI start-up's kernel code-selector limit (`00087E12`) made to do nothing, and `xact_sample_clock.c`, XACT's time function (`0016F12A`) reading a host hook in place of the APU sample counter, both host adapters rather than decompilations (issue #4; analysis sections 7 and 11; rexbox #55). `full_size_picture.c` puts the virtual screen size (the TV-overscan scale 1.2) back to the back buffer size before the 2D setup (`00075F10`) when the setting asks, also a host adapter (issue #14; analysis section 12). reCT3's `d3dx_matrix.c`, `adxm_lock.c` and `sj_lock.c` are bound to that build's addresses and to CRI's middleware, so they were not carried over (Exceed links no CRI). When the same kind of problem shows in this build, the same approach (a decompilation, or a host-concurrency adapter) is followed. After adding or changing a file here, run `regenerate.py` again and add cases to `reexceed_decomp_check`.
