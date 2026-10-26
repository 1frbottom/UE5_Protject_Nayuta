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

* **몬스터 Pooling 및 네트워크 최적화**
    * Object Pool과 Net Dormancy를 적용하고, 이동 정보 대신 활성 상태와 seed를 복제합니다.
    * [헤더](Source/ProjectNayuta/Characters/CharacterMonsters/NYMonsterBase.h) · [활성화 및 복제](Source/ProjectNayuta/Characters/CharacterMonsters/NYMonsterBase.cpp#L319-L452) · [Pooling](Source/ProjectNayuta/Monsters/NYMonsterPoolComponent.cpp#L14-L90)

* **Server-authoritative 전투**
    * 클라이언트는 입력과 조준 방향만 전달하며, cooldown 검증과 공격 생성은 서버에서 처리합니다.
    * [헤더](Source/ProjectNayuta/Weapons/NYWeaponComponent.h) · [cooldown](Source/ProjectNayuta/Weapons/NYWeaponComponent.cpp#L276-L303) · [공격 생성](Source/ProjectNayuta/Weapons/NYWeaponComponent.cpp#L341-L447)

* **멀티플레이 웨이브 및 보상**
    * `GameMode`가 진행을 관리하고, 개인별 보상은 OwnerOnly로 복제한 뒤 서버에서 선택을 검증합니다.
    * [웨이브·보상 진행](Source/ProjectNayuta/Game/NYGameModeStage.cpp#L323-L506) · [보상 검증](Source/ProjectNayuta/Player/NYPlayerStateStage.cpp#L420-L518)

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
