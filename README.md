# 🗺️ 하정빈 포트폴리오

> "프레임의 낭만, 끝까지 쫓다."
>
> **C++, 언리얼 기반 게임 클라이언트 프로그래머.** <br>
> 1 프레임의 성능 최적화를 위해 끝까지 파고드는 개발자 하정빈입니다. "동작하는 코드"를 넘어 "성능과 구조가 아름다운 코드"를 지향하며, 엔진 레벨의 깊이 있는 이해를 바탕으로 문제를 해결합니다.

### 🛠️ Tech Stack
<img src="https://img.shields.io/badge/C++-00599C?style=flat-square&logo=c%2B%2B&logoColor=white"/> <img src="https://img.shields.io/badge/Unreal Engine 5-0E1128?style=flat-square&logo=unrealengine&logoColor=white"/> 

---

## 목차<a name="table-of-contents"></a>

<table>
  <thead>
    <tr>
      <th>🛡️ Grid Defense 프로젝트 &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td valign="top">
        <br>
        <b><a href="#eternal-return-main">🎮 프로젝트 메인</a></b><br>
        <b><a href="#game-overview">📖 게임 개요</a></b><br>
        <b><a href="#learning-objectives">📌 학습 목표 및 달성</a></b><br>
        <b><a href="#main-features">🔨 주요 개발</a></b><br>
        <b><a href="#troubleshooting-eternal-return">🛠️ 문제 해결</a></b><br>
        &nbsp;&nbsp; └ <a href="#save-sync"> 랜덤 맵 생성과 세이브 데이터 간의 실행 순서 동기화 이슈 </a><br>
        &nbsp;&nbsp; └ <a href="#grid-block"> 실시간 그리드 점유 변경 시 경로 단절 및 붕괴 이슈 </a><br>
        &nbsp;&nbsp; └ <a href="#boss-optimization"> 대량의 투사체 및 몬스터 스폰 시 프레임 스파이크 최적화 </a><br>
      </td>
    </tr>
  </tbody>
</table>
<br>
<br>

---
<br>
<br>

# 🛡️ Grid Defense 프로젝트<a name="eternal-return-main"></a>

### 📌 프로젝트 정보

| 항목 | 내용 |
|:---:|:---:|
| 🎯 **장르** | 그리드 기반 지능형 타워 디펜스 |
| ⏱️ **개발 기간** | 1개월 |
| 👥 **개발 인원** | 1인 |
| 🛠️ **개발 환경** | C++, Unreal Engine 5, UMG |
| 💾 **GitHub** | [소스코드 레포지토리 바로가기](https://github.com/hauras/Grid_Defense) |

## 📑 프로젝트 목차<a name="toc-eternal"></a>

**1. 📖 [게임 개요](#game-overview)**

**2. 📌 [학습 목표 및 달성](#learning-objectives)**

**3. 🔨 [주요 개발 기능](#main-features)** <br>
&nbsp;&nbsp; └ <a href="#flow-field">Flow Field Navigation (BFS 기반 대규모 유닛 경로 탐색)</a><br>
&nbsp;&nbsp; └ <a href="#grid-validation">Predictive Grid Validation (실시간 건설 시뮬레이션)</a><br>
&nbsp;&nbsp; └ <a href="#object-pooling">High-Performance Object Pooling (객체 풀링 및 GC 최적화)</a><br>
&nbsp;&nbsp; └ <a href="#gameplay-tag">GameplayTag 기반 버프 / 속성 상성 시스템</a><br>

<br>

---

## 1. 📖 게임 개요 <a name="game-overview"></a>

> **"매번 변하는 전략의 판, 최적의 경로를 설계하여 넥서스를 방어하라!"**

언리얼 엔진 5의 C++를 기반으로 개발한 그리드 기반 타워 디펜스 게임입니다. 단순히 타워를 짓는 재미를 넘어, **Flow Field(BFS)** 알고리즘을 통해 수백 마리의 유닛이 실시간으로 변화하는 지형에 맞춰 최적의 경로를 찾아가는 지능형 시스템을 구축했습니다. 특히 클라이언트 사이드에서의 **런타임 연산 최적화**와 **GameplayTag 기반의 확장 가능한 게임 로직 설계**에 집중했습니다.

<div align="center">
  <img src="여기에_메인_하이라이트_GIF_경로_삽입" alt="게임 메인 플레이 화면" width="80%">
  <br>
  <i>(수많은 몬스터가 넥서스로 향하는 도중, 유저의 타워 건설로 경로가 실시간 우회되는 모습)</i>
</div>

<br>

## 2. 📌 학습 목표 및 달성 <a name="learning-objectives"></a>

본 프로젝트는 알고리즘을 통한 성능 최적화와 확장 가능한 게임 로직 설계를 깊이 있게 탐구하는 것을 목표로 진행되었습니다.

* **대규모 유닛 패스파인딩 최적화:** 개별 유닛의 A* 연산을 지양하고, 그리드 전역 벡터 필드 방식인 **Flow Field**를 직접 구현하여 수백 개의 액터가 동시 구동되는 환경에서도 안정적인 프레임을 확보했습니다.
* **실시간 지형 변화에 따른 데이터 무결성 확보:** 타워 건설로 인한 동적 장애물 생성 시, BFS 탐색을 통해 경로 단절 여부를 사전에 검증하는 예측 시스템을 구축했습니다.
* **확장 가능한 게임 로직 아키텍처:** 하드코딩 없이 GameplayTag와 DataTable을 조합하여, 코드 수정 없이 에디터에서 버프/속성 상성을 자유롭게 추가/수정할 수 있는 데이터 주도 설계를 달성했습니다.
* **GC 부하 최소화:** 직접 설계한 Object Pool 시스템으로 투사체와 이펙트의 SpawnActor/Destroy 반복 호출을 제거하여 메모리 단편화와 프레임 스파이크를 방지했습니다.

<br>

## 3. 🔨 주요 개발 기능 <a name="main-features"></a>

### 🧭 [경로 탐색] BFS 기반 Flow Field 대규모 유닛 최적화 <a name="flow-field"></a>

전통적인 길찾기 알고리즘의 연산 병목 현상을 극복하고, 디펜스 게임에 최적화된 그룹 경로 탐색 시스템을 구현했습니다.

<div align="center">
  <img src="여기에_경로_변경_GIF_경로_삽입" alt="실시간 경로 재계산 로직" width="80%">
  <br>
  <i>"개별 유닛의 연산을 배제하고, 그리드 타일에 구워진 방향 벡터(Vector)만 참조하여 이동하는 O(1) 비용의 고효율 길찾기 시스템"</i>
</div>
<br>

**1. Integration Field & Flow Field 구축**
* **알고리즘 설계:** 모든 타일에서 목적지(Nexus)까지의 최단 거리를 계산하는 BFS 기반의 Integration Field를 생성하고, 이를 인접 타일 간의 경사도(Gradient)로 변환하여 방향 벡터를 도출했습니다.
* **성능적 이점:** 수백 마리의 몬스터가 스스로 경로 연산을 하지 않고, 자신이 위치한 타일의 나침반(Vector)만 읽어 이동하도록 설계하여 CPU 부하를 획기적으로 낮췄습니다.

**2. 실시간 동적 경로 재계산**
* 유저가 타워를 건설하거나 파괴할 때마다 즉시 해당 구역의 비용을 갱신하고 Flow Field를 전역에 재전파하여, 몬스터들이 런타임 중에도 자연스럽게 우회 경로를 찾도록 구현했습니다.

<br>

### 🏗️ [그리드 검증] Predictive Grid Validation (실시간 예측 검증) <a name="grid-validation"></a>

유저의 자유로운 건설을 보장하되, 게임 로직의 붕괴를 사전에 방지하는 정교한 시뮬레이션 시스템입니다.

<div align="center">
  <img src="여기에_건설_거부_UI_GIF_경로_삽입" alt="건설 불가 시뮬레이션" width="80%">
</div>

**1. 가상 점유 시뮬레이션 및 트랜잭션(Transaction) 로직**
* 유저가 타워 설치를 시도하는 찰나, 맵 배열의 해당 인덱스를 임시로 '장애물'로 변경한 뒤 BFS 탐색을 돌려 스포너에서 출구까지의 경로 비용(`FlowCost`)이 유효한지 실시간으로 검증합니다.
* 경로가 완전히 단절될 경우(Cost가 최대치 도달), 타워 건설 및 재화 소모를 원천 차단하고 그리드를 원래 상태로 롤백하는 예외 처리 파이프라인을 구축했습니다.

<br>

### ⚔️ [성능 최적화] High-Performance Object Pooling <a name="object-pooling"></a>

수많은 유닛과 투사체가 쉴 새 없이 생성되고 파괴되는 디펜스 게임의 특성을 고려하여, 엔진 메모리와 가비지 컬렉터(GC) 부하를 최소화했습니다.

<div align="center">
  <img src="여기에_최적화_프레임_이미지_경로_삽입" alt="오브젝트 풀링 최적화 확인" width="80%">
</div>

**1. Actor Life-cycle 최적화 및 렌더링 컬링**
* 몬스터와 투사체를 매번 `SpawnActor`로 생성하지 않고, 런타임 이전에 미리 초기화해 두는 전용 `PoolManager`를 C++로 설계했습니다.
* 비활성화 시 `Destroy` 대신 `SetActorHiddenInGame(true)` 및 `SetActorEnableCollision(false)`를 호출하여 물리 및 렌더링 연산만 차단한 채 큐에 반환함으로써 메모리 단편화와 스파이크 렉을 방지했습니다.

**2. TMap 기반 클래스별 풀 분리 관리**
* 단일 배열이 아닌 `TMap<UClass*, FPoolArray>` 구조로 클래스 타입별 풀을 독립적으로 관리하여, 투사체 종류가 늘어나도 코드 수정 없이 자동으로 풀이 생성되는 확장성을 확보했습니다.

<br>

### 🏷️ [시스템 설계] GameplayTag 기반 버프 / 속성 상성 시스템 <a name="gameplay-tag"></a>

하드코딩된 조건문 없이, 데이터만으로 게임 로직을 제어하는 확장 가능한 아키텍처를 설계했습니다.

<div align="center">
  <img src="여기에_버프_카드_GIF_경로_삽입" alt="카드 버프 및 속성 상성 시스템" width="80%">
</div>

**1. GameplayTag 기반 타워-카드-적 연결 구조**
* 타워, 카드, 적 모두 `FGameplayTagContainer`를 보유하며, 웨이브 클리어 시 등장하는 카드 선택 UI에서 카드를 고르면 태그가 일치하는 모든 타워에 버프가 즉시 전파됩니다.
* `GameState`의 `OnBuffUpdated` 델리게이트를 통해 브로드캐스트되므로, 카드 선택 이후에 스폰된 타워도 `AppliedBuff` 배열을 순회하여 과거 버프를 누락 없이 적용받습니다.

**2. DataTable 기반 속성 상성 확장 구조**
* 기존 `if/else if` 하드코딩 방식에서 벗어나, `FElementalMatchup` 구조체 기반의 DataTable로 상성 데이터를 외부화했습니다.
* `TakeDamage()` 내부에서 테이블을 순회하여 공격 태그와 적 태그가 매칭되는 행을 찾아 배율과 텍스트 색상을 동적으로 적용합니다. 코드 수정 없이 에디터에서 새로운 속성 상성을 추가할 수 있습니다.

  <details>
  <summary><b>💡 핵심 C++ 로직 보기 (클릭하여 펼치기)</b></summary>

  ```cpp
  // EnemyBase::TakeDamage — DataTable 기반 상성 조회
  if (AttackTag.IsValid() && ElementalMatchupTable)
  {
      TArray<FElementalMatchup*> AllRows;
      ElementalMatchupTable->GetAllRows<FElementalMatchup>(TEXT(""), AllRows);

      for (FElementalMatchup* Row : AllRows)
      {
          if (Row->AttackTag == AttackTag && EnemyTags.HasTagExact(Row->EnemyTag))
          {
              DamageMultiplier = Row->DamageMultiplier;
              TextColor = Row->TextColor;
              break;
          }
      }
  }
  ```
  </details>

<br>

## 4. 🛠️ 문제 해결 (Troubleshooting) <a name="troubleshooting-eternal-return"></a>

### 1. 랜덤 맵 생성과 세이브 데이터 간의 실행 순서 동기화 이슈 <a name="save-sync"></a>
* **🔴 문제 상황:** 이어하기 기능 실행 시, 저장된 타워 데이터가 먼저 맵에 배치된 직후 '절차적 맵 생성(GenerateGrid)' 로직이 뒤늦게 실행되면서 타워가 허공에 뜨거나 장애물(바위)과 겹치는 치명적인 버그가 발생했습니다.
* **🔍 원인 분석:** `BeginPlay` 시점에서 게임 상태 복구 로직과 기본 맵 생성 로직 간의 **라이프사이클 실행 우선순위**가 제어되지 않아 발생한 로직 충돌이었습니다.
* **🟢 해결 방법 (Execution Order Control):**
  * `AGridManager`의 `BeginPlay` 최상단에 GameMode의 파라미터를 읽어오는 로직을 추가했습니다.
  * "LoadGame" 플래그가 참일 경우 기존 랜덤 생성 함수인 `GenerateGrid` 호출을 강제로 `return`시켜 건너뛰게 만들고, 저장된 맵 레이아웃 배열을 먼저 100% 복구한 뒤 그 위에 타워를 스폰하도록 실행 순서를 명확히 동기화했습니다.
* **✨ 결과:** 어떠한 상황에서도 맵의 지형지물과 타워의 위치가 저장 당시와 일치하는 심리스 복구 시스템을 완성했습니다.

### 2. 실시간 그리드 점유 변경 시 경로 단절 및 붕괴 이슈 <a name="grid-block"></a>
* **🔴 문제 상황:** 유저가 고의적으로 타워를 일렬로 지어 스포너에서 출구(Nexus)로 향하는 길을 완전히 틀어막을 경우, Flow Field 벡터가 소실되어 몬스터들이 제자리에 멈추거나 벽에 비비는 현상이 발생했습니다.
* **🔍 원인 분析:** 타워 건설을 승인한 '이후'에 경로 재계산을 돌리도록 설계되어 있어, 길이 막혔다는 사실을 엔진이 사후에 인지하여 대처가 불가능한 구조였습니다.
* **🟢 해결 방법 (Predictive BFS Simulation):**
  * `AddTower` 함수 내부에 **가상 검증 로직**을 도입했습니다. 타워 생성 명령이 들어오면 해당 그리드의 상태를 먼저 `Walkable = false`로 임시 변경한 뒤 즉시 `UpdateFlowField`를 돌립니다.
  * 이후 스폰 지점의 도달 비용(`FlowCost`)을 확인하여, 비용이 99999(무한대)일 경우 유저에게 에러 메시지를 띄우고 상태를 즉시 롤백(`Walkable = true`)하도록 구현했습니다.

  <details>
  <summary><b>💡 핵심 C++ 로직 보기 (클릭하여 펼치기)</b></summary>

  ```cpp
  // AGridManager::AddTower — 경로 검증 및 롤백
  GridArray[Index].bIsWalkable = false;
  UpdateFlowField();

  const bool bIsPathClear = (GridArray[GetIndex(0, 0)].FlowCost < INVALID_FLOW_COST);

  if (!bIsPathClear)
  {
      GridArray[Index].bIsWalkable = true;
      UpdateFlowField(); // 롤백 복구
      GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, 
          TEXT("설치 불가: 드래곤의 길을 완전히 막을 수 없습니다!"));
      return;
  }
  ```
  </details>

### 3. 대량의 투사체 및 이펙트 스폰 시 프레임 스파이크 최적화 <a name="boss-optimization"></a>
* **🔴 문제 상황:** 다수의 타워가 동시에 발사하는 상황에서 `SpawnActor`와 `Destroy` 반복 호출로 인해 GC(가비지 컬렉터)가 순간적으로 과부하되어 프레임이 급격히 떨어지는 스파이크 현상이 발생했습니다.
* **🔍 원인 분析:** 언리얼 엔진의 GC는 일정 주기로 메모리를 회수하는데, 짧은 시간에 대량의 액터가 생성/파괴되면 회수 대상이 누적되어 GC 실행 시점에 프레임이 집중적으로 저하됩니다.
* **🟢 해결 방법 (Object Pooling):**
  * `TMap<UClass*, FPoolArray>` 구조의 전용 `PoolManager`를 설계하여, 투사체와 이펙트를 재사용합니다.
  * 비활성화 시 `Destroy` 대신 `SetActorHiddenInGame`, `SetActorEnableCollision(false)`로 렌더링/물리만 차단한 채 풀에 반환하여 GC 호출 빈도를 대폭 낮췄습니다.
* **✨ 결과:** 대규모 교전 상황에서도 일정한 프레임을 유지하는 안정적인 성능을 확보했습니다.
