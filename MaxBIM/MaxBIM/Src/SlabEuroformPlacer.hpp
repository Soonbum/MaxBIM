#ifndef	__SLAB_EUROFORM_PLACER__
#define __SLAB_EUROFORM_PLACER__

#include "MaxBIM.hpp"

namespace slabBottomPlacerDG {
	// 객체 타입
	enum	libPartObjType {
		NONE,			// 없음
		EUROFORM,		// 유로폼v2.0
		PLYWOOD,		// 합판v1.0
		WOOD			// 목재v1.0
	};

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

		LABEL_GAP_LENGTH,
		EDITCONTROL_GAP_LENGTH,
		SEPARATOR_2,

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
		DG_CANCEL2 = 3,
		DG_PREV,
		LABEL_REMAIN_HORIZONTAL_LENGTH_LEFT,
		LABEL_REMAIN_HORIZONTAL_LENGTH_RIGHT,
		LABEL_REMAIN_VERTICAL_LENGTH_UP,
		LABEL_REMAIN_VERTICAL_LENGTH_DOWN,
		EDITCONTROL_REMAIN_HORIZONTAL_LENGTH_LEFT,
		EDITCONTROL_REMAIN_HORIZONTAL_LENGTH_RIGHT,
		EDITCONTROL_REMAIN_VERTICAL_LENGTH_UP,
		EDITCONTROL_REMAIN_VERTICAL_LENGTH_DOWN,
		LABEL_GRID_EUROFORM_WOOD,
		PUSHBUTTON_CONFIRM_REMAIN_LENGTH,
		PUSHBUTTON_ADD_ROW,
		PUSHBUTTON_DEL_ROW,
		PUSHBUTTON_ADD_COL,
		PUSHBUTTON_DEL_COL,

		// 이후에는 그리드 버튼이 배치됨
		GRIDBUTTON_IDX_START
	};

	enum	idxItems_3_forSlabBottomPlacer {
		LABEL_OBJ_TYPE	= 3,
		POPUP_OBJ_TYPE,
		LABEL_WIDTH,
		EDITCONTROL_WIDTH,
		LABEL_HEIGHT,
		EDITCONTROL_HEIGHT,
		LABEL_ORIENTATION,
		RADIO_ORIENTATION_1_PLYWOOD,
		RADIO_ORIENTATION_2_PLYWOOD,
		CHECKBOX_SET_STANDARD,
		LABEL_EUROFORM_WIDTH_OPTIONS,
		POPUP_EUROFORM_WIDTH_OPTIONS,
		EDITCONTROL_EUROFORM_WIDTH_OPTIONS,
		LABEL_EUROFORM_HEIGHT_OPTIONS,
		POPUP_EUROFORM_HEIGHT_OPTIONS,
		EDITCONTROL_EUROFORM_HEIGHT_OPTIONS,
		LABEL_EUROFORM_ORIENTATION_OPTIONS,
		RADIO_ORIENTATION_1_EUROFORM,
		RADIO_ORIENTATION_2_EUROFORM
	};

	enum	idxItems_4_forSlabBottomPlacer {
		LABEL_WOOD_WIDTH = 3,
		EDITCONTROL_WOOD_WIDTH
	};
}

// 모프 관련 정보
struct InfoMorphForSlab
{
	API_Guid	guid;		// 모프의 GUID
	short		floorInd;	// 층 인덱스
	double		level;		// 모프의 고도
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
class SlabPlacingZone
{
public:
	// 좌우상하: 슬래브 위에서 봤을 때의 방향을 따름

	// 슬래브 기하 정보
	double	level;				// 고도
	double	ang;				// 회전 각도 (단위: Radian, 회전축: Z축)
	double	leftBottomX;		// 유로폼 시작 좌표 X
	double	leftBottomY;		// 유로폼 시작 좌표 Y
	double	leftBottomZ;		// 유로폼 시작 좌표 Z
	double	formArrayWidth;		// 유로폼 배열 전체 너비
	double	formArrayHeight;	// 유로폼 배열 전체 높이
	double	upMove;				// 위로 이동
	double	leftMove;			// 아래로 이동

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

	double	gap;		// 슬래브와의 간격

	double	woodWidth;	// 목재 너비

	std::string		eu_wid;			// 유로폼 너비
	std::string		eu_hei;			// 유로폼 높이
	std::string		eu_ori;			// 유로폼 방향
	double	eu_wid_numeric;			// 유로폼 너비 (실수형)
	double	eu_hei_numeric;			// 유로폼 높이 (실수형)
	short	eu_count_hor;			// 가로 방향 유로폼 개수
	short	eu_count_ver;			// 세로 방향 유로폼 개수

	// 검토할 사항 (2. 배치된 객체 정보를 그리드로 관리)
	CellForSlab	cells [50][50];		// 유로폼 셀 정보 - 마지막 인덱스: [eu_count_ver-1][eu_count_hor-1]
	bool	topBoundsCells [50];	// 위쪽 목재 보강재 유무 - 마지막 인덱스: [eu_count_hor-2] : LeftBottom부터 RightBottom까지
	bool	bottomBoundsCells [50];	// 아래쪽 목재 보강재 유무 - 마지막 인덱스: [eu_count_hor-2] : LeftTop에서 RightTop까지
	bool	leftBoundsCells [50];	// 왼쪽 목재 보강재 유무 - 마지막 인덱스: [eu_count_ver-2] : LeftBottom에서 LeftTop까지
	bool	rightBoundsCells [50];	// 오른쪽 목재 보강재 유무 - 마지막 인덱스: [eu_count_ver-2] : RightBottom에서 RightTop까지

public:
	void		initCells (SlabPlacingZone* placingZone);											// Cell 배열을 초기화함
	void		firstPlacingSettings (SlabPlacingZone* placingZone);								// 1차 배치: 유로폼
	void		adjustOtherCellsInSameRow (SlabPlacingZone* target_zone, short row, short col);		// 해당 셀과 동일한 행에 있는 다른 셀들의 타입 및 높이를 조정함
	void		adjustOtherCellsInSameCol (SlabPlacingZone* target_zone, short row, short col);		// 해당 셀과 동일한 열에 있는 다른 셀들의 타입 및 너비를 조정함
	void		addNewRow (SlabPlacingZone* target_zone);											// 새로운 행을 추가함 (행 하나를 늘리고 추가된 행에 마지막 행 정보 복사)
	void		addNewCol (SlabPlacingZone* target_zone);											// 새로운 열을 추가함 (열 하나를 늘리고 추가된 열에 마지막 열 정보 복사)
	void		delLastRow (SlabPlacingZone* target_zone);											// 마지막 행을 삭제함
	void		delLastCol (SlabPlacingZone* target_zone);											// 마지막 열을 삭제함
	void		alignPlacingZone (SlabPlacingZone* target_zone);									// Cell 정보가 변경됨에 따라 파편화된 위치를 재조정함
	API_Guid	placeLibPart (CellForSlab objInfo);													// 해당 셀 정보를 기반으로 라이브러리 배치
	GSErrCode	fillRestAreas (void);																// 유로폼을 채운 후 자투리 공간 채우기
};

// 유로폼 슬래브 하부 배치 함수
GSErrCode	placeEuroformOnSlabBottom (void);	// 슬래브 하부에 유로폼을 배치하는 통합 루틴
short DGCALLBACK slabBottomPlacerHandler1 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 1차 배치를 위한 질의를 요청하는 1차 다이얼로그
short DGCALLBACK slabBottomPlacerHandler2 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 1차 배치 후 수정을 요청하는 2차 다이얼로그
short DGCALLBACK slabBottomPlacerHandler3 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 2차 다이얼로그에서 각 셀의 객체 타입을 변경하기 위한 3차 다이얼로그
short DGCALLBACK slabBottomPlacerHandler4 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 자투리 채우기를 할 때, 목재의 너비를 변경하기 위한 4차 다이얼로그

#endif
