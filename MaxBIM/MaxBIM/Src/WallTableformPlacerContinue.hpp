#ifndef	__WALL_TABLEFORM_PLACER_CONTINUE__
#define __WALL_TABLEFORM_PLACER_CONTINUE__

#include "MaxBIM.hpp"

namespace wallTableformPlacerContinueDG {
}

// 모프 관련 정보
struct InfoMorphForWallTableformContinue
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

GSErrCode	placeTableformOnWallContinually (void);		// 벽에 연속으로 테이블폼을 배치하는 통합 루틴

#endif