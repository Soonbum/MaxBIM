#ifndef	__SUPPORTING_POST__PLACER__
#define __SUPPORTING_POST__PLACER__

#include "MaxBIM.hpp"

namespace SupportingPostPlacerDG {
	// 다이얼로그 항목 인덱스
	enum	idxItems_1_forPERISupportingPostPlacer {
		LABEL_TYPE = 3,
		POPUP_TYPE,

		LABEL_SIDE_VIEW,
		LABEL_PLAN_VIEW,

		LABEL_UPWARD,
		SEPARATOR_UPWARD_BORDER,
		SEPARATOR_TIMBER,
		LABEL_TIMBER,
		SEPARATOR_CROSSHEAD,
		SEPARATOR_VERTICAL_2ND,
		LABEL_VERTICAL_2ND,
		SEPARATOR_VERTICAL_1ST,
		LABEL_VERTICAL_1ST,
		SEPARATOR_DOWNWARD_BORDER,
		LABEL_DOWNWARD,

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

		LABEL_TIMBER_HEIGHT,
		LABEL_CROSSHEAD_HEIGHT,

		ICON_LAYER,
		LABEL_LAYER_SETTINGS,
		CHECKBOX_LAYER_COUPLING,
		LABEL_LAYER_SUPPORT,
		LABEL_LAYER_VPOST,
		LABEL_LAYER_HPOST,
		LABEL_LAYER_TIMBER,
		LABEL_LAYER_GIRDER,
		LABEL_LAYER_BEAM_BRACKET,
		LABEL_LAYER_YOKE,

		USERCONTROL_LAYER_SUPPORT,
		USERCONTROL_LAYER_VPOST,
		USERCONTROL_LAYER_HPOST,
		USERCONTROL_LAYER_TIMBER,
		USERCONTROL_LAYER_GIRDER,
		USERCONTROL_LAYER_BEAM_BRACKET,
		USERCONTROL_LAYER_YOKE,

		SEPARATOR_CENTER,

		LABEL_TOTAL_WIDTH,
		EDITCONTROL_TOTAL_WIDTH,
		LABEL_EXPLANATION,

		LABEL_TOTAL_LENGTH,
		EDITCONTROL_TOTAL_LENGTH,
		LABEL_REMAIN_LENGTH,
		EDITCONTROL_REMAIN_LENGTH
	};
}

// 모프 관련 정보
struct InfoMorphForSupportingPost
{
	API_Guid	guid;		// 모프의 GUID
	short		floorInd;	// 층 인덱스

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

// PERI 동바리 배치 관련 정보
class PERISupportingPostPlacementInfo
{
public:
	bool	bFlipped;				// 가로/세로 길이 방향이 뒤바뀌어 있는가?
	double	width;					// 영역 가로 길이
	double	depth;					// 영역 세로 길이

	// ... 아래 검토할 것
	char	nomVPost1 [16];			// 수직재 1단 규격 - GDL의 수직재 규격과 동일
	double	heightVPost1;			// 수직재 1단 높이

	bool	bVPost2;				// 수직재 2단 유무
	char	nomVPost2 [16];			// 수직재 2단 규격 - GDL의 수직재 규격과 동일
	double	heightVPost2;			// 수직재 2단 높이

	bool	bCrosshead;				// 크로스헤드 유무
	double	heightCrosshead;		// 크로스헤드 높이

	double	heightTimber;			// 산승각/토류판/GT24거더 또는 보 멍에제 높이

	bool	bHPost;					// 수평재 유무
	char	nomHPost_North [16];	// 수평재 너비(북) - GDL의 수평재 규격과 동일, 없음의 경우 빈 문자열
	char	nomHPost_West [16];		// 수평재 너비(서) - GDL의 수평재 규격과 동일, 없음의 경우 빈 문자열
	char	nomHPost_East [16];		// 수평재 너비(동) - GDL의 수평재 규격과 동일, 없음의 경우 빈 문자열
	char	nomHPost_South [16];	// 수평재 너비(남) - GDL의 수평재 규격과 동일, 없음의 경우 빈 문자열

public:
	API_Guid	placeVPost (PERI_VPost params);		// 수직재 배치
	API_Guid	placeHPost (PERI_HPost params);		// 수평재 배치
};

GSErrCode	placePERIPost (void);		// 선택한 직육면체 모프를 기반으로 PERI 동바리를 배치함
short DGCALLBACK PERISupportingPostPlacerHandler1 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 동바리 설치 옵션을 설정함

#endif