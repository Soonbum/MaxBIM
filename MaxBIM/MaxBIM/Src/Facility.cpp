#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "Facility.hpp"
#include "Export.hpp"

using namespace FacilityDG;

// 3D 품질/속도 조정하기
GSErrCode	select3DQuality (void)
{
	GSErrCode	err = NoError;

	short	result;
	double	gs_resol;
	long	nElems;

	bool	bSuccess;
	long	ElemsChanged = 0;
	long	ElemsUnchanged = 0;

	GS::Array<API_Guid> elemList;
	API_Element			elem;
	API_ElementMemo		memo;

	// 그룹화 일시정지 ON
	suspendGroups (true);

	result = DGAlert (DG_INFORMATION, L"3D 품질/속도 조정하기", L"3D 품질을 선택하십시오", "", L"느림-고품질(32)", L"중간(12)", L"빠름-저품질(4)");

	if (result == 1)		gs_resol = 32.0;
	else if (result == 2)	gs_resol = 12.0;
	else					gs_resol = 4.0;

	result = DGAlert (DG_WARNING, L"3D 품질/속도 조정하기", L"계속 진행하시겠습니까?", "", L"예", L"아니오", "");

	if (result == DG_CANCEL)
		return err;

	// 모든 객체를 불러옴
	ACAPI_Element_GetElemList (API_ObjectID, &elemList, APIFilt_OnVisLayer);
	nElems = elemList.GetSize ();

	for (short xx = 0 ; xx < nElems ; ++xx) {
		BNZeroMemory (&elem, sizeof (API_Element));
		BNZeroMemory (&memo, sizeof (API_ElementMemo));
		elem.header.guid = elemList [xx];
		err = ACAPI_Element_Get (&elem);

		if (err == NoError && elem.header.hasMemo) {
			err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

			if (err == NoError) {
				bSuccess = setParameterByName (&memo, "gs_resol", gs_resol);	// 해상도 값을 변경함

				ACAPI_Element_Change (&elem, NULL, &memo, APIMemoMask_AddPars, true);

				if (bSuccess == true)
					ElemsChanged ++;
				else
					ElemsUnchanged ++;
			}
			ACAPI_DisposeElemMemoHdls (&memo);
		}
	}

	elemList.Clear ();

	WriteReport_Alert ("변경된 객체 개수: %d\n변경되지 않은 객체 개수: %d", ElemsChanged, ElemsUnchanged);

	// 그룹화 일시정지 OFF
	suspendGroups (false);

	return	err;
}

// 영역에 3D 라벨 붙이기
GSErrCode	attach3DLabelOnZone (void)
{
	GSErrCode	err = NoError;

	short	result;
	long	nElems;
	short	layerInd;
	char	roomName [256];
	const char*	foundStr;

	long	ElemsAdded = 0;
	long	ElemsDeleted = 0;

	GS::Array<API_Guid>	elemList;
	API_Element			elem;
	API_ElementMemo		memo;

	// 그룹화 일시정지 ON
	suspendGroups (true);

	ACAPI_Element_GetElemList (API_ObjectID, &elemList, APIFilt_InMyWorkspace);
	nElems = elemList.GetSize ();

	for (long xx = 0 ; xx < nElems ; ++xx) {
		BNZeroMemory (&elem, sizeof (API_Element));
		BNZeroMemory (&memo, sizeof (API_ElementMemo));
		elem.header.guid = elemList [xx];
		err = ACAPI_Element_Get (&elem);

		if (err == NoError && elem.header.hasMemo) {
			err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

			if (err == NoError) {
				foundStr = getParameterStringByName (&memo, "d_comp");
				if (my_strcmp (foundStr, "좌표") == 0) {
					GS::Array<API_Element>	elems;
					elems.Push (elem);
					deleteElements (elems);

					ElemsDeleted ++;
				}
			}

			ACAPI_DisposeElemMemoHdls (&memo);
		}
	}

	elemList.Clear ();
	
	// 새로운 라벨 객체를 배치함 (레이어 이름 선택)
	result = DGModalDialog (ACAPI_GetOwnResModule (), 32510, ACAPI_GetOwnResModule (), selectLayerHandler, (DGUserData) &layerInd);

	if (result == DG_CANCEL) {
		WriteReport_Alert ("제거된 라벨 개수: %ld\n추가된 라벨 개수: %ld", ElemsDeleted, ElemsAdded);
		return err;
	}

	// 모든 영역(Zone) 객체를 불러옴
	ACAPI_Element_GetElemList (API_ZoneID , &elemList, APIFilt_InMyWorkspace);
	nElems = elemList.GetSize ();

	EasyObjectPlacement	label;

	for (long xx = 0 ; xx < nElems ; ++xx) {
		BNZeroMemory (&elem, sizeof (API_Element));
		elem.header.guid = elemList [xx];
		err = ACAPI_Element_Get (&elem);

		if (err == NoError) {
			// 영역(Zone) 객체의 정보를 기반으로 라벨 배치
			label.init (L("라벨v1.0.gsm"), layerInd, elem.header.floorInd, elem.zone.pos.x, elem.zone.pos.y, elem.zone.roomBaseLev + 1.000, DegreeToRad (0.0));

			sprintf (roomName, "%s", (const char *) GS::UniString (elem.zone.roomName).ToCStr ());

			label.placeObject (13,
				"scaleValues", APIParT_CString, "스케일에 맞게 (모델 크기)",
				"iScaleValues", APIParT_Integer, "1",
				"bShowOn2D", APIParT_Boolean, "0",
				"bShowOn3D", APIParT_Boolean, "1",
				"bLocalOrigin", APIParT_Boolean, "0",
				"szFont", APIParT_Length, format_string ("%f", 2.000),
				"bCoords", APIParT_Boolean, "0",
				"bComment", APIParT_Boolean, "1",
				"txtComment", APIParT_CString, roomName,
				"gs_cont_pen", APIParT_PenCol, "20",
				"textMat", APIParT_Mater, "19",
				"bBg", APIParT_Boolean, "0",
				"thkTxt", APIParT_Length, "0.050");

			ElemsAdded ++;
		}
	}

	elemList.Clear ();

	WriteReport_Alert ("제거된 라벨 개수: %ld\n추가된 라벨 개수: %ld", ElemsDeleted, ElemsAdded);

	// 그룹화 일시정지 OFF
	suspendGroups (false);

	return	err;
}

// [다이얼로그 박스] 레이어 선택
short DGCALLBACK selectLayerHandler (short message, short dialogID, short item, DGUserData userData, DGMessageData /* msgData */)
{
	short	result;
	short	*layerInd = (short*) userData;
	API_UCCallbackType	ucb;

	switch (message) {
		case DG_MSG_INIT:
			// 다이얼로그 타이틀
			DGSetDialogTitle (dialogID, L"3D 라벨의 레이어 선택하기");

			// 확인 버튼
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, L"확인");

			// 취소 버튼
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, L"취소");

			// 라벨
			DGSetItemFont (dialogID, 3, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, 3, L"모든 영역(Zone)에 라벨 객체를 배치하시겠습니까?");

			DGSetItemFont (dialogID, 5, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, 5, L"부재별 레이어 설정");

			DGSetItemFont (dialogID, 6, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, 6, L"라벨");

			// 유저 컨트롤 초기화
			BNZeroMemory (&ucb, sizeof (ucb));
			ucb.dialogID = dialogID;
			ucb.type	 = APIUserControlType_Layer;
			ucb.itemID	 = 7;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, 7, 1);

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					*layerInd = (short)DGGetItemValLong (dialogID, 7);
					break;

				case DG_CANCEL:
					break;
			}
			break;

		case DG_MSG_CLOSE:
			switch (item) {
				case DG_CLOSEBOX:
					break;
			}
	}

	result = item;

	return	result;
}

// 현재 평면도의 테이블폼에 버블 자동 배치
GSErrCode	attachBubbleOnCurrentFloorPlan (void)
{
	GSErrCode	err = NoError;
	short	result;
	short	xx, yy, mm;
	API_StoryInfo	storyInfo;
	CircularBubble	cbInfo;		// 원형 버블에 대한 정보
	double	xMin, xMax;
	double	yMin, yMax;
	double	zMin, zMax;

	GS::Array<API_Guid>		elemList;
	GS::Array<API_Guid>		objects;
	long					nElems = 0;
	long					nObjects = 0;

	// 선택한 요소들의 정보 요약하기
	API_Element			elem;
	API_ElemInfo3D		info3D;

	// 레이어 관련 변수
	short			nLayers;
	API_Attribute	attrib;
	short			nVisibleLayers = 0;
	short			visLayerList [1024];
	char			fullLayerName [512];
	vector<LayerList>	layerList;

	// 선택한 객체들의 레이어 인덱스들을 저장함
	bool	bIndexFound;
	vector<short>	selectedLayerIndex;

	// 레이어 이름에서 접미사를 가져오기 위한 변수
	char	bubbleText [64];
	char	strEnd [32];
	char	strEndBefore [32];
	char	*token;			// 읽어온 문자열의 토큰

	// 진행바를 표현하기 위한 변수
	GS::UniString       title ("버블 배치 진행 상황");
	GS::UniString       subtitle ("진행중...");
	short	nPhase;
	Int32	cur, total;


	// [1차 다이얼로그] 원형 버블 설정
	result = DGModalDialog (ACAPI_GetOwnResModule (), 32511, ACAPI_GetOwnResModule (), setBubbleHandler, (DGUserData) &cbInfo);

	// 선택한 객체들이 있다면 객체들의 레이어 인덱스들을 수집함
	getGuidsOfSelection (&elemList, API_ObjectID, &nElems);
	for (xx = 0 ; xx < nElems ; ++xx) {
		BNZeroMemory (&elem, sizeof (API_Element));
		elem.header.guid = elemList.Pop ();
		err = ACAPI_Element_Get (&elem);

		bIndexFound = false;
		for (yy = 0 ; yy < selectedLayerIndex.size () ; ++yy) {
			if (selectedLayerIndex [yy] == elem.header.layer)
				bIndexFound = true;
		}
		if (bIndexFound == false)
			selectedLayerIndex.push_back (elem.header.layer);
	}
	elemList.Clear ();
	nElems = 0;

	// 현재 활성 층의 인덱스를 가져옴
	BNZeroMemory (&storyInfo, sizeof (API_StoryInfo));
	ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo);
	cbInfo.floorInd = storyInfo.actStory;
	BMKillHandle ((GSHandle *) &storyInfo.data);

	// 프로젝트 내 레이어 개수를 알아냄
	nLayers = getLayerCount ();

	// 보이는 레이어들의 목록 저장하기
	for (xx = 1 ; xx <= nLayers ; ++xx) {
		BNZeroMemory (&attrib, sizeof (API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = xx;
		err = ACAPI_Attribute_Get (&attrib);
		if (err == NoError) {
			if (!((attrib.layer.head.flags & APILay_Hidden) == true)) {
				visLayerList [nVisibleLayers++] = attrib.layer.head.index;
			}
		}
	}

	// 레이어 이름과 인덱스 저장
	for (xx = 0 ; xx < nVisibleLayers ; ++xx) {
		BNZeroMemory (&attrib, sizeof (API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = visLayerList [xx];
		err = ACAPI_Attribute_Get (&attrib);

		sprintf (fullLayerName, "%s", attrib.layer.head.name);
		fullLayerName [strlen (fullLayerName)] = '\0';

		LayerList newLayerItem;
		newLayerItem.layerInd = visLayerList [xx];
		newLayerItem.layerName = fullLayerName;

		layerList.push_back (newLayerItem);
	}

	// 레이어 이름 기준으로 정렬하여 레이어 인덱스 순서 변경
	sort (layerList.begin (), layerList.end (), compareLayerName);		// 레이어 이름 기준 오름차순 정렬

	// 일시적으로 모든 레이어 숨기기
	for (xx = 1 ; xx <= nLayers ; ++xx) {
		BNZeroMemory (&attrib, sizeof (API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = xx;
		err = ACAPI_Attribute_Get (&attrib);
		if (err == NoError) {
			attrib.layer.head.flags |= APILay_Hidden;
			ACAPI_Attribute_Modify (&attrib, NULL);
		}
	}

	// 진행 상황 표시하는 기능 - 초기화
	nPhase = 1;
	cur = 1;
	total = nVisibleLayers;
	ACAPI_Interface (APIIo_InitProcessWindowID, &title, &nPhase);
	ACAPI_Interface (APIIo_SetNextProcessPhaseID, &subtitle, &total);

	// 보이는 레이어들을 하나씩 순회하면서 전체 요소들을 선택한 후 테이블폼마다 원형 버블을 배치함
	for (mm = 1 ; mm <= nVisibleLayers ; ++mm) {
		BNZeroMemory (&attrib, sizeof (API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		//attrib.layer.head.index = visLayerList [mm-1];
		attrib.layer.head.index = layerList [mm-1].layerInd;
		err = ACAPI_Attribute_Get (&attrib);

		if (err == NoError) {
			// 레이어 보이기
			if ((attrib.layer.head.flags & APILay_Hidden) == true) {
				attrib.layer.head.flags ^= APILay_Hidden;
				ACAPI_Attribute_Modify (&attrib, NULL);
			}

			// 레이어 이름 가져옴
			sprintf (fullLayerName, "%s", attrib.layer.head.name);
			fullLayerName [strlen (fullLayerName)] = '\0';

			// 저장된 객체 정보 초기화
			nElems = 0;
			nObjects = 0;
			elemList.Clear ();
			objects.Clear ();

			// 현재 레이어의 모든 객체들을 가져옴
			ACAPI_Element_GetElemList (API_ObjectID, &elemList, APIFilt_OnVisLayer);	// 보이는 레이어에 있음, 객체 타입만
			nElems = elemList.GetSize ();

			// 현재 층, 선택한 객체에 속한 객체들만 걸러냄
			for (xx = 0 ; xx < nElems ; ++xx) {
				BNZeroMemory (&elem, sizeof (API_Element));
				elem.header.guid = elemList.Pop ();
				err = ACAPI_Element_Get (&elem);

				if ((elem.header.floorInd == cbInfo.floorInd) && (elem.header.layer == attrib.layer.head.index)) {
					bIndexFound = false;
					for (yy = 0 ; yy < selectedLayerIndex.size () ; ++yy) {
						if (elem.header.layer == selectedLayerIndex [yy])
							bIndexFound = true;
					}
					if (bIndexFound == true)
						objects.Push (elem.header.guid);
				}
			}
			nObjects = objects.GetSize ();

			// 현재 층에 속한 객체가 없을 경우 스킵
			if (nObjects == 0)
				continue;

			// 현재 층, 현재 레이어에 있는 모든 객체들의 3D 박스 정보를 가져옴
			for (xx = 0 ; xx < nObjects ; ++xx) {
				BNZeroMemory (&elem, sizeof (API_Element));
				elem.header.guid = objects.Pop ();
				err = ACAPI_Element_Get3DInfo (elem.header, &info3D);

				if (xx == 0) {
					xMin = info3D.bounds.xMin;	xMax = info3D.bounds.xMax;
					yMin = info3D.bounds.yMin;	yMax = info3D.bounds.yMax;
					zMin = info3D.bounds.zMin;	zMax = info3D.bounds.zMax;
				} else {
					if (xMin > info3D.bounds.xMin)	xMin = info3D.bounds.xMin;
					if (xMax < info3D.bounds.xMax)	xMax = info3D.bounds.xMax;

					if (yMin > info3D.bounds.yMin)	yMin = info3D.bounds.yMin;
					if (yMax < info3D.bounds.yMax)	yMax = info3D.bounds.yMax;

					if (zMin > info3D.bounds.zMin)	zMin = info3D.bounds.zMin;
					if (zMax < info3D.bounds.zMax)	zMax = info3D.bounds.zMax;
				}
			}

			// 레이어 이름으로부터 버블의 텍스트를 추출
			strcpy (strEnd, "\0");
			strcpy (strEndBefore, "\0");
			token = strtok (fullLayerName, "-");
			while (token != NULL) {
				strcpy (strEndBefore, strEnd);	// 마지막 접미사를 마지막 직전 접미사로 복사
				strcpy (strEnd, token);			// 마지막 접미사를 저장
				token = strtok (NULL, "-");
			}

			// 원형 버블에 들어갈 텍스트를 선택함
			if ((isStringDouble (strEndBefore) == TRUE) && (isStringDouble (strEnd) == TRUE)) {
				// 마지막 직전 접미사가 숫자, 마지막 접미사가 숫자이면?
				sprintf (bubbleText, "%d-%d", atoi (strEndBefore), atoi (strEnd));
			} else if ((isStringDouble (strEndBefore) == FALSE) && (isStringDouble (strEnd) == TRUE)) {
				// 마지막 직전 접미사가 문자, 마지막 접미사가 숫자이면?
				sprintf (bubbleText, "%d", atoi (strEnd));
			} else {
				// 마지막 접미사를 선택
				strcpy (bubbleText, strEnd);
			}
			
			// 원형 버블 배치
			EasyObjectPlacement	bubble;
			if (cbInfo.pos == UP) {
				bubble.init (L("원형버블v1.0.gsm"), cbInfo.layerInd, cbInfo.floorInd, xMin + (xMax - xMin)/2, yMax + cbInfo.lenWithdrawal + cbInfo.szBubbleDia/2, 0.0, DegreeToRad (0.0));
				bubble.placeObject (14,
					"bShowOn2D", APIParT_Boolean, "1.0",
					"bShowOn3D", APIParT_Boolean, "1.0",
					"szBubbleDia", APIParT_Length, format_string ("%f", cbInfo.szBubbleDia),
					"szFont", APIParT_Length, format_string ("%f", cbInfo.szFont),
					"bWithdrawal", APIParT_Boolean, "1.0",
					"lenWithdrawal", APIParT_Length, format_string ("%f", cbInfo.lenWithdrawal),
					"angWithdrawal", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)),
					"gs_cont_pen", APIParT_PenCol, "19",
					"gs_fill_pen", APIParT_PenCol, "19",
					"gs_back_pen", APIParT_PenCol, "19",
					"textMat", APIParT_Mater, "49",
					"bgMat", APIParT_Mater, "60",
					"txtComment", APIParT_CString, bubbleText,
					"thkTxt", APIParT_Length, "0.005");
			} else if (cbInfo.pos == DOWN) {
				bubble.init (L("원형버블v1.0.gsm"), cbInfo.layerInd, cbInfo.floorInd, xMin + (xMax - xMin)/2, yMin - cbInfo.lenWithdrawal - cbInfo.szBubbleDia/2, 0.0, DegreeToRad (0.0));
				bubble.placeObject (14,
					"bShowOn2D", APIParT_Boolean, "1.0",
					"bShowOn3D", APIParT_Boolean, "1.0",
					"szBubbleDia", APIParT_Length, format_string ("%f", cbInfo.szBubbleDia),
					"szFont", APIParT_Length, format_string ("%f", cbInfo.szFont),
					"bWithdrawal", APIParT_Boolean, "1.0",
					"lenWithdrawal", APIParT_Length, format_string ("%f", cbInfo.lenWithdrawal),
					"angWithdrawal", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)),
					"gs_cont_pen", APIParT_PenCol, "19",
					"gs_fill_pen", APIParT_PenCol, "19",
					"gs_back_pen", APIParT_PenCol, "19",
					"textMat", APIParT_Mater, "49",
					"bgMat", APIParT_Mater, "60",
					"txtComment", APIParT_CString, bubbleText,
					"thkTxt", APIParT_Length, "0.005");
			} else if (cbInfo.pos == LEFT) {
				bubble.init (L("원형버블v1.0.gsm"), cbInfo.layerInd, cbInfo.floorInd, xMin - cbInfo.lenWithdrawal - cbInfo.szBubbleDia/2, yMin + (yMax - yMin)/2, 0.0, DegreeToRad (0.0));
				bubble.placeObject (14,
					"bShowOn2D", APIParT_Boolean, "1.0",
					"bShowOn3D", APIParT_Boolean, "1.0",
					"szBubbleDia", APIParT_Length, format_string ("%f", cbInfo.szBubbleDia),
					"szFont", APIParT_Length, format_string ("%f", cbInfo.szFont),
					"bWithdrawal", APIParT_Boolean, "1.0",
					"lenWithdrawal", APIParT_Length, format_string ("%f", cbInfo.lenWithdrawal),
					"angWithdrawal", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)),
					"gs_cont_pen", APIParT_PenCol, "19",
					"gs_fill_pen", APIParT_PenCol, "19",
					"gs_back_pen", APIParT_PenCol, "19",
					"textMat", APIParT_Mater, "49",
					"bgMat", APIParT_Mater, "60",
					"txtComment", APIParT_CString, bubbleText,
					"thkTxt", APIParT_Length, "0.005");
			} else if (cbInfo.pos == RIGHT) {
				bubble.init (L("원형버블v1.0.gsm"), cbInfo.layerInd, cbInfo.floorInd, xMax + cbInfo.lenWithdrawal + cbInfo.szBubbleDia/2, yMin + (yMax - yMin)/2, 0.0, DegreeToRad (0.0));
				bubble.placeObject (14,
					"bShowOn2D", APIParT_Boolean, "1.0",
					"bShowOn3D", APIParT_Boolean, "1.0",
					"szBubbleDia", APIParT_Length, format_string ("%f", cbInfo.szBubbleDia),
					"szFont", APIParT_Length, format_string ("%f", cbInfo.szFont),
					"bWithdrawal", APIParT_Boolean, "1.0",
					"lenWithdrawal", APIParT_Length, format_string ("%f", cbInfo.lenWithdrawal),
					"angWithdrawal", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)),
					"gs_cont_pen", APIParT_PenCol, "19",
					"gs_fill_pen", APIParT_PenCol, "19",
					"gs_back_pen", APIParT_PenCol, "19",
					"textMat", APIParT_Mater, "49",
					"bgMat", APIParT_Mater, "60",
					"txtComment", APIParT_CString, bubbleText,
					"thkTxt", APIParT_Length, "0.005");
			}

			// 레이어 숨기기
			attrib.layer.head.flags |= APILay_Hidden;
			ACAPI_Attribute_Modify (&attrib, NULL);
		}

		// 진행 상황 표시하는 기능 - 진행
		cur = mm;
		ACAPI_Interface (APIIo_SetProcessValueID, &cur, NULL);
		if (ACAPI_Interface (APIIo_IsProcessCanceledID, NULL, NULL) == APIERR_CANCEL)
			break;
	}

	// 진행 상황 표시하는 기능 - 마무리
	ACAPI_Interface (APIIo_CloseProcessWindowID, NULL, NULL);

	// 모든 프로세스를 마치면 처음에 수집했던 보이는 레이어들을 다시 켜놓을 것
	for (xx = 1 ; xx <= nVisibleLayers ; ++xx) {
		BNZeroMemory (&attrib, sizeof (API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = visLayerList [xx-1];
		err = ACAPI_Attribute_Get (&attrib);
		if (err == NoError) {
			if ((attrib.layer.head.flags & APILay_Hidden) == true) {
				attrib.layer.head.flags ^= APILay_Hidden;
				ACAPI_Attribute_Modify (&attrib, NULL);
			}
		}
	}

	return	err;
}

// [다이얼로그 박스] 원형 버블 설정
short DGCALLBACK setBubbleHandler (short message, short dialogID, short item, DGUserData userData, DGMessageData /* msgData */)
{
	short	result;
	CircularBubble	*cbInfo = (CircularBubble*) userData;
	API_UCCallbackType	ucb;

	switch (message) {
		case DG_MSG_INIT:
			// 다이얼로그 타이틀
			DGSetDialogTitle (dialogID, L"원형 버블 설정하기");

			// 확인 버튼
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, L"확인");

			// 취소 버튼
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, L"취소");

			// 버블 직경
			DGSetItemFont (dialogID, LABEL_DIAMETER, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_DIAMETER, L"버블 직경");
			DGSetItemValDouble (dialogID, EDITCONTROL_DIAMETER, 0.550);

			// 글자 크기
			DGSetItemFont (dialogID, LABEL_LETTER_SIZE, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_LETTER_SIZE, L"글자 크기");
			DGSetItemValDouble (dialogID, EDITCONTROL_LETTER_SIZE, 0.200);

			// 인출선 길이
			DGSetItemFont (dialogID, LABEL_WITHDRAWAL_LENGTH, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_WITHDRAWAL_LENGTH, L"인출선 길이");
			DGSetItemValDouble (dialogID, EDITCONTROL_WITHDRAWAL_LENGTH, 0.700);

			// 버블 위치
			DGSetItemFont (dialogID, LABEL_BUBBLE_POS, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_BUBBLE_POS, L"버블 위치");
			// 팝업 컨트롤의 항목을 한글로 변환
			DGPopUpSetItemText (dialogID, POPUPCONTROL_BUBBLE_POS, 1, L"상");
			DGPopUpSetItemText (dialogID, POPUPCONTROL_BUBBLE_POS, 2, L"하");
			DGPopUpSetItemText (dialogID, POPUPCONTROL_BUBBLE_POS, 3, L"좌");
			DGPopUpSetItemText (dialogID, POPUPCONTROL_BUBBLE_POS, 4, L"우");

			// 레이어 및 유저 컨트롤 초기화
			DGSetItemFont (dialogID, LABEL_LAYER_SETTINGS, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_LAYER_SETTINGS, L"레이어 설정");
			DGSetItemFont (dialogID, LABEL_LAYER_CIRCULAR_BUBBLE, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_LAYER_CIRCULAR_BUBBLE, L"원형 버블");
			BNZeroMemory (&ucb, sizeof (ucb));
			ucb.dialogID = dialogID;
			ucb.type	 = APIUserControlType_Layer;
			ucb.itemID	 = USERCONTROL_LAYER_CIRCULAR_BUBBLE;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_CIRCULAR_BUBBLE, 1);

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					cbInfo->layerInd = (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_CIRCULAR_BUBBLE);
					cbInfo->lenWithdrawal = DGGetItemValDouble (dialogID, EDITCONTROL_WITHDRAWAL_LENGTH);
					cbInfo->pos = (short)DGPopUpGetSelected (dialogID, POPUPCONTROL_BUBBLE_POS);
					cbInfo->szBubbleDia = DGGetItemValDouble (dialogID, EDITCONTROL_DIAMETER);
					cbInfo->szFont = DGGetItemValDouble (dialogID, EDITCONTROL_LETTER_SIZE);
					break;

				case DG_CANCEL:
					break;
			}
			break;

		case DG_MSG_CLOSE:
			switch (item) {
				case DG_CLOSEBOX:
					break;
			}
	}

	result = item;

	return	result;
}
