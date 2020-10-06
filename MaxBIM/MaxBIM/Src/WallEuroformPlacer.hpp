#ifndef	__WALL_EUROFORM_PLACER__
#define __WALL_EUROFORM_PLACER__
#endif

#include "MaxBIM.hpp"

// 변수 선언
enum	idxItems_1;
enum	idxItems_2;
enum	idxItems_3;
enum	libPartObjType;
struct	InfoWall;
struct	InfoMorph;
struct	InterfereBeam;
struct	Euroform;
struct	FillerSpacer;
struct	IncornerPanel;
struct	Plywood;
struct	Wood;
struct	Cell;
struct	PlacingZone;
class	WallPlacingZone;

// 유로폼 벽 배치 함수
void	initCellsForWall (PlacingZone* placingZone);														// Cell 배열을 초기화함
void	firstPlacingSettingsForWall (PlacingZone* placingZone);												// 1차 배치: 인코너, 유로폼
void	copyPlacingZoneSymmetricForWall (PlacingZone* src_zone, PlacingZone* dst_zone, InfoWall* infoWall);	// 원본 벽면 영역 정보를 대칭하는 반대쪽에도 복사함
void	alignPlacingZoneForWall (PlacingZone* target_zone);													// Cell 정보가 변경됨에 따라 파편화된 위치를 재조정함
GSErrCode	placeEuroformOnWall (void);																		// 1번 메뉴: 유로폼/인코너 등을 배치하는 통합 루틴
GSErrCode	fillRestAreasForWall (void);																	// 가로 채우기까지 완료된 후 자투리 공간 채우기
short DGCALLBACK wallPlacerHandlerPrimary (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// 1차 배치를 위한 질의를 요청하는 1차 다이얼로그
short DGCALLBACK wallPlacerHandlerSecondary (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 1차 배치 후 수정을 요청하는 2차 다이얼로그
short DGCALLBACK wallPlacerHandlerThird (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// 2차 다이얼로그에서 각 셀의 객체 타입을 변경하기 위한 3차 다이얼로그
short DGCALLBACK wallPlacerHandlerFourth (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// 보 하부의 합판/목재 영역을 유로폼으로 채울지 물어보는 4차 다이얼로그

// 공통 함수
void	copyCellsToAnotherLine (PlacingZone* target_zone, short src_row, short dst_row);					// src행의 Cell 전체 라인을 dst행으로 복사
API_Guid	placeLibPart (Cell objInfo);																	// 해당 셀 정보를 기반으로 라이브러리 배치
double		getCellPositionLeftBottomX (PlacingZone *src_zone, short arr1, short idx);						// [arr1]행 - 해당 셀의 좌하단 좌표X 위치를 리턴
void		setCellPositionLeftBottomZ (PlacingZone *src_zone, short arr1, double new_hei);					// [arr1]행 - 전체 셀의 최하단 좌표Z 위치를 설정


// 다이얼로그 항목 인덱스
enum	idxItems_1 {
	LABEL_INCORNER				= 3,
	CHECKBOX_SET_LEFT_INCORNER	= 4,
	EDITCONTROL_LEFT_INCORNER	= 5,
	CHECKBOX_SET_RIGHT_INCORNER	= 6,
	EDITCONTROL_RIGHT_INCORNER	= 7,
	SEPARATOR_1					= 8,

	LABEL_PLACING_EUROFORM		= 9,
	LABEL_EUROFORM_WIDTH		= 10,
	POPUP_EUROFORM_WIDTH		= 11,
	LABEL_EUROFORM_HEIGHT		= 12,
	POPUP_EUROFORM_HEIGHT		= 13,
	LABEL_EUROFORM_ORIENTATION	= 14,
	POPUP_EUROFORM_ORIENTATION	= 15,
	SEPARATOR_2					= 16,

	ICON_LAYER					= 17,
	USERCONTROL_LAYER			= 18
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

// 객체 번호
enum	libPartObjType {
	NONE,			// 없음
	INCORNER,		// 인코너판넬v1.0
	EUROFORM,		// 유로폼v2.0
	FILLERSPACER,	// 휠러스페이서v1.0
	PLYWOOD,		// 합판v1.0
	WOOD			// 목재v1.0
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
struct InfoMorph
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
struct InterfereBeam
{
	double	leftBottomX;
	double	leftBottomY;
	double	leftBottomZ;

	double	horLen;
	double	verLen;
};

// 유로폼 정보
struct Euroform
{
	bool			eu_stan_onoff;	// 규격폼 On/Off
	double			eu_wid;			// 너비 (규격) : *600, 500, 450, 400, 300, 200
	double			eu_hei;			// 높이 (규격) : *1200, 900, 600
	double			eu_wid2;		// 너비 (비규격) : 50 ~ 900
	double			eu_hei2;		// 높이 (비규격) : 50 ~ 1500
	bool			u_ins_wall;		// 설치방향 : 벽세우기(true), 벽눕히기(false)
	/*
	std::string		u_ins;			// 설치방향 : *벽세우기, 벽눕히기
	double			ang_x;			// 회전X : 벽(90), 천장(0), 바닥(180)
	double			ang_y;			// 회전Y
	*/
};

// 휠러스페이서 정보
struct FillerSpacer
{
	double			f_thk;			// 두께 : 10 ~ 50 (*20)
	double			f_leng;			// 길이 : 150 ~ 2400
	/*
	double			f_ang;			// 각도 : 90
	double			f_rota;			// 회전 : 0
	*/
};

// 인코너판넬 정보
struct IncornerPanel
{
	double			wid_s;			// 가로(빨강) : 80 ~ 500 (*100)
	double			leng_s;			// 세로(파랑) : 80 ~ 500 (*100)
	double			hei_s;			// 높이 : 50 ~ 1500
	/*
	double			dir_s;			// 설치방향 : *세우기, 눕히기, 뒤집기
	*/
};

// 합판 정보
struct Plywood
{
	/*
	std::string		p_stan;			// 규격 : *3x6 [910x1820], 4x8 [1220x2440], 비규격, 비정형
	std::string		w_dir;			// 설치방향 : *벽세우기, 벽눕히기, 바닥깔기, 바닥덮기
	std::string		p_thk;			// 두께 : 2.7T, 4.8T, 8.5T, *11.5T, 14.5T
	*/
	double			p_wid;			// 가로
	double			p_leng;			// 세로
	bool			w_dir_wall;		// 설치방향 : 벽세우기(true), 벽눕히기(false)
	/*
	double			p_ang;			// 각도 : 0
	bool			sogak;			// 제작틀 *On/Off
	std::string		prof;			// 목재종류 : *소각, 중각, 대각
	*/
};

// 목재 정보
struct Wood
{
	/*
	std::string		w_ins;			// 설치방향 : *벽세우기, 바닥눕히기, 바닥덮기
	*/
	double			w_w;			// 두께
	double			w_h;			// 너비
	double			w_leng;			// 길이
	double			w_ang;			// 각도
};

// 그리드 각 셀 정보
struct Cell
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
struct PlacingZone
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
	Cell	cells [50][100];		// 마지막 인덱스: [eu_count_ver-1][nCells-1]
	short	nCells;
	
	// 간섭보 (0개 이상)
	short	nInterfereBeams;		// 간섭보 개수
	InterfereBeam	beams [30];		// 간섭보 정보
	Cell			woods [30][3];	// 보 주변 합판/목재 셀

	// 상단 합판/목재 셀 정보
	Cell	topRestCells [100];		// 상단 자투리 공간 셀
};
