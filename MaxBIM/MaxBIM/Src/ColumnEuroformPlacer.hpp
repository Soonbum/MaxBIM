#ifndef	__COLUMN_EUROFORM_PLACER__
#define __COLUMN_EUROFORM_PLACER__

#include "MaxBIM.hpp"

namespace columnPlacerDG {
	// 객체 타입
	enum	libPartObjType {
		NONE,			// 없음
		EUROFORM,		// 유로폼v2.0
		INCORNER,		// 인코너판넬v1.0
		OUTCORNER,		// 아웃코너판넬v1.0
		PLYWOOD,		// 합판v1.0
		MAGIC_BAR,		// 매직바v1.0
		MAGIC_INCORNER	// 매직인코너v1.0
	};

	// 부재가 부착되는 곳에 해당하는 기둥의 앵커 포인트
	enum	anchorPoint {
		LEFT_TOP,		// 좌상단
		RIGHT_TOP,		// 우상단
		LEFT_BOTTOM,	// 좌하단
		RIGHT_BOTTOM,	// 우하단
		TOP,			// 상단
		LEFT,			// 좌측
		RIGHT,			// 우측
		BOTTOM			// 하단
	};

	// 기둥을 중심으로 간섭 보, 벽이 있는 방향
	enum	direction {
		NORTH,
		SOUTH,
		EAST,
		WEST
	};

	// 다이얼로그 항목 인덱스
	enum	idxItems_1_forColumnPlacer {
		LABEL_COLUMN_SECTION		= 3,
		ICON_COLUMN_SECTION,
		
		EDITCONTROL_TOP_1,
		EDITCONTROL_TOP_2,
		EDITCONTROL_TOP_3,
		EDITCONTROL_TOP_4,
		EDITCONTROL_LEFT_1,
		EDITCONTROL_LEFT_2,
		EDITCONTROL_LEFT_3,
		EDITCONTROL_LEFT_4,
		EDITCONTROL_RIGHT_1,
		EDITCONTROL_RIGHT_2,
		EDITCONTROL_RIGHT_3,
		EDITCONTROL_RIGHT_4,
		EDITCONTROL_BOTTOM_1,
		EDITCONTROL_BOTTOM_2,
		EDITCONTROL_BOTTOM_3,
		EDITCONTROL_BOTTOM_4,

		CHECKBOX_TOP_ADDITIONAL_FORM,
		CHECKBOX_LEFT_ADDITIONAL_FORM,
		CHECKBOX_RIGHT_ADDITIONAL_FORM,
		CHECKBOX_BOTTOM_ADDITIONAL_FORM,

		LABEL_COLUMN_DEPTH,
		LABEL_COLUMN_WIDTH,
		EDITCONTROL_COLUMN_WIDTH,
		EDITCONTROL_COLUMN_DEPTH,

		ICON_LAYER,
		LABEL_LAYER_SETTINGS,
		LABEL_LAYER_EUROFORM,
		LABEL_LAYER_OUTCORNER,
		LABEL_LAYER_PLYWOOD,

		USERCONTROL_LAYER_EUROFORM,
		USERCONTROL_LAYER_OUTCORNER,
		USERCONTROL_LAYER_PLYWOOD,
	};

	enum	idxItems_2_forColumnPlacer {
		DG_UPDATE_BUTTON	= 3,
		DG_PREV,
		LABEL_COLUMN_SIDE,
		AFTER_ALL
	};
	
	enum	idxItems_3_forColumnPlacer {
		LABEL_OBJ_TYPE			= 3,
		POPUP_OBJ_TYPE,
		CHECKBOX_SET_STANDARD,
		LABEL_LENGTH,
		EDITCONTROL_LENGTH,
		POPUP_LENGTH
	};

	enum	idxItems_1_forWallColumnPlacer {
		LABEL_COLUMN_SECTION_WC	= 3,

		ICON_COLUMN_SECTION_01_WC,
		ICON_COLUMN_SECTION_02_WC,
		ICON_COLUMN_SECTION_03_WC,
		ICON_COLUMN_SECTION_04_WC,
		ICON_COLUMN_SECTION_05_WC,
		ICON_COLUMN_SECTION_06_WC,
		ICON_COLUMN_SECTION_07_WC,

		ICON_COLUMN_SECTION_08_WC,
		ICON_COLUMN_SECTION_09_WC,
		ICON_COLUMN_SECTION_10_WC,
		ICON_COLUMN_SECTION_11_WC,
		ICON_COLUMN_SECTION_12_WC,
		ICON_COLUMN_SECTION_13_WC,
		ICON_COLUMN_SECTION_14_WC,

		ICON_LAYER_WC,
		LABEL_LAYER_SETTINGS_WC,
		LABEL_LAYER_EUROFORM_WC,
		LABEL_LAYER_INCORNER_WC,
		LABEL_LAYER_OUTCORNER_WC,
		LABEL_LAYER_PLYWOOD_WC,

		USERCONTROL_LAYER_EUROFORM_WC,
		USERCONTROL_LAYER_INCORNER_WC,
		USERCONTROL_LAYER_OUTCORNER_WC,
		USERCONTROL_LAYER_PLYWOOD_WC,

		AFTER_ALL_WC
	};
}

// 벽 관련 정보
struct InfoWallForColumn
{
	API_Guid	guid;			// 보의 GUID

	double	wallThk;			// 벽 두께
	short	floorInd;			// 층 인덱스
	double	bottomOffset;		// 벽 하단 오프셋
	double	angle;				// 회전 각도 (단위: Radian)

	long	nCoords;			// 정점 개수
	API_Coord	poly [10];		// 정점 좌표
	
	API_Coord	begC;			// 레퍼런스 라인 시작점
	API_Coord	endC;			// 레퍼런스 라인 끝점

	API_Coord	begC_1;			// 벽면 1 시작점
	API_Coord	endC_1;			// 벽면 1 끝점
	API_Coord	begC_2;			// 벽면 2 시작점
	API_Coord	endC_2;			// 벽면 2 끝점
};

// 보 관련 정보
struct InfoBeamForColumn
{
	API_Guid	guid;	// 보의 GUID
	bool	valid;		// 정보의 유효성
	short	floorInd;	// 층 인덱스

	double	height;		// 보 높이
	double	width;		// 보 너비
	double	offset;		// 보 중심으로부터 보의 레퍼런스 라인의 오프셋입니다.
	double	level;		// 바닥 레벨에 대한 보의 위쪽면 높이입니다.

	API_Coord	begC;	// 보 시작 좌표
	API_Coord	endC;	// 보 끝 좌표
};

// 모프 관련 정보
struct InfoMorphForColumn
{
	API_Guid	guid;		// 모프의 GUID
	short		floorInd;	// 층 인덱스
	double		level;		// 모프의 고도
	double		height;		// 모프의 높이
};

// 그리드 각 셀 정보
struct CellForColumn
{
	short		objType;	// enum libPartObjType 참조

	API_Guid	guid;		// 객체의 GUID

	double	leftBottomX;	// 좌하단 좌표 X
	double	leftBottomY;	// 좌하단 좌표 Y
	double	leftBottomZ;	// 좌하단 좌표 Z
	double	ang;			// 회전 각도 (단위: Radian, 회전축: Z축)

	double	horLen;			// 가로 길이
	double	verLen;			// 세로 길이
	double	height;			// 높이

	union {
		Euroform		form;
		IncornerPanel	incorner;
		OutcornerPanel	outcorner;
		Plywood			plywood;
		MagicBar		mbar;
		MagicIncorner	mincorner;
	} libPart;
};

// 기둥 영역 정보
class ColumnPlacingZone
{
public:
	// 기둥 기하 정보
	bool	bRectangle;		// 직사각형인가?
	short	coreAnchor;		// 코어의 앵커 포인트
	double	coreWidth;		// 기둥의 너비 (X 길이)
	double	coreDepth;		// 기둥의 깊이 (Y 길이)
	double	venThick;		// 기둥 베니어 두께
	double	height;			// 기둥의 높이
	double	bottomOffset;	// 바닥 레벨에 대한 기둥 베이스 레벨
	double	topOffset;		// 만약 기둥이 윗층과 연결되어 있는 경우 윗층으로부터의 오프셋
	double	angle;			// 기둥 축을 중심으로 한 회전 각도 (단위: Radian, 회전축: Z축)
	API_Coord	origoPos;	// 기둥의 위치

	double	areaHeight;		// 모프가 지정한 영역의 높이

	// 간섭 보 관련 정보
	bool	bInterfereBeam;				// 간섭 보 여부
	short	nInterfereBeams;			// 간섭 보 개수 (0~4개)
	bool	bExistBeams [4];			// 간섭 보 유무
	double	bottomLevelOfBeams [4];		// 간섭 보의 하단 레벨
	InfoBeamForColumn	beams [4];		// 간섭 보 정보

	// 간섭 벽으로 인해 분리된 가로/세로 길이
	double	hLenDown, vLenDown;			// 벽의 아래쪽/왼쪽 가로/세로 길이
	double	hLenUp, vLenUp;				// 벽의 위쪽/오른쪽 가로/세로 길이

	// 간섭 벽 관련 정보
	bool	bWallHorizontalDirected;	// 벽의 방향 (true: 가로, false: 세로)
	double	posTopWallLine;				// 위쪽(오른쪽) 벽 라인의 수직(수평) 위치
	double	posBottomWallLine;			// 아래쪽(왼쪽) 벽 라인의 수직(수평) 위치
	double	posTopColumnLine;			// 위쪽(오른쪽) 기둥 라인의 수직(수평) 위치
	double	posBottomColumnLine;		// 아래쪽(왼쪽) 기둥 라인의 수직(수평) 위치
	short	relationCase;				// 벽과 기둥과의 관계
	/*
	1. 가로인 경우 (위부터)
		CASE 1. 기둥 바로 위에 벽이 붙은 경우:			top wall > bottom wall == top column > bottom column
		CASE 2. 기둥이 벽 아래에 조금 들어간 경우:		top wall > top column > bottom wall > bottom column
		CASE 3. 기둥 위쪽이 벽 위쪽과 일치한 경우:		top wall == top column > bottom wall > bottom column
		CASE 4. 기둥이 벽을 삼킨 경우:					top column > top wall > bottom wall > bottom column
		CASE 5. 기둥 아래쪽이 벽 아래쪽과 일치한 경우:	top column > top wall > bottom wall == bottom column
		CASE 6. 기둥이 벽 위에 조금 들어간 경우:		top column > top wall > bottom column > bottom wall
		CASE 7. 기둥 바로 아래에 벽이 붙은 경우:		top column > bottom column == top wall > bottom wall
	2. 세로인 경우 (오른쪽부터)
		CASE 1. 기둥 바로 오른쪽에 벽이 붙은 경우:		top wall > bottom wall == top column > bottom column
		CASE 2. 기둥이 벽 왼쪽에 조금 들어간 경우:		top wall > top column > bottom wall > bottom column
		CASE 3. 기둥 오른쪽이 벽 오른쪽과 일치한 경우:	top wall == top column > bottom wall > bottom column
		CASE 4. 기둥이 벽을 삼킨 경우:					top column > top wall > bottom wall > bottom column
		CASE 5. 기둥 왼쪽이 벽 왼쪽과 일치한 경우:		top column > top wall > bottom wall == bottom column
		CASE 6. 기둥이 벽 오른쪽에 조금 들어간 경우:	top column > top wall > bottom column > bottom wall
		CASE 7. 기둥 바로 왼쪽에 벽이 붙은 경우:		top column > bottom column == top wall > bottom wall
	*/

	// 기둥 위쪽 여백
	double	marginTopAtNorth;			// 기둥 북쪽 면의 위쪽 여백
	double	marginTopAtWest;			// 기둥 서쪽 면의 위쪽 여백
	double	marginTopAtEast;			// 기둥 동쪽 면의 위쪽 여백
	double	marginTopAtSouth;			// 기둥 남쪽 면의 위쪽 여백

	// 기둥 위쪽 여백을 채울지 여부
	bool	bFillMarginTopAtNorth;		// 기둥 북쪽 면의 위쪽 여백 채움
	bool	bFillMarginTopAtWest;		// 기둥 서쪽 면의 위쪽 여백 채움
	bool	bFillMarginTopAtEast;		// 기둥 동쪽 면의 위쪽 여백 채움
	bool	bFillMarginTopAtSouth;		// 기둥 남쪽 면의 위쪽 여백 채움

	// 셀 정보
	CellForColumn	cellsLT [20];		// 좌상단 셀
	CellForColumn	cellsRT [20];		// 우상단 셀
	CellForColumn	cellsLB [20];		// 좌하단 셀
	CellForColumn	cellsRB [20];		// 우하단 셀
	CellForColumn	cellsT1 [20];		// 상단1 셀 (왼쪽)
	CellForColumn	cellsT2 [20];		// 상단2 셀 (오른쪽)
	CellForColumn	cellsL1 [20];		// 좌측1 셀 (위)
	CellForColumn	cellsL2 [20];		// 좌측2 셀 (아래)
	CellForColumn	cellsR1 [20];		// 우측1 셀 (위)
	CellForColumn	cellsR2 [20];		// 우측2 셀 (아래)
	CellForColumn	cellsB1 [20];		// 하단1 셀 (왼쪽)
	CellForColumn	cellsB2 [20];		// 하단2 셀 (오른쪽)

	CellForColumn	cellsLin1 [20];		// 왼쪽 인코너 셀 1 (위)
	CellForColumn	cellsLin2 [20];		// 왼쪽 인코너 셀 2 (아래)
	CellForColumn	cellsRin1 [20];		// 오른쪽 인코너 셀 1 (위)
	CellForColumn	cellsRin2 [20];		// 오른쪽 인코너 셀 2 (아래)

	CellForColumn	cellsW1 [20];		// 벽 반대쪽 셀 1
	CellForColumn	cellsW2 [20];		// 벽 반대쪽 셀 2
	CellForColumn	cellsW3 [20];		// 벽 반대쪽 셀 3
	CellForColumn	cellsW4 [20];		// 벽 반대쪽 셀 4

	// 수직 방향으로의 셀 개수
	short	nCells;

public:
	void		initCells (ColumnPlacingZone* placingZone);						// Cell 배열을 초기화함
	void		addTopCell (ColumnPlacingZone* target_zone);					// 꼭대기에 셀 추가
	void		delTopCell (ColumnPlacingZone* target_zone);					// 꼭대기의 셀 삭제
	void		alignPlacingZone_soleColumn (ColumnPlacingZone* placingZone);	// Cell 정보가 변경됨에 따라 파편화된 위치를 재조정함
	void		alignPlacingZone_wallColumn (ColumnPlacingZone* placingZone);	// Cell 정보가 변경됨에 따라 파편화된 위치를 재조정함
	API_Guid	placeLibPart (CellForColumn objInfo);							// 해당 셀 정보를 기반으로 라이브러리 배치
	GSErrCode	fillRestAreas_soleColumn (ColumnPlacingZone* placingZone);		// 유로폼/아웃코너판넬을 채운 후 자투리 공간 채우기 (나머지는 합판으로 채움)
	GSErrCode	fillRestAreas_wallColumn (ColumnPlacingZone* placingZone);		// 유로폼/아웃코너판넬을 채운 후 자투리 공간 채우기 (나머지는 합판으로 채움)
};

// 유로폼 기둥 배치 함수
GSErrCode	placeEuroformOnColumn (void);	// 기둥에 유로폼을 배치하는 통합 루틴
short DGCALLBACK columnPlacerHandler_soleColumn_1 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 1차 배치를 위한 질의를 요청하는 1차 다이얼로그
short DGCALLBACK columnPlacerHandler_soleColumn_2 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 1차 배치 후 수정을 요청하는 2차 다이얼로그
short DGCALLBACK columnPlacerHandler_soleColumn_3 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 2차 다이얼로그에서 각 셀의 객체 타입을 변경하기 위한 3차 다이얼로그
short DGCALLBACK columnPlacerHandler_wallColumn_1 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 1차 배치를 위한 질의를 요청하는 1차 다이얼로그
short DGCALLBACK columnPlacerHandler_wallColumn_2 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 1차 배치 후 수정을 요청하는 2차 다이얼로그
short DGCALLBACK columnPlacerHandler_wallColumn_3 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 2차 다이얼로그에서 각 셀의 객체 타입을 변경하기 위한 3차 다이얼로그

#endif
