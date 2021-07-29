#ifndef	__BEAM_TABLEFORM_PLACER__
#define __BEAM_TABLEFORM_PLACER__

#include "MaxBIM.hpp"

namespace beamTableformPlacerDG {
	// ��ü Ÿ��
	enum	libPartObjType {
		NONE,			// ����
		EUROFORM,		// ������v2.0
		PLYWOOD,		// ����v1.0
		WOOD,			// ����v1.0
		OUTCORNER_ANGLE,// �ƿ��ڳʾޱ�v1.0
		FILLERSPACER	// �ٷ������̼�v1.0
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

// ���� ���� ����
struct InfoMorphForBeamTableform
{
	API_Guid	guid;		// ������ GUID
	short		floorInd;	// �� �ε���
	double		level;		// ������ ��
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

	union {
		Euroform		form;
		Plywood			plywood;
		FillerSpacer	fillersp;
		Wood			wood;
		OutcornerAngle	outangle;
	} libPart;
};

// �� ���� ����
class BeamTableformPlacingZone
{
public:
	// ���� ���� ��ǥ�� ����, �� ��ǥ�� �������̶�� ������

	// �� ���� ����
	double	level;				// �� ���� ��
	double	ang;				// ȸ�� ���� (����: Radian, ȸ����: Z��)
	double	areaHeight;			// ���� ����

	API_Coord3D		begC;		// ��ġ ���� ������
	API_Coord3D		endC;		// ��ġ ���� ����
	double	beamLength;			// ���� �� ��ü ����

	// ���� �� ���� ����
	bool	bInterfereBeam;				// ���� �� ����
	double	posInterfereBeamFromLeft;	// ���� �� ��ġ (�� ���� ��ǥ-����-�κ���)
	double	interfereBeamWidth;			// ���� �� �ʺ�
	double	interfereBeamHeight;		// ���� �� ����

	// ���� ���� ������ ���� ����
	double	centerLengthAtSide;			// ����

	// �� �糡 ����
	double	marginBeginAtSide;				// ���� ���� �κ� ����
	double	marginEndAtSide;				// ���� �� �κ� ����
	double	marginBeginAtBottom;			// �Ϻ� ���� �κ� ����
	double	marginEndAtBottom;				// �Ϻ� �� �κ� ����

	// �� �糡 ���� ä���� ����
	bool	bFillMarginBeginAtSide;			// ���� ���� �κ� ���� ä��
	bool	bFillMarginEndAtSide;			// ���� �� �κ� ���� ä��
	bool	bFillMarginBeginAtBottom;		// �Ϻ� ���� �κ� ���� ä��
	bool	bFillMarginEndAtBottom;			// �Ϻ� �� �κ� ���� ä��

	// �� ���� (����)
	CellForBeamTableform	cellsFromBeginAtLSide [4][50];	// ���� �κк��� �ٿ����� �� (����)
	CellForBeamTableform	cellsFromBeginAtRSide [4][50];	// ���� �κк��� �ٿ����� �� (������)
	CellForBeamTableform	cellsFromEndAtLSide [4][50];	// �� �κк��� �ٿ����� �� (����)
	CellForBeamTableform	cellsFromEndAtRSide [4][50];	// �� �κк��� �ٿ����� �� (������)
	CellForBeamTableform	cellCenterAtLSide [4];			// ��� �κп� ���̴� �� (����)
	CellForBeamTableform	cellCenterAtRSide [4];			// ��� �κп� ���̴� �� (������)
	short			nCellsFromBeginAtSide;			// ������ ���� �κ� �� ����
	short			nCellsFromEndAtSide;			// ������ �� �κ� �� ����

	// �� ���� (�Ϻ�)
	CellForBeamTableform	cellsFromBeginAtBottom [3][50];	// ���� �κк��� �ٿ����� ��
	CellForBeamTableform	cellsFromEndAtBottom [3][50];	// �� �κк��� �ٿ����� ��
	CellForBeamTableform	cellCenterAtBottom [3];			// ��� �κп� ���̴� ��
	short			nCellsFromBeginAtBottom;		// �Ϻ��� ���� �κ� �� ����
	short			nCellsFromEndAtBottom;			// �Ϻ��� �� �κ� �� ����

	double			gapSide;			// ������ ���� (����)
	double			gapBottom;			// ������ ���� (�Ϻ�)

public:
	void		initCells (BeamTableformPlacingZone* placingZone);						// Cell �迭�� �ʱ�ȭ��
	void		firstPlacingSettings (BeamTableformPlacingZone* placingZone);			// 1�� ��ġ ����
	void		addNewColFromBeginAtSide (BeamTableformPlacingZone* target_zone);		// ���� ���� �κ� - ���ο� ���� �߰��� (�� �ϳ��� �ø��� �߰��� ���� ������ �� ���� ����)
	void		delLastColFromBeginAtSide (BeamTableformPlacingZone* target_zone);		// ���� ���� �κ� - ������ ���� ������
	void		addNewColFromEndAtSide (BeamTableformPlacingZone* target_zone);			// ���� �� �κ� - ���ο� ���� �߰��� (�� �ϳ��� �ø��� �߰��� ���� ������ �� ���� ����)
	void		delLastColFromEndAtSide (BeamTableformPlacingZone* target_zone);		// ���� �� �κ� - ������ ���� ������
	void		addNewColFromBeginAtBottom (BeamTableformPlacingZone* target_zone);		// �Ϻ� ���� �κ� - ���ο� ���� �߰��� (�� �ϳ��� �ø��� �߰��� ���� ������ �� ���� ����)
	void		delLastColFromBeginAtBottom (BeamTableformPlacingZone* target_zone);	// �Ϻ� ���� �κ� - ������ ���� ������
	void		addNewColFromEndAtBottom (BeamTableformPlacingZone* target_zone);		// �Ϻ� �� �κ� - ���ο� ���� �߰��� (�� �ϳ��� �ø��� �߰��� ���� ������ �� ���� ����)
	void		delLastColFromEndAtBottom (BeamTableformPlacingZone* target_zone);		// �Ϻ� �� �κ� - ������ ���� ������
	void		alignPlacingZone (BeamTableformPlacingZone* placingZone);				// Cell ������ ����ʿ� ���� ����ȭ�� ��ġ�� ��������
	API_Guid	placeLibPart (CellForBeamTableform objInfo);							// �ش� �� ������ ������� ���̺귯�� ��ġ
	API_Guid	placeLibPart (EuroformHook params);										// ���̺귯�� ��ġ: ������ ��ũ
	API_Guid	placeLibPart (RectPipeHanger params);									// ���̺귯�� ��ġ: �����������
	API_Guid	placeLibPart (SquarePipe params);										// ���̺귯�� ��ġ: ���������
	API_Guid	placeLibPart (BlueTimberRail params);									// ���̺귯�� ��ġ: �����
	GSErrCode	fillRestAreas (BeamTableformPlacingZone* placingZone);					// ������/�ٷ�/���縦 ä�� �� ������ ���� ä��� (������ ����/���� �� �ƿ��ڳʾޱ�)
};

// ���̺����� �� ��ġ �Լ�
GSErrCode	placeTableformOnBeam (void);		// ���� ���̺����� ��ġ�ϴ� ���� ��ƾ
short DGCALLBACK beamTableformPlacerHandler1 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 1�� ��ġ�� ���� ���Ǹ� ��û�ϴ� 1�� ���̾�α�
short DGCALLBACK beamTableformPlacerHandler2 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 1�� ��ġ �� ������ ��û�ϴ� 2�� ���̾�α�
short DGCALLBACK beamTableformPlacerHandler3 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 2�� ���̾�α׿��� �� ���� ��ü Ÿ���� �����ϱ� ���� 3�� ���̾�α�

#endif
