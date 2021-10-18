#ifndef	__WALL_TABLEFORM_PLACER__
#define __WALL_TABLEFORM_PLACER__

#include "MaxBIM.hpp"

namespace wallTableformPlacerDG {
	// 다이얼로그 항목 인덱스
	enum	idxItems_2_forWallTableformPlacer_typeA {
		DG_PREV = 3,
		POPUP_TYPE_SELECTOR,
		LABEL_HEIGHT,
		EDITCONTROL_HEIGHT,
		LABEL_ERR_MESSAGE,
		LABEL_WIDTH,
		EDITCONTROL_WIDTH,

		LABEL_GAP_LENGTH,
		EDITCONTROL_GAP_LENGTH,
		LABEL_FILL_SIDE,
		RADIOBUTTON_DOUBLE,
		RADIOBUTTON_SINGLE,

		ICON_LAYER,
		LABEL_LAYER_SETTINGS,
		CHECKBOX_LAYER_COUPLING,
		LABEL_LAYER_EUROFORM,
		LABEL_LAYER_RECTPIPE,
		LABEL_LAYER_PINBOLT,
		LABEL_LAYER_WALLTIE,
		LABEL_LAYER_JOIN,
		LABEL_LAYER_HEADPIECE,
		LABEL_LAYER_PLYWOOD,
		LABEL_LAYER_WOOD,
		LABEL_LAYER_HIDDEN,

		USERCONTROL_LAYER_EUROFORM,
		USERCONTROL_LAYER_RECTPIPE,
		USERCONTROL_LAYER_PINBOLT,
		USERCONTROL_LAYER_WALLTIE,
		USERCONTROL_LAYER_JOIN,
		USERCONTROL_LAYER_HEADPIECE,
		USERCONTROL_LAYER_PLYWOOD,
		USERCONTROL_LAYER_WOOD,
		USERCONTROL_LAYER_HIDDEN,

		BUTTON_AUTOSET
	};

	enum	idxItems_2_forWallTableformPlacer_typeB {
		LABEL_LAYER_RECTPIPE_HANGER = LABEL_LAYER_PINBOLT,				// 기존 핀볼트세트 레이어 활용
		LABEL_LAYER_EUROFORM_HOOK = LABEL_LAYER_WALLTIE,				// 기존 벽체타이 레이어 활용
		USERCONTROL_LAYER_RECTPIPE_HANGER = USERCONTROL_LAYER_PINBOLT,
		USERCONTROL_LAYER_EUROFORM_HOOK = USERCONTROL_LAYER_WALLTIE
	};

	enum	idxItems_3_forWallTableformPlacer {
		LABEL_DESC1_TOPREST		= 3,
		LABEL_HEIGHT_TOPREST,
		EDITCONTROL_HEIGHT_TOPREST,
		LABEL_DESC2_TOPREST,
		LABEL_UP_TOPREST,
		LABEL_ARROWUP_TOPREST,
		LABEL_DOWN_TOPREST,
		CHECKBOX_FORM_ONOFF_1_TOPREST,
		CHECKBOX_FORM_ONOFF_2_TOPREST,
		LABEL_PLYWOOD_TOPREST,
		CHECKBOX_SET_STANDARD_1_TOPREST,
		CHECKBOX_SET_STANDARD_2_TOPREST,
		POPUP_EUROFORM_WIDTH_OPTIONS_1_TOPREST,
		POPUP_EUROFORM_WIDTH_OPTIONS_2_TOPREST,
		EDITCONTROL_EUROFORM_WIDTH_OPTIONS_1_TOPREST,
		EDITCONTROL_EUROFORM_WIDTH_OPTIONS_2_TOPREST,
		EDITCONTROL_PLYWOOD_TOPREST
	};

	enum	idxItems_1_forWallTableformPlacerCustomOrientation {
		VERTICAL_DIRECTION = 1,
		HORIZONTAL_DIRECTION
	};

	enum	idxItems_1_forWallTableformPlacerCustom {
		POPUP_TYPE_SELECTOR_CUSTOM = 3,
		LABEL_TABLEFORM_ORIENTATION_CUSTOM,
		POPUP_TABLEFORM_ORIENTATION_CUSTOM,
		LABEL_TOTAL_WIDTH_CUSTOM,
		EDITCONTROL_TOTAL_WIDTH_CUSTOM,
		LABEL_REMAIN_WIDTH_CUSTOM,
		EDITCONTROL_REMAIN_WIDTH_CUSTOM,
		LABEL_TOTAL_HEIGHT_CUSTOM,
		EDITCONTROL_TOTAL_HEIGHT_CUSTOM,
		LABEL_REMAIN_HEIGHT_CUSTOM,
		EDITCONTROL_REMAIN_HEIGHT_CUSTOM,
		BUTTON_ADD_COL_CUSTOM,
		BUTTON_DEL_COL_CUSTOM,
		BUTTON_ADD_ROW_CUSTOM,
		BUTTON_DEL_ROW_CUSTOM,
		REST_ITEM_START_CUSTOM
	};

	enum	idxItems_1_forLayerSelection {
		ICON_LAYER_CUSTOM = 3,
		LABEL_LAYER_SETTINGS_CUSTOM,
		CHECKBOX_LAYER_COUPLING_CUSTOM,

		LABEL_LAYER_SLABTABLEFORM_CUSTOM,
		LABEL_LAYER_PROFILE_CUSTOM,
		LABEL_LAYER_EUROFORM_CUSTOM,
		LABEL_LAYER_RECTPIPE_CUSTOM,
		LABEL_LAYER_PINBOLT_CUSTOM,
		LABEL_LAYER_WALLTIE_CUSTOM,
		LABEL_LAYER_JOIN_CUSTOM,
		LABEL_LAYER_HEADPIECE_CUSTOM,
		LABEL_LAYER_STEELFORM_CUSTOM,
		LABEL_LAYER_PLYWOOD_CUSTOM,
		LABEL_LAYER_FILLERSP_CUSTOM,
		LABEL_LAYER_OUTCORNER_ANGLE_CUSTOM,
		LABEL_LAYER_OUTCORNER_PANEL_CUSTOM,
		LABEL_LAYER_INCORNER_PANEL_CUSTOM,
		LABEL_LAYER_RECTPIPE_HANGER_CUSTOM,
		LABEL_LAYER_EUROFORM_HOOK_CUSTOM,
		LABEL_LAYER_HIDDEN_CUSTOM,

		USERCONTROL_LAYER_SLABTABLEFORM_CUSTOM,
		USERCONTROL_LAYER_PROFILE_CUSTOM,
		USERCONTROL_LAYER_EUROFORM_CUSTOM,
		USERCONTROL_LAYER_RECTPIPE_CUSTOM,
		USERCONTROL_LAYER_PINBOLT_CUSTOM,
		USERCONTROL_LAYER_WALLTIE_CUSTOM,
		USERCONTROL_LAYER_JOIN_CUSTOM,
		USERCONTROL_LAYER_HEADPIECE_CUSTOM,
		USERCONTROL_LAYER_STEELFORM_CUSTOM,
		USERCONTROL_LAYER_PLYWOOD_CUSTOM,
		USERCONTROL_LAYER_FILLERSP_CUSTOM,
		USERCONTROL_LAYER_OUTCORNER_ANGLE_CUSTOM,
		USERCONTROL_LAYER_OUTCORNER_PANEL_CUSTOM,
		USERCONTROL_LAYER_INCORNER_PANEL_CUSTOM,
		USERCONTROL_LAYER_RECTPIPE_HANGER_CUSTOM,
		USERCONTROL_LAYER_EUROFORM_HOOK_CUSTOM,
		USERCONTROL_LAYER_HIDDEN_CUSTOM,

		BUTTON_AUTOSET_CUSTOM
	};
}

// 모프 관련 정보
struct InfoMorphForWallTableform
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
struct CellForWallTableform
{
	double	leftBottomX;	// 좌하단 좌표 X
	double	leftBottomY;	// 좌하단 좌표 Y
	double	leftBottomZ;	// 좌하단 좌표 Z

	double	horLen;			// 가로 길이 (400~2300, 50 간격)
	double	verLen;			// 세로 길이 (1500~6000, 300 간격)
	double	ang;			// 회전 각도 (단위: Radian, 회전축: Z축)
};

// 그리드 각 상단 셀 정보
struct UpperCellForWallTableform
{
	double	leftBottomX;	// 좌하단 좌표 X
	double	leftBottomY;	// 좌하단 좌표 Y
	double	leftBottomZ;	// 좌하단 좌표 Z

	double	ang;			// 회전 각도 (단위: Radian, 회전축: Z축)

	bool	bFill;			// 채우기 여부

	bool	bEuroform1;				// 1단 유로폼 여부
	bool	bEuroformStandard1;		// 1단 유로폼이 규격폼인지 여부
	double	formWidth1;				// 1단 유로폼의 폭
	bool	bEuroform2;				// 2단 유로폼 여부
	bool	bEuroformStandard2;		// 2단 유로폼이 규격폼인지 여부
	double	formWidth2;				// 2단 유로폼의 폭
};

// 배치 정보
struct	placementInfoForWallTableform
{
	short	nHorEuroform;	// 수평 방향 유로폼 개수
	short	nVerEuroform;	// 수직 방향 유로폼 개수

	double	width [7];		// 수평 방향 각 유로폼 너비
	double	height [7];		// 수직 방향 각 유로폼 높이
};

// 벽면 영역 정보
class WallTableformPlacingZone
{
public:
	double	leftBottomX;	// 좌하단 좌표 X
	double	leftBottomY;	// 좌하단 좌표 Y
	double	leftBottomZ;	// 좌하단 좌표 Z

	double	horLen;			// 가로 길이
	double	verLen;			// 세로 길이
	double	ang;			// 회전 각도 (단위: Radian, 회전축: Z축)

	short	type;			// 타입A(1), 타입B(2)

	bool	bDoubleSide;	// 양면이면 true, 단면이면 false

	double	gap;			// 벽과의 간격

	double	remainWidth;	// 남은 길이

	CellForWallTableform		cells [50];			// 테이블폼 셀 정보
	UpperCellForWallTableform	upperCells [50];	// 테이블폼 상단의 유로폼 또는 합판 셀 정보
	short		nCells;								// 테이블폼 셀 개수 (수평 방향으로)
	double		marginTop;							// 상단 여백 높이

	short		orientation;						// 세로방향 (VERTICAL_DIRECTION), 가로방향 (HORIZONTAL_DIRECTION)
	short		nCells_vertical;					// 테이블폼 셀 개수 (수직 방향으로) - 커스텀 전용
	CellForWallTableform		customCells [5][5];	// 테이블폼 셀 정보 - 커스텀 전용

	// 테이블폼 개수 (각각은 너비 400~2300의 테이블폼을 의미함) - 세로 방향
	short	n400w;
	short	n450w;
	short	n500w;
	short	n600w;
	short	n650w;
	short	n700w;
	short	n750w;
	short	n800w;
	short	n850w;
	short	n900w;
	short	n950w;
	short	n1000w;
	short	n1050w;
	short	n1100w;
	short	n1150w;
	short	n1200w;
	short	n1250w;
	short	n1300w;
	short	n1350w;
	short	n1400w;
	short	n1450w;
	short	n1500w;
	short	n1550w;
	short	n1600w;
	short	n1650w;
	short	n1700w;
	short	n1750w;
	short	n1800w;
	short	n1850w;
	short	n1900w;
	short	n1950w;
	short	n2000w;
	short	n2050w;
	short	n2100w;
	short	n2150w;
	short	n2200w;
	short	n2250w;
	short	n2300w;

	// 테이블폼 개수 (각각은 너비 1500~6000의 테이블폼을 의미함) - 가로 방향
	short	n1500h;
	short	n1800h;
	short	n2100h;
	short	n2400h;
	short	n2700h;
	short	n3000h;
	short	n3300h;
	short	n3600h;
	short	n3900h;
	short	n4200h;
	short	n4500h;
	short	n4800h;
	short	n5100h;
	short	n5400h;
	short	n5700h;
	short	n6000h;

	// 타입B의 경우, 수평바 양 끝으로부터 수직바의 위치 오프셋
	short	nVerticalBar;
	double	verticalBarLeftOffset;
	double	verticalBarRightOffset;

public:
	void		initCells (WallTableformPlacingZone* placingZone);													// Cell 배열을 초기화함
	double		getCellPositionLeftBottomX (WallTableformPlacingZone *placingZone, short idx);						// 해당 셀의 좌하단 좌표X 위치를 리턴
	GSErrCode	placeTableformOnWall_Vertical_TypeA (CellForWallTableform cell);									// 테이블폼 배치하기 - 세로 방향 (타입A)
	GSErrCode	placeTableformOnWall_Vertical_TypeB (CellForWallTableform cell);									// 테이블폼 배치하기 - 세로 방향 (타입B)
	GSErrCode	placeTableformOnWall_Vertical_TypeC (CellForWallTableform cell);									// 테이블폼 배치하기 - 세로 방향 (타입C)
	GSErrCode	placeTableformOnWall_Horizontal_TypeA (CellForWallTableform cell);									// 테이블폼 배치하기 - 가로 방향 (타입A)
	GSErrCode	placeTableformOnWall_Horizontal_TypeB (CellForWallTableform cell);									// 테이블폼 배치하기 - 가로 방향 (타입B)
	GSErrCode	placeTableformOnWall_Horizontal_TypeC (CellForWallTableform cell);									// 테이블폼 배치하기 - 가로 방향 (타입C)
	GSErrCode	placeTableformOnWall_Custom_TypeA ();																// 테이블폼 배치하기 - 커스텀 (타입A)
	GSErrCode	placeTableformOnWall_Custom_TypeB ();																// 테이블폼 배치하기 - 커스텀 (타입B)
	GSErrCode	placeTableformOnWall_Custom_TypeC ();																// 테이블폼 배치하기 - 커스텀 (타입C)
	GSErrCode	placeTableformOnWall_Vertical (CellForWallTableform cell, UpperCellForWallTableform upperCell);		// 테이블폼 상단 배치하기 - 세로 방향
	GSErrCode	placeTableformOnWall_Horizontal (CellForWallTableform cell, UpperCellForWallTableform upperCell);	// 테이블폼 상단 배치하기 - 가로 방향

	API_Guid	placeUFOM (Euroform params);						// 배치: 유로폼
	API_Guid	placeUFOM_up (Euroform params);						// 배치: 유로폼 (상부)
	API_Guid	placeSPIP (SquarePipe params);						// 배치: 비계 파이프
	API_Guid	placeSPIP_sideHole (SquarePipe params);				// 배치: 비계 파이프 (측면 홀타공)
	API_Guid	placeSPIP_frontHole (SquarePipe params);			// 배치: 비계 파이프 (정면 홀타공)
	API_Guid	placePINB (PinBoltSet params);						// 배치: 핀볼트 세트
	API_Guid	placeTIE  (WallTie params);							// 배치: 벽체 타이
	API_Guid	placeCLAM (CrossClamp params);						// 배치: 직교 클램프
	API_Guid	placePUSH_ver (HeadpieceOfPushPullProps params);	// 배치: 헤드피스 (세로 방향: 타입 A)
	API_Guid	placePUSH_hor (HeadpieceOfPushPullProps params);	// 배치: 헤드피스 (가로 방향: 타입 B)
	API_Guid	placeJOIN (MetalFittings params);					// 배치: 결합철물
	API_Guid	placePLYW (Plywood params);							// 배치: 합판
	API_Guid	placeTIMB (Wood params);							// 배치: 목재
	API_Guid	placeJOIN2 (MetalFittings params);					// 배치: 사각파이프 연결철물
	API_Guid	placePUSH2_ver (HeadpieceOfPushPullProps params);	// 배치: 빔조인트용 Push-Pull Props (세로 방향: 타입 A)
	API_Guid	placePUSH2_hor (HeadpieceOfPushPullProps params);	// 배치: 빔조인트용 Push-Pull Props (가로 방향: 타입 B)
	API_Guid	placeHOOK (EuroformHook params);					// 배치: 유로폼 후크
	API_Guid	placeHANG (RectPipeHanger params);					// 배치: 각파이프 행거
	API_Guid	placeHOLE (API_Guid guid_Target, Cylinder operator_Object);		// 타공을 위한 기둥 객체를 배치하고 숨김, "원통 19" 객체를 이용함
};

GSErrCode	placeTableformOnWall_Vertical (void);		// 벽에 테이블폼을 배치하는 통합 루틴 - 세로 방향
short DGCALLBACK wallTableformPlacerHandler1_Vertical (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// 선호하는 테이블폼 너비를 선택하기 위한 다이얼로그 - 세로 방향
short DGCALLBACK wallTableformPlacerHandler2_Vertical (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// 테이블폼 배치를 위한 질의를 요청하는 다이얼로그 - 세로 방향
short DGCALLBACK wallTableformPlacerHandler3_Vertical (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// 벽 상단의 합판/목재 영역을 유로폼으로 채울지 물어보는 3차 다이얼로그 - 세로 방향

GSErrCode	placeTableformOnWall_Horizontal (void);		// 벽에 테이블폼을 배치하는 통합 루틴 - 가로 방향
short DGCALLBACK wallTableformPlacerHandler1_Horizontal (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 선호하는 테이블폼 너비를 선택하기 위한 다이얼로그 - 가로 방향
short DGCALLBACK wallTableformPlacerHandler2_Horizontal (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 테이블폼 배치를 위한 질의를 요청하는 다이얼로그 - 가로 방향
short DGCALLBACK wallTableformPlacerHandler3_Horizontal (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 벽 상단의 합판/목재 영역을 유로폼으로 채울지 물어보는 3차 다이얼로그 - 가로 방향

GSErrCode	placeTableformOnWall_Custom (void);			// 벽에 테이블폼을 배치하는 통합 루틴 - 커스텀
short DGCALLBACK wallTableformPlacerHandler1_Custom (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// 테이블폼 맞춤 제작을 위한 다이얼로그 (테이블폼 방향, 유로폼 가로/세로 개수 및 길이) - 커스텀
short DGCALLBACK wallTableformPlacerHandler2_Custom (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// 객체의 레이어를 선택하기 위한 다이얼로그

#endif