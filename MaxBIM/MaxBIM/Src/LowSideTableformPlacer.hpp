#ifndef	__LOW_SIDE_TABLEFORM_PLACER__
#define __LOW_SIDE_TABLEFORM_PLACER__

#include "MaxBIM.hpp"

namespace lowSideTableformPlacerDG {
	enum	objCellType_forLowSideTableformPlacer {
		NONE = 1,	// 없음
		TABLEFORM,	// 테이블폼
		EUROFORM,	// 유로폼
		FILLERSP,	// 휠러스페이서
		PLYWOOD,	// 합판
		TIMBER		// 각재
	};

	enum	objCornerType_forLowSideTableformPlacer {
		NOCORNER = 1,
		INCORNER_PANEL,
		OUTCORNER_PANEL,
		OUTCORNER_ANGLE
	};
}

// 모프 관련 정보
struct InfoMorphForLowSideTableform
{
	API_Guid	guid;		// 모프의 GUID

	double	leftBottomX;	// 좌하단 좌표 X
	double	leftBottomY;	// 좌하단 좌표 Y
	double	leftBottomZ;	// 좌하단 좌표 Z

	double	rightTopX;		// 우상단 좌표 X
	double	rightTopY;		// 우상단 좌표 Y
	double	rightTopZ;		// 우상단 좌표 Z

	double	horLen;			// 가로 길이
	double	verLen;			// 세로 길이
	double	ang;			// 회전 각도 (단위: Degree, 회전축: Z축)
};

// 그리드 각 셀 정보
struct CellForLowSideTableform
{
	double	leftBottomX;	// 좌하단 좌표 X
	double	leftBottomY;	// 좌하단 좌표 Y
	double	leftBottomZ;	// 좌하단 좌표 Z
	double	ang;			// 회전 각도 (단위: Radian, 회전축: Z축)

	short	objType;		// 객체 타입: 없음, 테이블폼, 유로폼, 휠러스페이서, 합판, 각재
	
	int		horLen;			// 가로 길이
	int		verLen;			// 세로 길이

	// 테이블폼 내 유로폼 길이
	int		tableInHor [10];	// 가로 방향
};

// 낮은 측면 영역 정보
class LowSideTableformPlacingZone
{
public:
	double	leftBottomX;		// 좌하단 좌표 X
	double	leftBottomY;		// 좌하단 좌표 Y
	double	leftBottomZ;		// 좌하단 좌표 Z

	double	horLen;				// 가로 길이
	double	verLen;				// 세로 길이
	double	ang;				// 회전 각도 (단위: Radian, 회전축: Z축)

	bool	bVertical;			// 방향: 세로방향(true), 가로방향(false)

	short	typeLcorner;		// 왼쪽 코너 타입	: (1) 없음 (2) 인코너판넬 (3) 아웃코너판넬 (4) 아웃코너앵글
	short	typeRcorner;		// 오른쪽 코너 타입	: (1) 없음 (2) 인코너판넬 (3) 아웃코너판넬 (4) 아웃코너앵글
	double	lenLcorner;			// 왼쪽 인코너판넬/아웃코너판넬 길이
	double	lenRcorner;			// 오른쪽 인코너판넬/아웃코너판넬 길이

	short	tableformType;		// 테이블폼 타입: 타입A (1), 타입B (2), 타입C (3)

	short	nCellsInHor;		// 수평 방향 셀(유로폼) 개수

	CellForLowSideTableform		cells [50];				// 셀 배열 (인코너 제외)

public:
	//int	presetWidthVertical_tableform [40];		// 세로 방향 테이블폼의 너비 모음 (3600 ... 200)
	//int	presetWidthHorizontal_tableform [16];	// 가로 방향 테이블폼의 높이 모음 (3600 ... 1200)

	//int	presetWidth_config_vertical [40][5];	// 세로 방향 테이블폼 내 유로폼의 배열 순서
	//int	presetWidth_config_horizontal [16][6];	// 가로 방향 테이블폼 내 유로폼의 배열 순서

public:
	//WallTableformPlacingZone ();	// 기본 생성자
	//void	initCells (WallTableformPlacingZone* placingZone, bool bVertical);				// 셀 정보 초기화
	//double	getCellPositionLeftBottomX (WallTableformPlacingZone* placingZone, short idx);	// 셀(0-기반 인덱스 번호)의 좌하단 점 위치 X 좌표를 구함
	//void	adjustCellsPosition (WallTableformPlacingZone* placingZone);					// 셀 위치를 바르게 교정함
	//GSErrCode	placeObjects (WallTableformPlacingZone* placingZone);						// 셀 정보를 기반으로 객체들을 배치함

	//void	placeEuroformsOfTableform (WallTableformPlacingZone* placingZone, short idxCell);	// 테이블폼 내 유로폼 배치 (공통)
	//void	placeTableformA (WallTableformPlacingZone* placingZone, short idxCell);				// 테이블폼 타입A 배치 (유로폼 제외) - 각파이프 2줄
	//void	placeTableformB (WallTableformPlacingZone* placingZone, short idxCell);				// 테이블폼 타입B 배치 (유로폼 제외) - 각파이프 1줄
	//void	placeTableformC (WallTableformPlacingZone* placingZone, short idxCell);				// 테이블폼 타입C 배치 (유로폼 제외) - 각파이프 1줄, 십자 조인트 바 활용

public:
	// 다이얼로그 동적 요소 인덱스 번호 저장
	//short	EDITCONTROL_GAP;
	//short	CHECKBOX_SINGLESIDE;
	//short	POPUP_DIRECTION;
	//short	POPUP_TABLEFORM_TYPE;
	//short	EDITCONTROL_REMAIN_WIDTH;
	//short	EDITCONTROL_REMAIN_HEIGHT_BASIC;
	//short	EDITCONTROL_REMAIN_HEIGHT_EXTRA;
	//short	BUTTON_ADD_HOR;
	//short	BUTTON_DEL_HOR;
	//short	CHECKBOX_LINCORNER;
	//short	EDITCONTROL_LINCORNER;
	//short	CHECKBOX_RINCORNER;
	//short	EDITCONTROL_RINCORNER;
	//short	BUTTON_ADD_VER_BASIC;
	//short	BUTTON_DEL_VER_BASIC;
	//short	BUTTON_ADD_VER_EXTRA;
	//short	BUTTON_DEL_VER_EXTRA;

	//short	BUTTON_OBJ [50];
	//short	POPUP_OBJ_TYPE [50];
	//short	POPUP_WIDTH [50];
	//short	EDITCONTROL_WIDTH [50];
	//short	POPUP_HEIGHT_PRESET;
	//short	POPUP_HEIGHT_BASIC [10];
	//short	POPUP_HEIGHT_EXTRA [10];

	//short	LABEL_TOTAL_WIDTH;
	//short	POPUP_WIDTH_IN_TABLE [4];
};

GSErrCode	placeTableformOnLowSide (void);				// 낮은 슬래브 측면에 테이블폼을 배치하는 통합 루틴

#endif