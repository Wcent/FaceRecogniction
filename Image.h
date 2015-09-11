// Image.h

#ifndef IMAGE_H
#define IMAGE_H



extern HWND hMainWnd; // handle of main window
extern HDC hWinDC;
extern char Image[320*240*3];
extern char NewImage[320*240*3];
extern int ImageWidth;
extern int ImageHeight;
extern int FaceWidth;
extern int FaceHeight;
extern char curDir[1024];

#define XPOS		    	20
#define YPOS		    	20
#define MAX_IMAGE_WIDTH	    320
#define MAX_IMAGE_HEIGHT	240


// 24位真彩色位图结构定义
// image[]：存放BMP真实像素RBG数据，最大可放（MAX_IMAGE_WIDTH * MAX_IMAGE_HEIGHT * 3）字节
// width & height：存放BMP真实尺寸
typedef struct _BmpImage
{
	char image[MAX_IMAGE_WIDTH*MAX_IMAGE_HEIGHT*3];
	int width;
	int height;
} BmpImage;



// 获取选择目标文件名
void OpenImageFile(char *OPDLTitle, char *pImgFileName);

// 读取bmp图像文件
BOOL ReadBmpFile(LPSTR imgFileName, BmpImage *oImage);

// 显示bmp图像
void ShowBmpImage(char *, int, int, int, int);

// 显示bmp灰度图像
void ShowBmpGreyImage(char *, int, int, int, int t);

// 清除图像显示
void CleanUpShowImage(int, int, int, int);



// 图像归一化，缩放到指定大小
void NormalizeImageSize(char *, int, int, char *, int, int);

// RGB色彩空间模型转换到YCbCr空间模型
void RgbToYcbcr(char *, char *, int , int );

// 图片膨胀处理
void Expand(char *, int, int );

// 图片腐蚀处理
void Erode(char *, int, int);

// 过滤掉小块人脸候选区域背景噪音
void FilterNoise(char *, int, int);



// 提取灰度图像纹理的LBP特征，统计LBP特征直方图表征人脸
void ExtractImageLbp(char *, int, int, int *);

// 卡方统计两图像LBP相似度
int ChiSquareStatistic(int *, int *, int);

// 查找人脸库中是否存在目标人脸
bool SearchFace(char *, int *);

// 识别人脸，库中找出目标人脸
bool RecognizeFace(char *, int, int, char *);

// 照片录入人脸库
bool EnterFace(char *, char *);

// 移除人脸取样
void DeleteFace();



// 得到人脸肤色cbcr[cb][cr]对比库
bool FaceCbcrProc();

// 处理FaceSample.bmp人脸肤色样本库
void FaceSampleCbcr(char *, int, int);

// 根据cb,cr肤色范围得到人脸肤色对比库
void FaceCbcr();

// 从YCbCr空间模型图像中检测出人脸
void FaceDetect(char *, int , int);

// 投影法分割人脸位置，投影肤色计算下标像素数
bool CountFacePixel(char *, int, int, char *, int &, int &);

// 提取人脸
bool ExtractFace(char *, int, int, int &, int &);



#endif

