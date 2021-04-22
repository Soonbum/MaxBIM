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
		ELEM_UNKNOWN	// 알 수 없음
	};

	enum	qPlywoodType {
		Q_WALL_INNER,		// 벽 (내벽)
		Q_WALL_OUTER,		// 벽 (외벽)
		Q_WALL_COMPOSITE,	// 벽 (합벽)
		Q_WALL_PARAPET,		// 벽 (파라펫)
		Q_WALL_WATERPROOF,	// 벽 (방수턱)
		Q_SLAB_BASE,		// 슬래브 (기초)
		Q_SLAB_RC,			// 슬래브 (RC)
		Q_SLAB_DECK,		// 슬래브 (데크)
		Q_SLAB_RAMP,		// 슬래브 (램프)
		Q_BEAM,				// 보
		Q_COLUMN_ISOLATED,	// 기둥 (독립)
		Q_COLUMN_INWALL,	// 기둥 (벽체)
		Q_UNKNOWN			// 그 외의 타입은 취급하지 않음
	};

	enum	sideStr {
		NORTH_SIDE,
		SOUTH_SIDE,
		EAST_SIDE,
		WEST_SIDE,
		BASE_SIDE
	};

	enum	modeStr {
		MODE_X,
		MODE_Y,
		MODE_Z
	};
}

class qElem
{
public:
	API_Guid	guid;			// 요소의 GUID

	short	typeOfElem;			// 요소의 타입 (enum elemType 참조)
	short	floorInd;			// 층 인덱스
	short	layerInd;			// 레이어 인덱스
	short	qLayerInd;			// 물량합판 레이어 인덱스
	short	typeOfQPlywood;		// 물량합판 객체의 타입 (enum qPlywoodType 참조)
	//double	ang;			// 회전 각도 (현재는 사용하지 않음)

	bool	hasHole;			// (문, 창문에 의한) 구멍이 나 있는가?

	API_Coord3D		bottomPoint;	// 최하단 점 (좌하단 바닥)
	API_Coord3D		topPoint;		// 최상단 점 (우상단 꼭대기)

	// 북쪽 측면
	API_Coord3D		NorthLeftBottom;	// 좌하단
	API_Coord3D		NorthRightTop;		// 우상단
	bool			validNorth;			// 유효한 경우 물량합판을 부착함

	// 남쪽 측면
	API_Coord3D		SouthLeftBottom;	// 좌하단
	API_Coord3D		SouthRightTop;		// 우상단
	bool			validSouth;			// 유효한 경우 물량합판을 부착함

	// 동쪽 측면
	API_Coord3D		EastLeftBottom;		// 좌하단
	API_Coord3D		EastRightTop;		// 우상단
	bool			validEast;			// 유효한 경우 물량합판을 부착함

	// 서쪽 측면
	API_Coord3D		WestLeftBottom;		// 좌하단
	API_Coord3D		WestRightTop;		// 우상단
	bool			validWest;			// 유효한 경우 물량합판을 부착함

	// 밑면
	API_Coord3D		BaseLeftBottom;		// 좌하단
	API_Coord3D		BaseRightTop;		// 우하단
	bool			validBase;			// 유효한 경우 물량합판을 부착함

	// 부착할 물량합판들의 GUID
	short		nQPlywoods;
	API_Guid	qPlywoodGuids [5];

	// 간섭을 일으키는 다른 요소들의 GUID
	short		nOtherElems;
	API_Guid	otherGuids [200];

public:
	short	invalidateShortTwoSide (qElem* element);	// 4개의 측면 중 짧은 2개의 측면을 무효화하고, 유효한 면 정보를 리턴함. 리턴 값은 다음 값의 조합입니다. 북쪽/남쪽(2), 동쪽/서쪽(1), 오류(0)
	short	invalidateLongTwoSide (qElem* element);		// 4개의 측면 중 긴 2개의 측면을 무효화하고, 유효한 면 정보를 리턴함. 리턴 값은 다음 값의 조합입니다. 북쪽/남쪽(2), 동쪽/서쪽(1), 오류(0)
	void	invalidateAllSide (qElem* element);			// 4개의 측면을 모두 무효화함
	void	invalidateBase (qElem* element);			// 밑면을 무효화함
	void	validateAllSide (qElem* element);			// 4개의 측면을 모두 유효화함
	void	validateBase (qElem* element);				// 밑면을 유효화함
	bool	subtractArea (qElem* src, qElem operand);	// src 요소의 측면, 밑면 영역이 operand 요소에 의해 침범 당할 경우, 솔리드 연산을 위해 operand의 GUID를 저장함
	void	placeQuantityPlywood (qElem* element);		// 요소의 측면들과 밑면 영역에 물량합판을 부착함
};

class qElemType
{
public:
	double	areas_wall_in;			// 면적: 벽체(내벽)
	double	areas_wall_out;			// 면적: 벽체(외벽)
	double	areas_wall_composite;	// 면적: 벽체(합벽)
	double	areas_wall_parapet;		// 면적: 벽체(파라펫)
	double	areas_wall_waterproof;	// 면적: 벽체(방수턱)
	double	areas_slab_base;		// 면적: 스라브(기초)
	double	areas_slab_rc;			// 면적: 스라브(RC)
	double	areas_slab_deck;		// 면적: 스라브(데크)
	double	areas_slab_ramp;		// 면적: 스라브(램프)
	double	areas_beam;				// 면적: 보
	double	areas_column_iso;		// 면적: 기둥(독립)
	double	areas_column_inwall;	// 면적: 기둥(벽체)
	double	areas_column_circle;	// 면적: 기둥(원형)
	double	areas_ramp_wall;		// 면적: 램프구간(벽)

public:
	void	initAreas (qElemType* elem);	// 면적 값을 초기화
};

GSErrCode	placeQuantityPlywood (void);				// 부재(벽,슬래브,보,기둥)들의 가설재가 붙을 수 있는 면에 물량합판을 자동으로 부착함
short	DGCALLBACK quantityPlywoodUIHandler (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// [다이얼로그] 물량산출 합판 레이어 지정

GSErrCode	calcAreasOfQuantityPlywood (void);			// 종류별 물량합판의 표면적 합계 값을 구함

#endif