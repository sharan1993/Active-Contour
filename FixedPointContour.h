#include<stdio.h>
#include<stdlib.h>
#include<math.h>


void InternalEnergyFixed(float **IntE, struct points *act, int pointern)
{
	(*IntE) = (float*)calloc(7 * 7, sizeof(float*)); int r, c, r1, c1;
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
	float *curve;
	curve = (float *)calloc(49, sizeof(float*));
	InternalEnergyCurv(&curve, act, pointern, pointern - 1, pointern + 1);
	for (int i = 0; i < 49; i++)
	{
		(*IntE)[i] = (((*IntE)[i] - min) / (max - min))+curve[i];
	}
}


void BalloonModel1(int move_point)
{
	for (int times = 0; times < 25; times++)
	{
		float *IE; int j; float *EE; float *EEwindow; int change_r, change_c; float *ie2;
		IE = (float*)calloc(7 * 7, sizeof(float*));
		ExternalEnergy(&EE); int k;
		new_set = (struct points*)calloc(count, sizeof(*new_set)); new_set = active;
		for (int i = move_point-(count/5); i <= move_point+(count/5); i++)
		{
			if (i == count - 1)
				j = 0;
			else j = i + 1;
			if (i == 0)
				k = count - 1;
			else k = i - 1;
			
			InternalEnergyFixed(&IE, active, i);
			//BalloonIE2(&ie2, active[i], active[j]);
			//InternalEnergyCurv(&ie2,active,i,k,j);
			Create7x7Window(EE, active[i].row, active[i].col, &EEwindow);
			CalTotalEBalloon(IE, EEwindow, &change_r, &change_c);
			new_set[i].row = active[i].row + change_r - 3;
			new_set[i].col = active[i].col + change_c - 3;
			if (i == move_point)
				new_set[i] = active[i];
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

void RBMModel1(int move_point)
{
	for (int times = 0; times < 5; times++){
		float *IE; int j; float *EE; float *EEwindow; int change_r, change_c;
		IE = (float*)calloc(7 * 7, sizeof(float*));
		ExternalEnergy(&EE);
		new_set = (struct points*)calloc(count, sizeof(*new_set)); new_set = active;
		for (int i = move_point - (count / 5); i <= move_point + (count / 5); i++)
		{
			if (i == count - 1)
				j = 0;
			else j = i + 1;
			InternalEnergyFixed(&IE, active, i);
			Create7x7Window(EE, active[i].row, active[i].col, &EEwindow);
			CalTotalERBM(IE, EEwindow, &change_r, &change_c);
			new_set[i].row = active[i].row + change_r - 3;
			new_set[i].col = active[i].col + change_c - 3;
			if (i == move_point)
				new_set[i] = active[i];
		}
		PaintImage(OutImage);
		HDC hDC = GetDC(MainWnd);
		for (int i = 0; i < count; i++)
		{
			for (int r = -2; r <= 2; r++)
			{
				for (int c = -2; c <= 2; c++)
				{
					if (c == 0 && r == 0)
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
