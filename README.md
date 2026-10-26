# [UE5 4인 협동 로그라이크]

<p align="center">
  <img src="https://github.com/user-attachments/assets/07ae800f-dfed-4b8f-b5fb-366bfa07fd53" width="600" />
</p>

* [Youtube Preview](https://youtu.be/VkQyTnjKNqA)

<br><br>

## 프로젝트 개요
* **개발 기간 :** 2026.05 ~ 진행중
* **개발 인원 :** 1명
* **엔진 및 언어 :** Unreal Engine 5.4 / C++
* **플랫폼 :** PC (Windows)

<br><br>

## 핵심 구현

### 몬스터 Pooling 및 네트워크 최적화

다수의 몬스터를 반복해서 생성·제거하지 않도록 Object Pool로 재사용합니다.  
비활성 몬스터에는 Net Dormancy를 적용하고, 활성화 시 위치·타깃·이동 seed를 복제합니다. 몬스터 이동은 각 machine에서 계산하되, 복제된 seed로 이동 속도와 접근 각도를 동일하게 구성합니다.

* [몬스터 구조 및 복제 데이터](Source/ProjectNayuta/Characters/CharacterMonsters/NYMonsterBase.h)
* [활성화·비활성화 처리](Source/ProjectNayuta/Characters/CharacterMonsters/NYMonsterBase.cpp#L319-L449)
* [Object Pool 구현](Source/ProjectNayuta/Monsters/NYMonsterPoolComponent.cpp#L12-L73)
* [웨이브 기반 몬스터 배치](Source/ProjectNayuta/Monsters/NYMonsterSpawnComponent.cpp#L116-L186)

### Server-authoritative 전투

클라이언트는 공격 입력과 조준 방향만 RPC로 전달합니다.  
서버가 공격 가능 상태와 cooldown을 검증하고 damage를 계산한 뒤 공격 Actor를 생성합니다. Animation Montage의 `AttackCommit` 시점을 읽어 애니메이션과 실제 공격 발생 시점도 일치시켰습니다.

* [무기 시스템 구조](Source/ProjectNayuta/Weapons/NYWeaponComponent.h)
* [공격 cooldown 처리](Source/ProjectNayuta/Weapons/NYWeaponComponent.cpp#L278-L303)
* [서버 공격 판정 및 생성](Source/ProjectNayuta/Weapons/NYWeaponComponent.cpp#L341-L447)
* [클라이언트 입력 및 RPC](Source/ProjectNayuta/Characters/CharacterPlayers/NYCharacterPlayer.cpp#L350-L477)

### 멀티플레이 웨이브 및 보상 흐름

게임 진행은 서버에만 존재하는 `GameMode`가 담당하고, 클라이언트에 필요한 페이즈와 킬 수는 `GameState`로 복제합니다.  
각 플레이어의 보상 후보는 `PlayerState`에 OwnerOnly로 복제하며, 선택 결과는 서버에서 다시 검증합니다. 모든 플레이어가 선택을 마치거나 중간에 이탈한 경우 현재 인원을 다시 계산하여 다음 웨이브로 진행합니다.

* [웨이브 진행 구조](Source/ProjectNayuta/Game/NYGameModeStage.h)
* [웨이브 종료 및 보상 페이즈 전환](Source/ProjectNayuta/Game/NYGameModeStage.cpp#L321-L505)
* [공용 매치 상태 복제](Source/ProjectNayuta/Game/NYGameStateStage.h)
* [OwnerOnly 보상 복제 및 서버 검증](Source/ProjectNayuta/Player/NYPlayerStateStage.cpp#L420-L516)

<br><br>

## 현재 상태

| 맵 | 역할 |
| :--- | :--- |
| `LV_MainMenu` | 시작. Steam 세션 생성 / 참가 / 초대 |
| `LV_Lobby` | Listen Server 로비. 스테이지로 Seamless Travel |
| `LV_Stage1` | 웨이브 전투, 몬스터 풀, 보상 페이즈 |
| `LV_TrainingRoom` | 무기·전투 샌드박스 |

<br>

**구현 예정**

* **UI / UX :** 
* **전투 시스템 :**
* **웨이브 :**
* **사운드 :** 

<br><br>

## 실행방법

* 구글드라이브 [링크](https://drive.google.com/drive/folders/1pTrk9NoL3PSQWJ5tqH26mLx6ymtuvojM?usp=drive_link)
* 위파일 압축해제 후 Windows/ProjectNayuta.exe 실행
* 싱글플레이, 멀티플레이(세션생성 후 스팀 친구 게임참여), 훈련장 가능

<br><br>
