// Image.h

#ifndef IMAGE_H
#define IMAGE_H



extern HWND hMainWnd; // handle of main window
extern HDC hWinDC;
extern char ImgFileName[256];
extern char Image[320*240*3];
extern char NewImage[320*240*3];
extern int ImageWidth;
extern int ImageHeight;
extern int FaceWidth;
extern int FaceHeight;
extern char curDir[1024];

#define XPOS			20
#define YPOS			20
#define MAXIMAGEWIDTH	320
#define MAXIMAGEHEIGHT	240
#define IMAGEGAP		40


// 
typedef struct _BmpImage
{
	char image[MAXIMAGEWIDTH*MAXIMAGEHEIGHT*3];
	int width;
	int height;
} BmpImage;



/*
	hWinDC = GetDC(hWnd);
	OpenImageFile("��ͼ���ļ�");
	ReadImage(ImgFileName, NewImage, 256, 256);	//��ͼ��ϵ��
	ShowImage(image, 256, 256, 100, 100);			//��ʾͼ��
*/



// ��ȡѡ���ļ���
void OpenImageFile(char *);

// ��ȡbmpͼ���ļ�
BOOL ReadBmpFile(LPSTR, char *);

// ��ʾbmpͼ��
void ShowBmpImage(char *, int, int, int, int);

// ��ʾbmp�Ҷ�ͼ��
void ShowBmpGreyImage(char *, int, int, int, int t);

// ���ͼ����ʾ
void CleanUpShowImage(int, int, int, int);



// ͼ���һ�������ŵ�ָ����С
void NormalizeImageSize(char *, int, int, char *, int, int);

// RGBɫ�ʿռ�ģ��ת����YCbCr�ռ�ģ��
void RgbToYcbcr(char *, char *, int , int );

// ͼƬ���ʹ���
void Expand(char *, int, int );

// ͼƬ��ʴ����
void Erode(char *, int, int);

// ���˵�С��������ѡ���򱳾�����
void FilterNoise(char *, int, int);



// ��ȡ�Ҷ�ͼ�������LBP������ͳ��LBP����ֱ��ͼ��������
void ExtractImageLbp(char *, int, int, int *);

// ����ͳ����ͼ��LBP���ƶ�
int ChiSquareStatistic(int *, int *, int);

// �������������Ƿ����Ŀ������
bool SearchFace(char *, int *);

// ʶ�������������ҳ�Ŀ������
bool RecognizeFace(char *, int, int, char *);

// ��Ƭ¼��������
bool EnterFace(char *, char *);

// �Ƴ�����ȡ��
void DeleteFace();



// �õ�������ɫcbcr[cb][cr]�Աȿ�
bool FaceCbcrProc();

// ����FaceSample.bmp������ɫ������
void FaceSampleCbcr(char *, int, int);

// ����cb,cr��ɫ��Χ�õ�������ɫ�Աȿ�
void FaceCbcr();

// ��YCbCr�ռ�ģ��ͼ���м�������
void FaceDetect(char *, int , int);

// ͶӰ���ָ�����λ�ã�ͶӰ��ɫ�����±�������
bool CountFacePixel(char *, int, int, char *, int &, int &);

// ��ȡ����
bool ExtractFace(char *, int, int, int &, int &);



#endif

