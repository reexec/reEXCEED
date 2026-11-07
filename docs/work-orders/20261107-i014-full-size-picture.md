# 작업 지시서: 원본 크기 출력 설정 (issue #14)

* 관련 이슈: #14
* 설계 문서: [`docs/design/20261107-i014-full-size-picture.md`](../design/20261107-i014-full-size-picture.md)

## 작업 단계

1. **문서**: `docs/analysis/exceed-xbe-overview.md` 12절에 가상 화면 크기(`[0x842358]`, `[0x842354]`), 배율(`HalBootSMCVideoMode`, 1.2/1.1), 소비 함수(`sub_00075F10`, `sub_00075FE0`)를 적고, `ARCHITECTURE.md`와 `src/game/exceed/decomp/README.md`에 새 어댑터를 적는다.
2. **호스트 훅과 어댑터**: `exceed_host.h/.c`에 `reexceed_set_full_size_picture`, `reexceed_host_full_size_picture`(기본 1), `reexceed_decomp_sync_full_size_picture`를 더하고, `decomp/full_size_picture.c`에서 `sub_00075F10`을 대체한다. `scripts/recomp/regenerate.py`로 `sub_00075F10_gen`을 만든다.
3. **앱 설정**: `Settings::full_size_picture`(기본 `true`), `[Graphics] FullSizePicture`, Graphics 탭 체크박스, 시작 시 적용(`main.cpp`).
4. **검증**: `reexceed_decomp_check`에 `TestFullSizePicture`, Win32 Release 빌드, `ctest`, 실행 확인.
5. **작업 로그**: `docs/work-logs/20261107-i014-full-size-picture.md`.

---

# Work Order: Full-Size Picture Setting (issue #14)

* Related issue: #14
* Design document: [`docs/design/20261107-i014-full-size-picture.md`](../design/20261107-i014-full-size-picture.md)

## Steps

1. **Documents**: record the virtual screen size (`[0x842358]`, `[0x842354]`), its scale (`HalBootSMCVideoMode`, 1.2/1.1) and its consumers (`sub_00075F10`, `sub_00075FE0`) in section 12 of `docs/analysis/exceed-xbe-overview.md`, and the new adapter in `ARCHITECTURE.md` and `src/game/exceed/decomp/README.md`.
2. **Host hooks and adapter**: add `reexceed_set_full_size_picture`, `reexceed_host_full_size_picture` (default 1) and `reexceed_decomp_sync_full_size_picture` to `exceed_host.h/.c`, and replace `sub_00075F10` in `decomp/full_size_picture.c`. Run `scripts/recomp/regenerate.py` to emit `sub_00075F10_gen`.
3. **App settings**: `Settings::full_size_picture` (default `true`), `[Graphics] FullSizePicture`, a Graphics tab checkbox, applied at start (`main.cpp`).
4. **Verification**: `TestFullSizePicture` in `reexceed_decomp_check`, Win32 Release build, `ctest`, a run.
5. **Work log**: `docs/work-logs/20261107-i014-full-size-picture.md`.
