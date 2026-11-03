#include <iostream>
#include <vector>
#include <windows.h>
#include "dirent.h"

#include "Utils.h"
#include <clocale>  // setlocale: 디렉터리 탐색 동안만 한글 경로 인식

void getWindowSize(int& width, int& height)  // 세로와 가로 너비 알아내는 함수
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle( STD_OUTPUT_HANDLE ), &csbi)) 
    {
        width  = csbi.srWindow.Right  -csbi.srWindow.Left +1;  // 오른쪽 끝 좌표 - 왼쪽 끝 좌표 + 1 = 전체 가로 너비 (좌표가 0부터 시작하기 때문에 +1)
        height = csbi.srWindow.Bottom -csbi.srWindow.Top  +1;  // 바닥 좌표 - 꼭대기 좌표 + 1 = 전체 세로 너비
    }
    else width = height = 0; 
}

void setWindowSize(int width, int height)  // 콘솔 창의 크기를 사용자가 원하는대로 변경하는 함수
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle( STD_OUTPUT_HANDLE ), &csbi))
    {
        // Make sure the new size isn't too big
        if(height > csbi.dwSize.Y) height = csbi.dwSize.Y;  // 사용자가 지정한 높이가 현재 콘솔의 최대 버퍼 높이보다 크면 강제로 최대치로 제한
        if(width  > csbi.dwSize.X) width  = csbi.dwSize.X;  // 사용자가 지정한 너비가 현재 콘솔의 최대 버퍼 너비보다 크면 강제로 최대치로 제한

        // Adjust window origin if necessary
        if((csbi.srWindow.Top  +height) > csbi.dwSize.Y) csbi.srWindow.Top  = csbi.dwSize.Y -height -1;  // 새로운 크기만큼 확장했을 때 화면 하단을 넘어가면 Top 좌표를 위로 올려서 화면 안으로 들어오게 함
        if((csbi.srWindow.Left +width)  > csbi.dwSize.Y) csbi.srWindow.Left = csbi.dwSize.X -width  -1; 

        // Calculate new size
        csbi.srWindow.Bottom = csbi.srWindow.Top  +height -1;  // Bottom의 새로운 좌표 정함
        csbi.srWindow.Right  = csbi.srWindow.Left +width  -1;  // Right의 새로운 좌표 정함

        SetConsoleWindowInfo(GetStdHandle(STD_OUTPUT_HANDLE), true, &csbi.srWindow);  //  srWindow를 실제로 적용시켜 콘솔 창의 크기를 바꿈
    }
}

void setCursor(int x, int y)
{
    COORD coord; 
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle( STD_OUTPUT_HANDLE ), coord);  // 콘솔의 커서를 내가 정한 coord 좌표로 옮겨라
}

void setColor(int color)
{
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);  // 글자의 색상이나 배경색 변경
}

void printColor(int color, int x, int y, std::string text)
{
    setCursor(x, y);  // 글자를 쓰고 싶은 좌표로 커서 이동
    setColor(color);  // 원하는 색상으로 콘솔 팔레트 변경
    std::cout << text;
    setColor(7); // 출력 후 색상을 다시 기본값으로 변경
}

void printCenter(int color, int width, std::string text)
{
    int len = text.length();  // 출력할 문자열의 길이
    CONSOLE_SCREEN_BUFFER_INFO SBInfo;  // 현재 콘솔 화면의 전체 너비 정보를 가져오기 위함
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &SBInfo);  
    printColor(color, (width-len)/2, SBInfo.dwCursorPosition.Y, text+"\n");  // (화면 너비 - 문자열 길이) / 2 = x좌표
}

void printError(std::string text)
{
    int width = 0, height = 0;
    getWindowSize(width, height);  

    text = "ERROR: " + text;
    int y = text.length() / width + 1;  //  (문자열 길이 / 가로 너비) 로 메시지가 총 몇 줄 차지할지 결정
    for(int i=y; i>=1; i--)
    {
        printColor(0x0f, 0, height-i, text.substr((y-i)*width, width)); // height - i 로 화면 맨 아래부터 메시지가 채워지도록 함
    }
}

std::vector<std::string> getSongList()
{
    setlocale(LC_CTYPE, "");  // 한글 경로도 opendir가 인식하도록 (탐색 동안만)
    std::vector<std::string> songList = searchDir(currentDir() + "\\songs", "txt");  // songs 폴더 내부에서 .txt로 끝나는 파일을 찾아 songList에 담음
    setlocale(LC_CTYPE, "C"); // 콘솔 박스 문자(CP437)가 깨지지 않게 로케일 원복
    for(std::string &song: songList)  
    {
        song = "songs\\" + song; // 찾은 파일 목록을 하나씩 꺼내서 song이라는 변수에 담음
    }
    return songList;
}

std::string currentDir()
{
    char buffer[MAX_PATH];
    GetModuleFileName(NULL, buffer, MAX_PATH); // 지금 실행중인 프로그램 파일의 전체 경로를 알려줌
    std::string::size_type pos = std::string(buffer).find_last_of("\\"); // 마지막 역슬래시가 위치한 인덱스 찾기
    return std::string(buffer).substr(0, pos);  // 마지막 파일 이름을 떼어내고 폴더 경로만 추출하여 반환
}

std::string getMusicFile(std::string fullPath)
{
    std::string music = fullPath.substr(0, fullPath.rfind('.')) + ".mp3";   // 문자열의 뒤에서부터 첫 번재로 나타나는 (.)의 위치를 찾음
    return music;
}

std::vector<std::string> searchDir(std::string rootDir, std::string extension)
{
    std::vector<std::string> files;  
    DIR *dir;
    struct dirent *ent;
    if((dir = opendir(rootDir.c_str())) != NULL)  // 해당 경로의 폴더를 열음
    {
        /* print all the files and directories within directory */
        while((ent = readdir(dir)) != NULL)  // 폴더 안에 읽을 파일이 있는 동안 반복
        {
            std::string fileName = ent->d_name;
            std::string extName = fileName.substr(fileName.rfind('.') + 1);  // 파일 이름에서 마지막 (.)을 찾고 그 바로 뒤부터 끝까지의 문자열 잘라냄
            if(extName == extension) files.push_back(ent->d_name);  // 잘라낸 확장자가 찾는 확장자와 일치하는지 확인
        }
        closedir(dir);  // 폴더 닫기
    }
    else
    {
        printError("Could not search for songs");  // 폴더 탐색 실패시 에러 메시지 출력
    }
    return files;
}

