#ifndef	__EXPORT__
#define __EXPORT__

#include "MaxBIM.hpp"


// ���� ����� ����
struct ColumnInfo
{
	short	floorInd;	// �� �ε���
	double	posX;		// �߽��� X��ǥ
	double	posY;		// �߽��� Y��ǥ
	double	horLen;		// ���� ����
	double	verLen;		// ���� ����
	double	height;		// ����

	short	iHor;		// ���� ����
	short	iVer;		// ���� ����
};

// ����� ��ġ ����
struct ColumnPos
{
	ColumnInfo	columns [5000];		// ��� ����
	short	nColumns;				// ��� ����

	double	node_hor [100][100];	// ���� ���� ���� ��� ��ǥ (X) : dim1 - ��, dim2 - ��� X��ǥ
	short	nNodes_hor [100];		// ���� ���� ���� ��� ����
	double	node_ver [100][100];	// ���� ���� ���� ��� ��ǥ (Y) : dim1 - ��, dim2 - ��� Y��ǥ
	short	nNodes_ver [100];		// ���� ���� ���� ��� ����

	short	nStories;				// �� ��
	short	firstStory;				// ������
	short	lastStory;				// �ֻ���
};

void initArray (double arr [], short arrSize);			// �迭 �ʱ�ȭ �Լ�
int compare (const void* first, const void* second);	// ������������ ������ �� ����ϴ� ���Լ�
GSErrCode	exportElementInfo (void);					// ����(���,��,������)���� ������ �����ϰ� �����ؼ� ���� ���Ϸ� ��������

#endif