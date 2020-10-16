#ifndef	__SLAB_EUROFORM_PLACER__
#define __SLAB_EUROFORM_PLACER__

#include "MaxBIM.hpp"

namespace slabBottomPlacerDG {
	// 다이얼로그 항목 인덱스
	enum	idxItems_1_forSlabBottomPlacer {
		LABEL_PLACING_EUROFORM		= 3,
		LABEL_EUROFORM_WIDTH,
		POPUP_EUROFORM_WIDTH,
		LABEL_EUROFORM_HEIGHT,
		POPUP_EUROFORM_HEIGHT,
		LABEL_EUROFORM_ORIENTATION,
		POPUP_EUROFORM_ORIENTATION,
		SEPARATOR_1,

		ICON_LAYER,
		LABEL_LAYER_SETTINGS,
		LABEL_LAYER_EUROFORM,
		LABEL_LAYER_PLYWOOD,
		LABEL_LAYER_WOOD,

		USERCONTROL_LAYER_EUROFORM,
		USERCONTROL_LAYER_PLYWOOD,
		USERCONTROL_LAYER_WOOD
	};
}

// 모프 관련 정보
struct InfoMorphForSlab
{
	API_Guid	guid;		// 모프의 GUID

	double	leftBottomX;	// 좌하단 좌표 X
	double	leftBottomY;	// 좌하단 좌표 Y
	double	leftBottomZ;	// 좌하단 좌표 Z

	//double	rightTopX;		// 우상단 좌표 X
	//double	rightTopY;		// 우상단 좌표 Y
	//double	rightTopZ;		// 우상단 좌표 Z

	double	horLen;			// 가로 길이
	double	verLen;			// 세로 길이
	double	ang;			// 회전 각도 (단위: Degree, 회전축: Z축)

	// ...
};

// 그리드 각 셀 정보
struct CellForSlab
{
	short		objType;	// enum libPartObjType 참조

	API_Guid	guid;		// 객체의 GUID

	double	leftBottomX;	// 좌하단 좌표 X
	double	leftBottomY;	// 좌하단 좌표 Y
	double	leftBottomZ;	// 좌하단 좌표 Z

	double	horLen;			// 가로 길이
	double	verLen;			// 세로 길이
	double	ang;			// 회전 각도 (단위: Radian, 회전축: Z축)

	union {
		Euroform		form;
		Plywood			plywood;
		Wood			wood;
	} libPart;
};

// 슬래브 하부 영역 정보
struct SlabPlacingZone
{
	/*
	double	horLen;			// 가로 길이
	double	verLen;			// 세로 길이
	*/
	double	level;			// 고도
	double	ang;			// 회전 각도 (단위: Radian, 회전축: Z축)

	// 최외곽 좌표
	double	outerLeft;		// X 좌표
	double	outerRight;		// X 좌표
	double	outerTop;		// Y 좌표
	double	outerBottom;	// Y 좌표

	// 꺾인 부분 코너 좌표 (없으면 NULL)
	API_Coord3D		*corner_leftTop;
	API_Coord3D		*corner_leftBottom;
	API_Coord3D		*corner_rightTop;
	API_Coord3D		*corner_rightBottom;

	// 검토할 사항 (1. 기본 채우기)
	double	remain_hor;				// 가로 방향 남은 길이
	double	remain_hor_updated;		// 가로 방향 남은 길이 (업데이트 후)
	double	remain_ver;				// 세로 방향 남은 길이
	double	remain_ver_updated;		// 세로 방향 남은 길이 (업데이트 후)

	std::string		eu_wid;			// 유로폼 너비
	std::string		eu_hei;			// 유로폼 높이
	std::string		eu_ori;			// 유로폼 방향
	double	eu_wid_numeric;			// 유로폼 너비 (실수형)
	double	eu_hei_numeric;			// 유로폼 높이 (실수형)
	short	eu_count_hor;			// 가로 방향 유로폼 개수
	short	eu_count_ver;			// 세로 방향 유로폼 개수

	// 검토할 사항 (2. 배치된 객체 정보를 그리드로 관리)
	CellForSlab	cells [50][50];		// 마지막 인덱스: [eu_count_ver-1][eu_count_hor-1]
};

// 유로폼 슬래브 하부 배치 함수
GSErrCode	placeEuroformOnSlabBottom (void);				// 2번 메뉴: 슬래브 하부에 유로폼을 배치하는 통합 루틴
bool		isSamePoint (API_Coord3D aPoint, API_Coord3D bPoint);												// aPoint가 bPoint와 같은 점인지 확인함
bool		isAlreadyStored (API_Coord3D aPoint, API_Coord3D pointList [], short startInd, short endInd);		// aPoint가 pointList에 보관이 되었는지 확인함
bool		isNextPoint (API_Coord3D prevPoint, API_Coord3D curPoint, API_Coord3D nextPoint);					// nextPoint가 curPoint의 다음 점입니까?
short		moreCloserPoint (API_Coord3D curPoint, API_Coord3D p1, API_Coord3D p2);								// curPoint에 가까운 점이 p1, p2 중 어떤 점입니까?
short DGCALLBACK slabBottomPlacerHandler1 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// 1차 배치를 위한 질의를 요청하는 1차 다이얼로그

#endif
