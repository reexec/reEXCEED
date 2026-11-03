# 작업 로그: GitHub Actions (issue #2)

설계: [20261103-i002](../design/20261103-i002-github-actions.md) · 작업 지시: [20261103-i002](../work-orders/20261103-i002-github-actions.md)

## 한 일

* reCT3의 `ci.yml`(Win32 x86 Debug·Release, Linux x64 debug·release)과 `release.yml`(태그 검증, 릴리스 노트로 Release)을 이름만 바꿔 넣었다. 설계 문서 경로는 `20261103-i002`, 릴리스 제목은 `reEXCEED <tag>`.
* 앱 smoke 테스트 두 개는 reCT3와 같이 `src/app/CMakeLists.txt`에 있다.

## 검증

* 로컬 Win32 x86(Visual Studio 2026): `ctest -C Debug`, `ctest -C Release` 모두 6/6 통과(`rexbox_recomp_probe`, `rexbox_kernel_probe`, `rexbox_dsound_probe`, `rexbox_d3d8_probe`, `reexceed_rejects_file`, `reexceed_rejects_directory_without_xbe`).
* Actions 실행: **아직 안 됨.** 작업 브랜치를 push하지 않았다(규칙상 push와 PR은 머지 요청 때 한다). PR을 만들면 `ci.yml`이 돈다. Linux는 WSL2 Ubuntu 24.04에서 `build_linux.sh x64 release`, `x86 release`가 CI와 같은 명령으로 빌드되고 CTest 6/6을 통과했다([작업 로그 i001](20261103-i001-project-bootstrap.md)).

---

# Work log: GitHub Actions (issue #2)

Design: [20261103-i002](../design/20261103-i002-github-actions.md) · Work order: [20261103-i002](../work-orders/20261103-i002-github-actions.md)

## Done

* reCT3's `ci.yml` (Win32 x86 Debug and Release, Linux x64 debug and release) and `release.yml` (tag verification, a Release from the release note) are in under the new name, with the design path `20261103-i002` and the release title `reEXCEED <tag>`.
* The two app smoke tests sit in `src/app/CMakeLists.txt`, as in reCT3.

## Verification

* Local Win32 x86 (Visual Studio 2026): `ctest -C Debug` and `ctest -C Release` both pass 6/6.
* Actions: **not run yet.** The task branch is not pushed (by the rules, the push and the pull request come with a merge request); `ci.yml` runs once the pull request exists. On WSL2 Ubuntu 24.04, `build_linux.sh x64 release` and `x86 release` (CI's command) build and pass CTest 6/6 ([work log i001](20261103-i001-project-bootstrap.md)).
