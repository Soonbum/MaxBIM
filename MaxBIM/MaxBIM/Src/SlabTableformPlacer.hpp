#ifndef	__SLAB_TABLEFORM_PLACER__
#define __SLAB_TABLEFORM_PLACER__

#include "MaxBIM.hpp"

namespace slabTableformPlacerDG {
}

// ���� ���� ����
struct InfoMorphForSlabTableform
{
	API_Guid	guid;		// ������ GUID
	short		floorInd;	// �� �ε���
	double		level;		// ������ ��
};





GSErrCode	placeTableformOnSlabBottom (void);											// ������ �Ϻο� ���̺����� ��ġ�ϴ� ���� ��ƾ

#endif