#ifndef	__BEAM_TABLEFORM_PLACER__
#define __BEAM_TABLEFORM_PLACER__

#include "MaxBIM.hpp"

namespace beamTableformPlacerDG {
	// 객체 타입
	enum	libPartObjType {
		NONE,			// 없음
		EUROFORM,		// 유로폼v2.0
		PLYWOOD,		// 합판v1.0
		TIMBER,			// 목재v1.0
		OUTCORNER_ANGLE,// 아웃코너앵글v1.0
		FILLERSP		// 휠러스페이서v1.0
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
		LABEL_BEAM_LEFT_HEIGHT,
		EDITCONTROL_BEAM_LEFT_HEIGHT,
		LABEL_BEAM_RIGHT_HEIGHT,
		EDITCONTROL_BEAM_RIGHT_HEIGHT,
		LABEL_BEAM_WIDTH,
		EDITCONTROL_BEAM_WIDTH,
		
		SEPARATOR_2_LEFT,
		SEPARATOR_3_BOTTOM,
		SEPARATOR_4_RIGHT,
		EDITCONTROL_GAP_SIDE1,
		EDITCONTROL_GAP_BOTTOM,
		EDITCONTROL_GAP_SIDE2,

		LABEL_TOTAL_LEFT_HEIGHT,
		EDITCONTROL_TOTAL_LEFT_HEIGHT,
		LABEL_TOTAL_WIDTH,
		EDITCONTROL_TOTAL_WIDTH,
		LABEL_TOTAL_RIGHT_HEIGHT,
		EDITCONTROL_TOTAL_RIGHT_HEIGHT,

		LABEL_REST_LSIDE,
		CHECKBOX_TIMBER_LSIDE,
		CHECKBOX_T_FORM_LSIDE,
		CHECKBOX_FILLER_LSIDE,
		CHECKBOX_B_FORM_LSIDE,

		EDITCONTROL_REST_LSIDE,
		EDITCONTROL_TIMBER_LSIDE,
		POPUP_T_FORM_LSIDE,
		EDITCONTROL_FILLER_LSIDE,
		POPUP_B_FORM_LSIDE,

		EDITCONTROL_REST_RSIDE,
		EDITCONTROL_TIMBER_RSIDE,
		POPUP_T_FORM_RSIDE,
		EDITCONTROL_FILLER_RSIDE,
		POPUP_B_FORM_RSIDE,

		LABEL_REST_RSIDE,
		CHECKBOX_TIMBER_RSIDE,
		CHECKBOX_T_FORM_RSIDE,
		CHECKBOX_FILLER_RSIDE,
		CHECKBOX_B_FORM_RSIDE,

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
		LABEL_LAYER_TIMBER,
		LABEL_LAYER_OUTCORNER_ANGLE,
		LABEL_LAYER_FILLERSPACER,
		LABEL_LAYER_RECTPIPE,
		LABEL_LAYER_RECTPIPE_HANGER,
		LABEL_LAYER_PINBOLT,
		LABEL_LAYER_EUROFORM_HOOK,
		LABEL_LAYER_BLUE_CLAMP,
		LABEL_LAYER_BLUE_TIMBER_RAIL,

		USERCONTROL_LAYER_EUROFORM,
		USERCONTROL_LAYER_PLYWOOD,
		USERCONTROL_LAYER_TIMBER,
		USERCONTROL_LAYER_OUTCORNER_ANGLE,
		USERCONTROL_LAYER_FILLERSPACER,
		USERCONTROL_LAYER_RECTPIPE,
		USERCONTROL_LAYER_RECTPIPE_HANGER,
		USERCONTROL_LAYER_PINBOLT,
		USERCONTROL_LAYER_EUROFORM_HOOK,
		USERCONTROL_LAYER_BLUE_CLAMP,
		USERCONTROL_LAYER_BLUE_TIMBER_RAIL,

		BUTTON_AUTOSET
	};

	// !!!
	enum	idxItems_2_forBeamPlacer {
		DG_UPDATE_BUTTON		= 3,
		DG_PREV,
		LABEL_BEAM_SIDE_BOTTOM,
		LABEL_BEAM_SIDE,
		LABEL_BEAM_BOTTOM,
		AFTER_ALL
	};

	// !!!
	enum	idxItems_3_forBeamPlacer {
		LABEL_OBJ_TYPE			= 3,
		POPUP_OBJ_TYPE,
		CHECKBOX_SET_STANDARD,
		LABEL_LENGTH,
		EDITCONTROL_LENGTH,
		POPUP_LENGTH
	};
}

// 모프 관련 정보
struct InfoMorphForBeamTableform
{
	API_Guid	guid;		// 모프의 GUID
	short		floorInd;	// 층 인덱스
	double		level;		// 모프의 고도

	double	ang;			// 회전 각도 (단위: Radian, 회전축: Z축)

	double	leftBottomX;	// 좌하단 좌표 X
	double	leftBottomY;	// 좌하단 좌표 Y
	double	leftBottomZ;	// 좌하단 좌표 Z

	double	rightTopX;		// 우상단 좌표 X
	double	rightTopY;		// 우상단 좌표 Y
	double	rightTopZ;		// 우상단 좌표 Z
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
};

// 보 영역 정보
class BeamTableformPlacingZone
{
public:
	// 보 기하 정보
	double	level;				// 보 윗면 고도
	double	ang;				// 보 회전 각도 (단위: Radian, 회전축: Z축)
	double	areaHeight_Left;	// 영역 높이 (좌측)	: 항상 높은 쪽
	double	areaHeight_Right;	// 영역 높이 (우측) : 항상 낮은 쪽
	double	areaWidth_Bottom;	// 영역 너비 (하부)

	API_Coord3D		begC;		// 배치 기준 시작점
	API_Coord3D		endC;		// 배치 기준 끝점

	double	beamLength;			// 보 길이
	double	offset;				// 보 오프셋

	double	gapSide;			// 보와의 간격 (측면)
	double	gapBottom;			// 보와의 간격 (하부)

	// 보 양끝 여백
	double	marginBeginAtLSide;				// 측면 (좌측) 시작 부분 여백
	double	marginEndAtLSide;				// 측면 (좌측) 끝 부분 여백
	double	marginBeginAtRSide;				// 측면 (우측) 시작 부분 여백
	double	marginEndAtRSide;				// 측면 (우측) 끝 부분 여백
	double	marginBeginAtBottom;			// 하부 시작 부분 여백
	double	marginEndAtBottom;				// 하부 끝 부분 여백

	// 보 양끝 여백 채울지 여부
	bool	bFillMarginBeginAtLSide;		// 측면 (좌측) 시작 부분 여백 채움
	bool	bFillMarginEndAtLSide;			// 측면 (좌측) 끝 부분 여백 채움
	bool	bFillMarginBeginAtRSide;		// 측면 (우측) 시작 부분 여백 채움
	bool	bFillMarginEndAtRSide;			// 측면 (우측) 끝 부분 여백 채움
	bool	bFillMarginBeginAtBottom;		// 하부 시작 부분 여백 채움
	bool	bFillMarginEndAtBottom;			// 하부 끝 부분 여백 채움

	// 셀 정보
	CellForBeamTableform	cellsAtLSide [4][50];	// 왼쪽 측면 셀
	CellForBeamTableform	cellsAtRSide [4][50];	// 오른쪽 측면 셀
	CellForBeamTableform	cellsAtBottom [3][50];	// 하부 셀
	short					nCellsAtLSide;			// 왼쪽 측면 셀 개수
	short					nCellsAtRSide;			// 오른쪽 측면 셀 개수
	short					nCellsAtBottom;			// 하부 셀 개수

public:
	void		initCells (BeamTableformPlacingZone* placingZone);				// Cell 배열을 초기화함
	void		addNewColAtLSide (BeamTableformPlacingZone* placingZone);		// 측면 왼쪽 - 새로운 열을 추가함 (열 하나를 늘리고 추가된 열에 마지막 열 정보 복사)
	void		delLastColAtLSide (BeamTableformPlacingZone* placingZone);		// 측면 왼쪽 - 마지막 열을 삭제함
	void		addNewColAtRSide (BeamTableformPlacingZone* placingZone);		// 측면 오른쪽 - 새로운 열을 추가함 (열 하나를 늘리고 추가된 열에 마지막 열 정보 복사)
	void		delLastColAtRSide (BeamTableformPlacingZone* placingZone);		// 측면 오른쪽 - 마지막 열을 삭제함
	void		addNewColAtBottom (BeamTableformPlacingZone* placingZone);		// 하부 - 새로운 열을 추가함 (열 하나를 늘리고 추가된 열에 마지막 열 정보 복사)
	void		delLastColAtBottom (BeamTableformPlacingZone* placingZone);		// 하부 - 마지막 열을 삭제함
	void		alignPlacingZone (BeamTableformPlacingZone* placingZone);		// Cell 정보가 변경됨에 따라 파편화된 위치를 재조정함
	GSErrCode	fillRestAreas (BeamTableformPlacingZone* placingZone);			// 유로폼/휠러/각재를 채운 후 자투리 공간 채우기 (나머지 합판/각재 및 아웃코너앵글)
};

// 테이블폼폼 보 배치 함수
GSErrCode	placeTableformOnBeam (void);		// 보에 테이블폼을 배치하는 통합 루틴
short DGCALLBACK beamTableformPlacerHandler1 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 1차 배치를 위한 질의를 요청하는 1차 다이얼로그
short DGCALLBACK beamTableformPlacerHandler2 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 1차 배치 후 수정을 요청하는 2차 다이얼로그
short DGCALLBACK beamTableformPlacerHandler3 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 2차 다이얼로그에서 각 셀의 객체 타입을 변경하기 위한 3차 다이얼로그

#endif
