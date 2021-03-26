#ifndef	__QUANTITIES__
#define __QUANTITIES__

#include "MaxBIM.hpp"

using namespace std;

namespace quantitiesDG {
	enum	elemType {
		ELEM_WALL,		// 벽
		ELEM_SLAB,		// 슬래브
		ELEM_BEAM,		// 보
		ELEM_COLUMN,	// 기둥
		ELEM_UNKNOWN	// 그 외의 타입은 취급하지 않음
	};
}

struct qElem
{
	API_Guid	guid;		// 요소의 GUID

	short	typeOfElem;		// 요소의 타입 (enum elemType 참조)
	double	ang;			// 회전 각도 (현재는 사용하지 않음)

	API_Coord3D		bottomPoint;	// 최하단 점
	API_Coord3D		topPoint;		// 최상단 점
};

GSErrCode	placeQuantityPlywood (void);	// 부재(벽,슬래브,보,기둥)들의 가설재가 붙을 수 있는 면에 물량합판을 자동으로 부착함
short DGCALLBACK quantityPlywoodUIHandler (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// [다이얼로그] 물량산출 합판 레이어 지정

#endif