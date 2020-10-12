#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "SlabEuroformPlacer.hpp"


// 2번 메뉴: 슬래브 하부에 유로폼을 배치하는 통합 루틴
GSErrCode	placeEuroformOnSlabBottom (void)
{
	GSErrCode	err = NoError;
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

	// 모프 객체 정보
	InfoMorphForSlab		infoMorph;

	// 작업 층 정보
	API_StoryInfo	storyInfo;
	double			minusLevel;


	err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, true);	// 선택한 요소 가져오기
	BMKillHandle ((GSHandle *) &selectionInfo.marquee.coords);
	if (err == APIERR_NOPLAN) {
		ACAPI_WriteReport ("열린 프로젝트 창이 없습니다.", true);
		return err;
	}
	if (err == APIERR_NOSEL) {
		ACAPI_WriteReport ("아무 것도 선택하지 않았습니다.\n필수 선택: 슬래브 하부를 덮는 모프 (1개)", true);
		return err;
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
		ACAPI_WriteReport ("슬래브 하부를 덮는 모프를 1개 선택하셔야 합니다.", true);
		err = APIERR_GENERAL;
		return err;
	}

	// 모프 정보를 가져옴
	BNZeroMemory (&elem, sizeof (API_Element));
	elem.header.guid = morphs.Pop ();
	err = ACAPI_Element_Get (&elem);
	err = ACAPI_Element_Get3DInfo (elem.header, &info3D);

	// 만약 모프가 누워 있어야 함
	if (abs (info3D.bounds.zMax - info3D.bounds.zMin) > EPS) {
		ACAPI_WriteReport ("모프가 누워 있지 않습니다.", true);
		err = APIERR_GENERAL;
		return err;
	}

	// 모프의 GUID 저장
	infoMorph.guid = elem.header.guid;




	// ... 모프의 3D 바디를 먼저 가져옴
	API_Component3D		component;
	API_Tranmat			tm;
	short				j;
	Int32				nVert, nEdge, nPgon;
	Int32				elemIdx, bodyIdx;

	BNZeroMemory (&component, sizeof (API_Component3D));
	component.header.typeID = API_BodyID;
	component.header.index = info3D.fbody;
	err = ACAPI_3D_GetComponent (&component);

	if (err != NoError)
		return err;

	nVert = component.body.nVert;
	nEdge = component.body.nEdge;
	nPgon = component.body.nPgon;
	tm = component.body.tranmat;
	elemIdx = component.body.head.elemIndex - 1;
	bodyIdx = component.body.head.bodyIndex - 1;
	std::string	tempString;
	
	err = ACAPI_CallUndoableCommand ("테스트", [&] () -> GSErrCode {
		for (j = 1; j <= nVert; j++) {
			component.header.typeID = API_VertID;
			component.header.index  = j;
			err = ACAPI_3D_GetComponent (&component);
			if (err == NoError) {
				API_Coord3D	trCoord;	// world coordinates
				tempString = format_string ("%d", j);
				trCoord.x = tm.tmx[0]*component.vert.x + tm.tmx[1]*component.vert.y + tm.tmx[2]*component.vert.z + tm.tmx[3];
				trCoord.y = tm.tmx[4]*component.vert.x + tm.tmx[5]*component.vert.y + tm.tmx[6]*component.vert.z + tm.tmx[7];
				trCoord.z = tm.tmx[8]*component.vert.x + tm.tmx[9]*component.vert.y + tm.tmx[10]*component.vert.z + tm.tmx[11];
				if (abs (trCoord.z - elem.morph.level) < EPS)
					placeCoordinateLabel (trCoord.x, trCoord.y, trCoord.z, true, tempString, 1, 0);
			}
		}
		
		return NoError;
	});

	// ... 좌하단을 찾아야 한다. 위 반복문의 순서는 폴리곤 정점 순서랑 무관함
	// ... 영역의 너비, 높이를 구해야 한다.




	/*
		* 사용 부재(3가지): 유로폼(회전X: 0도, 벽세우기로 고정), 합판(각도: 90도), 목재(설치방향: 바닥눕히기)
		
		1. 모프를 1개 그려야 함 (실행 전 사전 작업할 부분)
			- 직사각형이 아님 (마법봉 또는 하나씩 점을 찍어서 그려야 함)
		2. 모프 영역 설정
			- 좌하단 점을 찍으라고 사용자에게 요청함
			- 모프의 폴리곤 좌표를 가져와야 함
		3. 작업 층 높이 반영
		4. 모든 영역 정보의 Z 정보를 수정
		5. 사용자 입력 (1차)
			- 기본 배치 폼 선택
				: 규격폼 기준 -- 너비, 높이 (방향은 사용자가 찍은 모프 좌하단 점을 기준으로 함)
				: 설치방향을 어떤 기준으로 정할 것인가? - 모프의 처음 찍은 점(TMX 점 인식)을 좌하단 점이라고 가정함
				: 최하단 점에 기둥이나 보의 간섭이 있으면 어떻게 할 것인가? -> 구간별 길이를 측정해서 처음 긴 선을 아래쪽 라인이라고 하자. -> 아래쪽 긴 라인의 시작/끝 점을 기준으로 각도값 획득
			- 부재별 레이어 설정 (유로폼, 합판, 목재)
			- 유로폼 셀은 전부 붙어 있어도 됨
			- 목재 보강재 위치 셀 버튼도 있어야 함
		6. 사용자 입력 (2차)
			- 타이틀: 가로/세로 채우기
			- 버튼: 1. 남은 길이 확인 // 2. 배  치 // 3. 마무리
			- 배치 버튼을 보여줌 (x번행 y번열 버튼의 폼 크기를 변경하면? - 너비를 바꾸면 y열 전체에 영향을 줌, 높이를 바꾸면 x행 전체에 영향을 줌)
			- 남은 가로/세로 길이: 표시만 함 (양쪽 다 합친 것): 권장 남은 길이 표시 (150~300mm), 권장 길이인지 아닌지 글꼴로 표시
			- 사방 합판 너비를 조정할 수 있어야 함 (위/아래 배분, 왼쪽/오른쪽 배분 - 사용자가 직접.. 그에 따라 유로폼 전체 배열이 이동됨)
			- 배치 버튼 사방에는 버튼 사이마다 보강 목재를 댈지 여부를 선택할 수 있어야 함 (버튼 □■)
		7. 최종 배치
			- (1) 유로폼은 모프 센터를 기준으로 배치할 것
			- (2) 합판(11.5T, 제작틀 Off)을 모프에 맞게 배치하되, 겹치는 부분은 모깎기가 필요함! (난이도 높음)
			- (3) 안쪽에 목재를 댈 것 (Z방향 두께는 50이어야 함, 너비는 80)
			- (4) 목재 보강재도 댈 것 (안쪽 목재와 두께, 너비는 동일 - 길이는 사용자 설정을 따름)
	*/

	return	err;
}