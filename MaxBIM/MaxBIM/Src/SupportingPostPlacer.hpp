#ifndef	__SUPPORTING_POST__PLACER__
#define __SUPPORTING_POST__PLACER__

#include "MaxBIM.hpp"

// 모프 관련 정보
struct InfoMorphForSupportingPost
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

GSErrCode	placePERIPost (void);		// 선택한 직육면체 모프를 기반으로 PERI 동바리를 배치함

#endif