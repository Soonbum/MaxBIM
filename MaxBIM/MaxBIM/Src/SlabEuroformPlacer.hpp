#ifndef	__SLAB_EUROFORM_PLACER__
#define __SLAB_EUROFORM_PLACER__

#include "MaxBIM.hpp"

namespace slabBottomPlacerDG {
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

		ICON_LAYER,
		LABEL_LAYER_SETTINGS,
		LABEL_LAYER_EUROFORM,
		LABEL_LAYER_PLYWOOD,
		LABEL_LAYER_WOOD,

		USERCONTROL_LAYER_EUROFORM,
		USERCONTROL_LAYER_PLYWOOD,
		USERCONTROL_LAYER_WOOD
	};
}

// ���� ���� ����
struct InfoMorphForSlab
{
	API_Guid	guid;		// ������ GUID

	double	leftBottomX;	// ���ϴ� ��ǥ X
	double	leftBottomY;	// ���ϴ� ��ǥ Y
	double	leftBottomZ;	// ���ϴ� ��ǥ Z

	//double	rightTopX;		// ���� ��ǥ X
	//double	rightTopY;		// ���� ��ǥ Y
	//double	rightTopZ;		// ���� ��ǥ Z

	double	horLen;			// ���� ����
	double	verLen;			// ���� ����
	double	ang;			// ȸ�� ���� (����: Degree, ȸ����: Z��)

	// ...
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
struct SlabPlacingZone
{
	/*
	double	horLen;			// ���� ����
	double	verLen;			// ���� ����
	*/
	double	level;			// ��
	double	ang;			// ȸ�� ���� (����: Radian, ȸ����: Z��)

	// �ֿܰ� ��ǥ
	double	outerLeft;		// X ��ǥ
	double	outerRight;		// X ��ǥ
	double	outerTop;		// Y ��ǥ
	double	outerBottom;	// Y ��ǥ

	// ���� �κ� �ڳ� ��ǥ (������ NULL)
	API_Coord3D		*corner_leftTop;
	API_Coord3D		*corner_leftBottom;
	API_Coord3D		*corner_rightTop;
	API_Coord3D		*corner_rightBottom;

	// ������ ���� (1. �⺻ ä���)
	double	remain_hor;				// ���� ���� ���� ����
	double	remain_hor_updated;		// ���� ���� ���� ���� (������Ʈ ��)
	double	remain_ver;				// ���� ���� ���� ����
	double	remain_ver_updated;		// ���� ���� ���� ���� (������Ʈ ��)

	std::string		eu_wid;			// ������ �ʺ�
	std::string		eu_hei;			// ������ ����
	std::string		eu_ori;			// ������ ����
	double	eu_wid_numeric;			// ������ �ʺ� (�Ǽ���)
	double	eu_hei_numeric;			// ������ ���� (�Ǽ���)
	short	eu_count_hor;			// ���� ���� ������ ����
	short	eu_count_ver;			// ���� ���� ������ ����

	// ������ ���� (2. ��ġ�� ��ü ������ �׸���� ����)
	CellForSlab	cells [50][50];		// ������ �ε���: [eu_count_ver-1][eu_count_hor-1]
};

// ������ ������ �Ϻ� ��ġ �Լ�
GSErrCode	placeEuroformOnSlabBottom (void);				// 2�� �޴�: ������ �Ϻο� �������� ��ġ�ϴ� ���� ��ƾ
bool		isSamePoint (API_Coord3D aPoint, API_Coord3D bPoint);												// aPoint�� bPoint�� ���� ������ Ȯ����
bool		isAlreadyStored (API_Coord3D aPoint, API_Coord3D pointList [], short startInd, short endInd);		// aPoint�� pointList�� ������ �Ǿ����� Ȯ����
bool		isNextPoint (API_Coord3D prevPoint, API_Coord3D curPoint, API_Coord3D nextPoint);					// nextPoint�� curPoint�� ���� ���Դϱ�?
short		moreCloserPoint (API_Coord3D curPoint, API_Coord3D p1, API_Coord3D p2);								// curPoint�� ����� ���� p1, p2 �� � ���Դϱ�?
short DGCALLBACK slabBottomPlacerHandler1 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// 1�� ��ġ�� ���� ���Ǹ� ��û�ϴ� 1�� ���̾�α�

#endif
