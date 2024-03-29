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
		FILLERSP		// 휠러스페이서v1.0
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

		BUTTON_COPY_TO_RIGHT,

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

	enum	idxItems_2_forBeamPlacer {
		DG_PREV = 3,
		LABEL_BEAM_SIDE,
		LABEL_TOTAL_LENGTH,
		EDITCONTROL_TOTAL_LENGTH,
		LABEL_REMAIN_LENGTH,
		EDITCONTROL_REMAIN_LENGTH,
		LABEL_TABLEFORM_TYPE,
		POPUP_TABLEFORM_TYPE,
		BUTTON_ADD_COL,
		BUTTON_DEL_COL,

		AFTER_ALL
	};

	enum	idxItems_3_forBeamPlacer {
		DG_PREV_PERI = 3,

		LABEL_TYPE_SUPPORTING_POST_PERI,
		POPUP_TYPE_SUPPORTING_POST_PERI,

		LABEL_NUM_OF_POST_SET_PERI,
		POPUP_NUM_OF_POST_SET_PERI,

		LABEL_PLAN_VIEW_PERI,

		SEPARATOR_BEAM_PERI,
		SEPARATOR_SUPPORTING_POST1_PERI,
		SEPARATOR_SUPPORTING_POST2_PERI,

		LABEL_BEAM_WIDTH_PERI,
		EDITCONTROL_BEAM_WIDTH_PERI,
		LABEL_BEAM_LENGTH_PERI,
		EDITCONTROL_BEAM_LENGTH_PERI,

		LABEL_OFFSET_1_PERI,
		EDITCONTROL_OFFSET_1_PERI,
		LABEL_GAP_WIDTH_DIRECTION_1_PERI,
		POPUP_GAP_WIDTH_DIRECTION_1_PERI,
		LABEL_GAP_LENGTH_DIRECTION_1_PERI,
		POPUP_GAP_LENGTH_DIRECTION_1_PERI,

		LABEL_OFFSET_2_PERI,
		EDITCONTROL_OFFSET_2_PERI,
		LABEL_GAP_WIDTH_DIRECTION_2_PERI,
		POPUP_GAP_WIDTH_DIRECTION_2_PERI,
		LABEL_GAP_LENGTH_DIRECTION_2_PERI,
		POPUP_GAP_LENGTH_DIRECTION_2_PERI,

		ICON_LAYER_PERI,
		LABEL_LAYER_SETTINGS_PERI,
		CHECKBOX_LAYER_COUPLING_PERI,
		LABEL_LAYER_VERTICAL_POST_PERI,
		LABEL_LAYER_HORIZONTAL_POST_PERI,
		LABEL_LAYER_GIRDER_PERI,
		LABEL_LAYER_BEAM_BRACKET_PERI,
		LABEL_LAYER_YOKE_PERI,
		LABEL_LAYER_TIMBER_PERI,
		LABEL_LAYER_JACK_SUPPORT_PERI,

		USERCONTROL_LAYER_VERTICAL_POST_PERI,
		USERCONTROL_LAYER_HORIZONTAL_POST_PERI,
		USERCONTROL_LAYER_GIRDER_PERI,
		USERCONTROL_LAYER_BEAM_BRACKET_PERI,
		USERCONTROL_LAYER_YOKE_PERI,
		USERCONTROL_LAYER_TIMBER_PERI,
		USERCONTROL_LAYER_JACK_SUPPORT_PERI,

		BUTTON_AUTOSET_PERI
	};

	enum insulationDialog {
		LABEL_EXPLANATION_INS = 3,
		USERCONTROL_INSULATION_LAYER,
		LABEL_INSULATION_THK,
		EDITCONTROL_INSULATION_THK,
		CHECKBOX_INS_LIMIT_SIZE,
		LABEL_INS_HORLEN,
		EDITCONTROL_INS_HORLEN,
		LABEL_INS_VERLEN,
		EDITCONTROL_INS_VERLEN,
	};
}

// 단열재
struct insulElemForBeamTableform
{
	short	layerInd;		// 레이어 인덱스
	double	thk;			// 두께
	bool	bLimitSize;		// 가로/세로 크기 제한
	double	maxHorLen;		// 가로 최대 길이
	double	maxVerLen;		// 세로 최대 길이
};

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
};

// 보 영역 정보
class BeamTableformPlacingZone
{
public:
	short	tableformType;		// 테이블폼 타입: 타입A (1), 타입B (2)

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
	double	slantAngle;			// 기울어진 각도 (경사 보)

	// 보와의 간격
	double	gapSideLeft;		// 보와의 간격 (좌측면)
	double	gapSideRight;		// 보와의 간격 (우측면)
	double	gapBottom;			// 보와의 간격 (하부)

	// 측면 나머지 여백 길이
	double	hRest_Left;
	double	hRest_Right;

	// 보 양끝 여백
	double	marginBegin;			// 시작 부분 여백
	double	marginEnd;				// 끝 부분 여백

	// 보 양끝 여백 채울지 여부
	bool	bFillMarginBegin;		// 시작 부분 여백 채움
	bool	bFillMarginEnd;			// 끝 부분 여백 채움

	// 보 측면/하부 요소 여부
	bool	bFillBottomFormAtLSide;	// 좌측 하부 유로폼
	bool	bFillFillerAtLSide;		// 좌측 휠러스페이서
	bool	bFillTopFormAtLSide;	// 좌측 상부 유로폼
	bool	bFillWoodAtLSide;		// 좌측 합판/각재

	bool	bFillBottomFormAtRSide;	// 우측 하부 유로폼
	bool	bFillFillerAtRSide;		// 우측 휠러스페이서
	bool	bFillTopFormAtRSide;	// 우측 상부 유로폼
	bool	bFillWoodAtRSide;		// 우측 합판/각재

	bool	bFillLeftFormAtBottom;	// 하부 좌측 유로폼
	bool	bFillFillerAtBottom;	// 하부 휠러스페이서
	bool	bFillRightFormAtBottom;	// 하부 우측 유로폼

	// 보 양끝 셀
	CellForBeamTableform	beginCellAtLSide;
	CellForBeamTableform	beginCellAtRSide;
	CellForBeamTableform	beginCellAtBottom;
	CellForBeamTableform	endCellAtLSide;
	CellForBeamTableform	endCellAtRSide;
	CellForBeamTableform	endCellAtBottom;

	// 셀 정보
	CellForBeamTableform	cellsAtLSide [4][50];	// 왼쪽 측면 셀
	CellForBeamTableform	cellsAtRSide [4][50];	// 오른쪽 측면 셀
	CellForBeamTableform	cellsAtBottom [3][50];	// 하부 셀
	short					nCells;					// 셀 개수

	// 동바리/멍에제 프리셋 정보
	short	typeOfSupportingPost;	// 타입
	short	numOfSupportingPostSet;	// 동바리 세트 개수
	double	postStartOffset;		// 시작 위치
	double	postGapWidth;			// 너비
	double	postGapLength;			// 길이

public:
	void		initCells (BeamTableformPlacingZone* placingZone);											// Cell 배열을 초기화함
	double		getCellPositionLeftBottomX (BeamTableformPlacingZone* placingZone, short idx);				// 셀(0-기반 인덱스 번호)의 좌하단 점 위치 X 좌표를 구함
	void		alignPlacingZone (BeamTableformPlacingZone* placingZone);									// Cell 정보가 변경됨에 따라 파편화된 위치를 재조정함
	GSErrCode	placeBasicObjects (BeamTableformPlacingZone* placingZone);									// 유로폼/휠러/합판/각재를 배치함
	GSErrCode	placeAuxObjectsA (BeamTableformPlacingZone* placingZone);									// 유로폼/휠러/합판/각재를 채운 후 부자재 설치 (아웃코너앵글, 비계파이프, 핀볼트, 각파이프행거, 블루클램프, 블루목심) - 타입A
	GSErrCode	placeAuxObjectsB (BeamTableformPlacingZone* placingZone);									// 유로폼/휠러/합판/각재를 채운 후 부자재 설치 (아웃코너앵글, 비계파이프, 핀볼트, 각파이프행거, 블루클램프, 블루목심) - 타입B
	GSErrCode	placeInsulationsSide (BeamTableformPlacingZone* placingZone, InfoBeam* infoBeam, insulElemForBeamTableform* insulElem, bool bMoreWider);		// 벽과 테이블폼 사이에 단열재를 배치함 (보 측면)
	GSErrCode	placeInsulationsBottom (BeamTableformPlacingZone* placingZone, InfoBeam* infoBeam, insulElemForBeamTableform* insulElem, bool bMoreWider);		// 벽과 테이블폼 사이에 단열재를 배치함 (보 하부)
	GSErrCode	placeSupportingPostPreset (BeamTableformPlacingZone* placingZone);							// 동바리/멍에제 프리셋을 배치함
	short		getObjectType (BeamTableformPlacingZone* placingZone, bool bLeft, short idx);				// 왼쪽 혹은 오른쪽 면의 idx 번째 셀에 배치되는 객체의 타입을 조사함
	short		getAreaSeqNumOfCell (BeamTableformPlacingZone* placingZone, bool bLeft, bool bTableform, short idx);	// idx 번째 셀은 몇 번째 연속적인 테이블폼 혹은 합판 영역인가?

public:
	// 다이얼로그 동적 요소 인덱스 번호 저장
	short	CHECKBOX_MARGIN_LEFT_END;
	short	EDITCONTROL_MARGIN_LEFT_END;
	short	BUTTON_OBJ [50];
	short	POPUP_OBJ_TYPE [50];
	short	POPUP_WIDTH [50];
	short	EDITCONTROL_WIDTH [50];
	short	CHECKBOX_MARGIN_RIGHT_END;
	short	EDITCONTROL_MARGIN_RIGHT_END;
};

// 테이블폼폼 보 배치 함수
GSErrCode	placeTableformOnBeam (void);		// 보에 테이블폼을 배치하는 통합 루틴
short DGCALLBACK beamTableformPlacerHandler1 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 1차 배치를 위한 질의를 요청하는 1차 다이얼로그
short DGCALLBACK beamTableformPlacerHandler2 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 1차 배치 후 수정을 요청하는 2차 다이얼로그
short DGCALLBACK beamTableformPlacerHandler3 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 동바리/멍에제 프리셋을 배치할지 여부를 물어봄
short DGCALLBACK beamTableformPlacerHandler4_Insulation (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 보과 테이블폼 사이에 단열재를 넣을지 여부를 물어보는 다이얼로그
#endif
