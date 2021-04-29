#ifndef	__SLAB_EUROFORM_PLACER__
#define __SLAB_EUROFORM_PLACER__

#include "MaxBIM.hpp"

namespace slabBottomPlacerDG {
	// ��ü Ÿ��
	enum	libPartObjType {
		NONE,			// ����
		EUROFORM,		// ������v2.0
		PLYWOOD,		// ����v1.0
		WOOD			// ����v1.0
	};

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

		LABEL_GAP_LENGTH,
		EDITCONTROL_GAP_LENGTH,
		SEPARATOR_2,

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
		LABEL_GRID_EUROFORM_WOOD,
		PUSHBUTTON_CONFIRM_REMAIN_LENGTH,
		PUSHBUTTON_ADD_ROW,
		PUSHBUTTON_DEL_ROW,
		PUSHBUTTON_ADD_COL,
		PUSHBUTTON_DEL_COL,

		// ���Ŀ��� �׸��� ��ư�� ��ġ��
		GRIDBUTTON_IDX_START
	};

	enum	idxItems_3_forSlabBottomPlacer {
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
		LABEL_EUROFORM_WIDTH_OPTIONS,
		POPUP_EUROFORM_WIDTH_OPTIONS,
		EDITCONTROL_EUROFORM_WIDTH_OPTIONS,
		LABEL_EUROFORM_HEIGHT_OPTIONS,
		POPUP_EUROFORM_HEIGHT_OPTIONS,
		EDITCONTROL_EUROFORM_HEIGHT_OPTIONS,
		LABEL_EUROFORM_ORIENTATION_OPTIONS,
		RADIO_ORIENTATION_1_EUROFORM,
		RADIO_ORIENTATION_2_EUROFORM
	};

	enum	idxItems_4_forSlabBottomPlacer {
		LABEL_WOOD_WIDTH = 3,
		EDITCONTROL_WOOD_WIDTH
	};
}

// ���� ���� ����
struct InfoMorphForSlab
{
	API_Guid	guid;		// ������ GUID
	short		floorInd;	// �� �ε���
	double		level;		// ������ ��
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
class SlabPlacingZone
{
public:
	// �¿����: ������ ������ ���� ���� ������ ����

	// ������ ���� ����
	double	level;				// ��
	double	ang;				// ȸ�� ���� (����: Radian, ȸ����: Z��)
	double	leftBottomX;		// ������ ���� ��ǥ X
	double	leftBottomY;		// ������ ���� ��ǥ Y
	double	leftBottomZ;		// ������ ���� ��ǥ Z
	double	formArrayWidth;		// ������ �迭 ��ü �ʺ�
	double	formArrayHeight;	// ������ �迭 ��ü ����
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

	// �������� ��ġ�Ǵ� ������ �ʺ�/����
	double	innerWidth;		// ���� ����
	double	innerHeight;	// ���� ����

	// ������ ���� (1. �⺻ ä���)
	double	remain_hor;				// ���� ���� ���� ����
	double	remain_hor_updated;		// ���� ���� ���� ���� (������Ʈ ��)
	double	remain_ver;				// ���� ���� ���� ����
	double	remain_ver_updated;		// ���� ���� ���� ���� (������Ʈ ��)

	double	gap;		// ��������� ����

	double	woodWidth;	// ���� �ʺ�

	std::string		eu_wid;			// ������ �ʺ�
	std::string		eu_hei;			// ������ ����
	std::string		eu_ori;			// ������ ����
	double	eu_wid_numeric;			// ������ �ʺ� (�Ǽ���)
	double	eu_hei_numeric;			// ������ ���� (�Ǽ���)
	short	eu_count_hor;			// ���� ���� ������ ����
	short	eu_count_ver;			// ���� ���� ������ ����

	// ������ ���� (2. ��ġ�� ��ü ������ �׸���� ����)
	CellForSlab	cells [50][50];		// ������ �� ���� - ������ �ε���: [eu_count_ver-1][eu_count_hor-1]
	bool	topBoundsCells [50];	// ���� ���� ������ ���� - ������ �ε���: [eu_count_hor-2] : LeftBottom���� RightBottom����
	bool	bottomBoundsCells [50];	// �Ʒ��� ���� ������ ���� - ������ �ε���: [eu_count_hor-2] : LeftTop���� RightTop����
	bool	leftBoundsCells [50];	// ���� ���� ������ ���� - ������ �ε���: [eu_count_ver-2] : LeftBottom���� LeftTop����
	bool	rightBoundsCells [50];	// ������ ���� ������ ���� - ������ �ε���: [eu_count_ver-2] : RightBottom���� RightTop����

public:
	void		initCells (SlabPlacingZone* placingZone);											// Cell �迭�� �ʱ�ȭ��
	void		firstPlacingSettings (SlabPlacingZone* placingZone);								// 1�� ��ġ: ������
	void		adjustOtherCellsInSameRow (SlabPlacingZone* target_zone, short row, short col);		// �ش� ���� ������ �࿡ �ִ� �ٸ� ������ Ÿ�� �� ���̸� ������
	void		adjustOtherCellsInSameCol (SlabPlacingZone* target_zone, short row, short col);		// �ش� ���� ������ ���� �ִ� �ٸ� ������ Ÿ�� �� �ʺ� ������
	void		addNewRow (SlabPlacingZone* target_zone);											// ���ο� ���� �߰��� (�� �ϳ��� �ø��� �߰��� �࿡ ������ �� ���� ����)
	void		addNewCol (SlabPlacingZone* target_zone);											// ���ο� ���� �߰��� (�� �ϳ��� �ø��� �߰��� ���� ������ �� ���� ����)
	void		delLastRow (SlabPlacingZone* target_zone);											// ������ ���� ������
	void		delLastCol (SlabPlacingZone* target_zone);											// ������ ���� ������
	void		alignPlacingZone (SlabPlacingZone* target_zone);									// Cell ������ ����ʿ� ���� ����ȭ�� ��ġ�� ��������
	API_Guid	placeLibPart (CellForSlab objInfo);													// �ش� �� ������ ������� ���̺귯�� ��ġ
	GSErrCode	fillRestAreas (void);																// �������� ä�� �� ������ ���� ä���
};

// ������ ������ �Ϻ� ��ġ �Լ�
GSErrCode	placeEuroformOnSlabBottom (void);	// ������ �Ϻο� �������� ��ġ�ϴ� ���� ��ƾ
short DGCALLBACK slabBottomPlacerHandler1 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 1�� ��ġ�� ���� ���Ǹ� ��û�ϴ� 1�� ���̾�α�
short DGCALLBACK slabBottomPlacerHandler2 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 1�� ��ġ �� ������ ��û�ϴ� 2�� ���̾�α�
short DGCALLBACK slabBottomPlacerHandler3 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 2�� ���̾�α׿��� �� ���� ��ü Ÿ���� �����ϱ� ���� 3�� ���̾�α�
short DGCALLBACK slabBottomPlacerHandler4 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// ������ ä��⸦ �� ��, ������ �ʺ� �����ϱ� ���� 4�� ���̾�α�

#endif
