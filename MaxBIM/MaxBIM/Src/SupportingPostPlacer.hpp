#ifndef	__SUPPORTING_POST__PLACER__
#define __SUPPORTING_POST__PLACER__

#include "MaxBIM.hpp"

namespace SupportingPostPlacerDG {
	// 다이얼로그 항목 인덱스
	enum	idxItems_1_forPERISupportingPostPlacer {
		LABEL_VPOST = 3,
		LABEL_HPOST,

		LABEL_TOTAL_HEIGHT,
		EDITCONTROL_TOTAL_HEIGHT,
		LABEL_REMAIN_HEIGHT,
		EDITCONTROL_REMAIN_HEIGHT,

		CHECKBOX_CROSSHEAD,

		CHECKBOX_VPOST1,
		LABEL_VPOST1_NOMINAL,
		POPUP_VPOST1_NOMINAL,
		LABEL_VPOST1_HEIGHT,
		EDITCONTROL_VPOST1_HEIGHT,

		CHECKBOX_VPOST2,
		LABEL_VPOST2_NOMINAL,
		POPUP_VPOST2_NOMINAL,
		LABEL_VPOST2_HEIGHT,
		EDITCONTROL_VPOST2_HEIGHT,

		SEPARATOR_VPOST_L,
		SEPARATOR_VPOST_R,
		CHECKBOX_HPOST,
		SEPARATOR_HPOST_UP,
		SEPARATOR_HPOST_DOWN,

		SEPARATOR_HPOST_PLAN,

		LABEL_PLAN_WIDTH,
		EDITCONTROL_PLAN_WIDTH,
		LABEL_PLAN_DEPTH,
		EDITCONTROL_PLAN_DEPTH,

		LABEL_WIDTH_NORTH,
		POPUP_WIDTH_NORTH,
		LABEL_WIDTH_WEST,
		POPUP_WIDTH_WEST,
		LABEL_WIDTH_EAST,
		POPUP_WIDTH_EAST,
		LABEL_WIDTH_SOUTH,
		POPUP_WIDTH_SOUTH,

		ICON_LAYER,
		LABEL_LAYER_SETTINGS,
		CHECKBOX_LAYER_COUPLING,
		LABEL_LAYER_VPOST,
		LABEL_LAYER_HPOST,

		USERCONTROL_LAYER_VPOST,
		USERCONTROL_LAYER_HPOST
	};
}

// 모프 관련 정보
struct InfoMorphForSupportingPost
{
	API_Guid	guid;		// 모프의 GUID

	double	width;			// 가로 길이
	double	depth;			// 세로 길이
	double	height;			// 높이 길이

	double	ang;			// 회전 각도 (단위: Degree, 회전축: Z축)

	double	leftBottomX;	// 최하단 좌표 X
	double	leftBottomY;	// 최하단 좌표 Y
	double	leftBottomZ;	// 최하단 좌표 Z

	double	rightTopX;		// 우상단 좌표 X
	double	rightTopY;		// 우상단 좌표 Y
	double	rightTopZ;		// 우상단 좌표 Z

	API_Coord3D	points [8];	// 8개의 꼭지점 좌표
};

GSErrCode	placePERIPost (void);		// 선택한 직육면체 모프를 기반으로 PERI 동바리를 배치함
short DGCALLBACK PERISupportingPostPlacerHandler1 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 수직재 단수 (1/2단, 높이가 6미터 초과되면 2단 권유할 것), 수직재의 규격/높이, 수평재 유무(단, 높이가 3500 이상이면 추가할 것을 권유할 것), 수평재 너비, 크로스헤드 유무, 수직재/수평재 레이어를 설정

#endif