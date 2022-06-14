#ifndef	__FACILITY__
#define __FACILITY__

#include "MaxBIM.hpp"

GSErrCode	select3DQuality (void);					// 3D 품질/속도 조정하기
GSErrCode	attach3DLabelOnZone (void);				// 영역에 3D 라벨 붙이기
GSErrCode	attachBubbleOnCurrentFloorPlan (void);	// 현재 평면도의 테이블폼에 버블 자동 배치
short DGCALLBACK selectLayerHandler (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// [다이얼로그 박스] 레이어 선택

#endif