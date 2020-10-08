#ifndef	__SLAB_EUROFORM_PLACER__
#define __SLAB_EUROFORM_PLACER__
#endif

#include "MaxBIM.hpp"

// 변수 선언
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

// 유로폼 슬래브 하부 배치 함수
//API_Guid	placeLibPartForSlabBottom (Cell objInfo);		// 해당 셀 정보를 기반으로 라이브러리 배치
GSErrCode	placeEuroformOnSlabBottom (void);				// 2번 메뉴: 슬래브 하부에 유로폼을 배치하는 통합 루틴

// ... WallEuroformPlacer.hpp 의 자료형, 열거형들은 공통으로 쓸 수 있는 것도 있어서 고려해야 할 점이 있다