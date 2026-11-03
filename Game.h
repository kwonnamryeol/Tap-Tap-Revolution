#ifndef GAME_H 
#define GAME_H  

#include <vector>
#include <windows.h>

#include "Chuga.h"  // 4개 추가기능 클래스 정의

class Game
{
    public:
        Game();  // 생성자
        ~Game(); // 소멸자
        void parse(std::string dataPath);  // 곡 데이터 파일 읽기
        void play();    //Gap of time during which note can be accepted 

        const int range = 150;  // 판정 허용 시간 범위 (150ms)

    private:
        void drawGrid();  // 격자, 테두리, 판정선 그리기
        char getInput();  // 키 입력 받기
        void drawNote(int x, int y, int row); // 노트 1개 그리기
        void clearNote(int x, int y);        // 노트 1개 지우기   
        void drawJudgmentBar();              // 추가: 바닥 판정선 전체 그리기
        void drawJudgmentBarColumn(int row); // 추가: 특정 행의 판정선만 다시 그리기

        int numNotes;                   //Total number of notes
        std::string music;              //Music file filename
        std::vector<char>  notes;       //Type of note
        std::vector<int>   noteRow;     //Row number of note (1-5)
        std::vector<float> noteLength;  //Individual time for each note, for input
        std::vector<float> time;        //Total time for a note
        std::vector<bool>  pressed;     //Pressed/missed or not pressed
        std::vector<int>   lastY;       //마지막으로 그려진 y좌표 (-1이면 화면에 없음)
        std::vector<BYTE>  hitWavData;  //추가: hit.wav 데이터를 메모리에 보관 (PlaySound 비동기 재생용)

        JudgmentSystem judgeSys; // 판정 시스템
        PauseSystem    pauseSys; // 일시정지 시스템
        HpSystem       hpSys;  // HP바 시스템
        ScoreManager   scoreMgr; // 하이스코어 시스템
};

#endif // #ifndef 사용해서
