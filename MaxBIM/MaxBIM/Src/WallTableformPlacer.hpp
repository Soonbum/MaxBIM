#ifndef	__WALL_TABLEFORM_PLACER__
#define __WALL_TABLEFORM_PLACER__

#include "MaxBIM.hpp"

namespace wallTableformPlacerDG {
	// 다이얼로그 항목 인덱스
	enum	idxItems_2_forWallTableformPlacer {
		LABEL_HEIGHT	= 3,
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
		LABEL_LAYER_EUROFORM,
		LABEL_LAYER_RECTPIPE,
		LABEL_LAYER_PINBOLT,
		LABEL_LAYER_WALLTIE,
		LABEL_LAYER_JOIN,
		LABEL_LAYER_HEADPIECE,
		LABEL_LAYER_PLYWOOD,
		LABEL_LAYER_WOOD,

		USERCONTROL_LAYER_EUROFORM,
		USERCONTROL_LAYER_RECTPIPE,
		USERCONTROL_LAYER_PINBOLT,
		USERCONTROL_LAYER_WALLTIE,
		USERCONTROL_LAYER_JOIN,
		USERCONTROL_LAYER_HEADPIECE,
		USERCONTROL_LAYER_PLYWOOD,
		USERCONTROL_LAYER_WOOD
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
}

// 벽 관련 정보
struct InfoWallForWallTableform
{
	double	wallThk;			// 벽 두께
	short	floorInd;			// 층 인덱스
	double	bottomOffset;		// 벽 하단 오프셋

	double	begX;				// 시작점 X
	double	begY;				// 시작점 Y
	double	endX;				// 끝점 X
	double	endY;				// 끝점 Y
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

	double	horLen;			// 가로 길이 (1800~2300, 50 간격)
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

// 벽면 영역 정보
struct WallTableformPlacingZone
{
	double	leftBottomX;	// 좌하단 좌표 X
	double	leftBottomY;	// 좌하단 좌표 Y
	double	leftBottomZ;	// 좌하단 좌표 Z

	double	horLen;			// 가로 길이
	double	verLen;			// 세로 길이
	double	ang;			// 회전 각도 (단위: Radian, 회전축: Z축)

	bool	bDoubleSide;	// 양면이면 true, 단면이면 false

	double	gap;			// 벽과의 간격

	double	remainWidth;	// 남은 길이

	CellForWallTableform		cells [50];			// 테이블폼 셀 정보
	UpperCellForWallTableform	upperCells [50];	// 테이블폼 상단의 유로폼 또는 합판 셀 정보
	short		nCells;								// 테이블폼 셀 개수
	double		marginTop;							// 상단 여백 높이

	// 테이블폼 개수 (각각은 너비 1800~2300의 테이블폼을 의미함)
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
};

// 파라미터: 유로폼
struct paramsUFOM_ForWallTableform
{
	double	leftBottomX;	// 좌하단 좌표 X
	double	leftBottomY;	// 좌하단 좌표 Y
	double	leftBottomZ;	// 좌하단 좌표 Z

	double	ang;			// 회전 각도 (단위: Radian, 회전축: Z축)

	double	width;			// 너비
	double	height;			// 높이
};

// 파라미터: 비계 파이프
struct paramsSPIP_ForWallTableform
{
	double	leftBottomX;	// 좌하단 좌표 X
	double	leftBottomY;	// 좌하단 좌표 Y
	double	leftBottomZ;	// 좌하단 좌표 Z

	double	ang;			// 회전 각도 (단위: Radian, 회전축: Z축)

	double	length;			// 파이프 길이
	double	pipeAng;		// 각도 (수평: 0, 수직: 90)
};

// 파라미터: 핀볼트 세트
struct paramsPINB_ForWallTableform
{
	double	leftBottomX;	// 좌하단 좌표 X
	double	leftBottomY;	// 좌하단 좌표 Y
	double	leftBottomZ;	// 좌하단 좌표 Z

	double	ang;			// 회전 각도 (단위: Radian, 회전축: Z축)

	bool	bPinBoltRot90;	// 핀볼트 90도 회전
	double	boltLen;		// 볼트 길이
	double	angX;			// X축 회전
	double	angY;			// Y축 회전
};

// 파라미터: 벽체 타이
struct paramsTIE_ForWallTableform
{
	double	leftBottomX;	// 좌하단 좌표 X
	double	leftBottomY;	// 좌하단 좌표 Y
	double	leftBottomZ;	// 좌하단 좌표 Z

	double	ang;			// 회전 각도 (단위: Radian, 회전축: Z축)

	double	boltLen;		// 볼트 길이
	double	pipeBeg;		// 파이프 시작점
	double	pipeEnd;		// 파이프 끝점
	double	clampBeg;		// 조임쇠 시작점
	double	clampEnd;		// 조임쇠 끝점
};

// 파라미터: 직교 클램프
struct paramsCLAM_ForWallTableform
{
	double	leftBottomX;	// 좌하단 좌표 X
	double	leftBottomY;	// 좌하단 좌표 Y
	double	leftBottomZ;	// 좌하단 좌표 Z

	double	ang;			// 회전 각도 (단위: Radian, 회전축: Z축)

	double	angX;			// 본체 회전 (X)
	double	angY;			// 본체 회전 (Y)
};

// 파라미터: 헤드피스
struct paramsPUSH_ForWallTableform
{
	double	leftBottomX;	// 좌하단 좌표 X
	double	leftBottomY;	// 좌하단 좌표 Y
	double	leftBottomZ;	// 좌하단 좌표 Z

	double	ang;			// 회전 각도 (단위: Radian, 회전축: Z축)
};

// 파라미터: 결합철물
struct paramsJOIN_ForWallTableform
{
	double	leftBottomX;	// 좌하단 좌표 X
	double	leftBottomY;	// 좌하단 좌표 Y
	double	leftBottomZ;	// 좌하단 좌표 Z

	double	ang;			// 회전 각도 (단위: Radian, 회전축: Z축)

	double	angX;			// 본체 회전 (X)
	double	angY;			// 본체 회전 (Y)
};

// 파라미터: 합판
struct paramsPLYW_ForWallTableform
{
	double	leftBottomX;	// 좌하단 좌표 X
	double	leftBottomY;	// 좌하단 좌표 Y
	double	leftBottomZ;	// 좌하단 좌표 Z

	double	ang;			// 회전 각도 (단위: Radian, 회전축: Z축)

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

// 파라미터: 목재
struct paramsTIMB_ForWallTableform
{
	double	leftBottomX;	// 좌하단 좌표 X
	double	leftBottomY;	// 좌하단 좌표 Y
	double	leftBottomZ;	// 좌하단 좌표 Z

	double	ang;			// 회전 각도 (단위: Radian, 회전축: Z축)

	/*
	std::string		w_ins;	// 설치방향 : *벽세우기, 바닥눕히기, 바닥덮기
	*/

	double	w_w;			// 두께
	double	w_h;			// 너비
	double	w_leng;			// 길이
	double	w_ang;			// 각도
};

// 배치 정보
struct	placementInfoForWallTableform
{
	short	nHorEuroform;	// 수평 방향 유로폼 개수
	short	nVerEuroform;	// 수직 방향 유로폼 개수

	double	width [7];		// 수평 방향 각 유로폼 너비
	double	height [7];		// 수직 방향 각 유로폼 높이
};

GSErrCode	placeTableformOnWall (void);											// 벽에 테이블폼을 배치하는 통합 루틴
void		initCellsForWallTableform (WallTableformPlacingZone* placingZone);		// Cell 배열을 초기화함
GSErrCode	placeTableformOnWall (CellForWallTableform cell);						// 테이블폼 배치하기
GSErrCode	placeTableformOnWall (CellForWallTableform cell, UpperCellForWallTableform upperCell);		// 테이블폼 상단 배치하기
double		getCellPositionLeftBottomXForWallTableForm (WallTableformPlacingZone *placingZone, short idx);		// 해당 셀의 좌하단 좌표X 위치를 리턴
short DGCALLBACK wallTableformPlacerHandler1 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 선호하는 테이블폼 너비를 선택하기 위한 다이얼로그
short DGCALLBACK wallTableformPlacerHandler2 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 테이블폼 배치를 위한 질의를 요청하는 다이얼로그
short DGCALLBACK wallTableformPlacerHandler3 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 벽 상단의 합판/목재 영역을 유로폼으로 채울지 물어보는 3차 다이얼로그

double		moveXinParallel (double prevPosX, double ang, double offset);	// 이동 후의 X 좌표를 알려줌 (Z 회전각도 고려) - 벽과 평행한 방향으로 이동
double		moveYinParallel (double prevPosY, double ang, double offset);	// 이동 후의 Y 좌표를 알려줌 (Z 회전각도 고려) - 벽과 평행한 방향으로 이동
double		moveXinPerpend (double prevPosX, double ang, double offset);	// 이동 후의 X 좌표를 알려줌 (Z 회전각도 고려) - 벽과 수직한 방향으로 이동
double		moveYinPerpend (double prevPosY, double ang, double offset);	// 이동 후의 Y 좌표를 알려줌 (Z 회전각도 고려) - 벽과 수직한 방향으로 이동
double		moveZ (double prevPosZ, double offset);							// 이동 후의 Z 좌표를 알려줌 (Z 회전각도 고려)

API_Guid	placeUFOM (paramsUFOM_ForWallTableform	params);	// 배치: 유로폼
API_Guid	placeUFOM_up (paramsUFOM_ForWallTableform	params);	// 배치: 유로폼 (상부)
API_Guid	placeSPIP (paramsSPIP_ForWallTableform	params);	// 배치: 비계 파이프
API_Guid	placePINB (paramsPINB_ForWallTableform	params);	// 배치: 핀볼트 세트
API_Guid	placeTIE  (paramsTIE_ForWallTableform	params);	// 배치: 벽체 타이
API_Guid	placeCLAM (paramsCLAM_ForWallTableform	params);	// 배치: 직교 클램프
API_Guid	placePUSH (paramsPUSH_ForWallTableform	params);	// 배치: 헤드피스
API_Guid	placeJOIN (paramsJOIN_ForWallTableform	params);	// 배치: 결합철물
API_Guid	placePLYW (paramsPLYW_ForWallTableform	params);	// 배치: 합판
API_Guid	placeTIMB (paramsTIMB_ForWallTableform	params);	// 배치: 목재

#endif