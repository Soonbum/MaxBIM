#ifndef	__COLUMN_TABLEFORM_PLACER__
#define __COLUMN_TABLEFORM_PLACER__

#include "MaxBIM.hpp"

namespace columnTableformPlacerDG {
	// ��ü Ÿ��
	enum	libPartObjType {
		NONE,			// ����
		EUROFORM,		// ������v2.0
		OUTPANEL,		// �ƿ��ڳ��ǳ�v1.0
		OUTANGLE,		// �ƿ��ڳʾޱ�v1.0
		SQUARE_PIPE,	// ���������v1.0
		PINBOLT,		// �ɺ�Ʈ��Ʈv1.0
		HANGER,			// �����������
		HEAD,			// ������Ʈ�� Push-Pull Props ����ǽ� v1.0
		PLYWOOD			// ����v1.0
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

	// ���̾�α� �׸� �ε��� !!!
	enum	idxItems_1_forColumnTableformPlacer {
		LABEL_COLUMN_SECTION		= 3,

		ICON_COLUMN_SECTION_OUTCORNER_PANEL,
		ICON_COLUMN_SECTION_OUTCORNER_ANGLE,

		RADIO_OUTCORNER_PANEL,
		RADIO_OUTCORNER_ANGLE,
		
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
		CHECKBOX_LAYER_COUPLING,
		LABEL_LAYER_EUROFORM,
		LABEL_LAYER_OUTCORNER_PANEL,
		LABEL_LAYER_OUTCORNER_ANGLE,
		LABEL_LAYER_SQUARE_PIPE,
		LABEL_LAYER_PINBOLT,
		LABEL_LAYER_HANGER,
		LABEL_LAYER_HEADPIECE,
		LABEL_LAYER_PLYWOOD,

		USERCONTROL_LAYER_EUROFORM,
		USERCONTROL_LAYER_OUTCORNER_PANEL,
		USERCONTROL_LAYER_OUTCORNER_ANGLE,
		USERCONTROL_LAYER_SQUARE_PIPE,
		USERCONTROL_LAYER_PINBOLT,
		USERCONTROL_LAYER_HANGER,
		USERCONTROL_LAYER_HEADPIECE,
		USERCONTROL_LAYER_PLYWOOD,

		BUTTON_AUTOSET
	};

	enum	idxItems_2_forColumnTableformPlacer {
		DG_UPDATE_BUTTON	= 3,
		DG_PREV,
		LABEL_COLUMN_SIDE,
		AFTER_ALL
	};
	
	enum	idxItems_3_forColumnTableformPlacer {
		LABEL_OBJ_TYPE			= 3,
		POPUP_OBJ_TYPE,
		CHECKBOX_SET_STANDARD,
		LABEL_LENGTH,
		EDITCONTROL_LENGTH,
		POPUP_LENGTH
	};
}

// �� ���� ����
struct InfoBeamForColumnTableform
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
struct InfoMorphForColumnTableform
{
	API_Guid	guid;		// ������ GUID
	short		floorInd;	// �� �ε���
	double		level;		// ������ ��
	double		height;		// ������ ����
};

// �׸��� �� �� ����
struct CellForColumnTableform
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
		OutcornerAngle	outangle;
		SquarePipe		sqrPipe;
		PinBoltSet		pinbolt;
		HeadpieceOfPushPullProps	head;
	} libPart;
};

// ��� ���� ����
class ColumnTableformPlacingZone
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
	InfoBeamForColumnTableform	beams [4];		// ���� �� ����

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
	CellForColumnTableform	cellsLT [20];		// �»�� ��
	CellForColumnTableform	cellsRT [20];		// ���� ��
	CellForColumnTableform	cellsLB [20];		// ���ϴ� ��
	CellForColumnTableform	cellsRB [20];		// ���ϴ� ��
	CellForColumnTableform	cellsT1 [20];		// ���1 �� (����)
	CellForColumnTableform	cellsT2 [20];		// ���2 �� (������)
	CellForColumnTableform	cellsL1 [20];		// ����1 �� (��)
	CellForColumnTableform	cellsL2 [20];		// ����2 �� (�Ʒ�)
	CellForColumnTableform	cellsR1 [20];		// ����1 �� (��)
	CellForColumnTableform	cellsR2 [20];		// ����2 �� (�Ʒ�)
	CellForColumnTableform	cellsB1 [20];		// �ϴ�1 �� (����)
	CellForColumnTableform	cellsB2 [20];		// �ϴ�2 �� (������)

	// ���� ���������� �� ����
	short	nCells;

public:
	void		initCells (ColumnTableformPlacingZone* placingZone);					// Cell �迭�� �ʱ�ȭ��
	void		addTopCell (ColumnTableformPlacingZone* target_zone);					// ����⿡ �� �߰�
	void		delTopCell (ColumnTableformPlacingZone* target_zone);					// ������� �� ����
	void		alignPlacingZone_soleColumn (ColumnTableformPlacingZone* placingZone);	// Cell ������ ����ʿ� ���� ����ȭ�� ��ġ�� ��������
	API_Guid	placeLibPart (CellForColumnTableform objInfo);							// �ش� �� ������ ������� ���̺귯�� ��ġ
	GSErrCode	fillRestAreas_soleColumn (ColumnTableformPlacingZone* placingZone);		// ������/�ƿ��ڳ��ǳ��� ä�� �� ������ ���� ä��� (�������� �������� ä��)
};

// ������ ��� ��ġ �Լ�
GSErrCode	placeTableformOnColumn (void);	// ��տ� ���̺����� ��ġ�ϴ� ���� ��ƾ
short DGCALLBACK columnTableformPlacerHandler_soleColumn_1 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// 1�� ��ġ�� ���� ���Ǹ� ��û�ϴ� 1�� ���̾�α�
short DGCALLBACK columnTableformPlacerHandler_soleColumn_2 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// 1�� ��ġ �� ������ ��û�ϴ� 2�� ���̾�α�
short DGCALLBACK columnTableformPlacerHandler_soleColumn_3 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// 2�� ���̾�α׿��� �� ���� ��ü Ÿ���� �����ϱ� ���� 3�� ���̾�α�

#endif
