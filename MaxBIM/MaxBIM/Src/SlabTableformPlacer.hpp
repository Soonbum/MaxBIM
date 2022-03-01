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
		POPUP_WIDTH,
		LABEL_HEIGHT,
		POPUP_HEIGHT,
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

	short	iTableformType;		// ���̺��� Ÿ�� (1: ������, 2: ���ǳ� ���̺���, 3: ����)
	double	gap;				// ��������� ����
	bool	bVertical;			// true�̸� ���� ����, false�̸� ���� ����
	bool	bRectangleArea;		// ���簢�� �����̸� true, ���� �𼭸� �����̸� false

	double	borderHorLen;		// �ֿܰ� ���� ����
	double	borderVerLen;		// �ֿܰ� ���� ����

	API_Coord3D		center;		// �ֿܰ� �߽� ��
	API_Coord3D		leftBottom;	// �ֿܰ� ���ϴ� ��
	API_Coord3D		rightTop;	// �ֿܰ� ���� ��

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

	CellForSlabTableform	cells [50][50];		// �� ���� (0���� �ϴ�, 0���� ����)

public:
	void		initCells (SlabTableformPlacingZone* placingZone);											// Cell �迭�� �ʱ�ȭ��
	void		alignPlacingZone (SlabTableformPlacingZone* placingZone);									// Cell �迭�� ��ġ�� ������
	GSErrCode	fillTableformAreas (void);																	// ���̺��� ���� ä���
	GSErrCode	fillRestAreas (void);																		// ������ ���� ä��� (����, ����)
	//API_Guid	placeLibPart (CellForSlabTableform objInfo);												// �ش� �� ������ ������� ���̺귯�� ��ġ
	//API_Guid	placeLibPartOnSlabTableform (CellForSlabTableform objInfo);									// ������ ���̺����� �μ� ö���鿡 �ش��ϴ� ���̺귯�� ��ġ

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