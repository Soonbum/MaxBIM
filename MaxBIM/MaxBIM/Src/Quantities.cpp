#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "Quantities.hpp"

using namespace quantitiesDG;

long	nElems;		// 요소 개수
qElem*	elems;		// 요소 배열

short				nLayers;		// 레이어 개수
short				nObjects;		// 물량합판 객체 타입 개수
GS::Array<short>	layerInds;		// 보이는 레이어 인덱스
GS::Array<string>	layerNames;		// 보이는 레이어 이름
GS::Array<string>	objType;		// 객체 타입

short	itmPosX, itmPosY;

short	nPopupControl;				// 팝업 컨트롤 수
short	existingLayerPopup [50];	// 팝업 컨트롤: 기존 레이어
short	qLayerPopup [50];			// 팝업 컨트롤: 물량합판 레이어
short	objTypePopup [50];			// 팝업 컨트롤: 객체 타입

// 부재(벽,슬래브,보,기둥)들의 가설재가 붙을 수 있는 면에 물량합판을 자동으로 부착함
GSErrCode	placeQuantityPlywood (void)
{
	GSErrCode	err = NoError;

	API_Element				elem;
	API_ElementMemo			memo;
	API_ElemInfo3D			info3D;

	short	xx, yy;
	short	result;		// 다이얼로그 리턴 값
	
	API_Attribute	attrib;

	GS::Array<API_Guid>	elemList_All;		// 요소들의 GUID (전체 구조 요소)
	GS::Array<API_Guid>	elemList_Wall;		// 요소들의 GUID (벽)
	GS::Array<API_Guid>	elemList_Slab;		// 요소들의 GUID (슬래브)
	GS::Array<API_Guid>	elemList_Beam;		// 요소들의 GUID (보)
	GS::Array<API_Guid>	elemList_Column;	// 요소들의 GUID (기둥)


	// 프로젝트 내 레이어 개수를 알아냄
	BNZeroMemory (&attrib, sizeof (API_Attribute));
	attrib.header.typeID = API_LayerID;
	err = ACAPI_Attribute_GetNum (API_LayerID, &nLayers);

	// 보이는 레이어의 인덱스, 이름 저장
	for (xx = 1; xx <= nLayers ; ++xx) {
		attrib.header.index = xx;
		err = ACAPI_Attribute_Get (&attrib);

		if (err == NoError) {
			if ((attrib.layer.head.flags & APILay_Hidden) == false) {
				layerInds.Push (attrib.layer.head.index);
				layerNames.Push (attrib.layer.head.name);
			}
		}
	}

	// 레이어 개수 업데이트
	nLayers = static_cast<short> (layerInds.GetSize ());

	// 보이는 레이어 상의 벽, 슬래브, 보, 기둥 객체를 가져옴
	ACAPI_Element_GetElemList (API_WallID, &elemList_Wall, APIFilt_OnVisLayer);
	ACAPI_Element_GetElemList (API_SlabID, &elemList_Slab, APIFilt_OnVisLayer);
	ACAPI_Element_GetElemList (API_BeamID, &elemList_Beam, APIFilt_OnVisLayer);
	ACAPI_Element_GetElemList (API_ColumnID, &elemList_Column, APIFilt_OnVisLayer);
	while (!elemList_Wall.IsEmpty ())
		elemList_All.Push (elemList_Wall.Pop ());
	while (!elemList_Slab.IsEmpty ())
		elemList_All.Push (elemList_Slab.Pop ());
	while (!elemList_Beam.IsEmpty ())
		elemList_All.Push (elemList_Beam.Pop ());
	while (!elemList_Column.IsEmpty ())
		elemList_All.Push (elemList_Column.Pop ());

	// 모든 리스트 값을 동적 구조체 배열에 복사할 것
	nElems = elemList_All.GetSize ();
	elems = new qElem [nElems];
	for (xx = 0 ; xx < nElems ; ++xx) {
		// 요소 배열 업데이트
		elems [xx].guid = elemList_All.Pop ();

		// 요소를 가져온 후 필드값 채우기
		BNZeroMemory (&elem, sizeof (API_Element));
		BNZeroMemory (&memo, sizeof (API_ElementMemo));
		elem.header.guid = elems [xx].guid;
		err = ACAPI_Element_Get (&elem);						// elem.wall.poly.nCoords : 폴리곤 수를 가져올 수 있음
		err = ACAPI_Element_GetMemo (elem.header.guid, &memo);	// memo.coords : 폴리곤 좌표를 가져올 수 있음
		err = ACAPI_Element_Get3DInfo (elem.header, &info3D);

		// 요소 타입 지정
		if (elem.header.typeID == API_WallID)
			elems [xx].typeOfElem = ELEM_WALL;
		else if (elem.header.typeID == API_SlabID)
			elems [xx].typeOfElem = ELEM_SLAB;
		else if (elem.header.typeID == API_BeamID)
			elems [xx].typeOfElem = ELEM_BEAM;
		else if (elem.header.typeID == API_ColumnID)
			elems [xx].typeOfElem = ELEM_COLUMN;
		else
			elems [xx].typeOfElem = ELEM_UNKNOWN;

		// 층 인덱스 저장
		elems [xx].floorInd = elem.header.floorInd;

		// 레이어 인덱스 저장
		elems [xx].layerInd = elem.header.layer;

		// x, y, z의 최소, 최대값 저장
		elems [xx].bottomPoint.x = info3D.bounds.xMin;
		elems [xx].bottomPoint.y = info3D.bounds.yMin;
		elems [xx].bottomPoint.z = info3D.bounds.zMin;
		elems [xx].topPoint.x = info3D.bounds.xMax;
		elems [xx].topPoint.y = info3D.bounds.yMax;
		elems [xx].topPoint.z = info3D.bounds.zMax;

		// ...
		// bottomPoint, topPoint 변수를 조회하여 다음 항목 채울 것

		// 북쪽 측면
		//elems [xx].NorthLeftBottom;		// 좌하단
		//elems [xx].NorthRightTop;		// 우상단

		// 남쪽 측면
		//elems [xx].SouthLeftBottom;		// 좌하단
		//elems [xx].SouthRightTop;		// 우상단

		// 동쪽 측면
		//elems [xx].EastLeftBottom;		// 좌하단
		//elems [xx].EastRightTop;		// 우상단

		// 서쪽 측면
		//elems [xx].WestLeftBottom;		// 좌하단
		//elems [xx].WestRightTop;		// 우상단

		// 밑면
		//elems [xx].BaseLeftBottom;		// 좌하단
		//elems [xx].BaseRightTop;		// 우하단

		// memo 객체 해제
		ACAPI_DisposeElemMemoHdls (&memo);
	}

	// 객체 타입 저장
	objType.Push ("벽 (내벽)");
	objType.Push ("벽 (외벽)");
	objType.Push ("벽 (합벽)");
	objType.Push ("벽 (파라펫)");
	objType.Push ("벽 (방수턱)");
	objType.Push ("슬래프 (기초)");
	objType.Push ("슬래프 (RC)");
	objType.Push ("슬래프 (데크)");
	objType.Push ("슬래프 (램프)");
	objType.Push ("보");
	objType.Push ("기둥 (독립)");
	objType.Push ("기둥 (벽체)");

	nObjects = static_cast<short> (objType.GetSize ());

	// [다이얼로그] 물량합판 레이어 설정
	result = DGBlankModalDialog (750, 100, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, quantityPlywoodUIHandler, 0);

	if (result != DG_OK)
		return err;

	// 벽 (내벽)					측면(장)
	// 벽 (외벽)					...
	// 벽 (합벽)					...
	// 벽 (파라펫)					...
	// 벽 (방수턱)					...
	// 슬래브 (기초)				하부
	// 슬래브 (RC)					...
	// 슬래브 (데크)				...
	// 슬래브 (램프)				...
	// 보							측면(장), 하부
	// 기둥 (독립)					측면(전)
	// 기둥 (벽체)					...

	for (xx = 0 ; xx < nElems ; ++xx) {
		for (yy = 0 ; yy < nElems ; ++yy) {
			if (xx == yy)
				continue;

			// ...

			// xx가 벽일 경우
			if (elems [xx].typeOfElem == ELEM_WALL) {

				if ((elems [yy].typeOfElem == ELEM_WALL) || (elems [yy].typeOfElem == ELEM_SLAB) || (elems [yy].typeOfElem == ELEM_BEAM) || (elems [yy].typeOfElem == ELEM_COLUMN)) {
					// 측면 1
					// 측면 2
						// yy에 의해 xx의 X-Y 범위가 깎임
				}

			// xx가 슬래브일 경우
			} else if (elems [xx].typeOfElem == ELEM_SLAB) {

				if ((elems [yy].typeOfElem == ELEM_WALL) || (elems [yy].typeOfElem == ELEM_SLAB) || (elems [yy].typeOfElem == ELEM_BEAM) || (elems [yy].typeOfElem == ELEM_COLUMN)) {
					// 밑면
						// yy에 의해 xx의 X-Y 범위가 깎임
				}
			
			// xx가 보일 경우
			} else if (elems [xx].typeOfElem == ELEM_BEAM) {

				if ((elems [yy].typeOfElem == ELEM_WALL) || (elems [yy].typeOfElem == ELEM_SLAB) || (elems [yy].typeOfElem == ELEM_BEAM) || (elems [yy].typeOfElem == ELEM_COLUMN)) {
					// 밑면
						// yy에 의해 xx의 X-Y 범위가 깎임
					// 측면 1
					// 측면 2
						// yy에 의해 xx의 Z 범위가 깎임
				}

			// xx가 기둥일 경우
			} else if (elems [xx].typeOfElem == ELEM_COLUMN) {

				if ((elems [yy].typeOfElem == ELEM_WALL) || (elems [yy].typeOfElem == ELEM_SLAB) || (elems [yy].typeOfElem == ELEM_BEAM) || (elems [yy].typeOfElem == ELEM_COLUMN)) {
					// 측면 1
					// 측면 2
					// 측면 3
					// 측면 4
						// yy에 의해 xx의 Z 범위가 깎임
				}
			}
		}
	}

	delete []	elems;

	return	err;
}

// [다이얼로그] 물량산출 합판 레이어 지정
short DGCALLBACK quantityPlywoodUIHandler (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	short	itmIdx;
	short	xx, yy;
	short	foundExistLayerInd;
	short	foundQLayerInd;
	short	selectedObjType;

	switch (message) {
		case DG_MSG_INIT:
			// 다이얼로그 타이틀
			DGSetDialogTitle (dialogID, "물량합판 부착하기");

			// 확인 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 20, 20, 60, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, "확인");
			DGShowItem (dialogID, DG_OK);

			// 취소 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 20, 55, 60, 25);
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, "취소");
			DGShowItem (dialogID, DG_CANCEL);
			
			// 라벨: 부재 레이어
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 120, 25, 200, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "부재 레이어");
			DGShowItem (dialogID, itmIdx);

			// 라벨: 물량합판 레이어
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 330, 25, 200, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "물량합판 레이어");
			DGShowItem (dialogID, itmIdx);

			// 라벨: 물량합판 타입
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 550, 25, 150, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "물량합판 타입");
			DGShowItem (dialogID, itmIdx);

			itmPosX = 120;
			itmPosY = 50;

			// 기존 레이어: 처음에는 1행만 표시할 것
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 500, 25, 120, itmPosY, 200, 25);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "없음");
			for (xx = 0 ; xx < nLayers ; ++xx) {
				DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, layerNames.Get (xx).c_str ());
			}
			DGShowItem (dialogID, itmIdx);

			existingLayerPopup [0] = itmIdx;
			nPopupControl = 1;

			break;
		
		case DG_MSG_CHANGE:

			if (item == existingLayerPopup [nPopupControl - 1] && (nPopupControl < 50)) {

				// 팝업 컨트롤: 물량합판 레이어
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 500, 25, 330, itmPosY, 200, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				for (xx = 0 ; xx < nLayers ; ++xx) {
					DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, layerNames.Get (xx).c_str ());
				}
				DGShowItem (dialogID, itmIdx);
				qLayerPopup [nPopupControl - 1] = itmIdx;

				// 팝업 컨트롤: 객체 타입
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 500, 25, 550, itmPosY, 150, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				for (xx = 0 ; xx < nObjects ; ++xx) {
					DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, objType.Get (xx).c_str ());
				}
				DGShowItem (dialogID, itmIdx);
				objTypePopup [nPopupControl - 1] = itmIdx;

				itmPosY += 30;

				// 팝업 컨트롤: 기존 레이어 (새로 생성)
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 500, 25, 120, itmPosY, 200, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "없음");
				for (xx = 0 ; xx < nLayers ; ++xx) {
					DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, layerNames.Get (xx).c_str ());
				}
				DGShowItem (dialogID, itmIdx);
				existingLayerPopup [nPopupControl] = itmIdx;

				nPopupControl ++;

				DGGrowDialog (dialogID, 0, 30);
			}

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					for (xx = 0 ; xx < nPopupControl ; ++xx) {
						// 기존 레이어 "없음"이 아닌 경우
						if (DGGetItemValLong (dialogID, existingLayerPopup [xx]) != 0) {

							// 기존 레이어 팝업에서 선택한 이름에 해당하는 레이어 인덱스 찾기
							foundExistLayerInd = findLayerIndex (DGPopUpGetItemText (dialogID, existingLayerPopup [xx], static_cast<short>(DGGetItemValLong (dialogID, existingLayerPopup [xx]))).ToCStr ().Get ());

							// 물량합판 레이어 팝업에서 선택한 이름에 해당하는 레이어 인덱스 찾기
							foundQLayerInd = findLayerIndex (DGPopUpGetItemText (dialogID, qLayerPopup [xx], static_cast<short>(DGGetItemValLong (dialogID, qLayerPopup [xx]))).ToCStr ().Get ());

							// 물량합판 객체 타입
							selectedObjType = static_cast<short> (DGGetItemValLong (dialogID, objTypePopup [xx]));

							for (yy = 0 ; yy < nElems ; ++yy) {
								if (elems [yy].layerInd == foundExistLayerInd) {
									elems [yy].qLayerInd = foundQLayerInd;
									elems [yy].typeOfQPlywood = selectedObjType - 1;	// 열거형 값 할당됨
								}
							}
						}
					}

					break;

				case DG_CANCEL:
					break;

				default:
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

// 레이어 이름으로 레이어 인덱스 찾기
short findLayerIndex (const char* layerName)
{
	short	nLayers;
	short	xx;
	GSErrCode	err;

	API_Attribute	attrib;

	// 프로젝트 내 레이어 개수를 알아냄
	BNZeroMemory (&attrib, sizeof (API_Attribute));
	attrib.header.typeID = API_LayerID;
	err = ACAPI_Attribute_GetNum (API_LayerID, &nLayers);

	// 입력한 레이어 이름과 일치하는 레이어의 인덱스 번호 리턴
	for (xx = 1; xx <= nLayers ; ++xx) {
		attrib.header.index = xx;
		err = ACAPI_Attribute_Get (&attrib);

		if (err == NoError) {
			if (strncmp (attrib.layer.head.name, layerName, strlen (layerName)) == 0) {
				return	attrib.layer.head.index;
			}
		}
	}

	return 0;
}