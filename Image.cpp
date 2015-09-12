// Image.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "resource.h"
#include <commdlg.h>
#include <direct.h>
#include <math.h>
#include "image.h"

HDC hWinDC;
char Image[320*240*3];
char NewImage[320*240*3];
int ImageWidth;
int ImageHeight;
int FaceWidth;
int FaceHeight;
int cbcr[320][240];
int ci[320];
int cj[240];
int LBP[49*59];
int dstLBP[49*59];


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
*   ��ȡ24λ���ɫλͼ����RGB���ݺ���														  *	
*   ������������λͼwidth*3�ֽ�����4������0�������ݣ��õ�bmp��RGB��ʵͼ������				  *
*																							  *	
*   ���������pImageData - 24λ���ɫԭʼλͼ�ļ��зֽ��������������                         *
*             width      - λͼ���						    							  	  *
*             height     - λͼ�߶�															  *
*																							  *
**********************************************************************************************/
void ExtractImageData(char *pImageData, int width, int height)
{
	int i, j;
	int cntZeros;
	int cntPixels;
	
	// ÿ�������ֽ���Ϊ��4��������0����ÿ�������ֽ�����4�ı���
	if ( !(width*3 % 4) )
		return ;
	
	// ����0���ֽ���
	cntZeros = 4 - width * 3 % 4;
	
	// ��������ָʾ��
	cntPixels = 0;
	for ( j=0; j<height; j++ )
	{
		for ( i=0; i<width; i++ )
		{
			// λͼ��ʵ�������ݱ���
			pImageData[j*width*3+i*3]   = pImageData[cntPixels++];
			pImageData[j*width*3+i*3+1] = pImageData[cntPixels++];
			pImageData[j*width*3+i*3+2] = pImageData[cntPixels++];
		}
		// ÿ�ж���cntZeros���ֽڵĲ���4��������Ч��������
		for ( i=0; i<cntZeros; i++ )
			cntPixels++;
	}
}

/************************************************************************************************
*																								*
*   ��ȡ24λ���ɫλͼ�������ݺ�����ÿ��������3���ֽ�RGB��ʾ                                    *
*   ��ͼ���ļ�������λͼ�ļ��ṹ���ֽ��λͼ�ļ�ͷ��λͼ��Ϣͷ����������						*
*   ���������imgFileName - �ļ���                                                              *
*   ���������oImage      - ���24λ���ɫλͼ����RGB���ݼ�ͼ��ߴ�ṹָ��                     *
*																								*
************************************************************************************************/
BOOL ReadBmpFile(LPSTR imgFileName, BmpImage *oImage)
{
	OFSTRUCT of;
	HFILE Image_fp;
	BITMAPFILEHEADER bfImage;
	BITMAPINFOHEADER biImage;
	int i, j;
	char tmpPixel;
	char *pTmpImgData;

	
	Image_fp = OpenFile(imgFileName, &of, OF_READ);
	if (Image_fp == HFILE_ERROR) 
	{
		MessageBox(NULL, "�򿪶�bmpͼ���ļ�����", "���ļ�������Ϣ", MB_OK);
		return FALSE;
	}

	// ��λ����ȡλͼ�ļ�ͷ
	_llseek(Image_fp, 0, 0);
	_lread(Image_fp, &bfImage, sizeof(BITMAPFILEHEADER));

	// "BM" (0x4d42)����ʾλͼBMP
	if ((bfImage.bfType != 0x4d42)) 
	{
		MessageBox(NULL, "��bmp�ļ�", "��bmp�ļ�������Ϣ", MB_OK);
		return FALSE;
	}

	// ��ȡλͼ��Ϣͷ
	_lread(Image_fp, &biImage, sizeof(BITMAPINFOHEADER));

	if (biImage.biBitCount != 24)
	{
		MessageBox(NULL, "��24λ���ɫλͼ", "��ȡbmp�ļ�������Ϣ", MB_OK);
		return FALSE;
	}

	// ������ʱ�洢�������ݿռ�
	pTmpImgData = (char *)malloc(biImage.biSizeImage);
	if( pTmpImgData == NULL )
	{
		MessageBox(NULL, "ϵͳû���㹻�ڴ��Է�����ʱ�洢�������ݿռ�", "��ȡbmp�ļ�������Ϣ", MB_OK);
		return FALSE;
	}

	// ��λ����ȡλͼ������������
	_llseek(Image_fp, bfImage.bfOffBits, 0);
	_lread(Image_fp, pTmpImgData, biImage.biSizeImage);

	// ����bmp���������в���ÿ�������ֽ���4������Ч����0����RGB��ʵ��������
	ExtractImageData(pTmpImgData, biImage.biWidth, biImage.biHeight);
	
	_lclose(Image_fp);

	//�޶����ͼ��ߴ�
	oImage->width = 320;
	oImage->height = 240;
	//ͼ�����Ź�һ��
	NormalizeImageSize(oImage, oImage.width, oImage.height, 
						pTmpImgData, biImage.biWidth, biImage.biHeight);
						
	// �ͷ�����������ʱ�洢�ռ�
	free(pTmpImgData);

	// bmpͼƬ���ش��µ��ϣ������ң���תΪ���ϵ��£�������
	// 24λ���ɫλͼÿ��������3���ֽ�R��G��B����ʾ
	for (i=0; i<oImage->height/2; i++)
	{
		for (j=0; j<oImage->width; j++)
		{
			// R
			tmpPixel = oImage->image[i*oImage->width*3+j*3];
			oImage->image[i*oImage->width*3+j*3] = oImage->image[(oImage->height-i-1)*oImage->width*3+j*3+2];
			oImage->image[(oImage->height-i-1)*oImage->width*3+j*3+2] = tmpPixel;
			// G
			tmpPixel = oImage->image[i*oImage->width*3+j*3+1];
			oImage->image[i*oImage->width*3+j*3+1] = oImage->image[(oImage->height-i-1)*oImage->width*3+j*3+1];
			oImage->image[(oImage->height-i-1)*oImage->width*3+j*3+1] = tmpPixel;
			// B
			tmpPixel = oImage->image[i*oImage->width*3+j*3+2];
			oImage->image[i*oImage->width*3+j*3+2] = oImage->image[(oImage->height-i-1)*oImage->width*3+j*3];
			oImage->image[(oImage->height-i-1)*oImage->width*3+j*3] = tmpPixel;
		}
	}

	return TRUE;
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
			r = (BYTE) pImage->image[i*pImage->width*3+j*3]; 
			g = (BYTE) pImage->image[i*pImage->width*3+j*3+1];
			b = (BYTE) pImage->image[i*pImage->width*3+j*3+2];
			
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
			y = (BYTE)pImage->image[i*pImage->width*3+j*3];
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
*   ���������pSrcImage - ԭʼλͼ�ṹָ��														*
*                 width - ���ź���															*
*	             height - ���ź�߶�															*
*   ���������pDstImage - Ŀ��λͼ�ṹָ��														*
*																								*
************************************************************************************************/
void NormalizeImageSize( BmpImage *pDstImage, BmpImage *pSrcImage, int width, int height)
{
	int i, j;
	int x, y;
	double xRate, yRate;

	// ���ű���
	xRate = (double)pSrcImage->width/width;
	yRate = (double)pSrcImage->height/height;
	for ( j=0; j<height; j++ )
	{
		// ӳ�䵽ԭpSrcImage->height�±�
		y = (int)(j*yRate+0.5);
		if ( y<0 )
			y = 0;
		else if ( y>=pSrcImage->height )
			y = pSrcImage->height-1;

		for ( i=0; i<width; i++ )
		{
			// ӳ�䵽ԭpSrcImage->width�±�
			x = (int)(i*xRate+0.5);
			if ( x<0 )
				x = 0;
			else if ( x>=pSrcImage->width )
				x = pSrcImage->width-1;

			pDstImage->image[j*3*width+i*3+0] = pSrcImage->image[y*3*pSrcImage->width+x*3+0];
			pDstImage->image[j*3*width+i*3+1] = pSrcImage->image[y*3*pSrcImage->width+x*3+1];
			pDstImage->image[j*3*width+i*3+2] = pSrcImage->image[y*3*pSrcImage->width+x*3+2];
		}
	}
	// �õ�λͼ�³ߴ�
	pDstImage->width = width;
	pDstImage->height = height;
}

/****************************************************************************************************
*																								    *	
*   ��λͼRGBɫ�ʿռ�ģ��ת����YCbCrɫ�ʿռ�ģ�ͺ���												*
*																								    *
*   ���������pSrcImage - RGBɫ�ʿռ�ģ�͵�ԭʼλͼ�ṹָ��										    *
*   ���������pDstImage - YCbCrɫ�ʿռ�ģ�͵�Ŀ��λͼ�ṹָ��									    *
*																									*
****************************************************************************************************/
void RgbToYcbcr(BmpImage *pDstImage, BmpImage *pSrcImage)
{
	int i, j;
	int r, g, b;
	int y, cr, cb;

	// �õ�λͼ�ߴ�
	pDstImage->width = pSrcImage->width;
	pDstImage->height = pSrcImage->height;

	// ɫ�ʿռ�ģ��ת��
	for ( j=0; j<pSrcImage->height; j++ )
	{
		for ( i=0; i<pSrcImage->width; i++ )
		{
			r = (BYTE)pSrcImage->image[j*pSrcImage->width*3+i*3];
			g = (BYTE)pSrcImage->image[j*pSrcImage->width*3+i*3+1];
			b = (BYTE)pSrcImage->image[j*pSrcImage->width*3+i*3+2];
			
			/*****************************************************************\
			  RGB to YCbCr:
			| Y  |    | 16  |             |   65.738  129.057   25.06 |   | R |
			| Cb |  = | 128 | + (1/256) * |  -37.945  -74.494  112.43 | * | G |
			| Cr |    | 128 |             |  112.439  -94.154  -18.28 |   | B |
			
			\*****************************************************************/
			y  = (int)(16 + (65.738*r + 129.057*g + 25.06*b) / 256);
			cb = (int)(128 + (-37.945*r - 74.494*g + 112.43*b) / 256);
			cr = (int)(128 + (112.439*r - 94.154*g - 18.28*b) / 256);
			
			pDstImage->image[j*pDstImage->width*3+i*3] = y;
			pDstImage->image[j*pDstImage->width*3+i*3+1] = cb;
			pDstImage->image[j*pDstImage->width*3+i*3+2] = cr;
		}
	}
}

/*****************************************************************************************
*																						 *
*   ������������ɫλͼFaceSample.bmp��ȡ������ɫcbcr[cb][cr]�Աȿ⺯��					 *
*																				 	     *
*   ���������pImage   - YCbCrɫ�ʿռ�ģ�͵�����������ɫλͼ�ṹָ��					 *
*   ���������cbcr[][] - ������ɫCbCr��Χͳ�ƿ�                							 *
*																						 *		
*****************************************************************************************/
void FaceSampleCbcr(BmpImage *pImage)
{
	int i, j;
	int y, cb, cr;

	for ( j=0; j<pImage->height; j++ )
	{
		for ( i=0; i<pImage->width; i++ )
		{
			y  = (BYTE)pImage->image[j*pImage->width*3+i*3];
			cb = (BYTE)pImage->image[j*pImage->width*3+i*3+1];
			cr = (BYTE)pImage->image[j*pImage->width*3+i*3+2];

			// �����ɫ��Χ����Ƕ�Ӧcb��crֵ
			if ( cb>98 && cb<123 && cr>133 && cr<169 )
			    cbcr[cb][cr] = 1;
		}
	}
}

/****************************************************************************************
*																						*
*	ֱ���ɷ�ɫcb,cr��Χ�õ�������ɫ��cbcr[cb][cr]�Աȿ⺯��								*
*																						*
*   ���������cbcr[][] - ������ɫCbCr��Χͳ�ƿ�                							*
*																						*
****************************************************************************************/
void FaceCbcr()
{
	int i, j;

	// ��ɫ��Χ��98<cb<123 && 133<cr<169
	// �������������ɫ��Ӧcb��crֵ
	for ( i=98; i<123; i++ )
		for ( j=133; j<169; j++ )
			cbcr[i][j] = 1;
}

/****************************************************************************************
*																						*
*	��ȡ������ɫ��cbcr[cb][cr]�Աȿ⺯��												*
*   ����������ɫλͼFaceSample.bmp����ʱ����������ȡ��������ʱ��ֱ���ɷ�ɫ��Χȡ��      *
*																						*
*   ���������cbcr[][] - ������ɫCbCr��Χͳ�ƿ�                							*
*																						*
****************************************************************************************/
bool FaceCbcrProc()
{
	OFSTRUCT of;
	char sampleImagePath[1024];
	BmpImage image;

	strcpy(sampleImagePath, curDir);
	strcat(sampleImagePath, "\\FaceSample.bmp");		

	// ���Դ��ļ����ж��ļ��Ƿ����
	// ���ظ����ļ������ڣ�����ʱ���ر��ļ�����������Ч���
	if ( OpenFile(sampleImagePath, &of, OF_EXIST) < 0 )
	{
		// δ�ҵ���ɫ����ͼʱ��ֱ����Cb��Cr��ɫ��Χ�õ�������ɫ�Աȿ�
		FaceCbcr();
		return true;
	}

	// ��FaceSample.bmp��ɫ����ͼ
	if ( !ReadBmpFile(sampleImagePath, &image) )
		return false;

	RgbToYcbcr(&image, &image);
	// ��������ɫCb��Cr�õ�������ɫ�Աȿ�
	FaceSampleCbcr(&image);

	return true;
}

/*****************************************************************************************
*																						 *
*	YCbCr�ռ����������ѡ��������λͼ�з�ɫ����Ҷ�ֵ���255����ɫ��	    		 *
*																						 *
*   �������������pImage - YCbCrɫ�ʿռ�ģ�͵�λͼ�ṹָ��     							 *
*																						 *
*****************************************************************************************/
void FaceDetect(BmpImage *pImage)
{
	int i, j;
	int y, cb, cr;

	for ( j=0; j<pImage->height; j++ )
	{
		for ( i=0; i<pImage->width; i++ )
		{
			y = (BYTE)pImage->image[j*pImage->width*3+i*3];
			cb = (BYTE)pImage->image[j*pImage->width*3+i*3+1];
			cr = (BYTE)pImage->image[j*pImage->width*3+i*3+2];

			// ��ɫ��Χ
		//	if ( cb>98 && cb<123 && cr>133 && cr<169 )

			// ��Ƿ�ɫ����
			if ( cbcr[cb][cr] == 1 )
				y = 255; //white
			else
				y = 0; //black

			pImage->image[j*pImage->width*3+i*3] = y;
			pImage->image[j*pImage->width*3+i*3+1] = cb;
			pImage->image[j*pImage->width*3+i*3+2] = cr;
		}
	}
}

/*****************************************************************************************
*																						 *
*	ͶӰ���ָ�����λ�ú���                                                               *
*   ��ɫ����ͶӰ��x��y���ϣ�ͳ�Ƶ���Ӧ�±��ɫ���ص���									 *
*																						 *
*   ���������pImage     - ��ɫ����Ҷ�ֵ���255��YCbCrλͼ�ṹָ��						 *
*   ���������pFaceImage - ����λͼ�ṹָ��                       						 *
*																						 *
*****************************************************************************************/
bool CountFacePixel(BmpImage *pFaceImage, BmpImage *pImage)
{
	int i, j;
	int y;
	int widthStart, widthEnd, heightStart, heightEnd;

	// ��ʼ�����������Ϸ�ɫ����ͳ��ֵ
	memset(ci, 0, sizeof(ci));
	memset(cj, 0, sizeof(cj));

	for ( j=0; j<pImage->height; j++ )
	{
		for ( i=0; i<pImage->width; i++ )
		{
			// ȡYCbCrλͼ�Ҷ�ֵ
			y = (BYTE)pImage->image[j*pImage->width*3+i*3];

			// ������ɫ����ͶӰ������������ͳ��
			if ( y == 255 )
			{
				ci[i]++;
				cj[j]++;
			}
		}
	}

	// �õ�x��������ʼ�±�
	for ( widthStart=0; widthStart<pImage->width; widthStart++ )
	{
		if ( ci[widthStart] > 0 )
			break;
	}
	// �õ�x�����������±�
	for ( widthEnd=pImage->width-1; widthEnd>=0; widthEnd-- )
	{
		if ( ci[widthEnd] > 0 )
			break;
	}

	// �õ�y��������ʼ�±�
	for ( heightStart=0; heightStart<pImage->height; height++ )
	{
		if ( cj[height] > 0 )
			break;
	}

	// �õ�y�����������±�
	for ( heightEnd=pImage->height-1; heightEnd>=0; heightEnd-- )
	{
		if ( cj[heightEnd] > 0 )
		{
			double rate;
		
			// �����������
			if ( widthEnd - widthStart < 0 )
				rate = 1.0*(heightEnd-heightStart)/(widthEnd-widthStart);
			else
				return false;

			// ����������������ձ鷶Χ�ڣ����ʵ�����
			if ( rate < 0.5 && rate > 1.3 )
				heightEnd = heightStart+(int)(1.2*(widthEnd-widthStart));
			break;
		}
	}

	// ������Χ�Ƿ�Ƿ�
	if ( !(widthStart>=0 && widthStart<widthEnd && heightStart>=0 && heightStart<heightEnd) )
		return false;
/*
	// �����ָ������top/bottom width
	for ( i=widthStart; i<widthEnd; i++ )
	{
	SetPixel(hWinDC, 680+i, 20+heightStart, RGB(255, 0, 0));
	SetPixel(hWinDC, 680+i, 20+heightEnd, RGB(255, 0, 0));
	}
	// �����ָ������left/right height
	for ( i=heightStart; i<heightEnd; i++ )
	{
	SetPixel(hWinDC, 680+widthStart, 20+i, RGB(255, 0, 0));
	SetPixel(hWinDC, 680+widthEnd, 20+i, RGB(255, 0, 0));
	}
*/
	// �õ������ߴ�
	pFaceImage->width = widthEnd - widthStart;
	pFaceImage->height = heightEnd - heightStart;

	// �ָ��������
	for ( j=heightStart; j<heightEnd; j++ )
	{
		for ( i=widthStart; i<widthEnd; i++ )
		{
			pFaceImage->image[(j-heightStart)*(widthEnd-widthStart)*3+(i-widthStart)*3] 
				= pImage->image[j*pImage->width*3+i*3];
			pFaceImage->image[(j-heightStart)*(widthEnd-widthStart)*3+(i-widthStart)*3+1]
				= pImage->image[j*pImage->width*3+i*3+1];
			pFaceImage->image[(j-heightStart)*(widthEnd-widthStart)*3+(i-widthStart)*3+2]
				= pImage->image[j*pImage->width*3+i*3+2];
		}
	}
	return true;
}

/***********************************************************************************
*																				   *
*	��ȡimage(width, height)��(m,n)block��(i,j)pixel����������LBPֵ				   *
*	count��LBPģʽ��ֵ������0->1��1->0�仯������								   *
*	count<=2��ULBPģʽ															   *
*	r���뾶Ϊr�Ļ�������														   *
*	p����������Բ���ϵ����ص���													   *
*																				   *
***********************************************************************************/
BYTE PixelLbp(char *image, int width, int height, 
			  int m, int n, int bw, int bh, int i, int j)
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
	coff = (BYTE)image[(bh*n+j)*width*3+(bw*m+i)*3];
	// �뻷��Բ���ϵ�8���������ص�Ƚϣ��ö�ֵ���У�LBPģʽ��
	for ( k=0; k<p; k++ )
	{
		// ���ȷֲ��ڻ���Բ���ϵ����ص���block�е��±�
		x = (int)(i + r*sin(2*pi*k/p) + 0.5);
		y = (int)(j - r*cos(2*pi*k/p) + 0.5);
		// �����±��block����ϵת����image����ϵ
		x += bw * m;
		y += bh * n;
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

/********************************************************************************************
*																							*
*    ��ת������LBP��ѭ����λ�õ�ǰLBP����ģʽ��ȡ��СֵLBP									*
*    n : lbpģʽ������λ����ģʽλ��														*
*																							*
********************************************************************************************/
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

/******************************************************************************************
*																						  *
*	��ȡ�Ҷ�ͼ�������LBP������ͳ��LBP����ֱ��ͼ��������								  *
*																						  *
******************************************************************************************/
void ExtractImageLbp(char *image, int width, int height, int *LBP )
{
	int i, j, k;
	int n, m, blkCount;
	int bw, bh;
	BYTE lbp;
	int ULBPtable[256];
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
		// uniform LBP ����ӳ���
		if ( count <= 2 )
			ULBPtable[i] = k++;
	}

	// keepԭimage��ʱ�ռ�
	tmpImage = (char *)malloc(width*height*3);
	// copy image��Ϣ����ʱ�ռ�
	memcpy(tmpImage, image, width*height*3);
	
	// image�ֿ���ȡLBP��blkCount*blkCount blocks
	blkCount = 7;
	bw = width / blkCount;
	bh = height / blkCount;
	memset(LBP, 0, 59*blkCount*blkCount*sizeof(int));
	for ( n=0; n<blkCount; n++ )
	{
		for ( m=0; m<blkCount; m++ )
		{
			// �ֿ���ȡLBP
			for ( j=0; j<bh; j++ )
			{
				for ( i=0; i<bw; i++ )
				{
					// ��ȡimage(blkCount*blkCount)�ֿ���block(m,n)��pixel(i,j)��LBP
					lbp = PixelLbp(tmpImage, width, height, m, n, bw, bh, i, j);
					// ��ת������LBP
			//		lbp = MinLbp(lbp, 8);
					// LBP��ֵ���У�LBPģʽ����Ӧ��10��������LBPֵ
					image[(bh*n+j)*width*3+(bw*m+i)*3] = lbp;

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

/****************************************************************************************
*																						*
*	����ͳ������ͼ��LBPֱ��ͼ���������ƶ�												*
*	��^2 (S,M) = �� { (Si-Mi)^2 / (Si+Mi) }												*
*	     		 i																		*
****************************************************************************************/
int ChiSquareStatistic(int *dstLBP, int *baseLBP, int len)
{
	int i, j;
	int w[49];
	double x2;

	// allocate weights for chi square statistic of LBPs
	for ( i=0; i<49; i++ )
		w[i] = 1;
	// weight 4 for eyes
	w[7*1+1] = w[7*1+2] = w[7*2+1] = w[7*2+2] = 4;
	w[7*1+4] = w[7*1+5] = w[7*2+4] = w[7*2+5] = 4;
	// weight 2 for mouth and ��ͷ����
	w[7*4+3] = 2;
	w[0] = w[7*1+0] = 2;
	w[6] = w[7*1+6] = 2;
	// weight 0 for nose and ���°�����
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

/********************************************************************************************
*																							*
*	����Ŀ¼facebaseDir�����ļ�������Ŀ¼													*
*	�����ȡ����.bmpͼ���ļ����õ�LBP����													*
*	Ŀ�����face��dstLBp��������ƶ����													*
*	���ݸ�����ֵ���ж��Ƿ�Ϊͬһ����������													*
*	�жϽ��																				*
*																							*
********************************************************************************************/
bool SearchFace(char *facebaseDir, int *dstLBP)
{
	char findFileName[256];
	char strFilter[1024];
	char findFilePath[1024];
	BmpImage image, faceImage;
	WIN32_FIND_DATA findFileData;
	HANDLE hFindFile;

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
		ReadBmpFile(findFilePath, &image);
		
		if ( !ExtractFace(image, ImageWidth, ImageHeight, FaceWidth, FaceHeight) )
			continue;
		// normalize face image to 70 70 size
		NormalizeImageSize(&faceImage, 70, 70, image, FaceWidth, FaceHeight);
		ShowBmpImage(&faceImage, 750, 20);
		RgbToYcbcr(faceImage, faceImage, 70, 70);
		ExtractImageLbp(faceImage, 70, 70, LBP);
		ShowBmpGreyImage(&faceImage, 750, 100);

		int x2;
		char strx2[50];
		// LBP����ͳ�����ƶ���ֵ���趨��ֵ�Աȣ�С��Ĭ����ֵʱ��ͼ����ͬ
		if ( (x2=ChiSquareStatistic(dstLBP, LBP, 49*59)) < 3000 )
		{
			wsprintf(strx2, "%s%d", "x^2 = ", x2);
			TextOut(hWinDC, 700, 180, strx2, strlen(strx2));
			ReadBmpFile(findFilePath, &image);
			ShowBmpImage(&image, 660, 200);
			
			FindClose(hFindFile);
			return true;
		}

	} while ( FindNextFile(hFindFile, &findFileData) );
	FindClose(hFindFile);

	// Update Show Rect()
	RECT rt;
	rt.left = 750, rt.top = 20;
	rt.right = 1000, rt.bottom = 200;
	InvalidateRect(hMainWnd, &rt, true);

	return false;
}


/******************************************************************************************************
*																									  *
*	���������в��ҳ����ƶ����ȽϽӽ�Ŀ��������ͼƬ													  *
*																									  *
******************************************************************************************************/
bool RecognizeFace(char *Image, int width, int height, char *facebasePath)
{
	if ( !ExtractFace(Image, width, height, FaceWidth, FaceHeight) )
		return false;

	// normalize face image to 70 70 size
	NormalizeImageSize(&faceImage, 70, 70, Image, FaceWidth, FaceHeight);
	ShowBmpImage(&faceImage, 70, 70, 660, 20);
	RgbToYcbcr(NewImage, NewImage, 70, 70);
	ExtractImageLbp(NewImage, 70, 70, dstLBP);
	ShowBmpGreyImage(&faceImage, 660, 100);

	return SearchFace(facebasePath, dstLBP);
}

/*****************************************************************************************************
*																									 *	
*	��ͼ�������ȡ����																			 *
*																									 *	
*****************************************************************************************************/
bool ExtractFace(char *image, int width, int height, int &fWidth, int &fHeight)
{
	// RGBɫ�ʿռ� --> YCbCr�ռ�ת��
	RgbToYcbcr(image, NewImage, width, height);
	// Ԥ����YCbCr�ռ�ͼ��õ�������ѡ����Ķ�ֵ��ͼ��
	FaceDetect(NewImage, width, height);
	
	// �����㴦���ȸ�ʴ������
	Erode(NewImage, width, height);
	Expand(NewImage, width, height);
	// ȥ��������������Ĭ��������С����ֵʱȥ��
	FilterNoise(NewImage, width, height);

	// ͶӰ��������������ɫ��width��height��λ��ͶӰ�±꣬�ָ�����
	return CountFacePixel(NewImage, width, height, image, fWidth, fHeight);
}

/************************************************************************************************
*																								*
*	�����񵽵�����ͼ��¼��������																*
*																								*
************************************************************************************************/
bool EnterFace(char *imgFileName, char *facebasePath)
{
	char fileName[1024];
	char strFilter[1024];
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

	int t=0;
	while (1)
	{
		// ʧ�ܴ�������1000����Ƭ¼��ʧ��
		if ( t++ >= 1000 )
			return false;

		wsprintf(fileName, "%s%s%d%s", facebasePath, "\\face", count++, ".bmp");
		if ( CopyFile(imgFileName, fileName, 1) )
			break;
	}

	return true;
}

void DeleteFace()
{
	char facePath[256];
	char fileTitle[100], imgDlgFileDir[256];
	char selImgFileDir[256];
	OPENFILENAME ofn;
	BmpImage image;

	// ָ��ѡ��Ի����������Ŀ¼
	strcpy(imgDlgFileDir, curDir);
	strcat(imgDlgFileDir, "\\Facebase");

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
	ofn.lpstrInitialDir=imgDlgFileDir;
	ofn.lpstrTitle="ѡ��Ҫ�Ƴ�����������ͼ��";
	ofn.Flags=OFN_FILEMUSTEXIST;
	ofn.lpstrDefExt="raw";
	ofn.lCustData=NULL;
	ofn.lpfnHook=NULL;
	ofn.lpTemplateName=NULL;
	facePath[0]='\0';
	GetOpenFileName(&ofn); 

	getcwd(selImgFileDir, MAX_PATH);

	// ·����ʱ��ȡ��
	if ( !strlen(facePath) )
		return ;

	TextOut(hWinDC, 660, 20, "��ѡ����Ƴ�������������", strlen("��ѡ����Ƴ�������������"));
	ReadBmpFile(facePath, &image);
	ShowBmpImage(&image, ImageWidth, ImageHeight, 660, 50);
	ExtractFace(image, ImageWidth, ImageHeight, FaceWidth, FaceHeight);
	// normalize face image to 70 70 size
	NormalizeImageSize(NewImage, 70, 70, image, FaceWidth, FaceHeight);
	ShowBmpImage(&NewImage, 70, 70, 730, ImageHeight+80);
	RgbToYcbcr(NewImage, NewImage, 70, 70);
	ExtractImageLbp(NewImage, 70, 70, dstLBP);
	ShowBmpGreyImage(&faceImage, 850, ImageHeight+80);

	if ( strcmp(selImgFileDir, imgDlgFileDir) )
	{
		MessageBox(hMainWnd, "�Ƴ�ʧ�ܣ�ѡ��Ĳ�������ȡ����Ŀ¼...\\Facebase�µ�ͼ��", "�Ƴ���������", 0);
		return ;
	}

	if ( MessageBox(hMainWnd, "ȷ���Ƴ���ѡ��������ͼ��", "�Ƴ���������", MB_OKCANCEL) == IDCANCEL )
	{
		// Update Show Rect()
		RECT rt;
		rt.left = 650, rt.top = 20;
		rt.right = 1000, rt.bottom = 500;
		InvalidateRect(hMainWnd, &rt, true);
		return ;
	}

	// Update Show Rect()
	RECT rt;
	rt.left = 650, rt.top = 20;
	rt.right = 1000, rt.bottom = 500;
	InvalidateRect(hMainWnd, &rt, true);

	if ( !DeleteFile(facePath) )
	{
		MessageBox(hMainWnd, "�Ƴ�Ŀ����������ͼ��ʧ�ܣ�", "�Ƴ���������", 0);
		return ;
	}
	MessageBox(hMainWnd, "�Ƴ�Ŀ�����������ɹ���", "�Ƴ���������", 0);
}

/***********************************************************************************************
*																							   *
*	ͼƬ���ʹ���																			   *
*																							   *
***********************************************************************************************/
void Expand(char *image, int width, int height)
{
	int i, j, m, n;
	int coff;
	int B[9] = {1, 0, 1,
			    0, 0, 0,
		        1, 0, 1};
	char *tmpImage;

	tmpImage = (char *)malloc(width*height*3);
	memcpy(tmpImage, image, width*height*3);

	for ( j=1; j<height-1; j++ )
	{
		for ( i=1; i<width-1; i++ )
		{
			for ( m=0; m<3; m++ )
			{
				for ( n=0; n<3; n++ )
				{
					if ( B[m+n] == 1 )
						continue;
					// top/bottom/left/right exist same pixel to target face, expand
					coff = (BYTE)image[(j+m-1)*width*3+(i+n-1)*3];
					if ( coff == 255 )
					{
						tmpImage[j*width*3+i*3] = (char)255;
						goto EXPAND_BREAK_I_LOOP;
					}
				}
			}
EXPAND_BREAK_I_LOOP: ;
		}
	}
	memcpy(image, tmpImage, width*height*3);
	free(tmpImage);
}

/**********************************************************************************************
*																							  *
*	ͼƬ��ʴ����																			  *
*																							  *	
**********************************************************************************************/
void Erode(char *image, int width, int height)
{
	
	int i, j, m, n;
	int coff;
	int B[9] = {1, 0, 1,
			    0, 0, 0,
				1, 0, 1};
	char *tmpImage;

	tmpImage = (char *)malloc(width*height*3);
	memcpy(tmpImage, image, width*height*3);

	for ( j=1; j<height-1; j++ )
	{
		for ( i=1; i<width-1; i++ )
		{
			for ( m=0; m<3; m++ )
			{
				for ( n=0; n<3; n++ )
				{
					if ( B[m+n] == 1 )
						continue;
					// top/bottom/left/right exist same pixel to  background, erode
					coff = (BYTE)image[(j+m-1)*width*3+(i+n-1)*3];
					if ( coff == 0 )
					{
						tmpImage[j*width*3+i*3] = (char)0;
						goto ERODE__BREAK_I_LOOP;
					}
				}
			}
ERODE__BREAK_I_LOOP: ;
		}
	}
	memcpy(image, tmpImage, width*height*3);
	free(tmpImage);
}

/*******************************************************************************************
*	���˵�С��������ѡ���򱳾�����														   *
*	��ɫ���ص���С��5000ʱ���ٶ�Ϊ����������											   *	
*	�ö��й�����������ķ�ʽ��ͳ�Ʒ�ɫ���ص���											   *	 	
*																						   *	
*******************************************************************************************/
void FilterNoise(char *image, int width, int height)
{
	int i, j, k;
	int y;
	// ��ɫ���ص��������
	int count;
	int m, n;
	// С���������ɫ�������Χ
	int iMin, iMax, jMin, jMax;
	// ������β�±�ָ��
	int tail, head;
	// ��ɫ���ص���ʶ���
	int iQue[320*240];
	int jQue[320*20];
	// ���ص���ʱ�־
	int flagVisited[320][240] = {0}; 
	// ������������8�����ص���±��ֵ��ϵ
	int a[8] = {0, 1, 1, 1, 0, -1, -1, -1};
	int b[8] = {-1, -1, 0, 1, 1, 1, 0, -1};

	for ( j=0; j<height; j++ )
	{
		for ( i=0; i<width; i++ )
		{
			y = (BYTE)image[j*width*3+i*3];
			// �ҵ�һ����ɫ���ص㣬��ʼ���Ѽ���
			if ( y == 255 && flagVisited[i][j] == 0 )
			{
				count = 0;
				iMin = width;
				jMin = height;
				iMax = 0;
				jMax = 0;
				// ���г�ʼ��
				head = 0;
				tail = 0;
				// ��ǰ��һ����ɫ���ص����
				iQue[tail] = i;
				jQue[tail++] = j;
				// ��Ƿ��ʼ�¼
				flagVisited[i][j] = 1;
				// ���пգ�
				while ( head < tail )
				{
					count++;
					// ���׳���
					m = iQue[head];
					n = jQue[head++];
					if ( m > iMax )
						iMax = m;
					if ( m < iMin )
						iMin = m;
					if ( n > jMax )
						jMax = n;
					if ( n < jMin )
						jMin = n;
					// ������8�����ص����
					for ( k=0; k<8; k++ )
					{
						// ����8�����±�λ��
						m = iQue[head-1] + a[k];
						n = jQue[head-1] + b[k];
						// ��ֹԽ��
						if ( m<0 || m>=width || n<0 || n>=height )
							continue;
						// �·��ʵ㣿
						if ( flagVisited[m][n] == 1 )
							continue;
						// ��ɫ���ص㣿
						y = (BYTE)image[n*width*3+m*3];
						if ( y != 255 )
							continue;
						// ��β����
						iQue[tail] = m;
						jQue[tail++] = n;
						// ��Ƿ��ʼ�¼
						flagVisited[m][n] = 1;
					}
				} // end while ( head < tail )

				// ����С���������ɫ����
				if ( count < 1000 )
				{
					for ( n=jMin; n<=jMax; n++ )
						for ( m=iMin; m<=iMax; m++ )
							image[n*width*3+m*3] = 0;
				}
			} // end if ( y == 255 && flagVisited[i][j] == 0 )
			// ���ʼ�¼
			else
				flagVisited[i][j] = 1;
		}
	}
}
