#ifndef	__BEAM_TABLEFORM_PLACER__
#define __BEAM_TABLEFORM_PLACER__

#include "MaxBIM.hpp"

namespace beamTableformPlacerDG {
	// ��ü Ÿ��
	enum	libPartObjType {
		NONE,			// ����
		EUROFORM,		// ������v2.0
		PLYWOOD,		// ����v1.0
		TIMBER,			// ����v1.0
		FILLERSP		// �ٷ������̼�v1.0
	};

	// ���̾�α� �׸� �ε���
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

// �ܿ���
struct insulElemForBeamTableform
{
	short	layerInd;		// ���̾� �ε���
	double	thk;			// �β�
	bool	bLimitSize;		// ����/���� ũ�� ����
	double	maxHorLen;		// ���� �ִ� ����
	double	maxVerLen;		// ���� �ִ� ����
};

// ���� ���� ����
struct InfoMorphForBeamTableform
{
	API_Guid	guid;		// ������ GUID
	short		floorInd;	// �� �ε���
	double		level;		// ������ ��

	double	ang;			// ȸ�� ���� (����: Radian, ȸ����: Z��)

	double	leftBottomX;	// ���ϴ� ��ǥ X
	double	leftBottomY;	// ���ϴ� ��ǥ Y
	double	leftBottomZ;	// ���ϴ� ��ǥ Z

	double	rightTopX;		// ���� ��ǥ X
	double	rightTopY;		// ���� ��ǥ Y
	double	rightTopZ;		// ���� ��ǥ Z
};

// �׸��� �� �� ����
struct CellForBeamTableform
{
	short		objType;	// enum libPartObjType ����

	API_Guid	guid;		// ��ü�� GUID

	double	leftBottomX;	// ���ϴ� ��ǥ X
	double	leftBottomY;	// ���ϴ� ��ǥ Y
	double	leftBottomZ;	// ���ϴ� ��ǥ Z
	double	ang;			// ȸ�� ���� (����: Radian, ȸ����: Z��)

	double	dirLen;			// �� ��ġ���� ����
	double	perLen;			// �� �������� ����
};

// �� ���� ����
class BeamTableformPlacingZone
{
public:
	short	tableformType;		// ���̺��� Ÿ��: Ÿ��A (1), Ÿ��B (2)

	// �� ���� ����
	double	level;				// �� ���� ��
	double	ang;				// �� ȸ�� ���� (����: Radian, ȸ����: Z��)
	double	areaHeight_Left;	// ���� ���� (����)	: �׻� ���� ��
	double	areaHeight_Right;	// ���� ���� (����) : �׻� ���� ��
	double	areaWidth_Bottom;	// ���� �ʺ� (�Ϻ�)

	API_Coord3D		begC;		// ��ġ ���� ������
	API_Coord3D		endC;		// ��ġ ���� ����

	double	beamLength;			// �� ����
	double	offset;				// �� ������
	double	slantAngle;			// ������ ���� (��� ��)

	// ������ ����
	double	gapSide;			// ������ ���� (����)
	double	gapBottom;			// ������ ���� (�Ϻ�)

	// ���� ������ ���� ����
	double	hRest_Left;
	double	hRest_Right;

	// �� �糡 ����
	double	marginBegin;			// ���� �κ� ����
	double	marginEnd;				// �� �κ� ����

	// �� �糡 ���� ä���� ����
	bool	bFillMarginBegin;		// ���� �κ� ���� ä��
	bool	bFillMarginEnd;			// �� �κ� ���� ä��

	// �� �糡 ��
	CellForBeamTableform	beginCellAtLSide;
	CellForBeamTableform	beginCellAtRSide;
	CellForBeamTableform	beginCellAtBottom;
	CellForBeamTableform	endCellAtLSide;
	CellForBeamTableform	endCellAtRSide;
	CellForBeamTableform	endCellAtBottom;

	// �� ����
	CellForBeamTableform	cellsAtLSide [4][50];	// ���� ���� ��
	CellForBeamTableform	cellsAtRSide [4][50];	// ������ ���� ��
	CellForBeamTableform	cellsAtBottom [3][50];	// �Ϻ� ��
	short					nCells;					// �� ����

	// ���ٸ�/�ۿ��� ������ ����
	short	typeOfSupportingPost;	// Ÿ��
	short	numOfSupportingPostSet;	// ���ٸ� ��Ʈ ����
	double	postStartOffset;		// ���� ��ġ
	double	postGapWidth;			// �ʺ�
	double	postGapLength;			// ����

public:
	void		initCells (BeamTableformPlacingZone* placingZone);											// Cell �迭�� �ʱ�ȭ��
	double		getCellPositionLeftBottomX (BeamTableformPlacingZone* placingZone, short idx);				// ��(0-��� �ε��� ��ȣ)�� ���ϴ� �� ��ġ X ��ǥ�� ����
	void		alignPlacingZone (BeamTableformPlacingZone* placingZone);									// Cell ������ ����ʿ� ���� ����ȭ�� ��ġ�� ��������
	GSErrCode	placeBasicObjects (BeamTableformPlacingZone* placingZone);									// ������/�ٷ�/����/���縦 ��ġ��
	GSErrCode	placeAuxObjectsA (BeamTableformPlacingZone* placingZone);									// ������/�ٷ�/����/���縦 ä�� �� ������ ��ġ (�ƿ��ڳʾޱ�, ���������, �ɺ�Ʈ, �����������, ���Ŭ����, �����) - Ÿ��A
	GSErrCode	placeAuxObjectsB (BeamTableformPlacingZone* placingZone);									// ������/�ٷ�/����/���縦 ä�� �� ������ ��ġ (�ƿ��ڳʾޱ�, ���������, �ɺ�Ʈ, �����������, ���Ŭ����, �����) - Ÿ��B
	GSErrCode	placeInsulationsSide (BeamTableformPlacingZone* placingZone, InfoBeam* infoBeam, insulElemForBeamTableform* insulElem, bool bMoreWider);		// ���� ���̺��� ���̿� �ܿ��縦 ��ġ�� (�� ����)
	GSErrCode	placeInsulationsBottom (BeamTableformPlacingZone* placingZone, InfoBeam* infoBeam, insulElemForBeamTableform* insulElem, bool bMoreWider);		// ���� ���̺��� ���̿� �ܿ��縦 ��ġ�� (�� �Ϻ�)
	GSErrCode	placeSupportingPostPreset (BeamTableformPlacingZone* placingZone);							// ���ٸ�/�ۿ��� �������� ��ġ��
	short		getObjectType (BeamTableformPlacingZone* placingZone, bool bLeft, short idx);				// ���� Ȥ�� ������ ���� idx ��° ���� ��ġ�Ǵ� ��ü�� Ÿ���� ������
	short		getAreaSeqNumOfCell (BeamTableformPlacingZone* placingZone, bool bLeft, bool bTableform, short idx);	// idx ��° ���� �� ��° �������� ���̺��� Ȥ�� ���� �����ΰ�?

public:
	// ���̾�α� ���� ��� �ε��� ��ȣ ����
	short	CHECKBOX_MARGIN_LEFT_END;
	short	EDITCONTROL_MARGIN_LEFT_END;
	short	BUTTON_OBJ [50];
	short	POPUP_OBJ_TYPE [50];
	short	POPUP_WIDTH [50];
	short	EDITCONTROL_WIDTH [50];
	short	CHECKBOX_MARGIN_RIGHT_END;
	short	EDITCONTROL_MARGIN_RIGHT_END;
};

// ���̺����� �� ��ġ �Լ�
GSErrCode	placeTableformOnBeam (void);		// ���� ���̺����� ��ġ�ϴ� ���� ��ƾ
short DGCALLBACK beamTableformPlacerHandler1 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 1�� ��ġ�� ���� ���Ǹ� ��û�ϴ� 1�� ���̾�α�
short DGCALLBACK beamTableformPlacerHandler2 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 1�� ��ġ �� ������ ��û�ϴ� 2�� ���̾�α�
short DGCALLBACK beamTableformPlacerHandler3 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// ���ٸ�/�ۿ��� �������� ��ġ���� ���θ� ���
short DGCALLBACK beamTableformPlacerHandler4_Insulation (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// ���� ���̺��� ���̿� �ܿ��縦 ������ ���θ� ����� ���̾�α�
#endif
