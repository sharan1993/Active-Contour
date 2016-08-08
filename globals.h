
#define SQR(x) ((x)*(x))	/* macro for square */
#ifndef M_PI			/* in case M_PI not found in math.h */
#define M_PI 3.1415927
#endif
#ifndef M_E
#define M_E 2.718282
#endif

#define MAX_FILENAME_CHARS	320

char	filename[MAX_FILENAME_CHARS];

HWND	MainWnd;

		// Display flags
int		ShowPixelCoords;

		// Image data
unsigned char	*OriginalImage;
int				ROWS,COLS;

#define TIMER_SECOND	1			/* ID of timer used for animation */

		// Drawing flags
int		TimerRow,TimerCol;
int		ThreadRow,ThreadCol;
int		ThreadRunning;

		// Function prototypes
LRESULT CALLBACK WndProc(HWND,UINT,WPARAM,LPARAM);
void PaintImage(unsigned char *);
void AnimationThread(void *);		/* passes address of window */

void WindResize(HWND hWnd, int Width, int Height)
{
	RECT Client, Wind;
	POINT change;
	GetClientRect(hWnd, &Client);
	GetWindowRect(hWnd, &Wind);
	change.x = (Wind.right - Wind.left) - Client.right;
	change.y = (Wind.bottom - Wind.top) - Client.bottom;
	MoveWindow(hWnd, Wind.left, Wind.top, Width + change.x, Height + change.y, TRUE);
}

//contour gloabls

int contour_on=0;
int count = 0;
int *RBMPoints, *BalloonPoints;
struct points
{
	int row ; 
	int col ;
};
struct points *active,*bunch,*new_set,*all;
int bunch_end;
int RBMLabel = 1; int BalloonLabel = 1;
float *SobelImage;
void Sobel();
void ExternalEnergy(float**);
void InternalEnergyRBM();
void Create7x7Window();
void AssignContour();
void Create7x7Window();
void RBMModel();
void BalloonModel();
void CalTotalE();

unsigned char *OutImage, *red, *blue, *green, *H; unsigned char *intensity;
int value = 0;
float *Cx, *Cy;
int completed = 0;
int selected = 0; int move_point; int RBMFlag = 0; int BalloonFlag = 0;
struct points *final; int finalcount = 0; int PNFlag = 0;

