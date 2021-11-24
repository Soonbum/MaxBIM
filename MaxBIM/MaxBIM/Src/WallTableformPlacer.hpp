#ifndef	__WALL_TABLEFORM_PLACER__
#define __WALL_TABLEFORM_PLACER__

#include "MaxBIM.hpp"

namespace wallTableformPlacerDG {
	// ���̾�α� �׸� �ε���
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
		NONE,		// ����
		TABLEFORM,	// ���̺���
		EUROFORM,	// ������
		FILLERSP,	// �ٷ������̼�
		PLYWOOD,	// ����
		TIMBER		// ����
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

	double	horLen;			// ���� ����
	double	verLen;			// ���� ����
	double	ang;			// ȸ�� ���� (����: Radian, ȸ����: Z��)

	short	objType;		// ��ü Ÿ��: ����, ���̺���, ������, �ٷ������̼�, ����, ����
	short	tableformType;	// ���̺��� Ÿ��: Ÿ��A (1), Ÿ��B (2), Ÿ��C (3), Ÿ��D (4)
	bool	bVertical;		// ����: ����(true), ����(false)
	
	// ���̺��� �� ������ ����
	double	tableInHor [8];		// ���� ����
	double	tableInVer [8];		// ���� ����
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

// ���� ���� ���� (����)
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

	double	remainWidth;		// ���� �ʺ�
	double	remainHeightBasic;	// ���� ���� (������)
	double	remainHeightExtra;	// ���� ���� (������)

	bool	bLincorner;			// ���� ���ڳ� ����
	bool	bRincorner;			// ������ ���ڳ� ����
	double	lenLincorner;		// ���� ���ڳ� ����
	double	lenRincorner;		// ������ ���ڳ� ����

	short	nCellsInHor;		// ���� ���� ��(������) ����
	short	nCellsInVerBasic;	// ���� ���� ��(������) ���� (������)
	short	nCellsInVerExtra;	// ���� ���� ��(������) ���� (������)

	CellForWallTableform		cells [10];			// �� �迭 (���ڳ� ����)
	UpperCellForWallTableform	upperCells [10];	// ��� ���� �� �迭

	double	marginTopBasic;		// ��� ���� (������)
	double	marginTopExtra;		// ��� ���� (������)

public:
	void	initCells (WallTableformPlacingZone* placingZone);		// �� ���� �ʱ�ȭ
};

GSErrCode	placeTableformOnWall (void);				// ���� ���̺����� ��ġ�ϴ� ���� ��ƾ - ��ü ����
short DGCALLBACK wallTableformPlacerHandler1 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);				// ���̺���/������/�ٷ������̼�/����/���� ��ġ�� ���� ���̾�α� (���̺��� ����, ��� ����, ���� �� ����)
short DGCALLBACK wallTableformPlacerHandler2 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);				// ��ü�� ���̾ �����ϱ� ���� ���̾�α�

#endif