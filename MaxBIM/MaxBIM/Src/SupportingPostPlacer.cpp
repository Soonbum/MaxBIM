#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "SupportingPostPlacer.hpp"

using namespace SupportingPostPlacerDG;

InfoMorphForSupportingPost			infoMorph;				// 모프 정보
PERISupportingPostPlacementInfo		placementInfoForPERI;	// PERI 동바리 배치 정보
short HPOST_CENTER [5];
short HPOST_UP [5];
short HPOST_DOWN [5];
static short	layerInd_vPost;		// 레이어 번호: 수직재
static short	layerInd_hPost;		// 레이어 번호: 수평재

static GS::Array<API_Guid>	elemList;	// 그룹화를 위해 생성된 결과물들의 GUID를 전부 저장함

// 선택한 직육면체 모프를 기반으로 PERI 동바리를 배치함
GSErrCode	placePERIPost (void)
{
	GSErrCode	err = NoError;
	short		result;
	long		nSel;
	short		xx, yy;

	// Selection Manager 관련 변수
	API_SelectionInfo		selectionInfo;
	API_Element				tElem;
	API_Neig				**selNeigs;
	GS::Array<API_Guid>&	morphs = GS::Array<API_Guid> ();
	long					nMorphs = 0;

	// 객체 정보 가져오기
	API_Element				elem;
	API_ElemInfo3D			info3D;

	// 모프 3D 구성요소 가져오기
	API_Component3D			component;
	API_Tranmat				tm;
	Int32					nVert, nEdge, nPgon;
	Int32					elemIdx, bodyIdx;
	API_Coord3D				trCoord;
	GS::Array<API_Coord3D>&	coords = GS::Array<API_Coord3D> ();
	long					nNodes;

	// 점 입력
	API_GetPointType		pointInfo;
	API_Coord3D				point1, point2;
	double					morphHorLen, morphVerLen;
	bool					bPassed1, bPassed2;

	// 작업 층 정보
	//API_StoryInfo	storyInfo;
	//double			workLevel_morph;	// 모프의 작업 층 높이


	// 선택한 요소 가져오기
	err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, true);
	BMKillHandle ((GSHandle *) &selectionInfo.marquee.coords);
	if (err == APIERR_NOPLAN) {
		ACAPI_WriteReport ("열린 프로젝트 창이 없습니다.", true);
	}
	if (err == APIERR_NOSEL) {
		ACAPI_WriteReport ("아무 것도 선택하지 않았습니다.\n필수 선택: 모프 직육면체 (1개)", true);
		//ACAPI_WriteReport ("아무 것도 선택하지 않았습니다.\n필수 선택: 동바리 하부에 위치하는 슬래브 또는 보 (1개), 모프 직육면체 (1개)", true);
	}
	if (err != NoError) {
		BMKillHandle ((GSHandle *) &selNeigs);
		return err;
	}

	// 모프 1개 선택해야 함
	if (selectionInfo.typeID != API_SelEmpty) {
		nSel = BMGetHandleSize ((GSHandle) selNeigs) / sizeof (API_Neig);
		for (xx = 0 ; xx < nSel && err == NoError ; ++xx) {
			tElem.header.typeID = Neig_To_ElemID ((*selNeigs)[xx].neigID);

			tElem.header.guid = (*selNeigs)[xx].guid;
			if (ACAPI_Element_Get (&tElem) != NoError)	// 가져올 수 있는 요소인가?
				continue;

			if (tElem.header.typeID == API_MorphID)		// 모프인가?
				morphs.Push (tElem.header.guid);
		}
	}
	BMKillHandle ((GSHandle *) &selNeigs);
	nMorphs = morphs.GetSize ();

	// 모프가 1개인가?
	if (nMorphs != 1) {
		ACAPI_WriteReport ("직육면체 모프를 1개 선택하셔야 합니다.", true);
		err = APIERR_GENERAL;
		return err;
	}

	// 모프 정보를 가져옴
	BNZeroMemory (&elem, sizeof (API_Element));
	elem.header.guid = morphs.Pop ();
	err = ACAPI_Element_Get (&elem);
	err = ACAPI_Element_Get3DInfo (elem.header, &info3D);

	// 만약 모프의 높이 차이가 0이면 중단
	if (abs (info3D.bounds.zMax - info3D.bounds.zMin) < EPS) {
		ACAPI_WriteReport ("직육면체 모프의 높이 차이가 없습니다.", true);
		err = APIERR_GENERAL;
		return err;
	}

	// 모프의 GUID 저장
	infoMorph.guid = elem.header.guid;

	// 모프의 층 인덱스 저장
	infoMorph.floorInd = elem.header.floorInd;

	// 모프의 3D 바디를 가져옴
	BNZeroMemory (&component, sizeof (API_Component3D));
	component.header.typeID = API_BodyID;
	component.header.index = info3D.fbody;
	err = ACAPI_3D_GetComponent (&component);

	nVert = component.body.nVert;
	nEdge = component.body.nEdge;
	nPgon = component.body.nPgon;
	tm = component.body.tranmat;
	elemIdx = component.body.head.elemIndex - 1;
	bodyIdx = component.body.head.bodyIndex - 1;
	
	yy = 0;

	// 모프의 8개 꼭지점 구하기
	for (xx = 1 ; xx <= nVert ; ++xx) {
		component.header.typeID	= API_VertID;
		component.header.index	= xx;
		err = ACAPI_3D_GetComponent (&component);
		if (err == NoError) {
			trCoord.x = tm.tmx[0]*component.vert.x + tm.tmx[1]*component.vert.y + tm.tmx[2]*component.vert.z + tm.tmx[3];
			trCoord.y = tm.tmx[4]*component.vert.x + tm.tmx[5]*component.vert.y + tm.tmx[6]*component.vert.z + tm.tmx[7];
			trCoord.z = tm.tmx[8]*component.vert.x + tm.tmx[9]*component.vert.y + tm.tmx[10]*component.vert.z + tm.tmx[11];
			// 여기서 다음 점들은 리스트에 추가하지 않는다.
			if ((abs (trCoord.x) < EPS) && (abs (trCoord.y) < EPS) && (abs (trCoord.z - 1.0) < EPS))		// 1번 (0, 0, 1)
				; // pass
			else if ((abs (trCoord.x) < EPS) && (abs (trCoord.y - 1.0) < EPS) && (abs (trCoord.z) < EPS))	// 2번 (0, 1, 0)
				; // pass
			else if ((abs (trCoord.x - 1.0) < EPS) && (abs (trCoord.y) < EPS) && (abs (trCoord.z) < EPS))	// 3번 (1, 0, 0)
				; // pass
			else if ((abs (trCoord.x) < EPS) && (abs (trCoord.y) < EPS) && (abs (trCoord.z) < EPS))			// 4번 (0, 0, 0)
				; // pass
			else {
				//placeCoordinateLabel (trCoord.x, trCoord.y, trCoord.z);
				coords.Push (trCoord);
				if (yy < 8) infoMorph.points [yy++] = trCoord;
			}
		}
	}
	nNodes = coords.GetSize ();

	// 모프의 기하 정보 구하기
	infoMorph.leftBottomX = infoMorph.points [0].x;
	infoMorph.leftBottomY = infoMorph.points [0].y;
	infoMorph.leftBottomZ = infoMorph.points [0].z;
	infoMorph.rightTopX = infoMorph.points [0].x;
	infoMorph.rightTopY = infoMorph.points [0].y;
	infoMorph.rightTopZ = infoMorph.points [0].z;

	for (xx = 0 ; xx < 8 ; ++xx) {
		if (infoMorph.leftBottomX > infoMorph.points [xx].x)	infoMorph.leftBottomX = infoMorph.points [xx].x;
		if (infoMorph.leftBottomY > infoMorph.points [xx].y)	infoMorph.leftBottomY = infoMorph.points [xx].y;
		if (infoMorph.leftBottomZ > infoMorph.points [xx].z)	infoMorph.leftBottomZ = infoMorph.points [xx].z;

		if (infoMorph.rightTopX < infoMorph.points [xx].x)		infoMorph.rightTopX = infoMorph.points [xx].x;
		if (infoMorph.rightTopY < infoMorph.points [xx].y)		infoMorph.rightTopY = infoMorph.points [xx].y;
		if (infoMorph.rightTopZ < infoMorph.points [xx].z)		infoMorph.rightTopZ = infoMorph.points [xx].z;
	}
	infoMorph.ang = 0.0;
	infoMorph.width = GetDistance (infoMorph.leftBottomX, infoMorph.leftBottomY, infoMorph.rightTopX, infoMorph.leftBottomY);
	infoMorph.depth = GetDistance (infoMorph.leftBottomX, infoMorph.leftBottomY, infoMorph.leftBottomX, infoMorph.rightTopY);
	infoMorph.height = abs (info3D.bounds.zMax - info3D.bounds.zMin);

	// 모프의 가로, 세로 길이를 구함
	morphHorLen = infoMorph.width;
	morphVerLen = infoMorph.depth;

	// 사용자로부터 직육면체 모프의 두 점을 입력 받음 (보가 통과하는 방향과 일치하는 두 점을 클릭하십시오)
	BNZeroMemory (&pointInfo, sizeof (API_GetPointType));
	CHCopyC ("보가 통과하는 방향과 일치하는 두 점 중 왼쪽 점을 클릭하십시오.", pointInfo.prompt);
	pointInfo.enableQuickSelection = true;
	err = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
	point1 = pointInfo.pos;

	BNZeroMemory (&pointInfo, sizeof (API_GetPointType));
	CHCopyC ("보가 통과하는 방향과 일치하는 두 점 중 오른쪽 점을 클릭하십시오.", pointInfo.prompt);
	pointInfo.enableQuickSelection = true;
	err = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);
	point2 = pointInfo.pos;

	// 만약 point1, point2가 모프의 꼭지점이 아니면 오류 메시지 출력 후 종료
	bPassed1 = false;
	for (xx = 0 ; xx < 8 ; ++xx) {
		if (GetDistance (point1, infoMorph.points [xx]) < EPS)
			bPassed1 = true;
	}

	bPassed2 = false;
	for (xx = 0 ; xx < 8 ; ++xx) {
		if (GetDistance (point2, infoMorph.points [xx]) < EPS)
			bPassed2 = true;
	}

	if ( !(bPassed1 && bPassed2) ) {
		ACAPI_WriteReport ("모프의 꼭지점에 속하지 않은 점을 클릭했습니다.", true);
		return err;
	}

	// 만약 point1과 point2 간의 거리가 가로 또는 세로 길이가 아니면 오류 메시지 출력 후 종료
	if ( !((abs (GetDistance (point1, point2) - morphHorLen) < EPS) || (abs (GetDistance (point1, point2) - morphVerLen) < EPS)) ) {
		ACAPI_WriteReport ("두 점 간의 거리가 모프의 가로 또는 세로 길이와 일치하지 않습니다.", true);
		return err;
	}

	// 만약 point1과 point2 간의 거리가 가로 길이일 경우,
	if (abs (GetDistance (point1, point2) - morphHorLen) < EPS) {
		placementInfoForPERI.width = morphHorLen;
		placementInfoForPERI.depth = morphVerLen;
		placementInfoForPERI.bFlipped = false;

	// 만약 point1과 point2 간의 거리가 세로 길이일 경우,
	} else if (abs (GetDistance (point1, point2) - morphVerLen) < EPS) {
		placementInfoForPERI.width = morphVerLen;
		placementInfoForPERI.depth = morphHorLen;
		placementInfoForPERI.bFlipped = true;
	}

	// 크로스헤드, 각재는 처음에 없다고 가정함
	placementInfoForPERI.heightCrosshead = 0.0;
	placementInfoForPERI.heightTimber = 0.0;

	// 너비 방향의 수직재 쌍은 초기 1개
	placementInfoForPERI.nColVPost = 1;

	// [다이얼로그] 동바리 설치 옵션을 설정함
	result = DGModalDialog (ACAPI_GetOwnResModule (), 32521, ACAPI_GetOwnResModule (), PERISupportingPostPlacerHandler1, 0);

	// 영역 모프 제거
	//API_Elem_Head* headList = new API_Elem_Head [1];
	//headList [0] = elem.header;
	//err = ACAPI_Element_Delete (&headList, 1);
	//delete headList;

	// 입력한 데이터를 기반으로 수직재, 수평재 배치
	//if (result == DG_OK) {
	//	PERI_VPost vpost;
	//	PERI_HPost hpost;

	//	// 수평재가 있을 경우, 수평재의 규격에 맞게 변경함
	//	if (placementInfoForPERI.bHPost == true) {
	//		// 남쪽/북쪽의 수평재 규격에 따라 모프 영역의 가로 길이가 달라짐
	//		if (my_strcmp (placementInfoForPERI.nomHPost_South, "296 cm") == 0)			infoMorph.width = 2.960;
	//		else if (my_strcmp (placementInfoForPERI.nomHPost_South, "266 cm") == 0)	infoMorph.width = 2.660;
	//		else if (my_strcmp (placementInfoForPERI.nomHPost_South, "237 cm") == 0)	infoMorph.width = 2.370;
	//		else if (my_strcmp (placementInfoForPERI.nomHPost_South, "230 cm") == 0)	infoMorph.width = 2.300;
	//		else if (my_strcmp (placementInfoForPERI.nomHPost_South, "225 cm") == 0)	infoMorph.width = 2.250;
	//		else if (my_strcmp (placementInfoForPERI.nomHPost_South, "201.5 cm") == 0)	infoMorph.width = 2.015;
	//		else if (my_strcmp (placementInfoForPERI.nomHPost_South, "150 cm") == 0)	infoMorph.width = 1.500;
	//		else if (my_strcmp (placementInfoForPERI.nomHPost_South, "137.5 cm") == 0)	infoMorph.width = 1.375;
	//		else if (my_strcmp (placementInfoForPERI.nomHPost_South, "120 cm") == 0)	infoMorph.width = 1.200;
	//		else if (my_strcmp (placementInfoForPERI.nomHPost_South, "90 cm") == 0)		infoMorph.width = 0.900;
	//		else if (my_strcmp (placementInfoForPERI.nomHPost_South, "75 cm") == 0)		infoMorph.width = 0.750;
	//		else if (my_strcmp (placementInfoForPERI.nomHPost_South, "62.5 cm") == 0)	infoMorph.width = 0.625;

	//		// 서쪽/동쪽의 수평재 규격에 따라 모프 영역의 세로 길이가 달라짐
	//		if (my_strcmp (placementInfoForPERI.nomHPost_West, "296 cm") == 0)			infoMorph.depth = 2.960;
	//		else if (my_strcmp (placementInfoForPERI.nomHPost_West, "266 cm") == 0)		infoMorph.depth = 2.660;
	//		else if (my_strcmp (placementInfoForPERI.nomHPost_West, "237 cm") == 0)		infoMorph.depth = 2.370;
	//		else if (my_strcmp (placementInfoForPERI.nomHPost_West, "230 cm") == 0)		infoMorph.depth = 2.300;
	//		else if (my_strcmp (placementInfoForPERI.nomHPost_West, "225 cm") == 0)		infoMorph.depth = 2.250;
	//		else if (my_strcmp (placementInfoForPERI.nomHPost_West, "201.5 cm") == 0)	infoMorph.depth = 2.015;
	//		else if (my_strcmp (placementInfoForPERI.nomHPost_West, "150 cm") == 0)		infoMorph.depth = 1.500;
	//		else if (my_strcmp (placementInfoForPERI.nomHPost_West, "137.5 cm") == 0)	infoMorph.depth = 1.375;
	//		else if (my_strcmp (placementInfoForPERI.nomHPost_West, "120 cm") == 0)		infoMorph.depth = 1.200;
	//		else if (my_strcmp (placementInfoForPERI.nomHPost_West, "90 cm") == 0)		infoMorph.depth = 0.900;
	//		else if (my_strcmp (placementInfoForPERI.nomHPost_West, "75 cm") == 0)		infoMorph.depth = 0.750;
	//		else if (my_strcmp (placementInfoForPERI.nomHPost_West, "62.5 cm") == 0)	infoMorph.depth = 0.625;
	//	}

	//	// 수직재 1단 배치
	//	// 회전Y(180), 크로스헤드 위치(하단), 위로 len_current만큼 이동해야 함
	//	vpost.leftBottomX = infoMorph.leftBottomX;
	//	vpost.leftBottomY = infoMorph.leftBottomY;
	//	vpost.leftBottomZ = infoMorph.leftBottomZ + placementInfoForPERI.heightVPost1;
	//	vpost.ang = infoMorph.ang;
	//	if (placementInfoForPERI.bVPost2 == false)
	//		vpost.bCrosshead = placementInfoForPERI.bCrosshead;
	//	else
	//		vpost.bCrosshead = false;
	//	sprintf (vpost.stType, "%s", placementInfoForPERI.nomVPost1);
	//	sprintf (vpost.crossheadType, "PERI");
	//	sprintf (vpost.posCrosshead, "하단");
	//	vpost.angCrosshead = 0.0;
	//	vpost.angY = DegreeToRad (180.0);
	//	vpost.len_current = placementInfoForPERI.heightVPost1;
	//	vpost.text2_onoff = true;
	//	vpost.text_onoff = true;
	//	vpost.bShowCoords = true;

	//	elemList.Push (placementInfoForPERI.placeVPost (vpost));	// 좌하단
	//	moveIn3D ('x', vpost.ang, infoMorph.width, &vpost.leftBottomX, &vpost.leftBottomY, &vpost.leftBottomZ);
	//	elemList.Push (placementInfoForPERI.placeVPost (vpost));	// 우하단
	//	moveIn3D ('y', vpost.ang, infoMorph.depth, &vpost.leftBottomX, &vpost.leftBottomY, &vpost.leftBottomZ);
	//	elemList.Push (placementInfoForPERI.placeVPost (vpost));	// 우상단
	//	moveIn3D ('x', vpost.ang, -infoMorph.width, &vpost.leftBottomX, &vpost.leftBottomY, &vpost.leftBottomZ);
	//	elemList.Push (placementInfoForPERI.placeVPost (vpost));	// 좌상단

	//	// 수직재 2단이 있을 경우
	//	if (placementInfoForPERI.bVPost2 == true) {
	//		// 회전Y(0), 크로스헤드 위치(상단)
	//		vpost.leftBottomX = infoMorph.leftBottomX;
	//		vpost.leftBottomY = infoMorph.leftBottomY;
	//		vpost.leftBottomZ = infoMorph.leftBottomZ + placementInfoForPERI.heightVPost1;
	//		vpost.ang = infoMorph.ang;
	//		vpost.bCrosshead = placementInfoForPERI.bCrosshead;
	//		sprintf (vpost.stType, "%s", placementInfoForPERI.nomVPost2);
	//		sprintf (vpost.crossheadType, "PERI");
	//		sprintf (vpost.posCrosshead, "상단");
	//		vpost.angCrosshead = 0.0;
	//		vpost.angY = 0.0;
	//		vpost.len_current = placementInfoForPERI.heightVPost2;
	//		vpost.text2_onoff = true;
	//		vpost.text_onoff = true;
	//		vpost.bShowCoords = true;

	//		elemList.Push (placementInfoForPERI.placeVPost (vpost));	// 좌하단
	//		moveIn3D ('x', vpost.ang, infoMorph.width, &vpost.leftBottomX, &vpost.leftBottomY, &vpost.leftBottomZ);
	//		elemList.Push (placementInfoForPERI.placeVPost (vpost));	// 우하단
	//		moveIn3D ('y', vpost.ang, infoMorph.depth, &vpost.leftBottomX, &vpost.leftBottomY, &vpost.leftBottomZ);
	//		elemList.Push (placementInfoForPERI.placeVPost (vpost));	// 우상단
	//		moveIn3D ('x', vpost.ang, -infoMorph.width, &vpost.leftBottomX, &vpost.leftBottomY, &vpost.leftBottomZ);
	//		elemList.Push (placementInfoForPERI.placeVPost (vpost));	// 좌상단
	//	}

	//	// 수평재가 있을 경우
	//	if (placementInfoForPERI.bHPost == true) {
	//		hpost.leftBottomX = infoMorph.leftBottomX;
	//		hpost.leftBottomY = infoMorph.leftBottomY;
	//		hpost.leftBottomZ = infoMorph.leftBottomZ;
	//		hpost.ang = infoMorph.ang;
	//		hpost.angX = 0.0;
	//		hpost.angY = 0.0;
	//		
	//		// 1단 ----------------------------------------------------------------------------------------------------
	//		// 남쪽
	//		moveIn3D ('z', hpost.ang, 2.000, &hpost.leftBottomX, &hpost.leftBottomY, &hpost.leftBottomZ);
	//		moveIn3D ('x', hpost.ang, 0.050, &hpost.leftBottomX, &hpost.leftBottomY, &hpost.leftBottomZ);
	//		sprintf (hpost.stType, "%s", placementInfoForPERI.nomHPost_South);	// 빈 문자열이면 배치하지 않음
	//		if (my_strcmp (placementInfoForPERI.nomHPost_South, "") != 0)	elemList.Push (placementInfoForPERI.placeHPost (hpost));

	//		// 동쪽
	//		moveIn3D ('x', hpost.ang, infoMorph.width - 0.050, &hpost.leftBottomX, &hpost.leftBottomY, &hpost.leftBottomZ);
	//		moveIn3D ('y', hpost.ang, 0.050, &hpost.leftBottomX, &hpost.leftBottomY, &hpost.leftBottomZ);
	//		hpost.ang = infoMorph.ang + DegreeToRad (90.0);
	//		sprintf (hpost.stType, "%s", placementInfoForPERI.nomHPost_East);	// 빈 문자열이면 배치하지 않음
	//		if (my_strcmp (placementInfoForPERI.nomHPost_East, "") != 0)	elemList.Push (placementInfoForPERI.placeHPost (hpost));
	//		hpost.ang = infoMorph.ang;

	//		// 북쪽
	//		moveIn3D ('x', hpost.ang, -0.050, &hpost.leftBottomX, &hpost.leftBottomY, &hpost.leftBottomZ);
	//		moveIn3D ('y', hpost.ang, infoMorph.depth - 0.050, &hpost.leftBottomX, &hpost.leftBottomY, &hpost.leftBottomZ);
	//		hpost.ang = infoMorph.ang + DegreeToRad (180.0);
	//		sprintf (hpost.stType, "%s", placementInfoForPERI.nomHPost_North);	// 빈 문자열이면 배치하지 않음
	//		if (my_strcmp (placementInfoForPERI.nomHPost_North, "") != 0)	elemList.Push (placementInfoForPERI.placeHPost (hpost));
	//		hpost.ang = infoMorph.ang;

	//		// 서쪽
	//		moveIn3D ('x', hpost.ang, 0.050 - infoMorph.width, &hpost.leftBottomX, &hpost.leftBottomY, &hpost.leftBottomZ);
	//		moveIn3D ('y', hpost.ang, -0.050, &hpost.leftBottomX, &hpost.leftBottomY, &hpost.leftBottomZ);
	//		hpost.ang = infoMorph.ang + DegreeToRad (270.0);
	//		sprintf (hpost.stType, "%s", placementInfoForPERI.nomHPost_West);	// 빈 문자열이면 배치하지 않음
	//		if (my_strcmp (placementInfoForPERI.nomHPost_West, "") != 0)	elemList.Push (placementInfoForPERI.placeHPost (hpost));
	//		hpost.ang = infoMorph.ang;

	//		// 2단 ----------------------------------------------------------------------------------------------------
	//		hpost.leftBottomX = infoMorph.leftBottomX;
	//		hpost.leftBottomY = infoMorph.leftBottomY;
	//		hpost.leftBottomZ = infoMorph.leftBottomZ;
	//		hpost.ang = infoMorph.ang;

	//		// 수직재 1단/2단 높이의 합이 4000 이상일 경우
	//		if (((placementInfoForPERI.bVPost1 * placementInfoForPERI.heightVPost1) + (placementInfoForPERI.bVPost2 * placementInfoForPERI.heightVPost2)) > 4.000) {
	//			// 남쪽
	//			moveIn3D ('z', hpost.ang, 4.000, &hpost.leftBottomX, &hpost.leftBottomY, &hpost.leftBottomZ);
	//			moveIn3D ('x', hpost.ang, 0.050, &hpost.leftBottomX, &hpost.leftBottomY, &hpost.leftBottomZ);
	//			sprintf (hpost.stType, "%s", placementInfoForPERI.nomHPost_South);	// 빈 문자열이면 배치하지 않음
	//			if (my_strcmp (placementInfoForPERI.nomHPost_South, "") != 0)	elemList.Push (placementInfoForPERI.placeHPost (hpost));

	//			// 동쪽
	//			moveIn3D ('x', hpost.ang, infoMorph.width - 0.050, &hpost.leftBottomX, &hpost.leftBottomY, &hpost.leftBottomZ);
	//			moveIn3D ('y', hpost.ang, 0.050, &hpost.leftBottomX, &hpost.leftBottomY, &hpost.leftBottomZ);
	//			hpost.ang = infoMorph.ang + DegreeToRad (90.0);
	//			sprintf (hpost.stType, "%s", placementInfoForPERI.nomHPost_East);	// 빈 문자열이면 배치하지 않음
	//			if (my_strcmp (placementInfoForPERI.nomHPost_East, "") != 0)	elemList.Push (placementInfoForPERI.placeHPost (hpost));
	//			hpost.ang = infoMorph.ang;

	//			// 북쪽
	//			moveIn3D ('x', hpost.ang, -0.050, &hpost.leftBottomX, &hpost.leftBottomY, &hpost.leftBottomZ);
	//			moveIn3D ('y', hpost.ang, infoMorph.depth - 0.050, &hpost.leftBottomX, &hpost.leftBottomY, &hpost.leftBottomZ);
	//			hpost.ang = infoMorph.ang + DegreeToRad (180.0);
	//			sprintf (hpost.stType, "%s", placementInfoForPERI.nomHPost_North);	// 빈 문자열이면 배치하지 않음
	//			if (my_strcmp (placementInfoForPERI.nomHPost_North, "") != 0)	elemList.Push (placementInfoForPERI.placeHPost (hpost));
	//			hpost.ang = infoMorph.ang;

	//			// 서쪽
	//			moveIn3D ('x', hpost.ang, 0.050 - infoMorph.width, &hpost.leftBottomX, &hpost.leftBottomY, &hpost.leftBottomZ);
	//			moveIn3D ('y', hpost.ang, -0.050, &hpost.leftBottomX, &hpost.leftBottomY, &hpost.leftBottomZ);
	//			hpost.ang = infoMorph.ang + DegreeToRad (270.0);
	//			sprintf (hpost.stType, "%s", placementInfoForPERI.nomHPost_West);	// 빈 문자열이면 배치하지 않음
	//			if (my_strcmp (placementInfoForPERI.nomHPost_West, "") != 0)	elemList.Push (placementInfoForPERI.placeHPost (hpost));
	//			hpost.ang = infoMorph.ang;
	//		}
	//	}

	//	// 그룹화 하기
	//	if (!elemList.IsEmpty ()) {
	//		GSSize nElems = elemList.GetSize ();
	//		API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
	//		if (elemHead != NULL) {
	//			for (GSIndex i = 0; i < nElems; i++)
	//				(*elemHead)[i].guid = elemList[i];

	//			ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

	//			BMKillHandle ((GSHandle *) &elemHead);
	//		}
	//	}
	//	elemList.Clear (false);
	//}

	// 화면 새로고침
	ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
	bool	regenerate = true;
	ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

	return	err;
}

// 수직재 배치
API_Guid	PERISupportingPostPlacementInfo::placeVPost (PERI_VPost params)
{
	GSErrCode	err = NoError;
	API_Element			elem;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	const GS::uchar_t*	gsmName = L("PERI동바리 수직재 v0.1.gsm");
	double				aParam;
	double				bParam;
	Int32				addParNum;

	// 객체 로드
	BNZeroMemory (&elem, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, gsmName);
	err = ACAPI_LibPart_Search (&libPart, false);
	if (err != NoError)
		return elem.header.guid;
	if (libPart.location != NULL)
		delete libPart.location;

	ACAPI_LibPart_Get (&libPart);

	elem.header.typeID = API_ObjectID;
	elem.header.guid = GSGuid2APIGuid (GS::Guid (libPart.ownUnID));

	ACAPI_Element_GetDefaults (&elem, &memo);
	ACAPI_LibPart_GetParams (libPart.index, &aParam, &bParam, &addParNum, &memo.params);

	// 라이브러리의 파라미터 값 입력
	elem.object.libInd = libPart.index;
	elem.object.pos.x = params.leftBottomX;
	elem.object.pos.y = params.leftBottomY;
	elem.object.level = params.leftBottomZ;
	elem.object.xRatio = aParam;
	elem.object.yRatio = bParam;
	elem.object.angle = params.ang;
	elem.header.floorInd = infoMorph.floorInd;

	// 레이어
	elem.header.layer = layerInd_vPost;

	setParameterByName (&memo, "stType", params.stType);				// 규격
	setParameterByName (&memo, "bCrosshead", params.bCrosshead);		// 크로스헤드 On/Off
	setParameterByName (&memo, "posCrosshead", params.posCrosshead);	// 크로스헤드 위치 (상단, 하단)
	setParameterByName (&memo, "crossheadType", params.crossheadType);	// 크로스헤드 타입 (PERI, 당사 제작품)
	setParameterByName (&memo, "angCrosshead", params.angCrosshead);	// 크로스헤드 회전각도
	setParameterByName (&memo, "len_current", params.len_current);		// 현재 길이
	setParameterByName (&memo, "angY", params.angY);					// 회전 Y

	setParameterByName (&memo, "text2_onoff", params.text2_onoff);		// 2D 텍스트 On/Off
	setParameterByName (&memo, "text_onoff", params.text_onoff);		// 3D 텍스트 On/Off
	setParameterByName (&memo, "bShowCoords", params.bShowCoords);		// 좌표값 표시 On/Off

	// 규격에 따라 최소 길이, 최대 길이를 자동으로 설정
	if (my_strcmp (params.stType, "MP 120") == 0) {
		setParameterByName (&memo, "pos_lever", 0.600);
		setParameterByName (&memo, "len_min", 0.800);
		setParameterByName (&memo, "len_max", 1.200);
	} else if (my_strcmp (params.stType, "MP 250") == 0) {
		setParameterByName (&memo, "pos_lever", 1.250);
		setParameterByName (&memo, "len_min", 1.450);
		setParameterByName (&memo, "len_max", 2.500);
	} else if (my_strcmp (params.stType, "MP 350") == 0) {
		setParameterByName (&memo, "pos_lever", 1.750);
		setParameterByName (&memo, "len_min", 1.950);
		setParameterByName (&memo, "len_max", 3.500);
	} else if (my_strcmp (params.stType, "MP 480") == 0) {
		setParameterByName (&memo, "pos_lever", 2.400);
		setParameterByName (&memo, "len_min", 2.600);
		setParameterByName (&memo, "len_max", 4.800);
	} else if (my_strcmp (params.stType, "MP 625") == 0) {
		setParameterByName (&memo, "pos_lever", 4.100);
		setParameterByName (&memo, "len_min", 4.300);
		setParameterByName (&memo, "len_max", 6.250);
	}

	// 객체 배치
	ACAPI_Element_Create (&elem, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	// 파라미터 스크립트를 강제로 실행시킴
	ACAPI_Goodies (APIAny_RunGDLParScriptID, &elem.header, 0);

	return	elem.header.guid;
}

// 수평재 배치
API_Guid	PERISupportingPostPlacementInfo::placeHPost (PERI_HPost params)
{
	GSErrCode	err = NoError;
	API_Element			elem;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	const GS::uchar_t*	gsmName = L("PERI동바리 수평재 v0.2.gsm");
	double				aParam;
	double				bParam;
	Int32				addParNum;

	// 객체 로드
	BNZeroMemory (&elem, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, gsmName);
	err = ACAPI_LibPart_Search (&libPart, false);
	if (err != NoError)
		return elem.header.guid;
	if (libPart.location != NULL)
		delete libPart.location;

	ACAPI_LibPart_Get (&libPart);

	elem.header.typeID = API_ObjectID;
	elem.header.guid = GSGuid2APIGuid (GS::Guid (libPart.ownUnID));

	ACAPI_Element_GetDefaults (&elem, &memo);
	ACAPI_LibPart_GetParams (libPart.index, &aParam, &bParam, &addParNum, &memo.params);

	if (my_strcmp (params.stType, "296 cm") == 0)			{ setParameterByName (&memo, "A", 2.960);	aParam = 2.960;	}
	else if (my_strcmp (params.stType, "266 cm") == 0)		{ setParameterByName (&memo, "A", 2.660);	aParam = 2.660;	}
	else if (my_strcmp (params.stType, "237 cm") == 0)		{ setParameterByName (&memo, "A", 2.370);	aParam = 2.370;	}
	else if (my_strcmp (params.stType, "230 cm") == 0)		{ setParameterByName (&memo, "A", 2.300);	aParam = 2.300;	}
	else if (my_strcmp (params.stType, "225 cm") == 0)		{ setParameterByName (&memo, "A", 2.250);	aParam = 2.250;	}
	else if (my_strcmp (params.stType, "201.5 cm") == 0)	{ setParameterByName (&memo, "A", 2.015);	aParam = 2.015;	}
	else if (my_strcmp (params.stType, "150 cm") == 0)		{ setParameterByName (&memo, "A", 1.500);	aParam = 1.500;	}
	else if (my_strcmp (params.stType, "137.5 cm") == 0)	{ setParameterByName (&memo, "A", 1.375);	aParam = 1.375;	}
	else if (my_strcmp (params.stType, "120 cm") == 0)		{ setParameterByName (&memo, "A", 1.200);	aParam = 1.200;	}
	else if (my_strcmp (params.stType, "90 cm") == 0)		{ setParameterByName (&memo, "A", 0.900);	aParam = 0.900;	}
	else if (my_strcmp (params.stType, "75 cm") == 0)		{ setParameterByName (&memo, "A", 0.750);	aParam = 0.750;	}
	else if (my_strcmp (params.stType, "62.5 cm") == 0)		{ setParameterByName (&memo, "A", 0.625);	aParam = 0.625;	}
	
	setParameterByName (&memo, "lenFrame", aParam - 0.100);

	// 라이브러리의 파라미터 값 입력
	elem.object.libInd = libPart.index;
	elem.object.pos.x = params.leftBottomX;
	elem.object.pos.y = params.leftBottomY;
	elem.object.level = params.leftBottomZ;
	elem.object.xRatio = aParam;
	elem.object.yRatio = bParam;
	elem.object.angle = params.ang;
	elem.header.floorInd = infoMorph.floorInd;

	// 레이어
	elem.header.layer = layerInd_hPost;

	setParameterByName (&memo, "stType", params.stType);	// 규격
	setParameterByName (&memo, "angX", params.angX);		// 회전 X
	setParameterByName (&memo, "angY", params.angY);		// 회전 Y

	// 객체 배치
	ACAPI_Element_Create (&elem, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	// 파라미터 스크립트를 강제로 실행시킴
	ACAPI_Goodies (APIAny_RunGDLParScriptID, &elem.header, 0);

	return	elem.header.guid;
}

// 수직재 단수 (1/2단, 높이가 6미터 초과되면 2단 권유할 것), 수직재의 규격/높이, 수평재 유무(단, 높이가 3500 이상이면 추가할 것을 권유할 것), 수평재 너비, 크로스헤드 유무, 수직재/수평재 레이어를 설정
short DGCALLBACK PERISupportingPostPlacerHandler1 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	xx;
	short	itmIdx;
	short	result;
	//char	tempStr [16];
	API_UCCallbackType	ucb;

	// 수직재 개수 (최소 1개)
	// 수평재 설정 배열 [위쪽]
	// 수평재 설정 배열 [가운데]
	// 수평재 설정 배열 [아래쪽]

	switch (message) {
		case DG_MSG_INIT:
			// 다이얼로그 타이틀
			DGSetDialogTitle (dialogID, "PERI 동바리 자동 배치");

			//////////////////////////////////////////////////////////// 아이템 배치 (기본 버튼)
			// 적용 버튼
			DGSetItemText (dialogID, DG_OK, "확 인");

			// 종료 버튼
			DGSetItemText (dialogID, DG_CANCEL, "취 소");

			//////////////////////////////////////////////////////////// 아이템 배치 (나머지)
			DGSetItemText (dialogID, LABEL_TYPE, "타입");

			DGSetItemText (dialogID, LABEL_SIDE_VIEW, "측면도");
			DGSetItemText (dialogID, LABEL_PLAN_VIEW, "평면도");

			DGSetItemText (dialogID, LABEL_UPWARD, "보/슬래브");
			DGSetItemText (dialogID, LABEL_TIMBER, "없음");		// 타입에 따라 바뀜
			DGSetItemText (dialogID, LABEL_VERTICAL_2ND, "수직재\n2단");
			DGSetItemText (dialogID, LABEL_VERTICAL_1ST, "수직재\n1단");
			DGSetItemText (dialogID, LABEL_DOWNWARD, "바닥");

			DGSetItemText (dialogID, LABEL_TOTAL_HEIGHT, "총 높이");
			DGSetItemText (dialogID, LABEL_REMAIN_HEIGHT, "남은 높이");

			DGSetItemText (dialogID, CHECKBOX_CROSSHEAD, "크로스헤드");

			DGSetItemText (dialogID, CHECKBOX_VPOST1, "1단");
			DGSetItemText (dialogID, LABEL_VPOST1_NOMINAL, "규격");
			DGSetItemText (dialogID, LABEL_VPOST1_HEIGHT, "높이");
			DGSetItemText (dialogID, CHECKBOX_VPOST2, "2단");
			DGSetItemText (dialogID, LABEL_VPOST2_NOMINAL, "규격");
			DGSetItemText (dialogID, LABEL_VPOST2_HEIGHT, "높이");

			// 레이어 관련
			DGSetItemText (dialogID, LABEL_LAYER_SETTINGS, "레이어 설정");
			DGSetItemText (dialogID, LABEL_LAYER_SUPPORT, "강관 동바리");
			DGSetItemText (dialogID, LABEL_LAYER_VPOST, "PERI 수직재");
			DGSetItemText (dialogID, LABEL_LAYER_HPOST, "PERI 수평재");
			DGSetItemText (dialogID, LABEL_LAYER_TIMBER, "산승각/토류판");
			DGSetItemText (dialogID, LABEL_LAYER_GIRDER, "GT24 거더");
			DGSetItemText (dialogID, LABEL_LAYER_BEAM_BRACKET, "보 브라켓");
			DGSetItemText (dialogID, LABEL_LAYER_YOKE, "보 멍에제");

			// 체크박스: 레이어 묶음
			DGSetItemText (dialogID, CHECKBOX_LAYER_COUPLING, "레이어 묶음");
			DGSetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING, TRUE);

			// 유저 컨트롤 초기화
			BNZeroMemory (&ucb, sizeof (ucb));
			ucb.dialogID = dialogID;
			ucb.type	 = APIUserControlType_Layer;
			ucb.itemID	 = USERCONTROL_LAYER_SUPPORT;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_SUPPORT, 1);

			ucb.itemID	 = USERCONTROL_LAYER_VPOST;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_VPOST, 1);

			ucb.itemID	 = USERCONTROL_LAYER_HPOST;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_HPOST, 1);

			ucb.itemID	 = USERCONTROL_LAYER_TIMBER;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER, 1);
			
			ucb.itemID	 = USERCONTROL_LAYER_GIRDER;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_GIRDER, 1);

			ucb.itemID	 = USERCONTROL_LAYER_BEAM_BRACKET;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_BEAM_BRACKET, 1);

			ucb.itemID	 = USERCONTROL_LAYER_YOKE;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_YOKE, 1);

			DGSetItemText (dialogID, BUTTON_ADD, "열 추가");
			DGSetItemText (dialogID, BUTTON_DEL, "열 삭제");

			DGSetItemText (dialogID, LABEL_TOTAL_WIDTH, "전체 너비");
			DGSetItemText (dialogID, LABEL_EXPLANATION, "너비 방향\n수직재 간격은\n전체 너비보다\n작아야 함");
			DGSetItemText (dialogID, LABEL_TOTAL_LENGTH, "전체 길이");
			DGSetItemText (dialogID, LABEL_REMAIN_LENGTH, "남은 길이");

			// 초기 설정
			// 1. 타입 추가
			DGPopUpInsertItem (dialogID, POPUP_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_TYPE, DG_POPUP_BOTTOM, "강관 동바리");
			DGPopUpInsertItem (dialogID, POPUP_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_TYPE, DG_POPUP_BOTTOM, "PERI 동바리");
			DGPopUpInsertItem (dialogID, POPUP_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_TYPE, DG_POPUP_BOTTOM, "강관 동바리 + 산승각");
			DGPopUpInsertItem (dialogID, POPUP_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_TYPE, DG_POPUP_BOTTOM, "PERI 동바리 + 토류판");
			DGPopUpInsertItem (dialogID, POPUP_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_TYPE, DG_POPUP_BOTTOM, "PERI 동바리 + GT24 거더");
			DGPopUpInsertItem (dialogID, POPUP_TYPE, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_TYPE, DG_POPUP_BOTTOM, "PERI 동바리 + 보 멍에제");

			// 2. 크로스헤드 및 수직재 2단 비활성화 (강관 동바리의 경우)
			DGDisableItem (dialogID, CHECKBOX_CROSSHEAD);
			DGDisableItem (dialogID, CHECKBOX_VPOST2);

			// 3. 수직재 규격 팝업 추가
			// 강관 동바리의 경우 V0 (2.0m), V1 (3.2m), V2 (3.4m), V3 (3.8m), V4 (4.0m), V5 (5.0m), V6 (5.9m), V7 (0.5~2.0m)
			// PERI 동바리의 경우 MP 120, MP 250, MP 350, MP 480, MP 625
			DGPopUpInsertItem (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM, "V0 (2.0m)");
			DGPopUpInsertItem (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM, "V1 (3.2m)");
			DGPopUpInsertItem (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM, "V2 (3.4m)");
			DGPopUpInsertItem (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM, "V3 (3.8m)");
			DGPopUpInsertItem (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM, "V4 (4.0m)");
			DGPopUpInsertItem (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM, "V5 (5.0m)");
			DGPopUpInsertItem (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM, "V6 (5.9m)");
			DGPopUpInsertItem (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM, "V7 (0.5~2.0m)");

			DGPopUpInsertItem (dialogID, POPUP_VPOST2_NOMINAL, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_VPOST2_NOMINAL, DG_POPUP_BOTTOM, "MP 120");
			DGPopUpInsertItem (dialogID, POPUP_VPOST2_NOMINAL, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_VPOST2_NOMINAL, DG_POPUP_BOTTOM, "MP 250");
			DGPopUpInsertItem (dialogID, POPUP_VPOST2_NOMINAL, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_VPOST2_NOMINAL, DG_POPUP_BOTTOM, "MP 350");
			DGPopUpInsertItem (dialogID, POPUP_VPOST2_NOMINAL, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_VPOST2_NOMINAL, DG_POPUP_BOTTOM, "MP 480");
			DGPopUpInsertItem (dialogID, POPUP_VPOST2_NOMINAL, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_VPOST2_NOMINAL, DG_POPUP_BOTTOM, "MP 625");
			
			// 4. 수직재 높이 라벨 옆에 범위 표시
			// 강관 동바리의 경우 V0 (2.0m) (1200~2000), V1 (3.2m) (1800~3200), V2 (3.4m) (2000~3400), V3 (3.8m) (2400~3800), V4 (4.0m) (2600~4000), V5 (5.0m) (3000~5000), V6 (5.9m) (3200~5900), V7 (0.5~2.0m) (500~2000)
			// PERI 동바리의 경우 MP 120 (800~1200), MP 250 (1450~2500), MP 350 (1950~3500), MP 480 (2600~4800), MP 625 (4300~6250)
			DGSetItemText (dialogID, LABEL_VPOST1_HEIGHT, "H. 1200~2000");
			DGSetItemText (dialogID, LABEL_VPOST2_HEIGHT, "H. 800~1200");

			// 5. 수직재 2단 Off
			DGSetItemValLong (dialogID, CHECKBOX_VPOST2, FALSE);
			DGDisableItem (dialogID, POPUP_VPOST2_NOMINAL);
			DGDisableItem (dialogID, EDITCONTROL_VPOST2_HEIGHT);

			// 6. 수직재 높이 입력 범위 지정
			DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 1.200);
			DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 2.000);
			DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT, 0.800);
			DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT, 1.200);

			// 7. 각재, 크로스헤드 높이 지정
			DGSetItemText (dialogID, LABEL_TIMBER_HEIGHT, "0");
			DGSetItemText (dialogID, LABEL_CROSSHEAD_HEIGHT, "0");

			// 8. 총 높이, 남은 높이 비활성화 및 값 출력
			DGDisableItem (dialogID, EDITCONTROL_TOTAL_HEIGHT);
			DGDisableItem (dialogID, EDITCONTROL_REMAIN_HEIGHT);
			DGSetItemValDouble (dialogID, EDITCONTROL_TOTAL_HEIGHT, infoMorph.height);
			DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_HEIGHT, infoMorph.height - (DGGetItemValDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT) + DGGetItemValDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT) * DGGetItemValLong (dialogID, CHECKBOX_VPOST2)) - 0.003 * DGGetItemValLong (dialogID, CHECKBOX_CROSSHEAD));

			// 9. 전체 너비, 전체 길이 및 남은 길이
			DGSetItemValDouble (dialogID, EDITCONTROL_TOTAL_WIDTH, placementInfoForPERI.depth);
			DGDisableItem (dialogID, EDITCONTROL_TOTAL_WIDTH);
			DGSetItemValDouble (dialogID, EDITCONTROL_TOTAL_LENGTH, placementInfoForPERI.width);
			DGDisableItem (dialogID, EDITCONTROL_TOTAL_LENGTH);
			DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_LENGTH, placementInfoForPERI.width);
			DGDisableItem (dialogID, EDITCONTROL_REMAIN_LENGTH);

			// 10. 1열 수평재 선택 팝업컨트롤 추가
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 490, 230, 30, 30);
			DGShowItem (dialogID, itmIdx);
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 490, 400, 30, 30);
			DGShowItem (dialogID, itmIdx);
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 505, 261, 1, 138);
			DGShowItem (dialogID, itmIdx);
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 515, 290, 100, 25);
			HPOST_CENTER [0] = itmIdx;
			DGShowItem (dialogID, itmIdx);
			DGSetItemText (dialogID, itmIdx, "수평재");
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 200, 10, 515, 315, 70, 25);
			DGShowItem (dialogID, itmIdx);
			DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "없음");
			DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "625");
			DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "750");
			DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "900");
			DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "1200");
			DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "1375");
			DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "1500");
			DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "2015");
			DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "2250");
			DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "2300");
			DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "2370");
			DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "2660");
			DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "2960");
			DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "가변");
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 515, 340, 70, 25);
			DGShowItem (dialogID, itmIdx);

			// 11. 강관 동바리의 경우, 수평재는 비활성화됨
			DGDisableItem (dialogID, HPOST_CENTER [0]);
			DGDisableItem (dialogID, HPOST_CENTER [0]+1);

			break;

		case DG_MSG_CHANGE:
			// 1. 레이어 묶음 체크박스 On/Off에 따른 이벤트 처리
			if (DGGetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING) == 1) {
				switch (item) {
					case USERCONTROL_LAYER_SUPPORT:
						for (xx = USERCONTROL_LAYER_SUPPORT ; xx <= USERCONTROL_LAYER_YOKE ; ++xx)	DGSetItemValLong (dialogID, xx, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SUPPORT));
						break;
					case USERCONTROL_LAYER_VPOST:
						for (xx = USERCONTROL_LAYER_SUPPORT ; xx <= USERCONTROL_LAYER_YOKE ; ++xx)	DGSetItemValLong (dialogID, xx, DGGetItemValLong (dialogID, USERCONTROL_LAYER_VPOST));
						break;
					case USERCONTROL_LAYER_HPOST:
						for (xx = USERCONTROL_LAYER_SUPPORT ; xx <= USERCONTROL_LAYER_YOKE ; ++xx)	DGSetItemValLong (dialogID, xx, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HPOST));
						break;
					case USERCONTROL_LAYER_TIMBER:
						for (xx = USERCONTROL_LAYER_SUPPORT ; xx <= USERCONTROL_LAYER_YOKE ; ++xx)	DGSetItemValLong (dialogID, xx, DGGetItemValLong (dialogID, USERCONTROL_LAYER_TIMBER));
						break;
					case USERCONTROL_LAYER_GIRDER:
						for (xx = USERCONTROL_LAYER_SUPPORT ; xx <= USERCONTROL_LAYER_YOKE ; ++xx)	DGSetItemValLong (dialogID, xx, DGGetItemValLong (dialogID, USERCONTROL_LAYER_GIRDER));
						break;
					case USERCONTROL_LAYER_BEAM_BRACKET:
						for (xx = USERCONTROL_LAYER_SUPPORT ; xx <= USERCONTROL_LAYER_YOKE ; ++xx)	DGSetItemValLong (dialogID, xx, DGGetItemValLong (dialogID, USERCONTROL_LAYER_BEAM_BRACKET));
						break;
					case USERCONTROL_LAYER_YOKE:
						for (xx = USERCONTROL_LAYER_SUPPORT ; xx <= USERCONTROL_LAYER_YOKE ; ++xx)	DGSetItemValLong (dialogID, xx, DGGetItemValLong (dialogID, USERCONTROL_LAYER_YOKE));
						break;
				}
			}

			// 2. 타입에 따라 수직재, 수평재 설정이 바뀜

			// 3. 수직재 2단을 켜고 끌 때마다 수직재 2단의 규격, 높이 입력 컨트롤을 활성화/비활성화
			//if (DGGetItemValLong (dialogID, CHECKBOX_VPOST2) == TRUE) {
			//	DGEnableItem (dialogID, POPUP_VPOST2_NOMINAL);
			//	DGEnableItem (dialogID, EDITCONTROL_VPOST2_HEIGHT);
			//} else {
			//	DGDisableItem (dialogID, POPUP_VPOST2_NOMINAL);
			//	DGDisableItem (dialogID, EDITCONTROL_VPOST2_HEIGHT);
			//}

			// 4. 수직재 규격이 바뀔 때마다 높이 범위 문자열이 변경되고, 수직재 높이 값의 최소/최대값 변경됨
			//if (DGPopUpGetSelected (dialogID, POPUP_VPOST1_NOMINAL) == 1) {
			//	DGSetItemText (dialogID, LABEL_VPOST1_HEIGHT, "H. 800~1200");
			//	DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 0.800);
			//	DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 1.200);
			//} else if (DGPopUpGetSelected (dialogID, POPUP_VPOST1_NOMINAL) == 2) {
			//	DGSetItemText (dialogID, LABEL_VPOST1_HEIGHT, "H. 1450~2500");
			//	DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 1.450);
			//	DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 2.500);
			//} else if (DGPopUpGetSelected (dialogID, POPUP_VPOST1_NOMINAL) == 3) {
			//	DGSetItemText (dialogID, LABEL_VPOST1_HEIGHT, "H. 1950~3500");
			//	DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 1.950);
			//	DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 3.500);
			//} else if (DGPopUpGetSelected (dialogID, POPUP_VPOST1_NOMINAL) == 4) {
			//	DGSetItemText (dialogID, LABEL_VPOST1_HEIGHT, "H. 2600~4800");
			//	DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 2.600);
			//	DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 4.800);
			//} else if (DGPopUpGetSelected (dialogID, POPUP_VPOST1_NOMINAL) == 5) {
			//	DGSetItemText (dialogID, LABEL_VPOST1_HEIGHT, "H. 4300~6250");
			//	DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 4.300);
			//	DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 6.250);
			//}

			//if (DGPopUpGetSelected (dialogID, POPUP_VPOST2_NOMINAL) == 1) {
			//	DGSetItemText (dialogID, LABEL_VPOST2_HEIGHT, "H. 800~1200");
			//	DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT, 0.800);
			//	DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT, 1.200);
			//} else if (DGPopUpGetSelected (dialogID, POPUP_VPOST2_NOMINAL) == 2) {
			//	DGSetItemText (dialogID, LABEL_VPOST2_HEIGHT, "H. 1450~2500");
			//	DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT, 1.450);
			//	DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT, 2.500);
			//} else if (DGPopUpGetSelected (dialogID, POPUP_VPOST2_NOMINAL) == 3) {
			//	DGSetItemText (dialogID, LABEL_VPOST2_HEIGHT, "H. 1950~3500");
			//	DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT, 1.950);
			//	DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT, 3.500);
			//} else if (DGPopUpGetSelected (dialogID, POPUP_VPOST2_NOMINAL) == 4) {
			//	DGSetItemText (dialogID, LABEL_VPOST2_HEIGHT, "H. 2600~4800");
			//	DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT, 2.600);
			//	DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT, 4.800);
			//} else if (DGPopUpGetSelected (dialogID, POPUP_VPOST2_NOMINAL) == 5) {
			//	DGSetItemText (dialogID, LABEL_VPOST2_HEIGHT, "H. 4300~6250");
			//	DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT, 4.300);
			//	DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT, 6.250);
			//}

			// 5. 수직재 규격이 바뀌거나, 수직재 1단/2단 체크박스 상태가 바뀌거나, 수직재 높이가 바뀌거나, 크로스헤드 체크 상태에 따라 남은 높이 변경됨
			//DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_HEIGHT, infoMorph.height - (DGGetItemValDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT) * DGGetItemValLong (dialogID, CHECKBOX_VPOST1) + DGGetItemValDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT) * DGGetItemValLong (dialogID, CHECKBOX_VPOST2)) - 0.003 * DGGetItemValLong (dialogID, CHECKBOX_CROSSHEAD));

			// 6. 수평재 유무를 선택하거나, 수평재 크기를 변경하거나, 수평 거리를 변경할 때 남은 길이 변경됨 (아울러 위/아래 설정이 같이 변경되어야 함)
			// ...

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:

					//if (DGGetItemValLong (dialogID, CHECKBOX_VPOST1) == TRUE)
					//	placementInfoForPERI.bVPost1 = true;
					//else
					//	placementInfoForPERI.bVPost1 = false;
					//sprintf (placementInfoForPERI.nomVPost1, "%s", DGPopUpGetItemText (dialogID, POPUP_VPOST1_NOMINAL, DGPopUpGetSelected (dialogID, POPUP_VPOST1_NOMINAL)).ToCStr ().Get ());
					//placementInfoForPERI.heightVPost1 = DGGetItemValDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT);
					//
					//if (DGGetItemValLong (dialogID, CHECKBOX_VPOST2) == TRUE)
					//	placementInfoForPERI.bVPost2 = true;
					//else
					//	placementInfoForPERI.bVPost2 = false;
					//sprintf (placementInfoForPERI.nomVPost2, "%s", DGPopUpGetItemText (dialogID, POPUP_VPOST2_NOMINAL, DGPopUpGetSelected (dialogID, POPUP_VPOST2_NOMINAL)).ToCStr ().Get ());
					//placementInfoForPERI.heightVPost2 = DGGetItemValDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT);

					//if (DGGetItemValLong (dialogID, CHECKBOX_CROSSHEAD) == TRUE)
					//	placementInfoForPERI.bCrosshead = true;
					//else
					//	placementInfoForPERI.bCrosshead = false;

					//if (DGGetItemValLong (dialogID, CHECKBOX_HPOST) == TRUE)
					//	placementInfoForPERI.bHPost = true;
					//else
					//	placementInfoForPERI.bHPost = false;

					//sprintf (tempStr, "%s", DGPopUpGetItemText (dialogID, POPUP_WIDTH_NORTH, DGPopUpGetSelected (dialogID, POPUP_WIDTH_NORTH)).ToCStr ().Get ());
					//if (my_strcmp (tempStr, "없음") == 0)	strcpy (placementInfoForPERI.nomHPost_North, "");
					//if (my_strcmp (tempStr, "625") == 0)	strcpy (placementInfoForPERI.nomHPost_North, "62.5 cm");
					//if (my_strcmp (tempStr, "750") == 0)	strcpy (placementInfoForPERI.nomHPost_North, "75 cm");
					//if (my_strcmp (tempStr, "900") == 0)	strcpy (placementInfoForPERI.nomHPost_North, "90 cm");
					//if (my_strcmp (tempStr, "1200") == 0)	strcpy (placementInfoForPERI.nomHPost_North, "120 cm");
					//if (my_strcmp (tempStr, "1375") == 0)	strcpy (placementInfoForPERI.nomHPost_North, "137.5 cm");
					//if (my_strcmp (tempStr, "1500") == 0)	strcpy (placementInfoForPERI.nomHPost_North, "150 cm");
					//if (my_strcmp (tempStr, "2015") == 0)	strcpy (placementInfoForPERI.nomHPost_North, "201.5 cm");
					//if (my_strcmp (tempStr, "2250") == 0)	strcpy (placementInfoForPERI.nomHPost_North, "225 cm");
					//if (my_strcmp (tempStr, "2300") == 0)	strcpy (placementInfoForPERI.nomHPost_North, "230 cm");
					//if (my_strcmp (tempStr, "2370") == 0)	strcpy (placementInfoForPERI.nomHPost_North, "237 cm");
					//if (my_strcmp (tempStr, "2660") == 0)	strcpy (placementInfoForPERI.nomHPost_North, "266 cm");
					//if (my_strcmp (tempStr, "2960") == 0)	strcpy (placementInfoForPERI.nomHPost_North, "296 cm");

					//sprintf (tempStr, "%s", DGPopUpGetItemText (dialogID, POPUP_WIDTH_WEST, DGPopUpGetSelected (dialogID, POPUP_WIDTH_WEST)).ToCStr ().Get ());
					//if (my_strcmp (tempStr, "없음") == 0)	strcpy (placementInfoForPERI.nomHPost_West, "");
					//if (my_strcmp (tempStr, "625") == 0)	strcpy (placementInfoForPERI.nomHPost_West, "62.5 cm");
					//if (my_strcmp (tempStr, "750") == 0)	strcpy (placementInfoForPERI.nomHPost_West, "75 cm");
					//if (my_strcmp (tempStr, "900") == 0)	strcpy (placementInfoForPERI.nomHPost_West, "90 cm");
					//if (my_strcmp (tempStr, "1200") == 0)	strcpy (placementInfoForPERI.nomHPost_West, "120 cm");
					//if (my_strcmp (tempStr, "1375") == 0)	strcpy (placementInfoForPERI.nomHPost_West, "137.5 cm");
					//if (my_strcmp (tempStr, "1500") == 0)	strcpy (placementInfoForPERI.nomHPost_West, "150 cm");
					//if (my_strcmp (tempStr, "2015") == 0)	strcpy (placementInfoForPERI.nomHPost_West, "201.5 cm");
					//if (my_strcmp (tempStr, "2250") == 0)	strcpy (placementInfoForPERI.nomHPost_West, "225 cm");
					//if (my_strcmp (tempStr, "2300") == 0)	strcpy (placementInfoForPERI.nomHPost_West, "230 cm");
					//if (my_strcmp (tempStr, "2370") == 0)	strcpy (placementInfoForPERI.nomHPost_West, "237 cm");
					//if (my_strcmp (tempStr, "2660") == 0)	strcpy (placementInfoForPERI.nomHPost_West, "266 cm");
					//if (my_strcmp (tempStr, "2960") == 0)	strcpy (placementInfoForPERI.nomHPost_West, "296 cm");

					//sprintf (tempStr, "%s", DGPopUpGetItemText (dialogID, POPUP_WIDTH_EAST, DGPopUpGetSelected (dialogID, POPUP_WIDTH_EAST)).ToCStr ().Get ());
					//if (my_strcmp (tempStr, "없음") == 0)	strcpy (placementInfoForPERI.nomHPost_East, "");
					//if (my_strcmp (tempStr, "625") == 0)	strcpy (placementInfoForPERI.nomHPost_East, "62.5 cm");
					//if (my_strcmp (tempStr, "750") == 0)	strcpy (placementInfoForPERI.nomHPost_East, "75 cm");
					//if (my_strcmp (tempStr, "900") == 0)	strcpy (placementInfoForPERI.nomHPost_East, "90 cm");
					//if (my_strcmp (tempStr, "1200") == 0)	strcpy (placementInfoForPERI.nomHPost_East, "120 cm");
					//if (my_strcmp (tempStr, "1375") == 0)	strcpy (placementInfoForPERI.nomHPost_East, "137.5 cm");
					//if (my_strcmp (tempStr, "1500") == 0)	strcpy (placementInfoForPERI.nomHPost_East, "150 cm");
					//if (my_strcmp (tempStr, "2015") == 0)	strcpy (placementInfoForPERI.nomHPost_East, "201.5 cm");
					//if (my_strcmp (tempStr, "2250") == 0)	strcpy (placementInfoForPERI.nomHPost_East, "225 cm");
					//if (my_strcmp (tempStr, "2300") == 0)	strcpy (placementInfoForPERI.nomHPost_East, "230 cm");
					//if (my_strcmp (tempStr, "2370") == 0)	strcpy (placementInfoForPERI.nomHPost_East, "237 cm");
					//if (my_strcmp (tempStr, "2660") == 0)	strcpy (placementInfoForPERI.nomHPost_East, "266 cm");
					//if (my_strcmp (tempStr, "2960") == 0)	strcpy (placementInfoForPERI.nomHPost_East, "296 cm");

					//sprintf (tempStr, "%s", DGPopUpGetItemText (dialogID, POPUP_WIDTH_SOUTH, DGPopUpGetSelected (dialogID, POPUP_WIDTH_SOUTH)).ToCStr ().Get ());
					//if (my_strcmp (tempStr, "없음") == 0)	strcpy (placementInfoForPERI.nomHPost_South, "");
					//if (my_strcmp (tempStr, "625") == 0)	strcpy (placementInfoForPERI.nomHPost_South, "62.5 cm");
					//if (my_strcmp (tempStr, "750") == 0)	strcpy (placementInfoForPERI.nomHPost_South, "75 cm");
					//if (my_strcmp (tempStr, "900") == 0)	strcpy (placementInfoForPERI.nomHPost_South, "90 cm");
					//if (my_strcmp (tempStr, "1200") == 0)	strcpy (placementInfoForPERI.nomHPost_South, "120 cm");
					//if (my_strcmp (tempStr, "1375") == 0)	strcpy (placementInfoForPERI.nomHPost_South, "137.5 cm");
					//if (my_strcmp (tempStr, "1500") == 0)	strcpy (placementInfoForPERI.nomHPost_South, "150 cm");
					//if (my_strcmp (tempStr, "2015") == 0)	strcpy (placementInfoForPERI.nomHPost_South, "201.5 cm");
					//if (my_strcmp (tempStr, "2250") == 0)	strcpy (placementInfoForPERI.nomHPost_South, "225 cm");
					//if (my_strcmp (tempStr, "2300") == 0)	strcpy (placementInfoForPERI.nomHPost_South, "230 cm");
					//if (my_strcmp (tempStr, "2370") == 0)	strcpy (placementInfoForPERI.nomHPost_South, "237 cm");
					//if (my_strcmp (tempStr, "2660") == 0)	strcpy (placementInfoForPERI.nomHPost_South, "266 cm");
					//if (my_strcmp (tempStr, "2960") == 0)	strcpy (placementInfoForPERI.nomHPost_South, "296 cm");

					//layerInd_vPost	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_VPOST);
					//layerInd_hPost	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_HPOST);

					break;

				case DG_CANCEL:
					break;

				case BUTTON_ADD:
					item = 0;

					if (placementInfoForPERI.nColVPost < 5) {
						placementInfoForPERI.nColVPost ++;
						DGRemoveDialogItems (dialogID, AFTER_ALL + 6);
						DGGrowDialog (dialogID, 130, 0);

						// ...
						// 구분자/수평재 관련 컴포넌트 그리기
						for (xx = 2 ; xx <= placementInfoForPERI.nColVPost ; ++xx) {
							itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 521-260+(130*xx), 245, 100, 1);
							DGShowItem (dialogID, itmIdx);
							itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 521-260+(130*xx), 415, 100, 1);
							DGShowItem (dialogID, itmIdx);

							itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 490-130+(130*xx), 230, 30, 30);
							DGShowItem (dialogID, itmIdx);
							itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 490-130+(130*xx), 400, 30, 30);
							DGShowItem (dialogID, itmIdx);
							itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 505-130+(130*xx), 261, 1, 138);
							DGShowItem (dialogID, itmIdx);
						}

						//itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 515, 290, 100, 25);
						//HPOST_CENTER [0] = itmIdx;
						//DGShowItem (dialogID, itmIdx);
						//DGSetItemText (dialogID, itmIdx, "수평재");
						//itmIdx = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 200, 10, 515, 315, 70, 25);
						//DGShowItem (dialogID, itmIdx);
						//DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
						//DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "없음");
						//DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
						//DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "625");
						//DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
						//DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "750");
						//DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
						//DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "900");
						//DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
						//DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "1200");
						//DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
						//DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "1375");
						//DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
						//DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "1500");
						//DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
						//DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "2015");
						//DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
						//DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "2250");
						//DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
						//DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "2300");
						//DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
						//DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "2370");
						//DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
						//DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "2660");
						//DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
						//DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "2960");
						//DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
						//DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "가변");
						//itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 515, 340, 70, 25);
						//DGShowItem (dialogID, itmIdx);
					}

					break;

				case BUTTON_DEL:
					item = 0;

					if (placementInfoForPERI.nColVPost > 1) {
						placementInfoForPERI.nColVPost --;
						DGRemoveDialogItems (dialogID, AFTER_ALL + 6);
						DGGrowDialog (dialogID, -130, 0);

						// ...
						// 구분자/수평재 관련 컴포넌트 그리기
						for (xx = 2 ; xx <= placementInfoForPERI.nColVPost ; ++xx) {
							itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 521-260+(130*xx), 245, 100, 1);
							DGShowItem (dialogID, itmIdx);
							itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 521-260+(130*xx), 415, 100, 1);
							DGShowItem (dialogID, itmIdx);

							itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 490-130+(130*xx), 230, 30, 30);
							DGShowItem (dialogID, itmIdx);
							itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 490-130+(130*xx), 400, 30, 30);
							DGShowItem (dialogID, itmIdx);
							itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 505-130+(130*xx), 261, 1, 138);
							DGShowItem (dialogID, itmIdx);
						}
					}

					break;
			}
		case DG_MSG_CLOSE:
			switch (item) {
				case DG_CLOSEBOX:
					break;
			}
	}

	result = item;

	return	result;
}