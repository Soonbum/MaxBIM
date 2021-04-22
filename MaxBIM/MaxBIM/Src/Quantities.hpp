#ifndef	__QUANTITIES__
#define __QUANTITIES__

#include "MaxBIM.hpp"

using namespace std;

namespace quantitiesDG {
	enum	elemType {
		ELEM_WALL,		// ��
		ELEM_SLAB,		// ������
		ELEM_BEAM,		// ��
		ELEM_COLUMN,	// ���
		ELEM_UNKNOWN	// �� �� ����
	};

	enum	qPlywoodType {
		Q_WALL_INNER,		// �� (����)
		Q_WALL_OUTER,		// �� (�ܺ�)
		Q_WALL_COMPOSITE,	// �� (�պ�)
		Q_WALL_PARAPET,		// �� (�Ķ���)
		Q_WALL_WATERPROOF,	// �� (�����)
		Q_SLAB_BASE,		// ������ (����)
		Q_SLAB_RC,			// ������ (RC)
		Q_SLAB_DECK,		// ������ (��ũ)
		Q_SLAB_RAMP,		// ������ (����)
		Q_BEAM,				// ��
		Q_COLUMN_ISOLATED,	// ��� (����)
		Q_COLUMN_INWALL,	// ��� (��ü)
		Q_UNKNOWN			// �� ���� Ÿ���� ������� ����
	};

	enum	sideStr {
		NORTH_SIDE,
		SOUTH_SIDE,
		EAST_SIDE,
		WEST_SIDE,
		BASE_SIDE
	};

	enum	modeStr {
		MODE_X,
		MODE_Y,
		MODE_Z
	};
}

class qElem
{
public:
	API_Guid	guid;			// ����� GUID

	short	typeOfElem;			// ����� Ÿ�� (enum elemType ����)
	short	floorInd;			// �� �ε���
	short	layerInd;			// ���̾� �ε���
	short	qLayerInd;			// �������� ���̾� �ε���
	short	typeOfQPlywood;		// �������� ��ü�� Ÿ�� (enum qPlywoodType ����)
	//double	ang;			// ȸ�� ���� (����� ������� ����)

	bool	hasHole;			// (��, â���� ����) ������ �� �ִ°�?

	API_Coord3D		bottomPoint;	// ���ϴ� �� (���ϴ� �ٴ�)
	API_Coord3D		topPoint;		// �ֻ�� �� (���� �����)

	// ���� ����
	API_Coord3D		NorthLeftBottom;	// ���ϴ�
	API_Coord3D		NorthRightTop;		// ����
	bool			validNorth;			// ��ȿ�� ��� ���������� ������

	// ���� ����
	API_Coord3D		SouthLeftBottom;	// ���ϴ�
	API_Coord3D		SouthRightTop;		// ����
	bool			validSouth;			// ��ȿ�� ��� ���������� ������

	// ���� ����
	API_Coord3D		EastLeftBottom;		// ���ϴ�
	API_Coord3D		EastRightTop;		// ����
	bool			validEast;			// ��ȿ�� ��� ���������� ������

	// ���� ����
	API_Coord3D		WestLeftBottom;		// ���ϴ�
	API_Coord3D		WestRightTop;		// ����
	bool			validWest;			// ��ȿ�� ��� ���������� ������

	// �ظ�
	API_Coord3D		BaseLeftBottom;		// ���ϴ�
	API_Coord3D		BaseRightTop;		// ���ϴ�
	bool			validBase;			// ��ȿ�� ��� ���������� ������

	// ������ �������ǵ��� GUID
	short		nQPlywoods;
	API_Guid	qPlywoodGuids [5];

	// ������ ����Ű�� �ٸ� ��ҵ��� GUID
	short		nOtherElems;
	API_Guid	otherGuids [200];

public:
	short	invalidateShortTwoSide (qElem* element);	// 4���� ���� �� ª�� 2���� ������ ��ȿȭ�ϰ�, ��ȿ�� �� ������ ������. ���� ���� ���� ���� �����Դϴ�. ����/����(2), ����/����(1), ����(0)
	short	invalidateLongTwoSide (qElem* element);		// 4���� ���� �� �� 2���� ������ ��ȿȭ�ϰ�, ��ȿ�� �� ������ ������. ���� ���� ���� ���� �����Դϴ�. ����/����(2), ����/����(1), ����(0)
	void	invalidateAllSide (qElem* element);			// 4���� ������ ��� ��ȿȭ��
	void	invalidateBase (qElem* element);			// �ظ��� ��ȿȭ��
	void	validateAllSide (qElem* element);			// 4���� ������ ��� ��ȿȭ��
	void	validateBase (qElem* element);				// �ظ��� ��ȿȭ��
	bool	subtractArea (qElem* src, qElem operand);	// src ����� ����, �ظ� ������ operand ��ҿ� ���� ħ�� ���� ���, �ָ��� ������ ���� operand�� GUID�� ������
	void	placeQuantityPlywood (qElem* element);		// ����� ������ �ظ� ������ ���������� ������
};

class qElemType
{
public:
	double	areas_wall_in;			// ����: ��ü(����)
	double	areas_wall_out;			// ����: ��ü(�ܺ�)
	double	areas_wall_composite;	// ����: ��ü(�պ�)
	double	areas_wall_parapet;		// ����: ��ü(�Ķ���)
	double	areas_wall_waterproof;	// ����: ��ü(�����)
	double	areas_slab_base;		// ����: �����(����)
	double	areas_slab_rc;			// ����: �����(RC)
	double	areas_slab_deck;		// ����: �����(��ũ)
	double	areas_slab_ramp;		// ����: �����(����)
	double	areas_beam;				// ����: ��
	double	areas_column_iso;		// ����: ���(����)
	double	areas_column_inwall;	// ����: ���(��ü)
	double	areas_column_circle;	// ����: ���(����)
	double	areas_ramp_wall;		// ����: ��������(��)

public:
	void	initAreas (qElemType* elem);	// ���� ���� �ʱ�ȭ
};

GSErrCode	placeQuantityPlywood (void);				// ����(��,������,��,���)���� �����簡 ���� �� �ִ� �鿡 ���������� �ڵ����� ������
short	DGCALLBACK quantityPlywoodUIHandler (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// [���̾�α�] �������� ���� ���̾� ����

GSErrCode	calcAreasOfQuantityPlywood (void);			// ������ ���������� ǥ���� �հ� ���� ����

#endif