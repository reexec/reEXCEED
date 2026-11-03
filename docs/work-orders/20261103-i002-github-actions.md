# 작업 지시: GitHub Actions (issue #2)

설계: [20261103-i002](../design/20261103-i002-github-actions.md)

1. reCT3의 `.github/workflows/ci.yml`, `release.yml`을 이름만 바꿔 가져온다(설계 문서 경로, 릴리스 제목 `reEXCEED <tag>`).
2. 앱 smoke 테스트 두 개를 `src/app/CMakeLists.txt`에 둔다(reCT3와 같음).
3. 로컬에서 `ctest -C Debug`, `-C Release`로 6개 테스트를 확인한다.
4. 작업 로그를 쓰고 커밋한다(`(#2)`).

---

# Work order: GitHub Actions (issue #2)

Design: [20261103-i002](../design/20261103-i002-github-actions.md)

1. Bring reCT3's `.github/workflows/ci.yml` and `release.yml` over under the new name (design document paths, release title `reEXCEED <tag>`).
2. Keep the two app smoke tests in `src/app/CMakeLists.txt` (as in reCT3).
3. Check the six tests locally with `ctest -C Debug` and `-C Release`.
4. Write the work log and commit (`(#2)`).
