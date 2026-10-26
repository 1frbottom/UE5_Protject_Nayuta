# Project Nayuta

> UE5 4인 로그라이크(리슨서버)

<p align="center">
  <img src="https://github.com/user-attachments/assets/07ae800f-dfed-4b8f-b5fb-366bfa07fd53" width="600" />
</p>

* 유튜브 ([Preview](https://youtu.be/VkQyTnjKNqA))
* 실행파일 ([Google Drive](https://drive.google.com/drive/folders/1pTrk9NoL3PSQWJ5tqH26mLx6ymtuvojM?usp=drive_link))

<br>

## 프로젝트 개요

* **개발 기간 :** 2026.05 ~ 진행중
* **개발 인원 :** 1명
* **엔진 및 언어 :** Unreal Engine 5.4 / C++ / Steam OSS
* **플랫폼 :** PC (Windows)

<br>

## 핵심 구현

### 1. 대규모 몬스터 처리

* [활성화, 시드복제](Source/ProjectNayuta/Characters/CharacterMonsters/NYMonsterBase.cpp#L319-L452)
* [전용 이동컴포넌트](Source/ProjectNayuta/Characters/CharacterMonsters/NYMonsterMovementComponent.cpp#L43-L169)
* [오브젝트 풀](Source/ProjectNayuta/Monsters/NYMonsterPoolComponent.cpp#L14-L91)

### 2. 서버 권위 멀티플레이

* [서버 공격 생성](Source/ProjectNayuta/Weapons/NYWeaponComponent.cpp#L341-L447)
* [GameState 페이즈 복제](Source/ProjectNayuta/Game/NYGameStateStage.cpp#L26-L59)
* [보상선택 서버 검증](Source/ProjectNayuta/Player/NYPlayerStateStage.cpp#L420-L518)

<br>

## 현재 구현 범위

| 맵 | 내용 |
| :--- | :--- |
| `LV_MainMenu` | Steam 세션 생성 / 참가 / 초대 |
| `LV_Lobby` | Listen Server 로비, 스테이지로 Seamless Travel |
| `LV_Stage1` | 웨이브 전투, 몬스터 풀, 보상 페이즈 |
| `LV_TrainingRoom` | 무기·전투 샌드박스 |

<br>

## 실행 방법

1. 위 Google Drive 링크에서 파일을 받아 압축을 해제합니다.
2. `Windows/ProjectNayuta.exe`를 실행합니다.
3. 싱글플레이, 멀티플레이(세션 생성 후 Steam 친구 초대), 훈련장을 플레이할 수 있습니다.
