#ifndef	__WALL_TABLEFORM_PLACER__
#define __WALL_TABLEFORM_PLACER__

#include "MaxBIM.hpp"

namespace wallTableformPlacerDG {
	// ���̾�α� �׸� �ε���
	enum	idxItems_2_forWallTableformPlacer {
		DG_PREV = 3,
		LABEL_HEIGHT,
		EDITCONTROL_HEIGHT,
		LABEL_ERR_MESSAGE,
		LABEL_WIDTH,
		EDITCONTROL_WIDTH,

		LABEL_GAP_LENGTH,
		EDITCONTROL_GAP_LENGTH,
		LABEL_FILL_SIDE,
		RADIOBUTTON_DOUBLE,
		RADIOBUTTON_SINGLE,

		ICON_LAYER,
		LABEL_LAYER_SETTINGS,
		LABEL_LAYER_EUROFORM,
		LABEL_LAYER_RECTPIPE,
		LABEL_LAYER_PINBOLT,
		LABEL_LAYER_WALLTIE,
		LABEL_LAYER_JOIN,
		LABEL_LAYER_HEADPIECE,
		LABEL_LAYER_PLYWOOD,
		LABEL_LAYER_WOOD,

		USERCONTROL_LAYER_EUROFORM,
		USERCONTROL_LAYER_RECTPIPE,
		USERCONTROL_LAYER_PINBOLT,
		USERCONTROL_LAYER_WALLTIE,
		USERCONTROL_LAYER_JOIN,
		USERCONTROL_LAYER_HEADPIECE,
		USERCONTROL_LAYER_PLYWOOD,
		USERCONTROL_LAYER_WOOD
	};

	enum	idxItems_3_forWallTableformPlacer {
		LABEL_DESC1_TOPREST		= 3,
		LABEL_HEIGHT_TOPREST,
		EDITCONTROL_HEIGHT_TOPREST,
		LABEL_DESC2_TOPREST,
		LABEL_UP_TOPREST,
		LABEL_ARROWUP_TOPREST,
		LABEL_DOWN_TOPREST,
		CHECKBOX_FORM_ONOFF_1_TOPREST,
		CHECKBOX_FORM_ONOFF_2_TOPREST,
		LABEL_PLYWOOD_TOPREST,
		CHECKBOX_SET_STANDARD_1_TOPREST,
		CHECKBOX_SET_STANDARD_2_TOPREST,
		POPUP_EUROFORM_WIDTH_OPTIONS_1_TOPREST,
		POPUP_EUROFORM_WIDTH_OPTIONS_2_TOPREST,
		EDITCONTROL_EUROFORM_WIDTH_OPTIONS_1_TOPREST,
		EDITCONTROL_EUROFORM_WIDTH_OPTIONS_2_TOPREST,
		EDITCONTROL_PLYWOOD_TOPREST
	};
}

// ���� ���� ����
struct InfoMorphForWallTableform
{
	API_Guid	guid;		// ������ GUID

	double	leftBottomX;	// ���ϴ� ��ǥ X
	double	leftBottomY;	// ���ϴ� ��ǥ Y
	double	leftBottomZ;	// ���ϴ� ��ǥ Z

	double	rightTopX;		// ���� ��ǥ X
	double	rightTopY;		// ���� ��ǥ Y
	double	rightTopZ;		// ���� ��ǥ Z

	double	horLen;			// ���� ����
	double	verLen;			// ���� ����
	double	ang;			// ȸ�� ���� (����: Degree, ȸ����: Z��)
};

// �׸��� �� �� ����
struct CellForWallTableform
{
	double	leftBottomX;	// ���ϴ� ��ǥ X
	double	leftBottomY;	// ���ϴ� ��ǥ Y
	double	leftBottomZ;	// ���ϴ� ��ǥ Z

	double	horLen;			// ���� ���� (400~2300, 50 ����)
	double	verLen;			// ���� ���� (1500~6000, 300 ����)
	double	ang;			// ȸ�� ���� (����: Radian, ȸ����: Z��)
};

// �׸��� �� ��� �� ����
struct UpperCellForWallTableform
{
	double	leftBottomX;	// ���ϴ� ��ǥ X
	double	leftBottomY;	// ���ϴ� ��ǥ Y
	double	leftBottomZ;	// ���ϴ� ��ǥ Z

	double	ang;			// ȸ�� ���� (����: Radian, ȸ����: Z��)

	bool	bFill;			// ä��� ����

	bool	bEuroform1;				// 1�� ������ ����
	bool	bEuroformStandard1;		// 1�� �������� �԰������� ����
	double	formWidth1;				// 1�� �������� ��
	bool	bEuroform2;				// 2�� ������ ����
	bool	bEuroformStandard2;		// 2�� �������� �԰������� ����
	double	formWidth2;				// 2�� �������� ��
};

// ��ġ ����
struct	placementInfoForWallTableform
{
	short	nHorEuroform;	// ���� ���� ������ ����
	short	nVerEuroform;	// ���� ���� ������ ����

	double	width [7];		// ���� ���� �� ������ �ʺ�
	double	height [7];		// ���� ���� �� ������ ����
};

// ���� ���� ����
class WallTableformPlacingZone
{
public:
	double	leftBottomX;	// ���ϴ� ��ǥ X
	double	leftBottomY;	// ���ϴ� ��ǥ Y
	double	leftBottomZ;	// ���ϴ� ��ǥ Z

	double	horLen;			// ���� ����
	double	verLen;			// ���� ����
	double	ang;			// ȸ�� ���� (����: Radian, ȸ����: Z��)

	bool	bDoubleSide;	// ����̸� true, �ܸ��̸� false

	double	gap;			// ������ ����

	double	remainWidth;	// ���� ����

	CellForWallTableform		cells [50];			// ���̺��� �� ����
	UpperCellForWallTableform	upperCells [50];	// ���̺��� ����� ������ �Ǵ� ���� �� ����
	short		nCells;								// ���̺��� �� ����
	double		marginTop;							// ��� ���� ����

	// ���̺��� ���� (������ �ʺ� 400~2300�� ���̺����� �ǹ���) - ���� ����
	short	n400w;
	short	n450w;
	short	n500w;
	short	n600w;
	short	n650w;
	short	n700w;
	short	n750w;
	short	n800w;
	short	n850w;
	short	n900w;
	short	n950w;
	short	n1000w;
	short	n1050w;
	short	n1100w;
	short	n1150w;
	short	n1200w;
	short	n1250w;
	short	n1300w;
	short	n1350w;
	short	n1400w;
	short	n1450w;
	short	n1500w;
	short	n1550w;
	short	n1600w;
	short	n1650w;
	short	n1700w;
	short	n1750w;
	short	n1800w;
	short	n1850w;
	short	n1900w;
	short	n1950w;
	short	n2000w;
	short	n2050w;
	short	n2100w;
	short	n2150w;
	short	n2200w;
	short	n2250w;
	short	n2300w;

	// ���̺��� ���� (������ �ʺ� 1500~6000�� ���̺����� �ǹ���) - ���� ����
	short	n1500h;
	short	n1800h;
	short	n2100h;
	short	n2400h;
	short	n2700h;
	short	n3000h;
	short	n3300h;
	short	n3600h;
	short	n3900h;
	short	n4200h;
	short	n4500h;
	short	n4800h;
	short	n5100h;
	short	n5400h;
	short	n5700h;
	short	n6000h;

public:
	void		initCells (WallTableformPlacingZone* placingZone);													// Cell �迭�� �ʱ�ȭ��
	double		getCellPositionLeftBottomX (WallTableformPlacingZone *placingZone, short idx);						// �ش� ���� ���ϴ� ��ǥX ��ġ�� ����
	GSErrCode	placeTableformOnWall_Vertical (CellForWallTableform cell);											// ���̺��� ��ġ�ϱ� - ���� ����
	GSErrCode	placeTableformOnWall_Vertical (CellForWallTableform cell, UpperCellForWallTableform upperCell);		// ���̺��� ��� ��ġ�ϱ� - ���� ����
	GSErrCode	placeTableformOnWall_Horizontal (CellForWallTableform cell);										// ���̺��� ��ġ�ϱ� - ���� ����
	GSErrCode	placeTableformOnWall_Horizontal (CellForWallTableform cell, UpperCellForWallTableform upperCell);	// ���̺��� ��� ��ġ�ϱ� - ���� ����

	API_Guid	placeUFOM (Euroform params);					// ��ġ: ������
	API_Guid	placeUFOM_up (Euroform params);					// ��ġ: ������ (���)
	API_Guid	placeSPIP (SquarePipe params);					// ��ġ: ��� ������
	API_Guid	placePINB (PinBoltSet params);					// ��ġ: �ɺ�Ʈ ��Ʈ
	API_Guid	placeTIE  (WallTie params);						// ��ġ: ��ü Ÿ��
	API_Guid	placeCLAM (CrossClamp params);					// ��ġ: ���� Ŭ����
	API_Guid	placePUSH (HeadpieceOfPushPullProps params);	// ��ġ: ����ǽ�
	API_Guid	placeJOIN (MetalFittings params);				// ��ġ: ����ö��
	API_Guid	placePLYW (Plywood params);						// ��ġ: ����
	API_Guid	placeTIMB (Wood params);						// ��ġ: ����
};

GSErrCode	placeTableformOnWall_Vertical (void);		// ���� ���̺����� ��ġ�ϴ� ���� ��ƾ - ���� ����
short DGCALLBACK wallTableformPlacerHandler1_Vertical (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// ��ȣ�ϴ� ���̺��� �ʺ� �����ϱ� ���� ���̾�α� - ���� ����
short DGCALLBACK wallTableformPlacerHandler2_Vertical (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// ���̺��� ��ġ�� ���� ���Ǹ� ��û�ϴ� ���̾�α� - ���� ����
short DGCALLBACK wallTableformPlacerHandler3_Vertical (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// �� ����� ����/���� ������ ���������� ä���� ����� 3�� ���̾�α� - ���� ����

GSErrCode	placeTableformOnWall_Horizontal (void);		// ���� ���̺����� ��ġ�ϴ� ���� ��ƾ - ���� ����
short DGCALLBACK wallTableformPlacerHandler1_Horizontal (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// ��ȣ�ϴ� ���̺��� �ʺ� �����ϱ� ���� ���̾�α� - ���� ����
short DGCALLBACK wallTableformPlacerHandler2_Horizontal (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// ���̺��� ��ġ�� ���� ���Ǹ� ��û�ϴ� ���̾�α� - ���� ����
short DGCALLBACK wallTableformPlacerHandler3_Horizontal (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// �� ����� ����/���� ������ ���������� ä���� ����� 3�� ���̾�α� - ���� ����

#endif