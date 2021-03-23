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
	double	leftBottomX;	// 좌하단 점의 X 좌표
	double	leftBottomY;	// 좌하단 점의 Y 좌표
	double	leftBottomZ;	// 좌하단 점의 Z 좌표
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

// >> 클래스
// 멤버 변수
// ... 요소 개수
// ... qElem 배열 [요소 개수]
//
// 멤버 함수 
// ... 생성자 (초기화: qElem 배열 할당 및 내부 변수 설정)
// ... 소멸자 (qElem 배열 해제)
// ... 연산자 오버로딩 필요한가?

GSErrCode	placeQuantityPlywood (void);	// 부재(벽,슬래브,보,기둥)들의 가설재가 붙을 수 있는 면에 물량합판을 자동으로 부착함

#endif