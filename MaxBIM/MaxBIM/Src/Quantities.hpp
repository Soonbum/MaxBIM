#ifndef	__QUANTITIES__
#define __QUANTITIES__

#include "MaxBIM.hpp"

using namespace std;

namespace quantitiesDG {
	enum	elemType {
		ELEM_WALL,
		ELEM_SLAB,
		ELEM_BEAM,
		ELEM_COLUMN
	};
}

struct qElem
{
	API_Guid	guid;		// ����� GUID

	short	typeOfElem;		// ����� Ÿ�� (enum elemType ����)
	double	ang;			// ȸ�� ����

	// �ظ�
	double	bottomXmin;		double	bottomXmax;
	double	bottomYmin;		double	bottomYmax;
	double	bottomZmin;		double	bottomZmax;

	// ���� 1
	double	side1Xmin;		double	side1Xmax;
	double	side1Ymin;		double	side1Ymax;
	double	side1Zmin;		double	side1Zmax;

	// ���� 2
	double	side2Xmin;		double	side2Xmax;
	double	side2Ymin;		double	side2Ymax;
	double	side2Zmin;		double	side2Zmax;

	// ���� 3
	double	side3Xmin;		double	side3Xmax;
	double	side3Ymin;		double	side3Ymax;
	double	side3Zmin;		double	side3Zmax;

	// ���� 4
	double	side4Xmin;		double	side4Xmax;
	double	side4Ymin;		double	side4Ymax;
	double	side4Zmin;		double	side4Zmax;
};

GSErrCode	placeQuantityPlywood (void);	// ����(��,������,��,���)���� �����簡 ���� �� �ִ� �鿡 ���������� �ڵ����� ������

#endif