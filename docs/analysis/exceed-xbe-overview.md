# Exceed XBE 개요: 빌드 변형, 섹션, XDK 라이브러리, 커널 import, 디스크

근거: [작업 로그 i001](../work-logs/20261103-i001-project-bootstrap.md). 값은 사용자의 덤프(`roms/DEFAULT.XBE`)를 `scripts/recomp/register_xbe.py`, XBE 헤더 읽기, `reexceed.exe roms`의 로드 단계 출력으로 읽은 것이다. 게임의 바이트는 옮기지 않고 신원과 배치만 적는다.

## 1. 빌드 변형

| 변형 | 지역 | MD5 | 크기 | title id | 인증서 시각 | 이미지 시각 |
|---|---|---|---|---|---|---|
| North America (`exceed_us`) | `0x00000001` | `575f861eb700e86a82c0ad530dc4a310` | 1,896,448 | `41440002` | `42F8EC5C` (2005-08-09) | `42EDCD9F` (2005-08-01) |

* **확인됨**: 인증서 제목은 `PUMP IT UP: EXCEED`, title id의 앞 두 글자는 `AD`(Andamiro), 인증서 버전 1, 미디어 플래그 `0x202`, 등급 3.
* **확인됨**: 디스크에는 같은 title id·인증서 시각의 XBE가 셋 더 있다. `UPDATE.XBE`(Online Updater Application), `DOWNLOADER.XBE`(Downloader), `DASHUPDATE.XBE`(Xbox Dashboard Updater). 셋 다 Xbox Live·대시보드용 XDK 도구이며, 이 프로젝트는 `default.xbe`만 대상으로 한다.
* **미확정**: 한국판 등 다른 지역판의 존재와 신원.

## 2. 섹션과 메모리 배치

* **확인됨**: base `0x00010000`, 이미지 크기 `0x00A94D20`(약 10.6 MiB, 대부분 `.data`의 BSS). 진입점 `0x0008673A`(retail 키), TLS 디렉터리 `0x00190130`, kernel thunk 표 `0x0018F8C0`(`.rdata`의 시작).
* 섹션 19개:

| 섹션 | VA | 가상 크기 | 비고 |
|---|---|---|---|
| `.text` | `0x00011000` | `0xCB714` | 게임 코드와 정적 링크된 CRT 등 |
| `D3DX` | `0x000DC720` | `0xC198` | D3DX8 |
| `DSOUND` | `0x000E88C0` | `0xBB64` | |
| `WMADEC` | `0x000F4440` | `0x1907C` | WMA 디코더(곡 음악 `AUDIO/*.WMA`) |
| `XONLINE` | `0x0010D4C0` | `0x1F7DC` | Xbox Live |
| `XNET` | `0x0012CCA0` | `0x11FA0` | 네트워크 스택 |
| `XMV` | `0x0013EC40` | `0x27C3C` | XMV 동영상 재생(`BGA/*.XMV`, `CREDIT.XMV`) |
| `XACTENG` | `0x00166880` | `0x9E40` | XACT 사운드 뱅크(`MEDIA/*.xsb`, `*.xwb`) |
| `XGRPH` | `0x001706C0` | `0x1DA8` | XGraphics |
| `D3D` | `0x00172480` | `0x1480C` | D3D8(LTCG) |
| `XPP` | `0x00186CA0` | `0x8C04` | 주변기기(입력) |
| `.rdata` | `0x0018F8C0` | `0x2E67C` | kernel thunk 포함 |
| `.data` | `0x001BDF40` | `0x8DA4FC` | 원시 크기는 `0xF974`, 나머지는 0으로 채움 |
| `DOLBY` | `0x00A98440` | `0x7180` | DSP 코드 |
| `.data1`, `XON_RD`, `$$XTIMAGE`, `$$XSIMAGE`, `.XTLID` | `0x00A9F5C0`– | | 기타 |

* **확인됨**: 앱의 로드 단계에서 19개 섹션이 128 MiB 게스트 메모리에 놓이고, 남는 힙은 120,172 KiB다.

## 3. XDK 라이브러리

* **확인됨**: 모두 XDK 1.0.**5849**다. `XAPILIB`, `D3DX8`, `DSOUND`, `XVOICE`, `VOICMAIL`, `XONLINES`, `XBOXKRNL`, `XMV`, `UIX`, `LIBCPMT`, `LIBCMT`, `D3D8LTCG`, `XGRAPHCL`, `XACTENLT`.
* **추정**: reJSRF와 reCT3(4531)보다 새 XDK이므로, rexbox의 D3D8 HLE(렌더 상태 번호, Caps, 기본 상태 루틴)와 DSOUND HLE에 5849 배치가 필요할 수 있다. lift한 뒤 식별된 `D3DRS_*` 변수로 확인한다.
* **추정**: `D3D8`이 아니라 `D3D8LTCG`만 있다. `hle_libraries.json`은 두 이름을 다 고른다.
* **미확정**: `UIX`(Xbox Live 화면 UI 라이브러리)와 `XVOICE`/`VOICMAIL`이 Live 없이도 시작 경로에서 불리는지.

## 4. 커널 import

* **확인됨**: 146개. rexbox v0.0.21의 커널 HLE가 104개를 바인딩하고 42개는 export가 없다.
* HLE가 없는 import(로드 단계 출력, 부팅이 닿을 때 rexbox에 구현):
  * I/O 관리자와 장치: `IoCreateDevice`(65), `IoBuildSynchronousFsdRequest`(62), `IoInvalidDeviceRequest`(74), `IoStartNextPacket`(81), `IoStartPacket`(83), `IoSynchronousDeviceIoControlRequest`(84), `IoSynchronousFsdRequest`(85), `IofCallDriver`(86), `IofCompleteRequest`(87), `IoMarkIrpMustComplete`(359), `ObReferenceObjectByName`(247)
  * 파일: `NtQueryFullAttributesFile`(210), `NtDeleteFile`(195), `NtDuplicateObject`(197)
  * 메모리·스레드: `MmQueryStatistics`(181), `MmCreateKernelStack`(169), `MmDeleteKernelStack`(170), `KeSaveFloatingPointState`(142), `KeRestoreFloatingPointState`(139)
  * 시간: `KeQueryPerformanceCounter`(126), `KeQueryPerformanceFrequency`(127), `NtSetSystemTime`(228)
  * 런타임: `RtlRaiseException`(302), `RtlUnwind`(312), `RtlAnsiStringToUnicodeString`(260), `RtlUnicodeStringToAnsiString`(308), `RtlCompareString`(270)
  * 암호(Xbox Live): `XcHMAC`(340), `XcRC4Key`(338), `XcRC4Crypt`(339), `XcModExp`(345), `XcDESKeyParity`(346), `XcKeyTable`(347), `XcBlockCryptCBC`(349), `XcVerifyPKCS1Signature`(344), `XePublicKeyData`(355)
  * 네트워크 PHY: `PhyInitialize`(253), `PhyGetLinkState`(252)
  * 기타: `HalReturnToFirmware`(49), `HalInitiateShutdown`(360), `KeBugCheck`(95), `XeImageFileName`(326)
* **추정**: I/O 관리자 import(장치 만들기, IRP)는 XAPI의 메모리 유닛·하드디스크 장치 코드나 Xbox Live 쪽에서 오고, `Xc*`·`Phy*`는 Xbox Live와 네트워크에서 온다. Live를 쓰지 않는 시작 경로에서는 안 닿을 수 있다.

## 5. 디스크 구성

* **확인됨**: `AUDIO/`(208개, 곡 음악 `*.WMA`), `BGA/`(188개, 배경 동영상 `*.XMV`와 `*.DAT`), `TITLE/`(200개, 곡 배너 `*.PNZ`), `WAVE/`(44개, 효과음 `*.WAV`), `MEDIA/`(글꼴 `*.xpr`·`ArialUni.Ttf`, XACT `*.xsb`/`*.xwb`, `UIX.uix`, 세이브 이미지 `*.xbx`), 루트의 `STEP.DAT`(스텝 차트로 **추정**), `DSSTDFX.BIN`(DSOUND 효과 이미지), `CREDIT.XMV`.
* **추정**: `DSSTDFX.BIN`은 `XAudioDownloadEffectsImage`가 읽는 표준 DSP 효과 이미지다(reCT3의 같은 경로).

## 6. lift (issue #4)

* **확인됨**: upstream xboxrecomp v0.16.0이 함수 5,547개를 lift한다(실패 0, C 약 104만 줄, 23파일). 미해결 호출 대상 3개(`000E4AB2`, `001137C9`, `0018F8E2`), 미구현 명령 74개(14종). 미구현 명령 대부분은 `0x0014Dxxx`(`XMV` 섹션)의 `daa`/`aaa`/`das`/`aas`로, 데이터를 코드로 읽은 것으로 보인다(**추정**). 나머지는 특권 명령(`cli`/`sti`, `sgdt`, `ljmp`, `lldt`, `clts`, `wbinvd`)이다.
* **확인됨**: XDK 심볼 361개 식별(D3D8LTCG 함수 104, DSOUND 함수 159, XAPILIB 함수 43, XONLINES 함수 11, D3D8 변수 27 중 `D3DRS_*` 23 등). HLE override 268개, XDK 변수 바인딩 44개. XAPI 입력 함수 중 `XInputPoll`, `XInputSetState`, `XMountMUA`, `XUnmountMU`는 식별되지 않았다.
* **확인됨**: 시작 순서의 앞부분: `mainCRTStartup`(`0008673A`) → `PsCreateSystemThreadEx` → `XapiThreadStartup`(`000844CD`) → `mainXapiStartup`(`000866C6`) → `00087E76` → `00087E12`.

## 7. XAPI 시작 코드의 커널 의존 (XDK 5849)

* **확인됨**: `00087E76`은 `XboxHardwareInfo`(322)와 `XboxKrnlVersion`(324)의 Build로 커널 빌드별 커널 코드 주소(`0x8002AB88`, `0x80039749`± 등)를 고르고, `MmQueryAddressProtect`(179) → `MmSetAddressProtect`(182) → `cli` → 4바이트 쓰기(`0x28C5048D`) → `sti` → 보호 복원으로 **커널 코드를 패치**한다. rexbox가 보고하는 빌드 5838에서는 이 경로를 타지 않는다(HLE 추적에 179·182 호출 없음).
* **확인됨**: `00087E12`(이름 미식별)는 커널 PE 헤더(`0x8001003C`의 `e_lfanew`)에서 마지막 섹션의 끝을 구해 코드 세그먼트 디스크립터(`0x00C09B00 | limit`)를 만들고, `0008A804`가 `sgdt`로 GDT 항목 1(셀렉터 8)과 바꾼 뒤 `ljmp 8:...`로 CS를 다시 읽는다. 이전 디스크립터는 `[0x00A93978]`/`[0x00A9397C]`에 두고, `0008A7E0`(재부팅 전 경로)이 0이 아닐 때만 되돌린다.
* **확인됨**: HLE에서는 rexbox의 물리 메모리 창 때문에 `0x80010000`에 타이틀 자신의 XBE 헤더가 보여, lift 본문이 엉뚱한 섹션 표를 읽고 게스트 RAM 밖(`7509EE66`)을 읽다 fault한다. `decomp/xapi_code_selector.c`가 이 함수를 아무것도 하지 않는 어댑터로 바꾼다(저장값 0 → 복원 경로도 건너뜀).

## 8. 커널 데이터 `XeImageFileName`

* **확인됨**: XAPI(`0008723B`)는 `\??\D:` 링크를 푼 뒤 `XeImageFileName`(326)을 `ANSI_STRING`으로 읽어 실행 이미지 경로와 D: 대상을 접두어 비교한다. rexbox v0.0.21에는 이 export가 없어 fault했다(rexbox #49에서 `\Device\CdRom0\DEFAULT.XBE`로 해결).

## 9. D3D8LTCG와 디바이스 푸시 버퍼

* **확인됨**: XbSymbolDatabase가 D3D8LTCG 함수 104개를 식별하고, 그중 22개가 레지스터 규약 변형이다: `CDevice_InitializeFrameBuffers_4__LTCG_edi1`, `CDevice_KickOff_0__LTCG_edx1`, `CDevice_SetStateUP_0__LTCG_esi1`, `D3DDevice_GetViewportOffsetAndScale_0__LTCG_edx1_ecx2`, `D3DDevice_LightEnable_4__LTCG_eax1`, `D3DDevice_LoadVertexShader_0__LTCG_edx1_eax2`, `D3DDevice_SelectVertexShader_0__LTCG_eax1_ebx2`, `D3DDevice_SetFlickerFilter_0__LTCG_esi1`, `D3DDevice_SetIndices_4__LTCG_ebx1`, `D3DDevice_SetPixelShaderConstant_4__LTCG_ecx1_eax3`, `D3DDevice_SetStreamSource_4__LTCG_eax1_ebx2`, `D3DDevice_SetTextureStageStateNotInline_0__LTCG_ecx1_eax2_edx3`, `D3DDevice_SetTransform_0__LTCG_eax1_edx2`, `D3DDevice_Swap_0__LTCG_eax1`, `D3D_BlockOnResource_0__LTCG_eax1`, `D3D_CommonSetMultiSampleModeAndScale_4__LTCG_eax1`, `D3D_DestroyResource_0__LTCG_edi1`, `D3D_MakeRequestedSpace_4__LTCG_eax2`, `D3D_SetTileNoWait_0__LTCG_eax1_ecx2`, `Direct3D_CreateDevice_4__LTCG_eax1_ecx3`, `Get2DSurfaceDesc_0__LTCG_edi1_ebx2_esi3`, `Lock2DSurface_16__LTCG_esi4_eax5`.
* **확인됨**: 변형 이름의 인자 번호는 LTCG가 줄인 서명의 번호다(`Direct3D_CreateDevice_4__LTCG_eax1_ecx3` = BehaviorFlags eax, pPresentationParameters 스택, ppDevice ecx; 상세는 rexbox 설계 i050).
* **확인됨**: lift된 `Direct3D_CreateDevice`는 라이브러리의 정적 디바이스 `0x001835E0`을 쓰고 `[0x00185A88]`에 그 주소를 둔다. 디바이스의 `+0`은 푸시 버퍼 쓰기 포인터(`pPut`)로 쓰인다.
* **확인됨**: `D3D_MakeRequestedSpace`(`00179470`)는 D3D 라이브러리 함수 다수와 **게임 코드 함수 7개**(`0002AA40`, `0003B380`, `0003BA50`, `0003BDC0`, `000765E0`, `0007F1D0`, `0007FFA0`)가 부른다. 게임 코드는 인라인된 D3D 호출로 NV2A 메서드 헤더와 값을 `pPut`에 직접 쓰고 전진시킨다(예: `000765E0`의 `0x0004030C`, `0x00041D78`). 식별되지 않은 D3D 내부 함수(`00176C20` 등)도 같다.
* **추정**: API 경계 HLE만으로는 이 명령이 GPU 상태에 반영되지 않는다. 디바이스 푸시 버퍼의 NV2A 메서드를 해석하는 경로가 필요하다(rexbox의 `BeginPush`/`EndPush` 해석의 확장).

## 10. 네트워크 초기화

* **확인됨**: 게임의 네트워크 초기화(`00080E70`)는 `XNetStartup`(`0012D051`)이 0이 아니면 `E_FAIL`을 돌려주고, 호출자는 음수면 XOnline 시작(`0010D9B7`)을 건너뛴다. lift된 XNET은 `KeInitializeInterrupt`(벡터 52) 뒤 NIC MMIO(`0xFEF00000`)를 프로그래밍한다. rexbox #53이 `XNetStartup`을 실패로 돌려 오프라인 경로를 탄다.

## 11. 사운드: 스트림, XACT

* **확인됨**: 부팅 중 PCM 스트림 하나(2 ch, 44.1 kHz, 16 bit, 최대 43 패킷, 콜백 `0007E9C0`)와 Xbox ADPCM 효과음 버퍼 18개(모노·스테레오 44.1 kHz)를 만든다.
* **확인됨**: `CDirectSoundStream` vtable(`0x00190470`): AddRef, Release, GetInfo, GetStatus, Process, Discontinuity, Flush, 소멸자. 콜백은 `(stream context, packet context, status)` stdcall이며 둘 다 0이면 컨텍스트 `+8`을 늘린다.
* **확인됨**: XACT 엔진(`XACTENG` 섹션, 식별되지 않음)의 시간 함수 `0016F12A`는 IRQL을 올리고 APU 레지스터 `0xFE80200C`를 읽어(48 kHz 전역 샘플 카운터로 **추정**) 엔진 `+0x144`(직전 값), `+0x148`(64비트 누계)을 갱신하고 `누계 × 10000 / 48000 × 1000`(100 ns)을 돌려준다. XACT에서 하드웨어를 직접 만지는 곳은 이 함수뿐이다(상수 주소 기준).

## 12. D3D8 렌더 경로 (XDK 5849)

* **확인됨**: 렌더 상태 배치는 simple 0~91(0~81은 4134의 메서드, 82 `0x1D78`, 83 `0x147C`, 84~91 자리 채움, 메서드 표 `0x001B63B0`), deferred 92~135(44개, 기본값 표 `0x001825E4`), complex 136~165(30개, setter 표 `0x00182038`, `SampleAlpha`가 인덱스 22). 기본 상태 루틴 `0x0017A1E0`이 슬롯 57~165를 돈다.
* **확인됨**: 디바이스의 변환 배열은 `g_pDevice + 0xC60 + State × 64`(State: View 0, Projection 1, Texture 2~5, World 6~9), 더티 비트 표 `0x00181E58`, 더티 워드 `[0x00185A90]`. 게임 코드는 World를 인라인으로 쓴다(`0x00077390`).
* **확인됨**: 2D는 768×576 원점 중심 정사영(`1/384`, `1/288`)과 World 이동(−320, −240)으로 그린다. 동영상(XMV)은 640×480 선형 YUY2(`0x24`) 텍스처의 사각형이고, 둘레는 검은 사각형으로 덮는다. 포그를 켠 채 specular 없는 FVF `0x152`로 그린다.

## 13. 부팅 BGA와 libpng (issue #6)

* **확인됨**: 부팅 순서는 `BGA\ESRB.DAT`(ESRB 공지) → `81.DAT`(AM 로고) → `ML.DAT`(Andamiro·Mastiff) → `FREEVOLT.XMV` → `INTRO.XMV`다. DAT는 `RESPACK` 묶음이고, 그 안의 이미지는 libpng 1.2.5(`D3DX` 섹션, `png_handle_IHDR` `0x000E0737`)로 읽는다. 상태 객체 vtable은 ESRB가 `0x001B72A4`, 로고가 `0x001B7330`이다. BGA 핸들은 레이어 100개(`+0x14`)를 가지며 `sub_0006F560` → `sub_0006ED40`(키프레임 보간, ID3DXMatrixStack `[0x83FB28]`)으로 그린다. 텍스처 표는 `[0x1D1B44]`(항목 0x34바이트, 너비 `+0x32`, 높이 `+0x34`)다.
* **확인됨**: 이 타이틀의 실제 코드에 있는 8비트 단일 피연산자 곱은 `0x000E08F8 imul bl`, `0x000E2D32 imul cl`, `0x000E5F67 mul cl`(libpng), `0x000F1F31 imul bl`(DSOUND)이다.

## 14. 미해결 질문

* 물리 주소 창(`| 0x80000000`)을 타이틀이 쓰는지. 앱은 reCT3처럼 켜 두었다(**미확정**).
* WMA·XMV 디코딩의 CPU 비용과 DSOUND 스트림 형식.
* 댄스 패드가 XInput 장치 유형 중 무엇으로 열리는지(게임패드, 또는 다른 장치 유형).

---

# Exceed XBE overview: build variants, sections, XDK libraries, kernel imports, the disc

Based on: [work log i001](../work-logs/20261103-i001-project-bootstrap.md). The values were read from the user's dump (`roms/DEFAULT.XBE`) with `scripts/recomp/register_xbe.py`, by reading the XBE header, and from the load-stage output of `reexceed.exe roms`. None of the game's bytes are carried over; only the identity and layout are recorded.

## 1. Build variants

The table above applies: the North America build (`exceed_us`), region `0x00000001`, MD5 `575f861eb700e86a82c0ad530dc4a310`, 1,896,448 bytes, title id `41440002`, certificate time `42F8EC5C` (2005-08-09), image time `42EDCD9F` (2005-08-01).

* **Confirmed**: the certificate title is `PUMP IT UP: EXCEED`, the title id's two letters are `AD` (Andamiro), certificate version 1, media flags `0x202`, rating 3.
* **Confirmed**: the disc holds three more XBEs with the same title id and certificate time: `UPDATE.XBE` (Online Updater Application), `DOWNLOADER.XBE` (Downloader) and `DASHUPDATE.XBE` (Xbox Dashboard Updater), all XDK tools for Xbox Live and the dashboard. This project targets `default.xbe` only.
* **Unresolved**: whether other regional builds (a Korean one, for instance) exist, and their identity.

## 2. Sections and memory layout

* **Confirmed**: base `0x00010000`, image size `0x00A94D20` (about 10.6 MiB, mostly `.data`'s BSS), entry point `0x0008673A` (retail key), TLS directory `0x00190130`, kernel thunk table `0x0018F8C0` (the start of `.rdata`).
* Nineteen sections, as in the table above: `.text`, then the libraries `D3DX`, `DSOUND`, `WMADEC` (the WMA decoder for the songs in `AUDIO/*.WMA`), `XONLINE`, `XNET`, `XMV` (movie playback for `BGA/*.XMV` and `CREDIT.XMV`), `XACTENG` (XACT sound banks, `MEDIA/*.xsb`/`*.xwb`), `XGRPH`, `D3D` (D3D8, LTCG), `XPP` (peripherals), then `.rdata` (with the kernel thunks), `.data` (raw size `0xF974`, the rest zero-filled), `DOLBY` (DSP code) and the small `.data1`, `XON_RD`, `$$XTIMAGE`, `$$XSIMAGE`, `.XTLID`.
* **Confirmed**: at the app's load stage the 19 sections are placed in 128 MiB of guest memory, leaving 120,172 KiB of heap.

## 3. XDK libraries

* **Confirmed**: all are XDK 1.0.**5849**: `XAPILIB`, `D3DX8`, `DSOUND`, `XVOICE`, `VOICMAIL`, `XONLINES`, `XBOXKRNL`, `XMV`, `UIX`, `LIBCPMT`, `LIBCMT`, `D3D8LTCG`, `XGRAPHCL`, `XACTENLT`.
* **Inferred**: being newer than reJSRF's and reCT3's (4531) XDKs, rexbox's D3D8 HLE (render-state numbering, caps, the default-state routine) and DSOUND HLE may need the 5849 layout; to be checked with the identified `D3DRS_*` variables once the XBE is lifted.
* **Inferred**: there is `D3D8LTCG` and no `D3D8`; `hle_libraries.json` selects both names.
* **Unresolved**: whether `UIX` (the Xbox Live UI library) and `XVOICE`/`VOICMAIL` are called on the start path without Live.

## 4. Kernel imports

* **Confirmed**: 146; rexbox v0.0.21's kernel HLE binds 104 and has no export for 42.
* The imports without HLE (from the load stage; implemented in rexbox when the boot reaches them), grouped: the I/O manager and devices (65, 62, 74, 81, 83, 84, 85, 86, 87, 359, 247), files (210, 195, 197), memory and threads (181, 169, 170, 142, 139), time (126, 127, 228), runtime (302, 312, 260, 308, 270), cryptography for Xbox Live (340, 338, 339, 345, 346, 347, 349, 344, 355), the network PHY (253, 252) and others (49, 360, 95, 326); the names are in the Korean list above.
* **Inferred**: the I/O manager imports (creating devices, IRPs) come from XAPI's memory-unit and hard-disk device code or from Xbox Live, and `Xc*` and `Phy*` from Xbox Live and the network; the start path without Live may never reach them.

## 5. Disc layout

* **Confirmed**: `AUDIO/` (208 entries, song music `*.WMA`), `BGA/` (188, background movies `*.XMV` and `*.DAT`), `TITLE/` (200, song banners `*.PNZ`), `WAVE/` (44, sound effects `*.WAV`), `MEDIA/` (fonts `*.xpr` and `ArialUni.Ttf`, XACT `*.xsb`/`*.xwb`, `UIX.uix`, save images `*.xbx`), and at the root `STEP.DAT` (the step charts, **inferred**), `DSSTDFX.BIN` (the DSOUND effects image) and `CREDIT.XMV`.
* **Inferred**: `DSSTDFX.BIN` is the standard DSP effects image `XAudioDownloadEffectsImage` reads (the same path as in reCT3).

## 6. Open questions

* Whether the title uses the physical address window (`| 0x80000000`); the app turns it on as reCT3 does (**unresolved**).
* The CPU cost of WMA and XMV decoding, and the DSOUND stream formats.
* Which XInput device type the dance pad opens as (a gamepad, or another device type).

## 6–9. Lift, start-up and D3D8LTCG (issue #4)

* **Confirmed (lift)**: upstream xboxrecomp v0.16.0 lifts 5,547 functions (no failures, about 1.04 million lines of C, 23 files), with 3 unresolved call targets (`000E4AB2`, `001137C9`, `0018F8E2`) and 74 unimplemented instructions of 14 kinds, mostly `daa`/`aaa`/`das`/`aas` at `0x0014Dxxx` in `XMV` (data read as code, **inferred**) and privileged instructions (`cli`/`sti`, `sgdt`, `ljmp`, `lldt`, `clts`, `wbinvd`). 361 XDK symbols are identified (104 D3D8LTCG, 159 DSOUND, 43 XAPILIB and 11 XONLINES functions; 27 D3D8 variables, 23 of them `D3DRS_*`), giving 268 HLE overrides and 44 XDK variable bindings; `XInputPoll`, `XInputSetState`, `XMountMUA` and `XUnmountMU` are not identified. The start begins `mainCRTStartup` (`0008673A`) → `PsCreateSystemThreadEx` → `XapiThreadStartup` (`000844CD`) → `mainXapiStartup` (`000866C6`) → `00087E76` → `00087E12`.
* **Confirmed (kernel dependence)**: `00087E76` picks a kernel-code address by `XboxHardwareInfo` (322) and `XboxKrnlVersion` (324) and patches four bytes of kernel code through `MmQueryAddressProtect`/`MmSetAddressProtect` and `cli`/`sti`; with rexbox's build 5838 the path is not taken. `00087E12` reads the kernel PE header (`e_lfanew` at `0x8001003C`), builds a code-segment descriptor (`0x00C09B00 | limit`) ending at the kernel's last section, and `0008A804` swaps it into GDT entry 1 (selector 8) with `sgdt` and `ljmp 8:...`, keeping the old one at `[0x00A93978]`/`[0x00A9397C]` for `0008A7E0` to restore only when nonzero. Under the HLE the physical window shows the title's own XBE header at `0x80010000`, so it faulted; `decomp/xapi_code_selector.c` makes it do nothing.
* **Confirmed (`XeImageFileName`)**: XAPI (`0008723B`) resolves `\??\D:` and prefix-compares the running image's path from `XeImageFileName` (326, an `ANSI_STRING`) with it; rexbox v0.0.21 lacked the export (rexbox #49 adds `\Device\CdRom0\DEFAULT.XBE`).
* **Confirmed (D3D8LTCG)**: 22 of the 104 D3D8LTCG functions are register-convention variants (listed in the Korean section); their parameter numbers count the LTCG function's reduced signature (rexbox design i050). The lifted `Direct3D_CreateDevice` uses the library's static device `0x001835E0` and stores its address at `[0x00185A88]`; the device's `+0` is the push-buffer write pointer (`pPut`). `D3D_MakeRequestedSpace` (`00179470`) is called by many D3D library functions and by **seven game functions** (`0002AA40`, `0003B380`, `0003BA50`, `0003BDC0`, `000765E0`, `0007F1D0`, `0007FFA0`), whose inlined D3D calls write NV2A method headers and values straight to `pPut` (in `000765E0`, `0x0004030C` and `0x00041D78`), as do unidentified D3D internals (`00176C20` and others). **Inferred**: API-level HLE alone does not carry these commands into the GPU state; the device push buffer's NV2A methods need to be interpreted (an extension of rexbox's `BeginPush`/`EndPush` interpreter).

## 10–11. Network start-up and sound (issue #4)

* **Confirmed (network)**: the title's network init (`00080E70`) returns `E_FAIL` when `XNetStartup` (`0012D051`) returns nonzero, and its caller skips XOnline's start-up (`0010D9B7`) on a negative result; the lifted XNET programs the NIC's MMIO (`0xFEF00000`) after `KeInitializeInterrupt` (vector 52). rexbox #53 fails `XNetStartup` so the offline path runs.
* **Confirmed (sound)**: at boot one PCM stream (2 ch, 44.1 kHz, 16 bit, at most 43 packets, callback `0007E9C0`) and 18 Xbox ADPCM effect buffers (mono and stereo, 44.1 kHz) are created. The `CDirectSoundStream` vtable (`0x00190470`) is AddRef, Release, GetInfo, GetStatus, Process, Discontinuity, Flush and the destructor; the callback is `(stream context, packet context, status)` stdcall and counts completions at its context `+8` when both are 0. XACT's time function `0016F12A` (the `XACTENG` section, unidentified) raises the IRQL, reads the APU register `0xFE80200C` (the 48 kHz global sample counter, **inferred**), updates the engine's `+0x144` (last value) and `+0x148` (64-bit total) and returns `total x 10000 / 48000 x 1000` (100 ns); it is XACT's only direct hardware access (by constant address).

## 12. The D3D8 render path (XDK 5849)

* **Confirmed**: the render-state layout is simple 0..91 (0..81 with 4134's methods, 82 `0x1D78`, 83 `0x147C`, 84..91 filler; method table `0x001B63B0`), deferred 92..135 (44; default table `0x001825E4`) and complex 136..165 (30; setter table `0x00182038`, `SampleAlpha` at index 22); the default-state routine `0x0017A1E0` walks slots 57..165.
* **Confirmed**: the device's transform array is `g_pDevice + 0xC60 + State x 64` (View 0, Projection 1, Texture 2..5, World 6..9), with the dirty-bit table `0x00181E58` and the dirty word `[0x00185A90]`; game code writes World inline (`0x00077390`).
* **Confirmed**: 2D is drawn through a 768x576 origin-centred orthographic projection (`1/384`, `1/288`) with a World translation of (−320, −240); the movie (XMV) is a quad of a 640x480 linear YUY2 (`0x24`) texture with black quads over the borders, drawn with fog on and FVF `0x152`, which has no specular.

## 13. Boot BGA and libpng (issue #6)

* **Confirmed**: the boot order is `BGA\ESRB.DAT` (the ESRB notice) → `81.DAT` (the AM logo) → `ML.DAT` (Andamiro and Mastiff) → `FREEVOLT.XMV` → `INTRO.XMV`. The DATs are `RESPACK` bundles whose images are read through libpng 1.2.5 (the `D3DX` section, `png_handle_IHDR` `0x000E0737`); the state objects' vtables are `0x001B72A4` (ESRB) and `0x001B7330` (logos); a BGA handle holds 100 layers (`+0x14`) drawn through `sub_0006F560` → `sub_0006ED40` (keyframe interpolation, the ID3DXMatrixStack `[0x83FB28]`); the texture table is `[0x1D1B44]` (0x34-byte entries, width `+0x32`, height `+0x34`).
* **Confirmed**: the title's real code has four 8-bit one-operand products: `0x000E08F8 imul bl`, `0x000E2D32 imul cl` and `0x000E5F67 mul cl` (libpng), and `0x000F1F31 imul bl` (DSOUND).
