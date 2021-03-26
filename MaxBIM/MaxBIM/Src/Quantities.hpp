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
		ELEM_UNKNOWN	// �� ���� Ÿ���� ������� ����
	};
}

struct qElem
{
	API_Guid	guid;		// ����� GUID

	short	typeOfElem;		// ����� Ÿ�� (enum elemType ����)
	double	ang;			// ȸ�� ���� (����� ������� ����)

	API_Coord3D		bottomPoint;	// ���ϴ� ��
	API_Coord3D		topPoint;		// �ֻ�� ��
};

GSErrCode	placeQuantityPlywood (void);	// ����(��,������,��,���)���� �����簡 ���� �� �ִ� �鿡 ���������� �ڵ����� ������
short DGCALLBACK quantityPlywoodUIHandler (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// [���̾�α�] �������� ���� ���̾� ����

#endif