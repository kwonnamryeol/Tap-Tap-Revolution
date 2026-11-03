# Tap Tap Revolution (C++ 콘솔 리듬게임 개선판)

[MichaelKim/Tap-Tap-Revolution](https://github.com/MichaelKim/Tap-Tap-Revolution)(MIT)을 기반으로,
대학 객체지향프로그래밍(C++) 수업 과제로 기능을 추가하고 개선한 버전입니다.

<!-- 실행 화면 GIF를 screenshots/ 에 추가하고 여기에 넣어 주세요 -->

원본은 노트 낙하와 키 입력 매칭만 있는 단순한 게임이었습니다. 직접 플레이하며 불편한 점과 버그를 찾고,
판정 표시·일시정지·HP·하이스코어 기능을 추가했으며, 렉의 원인을 찾아 렌더링 방식을 바꿨습니다.

## 실행 방법

- Windows + Visual Studio 2022 이상
- `taptaprevolution.sln`을 열고 x64로 빌드·실행
- C++17, `winmm.lib` 링크 (프로젝트에 설정되어 있음)
- 콘솔 글꼴이 깨지면 콘솔 글꼴을 Consolas로 바꿔 주세요

## 조작

- 메뉴: `w` / `s`로 이동, `Enter`로 선택
- 노트: 5개 열에 각각 `[d] [f] [Space] [j] [k]`
- 플레이 중 `ESC`: 일시정지 / 재개

## 원본 대비 바뀐 점

### 추가 기능
기능마다 독립 클래스를 만들어 `Chuga.h / Chuga.cpp`에 두고, `Game` 클래스가 멤버로 갖도록 했습니다.

| 기능 | 클래스 | 내용 |
|---|---|---|
| 판정 표시 | `JudgmentSystem` | 타이밍 오차 50ms 미만 PERFECT(+2), 150ms 미만 GOOD(+1), 그 밖은 MISS(-1). 열마다 `PF/GD/MS` 표시 |
| 일시정지 | `PauseSystem` | `ESC`로 음악·노트·타이머 정지. 누적 일시정지 시간을 경과 시간에서 빼서 재개 시 노트가 몰려 나오지 않게 보정 |
| HP 바 | `HpSystem` | HP 100에서 시작, MISS -1, PERFECT +2. 남은 HP에 따라 초록·노랑·빨강, 0이 되면 게임오버 |
| 하이스코어 | `ScoreManager` | 곡별 최고 점수를 `songs/<곡>_score.txt`에 저장하고 곡 선택 화면에 `[BEST: 점수]` 표시 |

### 랭크는 정확도 기준
만점은 모든 노트를 PERFECT로 처리한 점수(노트 수 × 2)입니다. 노트를 하나도 놓치지 않아도 전부 GOOD이면
만점의 50%라 Failed입니다. 놓치지 않는 것만으로는 부족하고, 타이밍을 정확히 맞춰야 높은 랭크를 받도록 설계했습니다.

| 랭크 | 조건 |
|---|---|
| SS (FULL COMBO) | 모든 노트 PERFECT |
| S / A / B / C / D | 만점의 90% / 80% / 70% / 60% / 50% 초과 |
| Failed | 만점의 50% 이하 |

### 원본 버그 수정
- 같은 키를 연속으로 누르면 두 번째 입력이 무시되던 문제
- 노트를 지우는 범위가 그리는 범위보다 좁아 생기던 잔상
- 히트·미스 후에도 노트가 화면에 남던 문제
- `clock()` 대신 `timeGetTime()` 사용, 16ms 프레임 제한으로 CPU 100% 점유 해소
- 어두운 노트 색을 밝은 노란색으로 바꿔 가시성 개선

### 렌더링 최적화
플레이 중 렉이 심해 처음에는 기기 성능 문제로 의심했지만, 코드를 분석해 보니 **매 프레임 모든 노트를 지우고 다시 그리는 방식**이
원인이었습니다. 노트마다 마지막으로 그린 y좌표(`lastY`)를 기억해, 위치가 실제로 바뀐 노트만 이전 자리를 지우고 새로 그리도록
바꿨습니다. 이후 렉이 사라져 곡을 끝까지 플레이할 수 있게 됐습니다.

```cpp
if (lastY.at(i) != newY)            // y가 바뀌었을 때만 갱신
{
    if (lastY.at(i) >= 0) clearNote(x, lastY.at(i));  // 이전 위치만 지움
    drawNote(x, newY, noteRow.at(i));
    lastY.at(i) = newY;
}
```

## 개발 과정과 AI 사용

- 원본 분석, 플레이하며 문제 발견, 원인 분석, 추가할 기능 설계는 직접 했습니다.
- Visual Studio 빌드 오류 해결은 Gemini, 코드 작성·수정은 Claude의 도움을 받았습니다.
- 커밋 기록에서 원본 → 빌드 환경 이전 → 기능 추가 순서로 바뀐 내용을 볼 수 있습니다.

## 노래 데이터 형식

`songs/` 폴더에 같은 이름의 `.mp3`와 `.txt`를 두면 곡으로 인식합니다. `.txt` 형식은 원본과 같습니다.

- 곡 제목
- 첫 노트까지의 지연 (ms)
- BPM
- 노트 수
- 노트마다: `번호` `열(1~5)` `다음 노트까지의 간격(ms)`

## 출처와 라이선스

- 원본 게임: [MichaelKim/Tap-Tap-Revolution](https://github.com/MichaelKim/Tap-Tap-Revolution), Copyright (c) 2016 Michael Kim, [MIT](./LICENSE)
- `dirent.h`: [tronkko/dirent](https://github.com/tronkko/dirent), Copyright (C) 1998-2019 Toni Ronkko, MIT
- `songs/`의 음원은 원본 저장소에 포함된 것이며, 저작권은 각 권리자에게 있습니다.
