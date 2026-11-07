# 작업 로그: 원본 크기 출력 설정 (issue #14)

* 관련 이슈: #14
* 설계: [`docs/design/20261107-i014-full-size-picture.md`](../design/20261107-i014-full-size-picture.md)
* 작업 지시: [`docs/work-orders/20261107-i014-full-size-picture.md`](../work-orders/20261107-i014-full-size-picture.md)

## 경과

1. 사용자 보고: 게임플레이 화면이 창보다 작게 그려지고 둘레가 검다. 1280×960 창에서 그림은 1066×800(83.3%)이다.
2. 분석 문서 12절의 "2D는 768×576 정사영" 기록에서 출발해, 그 크기를 만드는 코드를 찾았다. `sub_00075F10`이 `D3DXMatrixOrthoLH([0x842358], [0x842354], −500, 500)`을 부르고, 두 전역을 쓰는 곳은 `sub_000765E0`의 `0x0007692B`/`0x0007693D`뿐이다. 배율은 `HalBootSMCVideoMode`가 1(HDTV)이면 1.1, 아니면 1.2(XBE `.rdata`의 `0x1BD3BC` = 1.1f, `0x1BD46C` = 1.2f를 읽어 확인). 3D 설정 `sub_00075FE0`도 같은 전역으로 시야각과 종횡비를 정한다. TV 오버스캔 보정으로 판단했다(**확인됨**: 코드와 상수; 의도는 **추정**).
3. 구현: `decomp/full_size_picture.c`(`sub_00075F10` 대체 어댑터), `exceed_host`의 `reexceed_set_full_size_picture`/`reexceed_host_full_size_picture`(기본 1)/동기화, `[Graphics] FullSizePicture`(기본 `true`), Graphics 탭 체크박스 `Full-size picture`, 시작 시 적용.
4. `scripts/recomp/regenerate.py`로 `sub_00075F10_gen`을 만들었다.

## 검증

* Win32 x86 Release 빌드: 경고·오류 없음.
* `reexceed_decomp_check`: 3 case 0 difference(`sub_00075F10 full_size_off_pack0` 포함), `TestUnlockAll`, `TestAutoplay`, `TestFullSizePicture` 통과. `ctest -C Release` 7/7 통과.
  * `sub_00075F10`은 끝에서 D3D `SetTransform` HLE를 부르는데 이 도구에는 HLE 디바이스가 없어 그 지점에서 멈춘다(`RunGuarded` stop 4). 그 앞의 어댑터 동작과 `D3DXMatrixOrthoLH` 결과까지가 비교 대상이다.
* 실행(프레임 1500, 인트로 동영상, `REXBOX_CAPTURE_FRAME`): 켜짐은 1280×960을 가득 채우고, 꺼짐(`FullSizePicture = false`)은 원래처럼 1066×800과 검은 테두리다.
* 남은 확인(사용자): 게임플레이·선곡 화면과 3D 장면에서의 모습, 실행 중 체크박스 전환(다음 장면부터 반영).

---

# Work Log: Full-Size Picture Setting (issue #14)

* Related issue: #14
* Design: [`docs/design/20261107-i014-full-size-picture.md`](../design/20261107-i014-full-size-picture.md)
* Work order: [`docs/work-orders/20261107-i014-full-size-picture.md`](../work-orders/20261107-i014-full-size-picture.md)

## Progress

1. User report: gameplay is drawn smaller than the window with a black border; in a 1280x960 window the picture is 1066x800 (83.3%).
2. Starting from analysis section 12's "2D through a 768x576 orthographic projection", the code behind that size: `sub_00075F10` calls `D3DXMatrixOrthoLH([0x842358], [0x842354], −500, 500)`, and the only writes of the two globals are `sub_000765E0`'s `0x0007692B`/`0x0007693D`. The scale is 1.1 when `HalBootSMCVideoMode` is 1 (HDTV) and 1.2 otherwise (checked in the XBE's `.rdata`: `0x1BD3BC` = 1.1f, `0x1BD46C` = 1.2f). The 3D setup `sub_00075FE0` takes its field of view and aspect from the same globals. Taken as TV overscan compensation (**confirmed**: the code and constants; the intent is **inferred**).
3. Implementation: `decomp/full_size_picture.c` (an adapter replacing `sub_00075F10`), `exceed_host`'s `reexceed_set_full_size_picture`/`reexceed_host_full_size_picture` (default 1) and sync, `[Graphics] FullSizePicture` (default `true`), the Graphics tab checkbox `Full-size picture`, applied at start.
4. `scripts/recomp/regenerate.py` emitted `sub_00075F10_gen`.

## Verification

* Win32 x86 Release build: no warnings or errors.
* `reexceed_decomp_check`: 3 cases, 0 differences (including `sub_00075F10 full_size_off_pack0`); `TestUnlockAll`, `TestAutoplay` and `TestFullSizePicture` pass. `ctest -C Release`: 7/7.
  * `sub_00075F10` ends in the D3D `SetTransform` HLE, which stops in this tool (no HLE device; `RunGuarded` stop 4); the adapter's work and the `D3DXMatrixOrthoLH` result before it are what is compared.
* A run (frame 1500, the intro movie, `REXBOX_CAPTURE_FRAME`): on fills 1280x960; off (`FullSizePicture = false`) is the original 1066x800 with the black border.
* Left for the user: gameplay, song select and 3D scenes, and switching the checkbox while running (shows from the next scene).
