#ifndef	__EXPORT__
#define __EXPORT__

#include "MaxBIM.hpp"

// ����� ��ġ, ũ�� ����
struct ColumnInfo
{
	// [����][����][��]
	short	pos		[100][100][100];	// ����� ���ο�, ���ο�, ���� ��ġ
	double	horLen	[100][100][100];	// �ش� ��ġ ����� ���� ����
	double	verLen	[100][100][100];	// �ش� ��ġ ����� ���� ����
	double	height	[100][100][100];	// �ش� ��ġ ����� ����

	short	nHor;	// ���� ���� ��� ����
	short	nVer;	// ���� ���� ��� ����
	short	nHei;	// �����ϴ� �� ��
};

int compare (const void* first, const void* second);	// ������������ ������ �� ����ϴ� ���Լ�
GSErrCode	exportElementInfo (void);					// ����(���,��,������)���� ������ �����ϰ� �����ؼ� ���� ���Ϸ� ��������

#endif