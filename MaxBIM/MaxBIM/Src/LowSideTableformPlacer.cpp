#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "LowSideTableformPlacer.hpp"

using namespace lowSideTableformPlacerDG;

static LowSideTableformPlacingZone	placingZone;		// 낮은 슬래브 측면 영역 정보

static InfoWall						infoWall;			// 벽 객체 정보
static InfoBeam						infoBeam;			// 보 객체 정보
static InfoSlab						infoSlab;			// 슬래브 객체 정보
static InfoMesh						infoMesh;			// 메시 객체 정보

API_Guid		structuralObject_forTableformLowSide;	// 구조 객체의 GUID

//static short	layerInd_Euroform;			// 레이어 번호: 유로폼 (공통)
//static short	layerInd_RectPipe;			// 레이어 번호: 비계 파이프 (공통)
//static short	layerInd_PinBolt;			// 레이어 번호: 핀볼트 세트
//static short	layerInd_WallTie;			// 레이어 번호: 빅체 타이 (더 이상 사용하지 않음)
//static short	layerInd_Clamp;				// 레이어 번호: 직교 클램프 (더 이상 사용하지 않음)
//static short	layerInd_HeadPiece;			// 레이어 번호: 헤드피스
//static short	layerInd_Join;				// 레이어 번호: 결합철물
//static short	layerInd_Plywood;			// 레이어 번호: 합판 (공통)
//static short	layerInd_Timber;			// 레이어 번호: 각재 (공통)
//static short	layerInd_EuroformHook;		// 레이어 번호: 유로폼 후크
//static short	layerInd_CrossJointBar;		// 레이어 번호: 십자 조인트 바
//static short	layerInd_BlueClamp;			// 레이어 번호: 블루 클램프
//static short	layerInd_BlueTimberRail;	// 레이어 번호: 블루 목심
//static short	layerInd_Hidden;			// 레이어 번호: 숨김 (더 이상 사용하지 않음)
//
//static short	layerInd_SlabTableform;		// 레이어 번호: 슬래브 테이블폼
//static short	layerInd_Profile;			// 레이어 번호: KS프로파일
//static short	layerInd_Steelform;			// 레이어 번호: 스틸폼
//static short	layerInd_Fillersp;			// 레이어 번호: 휠러스페이서
//static short	layerInd_OutcornerAngle;	// 레이어 번호: 아웃코너앵글
//static short	layerInd_OutcornerPanel;	// 레이어 번호: 아웃코너판넬
//static short	layerInd_IncornerPanel;		// 레이어 번호: 인코너판넬
//static short	layerInd_RectpipeHanger;	// 레이어 번호: 각파이프 행거
//
//static bool		bLayerInd_Euroform;			// 레이어 번호: 유로폼
//static bool		bLayerInd_RectPipe;			// 레이어 번호: 비계 파이프
//static bool		bLayerInd_PinBolt;			// 레이어 번호: 핀볼트 세트
//static bool		bLayerInd_WallTie;			// 레이어 번호: 벽체 타이
//static bool		bLayerInd_HeadPiece;		// 레이어 번호: 헤드피스
//static bool		bLayerInd_Join;				// 레이어 번호: 결합철물
//static bool		bLayerInd_Plywood;			// 레이어 번호: 합판
//static bool		bLayerInd_Timber;			// 레이어 번호: 각재
//static bool		bLayerInd_EuroformHook;		// 레이어 번호: 유로폼 후크
//static bool		bLayerInd_CrossJointBar;	// 레이어 번호: 십자 조인트 바
//static bool		bLayerInd_BlueClamp;		// 레이어 번호: 블루 클램프
//static bool		bLayerInd_BlueTimberRail;	// 레이어 번호: 블루 목심
//static bool		bLayerInd_Hidden;			// 레이어 번호: 숨김
//
//static bool		bLayerInd_SlabTableform;	// 레이어 번호: 슬래브 테이블폼
//static bool		bLayerInd_Profile;			// 레이어 번호: KS프로파일
//static bool		bLayerInd_Steelform;		// 레이어 번호: 스틸폼
//static bool		bLayerInd_Fillersp;			// 레이어 번호: 휠러스페이서
//static bool		bLayerInd_OutcornerAngle;	// 레이어 번호: 아웃코너앵글
//static bool		bLayerInd_OutcornerPanel;	// 레이어 번호: 아웃코너판넬
//static bool		bLayerInd_IncornerPanel;	// 레이어 번호: 인코너판넬
//static bool		bLayerInd_RectpipeHanger;	// 레이어 번호: 각파이프 행거
//
//static GS::Array<API_Guid>	elemList_Front;	// 그룹화를 위해 생성된 결과물들의 GUID를 전부 저장함 (앞면)
//static GS::Array<API_Guid>	elemList_Back;	// 그룹화를 위해 생성된 결과물들의 GUID를 전부 저장함 (뒷면)
//
//static int	clickedIndex;	// 클릭한 버튼의 인덱스
//static int	iMarginSide;	// 벽 상단을 채우고자 하는데 현재 진행되는 쪽이 어느 쪽 면인가? (1-낮은쪽, 2-높은쪽)


// 낮은 슬래브 측면에 테이블폼을 배치하는 통합 루틴
GSErrCode	placeTableformOnLowSide (void)
{
	GSErrCode	err = NoError;
	short		result;
	long		nSel;
	short		xx;
	double		dx, dy;

	// Selection Manager 관련 변수
	API_SelectionInfo		selectionInfo;
	API_Element				tElem;
	API_Neig				**selNeigs;
	
	GS::Array<API_Guid>		wall;
	GS::Array<API_Guid>		beam;
	GS::Array<API_Guid>		slab;
	GS::Array<API_Guid>		mesh;
	GS::Array<API_Guid>		morph;

	long	nWall;
	long	nBeam;
	long	nSlab;
	long	nMesh;
	long	nMorph;

	// 객체 정보 가져오기
	API_Element				elem;
	API_ElementMemo			memo;
	API_ElemInfo3D			info3D;

	// 모프 객체 정보
	InfoMorphForLowSideTableform	infoMorph;

	// 작업 층 정보
	API_StoryInfo	storyInfo;
	double			workLevel_structural;	// 구조 요소의 작업 층 높이

	// 선택한 요소 가져오기
	err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, true);
	BMKillHandle ((GSHandle *) &selectionInfo.marquee.coords);
	if (err == APIERR_NOPLAN) {
		WriteReport_Alert ("열린 프로젝트 창이 없습니다.");
	}
	if (err == APIERR_NOSEL) {
		WriteReport_Alert ("아무 것도 선택하지 않았습니다.\n필수 선택: 구조 요소 (벽, 보, 슬래브, 메시 중 1개만), 구조 요소 측면을 덮는 모프 (1개)");
	}
	if (err != NoError) {
		BMKillHandle ((GSHandle *) &selNeigs);
		return err;
	}

	// 구조 요소 1개, 영역 모프 1개 선택해야 함
	if (selectionInfo.typeID != API_SelEmpty) {
		nSel = BMGetHandleSize ((GSHandle) selNeigs) / sizeof (API_Neig);
		for (xx = 0 ; xx < nSel && err == NoError ; ++xx) {
			tElem.header.typeID = Neig_To_ElemID ((*selNeigs)[xx].neigID);

			tElem.header.guid = (*selNeigs)[xx].guid;
			if (ACAPI_Element_Get (&tElem) != NoError)	// 가져올 수 있는 요소인가?
				continue;

			if (tElem.header.typeID == API_WallID)		wall.Push (tElem.header.guid);		// 벽인가?
			if (tElem.header.typeID == API_BeamID)		beam.Push (tElem.header.guid);		// 보인가?
			if (tElem.header.typeID == API_SlabID)		slab.Push (tElem.header.guid);		// 슬래브인가?
			if (tElem.header.typeID == API_MeshID)		mesh.Push (tElem.header.guid);		// 메시인가?
			
			if (tElem.header.typeID == API_MorphID)		morph.Push (tElem.header.guid);		// 모프인가?
		}
	}
	BMKillHandle ((GSHandle *) &selNeigs);

	nWall = wall.GetSize ();
	nBeam = beam.GetSize ();
	nSlab = slab.GetSize ();
	nMesh = mesh.GetSize ();

	nMorph = morph.GetSize ();

	// 구조 요소가 1개인가?
	if (nWall + nBeam + nSlab + nMesh == 1) {
		WriteReport_Alert ("구조 요소(벽, 보, 슬래브, 메시)를 1개 선택해야 합니다.");
		err = APIERR_GENERAL;
		return err;
	}

	// 모프가 1개인가?
	if (nMorph != 1) {
		WriteReport_Alert ("구조 요소의 측면을 덮는 모프를 1개 선택해야 합니다.");
		err = APIERR_GENERAL;
		return err;
	}

	// 벽 정보를 가져옴
	if (nWall == 1) {
	}

	// 보 정보를 가져옴
	if (nBeam == 1) {
	}

	// 슬래브 정보를 가져옴
	if (nSlab == 1) {
	}

	// 메시 정보를 가져옴
	if (nMesh == 1) {
	}

	//BNZeroMemory (&elem, sizeof (API_Element));
	//BNZeroMemory (&memo, sizeof (API_ElementMemo));
	//elem.header.guid = walls.Pop ();
	//structuralObject_forTableformWall = elem.header.guid;
	//err = ACAPI_Element_Get (&elem);						// elem.wall.poly.nCoords : 폴리곤 수를 가져올 수 있음
	//err = ACAPI_Element_GetMemo (elem.header.guid, &memo);	// memo.coords : 폴리곤 좌표를 가져올 수 있음
	//
	//if (elem.wall.thickness != elem.wall.thickness1) {
	//	WriteReport_Alert ("벽의 두께는 균일해야 합니다.");
	//	err = APIERR_GENERAL;
	//	return err;
	//}
	//infoWall.wallThk		= elem.wall.thickness;
	//infoWall.floorInd		= elem.header.floorInd;
	//infoWall.bottomOffset	= elem.wall.bottomOffset;
	//infoWall.begX			= elem.wall.begC.x;
	//infoWall.begY			= elem.wall.begC.y;
	//infoWall.endX			= elem.wall.endC.x;
	//infoWall.endY			= elem.wall.endC.y;

	//ACAPI_DisposeElemMemoHdls (&memo);

	//// (2) 모프 정보를 가져옴
	//for (xx = 0 ; xx < nMorphs ; ++xx) {
	//	BNZeroMemory (&elem, sizeof (API_Element));
	//	elem.header.guid = morphs.Pop ();
	//	err = ACAPI_Element_Get (&elem);
	//	err = ACAPI_Element_Get3DInfo (elem.header, &info3D);

	//	// 만약 모프가 누워 있으면(세워져 있지 않으면) 중단
	//	if (abs (info3D.bounds.zMax - info3D.bounds.zMin) < EPS) {
	//		WriteReport_Alert ("모프가 세워져 있지 않습니다.");
	//		err = APIERR_GENERAL;
	//		return err;
	//	}

	//	// 모프의 GUID 저장
	//	infoMorph [xx].guid = elem.header.guid;

	//	// 모프의 좌하단, 우상단 점 지정
	//	if (abs (elem.morph.tranmat.tmx [11] - info3D.bounds.zMin) < EPS) {
	//		// 좌하단 좌표 결정
	//		infoMorph [xx].leftBottomX = elem.morph.tranmat.tmx [3];
	//		infoMorph [xx].leftBottomY = elem.morph.tranmat.tmx [7];
	//		infoMorph [xx].leftBottomZ = elem.morph.tranmat.tmx [11];

	//		// 우상단 좌표는?
	//		if (abs (infoMorph [xx].leftBottomX - info3D.bounds.xMin) < EPS)
	//			infoMorph [xx].rightTopX = info3D.bounds.xMax;
	//		else
	//			infoMorph [xx].rightTopX = info3D.bounds.xMin;
	//		if (abs (infoMorph [xx].leftBottomY - info3D.bounds.yMin) < EPS)
	//			infoMorph [xx].rightTopY = info3D.bounds.yMax;
	//		else
	//			infoMorph [xx].rightTopY = info3D.bounds.yMin;
	//		if (abs (infoMorph [xx].leftBottomZ - info3D.bounds.zMin) < EPS)
	//			infoMorph [xx].rightTopZ = info3D.bounds.zMax;
	//		else
	//			infoMorph [xx].rightTopZ = info3D.bounds.zMin;
	//	} else {
	//		// 우상단 좌표 결정
	//		infoMorph [xx].rightTopX = elem.morph.tranmat.tmx [3];
	//		infoMorph [xx].rightTopY = elem.morph.tranmat.tmx [7];
	//		infoMorph [xx].rightTopZ = elem.morph.tranmat.tmx [11];

	//		// 좌하단 좌표는?
	//		if (abs (infoMorph [xx].rightTopX - info3D.bounds.xMin) < EPS)
	//			infoMorph [xx].leftBottomX = info3D.bounds.xMax;
	//		else
	//			infoMorph [xx].leftBottomX = info3D.bounds.xMin;
	//		if (abs (infoMorph [xx].rightTopY - info3D.bounds.yMin) < EPS)
	//			infoMorph [xx].leftBottomY = info3D.bounds.yMax;
	//		else
	//			infoMorph [xx].leftBottomY = info3D.bounds.yMin;
	//		if (abs (infoMorph [xx].rightTopZ - info3D.bounds.zMin) < EPS)
	//			infoMorph [xx].leftBottomZ = info3D.bounds.zMax;
	//		else
	//			infoMorph [xx].leftBottomZ = info3D.bounds.zMin;
	//	}

	//	// 모프의 Z축 회전 각도 (벽의 설치 각도)
	//	dx = infoMorph [xx].rightTopX - infoMorph [xx].leftBottomX;
	//	dy = infoMorph [xx].rightTopY - infoMorph [xx].leftBottomY;
	//	infoMorph [xx].ang = RadToDegree (atan2 (dy, dx));

	//	// 모프의 가로 길이
	//	infoMorph [xx].horLen = GetDistance (info3D.bounds.xMin, info3D.bounds.yMin, info3D.bounds.xMax, info3D.bounds.yMax);

	//	// 모프의 세로 길이
	//	infoMorph [xx].verLen = abs (info3D.bounds.zMax - info3D.bounds.zMin);

	//	// 영역 모프 제거
	//	API_Elem_Head* headList = new API_Elem_Head [1];
	//	headList [0] = elem.header;
	//	err = ACAPI_Element_Delete (&headList, 1);
	//	delete headList;
	//}

	// 구조 요소의 고도를 파악함
	// 영역 모프의 너비와 높이를 추출
		// 높이가 600 미만이면 방향 -> 가로
		// 높이가 600 이상이면 방향 -> 세로
		// 너비 방향 개수
			// 가로 방향이면 1200으로 채움
			// 세로 방향이면 600으로 채움
		// 높이 선택
			// 슬래브가 1200 이하이면 1200
			// 슬래브가 900 이하이면 900
			// 슬래브가 600 이하이면 600
			// 슬래브가 500 이하이면 500
			// 슬래브가 450 이하이면 450
			// 슬래브가 400 이하이면 400
			// 슬래브가 300 이하이면 300
			// 슬래브가 200 이하이면 200
	// 1차 UI
		// 테이블폼 방향: 가로, 세로
		// 테이블폼 타입: 타입A
		// 추가 / 삭제 버튼
		// 남은 너비
		// 셀: 인코너, 아웃코너앵글, 아웃코너판넬, 테이블폼(유로폼 개수: 가로 방향이면 3개, 세로 방향이면 6개 - 최대 너비 3600), 유로폼, 휠러스페이서, 합판, 각재
	// 2차 UI : 레이어 선택하기
		// 유로폼, 비계 파이프, 핀볼트 세트, 결합철물, 헤드피스, 합판, 각재, 휠러스페이서, 아웃코너앵글, 아웃코너판넬, 인코너

	// 각파이프 정보
		// 하부 수평바 높이: 150
		// 상부 수평바 높이 (세웠을 경우에만): 위에서 300
		// 수직바 위치/길이: 양 끝을 50씩 절단함

	return	err;
}