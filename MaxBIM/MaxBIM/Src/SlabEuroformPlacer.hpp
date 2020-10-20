#ifndef	__SLAB_EUROFORM_PLACER__
#define __SLAB_EUROFORM_PLACER__

#include "MaxBIM.hpp"

namespace slabBottomPlacerDG {
	// ���̾�α� �׸� �ε���
	enum	idxItems_1_forSlabBottomPlacer {
		LABEL_PLACING_EUROFORM		= 3,
		LABEL_EUROFORM_WIDTH,
		POPUP_EUROFORM_WIDTH,
		LABEL_EUROFORM_HEIGHT,
		POPUP_EUROFORM_HEIGHT,
		LABEL_EUROFORM_ORIENTATION,
		POPUP_EUROFORM_ORIENTATION,
		SEPARATOR_1,

		ICON_LAYER,
		LABEL_LAYER_SETTINGS,
		LABEL_LAYER_EUROFORM,
		LABEL_LAYER_PLYWOOD,
		LABEL_LAYER_WOOD,

		USERCONTROL_LAYER_EUROFORM,
		USERCONTROL_LAYER_PLYWOOD,
		USERCONTROL_LAYER_WOOD
	};

	enum	idxItems_2_forSlabBottomPlacer {
		LABEL_REMAIN_HORIZONTAL_LENGTH			= 3,
		LABEL_REMAIN_VERTICAL_LENGTH			= 4,
		EDITCONTROL_REMAIN_HORIZONTAL_LENGTH	= 5,
		EDITCONTROL_REMAIN_VERTICAL_LENGTH		= 6,
		GROUPBOX_GRID_EUROFORM_WOOD				= 7,
		PUSHBUTTON_CONFIRM_REMAIN_LENGTH		= 8,

		// ���Ŀ��� �׸��� ��ư�� ��ġ��
		GRIDBUTTON_IDX_START					= 9
	};
}

// ������ ���� ����
struct InfoSlab
{
	short	floorInd;			// �� �ε���
	double	offsetFromTop;		// ���۷��� ������ ������ ���� ���� ���� �Ÿ�
	double	thickness;			// ������ �β�
};

// ���� ���� ����
struct InfoMorphForSlab
{
	API_Guid	guid;		// ������ GUID

	double	leftBottomX;	// ���ϴ� ��ǥ X
	double	leftBottomY;	// ���ϴ� ��ǥ Y
	double	leftBottomZ;	// ���ϴ� ��ǥ Z

	//double	rightTopX;		// ���� ��ǥ X
	//double	rightTopY;		// ���� ��ǥ Y
	//double	rightTopZ;		// ���� ��ǥ Z

	double	horLen;			// ���� ����
	double	verLen;			// ���� ����
	double	ang;			// ȸ�� ���� (����: Degree, ȸ����: Z��)

	// ...
};

// �׸��� �� �� ����
struct CellForSlab
{
	short		objType;	// enum libPartObjType ����

	API_Guid	guid;		// ��ü�� GUID

	double	leftBottomX;	// ���ϴ� ��ǥ X
	double	leftBottomY;	// ���ϴ� ��ǥ Y
	double	leftBottomZ;	// ���ϴ� ��ǥ Z

	double	horLen;			// ���� ����
	double	verLen;			// ���� ����
	double	ang;			// ȸ�� ���� (����: Radian, ȸ����: Z��)

	union {
		Euroform		form;
		Plywood			plywood;
		Wood			wood;
	} libPart;
};

// ������ �Ϻ� ���� ����
struct SlabPlacingZone
{
	// ������ ���� ����
	double	level;				// ��
	double	ang;				// ȸ�� ���� (����: Radian, ȸ����: Z��)
	double	leftBottomX;		// ������ ���� ��ǥ X
	double	leftBottomY;		// ������ ���� ��ǥ Y
	double	leftBottomZ;		// ������ ���� ��ǥ Z
	double	formArrayWidth;		// ������ �迭 ��ü �ʺ�
	double	formArrayHeight;	// ������ �迭 ��ü ����

	// �ֿܰ� ��ǥ
	double	outerLeft;		// X ��ǥ
	double	outerRight;		// X ��ǥ
	double	outerTop;		// Y ��ǥ
	double	outerBottom;	// Y ��ǥ

	// ���� �κ� �ڳ� ��ǥ, �Ǵ� �Ϲ� �ڳ� ��ǥ
	API_Coord3D		corner_leftTop;
	API_Coord3D		corner_leftBottom;
	API_Coord3D		corner_rightTop;
	API_Coord3D		corner_rightBottom;

	// ���� ��ǥ
	double	innerLeft;		// X ��ǥ
	double	innerRight;		// X ��ǥ
	double	innerTop;		// Y ��ǥ
	double	innerBottom;	// Y ��ǥ

	// �������� ��ġ�Ǵ� ������ �ʺ�/����
	double	innerWidth;		// ���� ����
	double	innerHeight;	// ���� ����

	// ������ ���� (1. �⺻ ä���)
	double	remain_hor;				// ���� ���� ���� ����
	double	remain_hor_updated;		// ���� ���� ���� ���� (������Ʈ ��)
	double	remain_ver;				// ���� ���� ���� ����
	double	remain_ver_updated;		// ���� ���� ���� ���� (������Ʈ ��)

	std::string		eu_wid;			// ������ �ʺ�
	std::string		eu_hei;			// ������ ����
	std::string		eu_ori;			// ������ ����
	double	eu_wid_numeric;			// ������ �ʺ� (�Ǽ���)
	double	eu_hei_numeric;			// ������ ���� (�Ǽ���)
	short	eu_count_hor;			// ���� ���� ������ ����
	short	eu_count_ver;			// ���� ���� ������ ����

	// ������ ���� (2. ��ġ�� ��ü ������ �׸���� ����)
	CellForSlab	cells [50][50];		// ������ �� ���� - ������ �ε���: [eu_count_ver-1][eu_count_hor-1]
	bool	topBoundsCells [50];	// ���� ���� ������ ���� - ������ �ε���: [eu_count_hor-2]
	bool	bottomBoundsCells [50];	// �Ʒ��� ���� ������ ���� - ������ �ε���: [eu_count_hor-2]
	bool	leftBoundsCells [50];	// ���� ���� ������ ���� - ������ �ε���: [eu_count_ver-2]
	bool	rightBoundsCells [50];	// ������ ���� ������ ���� - ������ �ε���: [eu_count_ver-2]
};

// ������ ������ �Ϻ� ��ġ �Լ�
GSErrCode	placeEuroformOnSlabBottom (void);				// 2�� �޴�: ������ �Ϻο� �������� ��ġ�ϴ� ���� ��ƾ
bool		isSamePoint (API_Coord3D aPoint, API_Coord3D bPoint);												// aPoint�� bPoint�� ���� ������ Ȯ����
bool		isAlreadyStored (API_Coord3D aPoint, API_Coord3D pointList [], short startInd, short endInd);		// aPoint�� pointList�� ������ �Ǿ����� Ȯ����
bool		isNextPoint (API_Coord3D prevPoint, API_Coord3D curPoint, API_Coord3D nextPoint);					// nextPoint�� curPoint�� ���� ���Դϱ�?
short		moreCloserPoint (API_Coord3D curPoint, API_Coord3D p1, API_Coord3D p2);								// curPoint�� ����� ���� p1, p2 �� � ���Դϱ�?
API_Coord3D	getUnrotatedPoint (API_Coord3D rotatedPoint, API_Coord3D axisPoint, double ang);					// ȸ���� ������� �ʾ��� ���� ��ġ (��ġ�Ǿ�� �� ���� ��ġ�� ����), ������ Degree
void		initCellsForSlabBottom (SlabPlacingZone* placingZone);												// Cell �迭�� �ʱ�ȭ��
void		firstPlacingSettingsForSlabBottom (SlabPlacingZone* placingZone);									// 1�� ��ġ: ������
void		setCellPositionForSlabBottom (SlabPlacingZone *target_zone, short ver, short hor);					// �ش� ���� LeftBottom ��ġ�� ����
void		alignPlacingZoneForSlabBottom (SlabPlacingZone* target_zone);										// Cell ������ ����ʿ� ���� ����ȭ�� ��ġ�� ��������
API_Guid	placeLibPartForSlabBottom (CellForSlab objInfo);													// �ش� �� ������ ������� ���̺귯�� ��ġ
GSErrCode	fillRestAreasForSlabBottom (void);																	// �������� ä�� �� ������ ���� ä���
short DGCALLBACK slabBottomPlacerHandler1 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 1�� ��ġ�� ���� ���Ǹ� ��û�ϴ� 1�� ���̾�α�
short DGCALLBACK slabBottomPlacerHandler2 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 1�� ��ġ �� ������ ��û�ϴ� 2�� ���̾�α�
short DGCALLBACK slabBottomPlacerHandler3 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 2�� ���̾�α׿��� �� ���� ��ü Ÿ���� �����ϱ� ���� 3�� ���̾�α�

#endif
