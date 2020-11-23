#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "ColumnEuroformPlacer.hpp"

using namespace columnPlacerDG;

// 5번 메뉴: 기둥에 유로폼을 배치하는 통합 루틴
GSErrCode	placeEuroformOnColumn (void)
{
	GSErrCode		err = NoError;
	long			nSel;
	short			xx, yy;
	double			dx, dy, ang;
	short			result;

	// Selection Manager 관련 변수
	API_SelectionInfo		selectionInfo;
	API_Element				tElem;
	API_Neig				**selNeigs;
	GS::Array<API_Guid>&	morphs = GS::Array<API_Guid> ();
	GS::Array<API_Guid>&	columns = GS::Array<API_Guid> ();
	GS::Array<API_Guid>&	beams = GS::Array<API_Guid> ();
	long					nMorphs = 0;
	long					nColumns = 0;
	long					nBeams = 0;

	// 객체 정보 가져오기
	API_Element				elem;
	API_ElementMemo			memo;
	API_ElemInfo3D			info3D;

	// 모프 3D 구성요소 가져오기
	API_Component3D			component;
	API_Tranmat				tm;
	Int32					nVert, nEdge, nPgon;
	Int32					elemIdx, bodyIdx;
	API_Coord3D				trCoord;
	GS::Array<API_Coord3D>&	coords = GS::Array<API_Coord3D> ();

	// 모프 객체 정보
	//InfoMorphForColumn		infoMorph;

	// 작업 층 정보
	API_StoryInfo			storyInfo;
	double					workLevel_column;


	// 선택한 요소 가져오기
	err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, true);
	BMKillHandle ((GSHandle *) &selectionInfo.marquee.coords);
	if (err == APIERR_NOPLAN) {
		ACAPI_WriteReport ("열린 프로젝트 창이 없습니다.", true);
		return err;
	}
	if (err == APIERR_NOSEL) {
		ACAPI_WriteReport ("아무 것도 선택하지 않았습니다.\n필수 선택: 기둥 (1개), 기둥 측면을 덮는 모프 (1개)\n옵션 선택: 기둥과 맞닿는 보 (다수)", true);
		return err;
	}
	if (err != NoError) {
		BMKillHandle ((GSHandle *) &selNeigs);
		return err;
	}

	// 메인 기둥 1개 선택해야 함
	if (selectionInfo.typeID != API_SelEmpty) {
		nSel = BMGetHandleSize ((GSHandle) selNeigs) / sizeof (API_Neig);
		for (xx = 0 ; xx < nSel && err == NoError ; ++xx) {
			tElem.header.typeID = Neig_To_ElemID ((*selNeigs)[xx].neigID);

			tElem.header.guid = (*selNeigs)[xx].guid;
			if (ACAPI_Element_Get (&tElem) != NoError)	// 가져올 수 있는 요소인가?
				continue;

			if (tElem.header.typeID == API_ColumnID)	// 기둥인가?
				morphs.Push (tElem.header.guid);

			if (tElem.header.typeID == API_MorphID)		// 모프인가?
				morphs.Push (tElem.header.guid);

			if (tElem.header.typeID == API_BeamID)		// 보인가?
				beams.Push (tElem.header.guid);
		}
	}
	BMKillHandle ((GSHandle *) &selNeigs);
	nColumns = columns.GetSize ();
	nMorphs = morphs.GetSize ();
	nBeams = beams.GetSize ();

	// 기둥이 1개인가?
	if (nColumns != 1) {
		ACAPI_WriteReport ("기둥을 1개 선택해야 합니다.", true);
		err = APIERR_GENERAL;
		return err;
	}

	// 모프가 1개인가?
	if (nMorphs != 1) {
		ACAPI_WriteReport ("기둥 측면을 덮는 모프를 1개 선택하셔야 합니다.", true);
		err = APIERR_GENERAL;
		return err;
	}

	// 기둥 정보 저장: 각도, 너비A/B, 높이
	// 모프 정보 저장
	// 보 정보 저장: 각도, 하부 고도, 좌표(기둥과 상대적인 위치를 통해 동/서/남/북을 찾으면 됨)...

	// 부재 정보를 통해 영역 설정 값을 저장함

	/*
	설치할 부재: 유로폼, 아웃코너판넬, 인코너판넬, 매직바, 매직인코너, 합판
	유로폼 최상단 라인과 보 하단 라인 간의 간격은 80mm 이상 (UI에서 유효하지 않은 여백이 나오면 텍스트로 경고!)

	- 메인 기둥과 보들과의 관계를 파악
		- 모든 보의 고도를 찾는다.
		- 그 중 제일 낮은 보의 고도를 찾아야 함
	- 1차 DG (보 단면)
		- 벽에 매립된 상황인지 아닌지를 그림으로 표현할 것 (예시. 기둥 너비: 600, 벽 두께: 200)
			- (CASE.1) 단독 보
			- (CASE.2) 매립X, 벽면과 기둥면이 맞닿음
			- (CASE.3) 기둥의 일부가 벽에 매립됨: 기둥 500이 벽 밖으로 튀어나옴
			- (CASE.4) 매립O, 벽면과 기둥면 일치: 기둥 400이 벽 밖으로 튀어나옴
			- (CASE.5) 벽 관통: 벽 양쪽으로 200씩 기둥이 돌출됨
			- 부재 치수(기둥 너비, 벽 두께, 매립/노출 깊이)를 보여주고... 케이스별 각 세그먼트 너비를 사용자가 지정할 것!
		- 기둥 코너: 아웃코너판넬 (기둥 코너가 벽 속에 들어가면 생략)
		- 벽과 맞닿는 부분: 인코너판넬 (벽이 돌출된 경우에만)
		- 유로폼: 기둥 가운데, 그리고 벽면 (기둥 반쪽이 벽속에 매립된 경우)
	- 2차 DG (보 측면)
		- 가장 낮은 보와 유로폼 간의 여백을 표현해야 함 (80mm 이상이어야 함)
		- 모든 방향은 동일한 높이의 유로폼이 적용됨
	*/

	return	err;
}