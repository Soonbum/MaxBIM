#ifndef	__COLUMN_EUROFORM_PLACER__
#define __COLUMN_EUROFORM_PLACER__

#include "MaxBIM.hpp"

namespace columnPlacerDG {
	// ��ü Ÿ��
	enum	libPartObjType {
		NONE,			// ����
		EUROFORM,		// ������v2.0
		INCORNER,		// ���ڳ��ǳ�v1.0
		OUTCORNER,		// �ƿ��ڳ��ǳ�v1.0
		PLYWOOD,		// ����v1.0
		MAGIC_BAR,		// ������v1.0
		MAGIC_INCORNER	// �������ڳ�v1.0
	};

	// ���簡 �����Ǵ� ���� �ش��ϴ� ����� ��Ŀ ����Ʈ
	enum	anchorPoint {
		LEFT_TOP,		// �»��
		RIGHT_TOP,		// ����
		LEFT_BOTTOM,	// ���ϴ�
		RIGHT_BOTTOM,	// ���ϴ�
		TOP,			// ���
		LEFT,			// ����
		RIGHT,			// ����
		BOTTOM			// �ϴ�
	};

	// ����� �߽����� ���� ��, ���� �ִ� ����
	enum	direction {
		NORTH,
		SOUTH,
		EAST,
		WEST
	};

	// ���̾�α� �׸� �ε���
	enum	idxItems_1_forColumnPlacer {
		LABEL_COLUMN_SECTION		= 3,
		ICON_COLUMN_SECTION,
		
		EDITCONTROL_TOP_1,
		EDITCONTROL_TOP_2,
		EDITCONTROL_TOP_3,
		EDITCONTROL_TOP_4,
		EDITCONTROL_LEFT_1,
		EDITCONTROL_LEFT_2,
		EDITCONTROL_LEFT_3,
		EDITCONTROL_LEFT_4,
		EDITCONTROL_RIGHT_1,
		EDITCONTROL_RIGHT_2,
		EDITCONTROL_RIGHT_3,
		EDITCONTROL_RIGHT_4,
		EDITCONTROL_BOTTOM_1,
		EDITCONTROL_BOTTOM_2,
		EDITCONTROL_BOTTOM_3,
		EDITCONTROL_BOTTOM_4,

		CHECKBOX_TOP_ADDITIONAL_FORM,
		CHECKBOX_LEFT_ADDITIONAL_FORM,
		CHECKBOX_RIGHT_ADDITIONAL_FORM,
		CHECKBOX_BOTTOM_ADDITIONAL_FORM,

		LABEL_COLUMN_DEPTH,
		LABEL_COLUMN_WIDTH,
		EDITCONTROL_COLUMN_WIDTH,
		EDITCONTROL_COLUMN_DEPTH,

		ICON_LAYER,
		LABEL_LAYER_SETTINGS,
		LABEL_LAYER_EUROFORM,
		LABEL_LAYER_OUTCORNER,
		LABEL_LAYER_PLYWOOD,

		USERCONTROL_LAYER_EUROFORM,
		USERCONTROL_LAYER_OUTCORNER,
		USERCONTROL_LAYER_PLYWOOD,
	};

	enum	idxItems_2_forColumnPlacer {
		DG_UPDATE_BUTTON	= 3,
		DG_PREV,
		LABEL_COLUMN_SIDE,
		AFTER_ALL
	};
	
	enum	idxItems_3_forColumnPlacer {
		LABEL_OBJ_TYPE			= 3,
		POPUP_OBJ_TYPE,
		CHECKBOX_SET_STANDARD,
		LABEL_LENGTH,
		EDITCONTROL_LENGTH,
		POPUP_LENGTH
	};

	enum	idxItems_1_forWallColumnPlacer {
		LABEL_COLUMN_SECTION_WC	= 3,

		ICON_COLUMN_SECTION_01_WC,
		ICON_COLUMN_SECTION_02_WC,
		ICON_COLUMN_SECTION_03_WC,
		ICON_COLUMN_SECTION_04_WC,
		ICON_COLUMN_SECTION_05_WC,
		ICON_COLUMN_SECTION_06_WC,
		ICON_COLUMN_SECTION_07_WC,

		ICON_COLUMN_SECTION_08_WC,
		ICON_COLUMN_SECTION_09_WC,
		ICON_COLUMN_SECTION_10_WC,
		ICON_COLUMN_SECTION_11_WC,
		ICON_COLUMN_SECTION_12_WC,
		ICON_COLUMN_SECTION_13_WC,
		ICON_COLUMN_SECTION_14_WC,

		ICON_LAYER_WC,
		LABEL_LAYER_SETTINGS_WC,
		LABEL_LAYER_EUROFORM_WC,
		LABEL_LAYER_INCORNER_WC,
		LABEL_LAYER_OUTCORNER_WC,
		LABEL_LAYER_PLYWOOD_WC,

		USERCONTROL_LAYER_EUROFORM_WC,
		USERCONTROL_LAYER_INCORNER_WC,
		USERCONTROL_LAYER_OUTCORNER_WC,
		USERCONTROL_LAYER_PLYWOOD_WC,

		AFTER_ALL_WC
	};
}

// �� ���� ����
struct InfoWallForColumn
{
	API_Guid	guid;			// ���� GUID

	double	wallThk;			// �� �β�
	short	floorInd;			// �� �ε���
	double	bottomOffset;		// �� �ϴ� ������
	double	angle;				// ȸ�� ���� (����: Radian)

	long	nCoords;			// ���� ����
	API_Coord	poly [10];		// ���� ��ǥ
	
	API_Coord	begC;			// ���۷��� ���� ������
	API_Coord	endC;			// ���۷��� ���� ����

	API_Coord	begC_1;			// ���� 1 ������
	API_Coord	endC_1;			// ���� 1 ����
	API_Coord	begC_2;			// ���� 2 ������
	API_Coord	endC_2;			// ���� 2 ����
};

// �� ���� ����
struct InfoBeamForColumn
{
	API_Guid	guid;	// ���� GUID
	bool	valid;		// ������ ��ȿ��
	short	floorInd;	// �� �ε���

	double	height;		// �� ����
	double	width;		// �� �ʺ�
	double	offset;		// �� �߽����κ��� ���� ���۷��� ������ �������Դϴ�.
	double	level;		// �ٴ� ������ ���� ���� ���ʸ� �����Դϴ�.

	API_Coord	begC;	// �� ���� ��ǥ
	API_Coord	endC;	// �� �� ��ǥ
};

// ���� ���� ����
struct InfoMorphForColumn
{
	API_Guid	guid;		// ������ GUID
	short		floorInd;	// �� �ε���
	double		level;		// ������ ��
	double		height;		// ������ ����
};

// �׸��� �� �� ����
struct CellForColumn
{
	short		objType;	// enum libPartObjType ����

	API_Guid	guid;		// ��ü�� GUID

	double	leftBottomX;	// ���ϴ� ��ǥ X
	double	leftBottomY;	// ���ϴ� ��ǥ Y
	double	leftBottomZ;	// ���ϴ� ��ǥ Z
	double	ang;			// ȸ�� ���� (����: Radian, ȸ����: Z��)

	double	horLen;			// ���� ����
	double	verLen;			// ���� ����
	double	height;			// ����

	union {
		Euroform		form;
		IncornerPanel	incorner;
		OutcornerPanel	outcorner;
		Plywood			plywood;
		MagicBar		mbar;
		MagicIncorner	mincorner;
	} libPart;
};

// ��� ���� ����
class ColumnPlacingZone
{
public:
	// ��� ���� ����
	bool	bRectangle;		// ���簢���ΰ�?
	short	coreAnchor;		// �ھ��� ��Ŀ ����Ʈ
	double	coreWidth;		// ����� �ʺ� (X ����)
	double	coreDepth;		// ����� ���� (Y ����)
	double	venThick;		// ��� ���Ͼ� �β�
	double	height;			// ����� ����
	double	bottomOffset;	// �ٴ� ������ ���� ��� ���̽� ����
	double	topOffset;		// ���� ����� ������ ����Ǿ� �ִ� ��� �������κ����� ������
	double	angle;			// ��� ���� �߽����� �� ȸ�� ���� (����: Radian, ȸ����: Z��)
	API_Coord	origoPos;	// ����� ��ġ

	double	areaHeight;		// ������ ������ ������ ����

	// ���� �� ���� ����
	bool	bInterfereBeam;				// ���� �� ����
	short	nInterfereBeams;			// ���� �� ���� (0~4��)
	bool	bExistBeams [4];			// ���� �� ����
	double	bottomLevelOfBeams [4];		// ���� ���� �ϴ� ����
	InfoBeamForColumn	beams [4];		// ���� �� ����

	// ���� ������ ���� �и��� ����/���� ����
	double	hLenDown, vLenDown;			// ���� �Ʒ���/���� ����/���� ����
	double	hLenUp, vLenUp;				// ���� ����/������ ����/���� ����

	// ���� �� ���� ����
	bool	bWallHorizontalDirected;	// ���� ���� (true: ����, false: ����)
	double	posTopWallLine;				// ����(������) �� ������ ����(����) ��ġ
	double	posBottomWallLine;			// �Ʒ���(����) �� ������ ����(����) ��ġ
	double	posTopColumnLine;			// ����(������) ��� ������ ����(����) ��ġ
	double	posBottomColumnLine;		// �Ʒ���(����) ��� ������ ����(����) ��ġ
	short	relationCase;				// ���� ��հ��� ����
	/*
	1. ������ ��� (������)
		CASE 1. ��� �ٷ� ���� ���� ���� ���:			top wall > bottom wall == top column > bottom column
		CASE 2. ����� �� �Ʒ��� ���� �� ���:		top wall > top column > bottom wall > bottom column
		CASE 3. ��� ������ �� ���ʰ� ��ġ�� ���:		top wall == top column > bottom wall > bottom column
		CASE 4. ����� ���� ��Ų ���:					top column > top wall > bottom wall > bottom column
		CASE 5. ��� �Ʒ����� �� �Ʒ��ʰ� ��ġ�� ���:	top column > top wall > bottom wall == bottom column
		CASE 6. ����� �� ���� ���� �� ���:		top column > top wall > bottom column > bottom wall
		CASE 7. ��� �ٷ� �Ʒ��� ���� ���� ���:		top column > bottom column == top wall > bottom wall
	2. ������ ��� (�����ʺ���)
		CASE 1. ��� �ٷ� �����ʿ� ���� ���� ���:		top wall > bottom wall == top column > bottom column
		CASE 2. ����� �� ���ʿ� ���� �� ���:		top wall > top column > bottom wall > bottom column
		CASE 3. ��� �������� �� �����ʰ� ��ġ�� ���:	top wall == top column > bottom wall > bottom column
		CASE 4. ����� ���� ��Ų ���:					top column > top wall > bottom wall > bottom column
		CASE 5. ��� ������ �� ���ʰ� ��ġ�� ���:		top column > top wall > bottom wall == bottom column
		CASE 6. ����� �� �����ʿ� ���� �� ���:	top column > top wall > bottom column > bottom wall
		CASE 7. ��� �ٷ� ���ʿ� ���� ���� ���:		top column > bottom column == top wall > bottom wall
	*/

	// ��� ���� ����
	double	marginTopAtNorth;			// ��� ���� ���� ���� ����
	double	marginTopAtWest;			// ��� ���� ���� ���� ����
	double	marginTopAtEast;			// ��� ���� ���� ���� ����
	double	marginTopAtSouth;			// ��� ���� ���� ���� ����

	// ��� ���� ������ ä���� ����
	bool	bFillMarginTopAtNorth;		// ��� ���� ���� ���� ���� ä��
	bool	bFillMarginTopAtWest;		// ��� ���� ���� ���� ���� ä��
	bool	bFillMarginTopAtEast;		// ��� ���� ���� ���� ���� ä��
	bool	bFillMarginTopAtSouth;		// ��� ���� ���� ���� ���� ä��

	// �� ����
	CellForColumn	cellsLT [20];		// �»�� ��
	CellForColumn	cellsRT [20];		// ���� ��
	CellForColumn	cellsLB [20];		// ���ϴ� ��
	CellForColumn	cellsRB [20];		// ���ϴ� ��
	CellForColumn	cellsT1 [20];		// ���1 �� (����)
	CellForColumn	cellsT2 [20];		// ���2 �� (������)
	CellForColumn	cellsL1 [20];		// ����1 �� (��)
	CellForColumn	cellsL2 [20];		// ����2 �� (�Ʒ�)
	CellForColumn	cellsR1 [20];		// ����1 �� (��)
	CellForColumn	cellsR2 [20];		// ����2 �� (�Ʒ�)
	CellForColumn	cellsB1 [20];		// �ϴ�1 �� (����)
	CellForColumn	cellsB2 [20];		// �ϴ�2 �� (������)

	CellForColumn	cellsLin1 [20];		// ���� ���ڳ� �� 1 (��)
	CellForColumn	cellsLin2 [20];		// ���� ���ڳ� �� 2 (�Ʒ�)
	CellForColumn	cellsRin1 [20];		// ������ ���ڳ� �� 1 (��)
	CellForColumn	cellsRin2 [20];		// ������ ���ڳ� �� 2 (�Ʒ�)

	CellForColumn	cellsW1 [20];		// �� �ݴ��� �� 1
	CellForColumn	cellsW2 [20];		// �� �ݴ��� �� 2
	CellForColumn	cellsW3 [20];		// �� �ݴ��� �� 3
	CellForColumn	cellsW4 [20];		// �� �ݴ��� �� 4

	// ���� ���������� �� ����
	short	nCells;

public:
	void		initCells (ColumnPlacingZone* placingZone);						// Cell �迭�� �ʱ�ȭ��
	void		addTopCell (ColumnPlacingZone* target_zone);					// ����⿡ �� �߰�
	void		delTopCell (ColumnPlacingZone* target_zone);					// ������� �� ����
	void		alignPlacingZone_soleColumn (ColumnPlacingZone* placingZone);	// Cell ������ ����ʿ� ���� ����ȭ�� ��ġ�� ��������
	void		alignPlacingZone_wallColumn (ColumnPlacingZone* placingZone);	// Cell ������ ����ʿ� ���� ����ȭ�� ��ġ�� ��������
	API_Guid	placeLibPart (CellForColumn objInfo);							// �ش� �� ������ ������� ���̺귯�� ��ġ
	GSErrCode	fillRestAreas_soleColumn (ColumnPlacingZone* placingZone);		// ������/�ƿ��ڳ��ǳ��� ä�� �� ������ ���� ä��� (�������� �������� ä��)
	GSErrCode	fillRestAreas_wallColumn (ColumnPlacingZone* placingZone);		// ������/�ƿ��ڳ��ǳ��� ä�� �� ������ ���� ä��� (�������� �������� ä��)
};

// ������ ��� ��ġ �Լ�
GSErrCode	placeEuroformOnColumn (void);	// ��տ� �������� ��ġ�ϴ� ���� ��ƾ
short DGCALLBACK columnPlacerHandler_soleColumn_1 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 1�� ��ġ�� ���� ���Ǹ� ��û�ϴ� 1�� ���̾�α�
short DGCALLBACK columnPlacerHandler_soleColumn_2 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 1�� ��ġ �� ������ ��û�ϴ� 2�� ���̾�α�
short DGCALLBACK columnPlacerHandler_soleColumn_3 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 2�� ���̾�α׿��� �� ���� ��ü Ÿ���� �����ϱ� ���� 3�� ���̾�α�
short DGCALLBACK columnPlacerHandler_wallColumn_1 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 1�� ��ġ�� ���� ���Ǹ� ��û�ϴ� 1�� ���̾�α�
short DGCALLBACK columnPlacerHandler_wallColumn_2 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 1�� ��ġ �� ������ ��û�ϴ� 2�� ���̾�α�
short DGCALLBACK columnPlacerHandler_wallColumn_3 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 2�� ���̾�α׿��� �� ���� ��ü Ÿ���� �����ϱ� ���� 3�� ���̾�α�

#endif
