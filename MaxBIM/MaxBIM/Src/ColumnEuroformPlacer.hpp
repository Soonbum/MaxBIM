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
}

// ��� ���� ����
struct InfoColumn
{
	bool	bRectangle;		// ���簢�� ���°� �´°�?
	short	coreAnchor;		// �ھ��� ��Ŀ ����Ʈ
	double	coreDepth;		// ����� ����
	double	coreWidth;		// ����� �ʺ�
	double	venThick;		// ��� ���Ͼ� �β�
	double	height;			// ����� ����
	double	bottomOffset;	// �ٴ� ������ ���� ��� ���̽� ����
	double	topOffset;		// ���� ����� ������ ����Ǿ� �ִ� ��� �������κ����� ������
	double	angle;			// ��� ���� �߽����� �� ȸ�� ���� (����: radian)
	API_Coord	origoPos;	// ����� ��ġ
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
	short	anchor;			// ������ ��Ŀ ����Ʈ: �»��(1), ���(2), ����(3), ��(4), ��(5), ���ϴ�(6), �ϴ�(7), ���ϴ�(8)

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
	// ��� ���� ���� !!!
	short	coreAnchor;		// �ھ��� ��Ŀ ����Ʈ
	double	coreDepth;		// ����� ����
	double	coreWidth;		// ����� �ʺ�
	double	venThick;		// ��� ���Ͼ� �β�
	double	height;			// ����� ����
	double	bottomOffset;	// �ٴ� ������ ���� ��� ���̽� ����
	double	topOffset;		// ���� ����� ������ ����Ǿ� �ִ� ��� �������κ����� ������
	double	angle;			// ��� ���� �߽����� �� ȸ�� ���� (����: Radian, ȸ����: Z��)
	API_Coord	origoPos;	// ����� ��ġ
	
	double	areaHeight;			// ���� ����

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

	short	nCellsLT;
	short	nCellsRT;
	short	nCellsLB;
	short	nCellsRB;
	short	nCellsT1;
	short	nCellsT2;
	short	nCellsL1;
	short	nCellsL2;
	short	nCellsR1;
	short	nCellsR2;
	short	nCellsB1;
	short	nCellsB2;

	double			gap;				// ��հ��� ����
};

// ������ ��� ��ġ �Լ�
GSErrCode	placeEuroformOnColumn (void);			// 5�� �޴�: ��տ� �������� ��ġ�ϴ� ���� ��ƾ

#endif
