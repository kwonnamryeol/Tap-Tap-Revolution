#include <iostream>
#include <windows.h>
#include <fstream>  // 데이터 파일 읽기
#include <vector> 
#include <ctime>
#include <conio.h>  // 키 입력 받기 (_getch)
#include <string>
#include "dirent.h"
#pragma comment(lib, "winmm.lib") 
// 컴파일러(MSVC)에게 "winmm.lib 라이브러리를 함께 링크하라"고 지시하는 명령    mciSendString 

#include "Game.h" 
#include "Utils.h"

#define K1 'd'  // 1번열 = d키
#define K2 'f'  // 2번열 = f키
#define K3 ' '  // 3번열 = 스페이스키
#define K4 'j'  // 4번열 = j키
#define K5 'k'  // 5번열 = k키

using namespace std;

Game::Game() //생성자 객체가 "생성될 때" 자동 호출
{
}

Game::~Game()  // 소멸자 객체가 "사라질 때" 자동 호출 
{
}

void Game::parse(string dataPath)  // Game 클래스의 parse함수 매개변수로 곡 데이터 파일 경로를 받음
{
    music = getMusicFile(dataPath); // 데이터 파일 경로에서 음악 파일 만들어냄

    ifstream musicData(dataPath); 
    if(!musicData.good())  // 파일을 열지 못했다면
    {
        printError("Could not open data file: " + dataPath);  // 에러 메시지 출력
        return;
    }

    string songName;  // 곡 이름
    float startDelay; // 첫 노트까지의 대기 시간
    int bpm;  // 분당 박자 수
    getline(musicData, songName);  // 공백포함 한 줄 전체 읽어서 songName에 넣기
    musicData >> startDelay >> bpm >> numNotes;  // 공백, 줄바꿈으로 구분된 값 하나씩 읽기

    notes.resize(numNotes);  // 노트 키
    noteRow.resize(numNotes); // 노트 열 번호
    noteLength.resize(numNotes); // 노트 개별시간
    time.resize(numNotes);  // 노트 누적시간
    pressed.resize(numNotes, false);  // 처리 여부 (모두 false 상태로 시작)
    lastY.assign(numNotes, -1); // 마지막 y좌표를 -1로 (화면에 안 그려짐)

    for(int i=0, temp; i<numNotes; i++) 
    {
        musicData >> temp >> noteRow.at(i) >> noteLength.at(i);
        if(noteRow.at(i) == 1) notes.at(i) = K1;        // 1번열이면 d
        else if(noteRow.at(i) == 2) notes.at(i) = K2;   // 2번열이면 f
        else if(noteRow.at(i) == 3) notes.at(i) = K3;   // 3번열이면 space
        else if(noteRow.at(i) == 4) notes.at(i) = K4;   // 4번열이면 j
        else if(noteRow.at(i) == 5) notes.at(i) = K5;   // 5번열이면 k
    }
    musicData.close();   // 파일 닫기

    time.at(0) = startDelay; // 첫 노트는 인트로 대기시간
    for(int i=1; i<numNotes; i++)  
    {
        time.at(i) = time.at(i-1) + noteLength.at(i-1);  // 각 노트의 절대시각 = 이전 노트의 절대 시각 + 이전 노트의 간격  (누적합 방식)
    }

    string musicOpen = "open " + music + " alias music";  // 음악 파일 열고 "music" 별명 붙이기 
    mciSendString(musicOpen.c_str(), 0, 0, 0 );  //   음악 열기  인자 (0, 0, 0) : (반환값 버퍼 = 없음, 버퍼크기 = 0, 콜백 창 = 없음)
    // hit.wav는 PlaySound 비동기 재생을 위해 메모리에 로드
    {
        std::ifstream wavFile("sfx\\hit.wav", std::ios::binary | std::ios::ate);  // binary : 파일을 이진모드로 열고  ate : 파일 열자마자 읽기 위치를 파일 끝에 둠
        if (wavFile.good())
        {
            std::streamsize wavSize = wavFile.tellg();  // telling() : 현재 위치 알려줌, ate로 읽기 위치가 파일 끝에 있으므로 현재 위치 = 파일 전체 크기
            wavFile.seekg(0, std::ios::beg);  // 읽기 위치를 파일 처음으로 옮기기
            hitWavData.resize((size_t)wavSize);  // 벡터를 파일 크기만큼 늘림
            wavFile.read(reinterpret_cast<char*>(hitWavData.data()), wavSize);  // 파일 전체를 벡터의 메모리로 한 번에 읽기
        }
        wavFile.close();
    }
    mciSendString("open sfx\\miss.mp3 alias miss", 0, 0, 0);  // MISS 사운드 열기

    scoreMgr.setPath(dataPath); // 하이스코어 경로 설정
    scoreMgr.load();  // 기존 최고 점수 로드
}

void Game::drawGrid()  // 게임 화면 격자 그리는 함수
{
    system("cls"); // 콘솔 비우기
    system("Color 17");
    setWindowSize(28, 32);  //변경: 원래 30->32 (HP바 공간 확보)

    printColor(0x17, 1, 0,
               "\x00c9\x00cd\x00cd\x00cd\x00cd\x00d1\x00cd\x00cd\x00cd\x00cd\x00d1\x00cd\x00cd"     
               "\x00cd\x00cd\x00d1\x00cd\x00cd\x00cd\x00cd\x00d1\x00cd\x00cd\x00cd\x00cd\x00bb");   // 상단 테두리
    printColor(0x17, 1, 25,
               "\x00c8\x00cd\x00cd\x00cd\x00cd\x00cf\x00cd\x00cd\x00cd\x00cd\x00cf\x00cd\x00cd"
               "\x00cd\x00cd\x00cf\x00cd\x00cd\x00cd\x00cd\x00cf\x00cd\x00cd\x00cd\x00cd\x00bc");  // 하단 테두리
    for(int i=0; i<20; i++)
    {
        for(int j=1; j<25; j++)  // 세로 칸막이
        {
            if(i==0 || i==5) printColor(0x17, i*5+1, j, "\x00ba");  // 왼쪽 바깥, 오른쪽 바깥은 이중선 ||
            else printColor(0x17, i*5+1, j, "\x00b3");  // 1,2열 사이, 2,3열 사이 처럼 열 사이 구분선은 단일선 |
        }
    }
    drawJudgmentBar();  // 바닥 5색 판정선을 grid 초기화 시 한 번만 그리기
}

void Game::drawJudgmentBar() // 바닥 판정선 전체 그리기 (5개 열 모두)
{
    for (int r = 1; r <= 5; r++)
    {
        drawJudgmentBarColumn(r);  // 게임 플레이 중 노트가 특정 열의 판정선을 침범했을 때, 그 한 열만 다시 그림
    }
}

// 특정 행(1~5)의 판정선만 다시 그리기 (노트가 침범했을 때 호출)
void Game::drawJudgmentBarColumn(int row)
{
    int color = 0x07;
    if (row == 1 || row == 5) color = 0x1f;
    else if (row == 2 || row == 4) color = 0x17;
    else if (row == 3) color = 0x18;

    int x = row * 5 - 2; // drawNote와 동일한 x 좌표 계산식
    printColor(color, x,     22, "\x00b0\x00b0");
    printColor(color, x - 1, 23, "\x00b0\x00b0\x00b0\x00b0");
    printColor(color, x,     24, "\x00b0\x00b0");
}

char Game::getInput()
{
    if(_kbhit())  // 키가 눌렸는지 확인  ( _getch() 단독으로 쓰면 문제: _getch()는 키가 눌릴 때까지 프로그램을 멈추고 기다림)
    {
        return _getch(); // 눌렸으면 입력받은 키 반환
    }
    return '\0';   // 안 눌렸으면 아무것도 없음 반환
}

void Game::drawNote(int x, int y, int row)
{

    // ✅ 변경 후: 노란색(0x1E) 기본, 판정선 근처는 흰색(0x1F)
    int color = 0x1E; // 밝은 노란색

    if (y == 23)  // 노트가 판정선에 도달했을 때
    {
        printColor(0x1F, x, y - 1, "\x00db\x00db");       // 흰색 (판정선)
        printColor(0x1F, x - 1, y, "\x00db\x00db\x00db\x00db");
        printColor(0x1F, x, y + 1, "\x00db\x00db");
    }
    else  // 노트가 떨어지는 중일때
    {
        printColor(color, x, y - 1, "\x00b1\x00b1");      // 노란색
        printColor(color, x - 1, y, "\x00b1\x00b1\x00b1\x00b1");
        printColor(color, x, y + 1, "\x00b1\x00b1");
    }
}

void Game::clearNote(int x, int y)  // 배경색으로 덮어써서 지우기
{
    printColor(0x17, x, y - 3, "  ");
    printColor(0x17, x - 1, y - 2, "    ");
    printColor(0x17, x - 1, y - 1, "    ");
    printColor(0x17, x - 1, y, "    ");
    printColor(0x17, x - 1, y + 1, "    "); 
    printColor(0x17, x, y + 2, "  "); // y-3 부터 y1=2까지 총 6줄 지우기 노트가 아래로 떨어지면서 생기는 잔상까지 지움
}


void Game::play()
{
    drawGrid();  // 격자, 테두리, 판정선 그리기

    char input = '\0';  // 키 입력
    int score = 0;  // 점수
    int nextNote = 0, nextNoteTemp = 0;  // 노트 검사 시작 위치
    int lastDisplayedScore = -1; // 점수가 바뀔 때에만 화면에 다시 그리기 위한 캐시  -1은 실제로 나올 수 없는 점수라서 첫 프레임에 무조건 점수가 한 번 그려지도록 하는 초기값

    // 추가: 정밀 타이머 활성화 (Sleep과 timeGetTime의 정밀도를 1ms로 향상)
    timeBeginPeriod(1);

    mciSendString("play music", 0, 0, 0);  // 음악 재생 시작
    DWORD startTime = timeGetTime();  // 게임 시작 시각 기록
    DWORD lastFrame = timeGetTime(); // 마지막 프레임 시각 기록

    hpSys.draw(); // 게임시작 시 HP바 초기 출력

    for (int i = 0; i < numNotes; i++) lastY.at(i) = -1;  // 시작 시 모든 노트를 미표시 상태로 초기화

    while (true)
    {
        DWORD now = timeGetTime(); 

        if (pauseSys.getIsPaused()) // 일시정지 중이면 ESC 입력만 감지하고 나머지는 스킵
        {
            if (_kbhit() && _getch() == 27) // ESC 키가 눌렸다면 (ASCII 코드로 ESC == 27)
            {
                pauseSys.resume(); // 일시정지 해제
                drawGrid();  // 화면 다시 그리기
                hpSys.draw();  // HP 바 다시 그리기
            
                for (int i = 0; i < numNotes; i++) lastY.at(i) = -1;   // 화면이 지워졌으므로 모든 노트와 점수 강제 재표시
                lastDisplayedScore = -1; // 점수 강제 재표시
                lastFrame = timeGetTime();  // 프레임 타이머 리셋
           }
            Sleep(1);  // 일시정지 중 CPU를 점유하지 않도록 잠깐 쉼
            continue;  // 루프의 나머지를 건너뛰고 루프 처음으로 돌아감
        }

        if (now - lastFrame < 16) // 마지막 프레임 후 16ms 안 지났으면
        {
            Sleep(1); // 1ms 쉬고
            continue; // 이번 반복 건너뜀
        }
        lastFrame = now; // 16ms가 지났으면 지금을 새 프레임 시각으로 기록

        DWORD currentTime = now - startTime-pauseSys.getTotalPausedTime(); // totalPausedTime을 빼서 일시정지 시간 보정

        if (currentTime >= (DWORD)(time.back() + 2000)) break;  // 곡이 끝난는지 확인하기 위해 마지막 노트 시각 + 2초가 넘으면 break

        if (hpSys.isGameOver()) break;  // HP가 0이되면 break

        input = getInput();  // 키 입력받기

        if (input == 27)  // ESC 감지
        {
            pauseSys.pause(); // 일시정지
            input = '\0';  // 입력 비우기
            continue;  // 나머지 처리 건너뛰고 루프 처음으로
        }

        nextNoteTemp = nextNote;  // 다음 프레임에 쓸 위치 임시 저장

        for (int i = nextNote; i < min((int)notes.size(), nextNote + 50); i++)  // i = nextNote 부터 nextNote + 50까지 검사
        {
            if (pressed.at(i)) continue;  // 이미 처리된 노트

            if ((double)currentTime + 1000.0 < (double)time.at(i)) break;  // 1초 이상 먼 미래 노트면 break

            if (currentTime <= (DWORD)(time.at(i) + range))  // 아직 미스 아님
            {
                if (currentTime >= (DWORD)(time.at(i) - range))  // 칠 수 있는 범위
                {
                    if (input && notes.at(i) == input) // 키가 맞으면
                    {
                        if (!hitWavData.empty()) 
                        {
                            PlaySound(reinterpret_cast<LPCSTR>(hitWavData.data()),  //히트 사운드 재생
                                      NULL,
                                      SND_MEMORY | SND_ASYNC | SND_NODEFAULT);
                        }

                        Judgment j = judgeSys.judge(currentTime, time.at(i));  // judge를 호출해서 PERFECT인지 GOOD인지 판단
                        if (j == PERFECT)
                        {
                            score += 2;    //PERFECT: +2점
                            hpSys.heal(2); //PERFECT: HP 2 회복
                        }
                        else
                        {
                            score += 1; //GOOD: +1점
                        }
                        judgeSys.display(j, noteRow.at(i)); //판정 텍스트 출력 (해당 열 위치에)

                        pressed.at(i) = true;  // 노트를 처리 완료로 표시
                        nextNoteTemp = max(0, i - 5);  // 다음 프레임에 노트 검사를 현재 노트 i보다 5칸 앞에서 시작
                        input = '\0';  // 입력 비우기

                        // ✅ 변경: 마지막 그려진 위치에서만 노트 제거 (있을 때만)
                        if (lastY.at(i) >= 0)
                        {
                            int cx = noteRow.at(i) * 5 - 2;
                            int oldY = lastY.at(i);
                            clearNote(cx, oldY);
                            // ✅ 판정선 영역(y=20~23)을 침범했으면 해당 컬럼 복구
                            if (oldY >= 20) drawJudgmentBarColumn(noteRow.at(i));
                            lastY.at(i) = -1; // 화면에서 사라짐 표시
                        }
                    }
                }
                if (currentTime > (DWORD)(time.at(i) - 1000))  // 칠 타이밍 1초 전부터 화면에 나타나 떨어짐
                {
                    int newY = 23 - max((int)((time.at(i) - (float)currentTime) / 50), 0);  // 50ms마다 1칸씩 내려와서 y=3부터 y=23까지 1초에 걸쳐 떨어짐
                    int x = noteRow.at(i) * 5 - 2;  // 노트의 열 가로 위치

                    if (lastY.at(i) != newY) // 최적화: y좌표가 실제로 바뀌었을 때만 다시 그리기
                    {
                        if (lastY.at(i) >= 0)  //  이전에 그려져 있었으면
                        {
                            int oldY = lastY.at(i);  
                            clearNote(x, oldY); // 이전 위치만 지움
                            if (oldY >= 20) drawJudgmentBarColumn(noteRow.at(i)); // clearNote가 판정선을 지웠다면 복구
                        }
                        drawNote(x, newY, noteRow.at(i));  // 새 위치에 노트 그리기
                        if (newY >= 21) drawJudgmentBarColumn(noteRow.at(i)); // 새로 그린 노트가 판정선과 겹치면, 원래 동작대로 판정선이 위에 오도록 다시 그리기
                        lastY.at(i) = newY;  // lastY = newY로 현재 위치 기억
                    }
                }
            }
            else  // 미스 현재 시각이 노트 판정 시각보다 150ms 이상 지났을 때
            {
                mciSendString("play miss from 0", 0, 0, 0);  // 미스 사운드 재생
                if (score > 0) score--; // 점수 -1

                judgeSys.countMiss();   // 미스 카운트 증가
                judgeSys.display(MISS, noteRow.at(i)); // 해당 열에 MISS 표시
                hpSys.damage(1);  // HP -1

                pressed.at(i) = true;  // 처리 완료 표시
                nextNoteTemp = max(0, i - 5);  // 다음 검사 시작점

                if (lastY.at(i) >= 0) // 마지막 그려진 위치에서만 노트 제거
                {
                    int cx = noteRow.at(i) * 5 - 2;  // 열의 가로 좌표
                    int oldY = lastY.at(i);  // 마지막으로 그려진 세로 좌표
                    clearNote(cx, oldY);  // 그 위치의 노트 지우기
                    if (oldY >= 20) drawJudgmentBarColumn(noteRow.at(i));  // // 판정선 영역 침범 시 그 열의 판정선 복구
                    lastY.at(i) = -1;  // 사라짐 표시
                }
            }
        }

        if (score != lastDisplayedScore)  // 점수 변경 시에만 다시 그리기
        {
            setCursor(13, 27);
            printCenter(0x1e, 28, "Score: " + to_string(score) + "  ");  // 공백 이유  스코어가 100 -> 99가 변할 때 뒤에 공백이 없으면 990처럼 보일 수 있음
            lastDisplayedScore = score;
        }

        nextNote = nextNoteTemp; // 검색 시작점 갱신
    }

    system("cls");  // 화면 지우기
    system("Color 1f");
    cout << endl;
    printCenter(0x17, 28, "Score: " + to_string(score));  // 최종 점수 출력
    Sleep(1000);  // 1초 멈춤

    if (hpSys.isGameOver())  // 게임 오버 판정
    {
        printCenter(0x1c, 28, "GAME OVER");
        Sleep(1000);
    }
    else if(score == numNotes*2) // 전부 PERPECT로 노트 처리하면 만점 (numNotes*2)
    {
        printCenter(0x1d, 28, "FULL COMBO");  // FULL COMBO 출력
        Sleep(1000);
        printCenter(0x1f, 28, "Rank SS"); // RANK SS 출력
    }
    else if(score > numNotes*2*0.9) printCenter(0x1e, 28, "Rank S"); 
    else if(score > numNotes*2*0.8) printCenter(0x1a, 28, "Rank A");
    else if(score > numNotes*2*0.7) printCenter(0x19, 28, "Rank B");
    else if(score > numNotes*2*0.6) printCenter(0x15, 28, "Rank C");
    else if(score > numNotes*2*0.5) printCenter(0x17, 28, "Rank D");
    else if(score > numNotes*2) printCenter(0x1b, 28, "Rank Hacker");  // 논리적으로 출력 불가능함
    else printCenter(0x16, 28, "Failed Song"); 

    Sleep(1000);

    printCenter(0x17, 28, "PERPECT: " + to_string(judgeSys.getPerfectCount()));  // PERFECT 횟수 출력 
    printCenter(0x17, 28, "GOOD:   " + to_string(judgeSys.getGoodCount()));  // GOOD 횟수 출력
    printCenter(0x17, 28, "MISS:   " + to_string(judgeSys.getMissCount()));  // MISS 횟수 출력
    Sleep(500);

    if (scoreMgr.isNewRecord(score))  // scoreMgr 객체를 통해서 현재 점수가 최고 기록인지 확인
    {
        scoreMgr.save(score);  // 신기록이면 파일에 저장
        printCenter(0x1e, 28, "★ NEW RECORD! ★"); // NEW RECORD! 출력
    }
    else
    {
        printCenter(0x17, 28, "BEST: " + to_string(scoreMgr.getHighScore()));  // 신기록이 아니라면 현재까지의 최고기록 출력
    }


    mciSendString("stop music",0,0,0);  // 배경음악 멈추기
    mciSendString("close music",0,0,0);
    PlaySound(NULL, NULL, 0);  // 효과음 정지
    mciSendString("close miss",0,0,0);

    timeEndPeriod(1);  // 정밀 타이머 해제

    while (_kbhit()) _getch(); // 버퍼 비우기 (키 입력 비우기)
    _getch();                  // 아무 키나 누를 때까지 대기
}
