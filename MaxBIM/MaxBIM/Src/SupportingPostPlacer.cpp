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
	API_ElementMemo			memo;
	API_ElemInfo3D			info3D;

	// 모프 3D 구성요소 가져오기
	API_Component3D			component;
	API_Tranmat				tm;
	Int32					nVert, nEdge, nPgon;
	Int32					elemIdx, bodyIdx;
	API_Coord3D				trCoord;
	GS::Array<API_Coord3D>&	coords = GS::Array<API_Coord3D> ();
	long					nNodes;
	API_Coord3D				point3D;

	// 작업 층 정보
	API_StoryInfo	storyInfo;
	double			workLevel_morph;	// 모프의 작업 층 높이


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

	// [다이얼로그] 수직재 단수 (1/2단, 높이가 6미터 초과되면 2단 권유할 것), 수직재의 규격/높이, 수평재 유무(단, 높이가 3500 이상이면 추가할 것을 권유할 것), 수평재 너비, 크로스헤드 유무, 수직재/수평재 레이어
	result = DGModalDialog (ACAPI_GetOwnResModule (), 32521, ACAPI_GetOwnResModule (), PERISupportingPostPlacerHandler1, 0);

	// 영역 모프 제거
	API_Elem_Head* headList = new API_Elem_Head [1];
	headList [0] = elem.header;
	err = ACAPI_Element_Delete (&headList, 1);
	delete headList;

	// 입력한 데이터를 기반으로 수직재, 수평재 배치
	if (result == DG_OK) {
		// ... placementInfoForPERI
		// 1. 수직재 1단 배치 (2단 없으면 크로스헤드 유무 고려)
		// 2. 수직재 2단 배치 (단, 옵션이 On일 때에만, 크로스헤드 유무 고려)
		// 3. 수평재 유무 고려
		// 3-1. 수평재 On이면, 수평재 (서쪽/동쪽) 배치 (단, 없음이 아닐 경우)
		// 3-2. 수평재 On이면, 수평재 (북쪽/남쪽) 배치 (단, 없음이 아닐 경우)

		//bool	bVPost1;				// 수직재 1단 유무
		//char	nomVPost1 [16];			// 수직재 1단 규격 - GDL의 수직재 규격과 동일
		//double	heightVPost1;			// 수직재 1단 높이

		//bool	bVPost2;				// 수직재 2단 유무
		//char	nomVPost2 [16];			// 수직재 2단 규격 - GDL의 수직재 규격과 동일
		//double	heightVPost2;			// 수직재 2단 높이

		//bool	bCrosshead;				// 크로스헤드 유무

		//bool	bHPost;					// 수평재 유무
		//char	nomHPost_North [16];	// 수평재 너비(북) - GDL의 수평재 규격과 동일, 없음의 경우 빈 문자열
		//char	nomHPost_West [16];		// 수평재 너비(서) - GDL의 수평재 규격과 동일, 없음의 경우 빈 문자열
		//char	nomHPost_East [16];		// 수평재 너비(동) - GDL의 수평재 규격과 동일, 없음의 경우 빈 문자열
		//char	nomHPost_South [16];	// 수평재 너비(남) - GDL의 수평재 규격과 동일, 없음의 경우 빈 문자열
	}

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

	//setParameterByName (&memo, "bolt_len", params.boltLen);		// 볼트 길이
	//setParameterByName (&memo, "bolt_dia", 0.010);				// 볼트 직경
	//setParameterByName (&memo, "washer_pos", 0.050);			// 와셔 위치
	//setParameterByName (&memo, "washer_size", 0.100);			// 와셔 크기
	//setParameterByName (&memo, "angX", params.angX);			// X축 회전
	//setParameterByName (&memo, "angY", params.angY);			// Y축 회전

	//char	stType [16];		// 규격
	//bool	bCrosshead;			// 크로스헤드 On/Off
	//char	posCrosshead [16];	// 크로스헤드 위치 (상단, 하단)
	//char	crossheadType [16];	// 크로스헤드 타입 (PERI, 당사 제작품)
	//double	angCrosshead;		// 크로스헤드 회전각도
	//double	len_current;		// 현재 길이
	//double	pos_lever;			// 레버 위치
	//double	angY;				// 회전 Y

	//bool	text2_onoff;		// 2D 텍스트 On/Off
	//bool	text_onoff;			// 3D 텍스트 On/Off
	//bool	bShowCoords;		// 좌표값 표시 On/Off

	// 객체 배치
	ACAPI_Element_Create (&elem, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

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

	//setParameterByName (&memo, "bolt_len", params.boltLen);		// 볼트 길이
	//setParameterByName (&memo, "bolt_dia", 0.010);				// 볼트 직경
	//setParameterByName (&memo, "washer_pos", 0.050);			// 와셔 위치
	//setParameterByName (&memo, "washer_size", 0.100);			// 와셔 크기
	//setParameterByName (&memo, "angX", params.angX);			// X축 회전
	//setParameterByName (&memo, "angY", params.angY);			// Y축 회전

	//char	stType [16];		// 규격
	//double	angX;				// 회전 X
	//double	angY;				// 회전 Y

	// 객체 배치
	ACAPI_Element_Create (&elem, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	elem.header.guid;
}

// 수직재 단수 (1/2단, 높이가 6미터 초과되면 2단 권유할 것), 수직재의 규격/높이, 수평재 유무(단, 높이가 3500 이상이면 추가할 것을 권유할 것), 수평재 너비, 크로스헤드 유무, 수직재/수평재 레이어를 설정
short DGCALLBACK PERISupportingPostPlacerHandler1 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	char	tempStr [16];
	API_UCCallbackType	ucb;

	short	itmIdx;

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
			DGSetItemText (dialogID, LABEL_VPOST, "수직재");				// 라벨: 수직재
			DGSetItemText (dialogID, LABEL_HPOST, "수평재");				// 라벨: 수평재

			DGSetItemText (dialogID, LABEL_TOTAL_HEIGHT, "총 높이");		// 라벨: 총 높이
			DGSetItemText (dialogID, LABEL_REMAIN_HEIGHT, "남은 높이");		// 라벨: 남은 높이

			DGSetItemText (dialogID, CHECKBOX_CROSSHEAD, "크로스헤드");		// 체크박스: 크로스헤드

			DGSetItemText (dialogID, CHECKBOX_VPOST1, "1단");				// 체크박스: 1단
			DGSetItemText (dialogID, LABEL_VPOST1_NOMINAL, "규격");			// 라벨: 규격
			DGSetItemText (dialogID, LABEL_VPOST1_HEIGHT, "높이");			// 라벨: 높이
			DGSetItemText (dialogID, CHECKBOX_VPOST2, "2단");				// 체크박스: 2단
			DGSetItemText (dialogID, LABEL_VPOST2_NOMINAL, "규격");			// 라벨: 규격
			DGSetItemText (dialogID, LABEL_VPOST2_HEIGHT, "높이");			// 라벨: 높이

			DGSetItemText (dialogID, CHECKBOX_HPOST, "수평재");				// 체크박스: 수평재

			DGSetItemText (dialogID, LABEL_PLAN_WIDTH, "가로");				// 라벨: 가로
			DGSetItemText (dialogID, LABEL_PLAN_DEPTH, "세로");				// 라벨: 세로

			DGSetItemText (dialogID, LABEL_WIDTH_NORTH, "너비(북)");		// 라벨: 너비(북)
			DGSetItemText (dialogID, LABEL_WIDTH_WEST, "너비(서)");			// 라벨: 너비(서)
			DGSetItemText (dialogID, LABEL_WIDTH_EAST, "너비(동)");			// 라벨: 너비(동)
			DGSetItemText (dialogID, LABEL_WIDTH_SOUTH, "너비(남)");		// 라벨: 너비(남)

			DGSetItemText (dialogID, LABEL_LAYER_SETTINGS, "부재별 레이어 설정");
			DGSetItemText (dialogID, LABEL_LAYER_VPOST, "수직재");
			DGSetItemText (dialogID, LABEL_LAYER_HPOST, "수평재");

			// 체크박스: 레이어 묶음
			DGSetItemText (dialogID, CHECKBOX_LAYER_COUPLING, "레이어 묶음");
			DGSetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING, TRUE);

			// 유저 컨트롤 초기화
			BNZeroMemory (&ucb, sizeof (ucb));
			ucb.dialogID = dialogID;
			ucb.type	 = APIUserControlType_Layer;
			ucb.itemID	 = USERCONTROL_LAYER_VPOST;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_VPOST, 1);

			ucb.itemID	 = USERCONTROL_LAYER_HPOST;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_HPOST, 1);

			// 초기 설정
			// 1. 수직재 규격 팝업 추가 - MP 120, MP 250, MP 350, MP 480, MP 625
			DGPopUpInsertItem (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM, "MP 120");
			DGPopUpInsertItem (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM, "MP 250");
			DGPopUpInsertItem (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM, "MP 350");
			DGPopUpInsertItem (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM, "MP 480");
			DGPopUpInsertItem (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_VPOST1_NOMINAL, DG_POPUP_BOTTOM, "MP 625");

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
			
			// 2. 수직재 높이 라벨 옆에 범위 표시 - MP 120 (800~1200), MP 250 (1450~2500), MP 350 (1950~3500), MP 480 (2600~4800), MP 625 (4300~6250)
			DGSetItemText (dialogID, LABEL_VPOST1_HEIGHT, "H. 800~1200");
			DGSetItemText (dialogID, LABEL_VPOST2_HEIGHT, "H. 800~1200");

			// 3. 수직재 1단 On, 2단 Off
			DGSetItemValLong (dialogID, CHECKBOX_VPOST1, TRUE);
			DGDisableItem (dialogID, CHECKBOX_VPOST1);
			DGSetItemValLong (dialogID, CHECKBOX_VPOST2, FALSE);
			DGDisableItem (dialogID, POPUP_VPOST2_NOMINAL);
			DGDisableItem (dialogID, EDITCONTROL_VPOST2_HEIGHT);

			// 4. 수평재의 너비 4개 비활성화
			DGSetItemValLong (dialogID, CHECKBOX_HPOST, FALSE);
			DGDisableItem (dialogID, LABEL_WIDTH_NORTH);
			DGDisableItem (dialogID, LABEL_WIDTH_SOUTH);
			DGDisableItem (dialogID, LABEL_WIDTH_WEST);
			DGDisableItem (dialogID, LABEL_WIDTH_EAST);
			DGDisableItem (dialogID, POPUP_WIDTH_NORTH);
			DGDisableItem (dialogID, POPUP_WIDTH_SOUTH);
			DGDisableItem (dialogID, POPUP_WIDTH_WEST);
			DGDisableItem (dialogID, POPUP_WIDTH_EAST);

			// 5. 수평재의 너비 팝업 추가 - 없음, 625, 750, 900, 1200, 1375, 1500, 2015, 2250, 2300, 2370, 2660, 2960
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM, "없음");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM, "625");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM, "750");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM, "900");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM, "1200");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM, "1375");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM, "1500");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM, "2015");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM, "2250");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM, "2300");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM, "2370");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM, "2660");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_NORTH, DG_POPUP_BOTTOM, "2960");

			DGPopUpInsertItem (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM, "없음");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM, "625");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM, "750");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM, "900");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM, "1200");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM, "1375");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM, "1500");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM, "2015");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM, "2250");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM, "2300");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM, "2370");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM, "2660");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_SOUTH, DG_POPUP_BOTTOM, "2960");

			DGPopUpInsertItem (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM, "없음");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM, "625");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM, "750");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM, "900");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM, "1200");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM, "1375");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM, "1500");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM, "2015");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM, "2250");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM, "2300");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM, "2370");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM, "2660");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_WEST, DG_POPUP_BOTTOM, "2960");

			DGPopUpInsertItem (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM, "없음");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM, "625");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM, "750");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM, "900");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM, "1200");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM, "1375");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM, "1500");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM, "2015");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM, "2250");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM, "2300");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM, "2370");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM, "2660");
			DGPopUpInsertItem (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, POPUP_WIDTH_EAST, DG_POPUP_BOTTOM, "2960");

			// 6. 수직재 높이 입력 범위 지정
			DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 0.800);
			DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 1.200);
			DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT, 0.800);
			DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT, 1.200);

			// 7. 총 높이, 남은 높이 비활성화 및 값 출력
			DGDisableItem (dialogID, EDITCONTROL_TOTAL_HEIGHT);
			DGDisableItem (dialogID, EDITCONTROL_REMAIN_HEIGHT);
			DGSetItemValDouble (dialogID, EDITCONTROL_TOTAL_HEIGHT, infoMorph.height);
			DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_HEIGHT, infoMorph.height - (DGGetItemValDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT) * DGGetItemValLong (dialogID, CHECKBOX_VPOST1) + DGGetItemValDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT) * DGGetItemValLong (dialogID, CHECKBOX_VPOST2)) - 0.003 * DGGetItemValLong (dialogID, CHECKBOX_CROSSHEAD));

			// 8. 수평재 가로, 세로 값 비활성화 및 값 출력
			DGDisableItem (dialogID, EDITCONTROL_PLAN_WIDTH);
			DGDisableItem (dialogID, EDITCONTROL_PLAN_DEPTH);
			DGSetItemValDouble (dialogID, EDITCONTROL_PLAN_WIDTH, infoMorph.width);
			DGSetItemValDouble (dialogID, EDITCONTROL_PLAN_DEPTH, infoMorph.depth);

			break;

		case DG_MSG_CHANGE:
			// 1. 레이어 묶음 체크박스 On/Off에 따른 이벤트 처리
			if (DGGetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING) == 1) {
				switch (item) {
					case USERCONTROL_LAYER_VPOST:
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HPOST, DGGetItemValLong (dialogID, USERCONTROL_LAYER_VPOST));
						break;
					case USERCONTROL_LAYER_HPOST:
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_VPOST, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HPOST));
						break;
				}
			}

			// 2. 수직재 2단을 켜고 끌 때마다 수직재 2단의 규격, 높이 입력 컨트롤을 활성화/비활성화
			if (DGGetItemValLong (dialogID, CHECKBOX_VPOST2) == TRUE) {
				DGEnableItem (dialogID, POPUP_VPOST2_NOMINAL);
				DGEnableItem (dialogID, EDITCONTROL_VPOST2_HEIGHT);
			} else {
				DGDisableItem (dialogID, POPUP_VPOST2_NOMINAL);
				DGDisableItem (dialogID, EDITCONTROL_VPOST2_HEIGHT);
			}

			// 3. 수직재 규격이 바뀔 때마다 높이 범위 문자열이 변경되고, 수직재 높이 값의 최소/최대값 변경됨 - MP 120 (800~1200), MP 250 (1450~2500), MP 350 (1950~3500), MP 480 (2600~4800), MP 625 (4300~6250)
			if (DGPopUpGetSelected (dialogID, POPUP_VPOST1_NOMINAL) == 1) {
				DGSetItemText (dialogID, LABEL_VPOST1_HEIGHT, "H. 800~1200");
				DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 0.800);
				DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 1.200);
			} else if (DGPopUpGetSelected (dialogID, POPUP_VPOST1_NOMINAL) == 2) {
				DGSetItemText (dialogID, LABEL_VPOST1_HEIGHT, "H. 1450~2500");
				DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 1.450);
				DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 2.500);
			} else if (DGPopUpGetSelected (dialogID, POPUP_VPOST1_NOMINAL) == 3) {
				DGSetItemText (dialogID, LABEL_VPOST1_HEIGHT, "H. 1950~3500");
				DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 1.950);
				DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 3.500);
			} else if (DGPopUpGetSelected (dialogID, POPUP_VPOST1_NOMINAL) == 4) {
				DGSetItemText (dialogID, LABEL_VPOST1_HEIGHT, "H. 2600~4800");
				DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 2.600);
				DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 4.800);
			} else if (DGPopUpGetSelected (dialogID, POPUP_VPOST1_NOMINAL) == 5) {
				DGSetItemText (dialogID, LABEL_VPOST1_HEIGHT, "H. 4300~6250");
				DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 4.300);
				DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT, 6.250);
			}

			if (DGPopUpGetSelected (dialogID, POPUP_VPOST2_NOMINAL) == 1) {
				DGSetItemText (dialogID, LABEL_VPOST2_HEIGHT, "H. 800~1200");
				DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT, 0.800);
				DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT, 1.200);
			} else if (DGPopUpGetSelected (dialogID, POPUP_VPOST2_NOMINAL) == 2) {
				DGSetItemText (dialogID, LABEL_VPOST2_HEIGHT, "H. 1450~2500");
				DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT, 1.450);
				DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT, 2.500);
			} else if (DGPopUpGetSelected (dialogID, POPUP_VPOST2_NOMINAL) == 3) {
				DGSetItemText (dialogID, LABEL_VPOST2_HEIGHT, "H. 1950~3500");
				DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT, 1.950);
				DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT, 3.500);
			} else if (DGPopUpGetSelected (dialogID, POPUP_VPOST2_NOMINAL) == 4) {
				DGSetItemText (dialogID, LABEL_VPOST2_HEIGHT, "H. 2600~4800");
				DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT, 2.600);
				DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT, 4.800);
			} else if (DGPopUpGetSelected (dialogID, POPUP_VPOST2_NOMINAL) == 5) {
				DGSetItemText (dialogID, LABEL_VPOST2_HEIGHT, "H. 4300~6250");
				DGSetItemMinDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT, 4.300);
				DGSetItemMaxDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT, 6.250);
			}

			// 4. 수직재 규격이 바뀌거나, 수직재 1단/2단 체크박스 상태가 바뀌거나, 수직재 높이가 바뀌거나, 크로스헤드 체크 상태에 따라 남은 높이 변경됨
			DGSetItemValDouble (dialogID, EDITCONTROL_REMAIN_HEIGHT, infoMorph.height - (DGGetItemValDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT) * DGGetItemValLong (dialogID, CHECKBOX_VPOST1) + DGGetItemValDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT) * DGGetItemValLong (dialogID, CHECKBOX_VPOST2)) - 0.003 * DGGetItemValLong (dialogID, CHECKBOX_CROSSHEAD));

			// 5. 수평재 체크박스에 따라 수평재 UI의 너비 관련 내용 (서쪽, 남쪽) 활성화/비활성화
			if (DGGetItemValLong (dialogID, CHECKBOX_HPOST) == TRUE) {
				DGEnableItem (dialogID, LABEL_WIDTH_WEST);
				DGEnableItem (dialogID, POPUP_WIDTH_WEST);
				DGEnableItem (dialogID, LABEL_WIDTH_SOUTH);
				DGEnableItem (dialogID, POPUP_WIDTH_SOUTH);
			} else {
				DGDisableItem (dialogID, LABEL_WIDTH_WEST);
				DGDisableItem (dialogID, POPUP_WIDTH_WEST);
				DGDisableItem (dialogID, LABEL_WIDTH_SOUTH);
				DGDisableItem (dialogID, POPUP_WIDTH_SOUTH);
			}

			// 6. 수평재 서쪽/남쪽 너비가 바뀌면 동쪽/북쪽 너비도 동일하게 변경됨
			DGPopUpSelectItem (dialogID, POPUP_WIDTH_EAST, DGPopUpGetSelected (dialogID, POPUP_WIDTH_WEST));
			DGPopUpSelectItem (dialogID, POPUP_WIDTH_NORTH, DGPopUpGetSelected (dialogID, POPUP_WIDTH_SOUTH));

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:

					if (DGGetItemValLong (dialogID, CHECKBOX_VPOST1) == TRUE)
						placementInfoForPERI.bVPost1 = true;
					else
						placementInfoForPERI.bVPost1 = false;
					sprintf (placementInfoForPERI.nomVPost1, "%s", DGPopUpGetItemText (dialogID, POPUP_VPOST1_NOMINAL, DGPopUpGetSelected (dialogID, POPUP_VPOST1_NOMINAL)).ToCStr ().Get ());
					placementInfoForPERI.heightVPost1 = DGGetItemValDouble (dialogID, EDITCONTROL_VPOST1_HEIGHT);
					
					if (DGGetItemValLong (dialogID, CHECKBOX_VPOST2) == TRUE)
						placementInfoForPERI.bVPost2 = true;
					else
						placementInfoForPERI.bVPost2 = false;
					sprintf (placementInfoForPERI.nomVPost2, "%s", DGPopUpGetItemText (dialogID, POPUP_VPOST2_NOMINAL, DGPopUpGetSelected (dialogID, POPUP_VPOST2_NOMINAL)).ToCStr ().Get ());
					placementInfoForPERI.heightVPost2 = DGGetItemValDouble (dialogID, EDITCONTROL_VPOST2_HEIGHT);

					if (DGGetItemValLong (dialogID, CHECKBOX_CROSSHEAD) == TRUE)
						placementInfoForPERI.bCrosshead = true;
					else
						placementInfoForPERI.bCrosshead = false;

					if (DGGetItemValLong (dialogID, CHECKBOX_HPOST) == TRUE)
						placementInfoForPERI.bHPost = true;
					else
						placementInfoForPERI.bHPost = false;

					sprintf (tempStr, "%s", DGPopUpGetItemText (dialogID, POPUP_WIDTH_NORTH, DGPopUpGetSelected (dialogID, POPUP_WIDTH_NORTH)).ToCStr ().Get ());
					if (my_strcmp (tempStr, "없음") == 0)	strcpy (placementInfoForPERI.nomHPost_North, "");
					if (my_strcmp (tempStr, "625") == 0)	strcpy (placementInfoForPERI.nomHPost_North, "62.5 cm");
					if (my_strcmp (tempStr, "750") == 0)	strcpy (placementInfoForPERI.nomHPost_North, "75 cm");
					if (my_strcmp (tempStr, "900") == 0)	strcpy (placementInfoForPERI.nomHPost_North, "90 cm");
					if (my_strcmp (tempStr, "1200") == 0)	strcpy (placementInfoForPERI.nomHPost_North, "120 cm");
					if (my_strcmp (tempStr, "1375") == 0)	strcpy (placementInfoForPERI.nomHPost_North, "137.5 cm");
					if (my_strcmp (tempStr, "1500") == 0)	strcpy (placementInfoForPERI.nomHPost_North, "150 cm");
					if (my_strcmp (tempStr, "2015") == 0)	strcpy (placementInfoForPERI.nomHPost_North, "201.5 cm");
					if (my_strcmp (tempStr, "2250") == 0)	strcpy (placementInfoForPERI.nomHPost_North, "225 cm");
					if (my_strcmp (tempStr, "2300") == 0)	strcpy (placementInfoForPERI.nomHPost_North, "230 cm");
					if (my_strcmp (tempStr, "2370") == 0)	strcpy (placementInfoForPERI.nomHPost_North, "237 cm");
					if (my_strcmp (tempStr, "2660") == 0)	strcpy (placementInfoForPERI.nomHPost_North, "266 cm");
					if (my_strcmp (tempStr, "2960") == 0)	strcpy (placementInfoForPERI.nomHPost_North, "296 cm");

					sprintf (tempStr, "%s", DGPopUpGetItemText (dialogID, POPUP_WIDTH_WEST, DGPopUpGetSelected (dialogID, POPUP_WIDTH_WEST)).ToCStr ().Get ());
					if (my_strcmp (tempStr, "없음") == 0)	strcpy (placementInfoForPERI.nomHPost_West, "");
					if (my_strcmp (tempStr, "625") == 0)	strcpy (placementInfoForPERI.nomHPost_West, "62.5 cm");
					if (my_strcmp (tempStr, "750") == 0)	strcpy (placementInfoForPERI.nomHPost_West, "75 cm");
					if (my_strcmp (tempStr, "900") == 0)	strcpy (placementInfoForPERI.nomHPost_West, "90 cm");
					if (my_strcmp (tempStr, "1200") == 0)	strcpy (placementInfoForPERI.nomHPost_West, "120 cm");
					if (my_strcmp (tempStr, "1375") == 0)	strcpy (placementInfoForPERI.nomHPost_West, "137.5 cm");
					if (my_strcmp (tempStr, "1500") == 0)	strcpy (placementInfoForPERI.nomHPost_West, "150 cm");
					if (my_strcmp (tempStr, "2015") == 0)	strcpy (placementInfoForPERI.nomHPost_West, "201.5 cm");
					if (my_strcmp (tempStr, "2250") == 0)	strcpy (placementInfoForPERI.nomHPost_West, "225 cm");
					if (my_strcmp (tempStr, "2300") == 0)	strcpy (placementInfoForPERI.nomHPost_West, "230 cm");
					if (my_strcmp (tempStr, "2370") == 0)	strcpy (placementInfoForPERI.nomHPost_West, "237 cm");
					if (my_strcmp (tempStr, "2660") == 0)	strcpy (placementInfoForPERI.nomHPost_West, "266 cm");
					if (my_strcmp (tempStr, "2960") == 0)	strcpy (placementInfoForPERI.nomHPost_West, "296 cm");

					sprintf (tempStr, "%s", DGPopUpGetItemText (dialogID, POPUP_WIDTH_EAST, DGPopUpGetSelected (dialogID, POPUP_WIDTH_EAST)).ToCStr ().Get ());
					if (my_strcmp (tempStr, "없음") == 0)	strcpy (placementInfoForPERI.nomHPost_East, "");
					if (my_strcmp (tempStr, "625") == 0)	strcpy (placementInfoForPERI.nomHPost_East, "62.5 cm");
					if (my_strcmp (tempStr, "750") == 0)	strcpy (placementInfoForPERI.nomHPost_East, "75 cm");
					if (my_strcmp (tempStr, "900") == 0)	strcpy (placementInfoForPERI.nomHPost_East, "90 cm");
					if (my_strcmp (tempStr, "1200") == 0)	strcpy (placementInfoForPERI.nomHPost_East, "120 cm");
					if (my_strcmp (tempStr, "1375") == 0)	strcpy (placementInfoForPERI.nomHPost_East, "137.5 cm");
					if (my_strcmp (tempStr, "1500") == 0)	strcpy (placementInfoForPERI.nomHPost_East, "150 cm");
					if (my_strcmp (tempStr, "2015") == 0)	strcpy (placementInfoForPERI.nomHPost_East, "201.5 cm");
					if (my_strcmp (tempStr, "2250") == 0)	strcpy (placementInfoForPERI.nomHPost_East, "225 cm");
					if (my_strcmp (tempStr, "2300") == 0)	strcpy (placementInfoForPERI.nomHPost_East, "230 cm");
					if (my_strcmp (tempStr, "2370") == 0)	strcpy (placementInfoForPERI.nomHPost_East, "237 cm");
					if (my_strcmp (tempStr, "2660") == 0)	strcpy (placementInfoForPERI.nomHPost_East, "266 cm");
					if (my_strcmp (tempStr, "2960") == 0)	strcpy (placementInfoForPERI.nomHPost_East, "296 cm");

					sprintf (tempStr, "%s", DGPopUpGetItemText (dialogID, POPUP_WIDTH_SOUTH, DGPopUpGetSelected (dialogID, POPUP_WIDTH_SOUTH)).ToCStr ().Get ());
					if (my_strcmp (tempStr, "없음") == 0)	strcpy (placementInfoForPERI.nomHPost_South, "");
					if (my_strcmp (tempStr, "625") == 0)	strcpy (placementInfoForPERI.nomHPost_South, "62.5 cm");
					if (my_strcmp (tempStr, "750") == 0)	strcpy (placementInfoForPERI.nomHPost_South, "75 cm");
					if (my_strcmp (tempStr, "900") == 0)	strcpy (placementInfoForPERI.nomHPost_South, "90 cm");
					if (my_strcmp (tempStr, "1200") == 0)	strcpy (placementInfoForPERI.nomHPost_South, "120 cm");
					if (my_strcmp (tempStr, "1375") == 0)	strcpy (placementInfoForPERI.nomHPost_South, "137.5 cm");
					if (my_strcmp (tempStr, "1500") == 0)	strcpy (placementInfoForPERI.nomHPost_South, "150 cm");
					if (my_strcmp (tempStr, "2015") == 0)	strcpy (placementInfoForPERI.nomHPost_South, "201.5 cm");
					if (my_strcmp (tempStr, "2250") == 0)	strcpy (placementInfoForPERI.nomHPost_South, "225 cm");
					if (my_strcmp (tempStr, "2300") == 0)	strcpy (placementInfoForPERI.nomHPost_South, "230 cm");
					if (my_strcmp (tempStr, "2370") == 0)	strcpy (placementInfoForPERI.nomHPost_South, "237 cm");
					if (my_strcmp (tempStr, "2660") == 0)	strcpy (placementInfoForPERI.nomHPost_South, "266 cm");
					if (my_strcmp (tempStr, "2960") == 0)	strcpy (placementInfoForPERI.nomHPost_South, "296 cm");

					break;
				case DG_CANCEL:
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