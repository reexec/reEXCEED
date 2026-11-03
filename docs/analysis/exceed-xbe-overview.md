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

## 6. 미해결 질문

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
