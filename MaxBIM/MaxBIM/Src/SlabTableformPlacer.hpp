#ifndef	__SLAB_TABLEFORM_PLACER__
#define __SLAB_TABLEFORM_PLACER__

#include "MaxBIM.hpp"

namespace slabTableformPlacerDG {
	// ��ü Ÿ��
	enum	libPartObjType {
		NONE,				// ����
		EUROFORM,			// ������
		TABLEFORM,			// ������ ���̺��� (���ǳ�)
		PLYWOOD				// ����
	};

	// ���̾�α� �׸� �ε���
	enum	idxItems_1_forSlabBottomTableformPlacer {
		LABEL_SELECT_TYPE = 3,
		PUSHRADIO_EUROFORM,
		PUSHRADIO_TABLEFORM,
		PUSHRADIO_PLYWOOD,

		LABEL_CELL_SETTINGS,
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
		CHECKBOX_LAYER_COUPLING,
		LABEL_LAYER_EUROFORM,
		LABEL_LAYER_SLABTABLEFORM,
		LABEL_LAYER_PLYWOOD,
		LABEL_LAYER_TIMBER,
		LABEL_LAYER_CPROFILE,
		LABEL_LAYER_PINBOLT,
		LABEL_LAYER_FITTINGS,
		LABEL_LAYER_GT24_GIRDER,
		LABEL_LAYER_PERI_SUPPORT,
		LABEL_LAYER_STEEL_SUPPORT,

		USERCONTROL_LAYER_EUROFORM,
		USERCONTROL_LAYER_SLABTABLEFORM,
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

	short	iTableformType;		// ���̺��� Ÿ�� (1: ������, 2: ���ǳ� ���̺���, 3: ����)
	double	gap;				// ��������� ����
	bool	bVertical;			// true�̸� ���� ����, false�̸� ���� ����

	double	borderHorLen;		// �ֿܰ� ���� ����
	double	borderVerLen;		// �ֿܰ� ���� ����

	double	initHorLen;			// �� ���� ���� �ʱⰪ
	double	initVerLen;			// �� ���� ���� �ʱⰪ

	double	leftBottomX;		// ���̺��� ���� ��ǥ X
	double	leftBottomY;		// ���̺��� ���� ��ǥ Y
	double	leftBottomZ;		// ���̺��� ���� ��ǥ Z
	double	formArrayWidth;		// ���̺��� �迭 ��ü �ʺ�
	double	formArrayHeight;	// ���̺��� �迭 ��ü ����

	double	marginLeft;			// ���� ���� ����
	double	marginRight;		// ������ ���� ����
	double	marginTop;			// ���� ���� ����
	double	marginBottom;		// �Ʒ��� ���� ����

	short	nHorCells;			// ���� ���� �� ����
	short	nVerCells;			// ���� ���� �� ����

	CellForSlabTableform	cells [50][50];		// �� ����

public:
	void		initCells (SlabTableformPlacingZone* placingZone);											// Cell �迭�� �ʱ�ȭ��
	//API_Guid	placeLibPart (CellForSlabTableform objInfo);												// �ش� �� ������ ������� ���̺귯�� ��ġ
	//API_Guid	placeLibPartOnSlabTableform (CellForSlabTableform objInfo);									// ������ ���̺����� �μ� ö���鿡 �ش��ϴ� ���̺귯�� ��ġ
	//GSErrCode	fillRestAreas (void);																		// �������� ä�� �� ������ ���� ä���
};

// ���̺��� ������ �Ϻ� ��ġ �Լ�
GSErrCode	placeTableformOnSlabBottom (void);	// ������ �Ϻο� ���̺����� ��ġ�ϴ� ���� ��ƾ
short DGCALLBACK slabBottomTableformPlacerHandler1 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 1�� ��ġ�� ���� ���Ǹ� ��û�ϴ� 1�� ���̾�α�
short DGCALLBACK slabBottomTableformPlacerHandler2 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 1�� ��ġ �� ������ ��û�ϴ� 2�� ���̾�α�
short DGCALLBACK slabBottomTableformPlacerHandler3 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 2�� ���̾�α׿��� �� ���� ��ü Ÿ���� �����ϱ� ���� 3�� ���̾�α�

#endif