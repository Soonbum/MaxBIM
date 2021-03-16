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

GSErrCode	placeTableformOnWall (void);	// 벽에 테이블폼을 배치하는 통합 루틴
short DGCALLBACK wallTableformPlacerHandler (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 테이블폼 배치를 위한 질의를 요청하는 다이얼로그

#endif