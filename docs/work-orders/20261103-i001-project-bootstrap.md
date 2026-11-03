# 작업 지시: 프로젝트 부트스트랩 (issue #1)

설계: [20261103-i001](../design/20261103-i001-project-bootstrap.md)

1. 저장소: 현재 디렉터리에 git 저장소를 만들고 `origin`을 `git@github.com:reexec/reEXCEED.git`으로 둔다. 원격 `main`(MIT `LICENSE`)을 받아 작업 브랜치 `i001-project-bootstrap`을 만든다. GitHub issue #1(부트스트랩), #2(Actions).
2. 트리: reCT3의 파일을 가져와 이름을 바꾼다(`rect3`→`reexceed`, `ct3`→`exceed`, `ct3_jp`→`exceed_us`). CT3 전용 파일(설계 §2의 "가져오지 않음")은 뺀다.
3. submodule: `third_party/rexbox`를 `v0.0.21`, `third_party/xboxrecomp`를 upstream `v0.16.0`에 고정한다.
4. 타이틀 계층: `register_xbe.py`로 북미판을 등록하고 `kVariants`를 채운다. XDK 내부 주소, 시드, 추가 심볼은 비운다. `hle_libraries.json` 설명을 Exceed에 맞춘다. `decomp_check`에서 CT3 경우를 빼고 틀만 남긴다.
5. reJSRF 개선: `build_stamp.cmake`의 rexbox 버전·커밋, Info 탭 두 행, Stats 그래프 절반 높이, `EnableStubLogInDebug`.
6. Exceed 보강: 앱과 스크립트의 대소문자 무시 XBE 찾기.
7. 문서: `README.md`, `ARCHITECTURE.md`, `AGENTS.md`, `PROJECT_CHARTER.md`, `CODING_STYLE.md`, `XBE_DESIGN.*`, `docs/analysis/exceed-xbe-overview.md`, 색인들, 가이드, `CREDITS.md`, `THIRD_PARTY_NOTICES.md`. `VERSION`은 `0.0.1`.
8. 검증: 설계 §7. 끝나면 작업 로그를 쓰고 커밋한다(`(#1)`).

---

# Work order: project bootstrap (issue #1)

Design: [20261103-i001](../design/20261103-i001-project-bootstrap.md)

1. Repository: create a git repository in the current directory with `origin` at `git@github.com:reexec/reEXCEED.git`; take the remote `main` (MIT `LICENSE`) and create the task branch `i001-project-bootstrap`. GitHub issues #1 (bootstrap) and #2 (Actions).
2. Tree: bring reCT3's files over and rename them (`rect3`→`reexceed`, `ct3`→`exceed`, `ct3_jp`→`exceed_us`), leaving out the CT3-only files (design §2, "not taken").
3. Submodules: pin `third_party/rexbox` at `v0.0.21` and `third_party/xboxrecomp` at upstream `v0.16.0`.
4. Title layer: register the North America build with `register_xbe.py` and fill `kVariants`; empty the XDK internals, seeds and extra symbols; fit the `hle_libraries.json` comment to Exceed; drop CT3's cases from `decomp_check`, keeping the framework.
5. reJSRF improvements: the rexbox version and commit in `build_stamp.cmake`, the two Info tab rows, the half-height Stats graphs, `EnableStubLogInDebug`.
6. For Exceed: the case-insensitive XBE lookup in the app and the scripts.
7. Documents: `README.md`, `ARCHITECTURE.md`, `AGENTS.md`, `PROJECT_CHARTER.md`, `CODING_STYLE.md`, `XBE_DESIGN.*`, `docs/analysis/exceed-xbe-overview.md`, the indexes, the guides, `CREDITS.md`, `THIRD_PARTY_NOTICES.md`; `VERSION` is `0.0.1`.
8. Verification: design §7. Then write the work log and commit (`(#1)`).
