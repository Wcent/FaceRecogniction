// FaceRecognize.h����������ͼ�������LBP���ֲ���ֵģʽ��������������ʶ����غ�������
// Copyright: cent
// 2015.9.17
// ~

#ifndef FACERECOGNIZE_H
#define FACERECOGNIZE_H



// ʶ�������������ҳ�Ŀ������
bool RecognizeFace(BmpImage *pImage, char *facebasePath);

// ��Ƭ¼��������
bool EnterFace(char *imgFileName, char *facebasePath);

// �Ƴ�����ȡ��
void DeleteFace(char *facebasePath);



#endif