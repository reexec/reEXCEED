# 작업 지시: 게스트 메모리 고갈 (issue #9)

설계: [20261106-i009](../design/20261106-i009-guest-memory.md)

1. 본게임까지 재현하고(키보드 Enter), 프레임 덤프와 로그로 증상을 확인한다.
2. 할당과 해제를 게스트 호출 경로와 함께 기록해(진단용 임시 코드), 남는 메모리의 출처를 찾는다.
3. rexbox issue #67~#69를 rexbox 규칙으로 진행하고, 프로파일에 `D3D_DestroyResource`를 바인딩한다.
4. Debug·Release 빌드와 CTest, 같은 시나리오로 다시 확인하고, 작업 로그를 쓰고 커밋한다(`(#9)`).

---

# Work order: guest memory runs out (issue #9)

Design: [20261106-i009](../design/20261106-i009-guest-memory.md)

1. Reproduce into gameplay (keyboard Enter) and check the symptoms in frame dumps and the log.
2. Log allocations and frees with their guest call paths (temporary diagnostic code) to find where the memory left comes from.
3. Carry rexbox issues #67–#69 under rexbox's rules and bind `D3D_DestroyResource` in the profile.
4. Build Debug and Release, run CTest, check the same scenario again, write the work log and commit (`(#9)`).
