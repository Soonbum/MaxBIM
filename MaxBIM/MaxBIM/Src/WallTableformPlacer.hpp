#ifndef	__WALL_TABLEFORM_PLACER__
#define __WALL_TABLEFORM_PLACER__

#include "MaxBIM.hpp"

namespace wallTableformPlacerDG {
	// 다이얼로그 항목 인덱스
	enum	idxItems_1_forWallTableformPlacer {
		LABEL_HEIGHT	= 3,
		EDITCONTROL_HEIGHT,
		LABEL_ERR_MESSAGE,
		LABEL_WIDTH,
		EDITCONTROL_WIDTH,
		LABEL_GAP_LENGTH,
		EDITCONTROL_GAP_LENGTH,

		ICON_LAYER,
		LABEL_LAYER_SETTINGS,
		LABEL_LAYER_EUROFORM,
		LABEL_LAYER_RECTPIPE,
		LABEL_LAYER_PINBOLT,
		LABEL_LAYER_WALLTIE,
		LABEL_LAYER_CLAMP,
		LABEL_LAYER_HEADPIECE,

		USERCONTROL_LAYER_EUROFORM,
		USERCONTROL_LAYER_RECTPIPE,
		USERCONTROL_LAYER_PINBOLT,
		USERCONTROL_LAYER_WALLTIE,
		USERCONTROL_LAYER_CLAMP,
		USERCONTROL_LAYER_HEADPIECE
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

// 벽면 영역 정보
struct WallTableformPlacingZone
{
	double	leftBottomX;	// 좌하단 좌표 X
	double	leftBottomY;	// 좌하단 좌표 Y
	double	leftBottomZ;	// 좌하단 좌표 Z

	double	horLen;			// 가로 길이
	double	verLen;			// 세로 길이
	double	ang;			// 회전 각도 (단위: Radian, 회전축: Z축)

	double	gap;			// 벽과의 간격

	// 검토할 사항 (2. 배치된 객체 정보를 그리드로 관리)
	CellForWallTableform	cells [50];
	short		nCells;

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

GSErrCode	placeTableformOnWall (void);											// 벽에 테이블폼을 배치하는 통합 루틴
void		initCellsForWallTableform (WallTableformPlacingZone* placingZone);		// Cell 배열을 초기화함
GSErrCode	placeTableformOnWall (CellForWallTableform cell);						// 테이블폼 배치하기
double		getCellPositionLeftBottomXForWallTableForm (WallTableformPlacingZone *placingZone, short idx);			// 해당 셀의 좌하단 좌표X 위치를 리턴
short DGCALLBACK wallTableformPlacerHandler (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 테이블폼 배치를 위한 질의를 요청하는 다이얼로그

double		moveXinParallel (double prevPosX, double ang, double offset);			// 이동 후의 X 좌표를 알려줌 (Z 회전각도 고려) - 벽과 평행한 방향으로 이동
double		moveYinParallel (double prevPosY, double ang, double offset);			// 이동 후의 Y 좌표를 알려줌 (Z 회전각도 고려) - 벽과 평행한 방향으로 이동
double		moveXinPerpend (double prevPosX, double ang, double offset);			// 이동 후의 X 좌표를 알려줌 (Z 회전각도 고려) - 벽과 수직한 방향으로 이동
double		moveYinPerpend (double prevPosY, double ang, double offset);			// 이동 후의 Y 좌표를 알려줌 (Z 회전각도 고려) - 벽과 수직한 방향으로 이동
double		moveZ (double prevPosZ, double offset);									// 이동 후의 Z 좌표를 알려줌 (Z 회전각도 고려)

API_Guid	placeUFOM (paramsUFOM_ForWallTableform	params);			// 배치: 유로폼
API_Guid	placeSPIP (paramsSPIP_ForWallTableform	params);			// 배치: 비계 파이프
API_Guid	placePINB (paramsPINB_ForWallTableform	params);			// 배치: 핀볼트 세트
API_Guid	placeTIE  (paramsTIE_ForWallTableform	params);			// 배치: 벽체 타이
API_Guid	placeCLAM (paramsCLAM_ForWallTableform	params);			// 배치: 직교 클램프
API_Guid	placePUSH (paramsPUSH_ForWallTableform	params);			// 배치: 헤드피스

GSErrCode	tableformOnWall_w2300_h6000 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2300, 높이 6000)
GSErrCode	tableformOnWall_w2300_h5700 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2300, 높이 5700)
GSErrCode	tableformOnWall_w2300_h5400 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2300, 높이 5400)
GSErrCode	tableformOnWall_w2300_h5100 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2300, 높이 5100)
GSErrCode	tableformOnWall_w2300_h4800 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2300, 높이 4800)
GSErrCode	tableformOnWall_w2300_h4500 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2300, 높이 4500)
GSErrCode	tableformOnWall_w2300_h4200 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2300, 높이 4200)
GSErrCode	tableformOnWall_w2300_h3900 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2300, 높이 3900)
GSErrCode	tableformOnWall_w2300_h3600 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2300, 높이 3600)
GSErrCode	tableformOnWall_w2300_h3300 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2300, 높이 3300)
GSErrCode	tableformOnWall_w2300_h3000 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2300, 높이 3000)
GSErrCode	tableformOnWall_w2300_h2700 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2300, 높이 2700)
GSErrCode	tableformOnWall_w2300_h2400 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2300, 높이 2400)
GSErrCode	tableformOnWall_w2300_h2100 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2300, 높이 2100)
GSErrCode	tableformOnWall_w2300_h1800 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2300, 높이 1800)
GSErrCode	tableformOnWall_w2300_h1500 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2300, 높이 1500)

GSErrCode	tableformOnWall_w2250_h6000 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2250, 높이 6000)
GSErrCode	tableformOnWall_w2250_h5700 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2250, 높이 5700)
GSErrCode	tableformOnWall_w2250_h5400 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2250, 높이 5400)
GSErrCode	tableformOnWall_w2250_h5100 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2250, 높이 5100)
GSErrCode	tableformOnWall_w2250_h4800 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2250, 높이 4800)
GSErrCode	tableformOnWall_w2250_h4500 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2250, 높이 4500)
GSErrCode	tableformOnWall_w2250_h4200 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2250, 높이 4200)
GSErrCode	tableformOnWall_w2250_h3900 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2250, 높이 3900)
GSErrCode	tableformOnWall_w2250_h3600 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2250, 높이 3600)
GSErrCode	tableformOnWall_w2250_h3300 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2250, 높이 3300)
GSErrCode	tableformOnWall_w2250_h3000 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2250, 높이 3000)
GSErrCode	tableformOnWall_w2250_h2700 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2250, 높이 2700)
GSErrCode	tableformOnWall_w2250_h2400 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2250, 높이 2400)
GSErrCode	tableformOnWall_w2250_h2100 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2250, 높이 2100)
GSErrCode	tableformOnWall_w2250_h1800 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2250, 높이 1800)
GSErrCode	tableformOnWall_w2250_h1500 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2250, 높이 1500)

GSErrCode	tableformOnWall_w2200_h6000 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2200, 높이 6000)
GSErrCode	tableformOnWall_w2200_h5700 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2200, 높이 5700)
GSErrCode	tableformOnWall_w2200_h5400 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2200, 높이 5400)
GSErrCode	tableformOnWall_w2200_h5100 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2200, 높이 5100)
GSErrCode	tableformOnWall_w2200_h4800 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2200, 높이 4800)
GSErrCode	tableformOnWall_w2200_h4500 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2200, 높이 4500)
GSErrCode	tableformOnWall_w2200_h4200 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2200, 높이 4200)
GSErrCode	tableformOnWall_w2200_h3900 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2200, 높이 3900)
GSErrCode	tableformOnWall_w2200_h3600 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2200, 높이 3600)
GSErrCode	tableformOnWall_w2200_h3300 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2200, 높이 3300)
GSErrCode	tableformOnWall_w2200_h3000 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2200, 높이 3000)
GSErrCode	tableformOnWall_w2200_h2700 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2200, 높이 2700)
GSErrCode	tableformOnWall_w2200_h2400 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2200, 높이 2400)
GSErrCode	tableformOnWall_w2200_h2100 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2200, 높이 2100)
GSErrCode	tableformOnWall_w2200_h1800 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2200, 높이 1800)
GSErrCode	tableformOnWall_w2200_h1500 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2200, 높이 1500)

GSErrCode	tableformOnWall_w2150_h6000 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2150, 높이 6000)
GSErrCode	tableformOnWall_w2150_h5700 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2150, 높이 5700)
GSErrCode	tableformOnWall_w2150_h5400 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2150, 높이 5400)
GSErrCode	tableformOnWall_w2150_h5100 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2150, 높이 5100)
GSErrCode	tableformOnWall_w2150_h4800 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2150, 높이 4800)
GSErrCode	tableformOnWall_w2150_h4500 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2150, 높이 4500)
GSErrCode	tableformOnWall_w2150_h4200 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2150, 높이 4200)
GSErrCode	tableformOnWall_w2150_h3900 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2150, 높이 3900)
GSErrCode	tableformOnWall_w2150_h3600 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2150, 높이 3600)
GSErrCode	tableformOnWall_w2150_h3300 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2150, 높이 3300)
GSErrCode	tableformOnWall_w2150_h3000 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2150, 높이 3000)
GSErrCode	tableformOnWall_w2150_h2700 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2150, 높이 2700)
GSErrCode	tableformOnWall_w2150_h2400 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2150, 높이 2400)
GSErrCode	tableformOnWall_w2150_h2100 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2150, 높이 2100)
GSErrCode	tableformOnWall_w2150_h1800 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2150, 높이 1800)
GSErrCode	tableformOnWall_w2150_h1500 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2150, 높이 1500)

GSErrCode	tableformOnWall_w2100_h6000 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2100, 높이 6000)
GSErrCode	tableformOnWall_w2100_h5700 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2100, 높이 5700)
GSErrCode	tableformOnWall_w2100_h5400 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2100, 높이 5400)
GSErrCode	tableformOnWall_w2100_h5100 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2100, 높이 5100)
GSErrCode	tableformOnWall_w2100_h4800 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2100, 높이 4800)
GSErrCode	tableformOnWall_w2100_h4500 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2100, 높이 4500)
GSErrCode	tableformOnWall_w2100_h4200 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2100, 높이 4200)
GSErrCode	tableformOnWall_w2100_h3900 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2100, 높이 3900)
GSErrCode	tableformOnWall_w2100_h3600 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2100, 높이 3600)
GSErrCode	tableformOnWall_w2100_h3300 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2100, 높이 3300)
GSErrCode	tableformOnWall_w2100_h3000 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2100, 높이 3000)
GSErrCode	tableformOnWall_w2100_h2700 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2100, 높이 2700)
GSErrCode	tableformOnWall_w2100_h2400 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2100, 높이 2400)
GSErrCode	tableformOnWall_w2100_h2100 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2100, 높이 2100)
GSErrCode	tableformOnWall_w2100_h1800 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2100, 높이 1800)
GSErrCode	tableformOnWall_w2100_h1500 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2100, 높이 1500)

GSErrCode	tableformOnWall_w2050_h6000 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2050, 높이 6000)
GSErrCode	tableformOnWall_w2050_h5700 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2050, 높이 5700)
GSErrCode	tableformOnWall_w2050_h5400 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2050, 높이 5400)
GSErrCode	tableformOnWall_w2050_h5100 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2050, 높이 5100)
GSErrCode	tableformOnWall_w2050_h4800 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2050, 높이 4800)
GSErrCode	tableformOnWall_w2050_h4500 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2050, 높이 4500)
GSErrCode	tableformOnWall_w2050_h4200 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2050, 높이 4200)
GSErrCode	tableformOnWall_w2050_h3900 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2050, 높이 3900)
GSErrCode	tableformOnWall_w2050_h3600 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2050, 높이 3600)
GSErrCode	tableformOnWall_w2050_h3300 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2050, 높이 3300)
GSErrCode	tableformOnWall_w2050_h3000 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2050, 높이 3000)
GSErrCode	tableformOnWall_w2050_h2700 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2050, 높이 2700)
GSErrCode	tableformOnWall_w2050_h2400 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2050, 높이 2400)
GSErrCode	tableformOnWall_w2050_h2100 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2050, 높이 2100)
GSErrCode	tableformOnWall_w2050_h1800 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2050, 높이 1800)
GSErrCode	tableformOnWall_w2050_h1500 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2050, 높이 1500)

GSErrCode	tableformOnWall_w2000_h6000 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2000, 높이 6000)
GSErrCode	tableformOnWall_w2000_h5700 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2000, 높이 5700)
GSErrCode	tableformOnWall_w2000_h5400 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2000, 높이 5400)
GSErrCode	tableformOnWall_w2000_h5100 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2000, 높이 5100)
GSErrCode	tableformOnWall_w2000_h4800 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2000, 높이 4800)
GSErrCode	tableformOnWall_w2000_h4500 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2000, 높이 4500)
GSErrCode	tableformOnWall_w2000_h4200 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2000, 높이 4200)
GSErrCode	tableformOnWall_w2000_h3900 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2000, 높이 3900)
GSErrCode	tableformOnWall_w2000_h3600 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2000, 높이 3600)
GSErrCode	tableformOnWall_w2000_h3300 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2000, 높이 3300)
GSErrCode	tableformOnWall_w2000_h3000 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2000, 높이 3000)
GSErrCode	tableformOnWall_w2000_h2700 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2000, 높이 2700)
GSErrCode	tableformOnWall_w2000_h2400 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2000, 높이 2400)
GSErrCode	tableformOnWall_w2000_h2100 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2000, 높이 2100)
GSErrCode	tableformOnWall_w2000_h1800 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2000, 높이 1800)
GSErrCode	tableformOnWall_w2000_h1500 (CellForWallTableform cell);	// 테이블폼 배치 (너비 2000, 높이 1500)

GSErrCode	tableformOnWall_w1950_h6000 (CellForWallTableform cell);	// 테이블폼 배치 (너비 1950, 높이 6000)
GSErrCode	tableformOnWall_w1950_h5700 (CellForWallTableform cell);	// 테이블폼 배치 (너비 1950, 높이 5700)
GSErrCode	tableformOnWall_w1950_h5400 (CellForWallTableform cell);	// 테이블폼 배치 (너비 1950, 높이 5400)
GSErrCode	tableformOnWall_w1950_h5100 (CellForWallTableform cell);	// 테이블폼 배치 (너비 1950, 높이 5100)
GSErrCode	tableformOnWall_w1950_h4800 (CellForWallTableform cell);	// 테이블폼 배치 (너비 1950, 높이 4800)
GSErrCode	tableformOnWall_w1950_h4500 (CellForWallTableform cell);	// 테이블폼 배치 (너비 1950, 높이 4500)
GSErrCode	tableformOnWall_w1950_h4200 (CellForWallTableform cell);	// 테이블폼 배치 (너비 1950, 높이 4200)
GSErrCode	tableformOnWall_w1950_h3900 (CellForWallTableform cell);	// 테이블폼 배치 (너비 1950, 높이 3900)
GSErrCode	tableformOnWall_w1950_h3600 (CellForWallTableform cell);	// 테이블폼 배치 (너비 1950, 높이 3600)
GSErrCode	tableformOnWall_w1950_h3300 (CellForWallTableform cell);	// 테이블폼 배치 (너비 1950, 높이 3300)
GSErrCode	tableformOnWall_w1950_h3000 (CellForWallTableform cell);	// 테이블폼 배치 (너비 1950, 높이 3000)
GSErrCode	tableformOnWall_w1950_h2700 (CellForWallTableform cell);	// 테이블폼 배치 (너비 1950, 높이 2700)
GSErrCode	tableformOnWall_w1950_h2400 (CellForWallTableform cell);	// 테이블폼 배치 (너비 1950, 높이 2400)
GSErrCode	tableformOnWall_w1950_h2100 (CellForWallTableform cell);	// 테이블폼 배치 (너비 1950, 높이 2100)
GSErrCode	tableformOnWall_w1950_h1800 (CellForWallTableform cell);	// 테이블폼 배치 (너비 1950, 높이 1800)
GSErrCode	tableformOnWall_w1950_h1500 (CellForWallTableform cell);	// 테이블폼 배치 (너비 1950, 높이 1500)

GSErrCode	tableformOnWall_w1900_h6000 (CellForWallTableform cell);	// 테이블폼 배치 (너비 1900, 높이 6000)
GSErrCode	tableformOnWall_w1900_h5700 (CellForWallTableform cell);	// 테이블폼 배치 (너비 1900, 높이 5700)
GSErrCode	tableformOnWall_w1900_h5400 (CellForWallTableform cell);	// 테이블폼 배치 (너비 1900, 높이 5400)
GSErrCode	tableformOnWall_w1900_h5100 (CellForWallTableform cell);	// 테이블폼 배치 (너비 1900, 높이 5100)
GSErrCode	tableformOnWall_w1900_h4800 (CellForWallTableform cell);	// 테이블폼 배치 (너비 1900, 높이 4800)
GSErrCode	tableformOnWall_w1900_h4500 (CellForWallTableform cell);	// 테이블폼 배치 (너비 1900, 높이 4500)
GSErrCode	tableformOnWall_w1900_h4200 (CellForWallTableform cell);	// 테이블폼 배치 (너비 1900, 높이 4200)
GSErrCode	tableformOnWall_w1900_h3900 (CellForWallTableform cell);	// 테이블폼 배치 (너비 1900, 높이 3900)
GSErrCode	tableformOnWall_w1900_h3600 (CellForWallTableform cell);	// 테이블폼 배치 (너비 1900, 높이 3600)
GSErrCode	tableformOnWall_w1900_h3300 (CellForWallTableform cell);	// 테이블폼 배치 (너비 1900, 높이 3300)
GSErrCode	tableformOnWall_w1900_h3000 (CellForWallTableform cell);	// 테이블폼 배치 (너비 1900, 높이 3000)
GSErrCode	tableformOnWall_w1900_h2700 (CellForWallTableform cell);	// 테이블폼 배치 (너비 1900, 높이 2700)
GSErrCode	tableformOnWall_w1900_h2400 (CellForWallTableform cell);	// 테이블폼 배치 (너비 1900, 높이 2400)
GSErrCode	tableformOnWall_w1900_h2100 (CellForWallTableform cell);	// 테이블폼 배치 (너비 1900, 높이 2100)
GSErrCode	tableformOnWall_w1900_h1800 (CellForWallTableform cell);	// 테이블폼 배치 (너비 1900, 높이 1800)
GSErrCode	tableformOnWall_w1900_h1500 (CellForWallTableform cell);	// 테이블폼 배치 (너비 1900, 높이 1500)

GSErrCode	tableformOnWall_w1850_h6000 (CellForWallTableform cell);	// 테이블폼 배치 (너비 1850, 높이 6000)
GSErrCode	tableformOnWall_w1850_h5700 (CellForWallTableform cell);	// 테이블폼 배치 (너비 1850, 높이 5700)
GSErrCode	tableformOnWall_w1850_h5400 (CellForWallTableform cell);	// 테이블폼 배치 (너비 1850, 높이 5400)
GSErrCode	tableformOnWall_w1850_h5100 (CellForWallTableform cell);	// 테이블폼 배치 (너비 1850, 높이 5100)
GSErrCode	tableformOnWall_w1850_h4800 (CellForWallTableform cell);	// 테이블폼 배치 (너비 1850, 높이 4800)
GSErrCode	tableformOnWall_w1850_h4500 (CellForWallTableform cell);	// 테이블폼 배치 (너비 1850, 높이 4500)
GSErrCode	tableformOnWall_w1850_h4200 (CellForWallTableform cell);	// 테이블폼 배치 (너비 1850, 높이 4200)
GSErrCode	tableformOnWall_w1850_h3900 (CellForWallTableform cell);	// 테이블폼 배치 (너비 1850, 높이 3900)
GSErrCode	tableformOnWall_w1850_h3600 (CellForWallTableform cell);	// 테이블폼 배치 (너비 1850, 높이 3600)
GSErrCode	tableformOnWall_w1850_h3300 (CellForWallTableform cell);	// 테이블폼 배치 (너비 1850, 높이 3300)
GSErrCode	tableformOnWall_w1850_h3000 (CellForWallTableform cell);	// 테이블폼 배치 (너비 1850, 높이 3000)
GSErrCode	tableformOnWall_w1850_h2700 (CellForWallTableform cell);	// 테이블폼 배치 (너비 1850, 높이 2700)
GSErrCode	tableformOnWall_w1850_h2400 (CellForWallTableform cell);	// 테이블폼 배치 (너비 1850, 높이 2400)
GSErrCode	tableformOnWall_w1850_h2100 (CellForWallTableform cell);	// 테이블폼 배치 (너비 1850, 높이 2100)
GSErrCode	tableformOnWall_w1850_h1800 (CellForWallTableform cell);	// 테이블폼 배치 (너비 1850, 높이 1800)
GSErrCode	tableformOnWall_w1850_h1500 (CellForWallTableform cell);	// 테이블폼 배치 (너비 1850, 높이 1500)

GSErrCode	tableformOnWall_w1800_h6000 (CellForWallTableform cell);	// 테이블폼 배치 (너비 1800, 높이 6000)
GSErrCode	tableformOnWall_w1800_h5700 (CellForWallTableform cell);	// 테이블폼 배치 (너비 1800, 높이 5700)
GSErrCode	tableformOnWall_w1800_h5400 (CellForWallTableform cell);	// 테이블폼 배치 (너비 1800, 높이 5400)
GSErrCode	tableformOnWall_w1800_h5100 (CellForWallTableform cell);	// 테이블폼 배치 (너비 1800, 높이 5100)
GSErrCode	tableformOnWall_w1800_h4800 (CellForWallTableform cell);	// 테이블폼 배치 (너비 1800, 높이 4800)
GSErrCode	tableformOnWall_w1800_h4500 (CellForWallTableform cell);	// 테이블폼 배치 (너비 1800, 높이 4500)
GSErrCode	tableformOnWall_w1800_h4200 (CellForWallTableform cell);	// 테이블폼 배치 (너비 1800, 높이 4200)
GSErrCode	tableformOnWall_w1800_h3900 (CellForWallTableform cell);	// 테이블폼 배치 (너비 1800, 높이 3900)
GSErrCode	tableformOnWall_w1800_h3600 (CellForWallTableform cell);	// 테이블폼 배치 (너비 1800, 높이 3600)
GSErrCode	tableformOnWall_w1800_h3300 (CellForWallTableform cell);	// 테이블폼 배치 (너비 1800, 높이 3300)
GSErrCode	tableformOnWall_w1800_h3000 (CellForWallTableform cell);	// 테이블폼 배치 (너비 1800, 높이 3000)
GSErrCode	tableformOnWall_w1800_h2700 (CellForWallTableform cell);	// 테이블폼 배치 (너비 1800, 높이 2700)
GSErrCode	tableformOnWall_w1800_h2400 (CellForWallTableform cell);	// 테이블폼 배치 (너비 1800, 높이 2400)
GSErrCode	tableformOnWall_w1800_h2100 (CellForWallTableform cell);	// 테이블폼 배치 (너비 1800, 높이 2100)
GSErrCode	tableformOnWall_w1800_h1800 (CellForWallTableform cell);	// 테이블폼 배치 (너비 1800, 높이 1800)
GSErrCode	tableformOnWall_w1800_h1500 (CellForWallTableform cell);	// 테이블폼 배치 (너비 1800, 높이 1500)

#endif