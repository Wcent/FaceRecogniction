// Image.cpp��λͼ��ش���������
// Copyright: cent
// 2015.9.17
// ~

#include "stdafx.h"
#include "resource.h"
#include <commdlg.h>
#include <direct.h>
#include <math.h>
#include "image.h"

HDC hWinDC;



/************************************************************************************************
*																								*
*	��̬����Ŀ��λͼ�ڴ溯��                                           							*
*																								*
*   ���������width  - ��̬����Ŀ��λͼ���                                                     *
*             height - ��̬����Ŀ��λͼ�߶�                                                     *
*   ���ػ�  ��         ָ��̬����Ŀ��λͼ�ڴ�ָ��                                             *
*																								*
************************************************************************************************/
BmpImage * MallocBmpImage(int width, int height)
{
	BmpImage *pImage;
	
	pImage = (BmpImage *)malloc(sizeof(BmpImage));
	if( pImage == NULL)
		return NULL;
		
	pImage->width  = width;
	pImage->height = height;
	
	// 3��width*height����������RGB����
	pImage->data = (char *)malloc(width*height*3);
	if( pImage->data == NULL )
	{
		free(pImage);
		return NULL;
	}
	
	return pImage;
}

/************************************************************************************************
*																								*
*	���ն�̬����λͼ�ڴ溯��                                           							*
*																								*
*   ���������pImage  - ָ��̬�����ڴ�Ŀ��λͼָ��                                            *
*																								*
************************************************************************************************/
void FreeBmpImage(BmpImage *pImage)
{
	pImage->width  = 0;
	pImage->height = 0;
	
	// ���ͷ����������ڴ棬���ͷ�λͼ�ṹ�ڴ�
	free(pImage->data);
	free(pImage);
}

/************************************************************************************************
*																								*
*	��ȡ�ļ�������OpenImageFile(char *OPDLTitle, char *pImgFileName)							*
*																								*
*   ���������OPDLTitle    - �ļ�ѡ��Ի������                                                 *
*   ���������pImgFileName - ѡ��Ŀ����ļ���                                                   *
*																								*
************************************************************************************************/
void OpenImageFile(char *OPDLTitle, char *pImgFileName)
{
	char FileTitle[100], ImgDlgFileDir[256];
	OPENFILENAME ofn;

	getcwd(ImgDlgFileDir, MAX_PATH);

	memset(&ofn,0,sizeof(ofn));
	ofn.lStructSize=sizeof(OPENFILENAME);
	ofn.hwndOwner=NULL;
	ofn.hInstance=NULL;
	ofn.lpstrFilter=TEXT("bmp files\0*.bmp\0All File\0*.*\0\0");
	ofn.lpstrCustomFilter=NULL;
	ofn.nMaxCustFilter=0;
	ofn.nFilterIndex=1;
	ofn.lpstrFile=pImgFileName;
	ofn.nMaxFile=MAX_PATH;
	ofn.lpstrFileTitle=FileTitle;
	ofn.nMaxFileTitle=99;
	ofn.lpstrInitialDir=ImgDlgFileDir;
	ofn.lpstrTitle=OPDLTitle;
	ofn.Flags=OFN_FILEMUSTEXIST;
	ofn.lpstrDefExt="raw";
	ofn.lCustData=NULL;
	ofn.lpfnHook=NULL;
	ofn.lpTemplateName=NULL;
	pImgFileName[0]='\0';
	GetOpenFileName(&ofn); 

	getcwd(ImgDlgFileDir, MAX_PATH);
}

/**********************************************************************************************
*																							  *	
*   ��ȡλͼ�ļ�ԭʼ��������24λ���ɫλͼ����RGB���ݺ���									  *	
*   ������������λͼwidth*3�ֽ�����4������0�������ݣ��õ�bmp��RGB��ʵͼ������				  *
*																							  *	
*   �������������pImage - ����24λ���ɫԭʼλͼ�ļ��зֽ�����������ݵ�λͼ                 *
*																							  *
**********************************************************************************************/
void ExtractImageData(BmpImage *pImage)
{
	int i, j;
	int cntZeros;
	int cntPixels;
	
	// ÿ�������ֽ���Ϊ��4��������0����ÿ�������ֽ�����4�ı���
	if ( !(pImage->width*3 % 4) )
		return ;
	
	// ����0���ֽ���
	cntZeros = 4 - pImage->width * 3 % 4;
	
	// ��������ָʾ��
	cntPixels = 0;
	for ( j=0; j<pImage->height; j++ )
	{
		for ( i=0; i<pImage->width; i++ )
		{
			// λͼ��ʵ�������ݱ���
			pImage->data[j*pImage->width*3+i*3]   = pImage->data[cntPixels++];
			pImage->data[j*pImage->width*3+i*3+1] = pImage->data[cntPixels++];
			pImage->data[j*pImage->width*3+i*3+2] = pImage->data[cntPixels++];
		}
		// ÿ�ж���cntZeros���ֽڵĲ���4��������Ч��������
		for ( i=0; i<cntZeros; i++ )
			cntPixels++;
	}
}

/**********************************************************************************************
*																							  *	
*   ��תԭʼ24λ���ɫλͼ����RGB���ݺ���													  *	
*   ԭʼλͼ�ļ����ش��µ��ϣ������ң���תΪ���ϵ��£�������                              *
*																							  *	
*   �������������pImage - ����24λ���ɫԭʼλͼ�ļ��зֽ������������λͼָ��               *
*																							  *
**********************************************************************************************/
void ReverseImageData(BmpImage *pImage)
{
	int i, j;
	char tmpPixel;
	
	// 24λ���ɫλͼÿ��������3���ֽ�R��G��B����ʾ
	for (i=0; i<pImage->height/2; i++)
	{
		for (j=0; j<pImage->width; j++)
		{
			// R
			tmpPixel = pImage->data[i*pImage->width*3+j*3];
			pImage->data[i*pImage->width*3+j*3] = pImage->data[(pImage->height-i-1)*pImage->width*3+j*3+2];
			pImage->data[(pImage->height-i-1)*pImage->width*3+j*3+2] = tmpPixel;
			// G
			tmpPixel = pImage->data[i*pImage->width*3+j*3+1];
			pImage->data[i*pImage->width*3+j*3+1] = pImage->data[(pImage->height-i-1)*pImage->width*3+j*3+1];
			pImage->data[(pImage->height-i-1)*pImage->width*3+j*3+1] = tmpPixel;
			// B
			tmpPixel = pImage->data[i*pImage->width*3+j*3+2];
			pImage->data[i*pImage->width*3+j*3+2] = pImage->data[(pImage->height-i-1)*pImage->width*3+j*3];
			pImage->data[(pImage->height-i-1)*pImage->width*3+j*3] = tmpPixel;
		}
	}
}

/************************************************************************************************
*																								*
*   ��ȡ24λ���ɫλͼ�������ݺ�����ÿ��������3���ֽ�RGB��ʾ                                    *
*   ��ͼ���ļ�������λͼ�ļ��ṹ���ֽ��λͼ�ļ�ͷ��λͼ��Ϣͷ����������						*
*   ���������imgFileName - �ļ���                                                              *
*   ����ֵ  ��              ���24λ���ɫλͼ����RGB���ݼ��ߴ�Ϊ320*240λͼ�ṹָ��            *
*																								*
************************************************************************************************/
BmpImage* ReadBmpFile(LPSTR imgFileName)
{
	OFSTRUCT of;
	HFILE Image_fp;
	BITMAPFILEHEADER bfImage;
	BITMAPINFOHEADER biImage;
	BmpImage *pImage;


	if( strlen(imgFileName) == 0 )
		return NULL;
	
	Image_fp = OpenFile(imgFileName, &of, OF_READ);
	if (Image_fp == HFILE_ERROR) 
	{
		MessageBox(NULL, "�򿪶�bmpͼ���ļ�����", "���ļ�������Ϣ", MB_OK);
		return NULL;
	}

	// ��λ����ȡλͼ�ļ�ͷ
	_llseek(Image_fp, 0, 0);
	_lread(Image_fp, &bfImage, sizeof(BITMAPFILEHEADER));

	// "BM" (0x4d42)����ʾλͼBMP
	if ((bfImage.bfType != 0x4d42)) 
	{
		MessageBox(NULL, "��bmp�ļ�", "��bmp�ļ�������Ϣ", MB_OK);
		_lclose(Image_fp);
		return NULL;
	}

	// ��ȡλͼ��Ϣͷ
	_lread(Image_fp, &biImage, sizeof(BITMAPINFOHEADER));

	if (biImage.biBitCount != 24)
	{
		MessageBox(NULL, "��24λ���ɫλͼ", "��ȡbmp�ļ�������Ϣ", MB_OK);
		_lclose(Image_fp);
		return NULL;
	}

	// ������ʱ�洢�������ݿռ�
	pImage = MallocBmpImage(biImage.biWidth, biImage.biHeight);
	if( pImage == NULL )
	{
		MessageBox(NULL, "ϵͳû���㹻�ڴ�ռ��Է���洢λͼ", "��ȡbmp�ļ�������Ϣ", MB_OK);
		_lclose(Image_fp);
		return NULL;
	}

	// ��λ����ȡλͼ������������
	_llseek(Image_fp, bfImage.bfOffBits, 0);
	_lread(Image_fp, pImage->data, biImage.biSizeImage);

	// ����bmp���������в���ÿ�������ֽ���4������Ч����0����RGB��ʵ��������
	ExtractImageData(pImage);
	
	// bmpͼƬ���ش��µ��ϣ������ң���תΪ���ϵ��£�������
	ReverseImageData(pImage);

	//ͼ�����Ź�һ�����޶����ͼ��ߴ磺320*240
	NormalizeImageSize(pImage, 320, 240);
						
	_lclose(Image_fp);

	return pImage;
}


/************************************************************************************************
*																								*
*	����Ļ����ʾλͼ����ShowBmpImage(BmpImage *pImage, int xPos, int yPos)                    	*
*																								*
*   ���������pImage - ���λͼ�������ݵ�����������ߴ�ṹ��ָ��								*
*		      xPos   - ͼ����ʾ��ʼλ��x����													*
*		      yPos   - ͼ����ʾ��ʼλ��y����													*
*																								*
************************************************************************************************/
void ShowBmpImage(BmpImage *pImage, int xPos, int yPos)	
{
	int i,j;
	int r, g, b;
	
	for (i=0; i<pImage->height; i++) 
	{
		for (j=0; j<pImage->width; j++) 
		{
			r = (BYTE) pImage->data[i*pImage->width*3+j*3]; 
			g = (BYTE) pImage->data[i*pImage->width*3+j*3+1];
			b = (BYTE) pImage->data[i*pImage->width*3+j*3+2];
			
			SetPixel(hWinDC, j+xPos, i+yPos, RGB(r, g, b));
		}
	}
}

/************************************************************************************************
*																						        *
*   ��ʾλͼ��YCbCr�ռ�Ҷ�ͼ����   													        *
*                                                                                               *
*   ���������pImage - ���λͼ����YCbCr�ռ�Ҷ����ݼ��ߴ�ṹ��ָ��							*
*		      xPos   - ͼ����ʾ��ʼλ��x����													*
*		      yPos   - ͼ����ʾ��ʼλ��y����													*
*																						        *
************************************************************************************************/
void ShowBmpGreyImage(BmpImage *pImage, int xPos, int yPos)
{
	int i,j;
	int y;
	
	for (i=0; i<pImage->height; i++) 
	{
		for (j=0; j<pImage->width; j++) 
		{
			y = (BYTE)pImage->data[i*pImage->width*3+j*3];
			SetPixel(hWinDC, j+xPos, i+yPos, RGB(y, y, y));
		}
	}
}

/************************************************************************************************
*																					   			*
*   ��������������ʾ��ͼ����Ϊ��ɫ													   			*
*                                                                                               *
*   ���������width  - ���������� 															*
*             height - ��������߶� 															*
*		      xPos   - ͼ����ʾ��ʼλ��x����													*
*		      yPos   - ͼ����ʾ��ʼλ��y����													*
*																					   			*
************************************************************************************************/
void CleanUpShowImage(int width, int height, int xPos, int yPos)
{
	int i, j;

	for ( j=0; j<height/2+1; j++ )
	{
		for ( i=0; i<width; i++ )
		{
			SetPixel(hWinDC, xPos+i, yPos+j, RGB(255, 255, 255));
			SetPixel(hWinDC, xPos+i, yPos+height-j, RGB(255, 255, 255));
		}
	}
}

/************************************************************************************************
*																								*	
*   ͼ��ߴ����Ź�һ�����������ٽ���ֵ�㷨��������												*
*                                                                                               *
*   �������������pImage  - λͼ�ṹָ��														*
*                 width   - ���ź���															*
*	              height  - ���ź�߶�															*
*																								*
************************************************************************************************/
void NormalizeImageSize( BmpImage *pImage, int width, int height)
{
	int i, j;
	int x, y;
	double xRate, yRate;
	char *tmpData;

	tmpData = (char *)malloc(width*height*3);
	if (tmpData == NULL)
		return ;

	// ���ű���
	xRate = (double)pImage->width/width;
	yRate = (double)pImage->height/height;
	
	for ( j=0; j<height; j++ )
	{
		// ӳ�䵽ԭpImage->height�±�
		y = (int)(j*yRate+0.5);
		if ( y<0 )
			y = 0;
		else if ( y>=pImage->height )
			y = pImage->height-1;

		for ( i=0; i<width; i++ )
		{
			// ӳ�䵽ԭpImage->width�±�
			x = (int)(i*xRate+0.5);
			if ( x<0 )
				x = 0;
			else if ( x>=pImage->width )
				x = pImage->width-1;

			tmpData[j*3*width+i*3+0] = pImage->data[y*3*pImage->width+x*3+0];  // R
			tmpData[j*3*width+i*3+1] = pImage->data[y*3*pImage->width+x*3+1];  // G
			tmpData[j*3*width+i*3+2] = pImage->data[y*3*pImage->width+x*3+2];  // B
		}
	}
	// �õ�λͼ�³ߴ�
	pImage->width = width;
	pImage->height = height;

	// �ͷ�ԭλͼ���ݣ���ָ�����ź��µ�λͼ����
	free(pImage->data);
	pImage->data = tmpData;
}
