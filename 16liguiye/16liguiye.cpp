// 16liguiye.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include "pch.h"
//#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <math.h>
typedef struct
{
	//命名规则:int型开头就是n
	int nWidth;
	int nHeight;
	int nBands;//波段数
	unsigned char *pData;//像元灰度值
	float fMax;//最大值
	float fMin;//最小值
	float fMean;//均值
	float fStd;//标准差
}struct_dib;

//定义一个颜色数组把所需的颜色放进去，根据需要取用
unsigned char colorarry[10][3] = { { 0, 255, 0 }, { 0,0,255 }, { 255,0,0 }, { 0,255,255 }, { 255,255,0 }, { 255,0,255 } };

//读取BMP影像的信息到dib结构体里
void readbmp(const char *path, struct_dib &mydib)
{
	BITMAPFILEHEADER header;
	BITMAPINFOHEADER info;
	RGBQUAD rgb[256];
	FILE *fp = fopen(path, "rb");
	fread(&header, sizeof(BITMAPFILEHEADER), 1, fp);
	fread(&info, sizeof(BITMAPINFOHEADER), 1, fp);
	mydib.nWidth = info.biWidth;
	mydib.nHeight = info.biHeight;
	if (info.biBitCount == 8)
	{
		mydib.nBands = 1;
		fread(rgb, sizeof(RGBQUAD), 256, fp);
		printf("8位影像读取成功");
	}
	if (info.biBitCount == 24)
	{
		mydib.nBands = 3;
		fread(rgb, sizeof(RGBQUAD), 256, fp);
		printf("24位影像读取成功");
	}
	mydib.pData = new unsigned char[mydib.nWidth*mydib.nHeight*mydib.nBands];
	fread(mydib.pData, sizeof(unsigned char), mydib.nWidth*mydib.nHeight*mydib.nBands, fp);
	fclose(fp);//与上面的fopen对应
}
//保存bmp影像
void writebmp(const char *path, struct_dib &dib)
{
	BITMAPFILEHEADER header;
	BITMAPINFOHEADER info;
	RGBQUAD rgb[256];
	FILE *fp = fopen(path, "wb");
	/////////////////////////////////////////////////
	header.bfType = 19778;
	header.bfReserved1 = 0;
	header.bfReserved2 = 0;
	if (dib.nBands == 1)
	{
		info.biBitCount = 8;
		header.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 256;
	}
	if (dib.nBands == 3)
	{
		info.biBitCount = 24;
		header.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	}
	header.bfSize = header.bfOffBits + dib.nWidth*dib.nHeight*dib.nBands;
	////////////////////////////////////////////////
	info.biClrImportant = 256;
	info.biClrUsed = 256;
	info.biCompression = 0;
	info.biHeight = dib.nHeight;
	info.biPlanes = 1;
	info.biSize = sizeof(BITMAPINFOHEADER);
	info.biSizeImage = dib.nWidth*dib.nHeight*dib.nBands;
	info.biWidth = dib.nWidth;
	info.biXPelsPerMeter = 0;
	info.biYPelsPerMeter = 0;
	////////////////////////////////////////////////
	for (int i = 0; i < 256; i++)
	{
		rgb[i].rgbRed = i;
		rgb[i].rgbGreen = i;
		rgb[i].rgbBlue = i;
		rgb[i].rgbReserved = 0;
	}
	////////////////////////////////////////////////
	fwrite(&header, sizeof(BITMAPFILEHEADER), 1, fp);
	fwrite(&info, sizeof(BITMAPINFOHEADER), 1, fp);
	if (info.biBitCount == 8)
	{
		fwrite(rgb, sizeof(RGBQUAD), 256, fp);
	}
	fwrite(dib.pData, sizeof(unsigned char), dib.nWidth*dib.nHeight*dib.nBands, fp);
	fclose(fp);
}
//bmp影像统计（最大值、最小值、均值、方差）
void bmpstatistics(struct_dib &dib)
{
	float fMax = -999999;
	float fMin = 9999999;
	double dMean = 0;
	double dStd = 0;
	double dSumbysum = 0;;
	for (int j = 0; j < dib.nHeight; j++)//一行一行遍历
	{
		for (int i = 0; i < dib.nWidth; i++)
		{
			unsigned char ctemp = dib.pData[j*dib.nWidth + i];//第j行第i列的数值
			if (ctemp > fMax) fMax = ctemp;
			if (ctemp < fMin) fMin = ctemp;
			dMean += ctemp;
			dSumbysum += ctemp * ctemp;//方差 = 平方的均值-均值的平方	
		}
	}
	dMean /= dib.nHeight*dib.nWidth;
	dSumbysum /= dib.nHeight*dib.nWidth;
	dStd = sqrt(dSumbysum - dMean * dMean);//标准差=sqrt(方差)
	/////////////////////////////////////
	dib.fMax = fMax;
	dib.fMin = fMin;
	dib.fMean = dMean;
	dib.fStd = dStd;
}
//统计灰度值并生成直方图
void GrayCount(struct_dib &mydib)
{
	//定义统计灰度值的数组
	int grayCount[256];
	for (int x = 0; x < 256; x++)
	{
		grayCount[x] = 0;
	}
	//遍历所有的行
	for (int i = 0; i < mydib.nHeight; i++)
	{
		//遍历所有的列
		for (int j = 0; j < mydib.nWidth; j++)
		{
			int grayvalue = mydib.pData[i*mydib.nWidth + j];
			grayCount[grayvalue]++;
		}
	}
	//命令行里用*生成灰度直方图
	for (int i = 0; i < 256; i++)
	{
		printf("%d:", i);
		for (int j = 0; j < grayCount[i]; j++)
		{
			printf("*");
		}
		printf(" %d\n", grayCount[i]);
	}
}
//影像拉伸
void stretch(struct_dib &mydib, float A, float B) {
	float a, b;
	a = mydib.fMin;
	b = mydib.fMax;
	if (A < B)
	{
		for (int i = 0; i < mydib.nHeight; i++) //一行一行遍历
		{
			for (int j = 0; j < mydib.nWidth; j++)
			{
				unsigned char ctemp = mydib.pData[i*mydib.nWidth + j];//第i行第j列的数值
				mydib.pData[i*mydib.nWidth + j] = A + (B - A)*(mydib.pData[i*mydib.nWidth + j] - a) / (b - a);
				//防止拉伸后越界
				if (mydib.pData[i*mydib.nWidth + j] > 255) mydib.pData[i*mydib.nWidth + j] = 255;
				if (mydib.pData[i*mydib.nWidth + j] < 0) mydib.pData[i*mydib.nWidth + j] = 0;
			}
		}
	}
	else
		printf("A必须小于B");
}
//k均值拉伸
void meanstretch(struct_dib &mydib, float k, float A, float B) {

	float a, b;
	a = mydib.fMean - k * mydib.fStd;
	b = mydib.fMean + k * mydib.fStd;

	if (A < B)
	{
		for (int i = 0; i < mydib.nHeight; i++) //一行一行遍历
		{
			for (int j = 0; j < mydib.nWidth; j++)
			{
				unsigned char ctemp = mydib.pData[i*mydib.nWidth + j];//第i行第j列的数值
				mydib.pData[i*mydib.nWidth + j] = A + (B - A)*(mydib.pData[i*mydib.nWidth + j] - a) / (b - a);
				//防止拉伸后越界
				if (mydib.pData[i*mydib.nWidth + j] > 255) mydib.pData[i*mydib.nWidth + j] = 255;
				if (mydib.pData[i*mydib.nWidth + j] < 0) mydib.pData[i*mydib.nWidth + j] = 0;
			}
		}
	}
	else
		printf("A必须小于B");

}
//假彩色合成
void pseudocolor(struct_dib &gray, struct_dib &color, int nClass) {
	/*其他信息暂不需要，所以只用改以下三个值*/
	color.nWidth = gray.nWidth;
	color.nHeight = gray.nHeight;
	color.nBands = 3;
	/*新开辟一块内存*/
	color.pData = new unsigned char[color.nWidth*color.nHeight*color.nBands];
	/*定义步长，分类赋颜色值*/
	float fStep = 255 / nClass;
	for (int j = 0; j < color.nHeight; j++)
	{
		for (int i = 0; i < color.nWidth; i++)
		{
			/*把以前的灰度信息值按照索引存进新的颜色表*/
			unsigned char cTemp = gray.pData[j*color.nWidth + i];
			/*定义索引*/
			int nindex = cTemp / fStep;
			/*颜色的存储模式为rgbrgbrgb...所以在行和列*3的基础上还要加上0(r)1(g)2(b)*/
			/*r赋成颜色数组里的第一种颜色，g第二种，b第三种*/
			color.pData[j*color.nWidth * 3 + i * 3 + 0] = colorarry[nindex][0];
			color.pData[j*color.nWidth * 3 + i * 3 + 1] = colorarry[nindex][1];
			color.pData[j*color.nWidth * 3 + i * 3 + 2] = colorarry[nindex][2];
		}
	}

}
//画十字叉叉
void draw(struct_dib &gray, struct_dib &color, int hang, int lie) {
	/*其他信息暂不需要，所以只用改以下三个值*/
	color.nWidth = gray.nWidth;
	color.nHeight = gray.nHeight;
	color.nBands = 3;
	/*新开辟一块内存*/
	color.pData = new unsigned char[color.nWidth*color.nHeight*color.nBands];

	for (int j = 0; j < color.nHeight; j++)
	{
		for (int i = 0; i < color.nWidth; i++)
		{
			/*把以前的灰度信息值按照索引存进新的颜色表*/
			unsigned char cTemp = gray.pData[j*color.nWidth + i];

			/*颜色的存储模式为rgbrgbrgb...所以在行和列*3的基础上还要加上0(r)1(g)2(b)*/
			/*r赋成颜色数组里的第一种颜色，g第二种，b第三种*/
			color.pData[j*color.nWidth * 3 + i * 3 + 0] = gray.pData[j*gray.nWidth + i];
			color.pData[j*color.nWidth * 3 + i * 3 + 1] = gray.pData[j*gray.nWidth + i];
			color.pData[j*color.nWidth * 3 + i * 3 + 2] = gray.pData[j*gray.nWidth + i];			
		}
	}
	//竖着五个像素
	for (int j = hang -5 ; j < hang + 6 ; j++)
	{
		color.pData[j*color.nWidth * 3 + lie * 3 + 0] = colorarry[1][0];
		color.pData[j*color.nWidth * 3 + lie * 3 + 1] = colorarry[1][1];
		color.pData[j*color.nWidth * 3 + lie * 3 + 2] = colorarry[1][2];
	}
	//横着五个像素
	for (int i = lie - 5; i < lie + 6; i++)
	{
		color.pData[hang*color.nWidth * 3 + i * 3 + 0] = colorarry[1][0];
		color.pData[hang*color.nWidth * 3 + i * 3 + 1] = colorarry[1][1];
		color.pData[hang*color.nWidth * 3 + i * 3 + 2] = colorarry[1][2];
	}
}
void main()
{
	struct_dib mydib;
	readbmp("lena.bmp", mydib);
	bmpstatistics(mydib);
	printf("The Result of bmp statistics as follows:\n");
	printf("max = %f, min = %f, mean = %f, std = %f\n", mydib.fMax, mydib.fMin, mydib.fMean, mydib.fStd);
	struct_dib pseu;
	draw(mydib, pseu, 10,130);
	writebmp("pseu_lena.bmp", pseu);
}
