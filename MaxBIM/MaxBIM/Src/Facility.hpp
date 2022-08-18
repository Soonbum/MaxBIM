#ifndef	__FACILITY__
#define __FACILITY__

#include "MaxBIM.hpp"

namespace FacilityDG {
	// 원형 버블 방향
	enum	circularBubblePos {
		UP = 1,
		DOWN,
		LEFT,
		RIGHT
	};

	enum	dgCircularBubbleSettings {
		LABEL_DIAMETER = 3,
		EDITCONTROL_DIAMETER,
		LABEL_LETTER_SIZE,
		EDITCONTROL_LETTER_SIZE,
		LABEL_WITHDRAWAL_LENGTH,
		EDITCONTROL_WITHDRAWAL_LENGTH,
		LABEL_BUBBLE_POS,
		POPUPCONTROL_BUBBLE_POS,

		ICON_LAYER,
		LABEL_LAYER_SETTINGS,
		LABEL_LAYER_CIRCULAR_BUBBLE,
		USERCONTROL_LAYER_CIRCULAR_BUBBLE
	};

	enum	dgCameraManager {
		BUTTON_LOAD = 3,
		BUTTON_CLOSE,
		LISTVIEW_CAMERA_POS_NAME,
		EDITCONTROL_CAMERA_POS_NAME
	};
}

struct CircularBubble
{
	short	floorInd;		// 층 인덱스

	short	layerInd;		// 레이어 인덱스
	short	pos;			// 위치 (UP/DOWN/LEFT/RIGHT)
	double	szBubbleDia;	// 버블 직경
	double	szFont;			// 글자 크기
	double	lenWithdrawal;	// 인출선 길이 (오프셋)
};

GSErrCode	select3DQuality (void);					// 3D 품질/속도 조정하기
GSErrCode	attach3DLabelOnZone (void);				// 영역에 3D 라벨 붙이기
short DGCALLBACK selectLayerHandler (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// [다이얼로그 박스] 레이어 선택

GSErrCode	attachBubbleOnCurrentFloorPlan (void);	// 현재 평면도의 테이블폼에 버블 자동 배치
short DGCALLBACK setBubbleHandler (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// [다이얼로그 박스] 원형 버블 설정
GSErrCode	saveDialogStatus_bubble (CircularBubble	*cbInfo);		// 원형 버블 설정 상태 저장
GSErrCode	loadDialogStatus_bubble (CircularBubble	*cbInfo);		// 원형 버블 설정 상태 로드

GSErrCode	manageCameraInfo (void);				// 카메라 위치 저장하기/불러오기
short DGCALLBACK cameraPosManagerHandler (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// [다이얼로그 박스] 카메라 위치 관리하기

#endif