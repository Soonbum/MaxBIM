#ifndef	__SLAB_TABLEFORM_PLACER__
#define __SLAB_TABLEFORM_PLACER__

#include "MaxBIM.hpp"

namespace slabTableformPlacerDG {
	// ��ü Ÿ��
	enum	libPartObjType {
		NONE,				// ����
		SLAB_TABLEFORM,		// ������ ���̺��� (���ǳ�) v1.0
		PLYWOOD,			// ����v1.0
		WOOD,				// ����v1.0
		CPROFILE,			// KS��������v1.0 - C����
		FITTINGS,			// ����ö�� (�簢�ͼ�Ȱ��) v1.0
	};

	// ���̾�α� �׸� �ε���
	enum	idxItems_1_forSlabBottomTableformPlacer {
		LABEL_PLACING_TABLEFORM		= 3,
		LABEL_TABLEFORM_WIDTH,
		POPUP_TABLEFORM_WIDTH,
		LABEL_TABLEFORM_HEIGHT,
		POPUP_TABLEFORM_HEIGHT,
		LABEL_TABLEFORM_ORIENTATION,
		POPUP_TABLEFORM_ORIENTATION,
		SEPARATOR_1,

		LABEL_GAP_LENGTH,
		EDITCONTROL_GAP_LENGTH,
		SEPARATOR_2,

		ICON_LAYER,
		LABEL_LAYER_SETTINGS,
		LABEL_LAYER_SLABTABLEFORM,
		LABEL_LAYER_PLYWOOD,
		LABEL_LAYER_WOOD,
		LABEL_LAYER_CPROFILE,
		LABEL_LAYER_FITTINGS,

		USERCONTROL_LAYER_SLABTABLEFORM,
		USERCONTROL_LAYER_PLYWOOD,
		USERCONTROL_LAYER_WOOD,
		USERCONTROL_LAYER_CPROFILE,
		USERCONTROL_LAYER_FITTINGS,
	};

	enum	idxItems_2_forSlabBottomTableformPlacer {
		DG_CANCEL2 = 3,
		DG_PREV,
		LABEL_REMAIN_HORIZONTAL_LENGTH_LEFT,
		LABEL_REMAIN_HORIZONTAL_LENGTH_RIGHT,
		LABEL_REMAIN_VERTICAL_LENGTH_UP,
		LABEL_REMAIN_VERTICAL_LENGTH_DOWN,
		EDITCONTROL_REMAIN_HORIZONTAL_LENGTH_LEFT,
		EDITCONTROL_REMAIN_HORIZONTAL_LENGTH_RIGHT,
		EDITCONTROL_REMAIN_VERTICAL_LENGTH_UP,
		EDITCONTROL_REMAIN_VERTICAL_LENGTH_DOWN,
		LABEL_GRID_TABLEFORM_WOOD,
		PUSHBUTTON_CONFIRM_REMAIN_LENGTH,
		PUSHBUTTON_ADD_ROW,
		PUSHBUTTON_DEL_ROW,
		PUSHBUTTON_ADD_COL,
		PUSHBUTTON_DEL_COL,

		// ���Ŀ��� �׸��� ��ư�� ��ġ��
		GRIDBUTTON_IDX_START
	};

	enum	idxItems_3_forSlabBottomTableformPlacer {
		LABEL_OBJ_TYPE	= 3,
		POPUP_OBJ_TYPE,
		LABEL_WIDTH,
		EDITCONTROL_WIDTH,
		LABEL_HEIGHT,
		EDITCONTROL_HEIGHT,
		LABEL_ORIENTATION,
		RADIO_ORIENTATION_1_PLYWOOD,
		RADIO_ORIENTATION_2_PLYWOOD,
		CHECKBOX_SET_STANDARD,
		LABEL_TABLEFORM_WIDTH_OPTIONS,
		POPUP_TABLEFORM_WIDTH_OPTIONS,
		EDITCONTROL_TABLEFORM_WIDTH_OPTIONS,
		LABEL_TABLEFORM_HEIGHT_OPTIONS,
		POPUP_TABLEFORM_HEIGHT_OPTIONS,
		EDITCONTROL_TABLEFORM_HEIGHT_OPTIONS,
		LABEL_TABLEFORM_ORIENTATION_OPTIONS,
		RADIO_ORIENTATION_1_TABLEFORM,
		RADIO_ORIENTATION_2_TABLEFORM,
		LABEL_CAUTION
	};
}

// ���� ���� ����
struct InfoMorphForSlabTableform
{
	API_Guid	guid;		// ������ GUID
	short		floorInd;	// �� �ε���
	double		level;		// ������ ��
};

// �׸��� �� �� ����
struct CellForSlabTableform
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
		SlabTableform	tableform;
		Plywood			plywood;
		Wood			wood;
		KSProfile		cprofile;
		MetalFittingsWithRectWasher		fittings;
	} libPart;
};

// ������ �Ϻ� ���� ����
class SlabTableformPlacingZone
{
public:
	// �¿����: ������ ������ ���� ���� ������ ����

	// ������ ���� ����
	double	level;				// ��
	double	ang;				// ȸ�� ���� (����: Radian, ȸ����: Z��)
	double	leftBottomX;		// ���̺��� ���� ��ǥ X
	double	leftBottomY;		// ���̺��� ���� ��ǥ Y
	double	leftBottomZ;		// ���̺��� ���� ��ǥ Z
	double	formArrayWidth;		// ���̺��� �迭 ��ü �ʺ�
	double	formArrayHeight;	// ���̺��� �迭 ��ü ����
	double	upMove;				// ���� �̵�
	double	leftMove;			// �Ʒ��� �̵�

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

	// ���̺����� ��ġ�Ǵ� ������ �ʺ�/����
	double	innerWidth;		// ���� ����
	double	innerHeight;	// ���� ����

	// ������ ���� (1. �⺻ ä���)
	double	remain_hor;				// ���� ���� ���� ����
	double	remain_hor_updated;		// ���� ���� ���� ���� (������Ʈ ��)
	double	remain_ver;				// ���� ���� ���� ����
	double	remain_ver_updated;		// ���� ���� ���� ���� (������Ʈ ��)

	double	gap;		// ��������� ����

	std::string		tb_wid;			// ���̺��� �ʺ�
	std::string		tb_hei;			// ���̺��� ����
	std::string		tb_ori;			// ���̺��� ����
	double	tb_wid_numeric;			// ���̺��� �ʺ� (�Ǽ���)
	double	tb_hei_numeric;			// ���̺��� ���� (�Ǽ���)
	short	tb_count_hor;			// ���� ���� ���̺��� ����
	short	tb_count_ver;			// ���� ���� ���̺��� ����

	// ������ ���� (2. ��ġ�� ��ü ������ �׸���� ����)
	CellForSlabTableform	cells [50][50];		// ������ �� ���� - ������ �ε���: [eu_count_ver-1][eu_count_hor-1]
	bool	topBoundsCells [50];				// ���� ���� ������ ���� - ������ �ε���: [eu_count_hor-2] : LeftBottom���� RightBottom����
	bool	bottomBoundsCells [50];				// �Ʒ��� ���� ������ ���� - ������ �ε���: [eu_count_hor-2] : LeftTop���� RightTop����
	bool	leftBoundsCells [50];				// ���� ���� ������ ���� - ������ �ε���: [eu_count_ver-2] : LeftBottom���� LeftTop����
	bool	rightBoundsCells [50];				// ������ ���� ������ ���� - ������ �ε���: [eu_count_ver-2] : RightBottom���� RightTop����

public:
	void		initCells (SlabTableformPlacingZone* placingZone);											// Cell �迭�� �ʱ�ȭ��
	void		firstPlacingSettings (SlabTableformPlacingZone* placingZone);								// 1�� ��ġ: ������
	void		adjustOtherCellsInSameRow (SlabTableformPlacingZone* target_zone, short row, short col);	// �ش� ���� ������ �࿡ �ִ� �ٸ� ������ Ÿ�� �� ���̸� ������
	void		adjustOtherCellsInSameCol (SlabTableformPlacingZone* target_zone, short row, short col);	// �ش� ���� ������ ���� �ִ� �ٸ� ������ Ÿ�� �� �ʺ� ������
	void		addNewRow (SlabTableformPlacingZone* target_zone);											// ���ο� ���� �߰��� (�� �ϳ��� �ø��� �߰��� �࿡ ������ �� ���� ����)
	void		addNewCol (SlabTableformPlacingZone* target_zone);											// ���ο� ���� �߰��� (�� �ϳ��� �ø��� �߰��� ���� ������ �� ���� ����)
	void		delLastRow (SlabTableformPlacingZone* target_zone);											// ������ ���� ������
	void		delLastCol (SlabTableformPlacingZone* target_zone);											// ������ ���� ������
	void		alignPlacingZone (SlabTableformPlacingZone* target_zone);									// Cell ������ ����ʿ� ���� ����ȭ�� ��ġ�� ��������
	API_Guid	placeLibPart (CellForSlabTableform objInfo);												// �ش� �� ������ ������� ���̺귯�� ��ġ
	API_Guid	placeLibPartOnSlabTableform (CellForSlabTableform objInfo);									// ������ ���̺����� �μ� ö���鿡 �ش��ϴ� ���̺귯�� ��ġ
	GSErrCode	fillRestAreas (void);																		// �������� ä�� �� ������ ���� ä���
};

// ���̺��� ������ �Ϻ� ��ġ �Լ�
GSErrCode	placeTableformOnSlabBottom (void);	// ������ �Ϻο� ���̺����� ��ġ�ϴ� ���� ��ƾ
short DGCALLBACK slabBottomTableformPlacerHandler1 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 1�� ��ġ�� ���� ���Ǹ� ��û�ϴ� 1�� ���̾�α�
short DGCALLBACK slabBottomTableformPlacerHandler2 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 1�� ��ġ �� ������ ��û�ϴ� 2�� ���̾�α�
short DGCALLBACK slabBottomTableformPlacerHandler3 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 2�� ���̾�α׿��� �� ���� ��ü Ÿ���� �����ϱ� ���� 3�� ���̾�α�

#endif