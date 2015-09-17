// FaceDetect.cpp������������ɫ�������������غ�������
// Copyright: cent
// 2015.9.17
// ~
//

#include "stdafx.h"
#include "resource.h"
#include <commdlg.h>
#include <direct.h>
#include "image.h"
#include "FaceDetect.h"

int cbcr[256][256];    // ������ɫCbCr��Χͳ�ƿ�

/************************************************************************************
	��̬��������

************************************************************************************/
// ͼƬ���ʹ���
static void Expand(BmpImage *pImage);
// ͼƬ��ʴ����
static void Erode(BmpImage *pImage);
// ���˵�С��������ѡ���򱳾�����
static void FilterNoise(BmpImage *pImage);

// ����FaceSample.bmp������ɫ������
static void FaceSampleCbcr(BmpImage *pImage);
// ����cb,cr��ɫ��Χ�õ�������ɫ�Աȿ�
static void FaceCbcr();
// ��YCbCr�ռ�ģ��ͼ���м�������
static void FaceDetect(BmpImage *pImage);
// ͶӰ���ָ�����
static BmpImage* SplitFace(BmpImage *pImage, BmpImage *pYcbcrImage);



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
			r = (BYTE)pSrcImage->data[j*pSrcImage->width*3+i*3];
			g = (BYTE)pSrcImage->data[j*pSrcImage->width*3+i*3+1];
			b = (BYTE)pSrcImage->data[j*pSrcImage->width*3+i*3+2];
			
			/*****************************************************************\
			  RGB to YCbCr:
			| Y  |    | 16  |             |   65.738  129.057   25.06 |   | R |
			| Cb |  = | 128 | + (1/256) * |  -37.945  -74.494  112.43 | * | G |
			| Cr |    | 128 |             |  112.439  -94.154  -18.28 |   | B |
			
			\*****************************************************************/
			y  = (int)(16 + (65.738*r + 129.057*g + 25.06*b) / 256);
			cb = (int)(128 + (-37.945*r - 74.494*g + 112.43*b) / 256);
			cr = (int)(128 + (112.439*r - 94.154*g - 18.28*b) / 256);
			
			pDstImage->data[j*pDstImage->width*3+i*3] = y;
			pDstImage->data[j*pDstImage->width*3+i*3+1] = cb;
			pDstImage->data[j*pDstImage->width*3+i*3+2] = cr;
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
static void FaceSampleCbcr(BmpImage *pImage)
{
	int i, j;
	int y, cb, cr;

	for ( j=0; j<pImage->height; j++ )
	{
		for ( i=0; i<pImage->width; i++ )
		{
			y  = (BYTE)pImage->data[j*pImage->width*3+i*3];
			cb = (BYTE)pImage->data[j*pImage->width*3+i*3+1];
			cr = (BYTE)pImage->data[j*pImage->width*3+i*3+2];

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
static void FaceCbcr()
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
bool FaceCbcrProc(char *sampleImagePath)
{
	OFSTRUCT of;
	BmpImage *image;		

	// ���Դ��ļ����ж��ļ��Ƿ����
	// ���ظ����ļ������ڣ�����ʱ���ر��ļ�����������Ч���
	if ( OpenFile(sampleImagePath, &of, OF_EXIST) < 0 )
	{
		// δ�ҵ���ɫ����ͼʱ��ֱ����Cb��Cr��ɫ��Χ�õ�������ɫ�Աȿ�
		FaceCbcr();
		return true;
	}

	// ��FaceSample.bmp��ɫ����ͼ
	image = ReadBmpFile(sampleImagePath);
	if ( image == NULL )
		return false;

	RgbToYcbcr(image, image);
	// ��������ɫCb��Cr�õ�������ɫ�Աȿ�
	FaceSampleCbcr(image);
	
	// �ͷ�ReadBmpFile��̬���ɵ�λͼ�ռ�
	FreeBmpImage(image);

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
			y = (BYTE)pImage->data[j*pImage->width*3+i*3];
			cb = (BYTE)pImage->data[j*pImage->width*3+i*3+1];
			cr = (BYTE)pImage->data[j*pImage->width*3+i*3+2];

			// ��ɫ��Χ
		//	if ( cb>98 && cb<123 && cr>133 && cr<169 )

			// ��Ƿ�ɫ����
			if ( cbcr[cb][cr] == 1 )
				y = 255; //white
			else
				y = 0; //black

			pImage->data[j*pImage->width*3+i*3] = y;
			pImage->data[j*pImage->width*3+i*3+1] = cb;
			pImage->data[j*pImage->width*3+i*3+2] = cr;
		}
	}
}

/*****************************************************************************************
*																						 *
*	ͶӰ���ָ�����λ�ú���                                                               *
*   ��ɫ����ͶӰ��x��y���ϣ�ͳ�Ƶ���Ӧ�±��ɫ���ص���									 *
*																						 *
*   ���������pSrcImage   - ԭʼλͼ�ṹָ��						                     *
*   ���������pYcbcrImage - ��ɫ����Ҷ�ֵ���255��YCbCrλͼ�ṹָ��					 *
*   ����ֵ  ��              ָ������λͼ�ṹָ��                   						 *
*																						 *
*****************************************************************************************/
static BmpImage* SplitFace(BmpImage *pSrcImage, BmpImage *pYcbcrImage)
{
	int i, j;
	int y;
	int widthStart, widthEnd, heightStart, heightEnd;
	int *ci;
	int *cj;
	BmpImage *pFaceImage;

	ci = (int *)malloc(pYcbcrImage->width*sizeof(int));
	if( ci == NULL )
		return NULL;
		
	cj = (int *)malloc(pYcbcrImage->height*sizeof(int));
	if( cj == NULL )
	{
		free(ci);
		return NULL;
	}
	
	// ��ʼ�����������Ϸ�ɫ����ͳ��ֵ
	memset(ci, 0, pYcbcrImage->width*sizeof(int));
	memset(cj, 0, pYcbcrImage->height*sizeof(int));

	for ( j=0; j<pYcbcrImage->height; j++ )
	{
		for ( i=0; i<pYcbcrImage->width; i++ )
		{
			// ȡYCbCrλͼ�Ҷ�ֵ
			y = (BYTE)pYcbcrImage->data[j*pYcbcrImage->width*3+i*3];

			// ������ɫ����ͶӰ������������ͳ��
			if ( y == 255 )
			{
				ci[i]++;
				cj[j]++;
			}
		}
	}

	// �õ�x��������ʼ�±�
	for ( widthStart=0; widthStart < pYcbcrImage->width; widthStart++ )
	{
		if ( ci[widthStart] > 0 )
			break;
	}
	// �õ�x�����������±�
	for ( widthEnd=pYcbcrImage->width-1; widthEnd>=0; widthEnd-- )
	{
		if ( ci[widthEnd] > 0 )
			break;
	}

	// �õ�y��������ʼ�±�
	for ( heightStart=0; heightStart < pYcbcrImage->height; heightStart++ )
	{
		if ( cj[heightStart] > 0 )
			break;
	}

	// �õ�y�����������±�
	for ( heightEnd=pYcbcrImage->height-1; heightEnd>=0; heightEnd-- )
	{
		if ( cj[heightEnd] > 0 )
		{
			double rate;
		
			// �����������
			if ( widthEnd - widthStart > 0 )
				rate = 1.0*(heightEnd-heightStart)/(widthEnd-widthStart);
			else
			{
				free(ci);
				free(cj);
				return NULL;
			}

			// ����������������ձ鷶Χ�ڣ����ʵ�����
			if ( rate < 0.5 || rate > 1.3 )
				heightEnd = heightStart+(int)(1.2*(widthEnd-widthStart));
			break;
		}
	}

	// ������Χ�Ƿ�Ƿ�
	if ( !(widthStart>=0 && widthStart<widthEnd && 
	      heightStart>=0 && heightStart<heightEnd && heightEnd<pYcbcrImage->height) )
	{
		free(ci);
		free(cj);
		return NULL;
	}
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
	// ��������λͼ�ṹ���ߴ�Ϊ��(widthEnd-widthStart) * (heightEnd-heightStart)
	pFaceImage = MallocBmpImage(widthEnd-widthStart, heightEnd-heightStart);
	if(pFaceImage == NULL)
	{
		free(ci);
		free(cj);
		return NULL;
	}
		
	for ( j=heightStart; j<heightEnd; j++ )
	{
		for ( i=widthStart; i<widthEnd; i++ )
		{
			// �ָ��������
			pFaceImage->data[(j-heightStart)*(widthEnd-widthStart)*3+(i-widthStart)*3] 
				= pSrcImage->data[j*pSrcImage->width*3+i*3];
			pFaceImage->data[(j-heightStart)*(widthEnd-widthStart)*3+(i-widthStart)*3+1]
				= pSrcImage->data[j*pSrcImage->width*3+i*3+1];
			pFaceImage->data[(j-heightStart)*(widthEnd-widthStart)*3+(i-widthStart)*3+2]
				= pSrcImage->data[j*pSrcImage->width*3+i*3+2];
		}
	}
	
	free(ci);
	free(cj);
	return pFaceImage;
}

/*****************************************************************************************
*																						 *	
*	��ȡ����λͼ���� 																	 *
*																						 *
*   ���������pImage       - Ŀ��λͼ�ṹָ��   	               						 *
*   ����ֵ  ��               ָ��ߴ�Ϊ70*70������λͼָ��               				 *
*																						 *	
*****************************************************************************************/
BmpImage* ExtractFace(BmpImage *pImage)
{
	BmpImage *ycbcrImage, *pFaceImage;
		
	ycbcrImage = MallocBmpImage(pImage->width, pImage->height);
	if (ycbcrImage == NULL)
		return NULL;

	// RGBɫ�ʿռ� --> YCbCr�ռ�ת��
	RgbToYcbcr(ycbcrImage, pImage);
	// Ԥ����YCbCr�ռ�ͼ��õ�������ѡ����Ķ�ֵ��ͼ��
	FaceDetect(ycbcrImage);
	
	// �����㴦���ȸ�ʴ������
	Erode(ycbcrImage);
	Expand(ycbcrImage);

	// ȥ��������������Ĭ��������С����ֵʱȥ��
	FilterNoise(ycbcrImage);

	// �ָ�������ͶӰ��������ͶӰ��width��height�����ϣ��ó��±�λ��
	pFaceImage = SplitFace(pImage, ycbcrImage);
	if( pFaceImage == NULL )
		return NULL;
	
	// �����޶����ųߴ�Ϊ��70 * 70
	NormalizeImageSize(pFaceImage, 70, 70);
	
	FreeBmpImage(ycbcrImage);
	return pFaceImage;
}

/*****************************************************************************************
*																						 *
*	ͼƬ���ʹ�����																	 *
*																						 *
*   �������������pImage  - λͼ�ṹָ��                               					 *
*																						 *
*****************************************************************************************/
static void Expand(BmpImage *pImage)
{
	int i, j, m, n;
	int coff;
	int B[9] = {1, 0, 1,
			    0, 0, 0,
		        1, 0, 1};
	char *tmpImage;

	tmpImage = (char *)malloc(pImage->width*pImage->height*3);
	memcpy(tmpImage, pImage->data, pImage->width*pImage->height*3);

	for ( j=1; j<pImage->height-1; j++ )
	{
		for ( i=1; i<pImage->width-1; i++ )
		{
			// ���������д�����Ŀ��Ҷ�ֵ255һ�����أ�ͬ����ǰ���أ�������
			for ( m=0; m<3; m++ )
			{
				for ( n=0; n<3; n++ )
				{
					if ( B[m+n] == 1 )
						continue;
						
					coff = (BYTE)pImage->data[(j+m-1)*pImage->width*3+(i+n-1)*3];
					if ( coff == 255 )
					{
						tmpImage[j*pImage->width*3+i*3] = (char)255;
						// ����������һ������
						goto EXPAND_BREAK_I_LOOP;
					}
				}
			}
EXPAND_BREAK_I_LOOP: ;
		}
	}
	memcpy(pImage->data, tmpImage, pImage->width*pImage->height*3);
	free(tmpImage);
}

/*****************************************************************************************
*																						 *
*	ͼƬ��ʴ������																	 *
*																						 *
*   �������������pImage  - λͼ�ṹָ��                               					 *
*																						 *
*****************************************************************************************/
static void Erode(BmpImage *pImage)
{
	
	int i, j, m, n;
	int coff;
	int B[9] = {1, 0, 1,
			    0, 0, 0,
				1, 0, 1};
	char *tmpImage;

	tmpImage = (char *)malloc(pImage->width*pImage->height*3);
	memcpy(tmpImage, pImage->data, pImage->width*pImage->height*3);

	for ( j=1; j<pImage->height-1; j++ )
	{
		for ( i=1; i<pImage->width-1; i++ )
		{
			// ���������д����뱳���Ҷ�ֵ0һ�����أ�ͬ����ǰ���أ�����ʴ
			for ( m=0; m<3; m++ )
			{
				for ( n=0; n<3; n++ )
				{
					if ( B[m+n] == 1 )
						continue;
					
					coff = (BYTE)pImage->data[(j+m-1)*pImage->width*3+(i+n-1)*3];
					if ( coff == 0 )
					{
						tmpImage[j*pImage->width*3+i*3] = (char)0;
						// ����������һ������
						goto ERODE__BREAK_I_LOOP;
					}
				}
			}
ERODE__BREAK_I_LOOP: ;
		}
	}
	memcpy(pImage->data, tmpImage, pImage->width*pImage->height*3);
	free(tmpImage);
}

/*****************************************************************************************
*                                                                                        *
*	���˱�����������								                					 *
*	��ɫ���ص���С��5000ʱ���ٶ�Ϊ����������											 *	
*	�ö��й�����������ķ�ʽ��ͳ�Ʒ�ɫ���ص���											 *
*																						 *
*   �������������pImage  - λͼ�ṹָ��                               					 *
*																						 *	
*****************************************************************************************/
static void FilterNoise(BmpImage *pImage)
{
	int i, j, k;
	int y;
	int count;                    // ��ɫ���ص��������
	int m, n;
	int iMin, iMax, jMin, jMax;   // С���������ɫ�������Χ
	int tail, head;               // ������β�±�ָ��
	
	int *iQue;            // ��ɫ���ص���ʶ���
	int *jQue;
	char *flagVisited;  // ���ص���ʱ�־
	
	// ������������8�����ص���±��ֵ��ϵ
	int a[8] = {0, 1, 1, 1, 0, -1, -1, -1};
	int b[8] = {-1, -1, 0, 1, 1, 1, 0, -1};

	iQue = (int *)malloc(pImage->width*pImage->height*sizeof(int));
	if ( iQue == NULL )
		return ;

	jQue = (int *)malloc(pImage->width*pImage->height*sizeof(int));
	if ( jQue == NULL )
	{
		free(iQue);
		return ;
	}

	flagVisited = (char *)malloc(pImage->width*pImage->height);
	if ( flagVisited == NULL )
	{
		free(iQue);
		free(jQue);
		return ;
	}

	memset(flagVisited, '0', pImage->width*pImage->height);
	for ( j=0; j<pImage->height; j++ )
	{
		for ( i=0; i<pImage->width; i++ )
		{
			y = (BYTE)pImage->data[j*pImage->width*3+i*3];
			
			// �ҵ�һ����ɫ���ص㣬��ʼ���Ѽ���
			if ( y == 255 && flagVisited[j*pImage->width+i] == '0' )
			{
				count = 0;
				iMin = pImage->width;
				jMin = pImage->height;
				iMax = 0;
				jMax = 0;
				
				// ���г�ʼ��
				head = 0;
				tail = 0;
				
				// ��ǰ��һ����ɫ���ص����
				iQue[tail] = i;
				jQue[tail++] = j;
				// ��Ƿ��ʼ�¼
				flagVisited[j*pImage->width+i] = '1';
				
				// ѭ���������зǿ�
				while ( head < tail )
				{
					count++;
					// ���׳���
					m = iQue[head];
					n = jQue[head++];
					
					// �������Χ
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
						if ( m<0 || m>=pImage->width || n<0 || n>=pImage->height )
							continue;
							
						// �Ƿ��ѷ��ʹ�
						if ( flagVisited[n*pImage->width+m] == '1' )
							continue;
							
						// �Ƿ�Ϊ��ɫ����
						y = (BYTE)pImage->data[n*pImage->width*3+m*3];
						if ( y != 255 )
							continue;
							
						// ��β����
						iQue[tail] = m;
						jQue[tail++] = n;
						// ��Ƿ��ʼ�¼
						flagVisited[n*pImage->width+m] = '1';
					}
				} // end while ( head < tail )

				// ����С���������ɫ���򣬼ٶ�С��1000�����ص�
				if ( count < 1000 )
				{
					for ( n=jMin; n<=jMax; n++ )
						for ( m=iMin; m<=iMax; m++ )
							pImage->data[n*pImage->width*3+m*3] = 0;
				}
			} // end if ( y == 255 && flagVisited[i][j] == '0' )
		} // end for i
	} // end for j

	free(iQue);
	free(jQue);
	free(flagVisited);
}
