// FaceRecognize.cpp����������ͼ�������LBP���ֲ���ֵģʽ��������������ʶ����غ�������
// Copyright: cent
// 2015.9.17
// ~

#include "stdafx.h"
#include "resource.h"
#include <commdlg.h>
#include <direct.h>
#include <math.h>
#include "image.h"
#include "FaceDetect.h"
#include "FaceRecognize.h"



/************************************************************************************
	��̬��������

************************************************************************************/
// ��ȡ�Ҷ�ͼ�������LBP������ͳ��LBP����ֱ��ͼ��������
static void ExtractImageLbp(BmpImage *pImage, int *LBP );
// ����ͳ����ͼ��LBP���ƶ�
static int ChiSquareStatistic(int *dstLBP, int *baseLBP, int len);
// �������������Ƿ����Ŀ������
static bool SearchFace(char *facebaseDir, int *dstLBP);



/***********************************************************************************
*																				   *
*	��ȡλͼ�е�(m,n)block�е�(i,j)���ص���������LBPֵ����         			  	   *
*   ȡ�뾶Ϊr�Ļ��������ϣ�Բ����P���ص���������Uniform LBP                        *
*   LBPģʽ��ֵ������0->1��1->0�仯����С�ڵ���2�ε�ΪUniform LBP                  *
*																				   *
*   ���������image      - λͼ��������		                            		   *
*             width      - λͼ���                                                *
*             height     - λͼ�߶�                                                *
*             m          - �������к�                                              *
*             n          - �������к�                                              *
*             blkWidth   - λͼ�ֿ���                                            *
*             blkHeight  - λͼ�ֿ�߶�                                            *
*             i          - ��ǰ�������к�                                          *
*             j          - ��ǰ�������к�                                          *
*   ����ֵ��  LBP                                                				   *
*                        														   *
*																				   *
***********************************************************************************/
BYTE PixelLbp(char *image, int width, int height, 
              int m, int n, int blkWidth, int blkHeight, int i, int j)
{
	int k;
	int r, p;
	double pi;
	BYTE bit, lbp;
	int coff, tmpCoff;
	int x, y;

	r  = 2;
	p  = 8;
	pi = 3.1415927;
	lbp = 0;
	// ������������ֵ
	coff = (BYTE)image[(blkHeight*n+j)*width*3+(blkWidth*m+i)*3];
	// �뻷��Բ���ϵ�8���������ص�Ƚϣ��ö�ֵ���У�LBPģʽ��
	for ( k=0; k<p; k++ )
	{
		// ���ȷֲ��ڻ���Բ���ϵ����ص���block�е��±�
		x = (int)(i + r*sin(2*pi*k/p) + 0.5);
		y = (int)(j - r*cos(2*pi*k/p) + 0.5);
		// �����±��block����ϵת����image����ϵ
		x += blkWidth * m;
		y += blkHeight * n;
		if ( x < 0 )
			x = 0;
		else if ( x >= width )
			x = width-1;
		if ( y < 0 )
			y = 0;
		else if ( y >= height )
			y = height-1;
		tmpCoff = (BYTE)image[y*width*3+x*3];
		if ( tmpCoff < coff )
			bit = 0;
		else
			bit = 1;
		// ��˳�����LBP��ֵȨֵ������LBP
		lbp |= bit << (p-1-k);
	} // end for (k)
	return lbp;
}

/***********************************************************************************
*																				   *	
*   ȡ��ת������LBPֵ����            		                            		   *
*   ʮ������ֵ��СLBP��Ϊ��ת������LBP��ѭ����λ�õ�ǰLBP����ģʽ��ȡ��СֵLBP	   *
*                                                                                  *
*   ���������lbp  - LBPģʽ		                            		           *
*             n    - ģʽ������λ��λ��                                            *
*   ����ֵ��  ��ת������lbp                                                        *
*												     							   *
***********************************************************************************/
BYTE MinLbp(BYTE lbp, int n)
{
	int i;
	BYTE bit;
	BYTE min;

	// ѭ����λ�õ�ǰLBP����ģʽ��ȡ��СֵLBP
	min = lbp;
	for ( i=0; i<n; i++ )
	{
		//ѭ����1λ
		bit = lbp & (BYTE)(1 << (n-1));
		lbp = (lbp << 1) | bit;
		if ( lbp < min )
			min = lbp;
	}
	return min;
}

/***********************************************************************************
*																				   *
*	��ȡ�Ҷ�ͼ�������LBP������ͳ��LBP����ֱ��ͼ������������					   *
*																				   *
*   ���������pImage  - ͼ��ṹָ��     		                            	   *
*   ���������LBP     - LBP����ֱ��ͼ                           		           *
*																				   *
***********************************************************************************/
static void ExtractImageLbp(BmpImage *pImage, int *LBP )
{
	int i, j, k;
	int n, m, blkCount;
	int blkWidth, blkHeight;
	BYTE lbp;
	int ULBPtable[256];    // uniform LBP ����ӳ���
	char *tmpImage;

	memset(ULBPtable, -1, sizeof(ULBPtable));
	for ( i=0,k=0; i<256; i++ )
	{
		int count = 0;
		for ( j=0; j<7; j++ )
		{
			// LBPģʽѭ���仯0->1 or 1->0������count<=2: Uniform LBP
			if ( ((i&(1<<j))<<1 ) != (i&(1<<(j+1))) )
				count++;
		}
		// ����ӳ���ϵ
		if ( count <= 2 )
			ULBPtable[i] = k++;
	}

	tmpImage = (char *)malloc(pImage->width*pImage->height*3);
	// copy image��Ϣ����ʱ�ռ�
	memcpy(tmpImage, pImage->data, pImage->width*pImage->height*3);
	
	// ͼ��ֿ���ȡLBP��7*& blocks
	blkCount = 7;
	blkWidth = pImage->width / blkCount;
	blkHeight = pImage->height / blkCount;
	memset(LBP, 0, 59*blkCount*blkCount*sizeof(int));
	for ( n=0; n<blkCount; n++ )
	{
		for ( m=0; m<blkCount; m++ )
		{
			// �ֿ���ȡLBP
			for ( j=0; j<blkHeight; j++ )
			{
				for ( i=0; i<blkWidth; i++ )
				{
					// ��ȡimage(blkCount*blkCount)�ֿ���block(m,n)��pixel(i,j)��LBP
					lbp = PixelLbp(tmpImage, pImage->width, pImage->height, m, n, blkWidth, blkHeight, i, j);
					
					// ��ת������LBP
			//		lbp = MinLbp(lbp, 8);
			
					// LBP��ֵ���У�LBPģʽ����Ӧ��10��������LBPֵ
					pImage->data[(blkHeight*n+j)*pImage->width*3+(blkWidth*m+i)*3] = lbp;

					// ����ͳ��Uniform LBPֱ��ͼ��58( p*(p-1)+2 )��ULBP
					if ( ULBPtable[lbp] != -1 )
						LBP[(n*blkCount+m)*59+ULBPtable[lbp]]++;
					else // ��ULBP��Ϊ1�ֻ��LBP�����LBP�����
						LBP[(n*blkCount+m)*59+58]++;
				} // end for (i)
			} // end for (j)

		} // end for (m)
	} // end for (n)

	// �ͷ���ʱimage�ռ�
	free(tmpImage);
}

/***********************************************************************************
*																				   *
*	����ͳ������ͼ��LBPֱ��ͼ���������ƶȺ���									   *
*	��^2 (S,M) = �� { (Si-Mi)^2 / (Si+Mi) }										   *
*	     		  																   *
*   ���������dstLBP  - Ŀ������λͼLBP����ֱ��ͼ                            	   *
*             baseLBP - ������λͼLBP����ֱ��ͼ                                    *
*             len     - LBP����ֱ��ͼά��                                          *
*   ����ֵ��  ����ֵ                                            		           *
*	     		 																   *
***********************************************************************************/
static int ChiSquareStatistic(int *dstLBP, int *baseLBP, int len)
{
	int i, j;
	int w[49];  // ����λͼ�ֿ飺7 * 7��ÿ��Ȩ��ֵ
	double x2;

	// ��ʼ��ΪLBP����ֱ��ͼ����ͳ��ʱ�����Ȩ��
	for ( i=0; i<49; i++ )
		w[i] = 1;
		
	// �۾�Я��������Ҫ��Ϣ������Ȩ��Ϊ4
	w[7*1+1] = w[7*1+2] = w[7*2+1] = w[7*2+2] = 4;
	w[7*1+4] = w[7*1+5] = w[7*2+4] = w[7*2+5] = 4;
	
	// ��Ͷ�ͷ����Ȩ��Ϊ2
	w[7*4+3] = 2;
	w[0] = w[7*1+0] = 2;
	w[6] = w[7*1+6] = 2;
	
	// ���Ӻ����°�����Ȩ��Ϊ0
	w[7*2+3] = w[7*3+3] = 0;
	w[7*3+0] = w[7*4+0] = w[7*5+0] = w[7*6+0] = 0;
	w[7*3+6] = w[7*4+6] = w[7*5+6] = w[7*6+6] = 0;

	for ( x2=0,i=0; i<49; i++ )
	{
		for ( j=0; j<59; j++ )
		{
			if ( dstLBP[i*59+j] || baseLBP[i*59+j] )
				x2 += 1.0*w[i]*(dstLBP[i*59+j]-baseLBP[i*59+j]) *
							   (dstLBP[i*59+j]-baseLBP[i*59+j]) / 
							   (dstLBP[i*59+j]+baseLBP[i*59+j]);
		}
	}
	return (int)x2;
}

/***********************************************************************************
*																				   *
*	���������в�����Ŀ������λͼLBP����ֱ��ͼ�����������						   *
*	�趨��ֵ��С�ڷ�ֵ����Ϊ�ҵ�ͬһ����										   *
*	  																			   *
*   ���������dstLBP      - Ŀ������λͼLBP����ֱ��ͼ                          	   *
*             facebaseDir - ������·��                                             *
*																				   *
***********************************************************************************/
static bool SearchFace(char *facebaseDir, int *dstLBP)
{
	char findFileName[256];
	char strFilter[256];
	char findFilePath[256];
	BmpImage *image, *faceImage;
	WIN32_FIND_DATA findFileData;
	HANDLE hFindFile;
	int LBP[49*59];    // ����λͼ�ֿ飺7 * 7

	// ͨ�������facebaseDirĿ¼�µ������ļ�������Ŀ¼
	strcpy(strFilter, facebaseDir);
	strcat(strFilter, "\\*.*");
	
	// ��ȡĿ¼���Ҿ��������һ���ļ�����
	hFindFile = FindFirstFile(strFilter, &findFileData);
	// ��ǰĿ¼�Ƿ����
	if ( hFindFile == INVALID_HANDLE_VALUE )
		return false;

	do
	{
		// ����Ŀ¼
		if ( findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			continue;

		strcpy(findFileName, findFileData.cFileName);
		// ���Է�.bmp�ļ�
		if ( strcmp(findFileName+strlen(findFileName)-4, ".bmp" ) != 0 )
			continue;

		// .bmp�ļ��ľ���·��
		strcpy(findFilePath, facebaseDir);
		strcat(findFilePath, "\\");
		strcat(findFilePath, findFileName);
		image = ReadBmpFile(findFilePath);
		if( image == NULL)
			continue;
		
		faceImage = ExtractFace(image);
		if ( faceImage == NULL )
		{
			FreeBmpImage(image);
			continue;
		}
		ShowBmpImage(faceImage, 750, 20);
		
		// ɫ�ʿռ�ģ��ת��
		RgbToYcbcr(faceImage, faceImage);
		ExtractImageLbp(faceImage, LBP);

		ShowBmpGreyImage(faceImage, 750, 100);

		int x2;
		// LBP����ͳ�����ƶ���ֵ���趨��ֵ�Աȣ�С��Ĭ����ֵʱ��ͼ����ͬ
		if ( (x2=ChiSquareStatistic(dstLBP, LBP, 49*59)) < 3000 )
		{
			char strx2[50];
			
			wsprintf(strx2, "%s%d", "x^2 = ", x2);
			TextOut(hWinDC, 700, 180, strx2, strlen(strx2));
			ShowBmpImage(image, 660, 200);
			
			FindClose(hFindFile);
			FreeBmpImage(image);
			FreeBmpImage(faceImage);
			return true;
		}
		
		// �ͷ�ReadBmpFile��ExtractFace��̬���ɵ�λͼ�ռ�
		FreeBmpImage(image);
		FreeBmpImage(faceImage);

	} while ( FindNextFile(hFindFile, &findFileData) );
	FindClose(hFindFile);
	
	// Update Show Rect()
	RECT rt;
	rt.left = 750, rt.top = 20;
	rt.right = 1000, rt.bottom = 200;
	InvalidateRect(hMainWnd, &rt, true);

	return false;
}


/*****************************************************************************************
*																						 *
*	���������в��ҳ����ƶ����ȽϽӽ�Ŀ��������ͼƬ����									 *
*																						 *
*   ���������pImage       - Ŀ��λͼ�ṹָ��  	                						 *
*             facebasePath - ������·��                          						 *
*																						 *
*****************************************************************************************/
bool RecognizeFace(BmpImage *pImage, char *facebasePath)
{
	int dstLBP[49*59];    // ����λͼ�ֿ飺7 * 7
	BmpImage *faceImage;
	
	faceImage = ExtractFace(pImage);
	if ( faceImage == NULL )
		return false;
	ShowBmpImage(faceImage, 660, 20);
	
	RgbToYcbcr(faceImage, faceImage);
	ExtractImageLbp(faceImage, dstLBP);

	ShowBmpGreyImage(faceImage, 660, 100);
	
	// �ͷ�ExtractFace��̬���ɵ�λͼ�ռ�
	FreeBmpImage(faceImage);

	return SearchFace(facebasePath, dstLBP);
}

/*****************************************************************************************
*																						 *
*	��������ͼ����⺯��         														 *
*																						 *
*   ���������imgFileName   - ����λͼ�ļ���    	               						 *
*             facebasePath  - ������λͼλ��·��                       					 *
*																						 *
*****************************************************************************************/
bool EnterFace(char *imgFileName, char *facebasePath)
{
	char fileName[256];
	char strFilter[256];
	WIN32_FIND_DATA findFileData;
	HANDLE hFindFile;
	int count;

	// ��������������Ŀ¼
	CreateDirectory(facebasePath, NULL);

	// ͨ�������facebaseDirĿ¼�µ������ļ�������Ŀ¼
	strcpy(strFilter, facebasePath);
	strcat(strFilter, "\\*.*");
	
	// ��ȡĿ¼���Ҿ��������һ���ļ�����
	hFindFile = FindFirstFile(strFilter, &findFileData);
	// ��ǰĿ¼�Ƿ����
	if ( hFindFile == INVALID_HANDLE_VALUE )
		return false;
	count = 0;
	do
	{
		// ����Ŀ¼
		if ( findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			continue;

		strcpy(fileName, findFileData.cFileName);
		// ���Է�.bmp�ļ�
		if ( strcmp(fileName+strlen(fileName)-4, ".bmp" ) != 0 )
			continue;
		
		count++;
	} while ( FindNextFile(hFindFile, &findFileData) );
	FindClose(hFindFile);

	// ����Ź���¼������ͼ���ļ���
	wsprintf(fileName, "%s%s%d%s", facebasePath, "\\face", count++, ".bmp");
	if ( CopyFile(imgFileName, fileName, 1) == NULL )
		return false;
	return true;
}

/*****************************************************************************************
*																						 *
*	ѡ��ɾ���������е������������� 														 *
*																						 *
*   ���������facebasePath  - ������λͼλ��·��                       					 *
*																						 *
*****************************************************************************************/
void DeleteFace(char *facebasePath)
{
	char facePath[256];
	char fileTitle[256];
	char selImgFileDir[256];
	OPENFILENAME ofn;
	BmpImage *image, *faceImage;
	int dstLBP[49*59];    // ����λͼ�ֿ飺7 * 7

	memset(&ofn,0,sizeof(ofn));
	ofn.lStructSize=sizeof(OPENFILENAME);
	ofn.hwndOwner=NULL;
	ofn.hInstance=NULL;
	ofn.lpstrFilter=TEXT("bmp files\0*.bmp\0All File\0*.*\0\0");
	ofn.lpstrCustomFilter=NULL;
	ofn.nMaxCustFilter=0;
	ofn.nFilterIndex=1;
	ofn.lpstrFile=facePath;
	ofn.nMaxFile=MAX_PATH;
	ofn.lpstrFileTitle=fileTitle;
	ofn.nMaxFileTitle=99;
	ofn.lpstrInitialDir=facebasePath;
	ofn.lpstrTitle="ѡ��Ҫ�Ƴ�����������ͼ��";
	ofn.Flags=OFN_FILEMUSTEXIST;
	ofn.lpstrDefExt="raw";
	ofn.lCustData=NULL;
	ofn.lpfnHook=NULL;
	ofn.lpTemplateName=NULL;
	facePath[0]='\0';
	GetOpenFileName(&ofn); 

	// ȡ��ѡ��Ŀ���ļ���
	getcwd(selImgFileDir, MAX_PATH);

	// ·����ʱ��ȡ��
	if ( !strlen(facePath) )
		return ;

	image = ReadBmpFile(facePath);
	if( image == NULL )
		return ;
		
	TextOut(hWinDC, 660, 20, "��ѡ����Ƴ�������������", strlen("��ѡ����Ƴ�������������"));
	ShowBmpImage(image, 660, 50);
	
	faceImage = ExtractFace(image);
	if( faceImage == NULL )
	{
		FreeBmpImage(image);
		return ;
	}
	ShowBmpImage(faceImage, 730, image->height+80);
	
	RgbToYcbcr(faceImage, faceImage);
	ExtractImageLbp(faceImage, dstLBP);
	ShowBmpGreyImage(faceImage, 850, image->height+80);

	if ( strcmp(selImgFileDir, facebasePath) )
	{
		MessageBox(hMainWnd, "�Ƴ�ʧ�ܣ�ѡ��Ĳ�������ȡ����Ŀ¼...\\Facebase�µ�ͼ��", "�Ƴ���������", 0);
		
		FreeBmpImage(image);
		FreeBmpImage(faceImage);
		return ;
	}

	if ( MessageBox(hMainWnd, "ȷ���Ƴ���ѡ��������ͼ��", "�Ƴ���������", MB_OKCANCEL) == IDOK )
	{
		if ( !DeleteFile(facePath) )
			MessageBox(hMainWnd, "�Ƴ�Ŀ����������ͼ��ʧ�ܣ�", "�Ƴ���������", 0);
		else
			MessageBox(hMainWnd, "�Ƴ�Ŀ�����������ɹ���", "�Ƴ���������", 0);
	}
	
	FreeBmpImage(image);
	FreeBmpImage(faceImage);

	// Update Show Rect()
	RECT rt;
	rt.left = 650, rt.top = 20;
	rt.right = 1000, rt.bottom = 500;
	InvalidateRect(hMainWnd, &rt, true);
}
