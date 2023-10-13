#include <iostream>
#include <cstdlib>
#include <ctime>
#include <stdio.h>
#include <termios.h>

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#include "colors.h"
#include "Matrix.h"

using namespace std;


/**************************************************************/
/**************** Linux System Functions **********************/
/**************************************************************/

char saved_key = 0;
int tty_raw(int fd);	/* put terminal into a raw mode */
int tty_reset(int fd);	/* restore terminal's mode */
  
/* Read 1 character - echo defines echo mode */
char getch() {    //getch() 함수를 사용하여 사용자로부터 한 문자씩 입력받음.
  char ch;
  int n;
  while (1) {
    tty_raw(0);
    n = read(0, &ch, 1);
    tty_reset(0);
    if (n > 0)
      break;
    else if (n < 0) {
      if (errno == EINTR) {
        if (saved_key != 0) {
          ch = saved_key;
          saved_key = 0;
          break;
        }
      }
    }
  }
  return ch;
}

void sigint_handler(int signo) {
  // cout << "SIGINT received!" << endl;
  // do nothing;
}

void sigalrm_handler(int signo) {
  alarm(1);
  saved_key = 's';
}

void registerInterrupt() {
  struct sigaction act, oact;
  act.sa_handler = sigint_handler;
  sigemptyset(&act.sa_mask);
#ifdef SA_INTERRUPT
  act.sa_flags = SA_INTERRUPT;
#else
  act.sa_flags = 0;
#endif
  if (sigaction(SIGINT, &act, &oact) < 0) {
    cerr << "sigaction error" << endl;
    exit(1);
  }
}

void registerAlarm() {
  struct sigaction act, oact;
  act.sa_handler = sigalrm_handler;
  sigemptyset(&act.sa_mask);
#ifdef SA_INTERRUPT
  act.sa_flags = SA_INTERRUPT;
#else
  act.sa_flags = 0;
#endif
  if (sigaction(SIGALRM, &act, &oact) < 0) {
    cerr << "sigaction error" << endl;
    exit(1);
  }
  alarm(1);
}

/**************************************************************/
/**************** Tetris Blocks Definitions *******************/
/**************************************************************/
#define MAX_BLK_TYPES 7
#define MAX_BLK_DEGREES 4

int T0D0[] = { 1, 1, 1, 1, -1 };
int T0D1[] = { 1, 1, 1, 1, -1 };
int T0D2[] = { 1, 1, 1, 1, -1 };
int T0D3[] = { 1, 1, 1, 1, -1 };

int T1D0[] = { 0, 1, 0, 1, 1, 1, 0, 0, 0, -1 };
int T1D1[] = { 0, 1, 0, 0, 1, 1, 0, 1, 0, -1 };
int T1D2[] = { 0, 0, 0, 1, 1, 1, 0, 1, 0, -1 };
int T1D3[] = { 0, 1, 0, 1, 1, 0, 0, 1, 0, -1 };

int T2D0[] = { 1, 0, 0, 1, 1, 1, 0, 0, 0, -1 };
int T2D1[] = { 0, 1, 1, 0, 1, 0, 0, 1, 0, -1 };
int T2D2[] = { 0, 0, 0, 1, 1, 1, 0, 0, 1, -1 };
int T2D3[] = { 0, 1, 0, 0, 1, 0, 1, 1, 0, -1 };

int T3D0[] = { 0, 0, 1, 1, 1, 1, 0, 0, 0, -1 };
int T3D1[] = { 0, 1, 0, 0, 1, 0, 0, 1, 1, -1 };
int T3D2[] = { 0, 0, 0, 1, 1, 1, 1, 0, 0, -1 };
int T3D3[] = { 1, 1, 0, 0, 1, 0, 0, 1, 0, -1 };

int T4D0[] = { 0, 1, 0, 1, 1, 0, 1, 0, 0, -1 };
int T4D1[] = { 1, 1, 0, 0, 1, 1, 0, 0, 0, -1 };
int T4D2[] = { 0, 1, 0, 1, 1, 0, 1, 0, 0, -1 };
int T4D3[] = { 1, 1, 0, 0, 1, 1, 0, 0, 0, -1 };

int T5D0[] = { 0, 1, 0, 0, 1, 1, 0, 0, 1, -1 };
int T5D1[] = { 0, 0, 0, 0, 1, 1, 1, 1, 0, -1 };
int T5D2[] = { 0, 1, 0, 0, 1, 1, 0, 0, 1, -1 };
int T5D3[] = { 0, 0, 0, 0, 1, 1, 1, 1, 0, -1 };

int T6D0[] = { 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, -1 };
int T6D1[] = { 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, -1 };
int T6D2[] = { 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, -1 };
int T6D3[] = { 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, -1 };
  
int *setOfBlockArrays[] = {
  T0D0, T0D1, T0D2, T0D3,
  T1D0, T1D1, T1D2, T1D3,
  T2D0, T2D1, T2D2, T2D3,
  T3D0, T3D1, T3D2, T3D3,
  T4D0, T4D1, T4D2, T4D3,
  T5D0, T5D1, T5D2, T5D3,
  T6D0, T6D1, T6D2, T6D3,
};

 Matrix *setOfBlockObjects[7][4];

void drawScreen(Matrix *screen, int wall_depth)
{
  int dy = screen->get_dy();
  int dx = screen->get_dx();
  int dw = wall_depth;
  int **array = screen->get_array();

  for (int y = 0; y < dy - dw + 3; y++) {
    for (int x = dw - 3; x < dx - dw + 3; x++) {
      if (array[y][x] == 0)
	      cout << "□ ";
      else if (array[y][x] == 1)
	      cout << "■ ";
      else if (array[y][x] == 10)
	      cout << "◈ ";
      else if (array[y][x] == 20)
	      cout << "★ ";
      else if (array[y][x] == 30)
	      cout << "● ";
      else if (array[y][x] == 40)
	      cout << "◆ ";
      else if (array[y][x] == 50)
	      cout << "▲ ";
      else if (array[y][x] == 60)
	      cout << "♣ ";
      else if (array[y][x] == 70)
	      cout << "♥ ";
      else
	      cout << "X ";
    }
    cout << endl;
  }
}
  
/**************************************************************/
/******************** Tetris Main Loop ************************/
/**************************************************************/

#define SCREEN_DY  10
#define SCREEN_DX  10
#define SCREEN_DW  3

#define ARRAY_DY (SCREEN_DY + SCREEN_DW)
#define ARRAY_DX (SCREEN_DX + 2*SCREEN_DW)

int arrayScreen[ARRAY_DY][ARRAY_DX] = {
  { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1 },
  { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1 },
  { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1 },
  { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1 },
  { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1 },
  { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1 },
  { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1 },
  { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1 },
  { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1 },
  { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1 },
  { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 ,1 ,1, 1, 1, 1, 1 },
  { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 ,1 ,1, 1, 1, 1, 1 },
  { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 ,1 ,1, 1, 1, 1, 1 },
};

int arrayBlk[3][3] = {
  { 0, 1, 0 },
  { 1, 1, 1 },
  { 0, 0, 0 },
};


int main(int argc, char *argv[]) {
  char key;
  int top = 0, left = 4; 


// for(int t=0; t<7; t++) {
//   for (int d=0; d<4; d++) {
//     int row, col;
//     if (t==0) {
//       row=col=2;
//     }
//     else if (t==6) {
//       row=col=4;
//     }
//     else {
//       row=col=3;
//     }
//     setOfBlockObjects[t][d] = new Matrix(setOfBlockArrays[t*4+d],row, col);
//   }
// }

// srand((unsigned int)time(NULL));
// int randCol = rand() % 7;
// int randRow = rand() % 4;
// Matrix *currBlk = setOfBlockObjects[randCol][randRow];


//블록 이동,회전 구현하는 부분
  Matrix *iScreen = new Matrix((int *) arrayScreen, ARRAY_DY, ARRAY_DX); //iScreen
  Matrix *currBlk = new Matrix((int *) arrayBlk, 3, 3);  //현재 이동하고 있는 블록의 정보 저장하는 arrayBlk으로 currBlk 객체 생성
  Matrix *tempBlk = iScreen->clip(top, left, top + currBlk->get_dy(), left + currBlk->get_dx());  //현재 블록이 이동할 수 있는 범위를 계산->tempBlk 객체는 현재 블록이 이동할 수 있는 공간
  Matrix *tempBlk2 = tempBlk->add(currBlk);  //tempBlk2는 덧셈의 결과물.블록의 모양을 이동시킴.
  

  //*tempBlk2 = *tempBlk + *currBlk;   //heap할당 사용. +연산자를 overloading 

  delete tempBlk;    
                    

  Matrix *oScreen = new Matrix(iScreen); //(현재게임판)iScreen을 복사해서 (새로운게임판)oScreen을 만듦.
  oScreen->paste(tempBlk2, top, left);   //현재 블록(currBlk)을 현재 게임판의(top, left)위치에 추가하기 위해, 현재 게임판(oScreen)의 해당 위치에 잘라낸 블록(tempBlk2)을 붙여넣음.
  delete tempBlk2;
  drawScreen(oScreen, SCREEN_DW);  //(게임판)oScreen 출력.
  delete oScreen;

  int idxBlockDegree = 0;

//사용자로부터 입력받아 블록위치 이동시키는 부분.
  while ((key = getch()) != 'q') { //getch() : 사용자로부터 입력받음. ->key에 저장
    switch (key) {
      case 'a': left--; break;  //a : 왼쪽한칸
      case 'd': left++; break;  //b : 우측한칸
      case 's': top++; break;   //s : 아래한칸
      case 'w':                 //w : 회전
        // idxBlockDegree = (idxBlockDegree + 1) % 4;
        // currBlk = setOfBlockObjects[randCol][idxBlockDegree];
        break;
       case ' ': //while(1) {
      //   					top++;
			// 						tempBlk = iScreen->clip(top, left, top + currBlk->get_dy(), left + currBlk->get_dx());
			// 					  tempBlk2 = tempBlk->add(currBlk);
      //             //delete tempBlk;
      //             if (tempBlk->anyGreaterThan(1)) {
      //               top--;
      //               //delete tempBlk;
      //               break;
	    //               }
      //             }
                  break;
      default: cout << "wrong key input" << endl;
    }


//<충돌검사>
  tempBlk = iScreen->clip(top, left, top + currBlk->get_dy(), left + currBlk->get_dx()); //현재 이동한 블록을 iScreen의 위치에 따라 잘라서 tempBlk 객체에 저장
  tempBlk2 = tempBlk->add(currBlk); //잘린 블록과 현재 블록을 더한 tempBlk2
  delete tempBlk;
  if (tempBlk2->anyGreaterThan(1)){  //tempBlk2 객체에서 anyGreaterThan(1)을 호출하면, 이 객체에 저장된 값 중에서 1보다 큰 값이 있는지 검사
    switch (key) {                    //이동키에 따라 블록을 원래 위치로 되돌리는 작업을 수행
      case 'a': left++; break;
      case 'd': left--; break;
      case 's': top--; break;
      case 'w': //idxBlockDegree = (idxBlockDegree + 3) % 4;
			   	      //currBlk = setOfBlockObjects[randCol][idxBlockDegree];
								//break;
      case ' ': break;
    }
    delete tempBlk;


//블록이동.회전 후, 결과가 게임화면에서 유효한지 확인하는 부분.
    tempBlk = iScreen->clip(top, left, top + currBlk->get_dy(), left + currBlk->get_dx()); //2. 새로 만들어진 블록이 게임 화면을 벗어나는 경우가 있으므로, 이를 방지하기 위해 게임 화면을 벗어나지 않는 부분만을 잘라내는 작업 수행
    tempBlk2 = tempBlk->add(currBlk);                          //1. 현재 블록의 위치와 회전 상태를 반영하여 새로운 위치와 회전 상태의 블록을 임시로 만듦.
  }
  delete tempBlk;

  oScreen = new Matrix(iScreen);     //현재 게임 화면(iScreen) 복사 -> 새로운 게임 화면(oScreen) 생성
  oScreen->paste(tempBlk2, top, left);  //tempBlk2에 있는 현재 블록을 이동한 위치(top, left)에 붙여넣음.
  delete tempBlk2;
  drawScreen(oScreen, SCREEN_DW);    //새로운 게임 화면(oScreen) 출력
  delete oScreen;

  }



  delete iScreen;
  delete currBlk;

  //delete tempBlk;
  //delete tempBlk2;
  //delete oScreen;

  //(main함수, deleteFullLines 함수 안에서 사용한) 모든 Matrix 객체는 
  //적절한 지점에 소멸시켜서 main함수 끝에는 남아있는 Matrix 객체의 수가 0이 되도록 할 조건.
  cout << "(nAlloc, nFree) = (" << Matrix::get_nAlloc() << ',' << Matrix::get_nFree() << ")" << endl;  
  cout << "Program terminated!" << endl;

  return 0;
}