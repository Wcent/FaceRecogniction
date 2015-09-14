// Image.h

#ifndef IMAGE_H
#define IMAGE_H



extern HWND hMainWnd; // handle of main window
extern HDC hWinDC;

#define MAX_IMAGE_WIDTH	    320
#define MAX_IMAGE_HEIGHT	240


// 24λ���ɫλͼ�ṹ����
// image[]�����BMP��ʵ����RBG���ݣ����ɷţ�MAX_IMAGE_WIDTH * MAX_IMAGE_HEIGHT * 3���ֽ�
// width & height�����BMP��ʵ�ߴ�
typedef struct _BmpImage
{
	char image[MAX_IMAGE_WIDTH*MAX_IMAGE_HEIGHT*3];
	int width;
	int height;
} BmpImage;



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

// ͼ��ߴ����Ź�һ�����������ٽ���ֵ�㷨��������
void NormalizeImageSize(char *pDstImageData, int dstWidth, int dstHeight,
						char *pSrcImageData, int srcWidth, int srcHeight);

// ͼ���һ�������ŵ�ָ����С
void NormalizeImageSize( BmpImage *pDstImage, BmpImage *pSrcImage, int width, int height);

// ʶ�������������ҳ�Ŀ������
bool RecognizeFace(BmpImage *pImage, char *facebasePath);

// �õ�������ɫcbcr[cb][cr]�Աȿ�
bool FaceCbcrProc(char *sampleImagePath);

// ��ȡ����
bool ExtractFace(BmpImage *pFaceImage, BmpImage *pImage);

// ��Ƭ¼��������
bool EnterFace(char *imgFileName, char *facebasePath);

// �Ƴ�����ȡ��
void DeleteFace(char *facebasePath);


#endif

