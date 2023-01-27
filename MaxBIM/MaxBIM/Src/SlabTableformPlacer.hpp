#ifndef	__SLAB_TABLEFORM_PLACER__
#define __SLAB_TABLEFORM_PLACER__

#include "MaxBIM.hpp"

namespace slabTableformPlacerDG {
	// ���̾�α� �׸� �ε���
	enum	idxItems_1_forSlabBottomTableformPlacer {
		GROUPBOX_PANEL = 3,
		LABEL_PANEL_TYPE,
		LABEL_PANEL_DIRECTION,
		LABEL_PANEL_THICKNESS,
		POPUP_PANEL_TYPE,
		POPUP_PANEL_DIRECTION,
		POPUP_PANEL_THICKNESS,

		GROUPBOX_BEAM,
		LABEL_BEAM_TYPE,
		LABEL_BEAM_DIRECTION,
		LABEL_BEAM_OFFSET_HORIZONTAL,
		LABEL_BEAM_OFFSET_VERTICAL,
		LABEL_BEAM_GAP,
		POPUP_BEAM_TYPE,
		POPUP_BEAM_DIRECTION,
		EDITCONTROL_BEAM_OFFSET_HORIZONTAL,
		EDITCONTROL_BEAM_OFFSET_VERTICAL,
		EDITCONTROL_BEAM_GAP,

		GROUPBOX_GIRDER,
		LABEL_GIRDER_TYPE,
		LABEL_GIRDER_QUANTITY,
		LABEL_GIRDER_OFFSET_HORIZONTAL,
		LABEL_GIRDER_OFFSET_VERTICAL,
		LABEL_GIRDER_GAP,
		POPUP_GIRDER_TYPE,
		POPUP_GIRDER_QUANTITY,
		EDITCONTROL_GIRDER_OFFSET_HORIZONTAL,
		EDITCONTROL_GIRDER_OFFSET_VERTICAL,
		EDITCONTROL_GIRDER_GAP,

		GROUPBOX_SUPPORTING_POST,
		LABEL_POST_TYPE,
		LABEL_POST_GAP,
		POPUP_POST_TYPE,
		POPUP_POST_GAP,
		EDITCONTROL_POST_GAP,

		LABEL_GAP_FROM_SLAB,
		EDITCONTROL_GAP_FROM_SLAB,
		LABEL_ROOM_HEIGHT,
		EDITCONTROL_ROOM_HEIGHT,

		ICON_LAYER,
		LABEL_LAYER_SETTINGS,
		CHECKBOX_LAYER_COUPLING,
		LABEL_LAYER_EUROFORM,
		LABEL_LAYER_CONPANEL,
		LABEL_LAYER_PLYWOOD,
		LABEL_LAYER_TIMBER,
		LABEL_LAYER_MRK,
		LABEL_LAYER_PINBOLT,
		LABEL_LAYER_FITTINGS,
		LABEL_LAYER_GT24_GIRDER,
		LABEL_LAYER_PERI_SUPPORT,
		LABEL_LAYER_STEEL_SUPPORT,

		USERCONTROL_LAYER_EUROFORM,
		USERCONTROL_LAYER_CONPANEL,
		USERCONTROL_LAYER_PLYWOOD,
		USERCONTROL_LAYER_TIMBER,
		USERCONTROL_LAYER_MRK,
		USERCONTROL_LAYER_PINBOLT,
		USERCONTROL_LAYER_FITTINGS,
		USERCONTROL_LAYER_GT24_GIRDER,
		USERCONTROL_LAYER_PERI_SUPPORT,
		USERCONTROL_LAYER_STEEL_SUPPORT,

		BUTTON_AUTOSET
	};

	// �ǳ� Ÿ��
	enum	idxItems_1_PanelType {
		PANEL_TYPE_NONE,			// ����
		PANEL_TYPE_CONPANEL,		// ���ǳ�
		PANEL_TYPE_PLYWOOD,			// ����
		PANEL_TYPE_EUROFORM			// ������
	};

	// �弱 Ÿ��
	enum	idxItems_1_BeamType {
		BEAM_TYPE_SANSEUNGGAK = 1,	// ��°�
		BEAM_TYPE_TUBAI,			// ������
		BEAM_TYPE_GT24				// GT24 �Ŵ�
	};

	// �ۿ��� Ÿ��
	enum	idxItems_1_GirderType {
		GIRDER_TYPE_GT24 = 1,		// GT24 �Ŵ�
		GIRDER_TYPE_SANSEUNGGAK		// ��°�
	};

	// ���ٸ� Ÿ��
	enum	idxItems_1_PostType {
		POST_TYPE_PERI_SUPPORT = 1,	// PERI ���ٸ�
		POST_TYPE_STEEL_SUPPORT		// ���� ���ٸ�
	};

	// ����
	enum	idxItems_1_Direction {
		HORIZONTAL = 1,				// ����
		VERTICAL					// ����
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
		POPUP_WIDTH,
		LABEL_HEIGHT,
		EDITCONTROL_HEIGHT,
		POPUP_HEIGHT,
		LABEL_ORIENTATION,
		POPUP_ORIENTATION
	};

	enum insulationDialog {
		LABEL_EXPLANATION_INS = 3,
		USERCONTROL_INSULATION_LAYER,
		LABEL_INSULATION_THK,
		EDITCONTROL_INSULATION_THK,
		CHECKBOX_INS_LIMIT_SIZE,
		LABEL_INS_HORLEN,
		EDITCONTROL_INS_HORLEN,
		LABEL_INS_VERLEN,
		EDITCONTROL_INS_VERLEN,
	};
}

// �ܿ���
struct insulElemForSlabTableform
{
	short	layerInd;		// ���̾� �ε���
	double	thk;			// �β�
	bool	bLimitSize;		// ����/���� ũ�� ����
	double	maxHorLen;		// ���� �ִ� ����
	double	maxVerLen;		// ���� �ִ� ����
};

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
	short	objType;		// �ǳ� Ÿ��

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
	double	level;					// ��
	double	ang;					// ȸ�� ���� (����: Radian, ȸ����: Z��)

	short	iPanelType;				// �ǳ� Ÿ�� (1: ���ǳ�, 2: ����, 3: ������)
	short	iPanelDirection;		// �ǳ� ���� (1: ����, 2: ����)
	char	panelThkStr [10];		// �ǳ� �β� (���ڿ� ����, 11.5T ��)
	double	panelThk;				// �ǳ� �β� -- roomHeight�� ����
	
	short	iBeamType;				// �弱 Ÿ�� (1: ��°�, 2: ������, 3: GT24 �Ŵ�)
	short	iBeamDirection;			// �弱 ���� (1: ����, 2: ����)
	double	beamOffsetHorizontal;	// �弱 ������(����)
	double	beamOffsetVertical;		// �弱 ������(����)
	double	beamGap;				// �弱 ����
	double	beamThk;				// �弱 �β� -- roomHeight�� ����

	short	iGirderType;			// �ۿ��� Ÿ�� (1: GT24 �Ŵ�, 2: ��°�)
	short	nGirders;				// �ۿ��� ����
	double	girderOffsetHorizontal;	// �ۿ��� ������(����) - ��ȿ���� ����
	double	girderOffsetVertical;	// �ۿ��� ������(����) - ��ȿ���� ����
	double	girderGap;				// �ۿ��� ����
	double	girderThk;				// �ۿ��� �β� -- roomHeight�� ����

	short	iSuppPostType;			// ���ٸ� Ÿ�� (1: PERI������(PERI���ٸ�), 2: ����Ʈ(�������ٸ�))
	double	suppPostGap;			// ���ٸ� ����
	double	crossHeadThk;			// ũ�ν���� �β� -- roomHeight�� ����
	double	postHeight;				// ���ٸ� ���� -- roomHeight�� ����

	double	gap;					// õ�� ��������� ����
	double	roomHeight;				// õ��-�ٴڰ� �Ÿ�
	bool	bRectangleArea;			// ���簢�� �����̸� true, ���� �𼭸� �����̸� false

	double	borderHorLen;			// �ֿܰ� ���� ����
	double	borderVerLen;			// �ֿܰ� ���� ����

	API_Coord3D		center;			// �ֿܰ� �߽� ��
	API_Coord3D		leftBottom;		// �ֿܰ� ���ϴ� ��
	API_Coord3D		rightTop;		// �ֿܰ� ���� ��

	double	initCellHorLen;			// �� �ʱ� �ʺ�
	double	initCellVerLen;			// �� �ʱ� ����

	double	cellArrayWidth;			// �� �迭 ��ü �ʺ�
	double	cellArrayHeight;		// �� �迭 ��ü ����

	short	nHorCells;				// ���� ���� �� ����
	short	nVerCells;				// ���� ���� �� ����

	double	marginLeft;				// ���� ���� ����
	double	marginRight;			// ������ ���� ����
	double	marginTop;				// ���� ���� ����
	double	marginBottom;			// �Ʒ��� ���� ����

	double	leftBottomX;			// �� ���� ��ǥ X
	double	leftBottomY;			// �� ���� ��ǥ Y
	double	leftBottomZ;			// �� ���� ��ǥ Z

	CellForSlabTableform	cells [50][50];		// �� ���� (0���� �ϴ�, 0���� ����)

public:
	void		initCells (SlabTableformPlacingZone* placingZone);			// Cell �迭�� �ʱ�ȭ��
	void		alignPlacingZone (SlabTableformPlacingZone* placingZone);	// Cell �迭�� ��ġ�� ������
	GSErrCode	fillCellAreas (void);										// �� �迭 ���� ä���
	GSErrCode	fillMarginAreas (void);										// ���� ���� ä��� (����)
	GSErrCode	placeInsulations (void);									// �ܿ��� ��ġ
	GSErrCode	placeBeams (void);											// �弱 ��ġ
	GSErrCode	placeGirdersAndPosts (void);								// �ۿ���, ���ٸ� ��ġ

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
short DGCALLBACK slabBottomTableformPlacerHandler4_Insulation (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// ������ �Ϻ��� ���ݿ� �ܿ��縦 ��ġ��

#endif