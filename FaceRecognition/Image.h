// Image.h
void OpenImageFile(char *);
BOOL ReadBmpFile(LPSTR, char *); //��ȡbmpͼ���ļ�
void ShowBmpImage(char *, int, int, int, int); //��ʾbmpͼ��
void ShowBmpGreyImage(char *, int, int, int, int t); //show bmp grey image
void CleanUpShowImage(int, int, int, int);

void NormalizeImageSize(char *, int, int, char *, int, int); // ͼ���һ��
void RgbToYcbcr(char *, char *, int , int ); //RGBɫ�ʿռ�ģ��ת����YCbCr�ռ�ģ��
void Expand(char *, int, int ); //ͼƬ���ʹ���
void Erode(char *, int, int); //ͼƬ��ʴ����
void FilterNoise(char *, int, int); //���˵�С��������ѡ���򱳾�����

void ExtractImageLbp(char *, int, int, int *); //��ȡ�Ҷ�ͼ�������LBP������ͳ��LBP����ֱ��ͼ��������
int ChiSquareStatistic(int *, int *, int); //����ͳ����ͼ��LBP���ƶ�
bool SearchFace(char *, int *); //�������������Ƿ����Ŀ������
bool RecognizeFace(char *, int, int, char *); //ʶ�������������ҳ�Ŀ������
bool EnterFace(char *, char *); //��Ƭ¼��������
void DeleteFace(); // �Ƴ�����ȡ��

bool FaceCbcrProc(); //�õ�������ɫcbcr[cb][cr]�Աȿ�
void FaceSampleCbcr(char *, int, int); //����FaceSample.bmp������ɫ������
void FaceCbcr(); // ����cb,cr��ɫ��Χ�õ�������ɫ�Աȿ�
void FaceDetect(char *, int , int); // detect face from YCbCr image
bool CountFacePixel(char *, int, int, char *, int &, int &); // ͶӰ���ָ�����λ�ã�ͶӰ��ɫ�����±�������
bool ExtractFace(char *, int, int, int &, int &); // ��ȡ����

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



/*					hWinDC = GetDC(hWnd);
					OpenImageFile("��ͼ���ļ�");
					ReadImage(ImgFileName, NewImage, 256, 256);	//��ͼ��ϵ��
					ShowImage(image, 256, 256, 100, 100);			//��ʾͼ��
*/