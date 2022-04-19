#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "Facility.hpp"

// 3D 품질/속도 조정하기
GSErrCode	select3DQuality (void)
{
	GSErrCode	err = NoError;

	bool	suspGrp;
	short	result;
	double	gs_resol;
	long	nElems;

	bool	bSuccess;
	long	ElemsChanged = 0;
	long	ElemsUnchanged = 0;

	GS::Array<API_Guid> elemList;
	API_Element			elem;
	API_ElementMemo		memo;

	// 그룹화 일시정지
	ACAPI_Environment (APIEnv_IsSuspendGroupOnID, &suspGrp);
	if (suspGrp == false)	ACAPI_Element_Tool (NULL, NULL, APITool_SuspendGroups, NULL);

	result = DGAlert (DG_INFORMATION, "3D 품질/속도 조정하기", "3D 품질을 선택하십시오", "", "느림-고품질(32)", "중간(12)", "빠름-저품질(4)");

	if (result == 1)		gs_resol = 32.0;
	else if (result == 2)	gs_resol = 12.0;
	else					gs_resol = 4.0;

	result = DGAlert (DG_WARNING, "3D 품질/속도 조정하기", "계속 진행하시겠습니까?", "", "예", "아니오", "");

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
				ACAPI_DisposeElemMemoHdls (&memo);

				if (bSuccess == true)
					ElemsChanged ++;
				else
					ElemsUnchanged ++;
			}
		}
	}

	elemList.Clear ();

	WriteReport_Alert ("변경된 객체 개수: %d\n변경되지 않은 객체 개수: %d", ElemsChanged, ElemsUnchanged);

	return	err;
}

// 영역에 3D 라벨 붙이기
GSErrCode	attach3DLabelOnZone (void)
{
	GSErrCode	err = NoError;

	bool	suspGrp;
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

	// 그룹화 일시정지
	ACAPI_Environment (APIEnv_IsSuspendGroupOnID, &suspGrp);
	if (suspGrp == false)	ACAPI_Element_Tool (NULL, NULL, APITool_SuspendGroups, NULL);

	// 기존 라벨 객체를 모두 제거
	result = DGAlert (DG_WARNING, "기존 라벨 객체 모두 제거", "기존 라벨 객체를 모두 제거하시겠습니까?", "", "예", "아니오", "");

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
					API_Elem_Head* headList = new API_Elem_Head [1];
					headList [0] = elem.header;
					ACAPI_Element_Delete (&headList, 1);
					delete headList;

					ElemsDeleted ++;
				}
			}

			ACAPI_DisposeElemMemoHdls (&memo);
		}
	}

	elemList.Clear ();
	
	// 새로운 라벨 객체를 배치함 (레이어 이름 선택)
	result = DGModalDialog (ACAPI_GetOwnResModule (), 32510, ACAPI_GetOwnResModule (), selectLayerHandler, (DGUserData) &layerInd);

	if (result == DG_CANCEL)
		goto EXIT;

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
			label.init (L("라벨v1.0.gsm"), layerInd, elem.header.floorInd, elem.zone.pos.x, elem.zone.pos.y, elem.zone.roomBaseLev + elem.zone.roomHeight/2, DegreeToRad (0.0));

			sprintf (roomName, "%s", (const char *) GS::UniString (elem.zone.roomName).ToCStr ());

			label.placeObject (9,
				"scaleValues", APIParT_CString, "스케일에 맞게 (모델 크기)",
				"iScaleValues", APIParT_Integer, "1",
				"bShowOn2D", APIParT_Boolean, "0",
				"bShowOn3D", APIParT_Boolean, "1",
				"bLocalOrigin", APIParT_Boolean, "0",
				"szFont", APIParT_Length, format_string ("%f", 1.000),
				"bCoords", APIParT_Boolean, "0",
				"bComment", APIParT_Boolean, "1",
				"txtComment", APIParT_CString, roomName);

			ElemsAdded ++;
		}
	}

	elemList.Clear ();

EXIT:
	WriteReport_Alert ("제거된 라벨 개수: %ld\n추가된 라벨 개수: %ld", ElemsDeleted, ElemsAdded);

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
			DGSetDialogTitle (dialogID, "3D 라벨의 레이어 선택하기");

			// 확인 버튼
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, "확인");

			// 취소 버튼
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, "취소");

			// 라벨
			DGSetItemFont (dialogID, 3, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, 3, "모든 영역(Zone)에 라벨 객체를 배치하시겠습니까?");

			DGSetItemFont (dialogID, 5, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, 5, "부재별 레이어 설정");

			DGSetItemFont (dialogID, 6, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, 6, "라벨");

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