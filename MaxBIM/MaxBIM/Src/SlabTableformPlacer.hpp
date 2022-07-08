#ifndef	__SLAB_TABLEFORM_PLACER__
#define __SLAB_TABLEFORM_PLACER__

#include "MaxBIM.hpp"

namespace slabTableformPlacerDG {
	// ��ü Ÿ��
	enum	libPartObjType {
		NONE,				// ����
		CONPANEL,			// ���ǳ�
		PLYWOOD,			// ����
		EUROFORM			// ������
	};

	// ���̾�α� �׸� �ε���
	enum	idxItems_1_forSlabBottomTableformPlacer {
		LABEL_SELECT_TYPE = 3,
		PUSHRADIO_CONPANEL,
		PUSHRADIO_PLYWOOD,
		PUSHRADIO_EUROFORM,

		LABEL_OTHER_SETTINGS,
		LABEL_CELL_DIRECTION,
		POPUP_CELL_DIRECTION,
		LABEL_JOIST_DIRECTION,
		POPUP_JOIST_DIRECTION,
		LABEL_YOKE_TYPE,
		POPUP_YOKE_TYPE,
		LABEL_SUPPORTING_POST_TYPE,
		POPUP_SUPPORTING_POST_TYPE,

		SEPARATOR_1,
		LABEL_GAP_LENGTH,
		EDITCONTROL_GAP_LENGTH,
		SEPARATOR_2,

		ICON_LAYER,
		LABEL_LAYER_SETTINGS,
		CHECKBOX_LAYER_COUPLING,
		LABEL_LAYER_EUROFORM,
		LABEL_LAYER_CONPANEL,
		LABEL_LAYER_PLYWOOD,
		LABEL_LAYER_TIMBER,
		LABEL_LAYER_CPROFILE,
		LABEL_LAYER_PINBOLT,
		LABEL_LAYER_FITTINGS,
		LABEL_LAYER_GT24_GIRDER,
		LABEL_LAYER_PERI_SUPPORT,
		LABEL_LAYER_STEEL_SUPPORT,

		USERCONTROL_LAYER_EUROFORM,
		USERCONTROL_LAYER_CONPANEL,
		USERCONTROL_LAYER_PLYWOOD,
		USERCONTROL_LAYER_TIMBER,
		USERCONTROL_LAYER_CPROFILE,
		USERCONTROL_LAYER_PINBOLT,
		USERCONTROL_LAYER_FITTINGS,
		USERCONTROL_LAYER_GT24_GIRDER,
		USERCONTROL_LAYER_PERI_SUPPORT,
		USERCONTROL_LAYER_STEEL_SUPPORT,

		BUTTON_AUTOSET
	};

	enum	idxItems_Direction_forSlabBottomTableformPlacer {
		HORIZONTAL = 1,
		VERTICAL
	};

	enum	idxItems_Yoke_forSlabBottomTableformPlacer {
		GT24_GIRDER = 1,
		SANSUNGAK
	};

	enum	idxItems_SupportingPostType_forSlabBottomTableformPlacer {
		SUPPORT = 1,
		PERI
	};

	enum	idxItems_2_forSlabBottomTableformPlacer {
		DG_PREV = 3,
		PUSHBUTTON_ADD_ROW,
		PUSHBUTTON_DEL_ROW,
		PUSHBUTTON_ADD_COL,
		PUSHBUTTON_DEL_COL,

		// ���Ŀ��� �׸��� ��ư�� ��ġ��
		AFTER_ALL
	};

	enum	idxItems_3_forSlabBottomTableformPlacer {
		LABEL_OBJ_TYPE	= 3,
		POPUP_OBJ_TYPE,
		LABEL_WIDTH,
		EDITCONTROL_WIDTH,
		LABEL_HEIGHT,
		EDITCONTROL_HEIGHT,
		LABEL_ORIENTATION,
		POPUP_ORIENTATION,
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
	short	objType;		// enum libPartObjType ����

	double	leftBottomX;	// ���ϴ� ��ǥ X
	double	leftBottomY;	// ���ϴ� ��ǥ Y
	double	leftBottomZ;	// ���ϴ� ��ǥ Z

	double	horLen;			// ���� ����
	double	verLen;			// ���� ����
	double	ang;			// ȸ�� ���� (����: Radian, ȸ����: Z��)
};

// ������ �Ϻ� ���� ����
class SlabTableformPlacingZone
{
public:
	// �¿����: ������ �Ʒ����� ���� ���� ������ ����
	// ������ ���� ����
	double	level;				// ��
	double	ang;				// ȸ�� ���� (����: Radian, ȸ����: Z��)

	short	iTableformType;		// ���̺��� Ÿ�� (1: ���ǳ�, 2: ����, 3: ������)
	short	iCellDirection;		// �� ���� (1: ����, 2: ����)
	short	iJoistDirection;	// �弱 ���� (1: ����, 2: ����)
	short	iYokeType;			// �ۿ��� ���� (1: GT24 �Ŵ�, 2: ��°�)
	short	iSuppPostType;		// ���ٸ� ���� (1: ����Ʈ(�������ٸ�), 2: PERI������(PERI���ٸ�))

	double	gap;				// ��������� ����
	bool	bRectangleArea;		// ���簢�� �����̸� true, ���� �𼭸� �����̸� false

	double	borderHorLen;		// �ֿܰ� ���� ����
	double	borderVerLen;		// �ֿܰ� ���� ����

	API_Coord3D		center;		// �ֿܰ� �߽� ��
	API_Coord3D		leftBottom;	// �ֿܰ� ���ϴ� ��
	API_Coord3D		rightTop;	// �ֿܰ� ���� ��

	double	initCellHorLen;		// �� �ʱ� �ʺ�
	double	initCellVerLen;		// �� �ʱ� ����

	double	cellArrayWidth;		// �� �迭 ��ü �ʺ�
	double	cellArrayHeight;	// �� �迭 ��ü ����

	short	nHorCells;			// ���� ���� �� ����
	short	nVerCells;			// ���� ���� �� ����

	double	marginLeft;			// ���� ���� ����
	double	marginRight;		// ������ ���� ����
	double	marginTop;			// ���� ���� ����
	double	marginBottom;		// �Ʒ��� ���� ����

	double	leftBottomX;		// �� ���� ��ǥ X
	double	leftBottomY;		// �� ���� ��ǥ Y
	double	leftBottomZ;		// �� ���� ��ǥ Z

	CellForSlabTableform	cells [50][50];		// �� ���� (0���� �ϴ�, 0���� ����)

public:
	void		initCells (SlabTableformPlacingZone* placingZone);			// Cell �迭�� �ʱ�ȭ��
	void		alignPlacingZone (SlabTableformPlacingZone* placingZone);	// Cell �迭�� ��ġ�� ������
	GSErrCode	fillCellAreas (void);										// �� �迭 ���� ä���
	GSErrCode	fillMarginAreas (void);										// ���� ���� ä��� (����)
	GSErrCode	placeInsulations (void);									// �ܿ��� ��ġ

public:
	// ���̾�α� ���� ��� �ε��� ��ȣ ����
	short	CELL_BUTTON [50][50];
	short	EDITCONTROL_MARGIN_LEFT;
	short	EDITCONTROL_MARGIN_RIGHT;
	short	EDITCONTROL_MARGIN_TOP;
	short	EDITCONTROL_MARGIN_BOTTOM;
};

// ���̺��� ������ �Ϻ� ��ġ �Լ�
GSErrCode	placeTableformOnSlabBottom (void);	// ������ �Ϻο� ���̺����� ��ġ�ϴ� ���� ��ƾ
short DGCALLBACK slabBottomTableformPlacerHandler1 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 1�� ��ġ�� ���� ���Ǹ� ��û�ϴ� 1�� ���̾�α�
short DGCALLBACK slabBottomTableformPlacerHandler2 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 1�� ��ġ �� ������ ��û�ϴ� 2�� ���̾�α�
short DGCALLBACK slabBottomTableformPlacerHandler3 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 2�� ���̾�α׿��� �� ���� ��ü Ÿ���� �����ϱ� ���� 3�� ���̾�α�

#endif