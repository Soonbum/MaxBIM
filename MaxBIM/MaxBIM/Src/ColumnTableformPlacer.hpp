#ifndef	__COLUMN_TABLEFORM_PLACER__
#define __COLUMN_TABLEFORM_PLACER__

#include "MaxBIM.hpp"

namespace columnTableformPlacerDG {
	// 객체 타입
	enum	libPartObjType {
		NONE,			// 없음
		EUROFORM,		// 유로폼v2.0
		OUTPANEL,		// 아웃코너판넬v1.0
		OUTANGLE,		// 아웃코너앵글v1.0
		FILLERSP		// 휠러스페이서v1.0
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
	enum	idxItems_1_forColumnTableformPlacer {
		LABEL_COLUMN_SECTION		= 3,

		ICON_COLUMN_SECTION_OUTCORNER_PANEL,
		ICON_COLUMN_SECTION_OUTCORNER_ANGLE,

		LABEL_OUTCORNER,
		RADIO_OUTCORNER_PANEL,
		RADIO_OUTCORNER_ANGLE,

		LABEL_COLUMN_BAND_TYPE,
		RADIO_COLUMN_BAND_1,
		RADIO_COLUMN_BAND_2,

		LABEL_TABLEFORM_TYPE,
		POPUP_TABLEFORM_TYPE,
		
		EDITCONTROL_TOP_1,
		EDITCONTROL_TOP_2,
		EDITCONTROL_TOP_3,
		EDITCONTROL_TOP_4,
		EDITCONTROL_TOP_5,
		EDITCONTROL_LEFT_1,
		EDITCONTROL_LEFT_2,
		EDITCONTROL_LEFT_3,
		EDITCONTROL_LEFT_4,
		EDITCONTROL_LEFT_5,
		EDITCONTROL_RIGHT_1,
		EDITCONTROL_RIGHT_2,
		EDITCONTROL_RIGHT_3,
		EDITCONTROL_RIGHT_4,
		EDITCONTROL_RIGHT_5,
		EDITCONTROL_BOTTOM_1,
		EDITCONTROL_BOTTOM_2,
		EDITCONTROL_BOTTOM_3,
		EDITCONTROL_BOTTOM_4,
		EDITCONTROL_BOTTOM_5,

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
		CHECKBOX_LAYER_COUPLING,
		LABEL_LAYER_EUROFORM,
		LABEL_LAYER_FILLERSP,
		LABEL_LAYER_OUTCORNER_PANEL,
		LABEL_LAYER_OUTCORNER_ANGLE,
		LABEL_LAYER_SQUARE_PIPE,
		LABEL_LAYER_PINBOLT,
		LABEL_LAYER_HANGER,
		LABEL_LAYER_HEADPIECE,
		LABEL_LAYER_COLUMN_BAND,
		LABEL_LAYER_PLYWOOD,

		USERCONTROL_LAYER_EUROFORM,
		USERCONTROL_LAYER_FILLERSP,
		USERCONTROL_LAYER_OUTCORNER_PANEL,
		USERCONTROL_LAYER_OUTCORNER_ANGLE,
		USERCONTROL_LAYER_SQUARE_PIPE,
		USERCONTROL_LAYER_PINBOLT,
		USERCONTROL_LAYER_HANGER,
		USERCONTROL_LAYER_HEADPIECE,
		USERCONTROL_LAYER_COLUMN_BAND,
		USERCONTROL_LAYER_PLYWOOD,

		BUTTON_AUTOSET
	};

	enum	idxItems_2_forColumnTableformPlacer {
		DG_UPDATE_BUTTON	= 3,
		DG_PREV,
		LABEL_COLUMN_SIDE,
		AFTER_ALL
	};
	
	enum	idxItems_3_forColumnTableformPlacer {
		LABEL_OBJ_TYPE			= 3,
		POPUP_OBJ_TYPE,
		CHECKBOX_SET_STANDARD,
		LABEL_LENGTH,
		EDITCONTROL_LENGTH,
		POPUP_LENGTH
	};
}

// 보 관련 정보
struct InfoBeamForColumnTableform
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
struct InfoMorphForColumnTableform
{
	API_Guid	guid;		// 모프의 GUID
	short		floorInd;	// 층 인덱스
	double		level;		// 모프의 고도
	double		height;		// 모프의 높이
};

// 그리드 각 셀 정보
struct CellForColumnTableform
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

	bool	bStandardEuroform;		// 규격폼인가? (유로폼인 경우)
};

// 기둥 영역 정보
class ColumnTableformPlacingZone
{
public:
	short	tableformType;	// 테이블폼 타입: 타입A (1), 타입B (2)

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
	InfoBeamForColumnTableform	beams [4];		// 간섭 보 정보

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
	CellForColumnTableform	cellsLT [20];		// 좌상단 셀
	CellForColumnTableform	cellsRT [20];		// 우상단 셀
	CellForColumnTableform	cellsLB [20];		// 좌하단 셀
	CellForColumnTableform	cellsRB [20];		// 우하단 셀
	CellForColumnTableform	cellsT1 [20];		// 상단1 셀 (왼쪽)
	CellForColumnTableform	cellsT2 [20];		// 상단2 셀 (가운데)
	CellForColumnTableform	cellsT3 [20];		// 상단3 셀 (오른쪽)
	CellForColumnTableform	cellsL1 [20];		// 좌측1 셀 (위)
	CellForColumnTableform	cellsL2 [20];		// 좌측2 셀 (가운데)
	CellForColumnTableform	cellsL3 [20];		// 좌측3 셀 (아래)
	CellForColumnTableform	cellsR1 [20];		// 우측1 셀 (위)
	CellForColumnTableform	cellsR2 [20];		// 우측2 셀 (가운데)
	CellForColumnTableform	cellsR3 [20];		// 우측3 셀 (아래)
	CellForColumnTableform	cellsB1 [20];		// 하단1 셀 (왼쪽)
	CellForColumnTableform	cellsB2 [20];		// 하단2 셀 (가운데)
	CellForColumnTableform	cellsB3 [20];		// 하단3 셀 (오른쪽)

	// 아웃코너판넬/아웃코너앵글
	bool	bUseOutcornerPanel;		// true이면 아웃코너판넬, false이면 아웃코너앵글

	// 기둥밴드, 웰라
	short	typeOfColumnBand;

	// 수직 방향으로의 셀 개수
	short	nCells;

public:
	void		initCells (ColumnTableformPlacingZone* placingZone);					// Cell 배열을 초기화함
	void		addTopCell (ColumnTableformPlacingZone* target_zone);					// 꼭대기에 셀 추가
	void		delTopCell (ColumnTableformPlacingZone* target_zone);					// 꼭대기의 셀 삭제
	void		alignPlacingZone_soleColumn (ColumnTableformPlacingZone* placingZone);	// Cell 정보가 변경됨에 따라 파편화된 위치를 재조정함
	GSErrCode	placeBasicObjects_soleColumn (ColumnTableformPlacingZone* placingZone);	// 유로폼/아웃코너판넬/아웃코너앵글/휠러스페이서 배치
	GSErrCode	placeRestObjectsA_soleColumn (ColumnTableformPlacingZone* placingZone);	// 비계파이프, 핀볼트세트/각파이프행거, 헤드피스 배치 (타입A)
	GSErrCode	placeRestObjectsB_soleColumn (ColumnTableformPlacingZone* placingZone);	// 비계파이프, 핀볼트세트/각파이프행거, 헤드피스 배치 (타입B)
	GSErrCode	fillRestAreas_soleColumn (ColumnTableformPlacingZone* placingZone);		// 유로폼/아웃코너판넬을 채운 후 자투리 공간 채우기 (나머지는 합판으로 채움)
};

// 유로폼 기둥 배치 함수
GSErrCode	placeTableformOnColumn (void);	// 기둥에 테이블폼을 배치하는 통합 루틴
short DGCALLBACK columnTableformPlacerHandler_soleColumn_1 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// 1차 배치를 위한 질의를 요청하는 1차 다이얼로그
short DGCALLBACK columnTableformPlacerHandler_soleColumn_2 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// 1차 배치 후 수정을 요청하는 2차 다이얼로그
short DGCALLBACK columnTableformPlacerHandler_soleColumn_3 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// 2차 다이얼로그에서 각 셀의 객체 타입을 변경하기 위한 3차 다이얼로그

#endif
