#ifndef UTILS_H
#define UTILS_H

#include <vector> 

void getWindowSize(int& width, int& height);  // 게임창 크기 지정 함수
void setWindowSize(int width, int height);
void setCursor(int x, int y);  // 커서 위치 이동하는 함수
void setColor(int color);  // 글자나 배경에 색상 입히는 함수
void printColor(int color, int x, int y, std::string text); 
void printCenter(int color, int x, std::string text);  //  화면 가로 중앙에 텍스트 배치하는 함수

void printError(std::string text);  // 에러 메시지 출력 함수

std::vector<std::string> getSongList();  // 노래 폴더에서 곡 가져오는 함수
std::string currentDir();  // 현재 프로그램이 실행중인 경로 확인하는 함수
std::string getMusicFile(std::string fullPath);  // 전체 경로에서 실제 파일 이름만 추출하는 함수

std::vector<std::string> searchDir(std::string rootDir, std::string extension);  // 특정 확장자를 가진 함수들만 검색하여 가져오는 함수

#endif 
