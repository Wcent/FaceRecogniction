// Image.h��λͼ�ṹ���壬��λͼ��ش���������
// Copyright: cent
// 2015.9.17
// ~

#ifndef IMAGE_H
#define IMAGE_H



extern HWND hMainWnd; // handle of main window
extern HDC hWinDC;


// 24λ���ɫλͼ�ṹ����
// data����̬�����ڴ���BMP��ʵ����RBG����
// width & height�����BMP��ʵ�ߴ�
typedef struct _BmpImage
{
	char *data;
	int width;
	int height;
} BmpImage;


// ��̬����Ŀ��λͼ�ڴ溯��
BmpImage * MallocBmpImage(int width, int height);

// ���ն�̬����λͼ�ڴ溯��
void FreeBmpImage(BmpImage *pImage);

// ��ȡѡ��Ŀ���ļ���
void OpenImageFile(char *OPDLTitle, char *pImgFileName);

// ��ȡbmpͼ���ļ�
BOOL ReadBmpFile(LPSTR imgFileName, BmpImage *oImage);

// ��ʾbmpͼ��
void ShowBmpImage(BmpImage *pImage, int xPos, int yPos);

// ��ʾbmp�Ҷ�ͼ��
void ShowBmpGreyImage(BmpImage *pImage, int xPos, int yPos);

// ���ͼ����ʾ
void CleanUpShowImage(int width, int height, int xPos, int yPos);

// ͼ���һ�������ŵ�ָ����С
void NormalizeImageSize( BmpImage *pImage, int width, int height);



#endif

