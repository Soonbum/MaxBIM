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

	double	horLen;			// ���� ���� (1800~2300, 50 ����)
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

	// ���̺��� ���� (������ �ʺ� 1800~2300�� ���̺����� �ǹ���)
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

public:
	void		initCells (WallTableformPlacingZone* placingZone);											// Cell �迭�� �ʱ�ȭ��
	GSErrCode	placeTableformOnWall (CellForWallTableform cell);											// ���̺��� ��ġ�ϱ�
	GSErrCode	placeTableformOnWall (CellForWallTableform cell, UpperCellForWallTableform upperCell);		// ���̺��� ��� ��ġ�ϱ�
	double		getCellPositionLeftBottomX (WallTableformPlacingZone *placingZone, short idx);				// �ش� ���� ���ϴ� ��ǥX ��ġ�� ����

	double		moveXinParallel (double prevPosX, double ang, double offset);		// �̵� ���� X ��ǥ�� �˷��� (Z ȸ������ ���) - ���� ������ �������� �̵�
	double		moveYinParallel (double prevPosY, double ang, double offset);		// �̵� ���� Y ��ǥ�� �˷��� (Z ȸ������ ���) - ���� ������ �������� �̵�
	double		moveXinPerpend (double prevPosX, double ang, double offset);		// �̵� ���� X ��ǥ�� �˷��� (Z ȸ������ ���) - ���� ������ �������� �̵�
	double		moveYinPerpend (double prevPosY, double ang, double offset);		// �̵� ���� Y ��ǥ�� �˷��� (Z ȸ������ ���) - ���� ������ �������� �̵�
	double		moveZ (double prevPosZ, double offset);								// �̵� ���� Z ��ǥ�� �˷��� (Z ȸ������ ���)

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

// ��ġ ����
struct	placementInfoForWallTableform
{
	short	nHorEuroform;	// ���� ���� ������ ����
	short	nVerEuroform;	// ���� ���� ������ ����

	double	width [7];		// ���� ���� �� ������ �ʺ�
	double	height [7];		// ���� ���� �� ������ ����
};

GSErrCode	placeTableformOnWall (void);	// ���� ���̺����� ��ġ�ϴ� ���� ��ƾ
short DGCALLBACK wallTableformPlacerHandler1 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// ��ȣ�ϴ� ���̺��� �ʺ� �����ϱ� ���� ���̾�α�
short DGCALLBACK wallTableformPlacerHandler2 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// ���̺��� ��ġ�� ���� ���Ǹ� ��û�ϴ� ���̾�α�
short DGCALLBACK wallTableformPlacerHandler3 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// �� ����� ����/���� ������ ���������� ä���� ����� 3�� ���̾�α�

#endif