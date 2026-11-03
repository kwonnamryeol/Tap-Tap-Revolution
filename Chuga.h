#ifndef CHUGA_H
#define CHUGA_H

#include <windows.h>
#include <conio.h>
#include <string>
#include <fstream>
#include "Utils.h"

enum Judgment { NONE, PERFECT, GOOD, MISS }; // 판정 열거형

//  1. JudgmentSystem — 판정 표시
class JudgmentSystem
{
public:
    JudgmentSystem();
    Judgment judge(DWORD currentTime, float noteTime); // 판정 계산
    void  display(Judgment judge, int row);    // 열별 화면 출력 (row 1~5)
    void  clear(int row);      // 특정 열의 판정 텍스트 지우기
    void  countMiss();         // 미스 카운트 증가
    int getPerfectCount();   // Perfect 횟수 외부로 전달
    int getGoodCount();
    int getMissCount();

private:
    int perfectCount;  // Perfect 횟수
    int goodCount;     // Good 횟수
    int missCount;     // miss 횟수
};

//  2. PauseSystem — 일시정지
class PauseSystem
{
public:
    PauseSystem();  
    void  pause();  // isPaused = true
    void  resume();  // isPaused = false
    bool  getIsPaused();
    DWORD getTotalPausedTime(); // 총 일시정지 시간을 현재시간에서 빼는 함수

private:
    bool  isPaused;  // 게임이 일시정지 상태인지 아닌지를 나타내는 상태 플래그
    DWORD pauseStartTime; // 일시정지 버튼 누른 시각 기록
    DWORD totalPausedTime;  // 일시정지 되어있던 시간의 누적 합계
};

//  3. HpSystem — HP바
class HpSystem
{
public:
    HpSystem(int maxHp = 100);
    void damage(int amount);  // HP 감소
    void heal(int amount);    // HP 회복
    void draw();    // HP바 출력
    bool isGameOver();  // 게임오버 여부
    int  getHp();  // 남은 체력값 외부로 전달하는 함수

private:
    int hp;  // HP
    int maxHp; // 최대 HP
};

//  4. ScoreManager — 하이스코어
class ScoreManager
{
public:
    ScoreManager();
    void setPath(std::string dataPath);  // 곡 경로 설정
    int  load();  // 파일에서 불러오기
    void save(int score);   // 파일에 저장
    bool isNewRecord(int score); // 신기록 여부
    int  getHighScore(); // 저장된 최고 점수 반환

private:
    std::string scorePath;  // 하이스코어 데이터를 저장할 파일의 경로를 저장
    int highScore;  // 하이스코어
};

#endif // CHUGA_H

