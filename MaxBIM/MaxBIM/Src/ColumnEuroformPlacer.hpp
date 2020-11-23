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
}

// 기둥 관련 정보
struct InfoColumn
{
	bool	bRectangle;		// 직사각형 형태가 맞는가?
	short	coreAnchor;		// 코어의 앵커 포인트
	double	coreDepth;		// 기둥의 깊이
	double	coreWidth;		// 기둥의 너비
	double	venThick;		// 기둥 베니어 두께
	double	height;			// 기둥의 높이
	double	bottomOffset;	// 바닥 레벨에 대한 기둥 베이스 레벨
	double	topOffset;		// 만약 기둥이 윗층과 연결되어 있는 경우 윗층으로부터의 오프셋
	double	angle;			// 기둥 축을 중심으로 한 회전 각도 (단위: radian)
	API_Coord	origoPos;	// 기둥의 위치
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
	short	anchor;			// 부착할 앵커 포인트: 좌상단(1), 상단(2), 우상단(3), 좌(4), 우(5), 좌하단(6), 하단(7), 우하단(8)

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
	// 기둥 기하 정보 !!!
	short	coreAnchor;		// 코어의 앵커 포인트
	double	coreDepth;		// 기둥의 깊이
	double	coreWidth;		// 기둥의 너비
	double	venThick;		// 기둥 베니어 두께
	double	height;			// 기둥의 높이
	double	bottomOffset;	// 바닥 레벨에 대한 기둥 베이스 레벨
	double	topOffset;		// 만약 기둥이 윗층과 연결되어 있는 경우 윗층으로부터의 오프셋
	double	angle;			// 기둥 축을 중심으로 한 회전 각도 (단위: Radian, 회전축: Z축)
	API_Coord	origoPos;	// 기둥의 위치
	
	double	areaHeight;			// 영역 높이

	// 간섭 보 관련 정보
	bool	bInterfereBeam;				// 간섭 보 여부
	short	nInterfereBeams;			// 간섭 보 개수 (0~4개)
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

	short	nCellsLT;
	short	nCellsRT;
	short	nCellsLB;
	short	nCellsRB;
	short	nCellsT1;
	short	nCellsT2;
	short	nCellsL1;
	short	nCellsL2;
	short	nCellsR1;
	short	nCellsR2;
	short	nCellsB1;
	short	nCellsB2;

	double			gap;				// 기둥과의 간격
};

// 유로폼 기둥 배치 함수
GSErrCode	placeEuroformOnColumn (void);			// 5번 메뉴: 기둥에 유로폼을 배치하는 통합 루틴

#endif
