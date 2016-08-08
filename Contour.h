#include<stdio.h>
#include<stdlib.h>
#include<math.h>
//#include "globals.h"

int sobel1[] = { 1, 2, 1, 0, 0, 0, -1, -2, -1 };
int sobel2[] = { -1, 0, 1, -2, 0, 2, -1, 0, 1 };

void Sobel()
{
	
	SobelImage = (float*)calloc(ROWS*COLS, sizeof(float *));
	Cx = (float *)calloc(ROWS*COLS, sizeof(unsigned char*));
	Cy = (float *)calloc(ROWS*COLS, sizeof(unsigned char*));
	float max, min,max1,max2,min1,min2;min1=min2=max1=max2= max = min = 0; float sum1, sum2, sum;
	float *SobelImage_unnorm;
	SobelImage_unnorm = (float*)calloc(ROWS*COLS, sizeof(float*));
	for (int i = 3; i < ROWS - 3; i++)
	{
		for (int j = 3; j < COLS - 3; j++)
		{
			sum1 = 0; sum2 = 0; sum = 0;
			for (int r = 0; r < 3; r++)
			{
				for (int c = 0; c < 3; c++)
				{
					sum1 += sobel1[r * 3 + c] * OriginalImage[(i + r - 1)*COLS + j + c - 1] / 9;
					sum2 += sobel2[r * 3 + c] * OriginalImage[(i + r - 1)*COLS + j + c - 1] / 9;

				}
			}Cx[i*COLS + j] = sum1; Cy[i*COLS + j] = sum2;
			SobelImage_unnorm[i*COLS + j] = (sum1*sum1 + sum2*sum2);
			if (min1 == 0 || Cx[i*COLS + j] < min1)
				min1 = Cx[i*COLS + j];
			if (max1 < Cx[i*COLS + j])
				max1 = Cx[i*COLS + j];
			if (min2 == 0 || Cy[i*COLS + j] < min2)
				min2 = Cy[i*COLS + j];
			if (max2 < Cy[i*COLS + j])
				max2 = Cy[i*COLS + j];
			if (min == 0 || SobelImage_unnorm[i*COLS + j] < min)
				min = SobelImage_unnorm[i*COLS + j];
			if (SobelImage_unnorm[i*COLS + j] > max)
				max = SobelImage_unnorm[i*COLS + j];
		}
	}
	for (int i = 0; i < ROWS; i++)
	{
		for (int j = 0; j < COLS; j++)
		{
			Cx[i*COLS + j] = (Cx[i*COLS + j] - min1) / (max1 - min1);
			Cy[i*COLS + j] = (Cy[i*COLS + j] - min2) / (max2 - min2);
			SobelImage[i*COLS + j] = (SobelImage_unnorm[i*COLS + j] - min) / (max - min);
		}
	}
	free(SobelImage_unnorm);
	SobelImage_unnorm = NULL;
}

void IntensityEdge()
{
	int thresr = 80; int thresb = 80; int thresg =80;
	for (int r = 1; r < ROWS-1; r++)
	{
		for (int c = 1; c < COLS-1; c++)
		{
			if (abs(red[r*COLS + c] - red[r*COLS + c - 1])>thresr || abs(red[r*COLS + c] - red[r*COLS + c + 1])>thresr || abs(red[r*COLS + c] - red[(r - 1)*COLS + c])>thresr|| abs(red[r*COLS + c] - red[(r + 1)*COLS + c])>thresr)
				intensity[r*COLS + c] = 1;
			else if (abs(blue[r*COLS + c] - blue[r*COLS + c - 1])>thresb || abs(blue[r*COLS + c] - blue[r*COLS + c + 1])>thresb || abs(blue[r*COLS + c] - blue[(r - 1)*COLS + c])>thresb || abs(blue[r*COLS + c] - blue[(r + 1)*COLS + c])>thresb)
				intensity[r*COLS + c] = 1;
			else if (abs(green[r*COLS + c] - green[r*COLS + c - 1])>thresg || abs(green[r*COLS + c] - green[r*COLS + c + 1])>thresg || (green[r*COLS + c] - green[(r - 1)*COLS + c])>thresg || abs(green[r*COLS + c] - green[(r + 1)*COLS + c])>thresg)
				intensity[r*COLS + c] = 1;
		}
	}
}

void ExternalEnergy(float **ExtE)
{
	(*ExtE) = (float*)calloc(ROWS*COLS, sizeof(float*));
	//Using Sobel Gradient Image and Intensity of Image
	float SobelWeight, IntensityWeight,IntEdgeW;
	SobelWeight = -1.5; IntensityWeight = 01.5;
	if (PNFlag==1)
	IntEdgeW = .5;
	else
	{
		IntEdgeW = 0; for (int r = 0; r < ROWS; r++)
			for (int c = 0; c < COLS; c++)
				intensity[r*COLS + c] = 0;
	}

	for (int r = 0; r < ROWS; r++)
		for (int c = 0; c < COLS; c++)
			(*ExtE)[r*COLS + c] = (IntensityWeight*OriginalImage[r*COLS + c] / 255) - SobelWeight*SobelImage[r*COLS + c] + IntEdgeW*intensity[r*COLS + c];;
}

void Create7x7Window(float *input, int r, int c, float **out)
{
	(*out) = (float*)calloc(7*7, sizeof(float*));
	int k = 0;
	for (int i = -3; i <= 3;i++)
	{
		for (int j = -3; j <= 3; j++){
			if (r + i >= 0&&r+i<ROWS&&c+j>=0&&c+j<COLS)
			(*out)[k] = input[(r + i)*COLS + c + j];
			else (*out)[k] = 0;

			k++;
		}
	}
}


void InternalEnergyCurv(float **IntE, struct points *act, int pointerc, int pointerp, int pointern)
{
	(*IntE) = (float*)calloc(7 * 7, sizeof(float*)); int r, c, r1, c1;
	unsigned char *window7x7;
	float *curve;
	curve = (float*)calloc(7 * 7, sizeof(float*));
	//r = act.row; c = act.col;	r1 = next.row; c1 = next.col;
	//Create7x7Window(OriginalImage, r, c, &window7x7);
	//calculate dist
	float sum = 0; float max, min; max = min = 0; int k = 0;
	for (int r = -3; r <= 3; r++)
	{
		for (int c = -3; c <= 3; c++)
		{

			sum = pow(act[pointern + 1].row - 2 * (act[pointerc].row + r) + act[pointerp].row, 2) + pow(act[pointern + 1].col - 2 * (act[pointerc].col + c) + act[pointerp].col, 2);
			(*IntE)[k] = sum; k++;
			if (min == 0 || min > sum)
				min = sum;
			if (max < sum)
				max = sum;

		}
	}
	for (int g = 0; g < 49; g++)
	{
		(*IntE)[g] = ((*IntE)[g] - min) / (max - min);
	}
}



void InternalEnergy(float **IntE,struct points *act,int pointern)
{
	(*IntE) = (float*)calloc(7*7, sizeof(float*)); int r, c,r1,c1;
	unsigned char *window7x7;
		//r = act.row; c = act.col;	r1 = next.row; c1 = next.col;
		//Create7x7Window(OriginalImage, r, c, &window7x7);
		//calculate dist
		float sum = 0; float max, min; max = min = 0; int k = 0;
		
		
				for (r = -3; r <= 3; r++)
				{
					for (c = -3; c <= 3; c++)
					{
						r1 = act[pointern].row + r; c1 = act[pointern].col + c;
						sum = 0;
						for (int i = 0; i < count; i++)
						{
							if (i != pointern)
							{
								sum += pow(r1 - act[i].row, 2) + pow(c1 - act[i].col, 2);
					}
				}
						(*IntE)[k] = sum;
						if (min == 0 || min>sum)
							min = sum;
						if (max < sum)
							max = sum;
						k++;
			}
		}
				float *curve; int weight; 
				if (BalloonFlag == 1)
					weight = -1;
				else if (RBMFlag == 1)
					weight = 1;
				//curve = (float *)calloc(49, sizeof(float*));
				InternalEnergyCurv(&curve, act, pointern, pointern - 1, pointern + 1);
				for (int i = 0; i < 49; i++)
				{
					(*IntE)[i] = (((*IntE)[i]- min) / (max - min))+weight*curve[i];
				}
}


void BalloonIE2(float **IntE,struct points act,struct points next)
{
	(*IntE) = (float*)calloc(49, sizeof(float*));
	float sum, max, min; max = min = 0; sum = 0; int r1, c1, r, c; int k=0;
	r1 = next.row; c1 = next.col; r = act.row; c = act.col;
	for (int i = -3; i <= 3; i++)
	{
	for (int j = -3; j <= 3; j++)
	{
	sum = pow((r1 - r + i), 2) + pow((c1 - c + j), 2);
	if (min == 0 || min > sum)
	min = sum;
	if (max < sum)
	max = sum;
	}
	}
	for (int i = -3; i <= 3; i++)
	{
	for (int j = -3; j <= 3; j++)
	{
	sum = pow((r1 - r + i), 2) + pow((c1 - c + j), 2);
	(*IntE)[k] = (sum - min) / (max - min);
	k++;
	}
	}
}
void CalTotalEBalloon(float *ie, float *ee, int *change_r, int *change_c)
{
	float total; float max, min; max = min = 0;
	for (int i = 0; i < 7; i++)
	{
		for (int j = 0; j < 7; j++)
		{
			total = +100*ie[i*7 + j] +100*ee[i*7+ j];
			if (min == 0 || min>total)
			{
				min = total;
				//(*change_r) = i; (*change_c) = j;
			}
			if (max < total){
				max = total;
			(*change_r) = i; (*change_c) = j;
		}
		}
	}
}

void CalTotalERBM(float *ie, float *ee, int *change_r, int *change_c)
{
	float total; float max, min; max = min = 0;
	for (int i = 0; i < 7; i++)
	{
		for (int j = 0; j < 7; j++)
		{
			total = 10* ie[i * 7 + j]+100*ee[i * 7 + j];
			if (min == 0 || min>total)
			{
				min = total;
				(*change_r) = i; (*change_c) = j;
			}
			if (max < total)
				max = total;
		}
	}
}



void BalloonModel()
{
	for (int times = 0; times < 45;times++)
	{
		float *IE; int j; float *EE; float *EEwindow; int change_r, change_c; float *ie2;
	IE = (float*)calloc(7 * 7, sizeof(float*));
	ExternalEnergy(&EE); int k;
	new_set = (struct points*)calloc(count, sizeof(*new_set));
	for (int i = 0; i < count; i++)
	{
		if (i == count - 1)
			j = 0;
		else j = i + 1;
		if (i == 0)
			k = count - 1;
		else k = i - 1;
		InternalEnergy(&IE, active, i);
		//BalloonIE2(&ie2, active[i], active[j]);
		//InternalEnergyCurv(&ie2,active,i,k,j);
		Create7x7Window(EE, active[i].row, active[i].col, &EEwindow);
		CalTotalEBalloon(IE, EEwindow, &change_r, &change_c);
		new_set[i].row = active[i].row + change_r - 3;
		new_set[i].col = active[i].col + change_c - 3;
	}
	PaintImage(OutImage);
	HDC hDC = GetDC(MainWnd);
	for (int i = 0; i < count; i++)
	{
		for (int r = -2; r <= 2; r++)
		{
			for (int c = -2; c <= 2; c++)
			{
				SetPixel(hDC, new_set[i].col + c, new_set[i].row + r, RGB(255, 0, 0));
				//OutImage[(new_set[i].row + r)*COLS + new_set[i].col + c]=
			}
		}
		//SetPixel(hDC,new_set[i].col, new_set[i].row, RGB(255, 0, 0));
	}
	ReleaseDC(MainWnd, hDC);
	active = new_set;
	
}

}


void RBMModel()
{
	for (int times = 0; times < 25;times++){
	float *IE; int j; float *EE; float *EEwindow; int change_r, change_c;
	IE = (float*)calloc(7 * 7, sizeof(float*));
	ExternalEnergy(&EE);
	new_set = (struct points*)calloc(count, sizeof(*new_set));
	for (int i = 0; i < count; i++)
	{
		if (i == count - 1)
			j = 0;
		else j = i + 1;
		InternalEnergy(&IE, active,i);
		Create7x7Window(EE, active[i].row, active[i].col, &EEwindow);
		CalTotalERBM(IE, EEwindow, &change_r, &change_c);
		new_set[i].row = active[i].row + change_r - 3;
		new_set[i].col = active[i].col + change_c - 3;
	}
	PaintImage(OutImage);
	HDC hDC = GetDC(MainWnd);
	for (int i = 0; i < count; i++)
	{
		for (int r = -2; r <= 2; r++)
		{
			for (int c = -2; c <= 2; c++)
			{ if (c==0&&r==0)
				SetPixel(hDC, new_set[i].col + c, new_set[i].row + r, RGB(0, 255, 0));
			else
				SetPixel(hDC, new_set[i].col + c, new_set[i].row + r, RGB(255, 0, 0));
			//OutImage[(new_set[i].row + r)*COLS + new_set[i].col + c]=
			}
		}
		//SetPixel(hDC,new_set[i].col, new_set[i].row, RGB(255, 0, 0));
	}
	ReleaseDC(MainWnd, hDC);
	active = new_set;
}
}

