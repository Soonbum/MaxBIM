#ifndef	__LIBRARY_CONVERT__
#define __LIBRARY_CONVERT__

#include "MaxBIM.hpp"

GSErrCode	convertVirtualTCO (void);		// 모든 가상 가설재(TCO: Temporary Construction Object)를 실제 가설재로 변환함
GSErrCode	placeTableformOnWall (WallTableform params);		// 테이블폼(벽) 배치
GSErrCode	placeTableformOnSlabBottom (SlabTableform params);	// 테이블폼(슬래브) 배치
GSErrCode	placeEuroform (Euroform params);					// 유로폼/스틸폼 배치
GSErrCode	placePlywood (Plywood params);						// 합판 배치
GSErrCode	placeFillersp (FillerSpacer params);				// 휠러스페이서 배치
GSErrCode	placeOutcornerAngle (OutcornerAngle params);		// 아웃코너앵글 배치
GSErrCode	placeOutcornerPanel (OutcornerPanel params);		// 아웃코너판넬 배치
GSErrCode	placeIncornerPanel (IncornerPanel params);			// 인코너판넬 배치

#endif