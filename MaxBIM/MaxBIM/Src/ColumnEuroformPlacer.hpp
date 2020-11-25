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
		RIGHT_BOTTOM	// ���ϴ�
	};
}

// ��� ���� ����
struct InfoColumn
{
	API_Guid	guid;		// ����� GUID
	short	floorInd;		// �� �ε���

	bool	bRectangle;		// ���簢�� ���°� �´°�?
	short	coreAnchor;		// �ھ��� ��Ŀ ����Ʈ
	double	coreWidth;		// ����� X ����
	double	coreDepth;		// ����� Y ����
	double	venThick;		// ��� ���Ͼ� �β�
	double	height;			// ����� ����
	double	bottomOffset;	// �ٴ� ������ ���� ��� ���̽� ����
	double	topOffset;		// ���� ����� ������ ����Ǿ� �ִ� ��� �������κ����� ������
	double	angle;			// ��� ���� �߽����� �� ȸ�� ���� (����: Radian)
	API_Coord	origoPos;	// ����� ��ġ
};

// �� ���� ����
struct InfoWallForColumn
{
	API_Guid	guid;			// ���� GUID

	double	wallThk;			// �� �β�
	short	floorInd;			// �� �ε���
	double	bottomOffset;		// �� �ϴ� ������
	double	offset;				// ���������� ���۷��� �������κ��� ���� ���� ������ ������
	double	angle;				// ȸ�� ���� (����: Radian)
	
	double								offsetFromOutside;		// ���� ���۷��� ���ΰ� ���� �ٱ��� �� ���� �Ÿ�
	API_WallReferenceLineLocationID     referenceLineLocation;	// ���۷��� ������ ��ġ
	/*
		APIWallRefLine_Outside (0)
			���۷��� ���� ��ġ�� ���� �ܺ� �� �� �ֽ��ϴ�.
		APIWallRefLine_Center (1)
			���۷��� ���� ��ġ�� ���� �߾ӿ� �ֽ��ϴ�.
		APIWallRefLine_Inside (2)
			���۷��� ���� ��ġ�� ���� ���� �� �� �ֽ��ϴ�.
		APIWallRefLine_CoreOutside (3)
			���۷��� ���� ��ġ�� ���� ������ �ھ� �ܺ� ��Ų �� �ֽ��ϴ�.
		APIWallRefLine_CoreCenter (4)
			���۷��� ���� ��ġ�� ���� ������ �ھ� ��Ų�� �߾ӿ� �ֽ��ϴ�.
		APIWallRefLine_CoreInside (5)
			���۷��� ���� ��ġ�� ���� ������ �ھ� ���� ��Ų �� �ֽ��ϴ�.
	 */

	double	begX;				// ������ X
	double	begY;				// ������ Y
	double	endX;				// ���� X
	double	endY;				// ���� Y
};

// �� ���� ����
struct InfoBeamForColumn
{
	API_Guid	guid;	// ���� GUID
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
	short	anchor;			// ������ ��Ŀ ����Ʈ

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
struct ColumnPlacingZone
{
	// ��� ���� ����
	bool	bRectangle;		// ���簢���ΰ�?
	short	coreAnchor;		// �ھ��� ��Ŀ ����Ʈ
	double	coreDepth;		// ����� ����
	double	coreWidth;		// ����� �ʺ�
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
	double	bottomLevelOfBeams [4];		// ���� ���� �ϴ� ����

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

	// ���� ���������� �� ����
	short	nCells;
};

// ������ ��� ��ġ �Լ�
GSErrCode	placeEuroformOnColumn (void);						// 5�� �޴�: ��տ� �������� ��ġ�ϴ� ���� ��ƾ
void	initCellsForColumn (ColumnPlacingZone* placingZone);	// Cell �迭�� �ʱ�ȭ��
short DGCALLBACK columnPlacerHandler1 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 1�� ��ġ�� ���� ���Ǹ� ��û�ϴ� 1�� ���̾�α�

#endif
