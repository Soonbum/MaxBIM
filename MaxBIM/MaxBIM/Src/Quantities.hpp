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
}

struct qElem
{
	API_Guid	guid;			// ����� GUID

	short	typeOfElem;			// ����� Ÿ�� (enum elemType ����)
	short	floorInd;			// �� �ε���
	short	layerInd;			// ���̾� �ε���
	short	qLayerInd;			// �������� ���̾� �ε���
	short	typeOfQPlywood;		// �������� ��ü�� Ÿ�� (enum qPlywoodType ����)
	//double	ang;			// ȸ�� ���� (����� ������� ����)

	API_Coord3D		bottomPoint;	// ���ϴ� �� (���ϴ� �ٴ�)
	API_Coord3D		topPoint;		// �ֻ�� �� (���� �����)

	// ���� ����
	API_Coord3D		NorthLeftBottom;	// ���ϴ�
	API_Coord3D		NorthRightTop;		// ����

	// ���� ����
	API_Coord3D		SouthLeftBottom;	// ���ϴ�
	API_Coord3D		SouthRightTop;		// ����

	// ���� ����
	API_Coord3D		EastLeftBottom;		// ���ϴ�
	API_Coord3D		EastRightTop;		// ����

	// ���� ����
	API_Coord3D		WestLeftBottom;		// ���ϴ�
	API_Coord3D		WestRightTop;		// ����

	// �ظ�
	API_Coord3D		BaseLeftBottom;		// ���ϴ�
	API_Coord3D		BaseRightTop;		// ���ϴ�
};

GSErrCode	placeQuantityPlywood (void);			// ����(��,������,��,���)���� �����簡 ���� �� �ִ� �鿡 ���������� �ڵ����� ������
short DGCALLBACK quantityPlywoodUIHandler (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// [���̾�α�] �������� ���� ���̾� ����
short findLayerIndex (const char* layerName);		// ���̾� �̸����� ���̾� �ε��� ã��

#endif