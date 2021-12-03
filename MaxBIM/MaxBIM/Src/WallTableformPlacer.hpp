#ifndef	__WALL_TABLEFORM_PLACER__
#define __WALL_TABLEFORM_PLACER__

#include "MaxBIM.hpp"

namespace wallTableformPlacerDG {
	// ���̾�α� �׸� �ε���
	enum	idxItems_1_forLayerSelection {
		ICON_LAYER_CUSTOM = 3,
		LABEL_LAYER_SETTINGS,
		CHECKBOX_LAYER_COUPLING,

		LABEL_LAYER_SLABTABLEFORM,
		LABEL_LAYER_PROFILE,
		LABEL_LAYER_EUROFORM,
		LABEL_LAYER_RECTPIPE,
		LABEL_LAYER_PINBOLT,
		LABEL_LAYER_WALLTIE,
		LABEL_LAYER_JOIN,
		LABEL_LAYER_HEADPIECE,
		LABEL_LAYER_STEELFORM,
		LABEL_LAYER_PLYWOOD,
		LABEL_LAYER_FILLERSP,
		LABEL_LAYER_OUTCORNER_ANGLE,
		LABEL_LAYER_OUTCORNER_PANEL,
		LABEL_LAYER_INCORNER_PANEL,
		LABEL_LAYER_RECTPIPE_HANGER,
		LABEL_LAYER_EUROFORM_HOOK,
		LABEL_LAYER_HIDDEN,

		USERCONTROL_LAYER_SLABTABLEFORM,
		USERCONTROL_LAYER_PROFILE,
		USERCONTROL_LAYER_EUROFORM,
		USERCONTROL_LAYER_RECTPIPE,
		USERCONTROL_LAYER_PINBOLT,
		USERCONTROL_LAYER_WALLTIE,
		USERCONTROL_LAYER_JOIN,
		USERCONTROL_LAYER_HEADPIECE,
		USERCONTROL_LAYER_STEELFORM,
		USERCONTROL_LAYER_PLYWOOD,
		USERCONTROL_LAYER_FILLERSP,
		USERCONTROL_LAYER_OUTCORNER_ANGLE,
		USERCONTROL_LAYER_OUTCORNER_PANEL,
		USERCONTROL_LAYER_INCORNER_PANEL,
		USERCONTROL_LAYER_RECTPIPE_HANGER,
		USERCONTROL_LAYER_EUROFORM_HOOK,
		USERCONTROL_LAYER_HIDDEN,

		BUTTON_AUTOSET
	};

	enum	objType_forWallTableformPlacer {
		NONE = 1,	// ����
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
	double	ang;			// ȸ�� ���� (����: Radian, ȸ����: Z��)

	short	objType;		// ��ü Ÿ��: ����, ���̺���, ������, �ٷ������̼�, ����, ����
	
	int		horLen;			// ���� ����
	int		verLenBasic;	// ���� ���� (������)
	int		verLenExtra;	// ���� ���� (������)

	// ���̺��� �� ������ ����
	int		tableInHor [10];		// ���� ����
	int		tableInVerBasic [10];	// ���� ���� (������)
	int		tableInVerExtra [10];	// ���� ���� (������)
};

// �׸��� �� ��/�ϴ� �� ����
struct MarginCellForWallTableform
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
	double	verLenBasic;	// ���� ���� (������)
	double	verLenExtra;	// ���� ���� (������)
	double	ang;			// ȸ�� ���� (����: Radian, ȸ����: Z��)

	bool	bVertical;		// ����: ���ι���(true), ���ι���(false)

	double	gap;			// ������ ����

	bool	bExtra;			// ������ ������ �ִ°�?

	bool	bLincorner;			// ���� ���ڳ� ����
	bool	bRincorner;			// ������ ���ڳ� ����
	double	lenLincorner;		// ���� ���ڳ� ����
	double	lenRincorner;		// ������ ���ڳ� ����

	short	tableformType;		// ���̺��� Ÿ��: Ÿ��A (1), Ÿ��B (2), Ÿ��C (3), Ÿ��D (4)

	short	nCellsInHor;		// ���� ���� ��(������) ����
	short	nCellsInVerBasic;	// ���� ���� ��(������) ���� (������)
	short	nCellsInVerExtra;	// ���� ���� ��(������) ���� (������)

	CellForWallTableform		cells [50];			// �� �迭 (���ڳ� ����)

	double	marginTopBasic;		// ��� ���� (������)
	double	marginTopExtra;		// ��� ���� (������)

	int	presetWidth_tableform [40];		// ���� ���� ���̺����� �ʺ� ���� (2300 ... 200)
	int	presetHeight_tableform [16];	// ���� ���� ���̺����� ���� ���� (6000 ... 1500)

	int	presetWidth_euroform [7];		// ���� ���� �������� �ʺ� ���� (600 ... 200, 0)
	int	presetHeight_euroform [4];		// ���� ���� �������� ���� ���� (1200 ... 600, 0)

	int	presetWidth_config_vertical [40][5];	// ���� ���� ���̺��� �� �������� �迭 ����
	int	presetHeight_config_vertical [16][6];	// ���� ���� ���̺��� �� �������� �迭 ����
	int	presetWidth_config_horizontal [16][6];	// ���� ���� ���̺��� �� �������� �迭 ����
	int	presetHeight_config_horizontal [40][5];	// ���� ���� ���̺��� �� �������� �迭 ����

public:
	WallTableformPlacingZone ();	// �⺻ ������
	void	initCells (WallTableformPlacingZone* placingZone, bool bVertical);				// �� ���� �ʱ�ȭ
	double	getCellPositionLeftBottomX (WallTableformPlacingZone* placingZone, short idx);	// ��(0-��� �ε��� ��ȣ)�� ���ϴ� �� ��ġ X ��ǥ�� ����
	void	adjustCellsPosition (WallTableformPlacingZone* placingZone);					// �� ��ġ�� �ٸ��� ������

public:
	// ���̾�α� ���� ��� �ε��� ��ȣ ����
	short	EDITCONTROL_GAP;
	short	POPUP_DIRECTION;
	short	POPUP_TABLEFORM_TYPE;
	short	EDITCONTROL_REMAIN_WIDTH;
	short	EDITCONTROL_REMAIN_HEIGHT_BASIC;
	short	EDITCONTROL_REMAIN_HEIGHT_EXTRA;
	short	BUTTON_ADD_HOR;
	short	BUTTON_DEL_HOR;
	short	CHECKBOX_LINCORNER;
	short	EDITCONTROL_LINCORNER;
	short	CHECKBOX_RINCORNER;
	short	EDITCONTROL_RINCORNER;
	short	BUTTON_ADD_VER_BASIC;
	short	BUTTON_DEL_VER_BASIC;
	short	BUTTON_ADD_VER_EXTRA;
	short	BUTTON_DEL_VER_EXTRA;

	short	BUTTON_OBJ [50];
	short	POPUP_OBJ_TYPE [50];
	short	POPUP_WIDTH [50];
	short	EDITCONTROL_WIDTH [50];
	short	POPUP_HEIGHT_PRESET;
	short	POPUP_HEIGHT_BASIC [10];
	short	POPUP_HEIGHT_EXTRA [10];

	short	LABEL_TOTAL_WIDTH;
	short	POPUP_WIDTH_IN_TABLE [4];
};

GSErrCode	placeTableformOnWall (void);				// ���� ���̺����� ��ġ�ϴ� ���� ��ƾ - ��ü ����
short DGCALLBACK wallTableformPlacerHandler1 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);				// ���̺���/������/�ٷ������̼�/����/���� ��ġ�� ���� ���̾�α� (���̺��� ����, ��� ����, ���� �� ����)
short DGCALLBACK wallTableformPlacerHandler2 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);				// ��ü�� ���̾ �����ϱ� ���� ���̾�α�
short DGCALLBACK wallTableformPlacerHandler3_Vertical (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// ���̺��� ���ι��⿡ ���Ͽ� �������� ���� �迭�� �����ϱ� ���� ���̾�α�
short DGCALLBACK wallTableformPlacerHandler3_Horizontal (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// ���̺��� ���ι��⿡ ���Ͽ� �������� ���� �迭�� �����ϱ� ���� ���̾�α�

#endif