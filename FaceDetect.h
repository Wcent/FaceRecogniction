// FaceDetect.h������������ɫ�������������غ�������
// Copyright: cent
// 2015.9.17
// ~

#ifndef FACEDETECT_H
#define FACEDETECT_H



// �õ�������ɫcbcr[cb][cr]�Աȿ�
bool FaceCbcrProc(char *sampleImagePath);

// RGBɫ�ʿռ�ģ��ת����YCbCr�ռ�ģ��
void RgbToYcbcr(BmpImage *pDstImage, BmpImage *pSrcImage);

// ��ȡ����
BmpImage* ExtractFace(BmpImage *pImage);



#endif