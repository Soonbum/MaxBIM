#ifndef	__SLAB_EUROFORM_PLACER__
#define __SLAB_EUROFORM_PLACER__
#endif

#include "MaxBIM.hpp"

// ���� ����
//enum	idxItems_1;
//enum	idxItems_2;
//enum	idxItems_3;
//enum	libPartObjType;
//struct	InfoWall;
//struct	InfoMorph;
//struct	InterfereBeam;
//struct	Euroform;
//struct	FillerSpacer;
//struct	IncornerPanel;
//struct	Plywood;
//struct	Wood;
//struct	Cell;
//struct	PlacingZone;
//class	WallPlacingZone;

// ������ ������ �Ϻ� ��ġ �Լ�
//API_Guid	placeLibPartForSlabBottom (Cell objInfo);		// �ش� �� ������ ������� ���̺귯�� ��ġ
GSErrCode	placeEuroformOnSlabBottom (void);				// 2�� �޴�: ������ �Ϻο� �������� ��ġ�ϴ� ���� ��ƾ

// ... WallEuroformPlacer.hpp �� �ڷ���, ���������� �������� �� �� �ִ� �͵� �־ ����ؾ� �� ���� �ִ�