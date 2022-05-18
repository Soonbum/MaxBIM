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

	short	nCellsInHor;		// ���� ���� ��(������) ����

	CellForLowSideTableform		cells [50];				// �� �迭 (���ڳ� ����)

public:
	//int	presetWidthVertical_tableform [40];		// ���� ���� ���̺����� �ʺ� ���� (3600 ... 200)
	//int	presetWidthHorizontal_tableform [16];	// ���� ���� ���̺����� ���� ���� (3600 ... 1200)

	//int	presetWidth_config_vertical [40][5];	// ���� ���� ���̺��� �� �������� �迭 ����
	//int	presetWidth_config_horizontal [16][6];	// ���� ���� ���̺��� �� �������� �迭 ����

public:
	//WallTableformPlacingZone ();	// �⺻ ������
	//void	initCells (WallTableformPlacingZone* placingZone, bool bVertical);				// �� ���� �ʱ�ȭ
	//double	getCellPositionLeftBottomX (WallTableformPlacingZone* placingZone, short idx);	// ��(0-��� �ε��� ��ȣ)�� ���ϴ� �� ��ġ X ��ǥ�� ����
	//void	adjustCellsPosition (WallTableformPlacingZone* placingZone);					// �� ��ġ�� �ٸ��� ������
	//GSErrCode	placeObjects (WallTableformPlacingZone* placingZone);						// �� ������ ������� ��ü���� ��ġ��

	//void	placeEuroformsOfTableform (WallTableformPlacingZone* placingZone, short idxCell);	// ���̺��� �� ������ ��ġ (����)
	//void	placeTableformA (WallTableformPlacingZone* placingZone, short idxCell);				// ���̺��� Ÿ��A ��ġ (������ ����) - �������� 2��
	//void	placeTableformB (WallTableformPlacingZone* placingZone, short idxCell);				// ���̺��� Ÿ��B ��ġ (������ ����) - �������� 1��
	//void	placeTableformC (WallTableformPlacingZone* placingZone, short idxCell);				// ���̺��� Ÿ��C ��ġ (������ ����) - �������� 1��, ���� ����Ʈ �� Ȱ��

public:
	// ���̾�α� ���� ��� �ε��� ��ȣ ����
	//short	EDITCONTROL_GAP;
	//short	CHECKBOX_SINGLESIDE;
	//short	POPUP_DIRECTION;
	//short	POPUP_TABLEFORM_TYPE;
	//short	EDITCONTROL_REMAIN_WIDTH;
	//short	EDITCONTROL_REMAIN_HEIGHT_BASIC;
	//short	EDITCONTROL_REMAIN_HEIGHT_EXTRA;
	//short	BUTTON_ADD_HOR;
	//short	BUTTON_DEL_HOR;
	//short	CHECKBOX_LINCORNER;
	//short	EDITCONTROL_LINCORNER;
	//short	CHECKBOX_RINCORNER;
	//short	EDITCONTROL_RINCORNER;
	//short	BUTTON_ADD_VER_BASIC;
	//short	BUTTON_DEL_VER_BASIC;
	//short	BUTTON_ADD_VER_EXTRA;
	//short	BUTTON_DEL_VER_EXTRA;

	//short	BUTTON_OBJ [50];
	//short	POPUP_OBJ_TYPE [50];
	//short	POPUP_WIDTH [50];
	//short	EDITCONTROL_WIDTH [50];
	//short	POPUP_HEIGHT_PRESET;
	//short	POPUP_HEIGHT_BASIC [10];
	//short	POPUP_HEIGHT_EXTRA [10];

	//short	LABEL_TOTAL_WIDTH;
	//short	POPUP_WIDTH_IN_TABLE [4];
};

GSErrCode	placeTableformOnLowSide (void);				// ���� ������ ���鿡 ���̺����� ��ġ�ϴ� ���� ��ƾ

#endif