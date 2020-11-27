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

	// 기둥을 중심으로 간섭 보가 있는 방향
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

		LABEL_COLUMN_WIDTH,
		LABEL_COLUMN_DEPTH,
		EDITCONTROL_COLUMN_WIDTH,
		EDITCONTROL_COLUMN_DEPTH,

		ICON_LAYER,
		LABEL_LAYER_SETTINGS,
		LABEL_LAYER_EUROFORM,
		LABEL_LAYER_INCORNER,
		LABEL_LAYER_OUTCORNER,
		LABEL_LAYER_PLYWOOD,
		LABEL_LAYER_MAGIC_BAR,
		LABEL_LAYER_MAGIC_INCORNER,

		USERCONTROL_LAYER_EUROFORM,
		USERCONTROL_LAYER_INCORNER,
		USERCONTROL_LAYER_OUTCORNER,
		USERCONTROL_LAYER_PLYWOOD,
		USERCONTROL_LAYER_MAGIC_BAR,
		USERCONTROL_LAYER_MAGIC_INCORNER
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
}

// 기둥 관련 정보
struct InfoColumn
{
	API_Guid	guid;		// 기둥의 GUID
	short	floorInd;		// 층 인덱스

	bool	bRectangle;		// 직사각형 형태가 맞는가?
	short	coreAnchor;		// 코어의 앵커 포인트
	double	coreWidth;		// 기둥의 X 길이
	double	coreDepth;		// 기둥의 Y 길이
	double	venThick;		// 기둥 베니어 두께
	double	height;			// 기둥의 높이
	double	bottomOffset;	// 바닥 레벨에 대한 기둥 베이스 레벨
	double	topOffset;		// 만약 기둥이 윗층과 연결되어 있는 경우 윗층으로부터의 오프셋
	double	angle;			// 기둥 축을 중심으로 한 회전 각도 (단위: Radian)
	API_Coord	origoPos;	// 기둥의 위치
};

// 벽 관련 정보
struct InfoWallForColumn
{
	API_Guid	guid;			// 보의 GUID

	double	wallThk;			// 벽 두께
	short	floorInd;			// 층 인덱스
	double	bottomOffset;		// 벽 하단 오프셋
	double	offset;				// 시작점에서 레퍼런스 라인으로부터 벽의 기초 라인의 오프셋
	double	angle;				// 회전 각도 (단위: Radian)
	
	double								offsetFromOutside;		// 벽의 레퍼런스 라인과 벽의 바깥쪽 면 간의 거리
	API_WallReferenceLineLocationID     referenceLineLocation;	// 레퍼런스 라인의 위치
	/*
		APIWallRefLine_Outside (0)
			레퍼런스 라인 위치는 벽의 외부 면 상에 있습니다.
		APIWallRefLine_Center (1)
			레퍼런스 라인 위치는 벽의 중앙에 있습니다.
		APIWallRefLine_Inside (2)
			레퍼런스 라인 위치는 벽의 내부 면 상에 있습니다.
		APIWallRefLine_CoreOutside (3)
			레퍼런스 라인 위치는 복합 구조의 코어 외부 스킨 상에 있습니다.
		APIWallRefLine_CoreCenter (4)
			레퍼런스 라인 위치는 복합 구조의 코어 스킨의 중앙에 있습니다.
		APIWallRefLine_CoreInside (5)
			레퍼런스 라인 위치는 복합 구조의 코어 내부 스킨 상에 있습니다.
	 */

	double	begX;				// 시작점 X
	double	begY;				// 시작점 Y
	double	endX;				// 끝점 X
	double	endY;				// 끝점 Y
};

// 보 관련 정보
struct InfoBeamForColumn
{
	API_Guid	guid;	// 보의 GUID
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
	short	anchor;			// 부착할 앵커 포인트

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
struct ColumnPlacingZone
{
	// 기둥 기하 정보
	bool	bRectangle;		// 직사각형인가?
	short	coreAnchor;		// 코어의 앵커 포인트
	double	coreDepth;		// 기둥의 깊이
	double	coreWidth;		// 기둥의 너비
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

	// 수직 방향으로의 셀 개수
	short	nCells;
};

// 유로폼 기둥 배치 함수
GSErrCode	placeEuroformOnColumn (void);									// 5번 메뉴: 기둥에 유로폼을 배치하는 통합 루틴
void		initCellsForColumn (ColumnPlacingZone* placingZone);			// Cell 배열을 초기화함
void		addTopCell (ColumnPlacingZone* target_zone);					// 꼭대기에 셀 추가
void		delTopCell (ColumnPlacingZone* target_zone);					// 꼭대기의 셀 삭제
void		alignPlacingZoneForColumn (ColumnPlacingZone* placingZone);		// Cell 정보가 변경됨에 따라 파편화된 위치를 재조정함
API_Guid	placeLibPartForColumn (CellForColumn objInfo);					// 해당 셀 정보를 기반으로 라이브러리 배치
GSErrCode	fillRestAreasForColumn (ColumnPlacingZone* placingZone);		// 유로폼/아웃코너판넬을 채운 후 자투리 공간 채우기 (나머지는 매직바, 매직인코너, 합판으로 채움)
short DGCALLBACK columnPlacerHandler_soleColumn_1 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 1차 배치를 위한 질의를 요청하는 1차 다이얼로그
short DGCALLBACK columnPlacerHandler_soleColumn_2 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 1차 배치 후 수정을 요청하는 2차 다이얼로그
short DGCALLBACK columnPlacerHandler_soleColumn_3 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 2차 다이얼로그에서 각 셀의 객체 타입을 변경하기 위한 3차 다이얼로그

#endif
