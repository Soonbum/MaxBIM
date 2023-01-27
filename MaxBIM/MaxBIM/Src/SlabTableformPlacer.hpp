#ifndef	__SLAB_TABLEFORM_PLACER__
#define __SLAB_TABLEFORM_PLACER__

#include "MaxBIM.hpp"

namespace slabTableformPlacerDG {
	// 다이얼로그 항목 인덱스
	enum	idxItems_1_forSlabBottomTableformPlacer {
		GROUPBOX_PANEL = 3,
		LABEL_PANEL_TYPE,
		LABEL_PANEL_DIRECTION,
		LABEL_PANEL_THICKNESS,
		POPUP_PANEL_TYPE,
		POPUP_PANEL_DIRECTION,
		POPUP_PANEL_THICKNESS,

		GROUPBOX_BEAM,
		LABEL_BEAM_TYPE,
		LABEL_BEAM_DIRECTION,
		LABEL_BEAM_OFFSET_HORIZONTAL,
		LABEL_BEAM_OFFSET_VERTICAL,
		LABEL_BEAM_GAP,
		POPUP_BEAM_TYPE,
		POPUP_BEAM_DIRECTION,
		EDITCONTROL_BEAM_OFFSET_HORIZONTAL,
		EDITCONTROL_BEAM_OFFSET_VERTICAL,
		EDITCONTROL_BEAM_GAP,

		GROUPBOX_GIRDER,
		LABEL_GIRDER_TYPE,
		LABEL_GIRDER_QUANTITY,
		LABEL_GIRDER_OFFSET_HORIZONTAL,
		LABEL_GIRDER_OFFSET_VERTICAL,
		LABEL_GIRDER_GAP,
		POPUP_GIRDER_TYPE,
		POPUP_GIRDER_QUANTITY,
		EDITCONTROL_GIRDER_OFFSET_HORIZONTAL,
		EDITCONTROL_GIRDER_OFFSET_VERTICAL,
		EDITCONTROL_GIRDER_GAP,

		GROUPBOX_SUPPORTING_POST,
		LABEL_POST_TYPE,
		LABEL_POST_GAP,
		POPUP_POST_TYPE,
		POPUP_POST_GAP,
		EDITCONTROL_POST_GAP,

		LABEL_GAP_FROM_SLAB,
		EDITCONTROL_GAP_FROM_SLAB,
		LABEL_ROOM_HEIGHT,
		EDITCONTROL_ROOM_HEIGHT,

		ICON_LAYER,
		LABEL_LAYER_SETTINGS,
		CHECKBOX_LAYER_COUPLING,
		LABEL_LAYER_EUROFORM,
		LABEL_LAYER_CONPANEL,
		LABEL_LAYER_PLYWOOD,
		LABEL_LAYER_TIMBER,
		LABEL_LAYER_MRK,
		LABEL_LAYER_PINBOLT,
		LABEL_LAYER_FITTINGS,
		LABEL_LAYER_GT24_GIRDER,
		LABEL_LAYER_PERI_SUPPORT,
		LABEL_LAYER_STEEL_SUPPORT,

		USERCONTROL_LAYER_EUROFORM,
		USERCONTROL_LAYER_CONPANEL,
		USERCONTROL_LAYER_PLYWOOD,
		USERCONTROL_LAYER_TIMBER,
		USERCONTROL_LAYER_MRK,
		USERCONTROL_LAYER_PINBOLT,
		USERCONTROL_LAYER_FITTINGS,
		USERCONTROL_LAYER_GT24_GIRDER,
		USERCONTROL_LAYER_PERI_SUPPORT,
		USERCONTROL_LAYER_STEEL_SUPPORT,

		BUTTON_AUTOSET
	};

	// 판넬 타입
	enum	idxItems_1_PanelType {
		PANEL_TYPE_NONE,			// 없음
		PANEL_TYPE_CONPANEL,		// 콘판넬
		PANEL_TYPE_PLYWOOD,			// 합판
		PANEL_TYPE_EUROFORM			// 유로폼
	};

	// 장선 타입
	enum	idxItems_1_BeamType {
		BEAM_TYPE_SANSEUNGGAK = 1,	// 산승각
		BEAM_TYPE_TUBAI,			// 투바이
		BEAM_TYPE_GT24				// GT24 거더
	};

	// 멍에제 타입
	enum	idxItems_1_GirderType {
		GIRDER_TYPE_GT24 = 1,		// GT24 거더
		GIRDER_TYPE_SANSEUNGGAK		// 산승각
	};

	// 동바리 타입
	enum	idxItems_1_PostType {
		POST_TYPE_PERI_SUPPORT = 1,	// PERI 동바리
		POST_TYPE_STEEL_SUPPORT		// 강관 동바리
	};

	// 방향
	enum	idxItems_1_Direction {
		HORIZONTAL = 1,				// 가로
		VERTICAL					// 세로
	};

	enum	idxItems_2_forSlabBottomTableformPlacer {
		DG_PREV = 3,
		PUSHBUTTON_ADD_ROW,
		PUSHBUTTON_DEL_ROW,
		PUSHBUTTON_ADD_COL,
		PUSHBUTTON_DEL_COL,

		// 이후에는 그리드 버튼이 배치됨
		AFTER_ALL
	};

	enum	idxItems_3_forSlabBottomTableformPlacer {
		LABEL_OBJ_TYPE	= 3,
		POPUP_OBJ_TYPE,
		LABEL_WIDTH,
		EDITCONTROL_WIDTH,
		POPUP_WIDTH,
		LABEL_HEIGHT,
		EDITCONTROL_HEIGHT,
		POPUP_HEIGHT,
		LABEL_ORIENTATION,
		POPUP_ORIENTATION
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
struct insulElemForSlabTableform
{
	short	layerInd;		// 레이어 인덱스
	double	thk;			// 두께
	bool	bLimitSize;		// 가로/세로 크기 제한
	double	maxHorLen;		// 가로 최대 길이
	double	maxVerLen;		// 세로 최대 길이
};

// 모프 관련 정보
struct InfoMorphForSlabTableform
{
	API_Guid	guid;		// 모프의 GUID
	short		floorInd;	// 층 인덱스
	double		level;		// 모프의 고도
};

// 그리드 각 셀 정보
struct CellForSlabTableform
{
	short	objType;		// 판넬 타입

	double	leftBottomX;	// 좌하단 좌표 X
	double	leftBottomY;	// 좌하단 좌표 Y
	double	leftBottomZ;	// 좌하단 좌표 Z

	double	horLen;			// 가로 길이
	double	verLen;			// 세로 길이
	double	ang;			// 회전 각도 (단위: Radian, 회전축: Z축)
};

// 슬래브 하부 영역 정보
class SlabTableformPlacingZone
{
public:
	// 좌우상하: 슬래브 아래에서 봤을 때의 방향을 따름
	// 슬래브 기하 정보
	double	level;					// 고도
	double	ang;					// 회전 각도 (단위: Radian, 회전축: Z축)

	short	iPanelType;				// 판넬 타입 (1: 콘판넬, 2: 합판, 3: 유로폼)
	short	iPanelDirection;		// 판넬 방향 (1: 가로, 2: 세로)
	char	panelThkStr [10];		// 판넬 두께 (문자열 변수, 11.5T 등)
	double	panelThk;				// 판넬 두께 -- roomHeight와 연관
	
	short	iBeamType;				// 장선 타입 (1: 산승각, 2: 투바이, 3: GT24 거더)
	short	iBeamDirection;			// 장선 방향 (1: 가로, 2: 세로)
	double	beamOffsetHorizontal;	// 장선 오프셋(가로)
	double	beamOffsetVertical;		// 장선 오프셋(세로)
	double	beamGap;				// 장선 간격
	double	beamThk;				// 장선 두께 -- roomHeight와 연관

	short	iGirderType;			// 멍에제 타입 (1: GT24 거더, 2: 산승각)
	short	nGirders;				// 멍에제 개수
	double	girderOffsetHorizontal;	// 멍에제 오프셋(가로) - 유효하지 않음
	double	girderOffsetVertical;	// 멍에제 오프셋(세로) - 유효하지 않음
	double	girderGap;				// 멍에제 간격
	double	girderThk;				// 멍에제 두께 -- roomHeight와 연관

	short	iSuppPostType;			// 동바리 타입 (1: PERI수직재(PERI동바리), 2: 서포트(강관동바리))
	double	suppPostGap;			// 동바리 간격
	double	crossHeadThk;			// 크로스헤드 두께 -- roomHeight와 연관
	double	postHeight;				// 동바리 높이 -- roomHeight와 연관

	double	gap;					// 천장 슬래브와의 간격
	double	roomHeight;				// 천장-바닥간 거리
	bool	bRectangleArea;			// 직사각형 모프이면 true, 꺾인 모서리 모프이면 false

	double	borderHorLen;			// 최외곽 가로 길이
	double	borderVerLen;			// 최외곽 세로 길이

	API_Coord3D		center;			// 최외곽 중심 점
	API_Coord3D		leftBottom;		// 최외곽 좌하단 점
	API_Coord3D		rightTop;		// 최외곽 우상단 점

	double	initCellHorLen;			// 셀 초기 너비
	double	initCellVerLen;			// 셀 초기 높이

	double	cellArrayWidth;			// 셀 배열 전체 너비
	double	cellArrayHeight;		// 셀 배열 전체 높이

	short	nHorCells;				// 가로 방향 셀 개수
	short	nVerCells;				// 세로 방향 셀 개수

	double	marginLeft;				// 왼쪽 여백 길이
	double	marginRight;			// 오른쪽 여백 길이
	double	marginTop;				// 위쪽 여백 길이
	double	marginBottom;			// 아래쪽 여백 길이

	double	leftBottomX;			// 셀 시작 좌표 X
	double	leftBottomY;			// 셀 시작 좌표 Y
	double	leftBottomZ;			// 셀 시작 좌표 Z

	CellForSlabTableform	cells [50][50];		// 셀 정보 (0행은 하단, 0열은 좌측)

public:
	void		initCells (SlabTableformPlacingZone* placingZone);			// Cell 배열을 초기화함
	void		alignPlacingZone (SlabTableformPlacingZone* placingZone);	// Cell 배열의 위치를 조정함
	GSErrCode	fillCellAreas (void);										// 셀 배열 공간 채우기
	GSErrCode	fillMarginAreas (void);										// 여백 공간 채우기 (합판)
	GSErrCode	placeInsulations (void);									// 단열재 배치
	GSErrCode	placeBeams (void);											// 장선 배치
	GSErrCode	placeGirdersAndPosts (void);								// 멍에제, 동바리 배치

public:
	// 다이얼로그 동적 요소 인덱스 번호 저장
	short	CELL_BUTTON [50][50];
	short	EDITCONTROL_MARGIN_LEFT;
	short	EDITCONTROL_MARGIN_RIGHT;
	short	EDITCONTROL_MARGIN_TOP;
	short	EDITCONTROL_MARGIN_BOTTOM;
};

// 테이블폼 슬래브 하부 배치 함수
GSErrCode	placeTableformOnSlabBottom (void);	// 슬래브 하부에 테이블폼을 배치하는 통합 루틴
short DGCALLBACK slabBottomTableformPlacerHandler1 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 1차 배치를 위한 질의를 요청하는 1차 다이얼로그
short DGCALLBACK slabBottomTableformPlacerHandler2 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 1차 배치 후 수정을 요청하는 2차 다이얼로그
short DGCALLBACK slabBottomTableformPlacerHandler3 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 2차 다이얼로그에서 각 셀의 객체 타입을 변경하기 위한 3차 다이얼로그
short DGCALLBACK slabBottomTableformPlacerHandler4_Insulation (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 슬래브 하부의 간격에 단열재를 배치함

#endif