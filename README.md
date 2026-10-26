# ProjectNayuta

> Unreal Engine 5 Listen Server 기반 4인 협동 3D 로그라이크

<p align="center">
  <img src="https://github.com/user-attachments/assets/07ae800f-dfed-4b8f-b5fb-366bfa07fd53" width="600" />
</p>

[Youtube Preview](https://youtu.be/VkQyTnjKNqA) · [실행 파일 (Google Drive)](https://drive.google.com/drive/folders/1pTrk9NoL3PSQWJ5tqH26mLx6ymtuvojM?usp=drive_link)

<br>

## 프로젝트 개요

* **개발 기간 :** 2026.05 ~ 진행중
* **개발 인원 :** 1명
* **엔진 및 언어 :** Unreal Engine 5.4 / C++
* **플랫폼 :** PC (Windows)

<br>

## 핵심 구현

### 1. 대규모 몬스터 처리 구조

* [활성화·비활성화와 시드 복제](Source/ProjectNayuta/Characters/CharacterMonsters/NYMonsterBase.cpp#L319-L452)
* [CMC 대신 만든 바닥·스텝업 이동](Source/ProjectNayuta/Characters/CharacterMonsters/NYMonsterMovementComponent.cpp#L43-L169)
* [Object Pool 생성·회수](Source/ProjectNayuta/Monsters/NYMonsterPoolComponent.cpp#L14-L91)

### 2. 서버 권위 전투

* [클라이언트 입력 → 서버 RPC](Source/ProjectNayuta/Characters/CharacterPlayers/NYCharacterPlayer.cpp#L352-L440)
* [쿨다운 검증](Source/ProjectNayuta/Weapons/NYWeaponComponent.cpp#L276-L303)
* [몽타주 시점에 맞춘 공격 생성](Source/ProjectNayuta/Weapons/NYWeaponComponent.cpp#L341-L447)

### 3. 서버 권위 웨이브 / 보상

* [GameState 페이즈 복제와 UI 전달](Source/ProjectNayuta/Game/NYGameStateStage.cpp#L26-L59)
* [보상 페이즈 시작·다음 웨이브 전환](Source/ProjectNayuta/Game/NYGameModeStage.cpp#L434-L506)
* [OwnerOnly 복제 설정](Source/ProjectNayuta/Player/NYPlayerStateStage.cpp#L17-L38)
* [보상 선택 서버 검증](Source/ProjectNayuta/Player/NYPlayerStateStage.cpp#L420-L518)

### 4. Steam 세션

* [네트워크 실패 시 세션 정리와 메인메뉴 복귀](Source/ProjectNayuta/Game/NYGameInstance.cpp#L44-L75)

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
