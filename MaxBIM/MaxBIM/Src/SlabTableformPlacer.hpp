#ifndef	__SLAB_TABLEFORM_PLACER__
#define __SLAB_TABLEFORM_PLACER__

#include "MaxBIM.hpp"

namespace slabTableformPlacerDG {
}

// 모프 관련 정보
struct InfoMorphForSlabTableform
{
	API_Guid	guid;		// 모프의 GUID
	short		floorInd;	// 층 인덱스
	double		level;		// 모프의 고도
};





GSErrCode	placeTableformOnSlabBottom (void);											// 슬래브 하부에 테이블폼을 배치하는 통합 루틴

#endif