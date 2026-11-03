#include "Chuga.h"
#pragma comment(lib, "winmm.lib")  // 프로그램을 컴파일할 때 "winmm.lib"이라는 라이브러리 파일을 함께 링크

using namespace std;

//  1. JudgmentSystem
JudgmentSystem::JudgmentSystem()
{
    perfectCount = 0;
    goodCount = 0;
    missCount = 0;
}

Judgment JudgmentSystem::judge(DWORD currentTime, float noteTime)
{
    int diff = abs((int)currentTime - (int)noteTime);  // 현재시각 - 노트 도착 예정시각
    if (diff < 50)  // 절댓값 차이가 50ms 미만
    {
        perfectCount++;  
        return PERFECT;  // PERFECT 리턴
    }
    else  // 절댓값 차이가 50ms 이상
    {
        goodCount++;
        return GOOD;  // GOOD 리턴
    }
    // MISS는 play()에서 별도 처리 (판정 범위 자체를 벗어났을 때라 이 함수가 호출되지 않음)
}

void JudgmentSystem::display(Judgment judge, int row)
{
    int x = row * 5 - 2;  // 각 열의 x 좌표 (drawNote의 계산식과 동일): row 1=3, 2=8, 3=13, 4=18, 5=23

    if (judge == PERFECT)
        printColor(0x1e, x, 26, "PF");  // 노란색 (Perfect)
    else if (judge == GOOD)
        printColor(0x1a, x, 26, "GD");  // 초록색 (Good)
    else if (judge == MISS)
        printColor(0x1c, x, 26, "MS");  // 빨간색 (Miss)
}

void JudgmentSystem::clear(int row)
{
    int x = row * 5 - 2;
    printColor(0x17, x, 26, "  "); // 공백 2칸으로 덮어써서 해당 열만 지우기
}

void JudgmentSystem::countMiss() // 미스 카운트 증가 (play()의 미스 처리부에서 호출)
{
    missCount++;
}

int JudgmentSystem::getPerfectCount() { return perfectCount; }  // PERFECT 횟수 외부로 보냄
int JudgmentSystem::getGoodCount() { return goodCount; }
int JudgmentSystem::getMissCount() { return missCount; }


//  2. PauseSystem
PauseSystem::PauseSystem()
{
    isPaused = false;
    pauseStartTime = 0;
    totalPausedTime = 0;
}

void PauseSystem::pause()
{
    isPaused = true;
    pauseStartTime = timeGetTime();        // 일시정지 시작 시각 기록
    mciSendString("pause music", 0, 0, 0); // 음악 일시정지
    printColor(0x1f, 8, 12, " PAUSED ");  // PAUSED 문구 출력
    printColor(0x17, 4, 14, "ESC to resume");  // ESC to resume 문구 출력
}

void PauseSystem::resume()
{
    isPaused = false;
    totalPausedTime += timeGetTime() - pauseStartTime; // 누적 일시정지 시간 갱신
    mciSendString("resume music", 0, 0, 0);            // 음악 재개
    // drawGrid()는 Game.cpp play() 안에서 호출
}

bool PauseSystem::getIsPaused()
{
    return isPaused;  // 일시정지
}

DWORD PauseSystem::getTotalPausedTime()
{
    return totalPausedTime;  // 일시정지 시간 반환
}


//  3. HpSystem
HpSystem::HpSystem(int maxHp)
{
    this->maxHp = maxHp;
    this->hp = maxHp; // 시작 시 HP 최대값으로 초기화
}

void HpSystem::damage(int amount)
{
    hp -= amount;  // 데미지 입으면 hp 줄어듬
    if (hp < 0) hp = 0; // 0 미만으로 내려가지 않게
    draw();  // HP 변화 시 갱신
}

void HpSystem::heal(int amount)
{
    hp += amount;  // hp 늘어남
    if (hp > maxHp) hp = maxHp; // 최대 HP 초과 방지
    draw();  // HP 변화 시 갱신
}

void HpSystem::draw()
{
    int barLen = hp / 5;  // 최대 20칸 (100 / 5 = 20)
    string bar = "[";
    for (int i = 0; i < 20; i++)
        bar += (i < barLen) ? "#" : "-";
    bar += "]";   // [##########----------] 형태로 HP바 그림

    // HP 상태별 색상
    int color;
    if (hp > 60) color = 0x1a; // 초록
    else if (hp > 30) color = 0x1e; // 노랑
    else              color = 0x1c; // 빨강

    printColor(color, 1, 28, "HP:" + bar);
}

bool HpSystem::isGameOver()
{
    return hp <= 0;  // HP가 0이되면 게임오버
}

int HpSystem::getHp()
{
    return hp;  // hp 외부로 보냄
}

//  4. ScoreManager
ScoreManager::ScoreManager()
{
    highScore = 0;
    scorePath = "";
}

void ScoreManager::setPath(string dataPath)
{
    scorePath = dataPath.substr(0, dataPath.rfind(".")) + "_score.txt";  // 마지막 (.)의 위치를 찾아 .txt를 떼고 _score.txt를 붙임 "songs\\example.txt" → "songs\\example_score.txt"
}

int ScoreManager::load()
{
    ifstream file(scorePath); // 파일 읽기 모드로 열기
    if (!file.good())  // 파일 없으면 (첫 플레이)
    {
        highScore = 0;
        return 0;
    }
    file >> highScore; // 파일 안의 값을 highscore 안에 넣기
    file.close();  // 파일 닫기
    return highScore;
}

void ScoreManager::save(int score)
{
    ofstream file(scorePath);  // 파일에 데이터 기록
    file << score;  // score를 파일에 저장
    file.close();  // 파일 닫기
}

bool ScoreManager::isNewRecord(int score)
{
    return score > highScore; // 점수를 기존 최고점수와 비교
}

int ScoreManager::getHighScore()
{
    return highScore;  // 최고 점수를 외부로 전달
}
