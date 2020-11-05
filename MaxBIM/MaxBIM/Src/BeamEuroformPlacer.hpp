#ifndef	__BEAM_EUROFORM_PLACER__
#define __BEAM_EUROFORM_PLACER__

#include "MaxBIM.hpp"

namespace beamPlacerDG {
	// ��ü Ÿ��
	enum	libPartObjType {
		NONE,			// ����
		EUROFORM,		// ������v2.0
		FILLERSPACER,	// �ٷ������̼�v1.0
		PLYWOOD,		// ����v1.0
		WOOD,			// ����v1.0
		OUTCORNER_ANGLE	// �ƿ��ڳʾޱ�v1.0
	};

	// �����Ǵ� �� ��ġ
	enum	attachedSide {
		BOTTOM_SIDE,
		LEFT_SIDE,
		RIGHT_SIDE
	};

	// ���̾�α� �׸� �ε���
	enum	idxItems_1_forBeamPlacer {
		LABEL_PLACING_EUROFORM		= 3,
		LABEL_EUROFORM_WIDTH,
		POPUP_EUROFORM_WIDTH,
		LABEL_EUROFORM_HEIGHT,
		POPUP_EUROFORM_HEIGHT,
		SEPARATOR_1,

		LABEL_GAP_LENGTH,
		EDITCONTROL_GAP_LENGTH,
		SEPARATOR_2,

		ICON_LAYER,
		LABEL_LAYER_SETTINGS,
		LABEL_LAYER_EUROFORM,
		LABEL_LAYER_FILLERSPACER,
		LABEL_LAYER_PLYWOOD,
		LABEL_LAYER_WOOD,
		LABEL_LAYER_OUTCORNER_ANGLE,

		USERCONTROL_LAYER_EUROFORM,
		USERCONTROL_LAYER_FILLERSPACER,
		USERCONTROL_LAYER_PLYWOOD,
		USERCONTROL_LAYER_WOOD,
		USERCONTROL_LAYER_OUTCORNER_ANGLE
	};
}

// �� ���� ����
struct InfoBeam
{
	API_Guid	guid;	// ���� GUID

	short	floorInd;	// �� �ε���
	double	height;		// �� ����
	double	width;		// �� �ʺ�
	double	offset;		// �� �߽����κ��� ���� ���۷��� ������ �������Դϴ�.
	double	level;		// �ٴ� ������ ���� ���� ���ʸ� �����Դϴ�.

	API_Coord	begC;	// �� ���� ��ǥ
	API_Coord	endC;	// �� �� ��ǥ
};

// ���� ���� ����
struct InfoMorphForBeam
{
	API_Guid	guid;		// ������ GUID
	short		floorInd;	// �� �ε���
	double		level;		// ������ ��
};

// �׸��� �� �� ����
struct CellForBeam
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
		Wood			wood;
		OutcornerAngle	outangle;
	} libPart;
};

// �� ���� ����
struct BeamPlacingZone
{
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

	// �� �糡 ����
	double	marginBeginAtSide;				// ���� ���� �κ� ����
	double	marginEndAtSide;				// ���� �� �κ� ����
	double	marginBeginAtBottom;			// �Ϻ� ���� �κ� ����
	double	marginEndAtBottom;				// �Ϻ� �� �κ� ����
	double	marginBeginAtSide_updated;		// ���� ���� �κ� ���� (������Ʈ ��)
	double	marginEndAtSide_updated;		// ���� �� �κ� ���� (������Ʈ ��)
	double	marginBeginAtBottom_updated;	// �Ϻ� ���� �κ� ���� (������Ʈ ��)
	double	marginEndAtBottom_updated;		// �Ϻ� �� �κ� ���� (������Ʈ ��)

	// �� ���� (����)
	//	!!!
	CellForBeam		cellsFromBeginAtLSide [20];		// ���� �κк��� �ٿ����� �� (����)
	CellForBeam		cellsFromBeginAtRSide [20];		// ���� �κк��� �ٿ����� �� (������)
	CellForBeam		cellsFromEndAtLSide [20];		// �� �κк��� �ٿ����� �� (����)
	CellForBeam		cellsFromEndAtRSide [20];		// �� �κк��� �ٿ����� �� (������)
	CellForBeam		cellCenterAtLSide;				// ��� �κп� ���̴� �� (����)
	CellForBeam		cellCenterAtRSide;				// ��� �κп� ���̴� �� (������)
	short			nCellsFromBeginAtSide;			// ������ ���� �κ� �� ����
	short			nCellsFromEndAtSide;			// ������ �� �κ� �� ����

	// �� ���� (�Ϻ�)
	//	!!!
	CellForBeam		cellsFromBeginAtBottom [20];	// ���� �κк��� �ٿ����� ��
	CellForBeam		cellsFromEndAtBottom [20];		// �� �κк��� �ٿ����� ��
	CellForBeam		cellCenterAtBottom;				// ��� �κп� ���̴� ��
	short			nCellsFromBeginAtBottom;		// �Ϻ��� ���� �κ� �� ����
	short			nCellsFromEndAtBottom;			// �Ϻ��� �� �κ� �� ����

	double	gap;		// ������ ����

	// ������ �ʺ� �ڵ� ���
	//	!!!
	double	eu_wid_numeric_side;		// ������ �ʺ� (�Ǽ���) - ����
	double	eu_wid_numeric_bottom;		// ������ �ʺ� (�Ǽ���) - �Ϻ�
};

// ������ �� ��ġ �Լ�
GSErrCode	placeEuroformOnBeam (void);				// 3�� �޴�: ���� �������� ��ġ�ϴ� ���� ��ƾ
void	initCellsForBeam (BeamPlacingZone* placingZone);										// Cell �迭�� �ʱ�ȭ��
void	firstPlacingSettingsForBeam (BeamPlacingZone* placingZone);								// 1�� ��ġ
void		adjustOtherCellsInSameRow (BeamPlacingZone* target_zone, short row, short col);		// �ش� ���� ������ �࿡ �ִ� �ٸ� ������ Ÿ�� �� ���̸� ������
void		addNewColFromBeginAtSide (BeamPlacingZone* target_zone);							// ���� ���� �κ� - ���ο� ���� �߰��� (�� �ϳ��� �ø��� �߰��� ���� ������ �� ���� ����)
void		delLastColFromBeginAtSide (BeamPlacingZone* target_zone);							// ���� ���� �κ� - ������ ���� ������
void		addNewColFromEndAtSide (BeamPlacingZone* target_zone);								// ���� �� �κ� - ���ο� ���� �߰��� (�� �ϳ��� �ø��� �߰��� ���� ������ �� ���� ����)
void		delLastColFromEndAtSide (BeamPlacingZone* target_zone);								// ���� �� �κ� - ������ ���� ������
void		addNewColFromBeginAtBottom (BeamPlacingZone* target_zone);							// �Ϻ� ���� �κ� - ���ο� ���� �߰��� (�� �ϳ��� �ø��� �߰��� ���� ������ �� ���� ����)
void		delLastColFromBeginAtBottom (BeamPlacingZone* target_zone);							// �Ϻ� ���� �κ� - ������ ���� ������
void		addNewColFromEndAtBottom (BeamPlacingZone* target_zone);							// �Ϻ� �� �κ� - ���ο� ���� �߰��� (�� �ϳ��� �ø��� �߰��� ���� ������ �� ���� ����)
void		delLastColFromEndAtBottom (BeamPlacingZone* target_zone);							// �Ϻ� �� �κ� - ������ ���� ������
void		alignPlacingZoneForBeam (BeamPlacingZone* target_zone);								// Cell ������ ����ʿ� ���� ����ȭ�� ��ġ�� ��������
API_Guid	placeLibPartForBeam (CellForBeam objInfo);											// �ش� �� ������ ������� ���̺귯�� ��ġ
GSErrCode	fillRestAreasForBeam (void);														// �������� ä�� �� ������ ���� ä���
short DGCALLBACK beamPlacerHandler1 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 1�� ��ġ�� ���� ���Ǹ� ��û�ϴ� 1�� ���̾�α�
short DGCALLBACK beamPlacerHandler2 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 1�� ��ġ �� ������ ��û�ϴ� 2�� ���̾�α�
short DGCALLBACK beamPlacerHandler3 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 2�� ���̾�α׿��� �� ���� ��ü Ÿ���� �����ϱ� ���� 3�� ���̾�α�

#endif
