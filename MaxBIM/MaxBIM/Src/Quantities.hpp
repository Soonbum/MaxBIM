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
	API_Guid	guid;		// 요소의 GUID

	short	typeOfElem;		// 요소의 타입 (enum elemType 참조)
	double	ang;			// 회전 각도

	// 밑면
	double	bottomXmin;		double	bottomXmax;
	double	bottomYmin;		double	bottomYmax;
	double	bottomZmin;		double	bottomZmax;

	// 측면 1
	double	side1Xmin;		double	side1Xmax;
	double	side1Ymin;		double	side1Ymax;
	double	side1Zmin;		double	side1Zmax;

	// 측면 2
	double	side2Xmin;		double	side2Xmax;
	double	side2Ymin;		double	side2Ymax;
	double	side2Zmin;		double	side2Zmax;

	// 측면 3
	double	side3Xmin;		double	side3Xmax;
	double	side3Ymin;		double	side3Ymax;
	double	side3Zmin;		double	side3Zmax;

	// 측면 4
	double	side4Xmin;		double	side4Xmax;
	double	side4Ymin;		double	side4Ymax;
	double	side4Zmin;		double	side4Zmax;
};

GSErrCode	placeQuantityPlywood (void);	// 부재(벽,슬래브,보,기둥)들의 가설재가 붙을 수 있는 면에 물량합판을 자동으로 부착함

#endif