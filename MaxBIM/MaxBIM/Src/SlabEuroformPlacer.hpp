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

	enum	idxItems_2_forSlabBottomPlacer {
		LABEL_REMAIN_HORIZONTAL_LENGTH			= 3,
		LABEL_REMAIN_VERTICAL_LENGTH			= 4,
		EDITCONTROL_REMAIN_HORIZONTAL_LENGTH	= 5,
		EDITCONTROL_REMAIN_VERTICAL_LENGTH		= 6,
		GROUPBOX_GRID_EUROFORM_WOOD				= 7,
		PUSHBUTTON_CONFIRM_REMAIN_LENGTH		= 8,

		// 이후에는 그리드 버튼이 배치됨
		GRIDBUTTON_IDX_START					= 9
	};
}

// 슬래브 관련 정보
struct InfoSlab
{
	short	floorInd;			// 층 인덱스
	double	offsetFromTop;		// 레퍼런스 레벨과 슬래브 위쪽 간의 수직 거리
	double	thickness;			// 슬래브 두께
};

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
	// 슬래브 기하 정보
	double	level;				// 고도
	double	ang;				// 회전 각도 (단위: Radian, 회전축: Z축)
	double	leftBottomX;		// 유로폼 시작 좌표 X
	double	leftBottomY;		// 유로폼 시작 좌표 Y
	double	leftBottomZ;		// 유로폼 시작 좌표 Z
	double	formArrayWidth;		// 유로폼 배열 전체 너비
	double	formArrayHeight;	// 유로폼 배열 전체 높이

	// 최외곽 좌표
	double	outerLeft;		// X 좌표
	double	outerRight;		// X 좌표
	double	outerTop;		// Y 좌표
	double	outerBottom;	// Y 좌표

	// 꺾인 부분 코너 좌표, 또는 일반 코너 좌표
	API_Coord3D		corner_leftTop;
	API_Coord3D		corner_leftBottom;
	API_Coord3D		corner_rightTop;
	API_Coord3D		corner_rightBottom;

	// 안쪽 좌표
	double	innerLeft;		// X 좌표
	double	innerRight;		// X 좌표
	double	innerTop;		// Y 좌표
	double	innerBottom;	// Y 좌표

	// 유로폼이 설치되는 영역의 너비/높이
	double	innerWidth;		// 가로 길이
	double	innerHeight;	// 세로 길이

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
	CellForSlab	cells [50][50];		// 유로폼 셀 정보 - 마지막 인덱스: [eu_count_ver-1][eu_count_hor-1]
	bool	topBoundsCells [50];	// 위쪽 목재 보강재 유무 - 마지막 인덱스: [eu_count_hor-2]
	bool	bottomBoundsCells [50];	// 아래쪽 목재 보강재 유무 - 마지막 인덱스: [eu_count_hor-2]
	bool	leftBoundsCells [50];	// 왼쪽 목재 보강재 유무 - 마지막 인덱스: [eu_count_ver-2]
	bool	rightBoundsCells [50];	// 오른쪽 목재 보강재 유무 - 마지막 인덱스: [eu_count_ver-2]
};

// 유로폼 슬래브 하부 배치 함수
GSErrCode	placeEuroformOnSlabBottom (void);				// 2번 메뉴: 슬래브 하부에 유로폼을 배치하는 통합 루틴
bool		isSamePoint (API_Coord3D aPoint, API_Coord3D bPoint);												// aPoint가 bPoint와 같은 점인지 확인함
bool		isAlreadyStored (API_Coord3D aPoint, API_Coord3D pointList [], short startInd, short endInd);		// aPoint가 pointList에 보관이 되었는지 확인함
bool		isNextPoint (API_Coord3D prevPoint, API_Coord3D curPoint, API_Coord3D nextPoint);					// nextPoint가 curPoint의 다음 점입니까?
short		moreCloserPoint (API_Coord3D curPoint, API_Coord3D p1, API_Coord3D p2);								// curPoint에 가까운 점이 p1, p2 중 어떤 점입니까?
API_Coord3D	getUnrotatedPoint (API_Coord3D rotatedPoint, API_Coord3D axisPoint, double ang);					// 회전이 적용되지 않았을 때의 위치 (배치되어야 할 본래 위치를 리턴), 각도는 Degree
void		initCellsForSlabBottom (SlabPlacingZone* placingZone);												// Cell 배열을 초기화함
void		firstPlacingSettingsForSlabBottom (SlabPlacingZone* placingZone);									// 1차 배치: 유로폼
void		setCellPositionForSlabBottom (SlabPlacingZone *target_zone, short ver, short hor);					// 해당 셀의 LeftBottom 위치를 설정
void		alignPlacingZoneForSlabBottom (SlabPlacingZone* target_zone);										// Cell 정보가 변경됨에 따라 파편화된 위치를 재조정함
API_Guid	placeLibPartForSlabBottom (CellForSlab objInfo);													// 해당 셀 정보를 기반으로 라이브러리 배치
GSErrCode	fillRestAreasForSlabBottom (void);																	// 유로폼을 채운 후 자투리 공간 채우기
short DGCALLBACK slabBottomPlacerHandler1 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 1차 배치를 위한 질의를 요청하는 1차 다이얼로그
short DGCALLBACK slabBottomPlacerHandler2 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 1차 배치 후 수정을 요청하는 2차 다이얼로그
short DGCALLBACK slabBottomPlacerHandler3 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 2차 다이얼로그에서 각 셀의 객체 타입을 변경하기 위한 3차 다이얼로그

#endif
