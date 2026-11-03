
#include <iostream>
#include <vector>
#include <fstream>  // 파일 입출력 스트림
#include <conio.h>  // 메뉴에서 키 입력 받기 (_getch())
#include <windows.h>  // Windows API 제공 (HANDLE, SetConsoleCursorInfo 등)
#include <string>

#include "Game.h"  // Game 클래스 사용 위함
#include "Utils.h"  // Utils 함수 사용하기 위함
#include "dirent.h" // 디렉터리 탐색 
using namespace std;

void intro() // 타이틀 연출
{
    SetConsoleTitle("Tap Tap Revolution");
    setWindowSize(40, 20);  // 콘솔 창 40열 20행으로 조정
    system("Color 1f");
    printColor(0x1f, 0, 4, "     Welcome to     \n");
    for (int i = 4; i >= 0; i--)
    {
        printColor(0x1f, i, 4, "~");  // 왼쪽에서 가운데로 ~ 표시
        printColor(0x1f, 19 - i, 4, "~");  // 오른쪽에서 가운데로 ~ 표시
        Sleep(500);  // 50ms 간격으로 실행
    }

    system("cls");
    setWindowSize(80, 6); // 콘솔 창 80열 6행으로 조정
    system("Color fc");
    std::cout << "  ______           ______            ___                __     __  _         \n";
    std::cout << " /_  __/__ ____   /_  __/__ ____    / _ \\___ _  _____  / /_ __/ /_(_)__  ___ \n";
    std::cout << "  / /  / _ `/ _ \\  / / / _ `/ _ \\  / , _/ -_) |/ / _ \\/ / // / __/ / _ \\/ _ \\\n";
    std::cout << " /_/   \\_,_/ .__/ /_/  \\_,_/ .__/ /_/|_|\\__/|___/\\___/_/\\_,_/\\__/_/\\___/_//_/\n";
    std::cout << "          /_/             /_/                                                \n";
    for (int i = 0; i < 10; i++)
    {
        Sleep(90);
        system("Color f2"); //초록색
        Sleep(90);
        system("Color fd"); //밝은 마젠타색 90ms 간격으로 글자색이 교차하며 깜빡임
    }
}

void displayMenu()
{
    system("cls");
    system("Color 1c");
    setWindowSize(45, 10); //45열 10행으로 조정 제목과 항목을 넣기 위해 세로길이를 늘림
    cout << " ___       ___        __                       \n";
    cout << "  | _  _    | _  _   |__)_   _ |   |_. _  _    \n";
    cout << "  |(_||_)   |(_||_)  | \\(-\\/(_)||_||_|(_)| ) \n";
    cout << "      |         |                              \n";
    printCenter(0x1b, 45, "Main Menu");    // Main Menu 초록색
    printCenter(0x1d, 45, "1. Choose a song");  // 1. Choose a song   보라색
    printCenter(0x1d, 45, "2. Credits");  // 2. Credits   보라색
    printCenter(0x1d, 45, "0. Exit");  // 0. Exit  보라색
}

void songsMenu()
{
    system("Color 2f");
    printCenter(0x2f, 45, "List of Songs");

    vector<string> files = getSongList(); // 곡 파일들의 경로를 vector로 반환
    for (int i = 0; i < files.size(); i++)
    {
        ifstream musicData(files.at(i));  // 파일 열기
        if (musicData.good())  // 파일이 정상 상태
        {
            string songName;
            getline(musicData, songName);  // 곡 파일의 첫 줄 = 곡 이름

            // 곡별 하이스코어 불러와서 [BEST:점수] 형태로 표시
            ScoreManager sm;
            sm.setPath(files.at(i));
            int hs = sm.load();
            string hsStr = (hs > 0) ? "  [BEST:" + to_string(hs) + "]" : "";
            string musicList = "Song " + to_string(i + 1) + ": " + songName + hsStr;
            printCenter(0x2e, 45, musicList);  // ex) Song 1: Bad Apple [BEST:555]
        }
        else  // 파일을 열지 못한 경우
        {
            printError("Could not open data file: " + files.at(i));  // 에러 메시지 출력
        }
        musicData.close();
    }
    printCenter(0x2f, 45, "0: Back");

    int songChoice = 0;
    char menu;
    printColor(0x2f, 3, songChoice + 1, "->"); // 첫 곡 위치 (y=1)에 화살표
    do
    {
        menu = _getch();

        printColor(0x2f, 3, songChoice + 1, "  ");  //기존 화살표 지우기
        if (menu == 's')  // s키 입력받으면
        {
            if (songChoice < files.size()) songChoice++;  // 아래로 이동
            else songChoice = 0;  // 맨 아래에서 s누르면 맨 위로 이동
        }
        else if (menu == 'w') // w키 입력받으면
        {
            if (songChoice > 0) songChoice--;  // 위로 이동
            else songChoice = files.size();  // 맨 위에서 w누르면 맨 아래로 이동
        }
        printColor(0x2f, 3, songChoice + 1, "->"); // 새로운 위치에 화살표 그리기
    } while (menu != '\r' && menu != '0' && menu - '1' >= files.size()); // 엔터키가 안눌리고 뒤로가기 입력을 안 받고 유효한 곡 번호가 입력되는 동안 루프 

    if (menu >= '0' && menu <= '9')
    {
        songChoice = menu - '1'; // 0을 누르면 함수 종료
    }

    if (songChoice >= 0 && songChoice < files.size())  // 유효한 곡이 선택되면 
    {
        Game game;  // Game 객체 생성
        game.parse(files.at(songChoice));  //곡 데이터 로드
        game.play();  // 게임 실행
    }
}

int main()
{

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    //콘솔 화면에 대한 핸들 얻기 HANDLE은 Windows에서 시스템 자원(파일, 콘솔 등)을 가리키는 식별자이고 이 핸들이 있어야 콘솔을 제어하는 함수들을 호출할 수 있음.
    CONSOLE_CURSOR_INFO cursorInfo;  //커서 정보 담는 구조체
    cursorInfo.bVisible = FALSE; // 직접 FALSE 설정
    cursorInfo.dwSize = 1; // 커서 크기는 최소로
    SetConsoleCursorInfo(hConsole, &cursorInfo); //커서 설정(cursorInfo)을 콘솔(hConsole)에 실제로 적용 구조체의 주소를 넘겨서 적용

    system("chcp 437"); // 코드페이지 437은 원조 IBM PC 문자 집합으로, 게임 화면을 그리는 데 쓰는 박스 그리기 문자들이 들어 있음

    intro(); // 인트로 애니메이션 실행

    char menu; 
    do
    {
        displayMenu(); // 메인메뉴 화면 그리기
        menu = _getch(); // 키 하나 입력 대기
        system("cls"); // 화면 지우기

        if (menu == '1')  // 1입력
        {
            songsMenu(); //곡 선택 메뉴 진입
        }
        else if (menu == '2') // 2 입력
        { 
            system("Color 3a");  // 색상 변경
            cout << endl; // 한 줄 내림
            printCenter(0x3d, 45, "By [LenKagamine]"); // 제작자
            printCenter(0x3a, 45, "For TEJ Summative"); // 제작 목적
            _getch(); // 키 입력 대기
        }
    } while (menu != '0');  // 0을 누르면 루프 빠져나감
    return 0;
}