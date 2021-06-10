#ifndef	__WALL_TABLEFORM_PLACER_CONTINUE__
#define __WALL_TABLEFORM_PLACER_CONTINUE__

#include "MaxBIM.hpp"

namespace wallTableformPlacerContinueDG {
}

// ���� ���� ����
struct InfoMorphForWallTableformContinue
{
	API_Guid	guid;		// ������ GUID

	double	leftBottomX;	// ���ϴ� ��ǥ X
	double	leftBottomY;	// ���ϴ� ��ǥ Y
	double	leftBottomZ;	// ���ϴ� ��ǥ Z

	double	rightTopX;		// ���� ��ǥ X
	double	rightTopY;		// ���� ��ǥ Y
	double	rightTopZ;		// ���� ��ǥ Z

	double	horLen;			// ���� ����
	double	verLen;			// ���� ����
	double	ang;			// ȸ�� ���� (����: Degree, ȸ����: Z��)
};

GSErrCode	placeTableformOnWallContinually (void);		// ���� �������� ���̺����� ��ġ�ϴ� ���� ��ƾ

#endif