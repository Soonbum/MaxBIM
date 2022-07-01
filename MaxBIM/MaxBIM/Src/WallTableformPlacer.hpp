#ifndef	__WALL_TABLEFORM_PLACER__
#define __WALL_TABLEFORM_PLACER__

#include "MaxBIM.hpp"

namespace wallTableformPlacerDG {
	// 다이얼로그 항목 인덱스
	enum	idxItems_1_forWallTableform {
		ICON_LAYER_CUSTOM = 3,
		LABEL_LAYER_SETTINGS,
		CHECKBOX_LAYER_COUPLING,

		LABEL_LAYER_SLABTABLEFORM,
		LABEL_LAYER_PROFILE,
		LABEL_LAYER_EUROFORM,
		LABEL_LAYER_RECTPIPE,
		LABEL_LAYER_PINBOLT,
		LABEL_LAYER_WALLTIE,
		LABEL_LAYER_JOIN,
		LABEL_LAYER_HEADPIECE,
		LABEL_LAYER_PROPS,
		LABEL_LAYER_STEELFORM,
		LABEL_LAYER_PLYWOOD,
		LABEL_LAYER_TIMBER,
		LABEL_LAYER_FILLERSP,
		LABEL_LAYER_OUTCORNER_ANGLE,
		LABEL_LAYER_OUTCORNER_PANEL,
		LABEL_LAYER_INCORNER_PANEL,
		LABEL_LAYER_RECTPIPE_HANGER,
		LABEL_LAYER_EUROFORM_HOOK,
		LABEL_LAYER_CROSS_JOINT_BAR,
		LABEL_LAYER_BLUE_CLAMP,
		LABEL_LAYER_BLUE_TIMBER_RAIL,
		LABEL_LAYER_HIDDEN,

		USERCONTROL_LAYER_SLABTABLEFORM,
		USERCONTROL_LAYER_PROFILE,
		USERCONTROL_LAYER_EUROFORM,
		USERCONTROL_LAYER_RECTPIPE,
		USERCONTROL_LAYER_PINBOLT,
		USERCONTROL_LAYER_WALLTIE,
		USERCONTROL_LAYER_JOIN,
		USERCONTROL_LAYER_HEADPIECE,
		USERCONTROL_LAYER_PROPS,
		USERCONTROL_LAYER_STEELFORM,
		USERCONTROL_LAYER_PLYWOOD,
		USERCONTROL_LAYER_TIMBER,
		USERCONTROL_LAYER_FILLERSP,
		USERCONTROL_LAYER_OUTCORNER_ANGLE,
		USERCONTROL_LAYER_OUTCORNER_PANEL,
		USERCONTROL_LAYER_INCORNER_PANEL,
		USERCONTROL_LAYER_RECTPIPE_HANGER,
		USERCONTROL_LAYER_EUROFORM_HOOK,
		USERCONTROL_LAYER_CROSS_JOINT_BAR,
		USERCONTROL_LAYER_BLUE_CLAMP,
		USERCONTROL_LAYER_BLUE_TIMBER_RAIL,
		USERCONTROL_LAYER_HIDDEN,

		BUTTON_AUTOSET
	};

	enum	idxItems_2_forWallTableform {
		LABEL_DESC1_TOPREST = 3,
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

	enum	tableformOrientation_forWallTableformPlacer {
		VERTICAL = 1,
		HORIZONTAL
	};

	enum	objType_forWallTableformPlacer {
		NONE = 1,	// 없음
		TABLEFORM,	// 테이블폼
		EUROFORM,	// 유로폼
		FILLERSP,	// 휠러스페이서
		PLYWOOD,	// 합판
		TIMBER		// 각재
	};

	enum	objCornerType_forWallTableformPlacer {
		NOCORNER = 1,
		INCORNER_PANEL,
		OUTCORNER_PANEL,
		OUTCORNER_ANGLE
	};

	enum	PushPullProps_InstallationType_forWallTableformPlacer {
		PUSHPULLPROPS_NONE = 1,
		PUSHPULLPROPS_INNER,
		PUSHPULLPROPS_OUTER,
		PUSHPULLPROPS_INOUT
	};

	enum insulationDialog {
		LABEL_EXPLANATION_INS = 3,
		USERCONTROL_INSULATION_LAYER,
		LABEL_INSULATION_THK,
		EDITCONTROL_INSULATION_THK,
		CHECKBOX_INS_LIMIT_SIZE,
		LABEL_INS_HORLEN,
		EDITCONTROL_INS_HORLEN,
		LABEL_INS_VERLEN,
		EDITCONTROL_INS_VERLEN,
	};
}

// 단열재
struct insulElemForWallTableform
{
	short	layerInd;		// 레이어 인덱스
	double	thk;			// 두께
	bool	bLimitSize;		// 가로/세로 크기 제한
	double	maxHorLen;		// 가로 최대 길이
	double	maxVerLen;		// 세로 최대 길이
};

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
	double	ang;			// 회전 각도 (단위: Radian, 회전축: Z축)

	short	objType;		// 객체 타입: 없음, 테이블폼, 유로폼, 휠러스페이서, 합판, 각재
	
	int		horLen;			// 가로 길이
	int		verLenBasic;	// 세로 길이 (낮은쪽)
	int		verLenExtra;	// 세로 길이 (높은쪽)

	// 테이블폼 내 유로폼 길이
	int		tableInHor [10];		// 가로 방향
	int		tableInVerBasic [10];	// 세로 방향 (낮은쪽)
	int		tableInVerExtra [10];	// 세로 방향 (높은쪽)
};

// 그리드 각 상/하단 셀 정보
struct MarginCellForWallTableform
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

// 벽면 영역 정보 (통합)
class WallTableformPlacingZone
{
public:
	double	leftBottomX;		// 좌하단 좌표 X
	double	leftBottomY;		// 좌하단 좌표 Y
	double	leftBottomZ;		// 좌하단 좌표 Z

	double	horLen;				// 가로 길이
	double	verLenBasic;		// 세로 길이 (낮은쪽)
	double	verLenExtra;		// 세로 길이 (높은쪽)
	double	ang;				// 회전 각도 (단위: Radian, 회전축: Z축)

	bool	bVertical;			// 방향: 세로방향(true), 가로방향(false)

	double	gap;				// 벽과의 간격

	bool	bSingleSide;		// 단면인가?

	bool	bExtra;				// 높은쪽 모프가 있는가?

	short	typeLcorner;		// 왼쪽 코너 타입	: (1) 없음 (2) 인코너판넬 (3) 아웃코너판넬 (4) 아웃코너앵글
	short	typeRcorner;		// 오른쪽 코너 타입	: (1) 없음 (2) 인코너판넬 (3) 아웃코너판넬 (4) 아웃코너앵글
	double	lenLcorner;			// 왼쪽 인코너판넬/아웃코너판넬 길이
	double	lenRcorner;			// 오른쪽 인코너판넬/아웃코너판넬 길이

	short	tableformType;		// 테이블폼 타입: 타입A (1), 타입B (2), 타입C (3)
	short	propsInstallType;	// Push-Pull Props 설치 타입: 없음 (0), 안쪽 (1), 바깥쪽 (2), 양쪽 (3)

	short	nCellsInHor;		// 수평 방향 셀(유로폼) 개수
	short	nCellsInVerBasic;	// 수직 방향 셀(유로폼) 개수 (낮은쪽)
	short	nCellsInVerExtra;	// 수직 방향 셀(유로폼) 개수 (높은쪽)

	CellForWallTableform		cells [50];				// 셀 배열 (인코너 제외)
	MarginCellForWallTableform	marginCellsBasic [50];	// 셀 배열 (낮은쪽)
	MarginCellForWallTableform	marginCellsExtra [50];	// 셀 배열 (높은쪽)

	double	marginTopBasic;		// 상단 여백 (낮은쪽)
	double	marginTopExtra;		// 상단 여백 (높은쪽)

public:
	int	presetWidth_tableform [40];				// 세로 방향 테이블폼의 너비 모음 (2300 ... 200)
	int	presetHeight_tableform [16];			// 세로 방향 테이블폼의 높이 모음 (6000 ... 1500)

	int	presetWidth_euroform [7];				// 세로 방향 유로폼의 너비 모음 (600 ... 200, 0)
	int	presetHeight_euroform [4];				// 세로 방향 유로폼의 높이 모음 (1200 ... 600, 0)

	int	presetWidth_config_vertical [40][5];	// 세로 방향 테이블폼 내 유로폼의 배열 순서
	int	presetHeight_config_vertical [16][6];	// 세로 방향 테이블폼 내 유로폼의 배열 순서
	int	presetWidth_config_horizontal [16][6];	// 가로 방향 테이블폼 내 유로폼의 배열 순서
	int	presetHeight_config_horizontal [40][5];	// 가로 방향 테이블폼 내 유로폼의 배열 순서

public:
	WallTableformPlacingZone ();	// 기본 생성자
	void	initCells (WallTableformPlacingZone* placingZone, bool bVertical);				// 셀 정보 초기화
	double	getCellPositionLeftBottomX (WallTableformPlacingZone* placingZone, short idx);	// 셀(0-기반 인덱스 번호)의 좌하단 점 위치 X 좌표를 구함
	void	adjustCellsPosition (WallTableformPlacingZone* placingZone);					// 셀 위치를 바르게 교정함
	void	adjustMarginCellsPosition (WallTableformPlacingZone* placingZone);				// 상단 여백 셀 위치를 바르게 교정함
	GSErrCode	placeObjects (WallTableformPlacingZone* placingZone);						// 셀 정보를 기반으로 객체들을 배치함
	GSErrCode	fillRestAreas (WallTableformPlacingZone* placingZone, short idxCell);		// 상단 여백을 유로폼 또는 합판, 각재 등으로 채움
	GSErrCode	placeInsulations (WallTableformPlacingZone* placingZone, InfoWall* infoWall, insulElemForWallTableform* insulElem);		// 벽과 테이블폼 사이에 단열재를 배치함

	void	placeEuroformsOfTableform (WallTableformPlacingZone* placingZone, short idxCell);	// 테이블폼 내 유로폼 배치 (공통)
	void	placeTableformA (WallTableformPlacingZone* placingZone, short idxCell);				// 테이블폼 타입A 배치 (유로폼 제외) - 각파이프 2줄
	void	placeTableformB (WallTableformPlacingZone* placingZone, short idxCell);				// 테이블폼 타입B 배치 (유로폼 제외) - 각파이프 1줄
	void	placeTableformC (WallTableformPlacingZone* placingZone, short idxCell);				// 테이블폼 타입C 배치 (유로폼 제외) - 각파이프 1줄, 십자 조인트 바 활용

public:
	// 다이얼로그 동적 요소 인덱스 번호 저장
	short	EDITCONTROL_GAP;
	short	CHECKBOX_SINGLESIDE;
	short	POPUP_DIRECTION;
	short	POPUP_TABLEFORM_TYPE;
	short	POPUP_PROPS_INSTALL;
	short	EDITCONTROL_REMAIN_WIDTH;
	short	EDITCONTROL_REMAIN_HEIGHT_BASIC;
	short	EDITCONTROL_REMAIN_HEIGHT_EXTRA;
	short	BUTTON_ADD_HOR;
	short	BUTTON_DEL_HOR;
	short	BUTTON_LCORNER;
	short	POPUP_OBJ_TYPE_LCORNER;
	short	EDITCONTROL_WIDTH_LCORNER;
	short	BUTTON_RCORNER;
	short	POPUP_OBJ_TYPE_RCORNER;
	short	EDITCONTROL_WIDTH_RCORNER;
	short	BUTTON_ADD_VER_BASIC;
	short	BUTTON_DEL_VER_BASIC;
	short	BUTTON_ADD_VER_EXTRA;
	short	BUTTON_DEL_VER_EXTRA;

	short	BUTTON_OBJ [50];
	short	POPUP_OBJ_TYPE [50];
	short	POPUP_WIDTH [50];
	short	EDITCONTROL_WIDTH [50];
	short	POPUP_HEIGHT_PRESET;
	short	POPUP_HEIGHT_BASIC [10];
	short	POPUP_HEIGHT_EXTRA [10];

	short	LABEL_TOTAL_WIDTH;
	short	POPUP_WIDTH_IN_TABLE [10];
};

GSErrCode	placeTableformOnWall (void);				// 벽에 테이블폼을 배치하는 통합 루틴 - 전체 통합
short DGCALLBACK wallTableformPlacerHandler1 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);				// 테이블폼/유로폼/휠러스페이서/합판/목재 배치를 위한 다이얼로그 (테이블폼 구성, 요소 방향, 개수 및 길이)
short DGCALLBACK wallTableformPlacerHandler2 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);				// 객체의 레이어를 선택하기 위한 다이얼로그
short DGCALLBACK wallTableformPlacerHandler3_Vertical (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// 테이블폼 세로방향에 대하여 유로폼의 수평 배열을 변경하기 위한 다이얼로그
short DGCALLBACK wallTableformPlacerHandler3_Horizontal (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 테이블폼 가로방향에 대하여 유로폼의 수평 배열을 변경하기 위한 다이얼로그
short DGCALLBACK wallTableformPlacerHandler4 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);				// 벽 상단의 나머지 영역을 유로폼 또는 합판/각재로 채울지 물어보는 다이얼로그
short DGCALLBACK wallTableformPlacerHandler5_Insulation (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 벽과 테이블폼 사이에 단열재를 넣을지 여부를 물어보는 다이얼로그

#endif