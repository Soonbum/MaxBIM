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

		LABEL_VPOST1,
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

		BUTTON_ADD,
		BUTTON_DEL,

		LABEL_TOTAL_WIDTH,
		EDITCONTROL_TOTAL_WIDTH,
		LABEL_EXPLANATION,

		LABEL_TOTAL_LENGTH,
		EDITCONTROL_TOTAL_LENGTH,
		LABEL_REMAIN_LENGTH,
		EDITCONTROL_REMAIN_LENGTH,

		AFTER_ALL
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
	double	leftBottomX;			// 최하단 좌표 X
	double	leftBottomY;			// 최하단 좌표 Y
	double	leftBottomZ;			// 최하단 좌표 Z

	double	ang;					// 회전 각도 (단위: Degree, 회전축: Z축)

	bool	bFlipped;				// 가로/세로 길이 방향이 뒤바뀌어 있는가?
	double	width;					// 영역 가로 길이
	double	depth;					// 영역 세로 길이

	char	nameVPost1 [64];		// 수직재 1단 GDL 이름
	char	nomVPost1 [16];			// 수직재 1단 규격 - GDL의 수직재 규격과 동일
	double	heightVPost1;			// 수직재 1단 높이

	bool	bVPost2;				// 수직재 2단 유무
	char	nameVPost2 [64];		// 수직재 2단 GDL 이름
	char	nomVPost2 [16];			// 수직재 2단 규격 - GDL의 수직재 규격과 동일
	double	heightVPost2;			// 수직재 2단 높이

	bool	bCrosshead;				// 크로스헤드 유무
	double	heightCrosshead;		// 크로스헤드 높이

	char	nameTimber [64];		// 산승각/토류판/GT24거더 또는 보 멍에제 - 객체 이름
	double	heightTimber;			// 산승각/토류판/GT24거더 또는 보 멍에제 높이

	short	nColVPost;				// 너비 방향의 수직재 쌍의 개수 (최소 1개, 최대 5개까지 가능)

	bool	bHPost_center [5];			// 수평재 유무 [가운데]
	bool	bHPost_up [5];				// 수평재 유무 [위쪽], 단 [0]은 사용하지 않음
	bool	bHPost_down [5];			// 수평재 유무 [아래쪽], 단 [0]은 사용하지 않음

	char	sizeHPost_center [5][8];	// 수평재 규격 [가운데]
	char	sizeHPost_up [5][8];		// 수평재 규격 [위쪽], 단 [0]은 사용하지 않음
	char	sizeHPost_down [5][8];		// 수평재 규격 [아래쪽], 단 [0]은 사용하지 않음

	double	lengthHPost_center [5];		// 수평 거리 [가운데]
	double	lengthHPost_up [5];			// 수평 거리 [위쪽]
	double	lengthHPost_down [5];		// 수평 거리 [아래쪽]

public:
	API_Guid	placeVPost (PERI_VPost params);				// 수직재 배치
	API_Guid	placeHPost (PERI_HPost params);				// 수평재 배치
	API_Guid	placeSupport (SuppPost params);				// 서포트(동바리) 배치
	API_Guid	placeTimber (Wood params);					// 목재(산승각/토류판) 배치
	API_Guid	placeGT24Girder (GT24Girder params);		// GT24 거더 배치
	API_Guid	placeBeamBracket (BlueBeamBracket params);	// 블루 보 브라켓 배치
	API_Guid	placeYoke (Yoke params);					// 보 멍에제 배치
};

GSErrCode	placePERIPost (void);		// 선택한 직육면체 모프를 기반으로 PERI 동바리를 배치함
short DGCALLBACK PERISupportingPostPlacerHandler1 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 동바리 설치 옵션을 설정함

#endif