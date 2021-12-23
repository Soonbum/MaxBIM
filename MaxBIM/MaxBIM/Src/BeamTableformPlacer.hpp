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
		OUTCORNER_ANGLE,// �ƿ��ڳʾޱ�v1.0
		FILLERSP		// �ٷ������̼�v1.0
	};

	// �����Ǵ� �� ��ġ
	enum	attachedSide {
		BOTTOM_SIDE,
		LEFT_SIDE,
		RIGHT_SIDE
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
	short	attached_side;	// �Ϻ�(BOTTOM_SIDE), ������(LEFT_SIDE), ������(RIGHT_SIDE)
};

// �� ���� ����
class BeamTableformPlacingZone
{
public:
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

	double	gapSide;			// ������ ���� (����)
	double	gapBottom;			// ������ ���� (�Ϻ�)

	// �� �糡 ����
	double	marginBeginAtLSide;				// ���� (����) ���� �κ� ����
	double	marginEndAtLSide;				// ���� (����) �� �κ� ����
	double	marginBeginAtRSide;				// ���� (����) ���� �κ� ����
	double	marginEndAtRSide;				// ���� (����) �� �κ� ����
	double	marginBeginAtBottom;			// �Ϻ� ���� �κ� ����
	double	marginEndAtBottom;				// �Ϻ� �� �κ� ����

	// �� �糡 ���� ä���� ����
	bool	bFillMarginBeginAtLSide;		// ���� (����) ���� �κ� ���� ä��
	bool	bFillMarginEndAtLSide;			// ���� (����) �� �κ� ���� ä��
	bool	bFillMarginBeginAtRSide;		// ���� (����) ���� �κ� ���� ä��
	bool	bFillMarginEndAtRSide;			// ���� (����) �� �κ� ���� ä��
	bool	bFillMarginBeginAtBottom;		// �Ϻ� ���� �κ� ���� ä��
	bool	bFillMarginEndAtBottom;			// �Ϻ� �� �κ� ���� ä��

	// �� ����
	CellForBeamTableform	cellsAtLSide [4][50];	// ���� ���� ��
	CellForBeamTableform	cellsAtRSide [4][50];	// ������ ���� ��
	CellForBeamTableform	cellsAtBottom [3][50];	// �Ϻ� ��
	short					nCellsAtLSide;			// ���� ���� �� ����
	short					nCellsAtRSide;			// ������ ���� �� ����
	short					nCellsAtBottom;			// �Ϻ� �� ����

public:
	void		initCells (BeamTableformPlacingZone* placingZone);				// Cell �迭�� �ʱ�ȭ��
	void		addNewColAtLSide (BeamTableformPlacingZone* placingZone);		// ���� ���� - ���ο� ���� �߰��� (�� �ϳ��� �ø��� �߰��� ���� ������ �� ���� ����)
	void		delLastColAtLSide (BeamTableformPlacingZone* placingZone);		// ���� ���� - ������ ���� ������
	void		addNewColAtRSide (BeamTableformPlacingZone* placingZone);		// ���� ������ - ���ο� ���� �߰��� (�� �ϳ��� �ø��� �߰��� ���� ������ �� ���� ����)
	void		delLastColAtRSide (BeamTableformPlacingZone* placingZone);		// ���� ������ - ������ ���� ������
	void		addNewColAtBottom (BeamTableformPlacingZone* placingZone);		// �Ϻ� - ���ο� ���� �߰��� (�� �ϳ��� �ø��� �߰��� ���� ������ �� ���� ����)
	void		delLastColAtBottom (BeamTableformPlacingZone* placingZone);		// �Ϻ� - ������ ���� ������
	void		alignPlacingZone (BeamTableformPlacingZone* placingZone);		// Cell ������ ����ʿ� ���� ����ȭ�� ��ġ�� ��������
	GSErrCode	fillRestAreas (BeamTableformPlacingZone* placingZone);			// ������/�ٷ�/���縦 ä�� �� ������ ���� ä��� (������ ����/���� �� �ƿ��ڳʾޱ�)
};

// ���̺����� �� ��ġ �Լ�
GSErrCode	placeTableformOnBeam (void);		// ���� ���̺����� ��ġ�ϴ� ���� ��ƾ
short DGCALLBACK beamTableformPlacerHandler1 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 1�� ��ġ�� ���� ���Ǹ� ��û�ϴ� 1�� ���̾�α�
short DGCALLBACK beamTableformPlacerHandler2 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 1�� ��ġ �� ������ ��û�ϴ� 2�� ���̾�α�
short DGCALLBACK beamTableformPlacerHandler3 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 2�� ���̾�α׿��� �� ���� ��ü Ÿ���� �����ϱ� ���� 3�� ���̾�α�

#endif
