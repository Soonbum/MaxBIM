#ifndef	__SLAB_TABLEFORM_PLACER__
#define __SLAB_TABLEFORM_PLACER__

#include "MaxBIM.hpp"

namespace slabTableformPlacerDG {
	// 객체 타입
	enum	libPartObjType {
		NONE,				// 없음
		EUROFORM,			// 유로폼
		TABLEFORM,			// 슬래브 테이블폼 (콘판넬)
		PLYWOOD				// 합판
	};

	// 다이얼로그 항목 인덱스
	enum	idxItems_1_forSlabBottomTableformPlacer {
		LABEL_SELECT_TYPE = 3,
		PUSHRADIO_EUROFORM,
		PUSHRADIO_TABLEFORM,
		PUSHRADIO_PLYWOOD,

		LABEL_CELL_SETTINGS,
		LABEL_TABLEFORM_WIDTH,
		POPUP_TABLEFORM_WIDTH,
		LABEL_TABLEFORM_HEIGHT,
		POPUP_TABLEFORM_HEIGHT,
		LABEL_TABLEFORM_ORIENTATION,
		POPUP_TABLEFORM_ORIENTATION,

		SEPARATOR_1,
		LABEL_GAP_LENGTH,
		EDITCONTROL_GAP_LENGTH,
		SEPARATOR_2,

		ICON_LAYER,
		LABEL_LAYER_SETTINGS,
		CHECKBOX_LAYER_COUPLING,
		LABEL_LAYER_EUROFORM,
		LABEL_LAYER_SLABTABLEFORM,
		LABEL_LAYER_PLYWOOD,
		LABEL_LAYER_TIMBER,
		LABEL_LAYER_CPROFILE,
		LABEL_LAYER_PINBOLT,
		LABEL_LAYER_FITTINGS,
		LABEL_LAYER_GT24_GIRDER,
		LABEL_LAYER_PERI_SUPPORT,
		LABEL_LAYER_STEEL_SUPPORT,

		USERCONTROL_LAYER_EUROFORM,
		USERCONTROL_LAYER_SLABTABLEFORM,
		USERCONTROL_LAYER_PLYWOOD,
		USERCONTROL_LAYER_TIMBER,
		USERCONTROL_LAYER_CPROFILE,
		USERCONTROL_LAYER_PINBOLT,
		USERCONTROL_LAYER_FITTINGS,
		USERCONTROL_LAYER_GT24_GIRDER,
		USERCONTROL_LAYER_PERI_SUPPORT,
		USERCONTROL_LAYER_STEEL_SUPPORT,

		BUTTON_AUTOSET
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
		POPUP_WIDTH,
		LABEL_HEIGHT,
		POPUP_HEIGHT,
		LABEL_ORIENTATION,
		POPUP_ORIENTATION,
		LABEL_CAUTION
	};
}

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
	short	objType;		// enum libPartObjType 참조

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
	double	level;				// 고도
	double	ang;				// 회전 각도 (단위: Radian, 회전축: Z축)

	short	iTableformType;		// 테이블폼 타입 (1: 유로폼, 2: 콘판넬 테이블폼, 3: 합판)
	double	gap;				// 슬래브와의 간격
	bool	bVertical;			// true이면 세로 방향, false이면 가로 방향
	bool	bRectangleArea;		// 직사각형 모프이면 true, 꺾인 모서리 모프이면 false

	double	borderHorLen;		// 최외곽 가로 길이
	double	borderVerLen;		// 최외곽 세로 길이

	API_Coord3D		center;		// 최외곽 중심 점
	API_Coord3D		leftBottom;	// 최외곽 좌하단 점
	API_Coord3D		rightTop;	// 최외곽 우상단 점

	double	initHorLen;			// 셀 가로 길이 초기값
	double	initVerLen;			// 셀 세로 길이 초기값

	double	leftBottomX;		// 테이블폼 시작 좌표 X
	double	leftBottomY;		// 테이블폼 시작 좌표 Y
	double	leftBottomZ;		// 테이블폼 시작 좌표 Z
	double	formArrayWidth;		// 테이블폼 배열 전체 너비
	double	formArrayHeight;	// 테이블폼 배열 전체 높이

	double	marginLeft;			// 왼쪽 여백 길이
	double	marginRight;		// 오른쪽 여백 길이
	double	marginTop;			// 위쪽 여백 길이
	double	marginBottom;		// 아래쪽 여백 길이

	short	nHorCells;			// 가로 방향 셀 개수
	short	nVerCells;			// 세로 방향 셀 개수

	CellForSlabTableform	cells [50][50];		// 셀 정보 (0행은 하단, 0열은 좌측)

public:
	void		initCells (SlabTableformPlacingZone* placingZone);											// Cell 배열을 초기화함
	void		alignPlacingZone (SlabTableformPlacingZone* placingZone);									// Cell 배열의 위치를 조정함
	GSErrCode	fillTableformAreas (void);																	// 테이블폼 공간 채우기
	GSErrCode	fillRestAreas (void);																		// 자투리 공간 채우기 (합판, 각재)
	//API_Guid	placeLibPart (CellForSlabTableform objInfo);												// 해당 셀 정보를 기반으로 라이브러리 배치
	//API_Guid	placeLibPartOnSlabTableform (CellForSlabTableform objInfo);									// 슬래브 테이블폼의 부속 철물들에 해당하는 라이브러리 배치

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

#endif