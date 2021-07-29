#ifndef	__BEAM_TABLEFORM_PLACER__
#define __BEAM_TABLEFORM_PLACER__

#include "MaxBIM.hpp"

namespace beamTableformPlacerDG {
	// 객체 타입
	enum	libPartObjType {
		NONE,			// 없음
		EUROFORM,		// 유로폼v2.0
		PLYWOOD,		// 합판v1.0
		WOOD,			// 목재v1.0
		OUTCORNER_ANGLE,// 아웃코너앵글v1.0
		FILLERSPACER	// 휠러스페이서v1.0
	};

	// 부착되는 면 위치
	enum	attachedSide {
		BOTTOM_SIDE,
		LEFT_SIDE,
		RIGHT_SIDE
	};

	// 다이얼로그 항목 인덱스
	enum	idxItems_1_forBeamPlacer {
		LABEL_BEAM_SECTION		= 3,
		SEPARATOR_1_BEAM_SECTION,
		LABEL_BEAM_HEIGHT,
		EDITCONTROL_BEAM_HEIGHT,
		LABEL_BEAM_WIDTH,
		EDITCONTROL_BEAM_WIDTH,
		
		SEPARATOR_2_LEFT,
		SEPARATOR_3_BOTTOM,
		SEPARATOR_4_RIGHT,
		EDITCONTROL_GAP_SIDE1,
		EDITCONTROL_GAP_BOTTOM,
		EDITCONTROL_GAP_SIDE2,

		LABEL_TOTAL_HEIGHT,
		EDITCONTROL_TOTAL_HEIGHT,
		LABEL_TOTAL_WIDTH,
		EDITCONTROL_TOTAL_WIDTH,

		LABEL_REST_SIDE,
		CHECKBOX_WOOD_SIDE,
		CHECKBOX_T_FORM_SIDE,
		CHECKBOX_FILLER_SIDE,
		CHECKBOX_B_FORM_SIDE,

		EDITCONTROL_REST_SIDE,
		EDITCONTROL_WOOD_SIDE,
		POPUP_T_FORM_SIDE,
		EDITCONTROL_FILLER_SIDE,
		POPUP_B_FORM_SIDE,

		CHECKBOX_L_FORM_BOTTOM,
		CHECKBOX_FILLER_BOTTOM,
		CHECKBOX_R_FORM_BOTTOM,

		POPUP_L_FORM_BOTTOM,
		EDITCONTROL_FILLER_BOTTOM,
		POPUP_R_FORM_BOTTOM,

		ICON_LAYER,
		LABEL_LAYER_SETTINGS,
		CHECKBOX_LAYER_COUPLING,
		LABEL_LAYER_EUROFORM,
		LABEL_LAYER_PLYWOOD,
		LABEL_LAYER_WOOD,
		LABEL_LAYER_OUTCORNER_ANGLE,
		LABEL_LAYER_FILLERSPACER,
		LABEL_LAYER_RECTPIPE,
		LABEL_LAYER_RECTPIPE_HANGER,
		LABEL_LAYER_EUROFORM_HOOK,
		LABEL_LAYER_TIMBER_RAIL,

		USERCONTROL_LAYER_EUROFORM,
		USERCONTROL_LAYER_PLYWOOD,
		USERCONTROL_LAYER_WOOD,
		USERCONTROL_LAYER_OUTCORNER_ANGLE,
		USERCONTROL_LAYER_FILLERSPACER,
		USERCONTROL_LAYER_RECTPIPE,
		USERCONTROL_LAYER_RECTPIPE_HANGER,
		USERCONTROL_LAYER_EUROFORM_HOOK,
		USERCONTROL_LAYER_TIMBER_RAIL
	};

	enum	idxItems_2_forBeamPlacer {
		DG_UPDATE_BUTTON		= 3,
		DG_PREV,
		LABEL_BEAM_SIDE_BOTTOM,
		LABEL_BEAM_SIDE,
		LABEL_BEAM_BOTTOM,
		AFTER_ALL
	};

	enum	idxItems_3_forBeamPlacer {
		LABEL_OBJ_TYPE			= 3,
		POPUP_OBJ_TYPE,
		CHECKBOX_SET_STANDARD,
		LABEL_LENGTH,
		EDITCONTROL_LENGTH,
		POPUP_LENGTH
	};

	enum	cellType {
		FROM_BEGIN_AT_SIDE = 1,
		CENTER_AT_SIDE,
		FROM_END_AT_SIDE,
		FROM_BEGIN_AT_BOTTOM,
		CENTER_AT_BOTTOM,
		FROM_END_AT_BOTTOM
	};
}

// 모프 관련 정보
struct InfoMorphForBeamTableform
{
	API_Guid	guid;		// 모프의 GUID
	short		floorInd;	// 층 인덱스
	double		level;		// 모프의 고도
};

// 그리드 각 셀 정보
struct CellForBeamTableform
{
	short		objType;	// enum libPartObjType 참조

	API_Guid	guid;		// 객체의 GUID

	double	leftBottomX;	// 좌하단 좌표 X
	double	leftBottomY;	// 좌하단 좌표 Y
	double	leftBottomZ;	// 좌하단 좌표 Z
	double	ang;			// 회전 각도 (단위: Radian, 회전축: Z축)

	double	dirLen;			// 보 설치방향 길이
	double	perLen;			// 보 직각방향 길이
	short	attached_side;	// 하부(BOTTOM_SIDE), 좌측면(LEFT_SIDE), 우측면(RIGHT_SIDE)

	union {
		Euroform		form;
		Plywood			plywood;
		FillerSpacer	fillersp;
		Wood			wood;
		OutcornerAngle	outangle;
	} libPart;
};

// 보 영역 정보
class BeamTableformPlacingZone
{
public:
	// 보의 시작 좌표는 왼쪽, 끝 좌표는 오른쪽이라고 가정함

	// 보 기하 정보
	double	level;				// 보 윗면 고도
	double	ang;				// 회전 각도 (단위: Radian, 회전축: Z축)
	double	areaHeight;			// 영역 높이

	API_Coord3D		begC;		// 배치 기준 시작점
	API_Coord3D		endC;		// 배치 기준 끝점
	double	beamLength;			// 메인 보 전체 길이

	// 간섭 보 관련 정보
	bool	bInterfereBeam;				// 간섭 보 여부
	double	posInterfereBeamFromLeft;	// 간섭 보 위치 (보 시작 좌표-왼쪽-로부터)
	double	interfereBeamWidth;			// 간섭 보 너비
	double	interfereBeamHeight;		// 간섭 보 높이

	// 간섭 보가 들어오는 영역 길이
	double	centerLengthAtSide;			// 측면

	// 보 양끝 여백
	double	marginBeginAtSide;				// 측면 시작 부분 여백
	double	marginEndAtSide;				// 측면 끝 부분 여백
	double	marginBeginAtBottom;			// 하부 시작 부분 여백
	double	marginEndAtBottom;				// 하부 끝 부분 여백

	// 보 양끝 여백 채울지 여부
	bool	bFillMarginBeginAtSide;			// 측면 시작 부분 여백 채움
	bool	bFillMarginEndAtSide;			// 측면 끝 부분 여백 채움
	bool	bFillMarginBeginAtBottom;		// 하부 시작 부분 여백 채움
	bool	bFillMarginEndAtBottom;			// 하부 끝 부분 여백 채움

	// 셀 정보 (측면)
	CellForBeamTableform	cellsFromBeginAtLSide [4][50];	// 시작 부분부터 붙여가는 셀 (왼쪽)
	CellForBeamTableform	cellsFromBeginAtRSide [4][50];	// 시작 부분부터 붙여가는 셀 (오른쪽)
	CellForBeamTableform	cellsFromEndAtLSide [4][50];	// 끝 부분부터 붙여가는 셀 (왼쪽)
	CellForBeamTableform	cellsFromEndAtRSide [4][50];	// 끝 부분부터 붙여가는 셀 (오른쪽)
	CellForBeamTableform	cellCenterAtLSide [4];			// 가운데 부분에 붙이는 셀 (왼쪽)
	CellForBeamTableform	cellCenterAtRSide [4];			// 가운데 부분에 붙이는 셀 (오른쪽)
	short			nCellsFromBeginAtSide;			// 측면의 시작 부분 셀 개수
	short			nCellsFromEndAtSide;			// 측면의 끝 부분 셀 개수

	// 셀 정보 (하부)
	CellForBeamTableform	cellsFromBeginAtBottom [3][50];	// 시작 부분부터 붙여가는 셀
	CellForBeamTableform	cellsFromEndAtBottom [3][50];	// 끝 부분부터 붙여가는 셀
	CellForBeamTableform	cellCenterAtBottom [3];			// 가운데 부분에 붙이는 셀
	short			nCellsFromBeginAtBottom;		// 하부의 시작 부분 셀 개수
	short			nCellsFromEndAtBottom;			// 하부의 끝 부분 셀 개수

	double			gapSide;			// 보와의 간격 (측면)
	double			gapBottom;			// 보와의 간격 (하부)

public:
	void		initCells (BeamTableformPlacingZone* placingZone);						// Cell 배열을 초기화함
	void		firstPlacingSettings (BeamTableformPlacingZone* placingZone);			// 1차 배치 설정
	void		addNewColFromBeginAtSide (BeamTableformPlacingZone* target_zone);		// 측면 시작 부분 - 새로운 열을 추가함 (열 하나를 늘리고 추가된 열에 마지막 열 정보 복사)
	void		delLastColFromBeginAtSide (BeamTableformPlacingZone* target_zone);		// 측면 시작 부분 - 마지막 열을 삭제함
	void		addNewColFromEndAtSide (BeamTableformPlacingZone* target_zone);			// 측면 끝 부분 - 새로운 열을 추가함 (열 하나를 늘리고 추가된 열에 마지막 열 정보 복사)
	void		delLastColFromEndAtSide (BeamTableformPlacingZone* target_zone);		// 측면 끝 부분 - 마지막 열을 삭제함
	void		addNewColFromBeginAtBottom (BeamTableformPlacingZone* target_zone);		// 하부 시작 부분 - 새로운 열을 추가함 (열 하나를 늘리고 추가된 열에 마지막 열 정보 복사)
	void		delLastColFromBeginAtBottom (BeamTableformPlacingZone* target_zone);	// 하부 시작 부분 - 마지막 열을 삭제함
	void		addNewColFromEndAtBottom (BeamTableformPlacingZone* target_zone);		// 하부 끝 부분 - 새로운 열을 추가함 (열 하나를 늘리고 추가된 열에 마지막 열 정보 복사)
	void		delLastColFromEndAtBottom (BeamTableformPlacingZone* target_zone);		// 하부 끝 부분 - 마지막 열을 삭제함
	void		alignPlacingZone (BeamTableformPlacingZone* placingZone);				// Cell 정보가 변경됨에 따라 파편화된 위치를 재조정함
	API_Guid	placeLibPart (CellForBeamTableform objInfo);							// 해당 셀 정보를 기반으로 라이브러리 배치
	API_Guid	placeLibPart (EuroformHook params);										// 라이브러리 배치: 유로폼 후크
	API_Guid	placeLibPart (RectPipeHanger params);									// 라이브러리 배치: 각파이프행거
	API_Guid	placeLibPart (SquarePipe params);										// 라이브러리 배치: 비계파이프
	API_Guid	placeLibPart (BlueTimberRail params);									// 라이브러리 배치: 블루목심
	GSErrCode	fillRestAreas (BeamTableformPlacingZone* placingZone);					// 유로폼/휠러/목재를 채운 후 자투리 공간 채우기 (나머지 합판/목재 및 아웃코너앵글)
};

// 테이블폼폼 보 배치 함수
GSErrCode	placeTableformOnBeam (void);		// 보에 테이블폼을 배치하는 통합 루틴
short DGCALLBACK beamTableformPlacerHandler1 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 1차 배치를 위한 질의를 요청하는 1차 다이얼로그
short DGCALLBACK beamTableformPlacerHandler2 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 1차 배치 후 수정을 요청하는 2차 다이얼로그
short DGCALLBACK beamTableformPlacerHandler3 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 2차 다이얼로그에서 각 셀의 객체 타입을 변경하기 위한 3차 다이얼로그

#endif
