#ifndef	__SLAB_TABLEFORM_PLACER__
#define __SLAB_TABLEFORM_PLACER__

#include "MaxBIM.hpp"

namespace slabTableformPlacerDG {
	// 객체 타입
	enum	libPartObjType {
		NONE,				// 없음
		SLAB_TABLEFORM,		// 슬래브 테이블폼 (콘판넬) v1.0
		PLYWOOD,			// 합판v1.0
		WOOD,				// 목재v1.0
		CPROFILE,			// KS프로파일v1.0 - C형강
		FITTINGS,			// 결합철물 (사각와셔활용) v1.0
	};

	// 다이얼로그 항목 인덱스
	enum	idxItems_1_forSlabBottomTableformPlacer {
		LABEL_PLACING_TABLEFORM		= 3,
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
		LABEL_LAYER_SLABTABLEFORM,
		LABEL_LAYER_PLYWOOD,
		LABEL_LAYER_WOOD,
		LABEL_LAYER_CPROFILE,
		LABEL_LAYER_FITTINGS,

		USERCONTROL_LAYER_SLABTABLEFORM,
		USERCONTROL_LAYER_PLYWOOD,
		USERCONTROL_LAYER_WOOD,
		USERCONTROL_LAYER_CPROFILE,
		USERCONTROL_LAYER_FITTINGS,
	};

	enum	idxItems_2_forSlabBottomTableformPlacer {
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
		LABEL_GRID_TABLEFORM_WOOD,
		PUSHBUTTON_CONFIRM_REMAIN_LENGTH,
		PUSHBUTTON_ADD_ROW,
		PUSHBUTTON_DEL_ROW,
		PUSHBUTTON_ADD_COL,
		PUSHBUTTON_DEL_COL,

		// 이후에는 그리드 버튼이 배치됨
		GRIDBUTTON_IDX_START
	};

	enum	idxItems_3_forSlabBottomTableformPlacer {
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
		LABEL_TABLEFORM_WIDTH_OPTIONS,
		POPUP_TABLEFORM_WIDTH_OPTIONS,
		EDITCONTROL_TABLEFORM_WIDTH_OPTIONS,
		LABEL_TABLEFORM_HEIGHT_OPTIONS,
		POPUP_TABLEFORM_HEIGHT_OPTIONS,
		EDITCONTROL_TABLEFORM_HEIGHT_OPTIONS,
		LABEL_TABLEFORM_ORIENTATION_OPTIONS,
		RADIO_ORIENTATION_1_TABLEFORM,
		RADIO_ORIENTATION_2_TABLEFORM,
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
	short		objType;	// enum libPartObjType 참조

	API_Guid	guid;		// 객체의 GUID

	double	leftBottomX;	// 좌하단 좌표 X
	double	leftBottomY;	// 좌하단 좌표 Y
	double	leftBottomZ;	// 좌하단 좌표 Z

	double	horLen;			// 가로 길이
	double	verLen;			// 세로 길이
	double	ang;			// 회전 각도 (단위: Radian, 회전축: Z축)

	union {
		SlabTableform	tableform;
		Plywood			plywood;
		Wood			wood;
		KSProfile		cprofile;
		MetalFittingsWithRectWasher		fittings;
	} libPart;
};

// 슬래브 하부 영역 정보
class SlabTableformPlacingZone
{
public:
	// 좌우상하: 슬래브 위에서 봤을 때의 방향을 따름

	// 슬래브 기하 정보
	double	level;				// 고도
	double	ang;				// 회전 각도 (단위: Radian, 회전축: Z축)
	double	leftBottomX;		// 테이블폼 시작 좌표 X
	double	leftBottomY;		// 테이블폼 시작 좌표 Y
	double	leftBottomZ;		// 테이블폼 시작 좌표 Z
	double	formArrayWidth;		// 테이블폼 배열 전체 너비
	double	formArrayHeight;	// 테이블폼 배열 전체 높이
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

	// 테이블폼이 설치되는 영역의 너비/높이
	double	innerWidth;		// 가로 길이
	double	innerHeight;	// 세로 길이

	// 검토할 사항 (1. 기본 채우기)
	double	remain_hor;				// 가로 방향 남은 길이
	double	remain_hor_updated;		// 가로 방향 남은 길이 (업데이트 후)
	double	remain_ver;				// 세로 방향 남은 길이
	double	remain_ver_updated;		// 세로 방향 남은 길이 (업데이트 후)

	double	gap;		// 슬래브와의 간격

	std::string		tb_wid;			// 테이블폼 너비
	std::string		tb_hei;			// 테이블폼 높이
	std::string		tb_ori;			// 테이블폼 방향
	double	tb_wid_numeric;			// 테이블폼 너비 (실수형)
	double	tb_hei_numeric;			// 테이블폼 높이 (실수형)
	short	tb_count_hor;			// 가로 방향 테이블폼 개수
	short	tb_count_ver;			// 세로 방향 테이블폼 개수

	// 검토할 사항 (2. 배치된 객체 정보를 그리드로 관리)
	CellForSlabTableform	cells [50][50];		// 유로폼 셀 정보 - 마지막 인덱스: [eu_count_ver-1][eu_count_hor-1]
	bool	topBoundsCells [50];				// 위쪽 목재 보강재 유무 - 마지막 인덱스: [eu_count_hor-2] : LeftBottom부터 RightBottom까지
	bool	bottomBoundsCells [50];				// 아래쪽 목재 보강재 유무 - 마지막 인덱스: [eu_count_hor-2] : LeftTop에서 RightTop까지
	bool	leftBoundsCells [50];				// 왼쪽 목재 보강재 유무 - 마지막 인덱스: [eu_count_ver-2] : LeftBottom에서 LeftTop까지
	bool	rightBoundsCells [50];				// 오른쪽 목재 보강재 유무 - 마지막 인덱스: [eu_count_ver-2] : RightBottom에서 RightTop까지

public:
	void		initCells (SlabTableformPlacingZone* placingZone);											// Cell 배열을 초기화함
	void		firstPlacingSettings (SlabTableformPlacingZone* placingZone);								// 1차 배치: 유로폼
	void		adjustOtherCellsInSameRow (SlabTableformPlacingZone* target_zone, short row, short col);	// 해당 셀과 동일한 행에 있는 다른 셀들의 타입 및 높이를 조정함
	void		adjustOtherCellsInSameCol (SlabTableformPlacingZone* target_zone, short row, short col);	// 해당 셀과 동일한 열에 있는 다른 셀들의 타입 및 너비를 조정함
	void		addNewRow (SlabTableformPlacingZone* target_zone);											// 새로운 행을 추가함 (행 하나를 늘리고 추가된 행에 마지막 행 정보 복사)
	void		addNewCol (SlabTableformPlacingZone* target_zone);											// 새로운 열을 추가함 (열 하나를 늘리고 추가된 열에 마지막 열 정보 복사)
	void		delLastRow (SlabTableformPlacingZone* target_zone);											// 마지막 행을 삭제함
	void		delLastCol (SlabTableformPlacingZone* target_zone);											// 마지막 열을 삭제함
	void		alignPlacingZone (SlabTableformPlacingZone* target_zone);									// Cell 정보가 변경됨에 따라 파편화된 위치를 재조정함
	API_Guid	placeLibPart (CellForSlabTableform objInfo);												// 해당 셀 정보를 기반으로 라이브러리 배치
	API_Guid	placeLibPartOnSlabTableform (CellForSlabTableform objInfo);									// 슬래브 테이블폼의 부속 철물들에 해당하는 라이브러리 배치
	GSErrCode	fillRestAreas (void);																		// 유로폼을 채운 후 자투리 공간 채우기
};

// 테이블폼 슬래브 하부 배치 함수
GSErrCode	placeTableformOnSlabBottom (void);	// 슬래브 하부에 테이블폼을 배치하는 통합 루틴
short DGCALLBACK slabBottomTableformPlacerHandler1 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 1차 배치를 위한 질의를 요청하는 1차 다이얼로그
short DGCALLBACK slabBottomTableformPlacerHandler2 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 1차 배치 후 수정을 요청하는 2차 다이얼로그
short DGCALLBACK slabBottomTableformPlacerHandler3 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 2차 다이얼로그에서 각 셀의 객체 타입을 변경하기 위한 3차 다이얼로그

#endif