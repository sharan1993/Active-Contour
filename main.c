
#include <stdio.h>
#include<stdlib.h>
#include <math.h>
#include <time.h>
#include <sys/timeb.h>
#include <windows.h>
#include <wingdi.h>
#include <winuser.h>
#include <process.h>	/* needed for multithreading */
#include "resource.h"
#include "globals.h"
#include "Contour.h"
#include "FixedPointContour.h"


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
				LPTSTR lpCmdLine, int nCmdShow)

{
MSG			msg;
HWND		hWnd;
WNDCLASS	wc;

wc.style=CS_HREDRAW | CS_VREDRAW;
wc.lpfnWndProc=(WNDPROC)WndProc;
wc.cbClsExtra=0;
wc.cbWndExtra=0;
wc.hInstance=hInstance;
wc.hIcon=LoadIcon(hInstance,"ID_PLUS_ICON");
wc.hCursor=LoadCursor(NULL,IDC_ARROW);
wc.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);
wc.lpszMenuName="ID_MAIN_MENU";
wc.lpszClassName="PLUS";

if (!RegisterClass(&wc))
  return(FALSE);

hWnd=CreateWindow("PLUS","plus program",
		WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL,
		CW_USEDEFAULT,0,400,400,NULL,NULL,hInstance,NULL);
if (!hWnd)
  return(FALSE);

ShowScrollBar(hWnd,SB_BOTH,FALSE);
ShowWindow(hWnd,nCmdShow);
UpdateWindow(hWnd);
MainWnd=hWnd;

ShowPixelCoords=0;

strcpy(filename,"");
OriginalImage=NULL;
ROWS=COLS=0;

InvalidateRect(hWnd,NULL,TRUE);
UpdateWindow(hWnd);

while (GetMessage(&msg,NULL,0,0))
  {
  TranslateMessage(&msg);
  DispatchMessage(&msg);
  }
return(msg.wParam);
}




LRESULT CALLBACK WndProc (HWND hWnd, UINT uMsg,
		WPARAM wParam, LPARAM lParam)

{
HMENU				hMenu;
OPENFILENAME		ofn;
FILE				*fpt;
HDC					hDC;
char				header[320],text[320];
int					BYTES,xPos,yPos;

switch (uMsg)
  {
  case WM_COMMAND:
    switch (LOWORD(wParam))
      {
	case ID_CONTOUR_ON:
		contour_on = (contour_on + 1) % 2;
		//(contour_used) = (unsigned char*)calloc(ROWS*COLS, sizeof(unsigned char*));
		if (contour_on==1)
		bunch = (struct points*)calloc(ROWS*COLS, sizeof(*active));
		if (contour_on == 0)
		{
			free(final); final = NULL; finalcount = 0; final = (struct points*)calloc(ROWS*COLS, sizeof(*final));
		}
		PaintImage(OriginalImage);
		break;
	  case ID_SHOWPIXELCOORDS:
		ShowPixelCoords=(ShowPixelCoords+1)%2;
		PaintImage(OriginalImage);
		break;
	  case ID_FILE_LOAD:
		if (OriginalImage != NULL)
		  {
		  free(OriginalImage);
		  OriginalImage=NULL;
		  }
		memset(&(ofn),0,sizeof(ofn));
		ofn.lStructSize=sizeof(ofn);
		ofn.lpstrFile=filename;
		filename[0]=0;
		ofn.nMaxFile=MAX_FILENAME_CHARS;
		ofn.Flags=OFN_EXPLORER | OFN_HIDEREADONLY;
		ofn.lpstrFilter = "PPM files\0*.ppm\0PNM files\0*.pnm\0All files\0*.*\0\0";
		if (!( GetOpenFileName(&ofn))  ||  filename[0] == '\0')
		  break;		/* user cancelled load */
		if ((fpt=fopen(filename,"rb")) == NULL)
		  {
		  MessageBox(NULL,"Unable to open file",filename,MB_OK | MB_APPLMODAL);
		  break;
		  }
		fscanf(fpt,"%s %d %d %d",header,&COLS,&ROWS,&BYTES);
		if (!(strcmp(header, "P5") == 0 || strcmp(header, "P6") == 0 ) || BYTES != 255)
		  {
		  MessageBox(NULL,"Not a PPM (P5 greyscale) image",filename,MB_OK | MB_APPLMODAL);
		  fclose(fpt);
		  break;
		  }
		if (strcmp(header, "P5") == 0)
		{
			PNFlag = 0;
			free(OriginalImage); OriginalImage = NULL;
			OriginalImage = (unsigned char *)calloc(ROWS*COLS, 1);
			header[0] = fgetc(fpt);	/* whitespace character after header */
			fread(OriginalImage, 1, ROWS*COLS, fpt);
			fclose(fpt);
			intensity = (unsigned char*)calloc(ROWS*COLS, 1);

		}
		if (strcmp(header, "P6") == 0)
		{
			PNFlag = 1;
			free(OriginalImage); OriginalImage = NULL;
			OriginalImage = (unsigned char *)calloc(ROWS*COLS, 1);
			OutImage = (unsigned char *)calloc(ROWS*COLS, 1);
			red = (unsigned char *)calloc(ROWS*COLS, 1);
			green = (unsigned char *)calloc(ROWS*COLS, 1);
			blue = (unsigned char *)calloc(ROWS*COLS, 1);
			intensity = (unsigned char*)calloc(ROWS*COLS, 1);
			header[0] = fgetc(fpt);	/* whitespace character after header */
			unsigned char *temp;
			temp = (unsigned char*)calloc(ROWS*COLS * 3, 1);
			fread(temp, 1, ROWS*COLS * 3, fpt);
			fclose(fpt); int k = 0;
			for (int i = 0; i < ROWS; i++)
				for (int j = 0; j < COLS; j++)
				{
				OriginalImage[i*COLS + j] = (temp[k] + temp[k + 1] + temp[k + 2]) / 3;
				red[i*COLS + j] = temp[k];
				blue[i*COLS + j] = temp[k + 1];
				green[i*COLS + j] = temp[k + 2];
				intensity[i*COLS + j] = 0;
				OutImage[i*COLS + j] = OriginalImage[i*COLS + j];
				k = k + 3;
				}
		}
		final = (struct points*)calloc(ROWS*COLS, sizeof(*final));
		RBMPoints = (unsigned char*)calloc(ROWS*COLS, sizeof(unsigned char*));
		BalloonPoints = (unsigned char*)calloc(ROWS*COLS, sizeof(unsigned char*));
		for (int i = 0; i < ROWS;i++)
		{
			for (int j = 0; j < COLS; j++)
			{
				RBMPoints[i*COLS + j] = 0;
				BalloonPoints[i*COLS + j] = 0;
			}
		}
		SetWindowText(hWnd,filename);
		WindResize(hWnd, COLS, ROWS);
		PaintImage(OriginalImage);
		Sobel();
		if (PNFlag==1)
			IntensityEdge();
		
		break;

      case ID_FILE_QUIT:
        DestroyWindow(hWnd);
        break;
      }
    break;
  case WM_SIZE:		  /* could be used to detect when window size changes */
	PaintImage(OriginalImage);
	if (contour_on == 1)
	{
		  HDC hDC = GetDC(MainWnd);
			  for (int i = 0; i < finalcount; i++)
			  {
				  for (int r = -2; r <= 2; r++)
				  {
					  for (int c = -2; c <= 2; c++)
					  {
					
							  SetPixel(hDC, final[i].col + c, final[i].row + r, RGB(255, 0, 0));
						  //OutImage[(new_set[i].row + r)*COLS + new_set[i].col + c]=
					  }
				  }
				  //SetPixel(hDC,new_set[i].col, new_set[i].row, RGB(255, 0, 0));
			  }
			  ReleaseDC(MainWnd, hDC);
	}
    return(DefWindowProc(hWnd,uMsg,wParam,lParam));
	break;
  case WM_PAINT:
	PaintImage(OriginalImage);
    return(DefWindowProc(hWnd,uMsg,wParam,lParam));
	break;
  case WM_LBUTTONDOWN:
    return(DefWindowProc(hWnd,uMsg,wParam,lParam));
	break;
  case WM_MOUSEMOVE:
	  if (completed == 1 && contour_on == 1 && (wParam == MK_LBUTTON|| wParam==MK_RBUTTON))
	  {
		  xPos = LOWORD(lParam);
		  yPos = HIWORD(lParam);
		  if (xPos >= 0 && xPos < COLS&&yPos >= 0 && yPos < ROWS){
		  int j;
		  for (int i = 0; i < count; i++)
		  {
			  if (i == count - 1)
				  j = 0;
			  else j = i + 1;
			  if (xPos >= active[i].col - 2 && yPos >= active[i].row - 2 && xPos <= active[i].col + 2 && yPos <= active[i].row + 2)
			  {
				  move_point = i; selected = 1;
				  //MessageBox(NULL, "Sel", filename, MB_OK | MB_APPLMODAL);
				  break;
			  }
			  //else 
			  //{
				//  MessageBox(NULL, "Select Contour Point", filename, MB_OK | MB_APPLMODAL);
				 // break;
			  //}
		  }
		  hDC = GetDC(MainWnd);

		  int prev_row, prev_col; prev_row = active[move_point].row; prev_col = active[move_point].col;
		  active[move_point].row = xPos; active[move_point].row = yPos;
		  sprintf(text, "%d,%d=>%d     ", xPos, yPos, OriginalImage[yPos*COLS + xPos]);
		  TextOut(hDC, 0, 0, text, strlen(text));		/* draw text on the window */
		  if (BalloonFlag == 1)
			  BalloonModel1(move_point);
		  else if (RBMFlag == 1)
			  RBMModel1(move_point);
		  ReleaseDC(MainWnd, hDC);
		 
			  //execute drag contour for xPos and yPos after adding to contour array;
		  }
	  }
	  if (contour_on == 1 && wParam == MK_LBUTTON)
	  {
		  int filter = 0; 
		  xPos = LOWORD(lParam);
		  yPos = HIWORD(lParam);
		  if (xPos >= 0 && xPos < COLS&&yPos >= 0 && yPos < ROWS)
		  {
			  if (filter % 5 == 0)
			  {
				  bunch[value].row = yPos; bunch[value].col = xPos;
				  value++;
			  }
			  filter++;
		  }
		  hDC = GetDC(MainWnd);
		  for (int i = -2; i <=2; i++)
			  for (int j = -2; j <=2; j++)
				  SetPixel(hDC, xPos + i, yPos + j, RGB(255, 0, 0));	/* color the cursor position red */
		  ReleaseDC(MainWnd, hDC);
		  //count = value;
	  }
	if (ShowPixelCoords == 1)
	  {
	  xPos=LOWORD(lParam);
	  yPos=HIWORD(lParam);
	  if (xPos >= 0  &&  xPos < COLS  &&  yPos >= 0  &&  yPos < ROWS)
		{
		sprintf(text,"%d,%d=>%d     ",xPos,yPos,OriginalImage[yPos*COLS+xPos]);
		hDC=GetDC(MainWnd);
		TextOut(hDC,0,0,text,strlen(text));		/* draw text on the window */
		SetPixel(hDC,xPos,yPos,RGB(255,0,0));	/* color the cursor position red */
		ReleaseDC(MainWnd,hDC);
		}
	  }
    return(DefWindowProc(hWnd,uMsg,wParam,lParam));
	break;
  case WM_RBUTTONDOWN:
	  if (contour_on == 1)
	  {
		  int filter = 0; int value = 0;
		  xPos = LOWORD(lParam);
		  yPos = HIWORD(lParam);
		  int r, c;
		  hDC = GetDC(MainWnd);
		  for (int d = -10; d <= 10; d++)
		  {
			  r = yPos + d;
			  c = xPos + sqrt(100 - d*d);
			  filter++;
			  if (filter % 1 == 0){
				  // BalloonPoints[r*COLS + c] = value++;
				  bunch[value].row = r; bunch[value].col = c;
				  value++;
			  }
			  for (int i = -2; i <=2; i++)
				  for (int j = -2; j <= 2; j++)
					  SetPixel(hDC, c + i, r + j, RGB(255, 0, 0));
		  }
		  for (int d = +10; d >=- 10; d--)
		  {
			  r = yPos + d;
			  c = xPos - sqrt(100 - d*d);
			  filter++;
			  if (filter % 1 == 0){
				  bunch[value].row = r; bunch[value].col = c;
				  value++;
			  }
			  for (int i = -2; i <=2; i++)
				  for (int j = -2; j <=2; j++)
					  SetPixel(hDC, c + i, r + j, RGB(255, 0, 0));
		  }
		  count = value;
		  ReleaseDC(MainWnd, hDC);
	  }
	  return(DefWindowProc(hWnd, uMsg, wParam, lParam));
	  break;
  case WM_KEYDOWN:
	  if (wParam == 'r' || wParam == 'R')
	  {
		  count = value; RBMFlag = 1;
		  active = (struct points*)calloc(count, sizeof(*active));
		  for (int i = 0; i < count;i++)
		  {
			  active[i].row = bunch[i].row; active[i].col = bunch[i].col;
		  }
		 // AssignContour(RBMPoints, RBMLabel, count, &active); 
		  free(RBMPoints);
		  RBMPoints = NULL;
		  RBMPoints = (int*)calloc(ROWS*COLS, sizeof(int*));
		//  for (int i = 0; i < 10; i++){
			  RBMModel();
			  completed = 1; int k = 0;
			  for (int i = finalcount; i <finalcount+ count; i++)
			  {
				  final[i] = active[k]; k++;
			  }
			  finalcount += count;
			//  active = new_set;
		  //}
			
		  //RBMLabel++;
			  PaintImage(OutImage);
			  HDC hDC = GetDC(MainWnd);
			  for (int i = 0; i < finalcount; i++)
			  {
				  for (int r = -2; r <= 2; r++)
				  {
					  for (int c = -2; c <= 2; c++)
					  {
					
							  SetPixel(hDC, final[i].col + c, final[i].row + r, RGB(255, 0, 0));
						  //OutImage[(new_set[i].row + r)*COLS + new_set[i].col + c]=
					  }
				  }
				  //SetPixel(hDC,new_set[i].col, new_set[i].row, RGB(255, 0, 0));
			  }
			  ReleaseDC(MainWnd, hDC);
		  
	  }
	  if (wParam == 'b' || wParam == 'B')
	  {
		  active = (struct points*)calloc(count, sizeof(*active)); BalloonFlag = 1;
		  for (int i = 0; i < count; i++)
		  {
			  active[i].row = bunch[i].row; active[i].col = bunch[i].col;
		  }
		  //AssignContour(BalloonPoints, BalloonLabel, count, &active);
		  free(BalloonPoints);
		  BalloonPoints = NULL;
		  BalloonPoints = (int*)calloc(ROWS*COLS, sizeof(int*));
		 // for (int i = 0; i < 30; i++){
			  BalloonModel();
			  completed = 1;
			  int k = 0;
			  for (int i = finalcount; i <finalcount+ count; i++)
			  {
				  final[i] = active[k]; k++;
			  }
			  finalcount += count;
			  PaintImage(OutImage);
			  HDC hDC = GetDC(MainWnd);
			  for (int i = 0; i < finalcount; i++)
			  {
				  for (int r = -2; r <= 2; r++)
				  {
					  for (int c = -2; c <= 2; c++)
					  {
						
							  SetPixel(hDC, final[i].col + c, final[i].row + r, RGB(255, 0, 0));
						  //OutImage[(new_set[i].row + r)*COLS + new_set[i].col + c]=
					  }
				  }
				  //SetPixel(hDC,new_set[i].col, new_set[i].row, RGB(255, 0, 0));
			  }
			  ReleaseDC(MainWnd, hDC);
			  //  active = new_set;
		  //}
		  // _beginthread(BalloonModel, 0, MainWnd);
		//  value = 0;
//		  BalloonLabel++;

	  }
	  if (wParam == 'a' || wParam == 'A'){
		  
		  free(active); active = NULL; BalloonFlag = 0; RBMFlag = 0;
		  free(bunch); bunch = NULL;
		  bunch = (struct points*)calloc(ROWS*COLS, sizeof(bunch));
		  value = 0; completed = 0;
		  PaintImage(OutImage);
		  HDC hDC = GetDC(MainWnd);
		  for (int i = 0; i < finalcount; i++)
		  {
			  for (int r = -2; r <= 2; r++)
			  {
				  for (int c = -2; c <= 2; c++)
				  {
					  if (c == 0 && r == 0)
						  SetPixel(hDC, final[i].col + c, final[i].row + r, RGB(0, 255, 0));
					  else
						  SetPixel(hDC, final[i].col + c, final[i].row + r, RGB(255, 0, 0));
					  //OutImage[(new_set[i].row + r)*COLS + new_set[i].col + c]=
				  }
			  }
			  //SetPixel(hDC,new_set[i].col, new_set[i].row, RGB(255, 0, 0));
		  }
		  ReleaseDC(MainWnd, hDC);
	  }
	  if (wParam == 'i' || wParam == 'I')
		  PaintImage(intensity);
	if (wParam == 's'  ||  wParam == 'S')
	  PostMessage(MainWnd,WM_COMMAND,ID_SHOWPIXELCOORDS,0);	  /* send message to self */
	if ((TCHAR)wParam == '1')
	  {
	  TimerRow=TimerCol=0;
	  SetTimer(MainWnd,TIMER_SECOND,10,NULL);	/* start up 10 ms timer */
	  }
	if ((TCHAR)wParam == '2')
	  {
	  KillTimer(MainWnd,TIMER_SECOND);			/* halt timer, stopping generation of WM_TIME events */
	  PaintImage(OriginalImage);								/* redraw original image, erasing animation */
	  }
	if ((TCHAR)wParam == '3')
	  {
	  ThreadRunning=1;
	  _beginthread(AnimationThread,0,MainWnd);	/* start up a child thread to do other work while this thread continues GUI */
	  }
 	if ((TCHAR)wParam == '4')
	  {
	  ThreadRunning=0;							/* this is used to stop the child thread (see its code below) */
	  }
	return(DefWindowProc(hWnd,uMsg,wParam,lParam));
	break;
  case WM_TIMER:	  /* this event gets triggered every time the timer goes off */
	hDC=GetDC(MainWnd);
	SetPixel(hDC,TimerCol,TimerRow,RGB(0,0,255));	/* color the animation pixel blue */
	ReleaseDC(MainWnd,hDC);
	TimerRow++;
	TimerCol+=2;
	break;
  case WM_HSCROLL:	  /* this event could be used to change what part of the image to draw */
	PaintImage(OriginalImage);	  /* direct PaintImage calls eliminate flicker; the alternative is InvalidateRect(hWnd,NULL,TRUE); UpdateWindow(hWnd); */
    return(DefWindowProc(hWnd,uMsg,wParam,lParam));
	break;
  case WM_VSCROLL:	  /* this event could be used to change what part of the image to draw */
	PaintImage(OriginalImage);
    return(DefWindowProc(hWnd,uMsg,wParam,lParam));
	break;
  case WM_DESTROY:
    PostQuitMessage(0);
	break;
  default:
    return(DefWindowProc(hWnd,uMsg,wParam,lParam));
    break;
  }

hMenu=GetMenu(MainWnd);
if (ShowPixelCoords == 1)
  CheckMenuItem(hMenu,ID_SHOWPIXELCOORDS,MF_CHECKED);	/* you can also call EnableMenuItem() to grey(disable) an option */
else
  CheckMenuItem(hMenu,ID_SHOWPIXELCOORDS,MF_UNCHECKED);
if (contour_on == 1)
CheckMenuItem(hMenu, ID_CONTOUR_ON, MF_CHECKED);
else
CheckMenuItem(hMenu, ID_CONTOUR_ON, MF_UNCHECKED);
DrawMenuBar(hWnd);

return(0L);
}




void PaintImage(unsigned char *input)

{
PAINTSTRUCT			Painter;
HDC					hDC;
BITMAPINFOHEADER	bm_info_header;
BITMAPINFO			*bm_info;
int					i,r,c,DISPLAY_ROWS,DISPLAY_COLS;
unsigned char		*DisplayImage;

if (input == NULL)
  return;		/* no image to draw */

		/* Windows pads to 4-byte boundaries.  We have to round the size up to 4 in each dimension, filling with black. */
DISPLAY_ROWS=ROWS;
DISPLAY_COLS=COLS;
if (DISPLAY_ROWS % 4 != 0)
  DISPLAY_ROWS=(DISPLAY_ROWS/4+1)*4;
if (DISPLAY_COLS % 4 != 0)
  DISPLAY_COLS=(DISPLAY_COLS/4+1)*4;
DisplayImage=(unsigned char *)calloc(DISPLAY_ROWS*DISPLAY_COLS,1);
for (r=0; r<ROWS; r++)
  for (c=0; c<COLS; c++)
	DisplayImage[r*DISPLAY_COLS+c]=input[r*COLS+c];

BeginPaint(MainWnd,&Painter);
hDC=GetDC(MainWnd);
bm_info_header.biSize=sizeof(BITMAPINFOHEADER); 
bm_info_header.biWidth=DISPLAY_COLS;
bm_info_header.biHeight=-DISPLAY_ROWS; 
bm_info_header.biPlanes=1;
bm_info_header.biBitCount=8; 
bm_info_header.biCompression=BI_RGB; 
bm_info_header.biSizeImage=0; 
bm_info_header.biXPelsPerMeter=0; 
bm_info_header.biYPelsPerMeter=0;
bm_info_header.biClrUsed=256;
bm_info_header.biClrImportant=256;
bm_info=(BITMAPINFO *)calloc(1,sizeof(BITMAPINFO) + 256*sizeof(RGBQUAD));
bm_info->bmiHeader=bm_info_header;
for (i=0; i<256; i++)
  {
  bm_info->bmiColors[i].rgbBlue=bm_info->bmiColors[i].rgbGreen=bm_info->bmiColors[i].rgbRed=i;
  bm_info->bmiColors[i].rgbReserved=0;
  } 

SetDIBitsToDevice(hDC,0,0,DISPLAY_COLS,DISPLAY_ROWS,0,0,
			  0, /* first scan line */
			  DISPLAY_ROWS, /* number of scan lines */
			  DisplayImage,bm_info,DIB_RGB_COLORS);
ReleaseDC(MainWnd,hDC);
EndPaint(MainWnd,&Painter);

free(DisplayImage);
free(bm_info);
}




void AnimationThread(HWND AnimationWindowHandle)

{
HDC		hDC;
char	text[300];

ThreadRow=ThreadCol=0;
while (ThreadRunning == 1)
  {
  hDC=GetDC(MainWnd);
  SetPixel(hDC,ThreadCol,ThreadRow,RGB(0,255,0));	/* color the animation pixel green */
  sprintf(text,"%d,%d     ",ThreadRow,ThreadCol);
  TextOut(hDC,300,0,text,strlen(text));		/* draw text on the window */
  ReleaseDC(MainWnd,hDC);
  ThreadRow+=3;
  ThreadCol++;
  Sleep(100);		/* pause 100 ms */
  }
}

