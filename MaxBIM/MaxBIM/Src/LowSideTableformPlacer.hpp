#ifndef	__LOW_SIDE_TABLEFORM_PLACER__
#define __LOW_SIDE_TABLEFORM_PLACER__

#include "MaxBIM.hpp"

namespace lowSideTableformPlacerDG {
	enum	objCellType_forLowSideTableformPlacer {
		NONE = 1,	// ����
		TABLEFORM,	// ���̺���
		EUROFORM,	// ������
		FILLERSP,	// �ٷ������̼�
		PLYWOOD,	// ����
		TIMBER		// ����
	};

	enum	objCornerType_forLowSideTableformPlacer {
		NOCORNER = 1,
		INCORNER_PANEL,
		OUTCORNER_PANEL,
		OUTCORNER_ANGLE
	};
}

// ���� ���� ����
struct InfoMorphForLowSideTableform
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
struct CellForLowSideTableform
{
	double	leftBottomX;	// ���ϴ� ��ǥ X
	double	leftBottomY;	// ���ϴ� ��ǥ Y
	double	leftBottomZ;	// ���ϴ� ��ǥ Z
	double	ang;			// ȸ�� ���� (����: Radian, ȸ����: Z��)

	short	objType;		// ��ü Ÿ��: ����, ���̺���, ������, �ٷ������̼�, ����, ����
	
	int		horLen;			// ���� ����
	int		verLen;			// ���� ����

	// ���̺��� �� ������ ����
	int		tableInHor [10];	// ���� ����
};

// ���� ���� ���� ����
class LowSideTableformPlacingZone
{
public:
	double	leftBottomX;		// ���ϴ� ��ǥ X
	double	leftBottomY;		// ���ϴ� ��ǥ Y
	double	leftBottomZ;		// ���ϴ� ��ǥ Z

	double	horLen;				// ���� ����
	double	verLen;				// ���� ����
	double	ang;				// ȸ�� ���� (����: Radian, ȸ����: Z��)

	bool	bVertical;			// ����: ���ι���(true), ���ι���(false)

	short	typeLcorner;		// ���� �ڳ� Ÿ��	: (1) ���� (2) ���ڳ��ǳ� (3) �ƿ��ڳ��ǳ� (4) �ƿ��ڳʾޱ�
	short	typeRcorner;		// ������ �ڳ� Ÿ��	: (1) ���� (2) ���ڳ��ǳ� (3) �ƿ��ڳ��ǳ� (4) �ƿ��ڳʾޱ�
	double	lenLcorner;			// ���� ���ڳ��ǳ�/�ƿ��ڳ��ǳ� ����
	double	lenRcorner;			// ������ ���ڳ��ǳ�/�ƿ��ڳ��ǳ� ����

	short	tableformType;		// ���̺��� Ÿ��: Ÿ��A (1), Ÿ��B (2), Ÿ��C (3)

	short	nCellsInHor;		// ���� ���� �� ����

	CellForLowSideTableform		cells [50];				// �� �迭 (���ڳ� ����)

public:
	int	presetWidthVertical_tableform [65];		// ���� ���� ���̺����� �ʺ� ���� (3600 ... 200)
	int	presetWidthHorizontal_tableform [11];	// ���� ���� ���̺����� �ʺ� ���� (3600 ... 600)

	int	presetWidthVertical_euroform [7];		// ���� ���� �������� �ʺ� ���� (600 .. 200)
	int	presetHeightHorizontal_euroform [4];	// ���� ���� �������� �ʺ� ���� (1200 ... 600)

	int	presetWidth_config_vertical [65][7];	// ���� ���� ���̺��� �� �������� �迭 ���� !!!
	int	presetWidth_config_horizontal [11][4];	// ���� ���� ���̺��� �� �������� �迭 ���� !!!

public:
	LowSideTableformPlacingZone ();		// �⺻ ������
	void	initCells (LowSideTableformPlacingZone* placingZone, bool bVertical);					// �� ���� �ʱ�ȭ
	double	getCellPositionLeftBottomX (LowSideTableformPlacingZone* placingZone, short idx);		// ��(0-��� �ε��� ��ȣ)�� ���ϴ� �� ��ġ X ��ǥ�� ����
	void	adjustCellsPosition (LowSideTableformPlacingZone* placingZone);							// �� ��ġ�� �ٸ��� ������
	GSErrCode	placeObjects (LowSideTableformPlacingZone* placingZone);							// �� ������ ������� ��ü���� ��ġ��

	void	placeEuroformsOfTableform (LowSideTableformPlacingZone* placingZone, short idxCell);	// ���̺��� �� ������ ��ġ (����)
	void	placeTableformA (LowSideTableformPlacingZone* placingZone, short idxCell);				// ���̺��� Ÿ��A ��ġ (������ ����) - �������� 2��

public:
	// ���̾�α� ���� ��� �ε��� ��ȣ ����
	short	POPUP_DIRECTION;
	short	POPUP_TABLEFORM_TYPE;
	short	BUTTON_ADD_HOR;
	short	BUTTON_DEL_HOR;
	short	EDITCONTROL_REMAIN_WIDTH;
	short	EDITCONTROL_CURRENT_HEIGHT;
	short	BUTTON_LCORNER;
	short	POPUP_OBJ_TYPE_LCORNER;
	short	EDITCONTROL_WIDTH_LCORNER;
	short	BUTTON_RCORNER;
	short	POPUP_OBJ_TYPE_RCORNER;
	short	EDITCONTROL_WIDTH_RCORNER;
	short	BUTTON_OBJ [50];
	short	POPUP_OBJ_TYPE [50];
	short	POPUP_WIDTH [50];
	short	EDITCONTROL_WIDTH [50];

	//short	LABEL_TOTAL_WIDTH;
	//short	POPUP_WIDTH_IN_TABLE [4];
};

GSErrCode	placeTableformOnLowSide (void);				// ���� ������ ���鿡 ���̺����� ��ġ�ϴ� ���� ��ƾ
short DGCALLBACK lowSideTableformPlacerHandler1 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);				// ���̺���/������/�ٷ������̼�/����/���� ��ġ�� ���� ���̾�α� (���̺��� ����, ��� ����, ���� �� ����)
short DGCALLBACK lowSideTableformPlacerHandler2 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);				// ��ü�� ���̾ �����ϱ� ���� ���̾�α�
short DGCALLBACK lowSideTableformPlacerHandler3_Vertical (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// ���̺��� ���ι��⿡ ���Ͽ� �������� ���� �迭�� �����ϱ� ���� ���̾�α�
short DGCALLBACK lowSideTableformPlacerHandler3_Horizontal (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// ���̺��� ���ι��⿡ ���Ͽ� �������� ���� �迭�� �����ϱ� ���� ���̾�α�

#endif