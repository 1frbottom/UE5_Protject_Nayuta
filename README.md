# [UE5 4인 협동 로그라이크]

<p align="center">
  <img src="https://github.com/user-attachments/assets/e763eb87-4000-430b-afca-d9a05946055c" width="600" />
</p>

* 유튜브 [링크](https://www.youtube.com/watch?v=KwKsSuFFGsM)

<br><br>

## 프로젝트 개요
* **개발 기간 :** 2026.05 ~ 진행중
* **개발 인원 :** 1명
* **엔진 및 언어 :** Unreal Engine 5.4 / C++
* **플랫폼 :** PC (Windows)

<br><br>

## 현재 상태

**진행 중**

| 맵 | 역할 |
| :--- | :--- |
| `LV_MainMenu` | 시작. Steam 세션 생성 / 참가 / 초대 |
| `LV_Lobby` | Listen Server 로비. 스테이지로 Seamless Travel |
| `LV_Stage1` | 웨이브 전투, 몬스터 풀, 보상 페이즈 |
| `LV_TrainingRoom` | 무기·전투 샌드박스 |

<br>

**구현 예정**

* **UI / UX :** 메인메뉴 및 기타 UI 외관
* **전투 시스템 :** 플레이어 공격 애니메이션, 전투 피드백
* **웨이브 :** 스케일링
* **사운드 :** 기타 사운드

<br><br>

## 주요 클래스

| 클래스 | 역할 |
| :--- | :--- |
| [NYGameModeStage](Source/ProjectNayuta/Game/NYGameModeStage.h) | 스테이지 권한, 스폰, 몬스터 라이프사이클 |
| [NYGameStateStage](Source/ProjectNayuta/Game/NYGameStateStage.h) | 복제되는 매치 상태 (페이즈, 킬 수) |
| [NYPlayerControllerStage](Source/ProjectNayuta/Player/NYPlayerControllerStage.h) | 로컬 입력 / UI 진입 |
| [NYMonsterPoolComponent](Source/ProjectNayuta/Monsters/NYMonsterPoolComponent.h) | 서버 전용 몬스터 오브젝트 풀 |
| [NYWeaponComponent](Source/ProjectNayuta/Weapons/NYWeaponComponent.h) | 무기 슬롯·레벨 복제 |
| [NYStageContentRegistry](Source/ProjectNayuta/Data/NYStageContentRegistry.h) | 데이터 테이블 → 몬스터 정의 해석 |

<br><br>

## 실행방법

* 구글드라이브 [링크](https://drive.google.com/drive/folders/1pTrk9NoL3PSQWJ5tqH26mLx6ymtuvojM?usp=drive_link)
* 위파일 압축해제 후 Windows/ProjectNayuta.exe 실행
* 싱글플레이, 멀티플레이(세션생성 후 스팀 친구 게임참여), 훈련장 가능

<br><br>
