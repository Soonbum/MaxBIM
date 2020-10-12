#ifndef	__WALL_EUROFORM_PLACER__
#define __WALL_EUROFORM_PLACER__

#include "MaxBIM.hpp"

// 변수 선언
enum	idxItems_1;
enum	idxItems_2;
enum	idxItems_3;
struct	InfoWall;
struct	InfoMorphForWall;
struct	InterfereBeamForWall;
struct	CellForWall;
struct	WallPlacingZone;

// 유로폼 벽 배치 함수
void	initCellsForWall (WallPlacingZone* placingZone);															// Cell 배열을 초기화함
void	firstPlacingSettingsForWall (WallPlacingZone* placingZone);													// 1차 배치: 인코너, 유로폼
void	copyPlacingZoneSymmetricForWall (WallPlacingZone* src_zone, WallPlacingZone* dst_zone, InfoWall* infoWall);	// 원본 벽면 영역 정보를 대칭하는 반대쪽에도 복사함
void	alignPlacingZoneForWall (WallPlacingZone* target_zone);														// Cell 정보가 변경됨에 따라 파편화된 위치를 재조정함
void	copyCellsToAnotherLineForWall (WallPlacingZone* target_zone, short src_row, short dst_row);					// src행의 Cell 전체 라인을 dst행으로 복사
void	setCellPositionLeftBottomZForWall (WallPlacingZone *src_zone, short arr1, double new_hei);					// [arr1]행 - 전체 셀의 최하단 좌표Z 위치를 설정
double	getCellPositionLeftBottomXForWall (WallPlacingZone *src_zone, short arr1, short idx);						// [arr1]행 - 해당 셀의 좌하단 좌표X 위치를 리턴
API_Guid	placeLibPartForWall (CellForWall objInfo);																// 해당 셀 정보를 기반으로 라이브러리 배치
GSErrCode	placeEuroformOnWall (void);																				// 1번 메뉴: 벽에 유로폼을 배치하는 통합 루틴
GSErrCode	fillRestAreasForWall (void);																			// 가로 채우기까지 완료된 후 자투리 공간 채우기
short DGCALLBACK wallPlacerHandler1 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 1차 배치를 위한 질의를 요청하는 1차 다이얼로그
short DGCALLBACK wallPlacerHandler2 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 1차 배치 후 수정을 요청하는 2차 다이얼로그
short DGCALLBACK wallPlacerHandler3 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 2차 다이얼로그에서 각 셀의 객체 타입을 변경하기 위한 3차 다이얼로그
short DGCALLBACK wallPlacerHandler4 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 보 하부의 합판/목재 영역을 유로폼으로 채울지 물어보는 4차 다이얼로그
short DGCALLBACK wallPlacerHandler5 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 벽 상단의 합판/목재 영역을 유로폼으로 채울지 물어보는 5차 다이얼로그


// 다이얼로그 항목 인덱스
enum	idxItems_1 {
	LABEL_INCORNER				= 3,
	CHECKBOX_SET_LEFT_INCORNER,
	EDITCONTROL_LEFT_INCORNER,
	CHECKBOX_SET_RIGHT_INCORNER,
	EDITCONTROL_RIGHT_INCORNER,
	SEPARATOR_1,

	LABEL_PLACING_EUROFORM,
	LABEL_EUROFORM_WIDTH,
	POPUP_EUROFORM_WIDTH,
	LABEL_EUROFORM_HEIGHT,
	POPUP_EUROFORM_HEIGHT,
	LABEL_EUROFORM_ORIENTATION,
	POPUP_EUROFORM_ORIENTATION,
	SEPARATOR_2,

	ICON_LAYER,
	LABEL_LAYER_SETTINGS,
	LABEL_LAYER_INCORNER,
	LABEL_LAYER_EUROFORM,
	LABEL_LAYER_FILLERSPACER,
	LABEL_LAYER_PLYWOOD,
	LABEL_LAYER_WOOD,

	USERCONTROL_LAYER_INCORNER,
	USERCONTROL_LAYER_EUROFORM,
	USERCONTROL_LAYER_FILLERSPACER,
	USERCONTROL_LAYER_PLYWOOD,
	USERCONTROL_LAYER_WOOD
};

enum	idxItems_2 {
	LABEL_REMAIN_HORIZONTAL_LENGTH			= 3,
	EDITCONTROL_REMAIN_HORIZONTAL_LENGTH	= 4,
	GROUPBOX_GRID_EUROFORM_FILLERSPACER		= 5,
	PUSHBUTTON_CONFIRM_REMAIN_LENGTH		= 6,

	// 이후에는 그리드 버튼이 배치됨
	GRIDBUTTON_IDX_START					= 7
};

enum	idxItem_3 {
	//////////////////////////////////////// 선택한 셀 설정
	// 객체 타입
	LABEL_OBJ_TYPE							= 3,
	POPUP_OBJ_TYPE,

	// 인코너, 휠러스페이서, 합판의 경우 (두께는 목재 전용)
	LABEL_WIDTH,
	EDITCONTROL_WIDTH,
	LABEL_HEIGHT,
	EDITCONTROL_HEIGHT,
	LABEL_THK,
	EDITCONTROL_THK,
	LABEL_ORIENTATION,
	RADIO_ORIENTATION_1_PLYWOOD,
	RADIO_ORIENTATION_2_PLYWOOD,

	// 유로폼의 경우
	CHECKBOX_SET_STANDARD,
	LABEL_EUROFORM_WIDTH_OPTIONS,
	POPUP_EUROFORM_WIDTH_OPTIONS,
	EDITCONTROL_EUROFORM_WIDTH_OPTIONS,
	LABEL_EUROFORM_HEIGHT_OPTIONS,
	POPUP_EUROFORM_HEIGHT_OPTIONS,
	EDITCONTROL_EUROFORM_HEIGHT_OPTIONS,
	LABEL_EUROFORM_ORIENTATION_OPTIONS,
	RADIO_ORIENTATION_1_EUROFORM,
	RADIO_ORIENTATION_2_EUROFORM,

	//////////////////////////////////////// 예전 셀 설정
	// 객체 타입
	LABEL_OBJ_TYPE_PREV,
	POPUP_OBJ_TYPE_PREV,

	// 인코너, 휠러스페이서, 합판의 경우 (두께는 목재 전용)
	LABEL_WIDTH_PREV,
	EDITCONTROL_WIDTH_PREV,
	LABEL_HEIGHT_PREV,
	EDITCONTROL_HEIGHT_PREV,
	LABEL_THK_PREV,
	EDITCONTROL_THK_PREV,
	LABEL_ORIENTATION_PREV,
	RADIO_ORIENTATION_1_PLYWOOD_PREV,
	RADIO_ORIENTATION_2_PLYWOOD_PREV,

	// 유로폼의 경우
	CHECKBOX_SET_STANDARD_PREV,
	LABEL_EUROFORM_WIDTH_OPTIONS_PREV,
	POPUP_EUROFORM_WIDTH_OPTIONS_PREV,
	EDITCONTROL_EUROFORM_WIDTH_OPTIONS_PREV,
	LABEL_EUROFORM_HEIGHT_OPTIONS_PREV,
	POPUP_EUROFORM_HEIGHT_OPTIONS_PREV,
	EDITCONTROL_EUROFORM_HEIGHT_OPTIONS_PREV,
	LABEL_EUROFORM_ORIENTATION_OPTIONS_PREV,
	RADIO_ORIENTATION_1_EUROFORM_PREV,
	RADIO_ORIENTATION_2_EUROFORM_PREV,

	//////////////////////////////////////// 다음 셀 설정
	// 객체 타입
	LABEL_OBJ_TYPE_NEXT,
	POPUP_OBJ_TYPE_NEXT,

	// 인코너, 휠러스페이서, 합판의 경우 (두께는 목재 전용)
	LABEL_WIDTH_NEXT,
	EDITCONTROL_WIDTH_NEXT,
	LABEL_HEIGHT_NEXT,
	EDITCONTROL_HEIGHT_NEXT,
	LABEL_THK_NEXT,
	EDITCONTROL_THK_NEXT,
	LABEL_ORIENTATION_NEXT,
	RADIO_ORIENTATION_1_PLYWOOD_NEXT,
	RADIO_ORIENTATION_2_PLYWOOD_NEXT,

	// 유로폼의 경우
	CHECKBOX_SET_STANDARD_NEXT,
	LABEL_EUROFORM_WIDTH_OPTIONS_NEXT,
	POPUP_EUROFORM_WIDTH_OPTIONS_NEXT,
	EDITCONTROL_EUROFORM_WIDTH_OPTIONS_NEXT,
	LABEL_EUROFORM_HEIGHT_OPTIONS_NEXT,
	POPUP_EUROFORM_HEIGHT_OPTIONS_NEXT,
	EDITCONTROL_EUROFORM_HEIGHT_OPTIONS_NEXT,
	LABEL_EUROFORM_ORIENTATION_OPTIONS_NEXT,
	RADIO_ORIENTATION_1_EUROFORM_NEXT,
	RADIO_ORIENTATION_2_EUROFORM_NEXT
};

enum	idxItem_4 {
	LABEL_DESC1_BEAMAROUND	= 3,
	LABEL_WIDTH_BEAMAROUND,
	EDITCONTROL_WIDTH_BEAMAROUND,
	LABEL_HEIGHT_BEAMAROUND,
	EDITCONTROL_HEIGHT_BEAMAROUND,
	LABEL_DESC2_BEAMAROUND,
	CHECKBOX_SET_STANDARD_BEAMAROUND,
	LABEL_EUROFORM_WIDTH_OPTIONS_BEAMAROUND,
	POPUP_EUROFORM_WIDTH_OPTIONS_BEAMAROUND,
	EDITCONTROL_EUROFORM_WIDTH_OPTIONS_BEAMAROUND,
	LABEL_EUROFORM_HEIGHT_OPTIONS_BEAMAROUND,
	POPUP_EUROFORM_HEIGHT_OPTIONS_BEAMAROUND,
	EDITCONTROL_EUROFORM_HEIGHT_OPTIONS_BEAMAROUND,
	LABEL_EUROFORM_ORIENTATION_OPTIONS_BEAMAROUND,
	RADIO_ORIENTATION_1_EUROFORM_BEAMAROUND,
	RADIO_ORIENTATION_2_EUROFORM_BEAMAROUND
};

enum	idxItem_5 {
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


// 벽 관련 정보
struct InfoWall
{
	double	wallThk;			// 벽 두께
	short	floorInd;			// 층 인덱스

	double	begX;				// 시작점 X
	double	begY;				// 시작점 Y
	double	endX;				// 끝점 X
	double	endY;				// 끝점 Y
};

// 모프 관련 정보
struct InfoMorphForWall
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

// 간섭 보 정보
struct InterfereBeamForWall
{
	double	leftBottomX;
	double	leftBottomY;
	double	leftBottomZ;

	double	horLen;
	double	verLen;
};

// 그리드 각 셀 정보
struct CellForWall
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
		FillerSpacer	fillersp;
		IncornerPanel	incorner;
		Plywood			plywood;
		Wood			wood;
	} libPart;
};

// 벽면 영역 정보 (가장 중요한 정보)
struct WallPlacingZone
{
	double	leftBottomX;	// 좌하단 좌표 X
	double	leftBottomY;	// 좌하단 좌표 Y
	double	leftBottomZ;	// 좌하단 좌표 Z

	double	horLen;			// 가로 길이
	double	verLen;			// 세로 길이
	double	ang;			// 회전 각도 (단위: Radian, 회전축: Z축)

	// 검토할 사항 (1. 기본 채우기)
	double	remain_hor;				// 가로 방향 남은 길이
	double	remain_hor_updated;		// 가로 방향 남은 길이 (업데이트 후)
	double	remain_ver;				// 세로 방향 남은 길이
	double	remain_ver_wo_beams;	// 간섭보의 영향을 받지 않는 세로 길이

	bool	bLIncorner;				// 인코너 왼쪽 배치
	double	lenLIncorner;			// 인코너 왼쪽 너비
	bool	bRIncorner;				// 인코너 오른쪽 배치
	double	lenRIncorner;			// 인코너 오른쪽 너비

	std::string		eu_wid;			// 유로폼 너비
	std::string		eu_hei;			// 유로폼 높이
	std::string		eu_ori;			// 유로폼 방향
	double	eu_wid_numeric;			// 유로폼 너비 (실수형)
	double	eu_hei_numeric;			// 유로폼 높이 (실수형)
	short	eu_count_hor;			// 가로 방향 유로폼 개수
	short	eu_count_ver;			// 세로 방향 유로폼 개수

	// 검토할 사항 (2. 배치된 객체 정보를 그리드로 관리)
	// 인코너[0] | 예비[홀수] | 폼[짝수] | ... | 인코너[n-1]
	CellForWall	cells [50][100];	// 마지막 인덱스: [eu_count_ver-1][nCells-1]
	short		nCells;
	
	// 간섭보 (0개 이상)
	short			nInterfereBeams;		// 간섭보 개수
	InterfereBeamForWall	beams [30];		// 간섭보 정보
	CellForWall				woods [30][3];	// 보 주변 합판/목재 셀

	// 상단 합판/목재 셀 정보
	CellForWall		topRestCells [100];		// 상단 자투리 공간 합판/목재 셀
};

#endif
