#ifndef	__WALL_TABLEFORM_PLACER__
#define __WALL_TABLEFORM_PLACER__

#include "MaxBIM.hpp"

namespace wallTableformPlacerDG {
	// 다이얼로그 항목 인덱스
	enum	idxItems_1_forLayerSelection {
		ICON_LAYER_CUSTOM = 3,
		LABEL_LAYER_SETTINGS_CUSTOM,
		CHECKBOX_LAYER_COUPLING_CUSTOM,

		LABEL_LAYER_SLABTABLEFORM_CUSTOM,
		LABEL_LAYER_PROFILE_CUSTOM,
		LABEL_LAYER_EUROFORM_CUSTOM,
		LABEL_LAYER_RECTPIPE_CUSTOM,
		LABEL_LAYER_PINBOLT_CUSTOM,
		LABEL_LAYER_WALLTIE_CUSTOM,
		LABEL_LAYER_JOIN_CUSTOM,
		LABEL_LAYER_HEADPIECE_CUSTOM,
		LABEL_LAYER_STEELFORM_CUSTOM,
		LABEL_LAYER_PLYWOOD_CUSTOM,
		LABEL_LAYER_FILLERSP_CUSTOM,
		LABEL_LAYER_OUTCORNER_ANGLE_CUSTOM,
		LABEL_LAYER_OUTCORNER_PANEL_CUSTOM,
		LABEL_LAYER_INCORNER_PANEL_CUSTOM,
		LABEL_LAYER_RECTPIPE_HANGER_CUSTOM,
		LABEL_LAYER_EUROFORM_HOOK_CUSTOM,
		LABEL_LAYER_HIDDEN_CUSTOM,

		USERCONTROL_LAYER_SLABTABLEFORM_CUSTOM,
		USERCONTROL_LAYER_PROFILE_CUSTOM,
		USERCONTROL_LAYER_EUROFORM_CUSTOM,
		USERCONTROL_LAYER_RECTPIPE_CUSTOM,
		USERCONTROL_LAYER_PINBOLT_CUSTOM,
		USERCONTROL_LAYER_WALLTIE_CUSTOM,
		USERCONTROL_LAYER_JOIN_CUSTOM,
		USERCONTROL_LAYER_HEADPIECE_CUSTOM,
		USERCONTROL_LAYER_STEELFORM_CUSTOM,
		USERCONTROL_LAYER_PLYWOOD_CUSTOM,
		USERCONTROL_LAYER_FILLERSP_CUSTOM,
		USERCONTROL_LAYER_OUTCORNER_ANGLE_CUSTOM,
		USERCONTROL_LAYER_OUTCORNER_PANEL_CUSTOM,
		USERCONTROL_LAYER_INCORNER_PANEL_CUSTOM,
		USERCONTROL_LAYER_RECTPIPE_HANGER_CUSTOM,
		USERCONTROL_LAYER_EUROFORM_HOOK_CUSTOM,
		USERCONTROL_LAYER_HIDDEN_CUSTOM,

		BUTTON_AUTOSET_CUSTOM
	};

	enum	objType_forWallTableformPlacer {
		NONE = 1,	// 없음
		TABLEFORM,	// 테이블폼
		EUROFORM,	// 유로폼
		FILLERSP,	// 휠러스페이서
		PLYWOOD,	// 합판
		TIMBER		// 각재
	};
}

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

	double	horLen;			// 가로 길이
	double	verLen;			// 세로 길이
	double	ang;			// 회전 각도 (단위: Radian, 회전축: Z축)

	short	objType;		// 객체 타입: 없음, 테이블폼, 유로폼, 휠러스페이서, 합판, 각재
	
	// 테이블폼 내 유로폼 길이
	int		tableInHor [10];	// 가로 방향
	int		tableInVer [10];	// 세로 방향
};

// 그리드 각 상단 셀 정보
struct UpperCellForWallTableform
{
	double	leftBottomX;	// 좌하단 좌표 X
	double	leftBottomY;	// 좌하단 좌표 Y
	double	leftBottomZ;	// 좌하단 좌표 Z

	double	ang;			// 회전 각도 (단위: Radian, 회전축: Z축)

	bool	bFill;			// 채우기 여부

	bool	bEuroform1;				// 1단 유로폼 여부
	bool	bEuroformStandard1;		// 1단 유로폼이 규격폼인지 여부
	double	formWidth1;				// 1단 유로폼의 폭
	bool	bEuroform2;				// 2단 유로폼 여부
	bool	bEuroformStandard2;		// 2단 유로폼이 규격폼인지 여부
	double	formWidth2;				// 2단 유로폼의 폭
};

// 벽면 영역 정보 (통합)
class WallTableformPlacingZone
{
public:
	double	leftBottomX;	// 좌하단 좌표 X
	double	leftBottomY;	// 좌하단 좌표 Y
	double	leftBottomZ;	// 좌하단 좌표 Z

	double	horLen;			// 가로 길이
	double	verLenBasic;	// 세로 길이 (낮은쪽)
	double	verLenExtra;	// 세로 길이 (높은쪽)
	double	ang;			// 회전 각도 (단위: Radian, 회전축: Z축)

	bool	bVertical;		// 방향: 수직(true), 수평(false)

	double	gap;			// 벽과의 간격

	bool	bExtra;			// 높은쪽 모프가 있는가?

	double	remainWidth;		// 남은 너비
	double	remainHeightBasic;	// 남은 높이 (낮은쪽)
	double	remainHeightExtra;	// 남은 높이 (높은쪽)

	bool	bLincorner;			// 왼쪽 인코너 유무
	bool	bRincorner;			// 오른쪽 인코너 유무
	double	lenLincorner;		// 왼쪽 인코너 길이
	double	lenRincorner;		// 오른쪽 인코너 길이

	short	tableformType;		// 테이블폼 타입: 타입A (1), 타입B (2), 타입C (3), 타입D (4)

	short	nCellsInHor;		// 수평 방향 셀(유로폼) 개수
	short	nCellsInVerBasic;	// 수직 방향 셀(유로폼) 개수 (낮은쪽)
	short	nCellsInVerExtra;	// 수직 방향 셀(유로폼) 개수 (높은쪽)

	CellForWallTableform		cells [50];			// 셀 배열 (인코너 제외)
	UpperCellForWallTableform	upperCells [50];	// 상단 여백 셀 배열

	double	marginTopBasic;		// 상단 여백 (낮은쪽)
	double	marginTopExtra;		// 상단 여백 (높은쪽)

	int	presetWidth_tableform [38];		// 세로 방향 테이블폼의 너비 모음 (2300 ... 400)
	int	presetHeight_tableform [16];	// 세로 방향 테이블폼의 높이 모음 (6000 ... 1500)

	int	presetWidth_euroform [6];		// 세로 방향 유로폼의 너비 모음 (600 ... 200)
	int	presetHeight_euroform [3];		// 세로 방향 유로폼의 높이 모음 (1200 ... 600)

	int	presetWidth_config_vertical [38][5];	// 세로 방향 테이블폼 내 유로폼의 배열 순서
	int	presetHeight_config_vertical [16][6];	// 세로 방향 테이블폼 내 유로폼의 배열 순서
	int	presetWidth_config_horizontal [16][6];	// 가로 방향 테이블폼 내 유로폼의 배열 순서
	int	presetHeight_config_horizontal [38][5];	// 가로 방향 테이블폼 내 유로폼의 배열 순서

public:
	WallTableformPlacingZone ();									// 기본 생성자
	void	initCells (WallTableformPlacingZone* placingZone);		// 셀 정보 초기화
};

GSErrCode	placeTableformOnWall (void);				// 벽에 테이블폼을 배치하는 통합 루틴 - 전체 통합
short DGCALLBACK wallTableformPlacerHandler1 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);				// 테이블폼/유로폼/휠러스페이서/합판/목재 배치를 위한 다이얼로그 (테이블폼 구성, 요소 방향, 개수 및 길이)
short DGCALLBACK wallTableformPlacerHandler2 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);				// 객체의 레이어를 선택하기 위한 다이얼로그

#endif