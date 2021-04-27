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

const GS::uchar_t*	gsmQuantityPlywood = L("물량합판(핫스팟축소).gsm");

// 부재(벽,슬래브,보,기둥)들의 가설재가 붙을 수 있는 면에 물량합판을 자동으로 부착함
GSErrCode	placeQuantityPlywood (void)
{
	GSErrCode	err = NoError;

	API_Element				elem;
	API_ElementMemo			memo;
	API_ElemInfo3D			info3D;

	short	xx, yy, zz;
	short	pp, qq;
	short	result;		// 다이얼로그 리턴 값
	
	API_Attribute	attrib;

	GS::Array<API_Guid>	elemList_All;		// 요소들의 GUID (전체 구조 요소)
	GS::Array<API_Guid>	elemList_Wall;		// 요소들의 GUID (벽)
	GS::Array<API_Guid>	elemList_Slab;		// 요소들의 GUID (슬래브)
	GS::Array<API_Guid>	elemList_Beam;		// 요소들의 GUID (보)
	GS::Array<API_Guid>	elemList_Column;	// 요소들의 GUID (기둥)

	GS::Array<API_Guid> elemList_Mesh;		// 요소들의 GUID (메시)
	GS::Array<API_Guid> elemList_Roof;		// 요소들의 GUID (지붕)
	GS::Array<API_Guid> elemList_Object;	// 요소들의 GUID (객체, 계단 포함)


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

	// 벽, 슬래브, 보, 기둥 객체를 가져옴
	ACAPI_Element_GetElemList (API_WallID, &elemList_Wall, APIFilt_In3D);
	ACAPI_Element_GetElemList (API_SlabID, &elemList_Slab, APIFilt_In3D);
	ACAPI_Element_GetElemList (API_BeamID, &elemList_Beam, APIFilt_In3D);
	ACAPI_Element_GetElemList (API_ColumnID, &elemList_Column, APIFilt_In3D);
	ACAPI_Element_GetElemList (API_MeshID, &elemList_Mesh, APIFilt_In3D);
	ACAPI_Element_GetElemList (API_RoofID, &elemList_Roof, APIFilt_In3D);
	ACAPI_Element_GetElemList (API_ObjectID, &elemList_Object, APIFilt_In3D);
	while (!elemList_Wall.IsEmpty ())
		elemList_All.Push (elemList_Wall.Pop ());
	while (!elemList_Slab.IsEmpty ())
		elemList_All.Push (elemList_Slab.Pop ());
	while (!elemList_Beam.IsEmpty ())
		elemList_All.Push (elemList_Beam.Pop ());
	while (!elemList_Column.IsEmpty ())
		elemList_All.Push (elemList_Column.Pop ());
	while (!elemList_Mesh.IsEmpty ())
		elemList_All.Push (elemList_Mesh.Pop ());
	while (!elemList_Roof.IsEmpty ())
		elemList_All.Push (elemList_Roof.Pop ());
	while (!elemList_Object.IsEmpty ())
		elemList_All.Push (elemList_Object.Pop ());

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

		// 북쪽 측면
		elems [xx].NorthLeftBottom.x = elems [xx].topPoint.x;
		elems [xx].NorthLeftBottom.y = elems [xx].topPoint.y;
		elems [xx].NorthLeftBottom.z = elems [xx].bottomPoint.z;
		elems [xx].NorthRightTop.x = elems [xx].bottomPoint.x;
		elems [xx].NorthRightTop.y = elems [xx].topPoint.y;
		elems [xx].NorthRightTop.z = elems [xx].topPoint.z;

		// 남쪽 측면
		elems [xx].SouthLeftBottom.x = elems [xx].bottomPoint.x;
		elems [xx].SouthLeftBottom.y = elems [xx].bottomPoint.y;
		elems [xx].SouthLeftBottom.z = elems [xx].bottomPoint.z;
		elems [xx].SouthRightTop.x = elems [xx].topPoint.x;
		elems [xx].SouthRightTop.y = elems [xx].bottomPoint.y;
		elems [xx].SouthRightTop.z = elems [xx].topPoint.z;

		// 동쪽 측면
		elems [xx].EastLeftBottom.x = elems [xx].topPoint.x;
		elems [xx].EastLeftBottom.y = elems [xx].bottomPoint.y;
		elems [xx].EastLeftBottom.z = elems [xx].bottomPoint.z;
		elems [xx].EastRightTop.x = elems [xx].topPoint.x;
		elems [xx].EastRightTop.y = elems [xx].topPoint.y;
		elems [xx].EastRightTop.z = elems [xx].topPoint.z;

		// 서쪽 측면
		elems [xx].WestLeftBottom.x = elems [xx].bottomPoint.x;
		elems [xx].WestLeftBottom.y = elems [xx].topPoint.y;
		elems [xx].WestLeftBottom.z = elems [xx].bottomPoint.z;
		elems [xx].WestRightTop.x = elems [xx].bottomPoint.x;
		elems [xx].WestRightTop.y = elems [xx].bottomPoint.y;
		elems [xx].WestRightTop.z = elems [xx].topPoint.z;

		// 밑면
		elems [xx].BaseLeftBottom.x = elems [xx].bottomPoint.x;
		elems [xx].BaseLeftBottom.y = elems [xx].bottomPoint.y;
		elems [xx].BaseLeftBottom.z = elems [xx].bottomPoint.z;
		elems [xx].BaseRightTop.x = elems [xx].topPoint.x;
		elems [xx].BaseRightTop.y = elems [xx].topPoint.y;
		elems [xx].BaseRightTop.z = elems [xx].bottomPoint.z;

		// 유효성 지정
		elems [xx].validNorth = true;
		elems [xx].validSouth = true;
		elems [xx].validEast = true;
		elems [xx].validWest = true;
		elems [xx].validBase = true;

		// 벽 구멍 여부
		if ((elem.wall.hasDoor == TRUE) || (elem.wall.hasWindow == TRUE))
			elems [xx].hasHole = true;
		else
			elems [xx].hasHole = false;

		// 요소 타입 지정
		if (elem.header.typeID == API_WallID) {
			elems [xx].typeOfElem = ELEM_WALL;

			// 벽: 밑면은 유효하지 않음
			elems [xx].invalidateBase (&elems [xx]);

			// 벽에 문이나 창에 의한 구멍이 있으면 긴 면이 유효하지 않음
			if (elems [xx].hasHole == true)
				elems [xx].invalidateLongTwoSide (&elems [xx]);
		} else if (elem.header.typeID == API_SlabID) {
			elems [xx].typeOfElem = ELEM_SLAB;

			// 슬래브: 모든 측면은 유효하지 않음
			elems [xx].invalidateAllSide (&elems [xx]);
		} else if (elem.header.typeID == API_BeamID) {
			elems [xx].typeOfElem = ELEM_BEAM;

			// 보: 짧은 면은 유효하지 않음
			elems [xx].invalidateShortTwoSide (&elems [xx]);
		} else if (elem.header.typeID == API_ColumnID) {
			elems [xx].typeOfElem = ELEM_COLUMN;

			// 기둥: 밑면은 유효하지 않음
			elems [xx].invalidateBase (&elems [xx]);
		} else {
			elems [xx].typeOfElem = ELEM_UNKNOWN;

			// 다른 객체들은 모든 측면과 밑면이 유효하지 않음
			elems [xx].invalidateAllSide (&elems [xx]);
			elems [xx].invalidateBase (&elems [xx]);
		}

		elems [xx].nQPlywoods = 0;		// 부착할 물량합판들의 GUID
		elems [xx].nOtherElems = 0;		// 간섭을 일으키는 다른 요소들의 GUID

		// memo 객체 해제
		ACAPI_DisposeElemMemoHdls (&memo);
	}

	// 객체 타입 저장
	objType.Push ("벽 (내벽)");
	objType.Push ("벽 (외벽)");
	objType.Push ("벽 (합벽)");
	objType.Push ("벽 (파라펫)");
	objType.Push ("벽 (방수턱)");
	objType.Push ("슬래브 (기초)");
	objType.Push ("슬래브 (RC)");
	objType.Push ("슬래브 (데크)");
	objType.Push ("슬래브 (램프)");
	objType.Push ("보");
	objType.Push ("기둥 (독립)");
	objType.Push ("기둥 (벽체)");

	nObjects = static_cast<short> (objType.GetSize ());

	// [다이얼로그] 물량합판 레이어 설정
	result = DGBlankModalDialog (750, 100, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, quantityPlywoodUIHandler, 0);

	if (result != DG_OK) {
		delete []	elems;
		return err;
	}

	for (xx = 0 ; xx < nElems ; ++xx) {
		for (yy = 0 ; yy < nElems ; ++yy) {
			// 동일한 요소끼리는 비교하지 않음
			if (xx == yy)	continue;

			// 간섭하는 요소를 찾아내어 GUID를 저장함
			elems [xx].subtractArea (&elems [xx], elems [yy]);
		}
	}

	// 물량합판 객체 배치
	for (xx = 0 ; xx < nElems ; ++xx) {
		elems [xx].placeQuantityPlywood (&elems [xx]);
	}

	// 솔리드 연산을 통해 물량합판을 공제함
	for (xx = 0 ; xx < nElems ; ++xx) {
		for (yy = 0 ; yy < elems [xx].nQPlywoods ; ++yy) {
			for (zz = 0 ; zz < elems [xx].nOtherElems ; ++zz) {

				// 겹치는 요소에 의해 물량합판이 공제됨 (1번째 파라미터: Target, 2번째 파라미터: Operator)
				err = ACAPI_Element_SolidLink_Create (elems [xx].qPlywoodGuids [yy], elems [xx].otherGuids [zz], APISolid_Substract, APISolidFlag_OperatorAttrib);

				// Operator에게 부착되어 있는 물량합판들의 GUID를 가져와서 역시 공제시킴
				for (pp = 0 ; pp < nElems ; ++pp) {
					if ((elems [xx].otherGuids [zz] == elems [pp].guid) && (pp != xx)) {
						for (qq = 0 ; qq < elems [pp].nQPlywoods ; ++qq) {
							err = ACAPI_Element_SolidLink_Create (elems [xx].qPlywoodGuids [yy], elems [pp].qPlywoodGuids [qq], APISolid_Substract, APISolidFlag_OperatorAttrib);
						}
					}
				}
			}
		}
	}

	delete []	elems;

	// 화면 새로고침
	ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
	bool	regenerate = true;
	ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

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
				DGPopUpDisableDraw (dialogID, itmIdx);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				for (xx = 0 ; xx < nLayers ; ++xx) {
					DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, layerNames.Get (xx).c_str ());
				}
				DGShowItem (dialogID, itmIdx);
				DGPopUpEnableDraw (dialogID, itmIdx);
				qLayerPopup [nPopupControl - 1] = itmIdx;

				// 팝업 컨트롤: 객체 타입
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 500, 25, 550, itmPosY, 150, 25);
				DGPopUpDisableDraw (dialogID, itmIdx);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				for (xx = 0 ; xx < nObjects ; ++xx) {
					DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, objType.Get (xx).c_str ());
				}
				DGShowItem (dialogID, itmIdx);
				DGPopUpEnableDraw (dialogID, itmIdx);
				objTypePopup [nPopupControl - 1] = itmIdx;

				itmPosY += 30;

				// 팝업 컨트롤: 기존 레이어 (새로 생성)
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 500, 25, 120, itmPosY, 200, 25);
				DGPopUpDisableDraw (dialogID, itmIdx);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "없음");
				for (xx = 0 ; xx < nLayers ; ++xx) {
					DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
					DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, layerNames.Get (xx).c_str ());
				}
				DGShowItem (dialogID, itmIdx);
				DGPopUpEnableDraw (dialogID, itmIdx);
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

							// 요소별 레이어 정보 할당
							for (yy = 0 ; yy < nElems ; ++yy) {
								if (elems [yy].layerInd == foundExistLayerInd) {
									elems [yy].qLayerInd = foundQLayerInd;
									elems [yy].typeOfQPlywood = selectedObjType - 1;	// 열거형 값 할당됨
									
									// 만약 물량합판 타입이 슬래브 (기초)인 경우, [Q_SLAB_BASE]
									if (elems [yy].typeOfQPlywood == Q_SLAB_BASE) {
										// 밑면은 붙이지 않고 옆면은 모두 붙임
										elems [yy].invalidateBase (&elems [yy]);
										elems [yy].validateAllSide (&elems [yy]);
									}
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

// 4개의 측면 중 짧은 2개의 측면을 무효화하고, 유효한 면 정보를 리턴함. 리턴 값은 다음 값의 조합입니다. 북쪽/남쪽(2), 동쪽/서쪽(1), 오류(0)
short qElem::invalidateShortTwoSide (qElem* element)
{
	short	result = 0;

	double	northWidth = GetDistance (element->NorthLeftBottom.x, element->NorthLeftBottom.y, element->NorthRightTop.x, element->NorthRightTop.y);
	double	eastWidth = GetDistance (element->EastLeftBottom.x, element->EastLeftBottom.y, element->EastRightTop.x, element->EastRightTop.y);

	// 북쪽/남쪽이 길 경우
	if (northWidth - eastWidth > EPS) {
		// 동쪽/서쪽 면을 무효화
		element->validEast = false;
		element->validWest = false;

		result = 2;

	// 동쪽/서쪽이 길 경우
	} else {
		// 북쪽/남쪽 면을 무효화
		element->validNorth = false;
		element->validSouth = false;

		result = 1;
	}

	return result;
}

// 4개의 측면 중 긴 2개의 측면을 무효화하고, 유효한 면 정보를 리턴함. 리턴 값은 다음 값의 조합입니다. 북쪽/남쪽(2), 동쪽/서쪽(1), 오류(0)
short qElem::invalidateLongTwoSide (qElem* element)
{
	short	result = 0;

	double	northWidth = GetDistance (element->NorthLeftBottom.x, element->NorthLeftBottom.y, element->NorthRightTop.x, element->NorthRightTop.y);
	double	eastWidth = GetDistance (element->EastLeftBottom.x, element->EastLeftBottom.y, element->EastRightTop.x, element->EastRightTop.y);

	// 북쪽/남쪽이 길 경우
	if (northWidth - eastWidth > EPS) {
		// 북쪽/남쪽 면을 무효화
		element->validNorth = false;
		element->validSouth = false;

		result = 1;

	// 동쪽/서쪽이 길 경우
	} else {
		// 동쪽/서쪽 면을 무효화
		element->validEast = false;
		element->validWest = false;

		result = 2;
	}

	return result;
}

// 4개의 측면을 모두 무효화함
void qElem::invalidateAllSide (qElem* element)
{
	element->validNorth = false;
	element->validSouth = false;
	element->validEast = false;
	element->validWest = false;
}

// 밑면을 무효화함
void qElem::invalidateBase (qElem* element)
{
	element->validBase = false;
}

// 4개의 측면을 모두 유효화함
void qElem::validateAllSide (qElem* element)
{
	element->validNorth = true;
	element->validSouth = true;
	element->validEast = true;
	element->validWest = true;
}

// 밑면을 유효화함
void qElem::validateBase (qElem* element)
{
	element->validBase = true;
}

// src 요소의 측면, 밑면 영역이 operand 요소에 의해 침범 당할 경우, 솔리드 연산을 위해 operand의 GUID를 저장함
bool qElem::subtractArea (qElem* src, qElem operand)
{
	bool precondition = false;
	bool result = false;

	// 전제 조건: src는 벽, 슬래브, 보, 기둥이어야 함
	if ((src->typeOfElem == ELEM_WALL) || (src->typeOfElem == ELEM_SLAB) || (src->typeOfElem == ELEM_BEAM) || (src->typeOfElem == ELEM_COLUMN))
		precondition = true;

	// 전제 조건을 만족하지 않으면 실패
	if (precondition == false)
		return false;

	// 북쪽 측면에 물량합판을 붙일 경우
	if (src->validNorth == true) {
		//선행조건
		//	(1) src 측면의 X값 범위 안에 operand의 X값 범위가 침범하는가?
		//	(2) src 측면의 Y값이 operand의 Y값 범위 안에 들어가는가?
		//	(3) src 측면의 Z값 범위 안에 operand의 Z값 범위가 침범하는가?
		if ((overlapRange (src->NorthRightTop.x, src->NorthLeftBottom.x, operand.bottomPoint.x, operand.topPoint.x) > EPS) &&
			inRange (src->NorthLeftBottom.y, operand.bottomPoint.y, operand.topPoint.y) &&
			(overlapRange (src->NorthLeftBottom.z, src->NorthRightTop.z, operand.bottomPoint.z, operand.topPoint.z) > EPS)) {

				// operand.guid를 src의 otherGuids에 넣고 nOtherElems에 넣을 것
				src->otherGuids [src->nOtherElems ++] = operand.guid;
		}

		result = true;
	}

	// 남쪽 측면에 물량합판을 붙일 경우
	if (src->validSouth == true) {
		//선행조건
		//	(1) src 측면의 X값 범위 안에 operand의 X값 범위가 침범하는가?
		//	(2) src 측면의 Y값이 operand의 Y값 범위 안에 들어가는가?
		//	(3) src 측면의 Z값 범위 안에 operand의 Z값 범위가 침범하는가?
		if ((overlapRange (src->SouthLeftBottom.x, src->SouthRightTop.x, operand.bottomPoint.x, operand.topPoint.x) > EPS) &&
			inRange (src->SouthLeftBottom.y, operand.bottomPoint.y, operand.topPoint.y) &&
			(overlapRange (src->SouthLeftBottom.z, src->SouthRightTop.z, operand.bottomPoint.z, operand.topPoint.z) > EPS)) {

				// operand.guid를 src의 otherGuids에 넣고 nOtherElems에 넣을 것
				src->otherGuids [src->nOtherElems ++] = operand.guid;
		}

		result = true;
	}

	// 동쪽 측면에 물량합판을 붙일 경우
	if (src->validEast == true) {
		//선행조건
		//	(1) src 측면의 X값이 operand의 X값 범위 안에 들어가는가?
		//	(2) src 측면의 Y값 범위 안에 operand의 Y값 범위가 침범하는가?
		//	(3) src 측면의 Z값 범위 안에 operand의 Z값 범위가 침범하는가?
		if (inRange (src->EastLeftBottom.x, operand.bottomPoint.x, operand.topPoint.x) &&
			(overlapRange (src->EastLeftBottom.y, src->EastRightTop.y, operand.bottomPoint.y, operand.topPoint.y) > EPS) &&
			(overlapRange (src->EastLeftBottom.z, src->EastRightTop.z, operand.bottomPoint.z, operand.topPoint.z) > EPS)) {

				// operand.guid를 src의 otherGuids에 넣고 nOtherElems에 넣을 것
				src->otherGuids [src->nOtherElems ++] = operand.guid;
		}

		result = true;
	}

	// 서쪽 측면에 물량합판을 붙일 경우
	if (src->validWest == true) {
		//선행조건
		//	(1) src 측면의 X값이 operand의 X값 범위 안에 들어가는가?
		//	(2) src 측면의 Y값 범위 안에 operand의 Y값 범위가 침범하는가?
		//	(3) src 측면의 Z값 범위 안에 operand의 Z값 범위가 침범하는가?
		if (inRange (src->WestLeftBottom.x, operand.bottomPoint.x, operand.topPoint.x) &&
			(overlapRange (src->WestRightTop.y, src->WestLeftBottom.y, operand.bottomPoint.y, operand.topPoint.y) > EPS) &&
			(overlapRange (src->WestLeftBottom.z, src->WestRightTop.z, operand.bottomPoint.z, operand.topPoint.z) > EPS)) {

				// operand.guid를 src의 otherGuids에 넣고 nOtherElems에 넣을 것
				src->otherGuids [src->nOtherElems ++] = operand.guid;
		}

		result = true;
	}

	// 밑면에 물량합판을 붙일 경우
	if (src->validBase == true) {
		//선행조건
		//	(1) src 밑면의 X값 범위 안에 operand의 X값 범위가 침범하는가?
		//	(2) src 밑면의 Y값 범위 안에 operand의 Y값 범위가 침범하는가?
		//	(3) src 밑면의 Z값이 operand의 Z값 범위 안에 들어가는가?
		if ((overlapRange (src->BaseLeftBottom.x, src->BaseRightTop.x, operand.bottomPoint.x, operand.topPoint.x) > EPS) &&
			(overlapRange (src->BaseLeftBottom.y, src->BaseRightTop.y, operand.bottomPoint.y, operand.topPoint.y) > EPS) &&
			inRange (src->BaseLeftBottom.z, operand.bottomPoint.z, operand.topPoint.z)) {

				// operand.guid를 src의 otherGuids에 넣고 nOtherElems에 넣을 것
				src->otherGuids [src->nOtherElems ++] = operand.guid;
		}

		result = true;
	}

	return	result;
}

// 요소의 측면들과 밑면 영역에 물량합판을 부착함
void qElem::placeQuantityPlywood (qElem* element)
{
	GSErrCode	err = NoError;
	API_Element			elem;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	const GS::uchar_t*	gsmName = gsmQuantityPlywood;
	double				aParam;
	double				bParam;
	Int32				addParNum;

	std::string			tempStr;
	double				horLen, verLen;

	// 작업 층 정보
	short			xx;
	API_StoryInfo	storyInfo;
	double			workLevel;		// 벽의 작업 층 높이


	// 작업 층 높이 반영
	BNZeroMemory (&storyInfo, sizeof (API_StoryInfo));
	workLevel = 0.0;
	ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo);
	for (xx = 0 ; xx < (storyInfo.lastStory - storyInfo.firstStory) ; ++xx) {
		if (storyInfo.data [0][xx].index == element->floorInd) {
			workLevel = storyInfo.data [0][xx].level;
			break;
		}
	}
	BMKillHandle ((GSHandle *) &storyInfo.data);

	// 객체 로드
	BNZeroMemory (&elem, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, gsmName);
	err = ACAPI_LibPart_Search (&libPart, false);
	if (libPart.location != NULL)
		delete libPart.location;
	if (err != NoError)
		return;

	ACAPI_LibPart_Get (&libPart);

	elem.header.typeID = API_ObjectID;
	elem.header.guid = GSGuid2APIGuid (GS::Guid (libPart.ownUnID));

	ACAPI_Element_GetDefaults (&elem, &memo);
	ACAPI_LibPart_GetParams (libPart.index, &aParam, &bParam, &addParNum, &memo.params);

	// 라이브러리의 파라미터 값 입력 (공통)
	elem.header.floorInd = element->floorInd;
	elem.object.libInd = libPart.index;
	elem.object.xRatio = aParam;
	elem.object.yRatio = bParam;
	elem.header.layer = element->qLayerInd;	// 물량합판 레이어

	// 분류: 물량합판 객체의 타입 (enum qPlywoodType 참조)
	if (element->typeOfQPlywood == Q_WALL_INNER) {
		tempStr = "벽체(내벽)";
		memo.params [0][82].value.real = 75;
	} else if (element->typeOfQPlywood == Q_WALL_OUTER) {
		tempStr = "벽체(외벽)";
		memo.params [0][82].value.real = 76;
	} else if (element->typeOfQPlywood == Q_WALL_COMPOSITE) {
		tempStr = "벽체(합벽)";
		memo.params [0][82].value.real = 72;
	} else if (element->typeOfQPlywood == Q_WALL_PARAPET) {
		tempStr = "벽체(파라펫)";
		memo.params [0][82].value.real = 32;
	} else if (element->typeOfQPlywood == Q_WALL_WATERPROOF) {
		tempStr = "벽체(방수턱)";
		memo.params [0][82].value.real = 12;
	} else if (element->typeOfQPlywood == Q_SLAB_BASE) {
		tempStr = "스라브(기초)";
		memo.params [0][82].value.real = 66;
	} else if (element->typeOfQPlywood == Q_SLAB_RC) {
		tempStr = "스라브(RC)";
		memo.params [0][82].value.real = 100;
	} else if (element->typeOfQPlywood == Q_SLAB_DECK) {
		tempStr = "스라브(데크)";
		memo.params [0][82].value.real = 99;
	} else if (element->typeOfQPlywood == Q_SLAB_RAMP) {
		tempStr = "스라브(램프)";
		memo.params [0][82].value.real = 3;
	} else if (element->typeOfQPlywood == Q_BEAM) {
		tempStr = "보";
		memo.params [0][82].value.real = 78;
	} else if (element->typeOfQPlywood == Q_COLUMN_ISOLATED) {
		tempStr = "기둥(독립)";
		memo.params [0][82].value.real = 20;
	} else if (element->typeOfQPlywood == Q_COLUMN_INWALL) {
		tempStr = "기둥(벽체)";
		memo.params [0][82].value.real = 77;
	} else {
		tempStr = "벽체(외벽)";
	}
	GS::ucscpy (memo.params [0][30].value.uStr, GS::UniString (tempStr.c_str ()).ToUStr ().Get ());

	// 합판두께 (문자열)
	tempStr = "12";		// 12mm
	GS::ucscpy (memo.params [0][31].value.uStr, GS::UniString (tempStr.c_str ()).ToUStr ().Get ());

	// 설치위치
	tempStr = "비규격";
	GS::ucscpy (memo.params [0][32].value.uStr, GS::UniString (tempStr.c_str ()).ToUStr ().Get ());

	// 품명
	tempStr = "합판면적";
	GS::ucscpy (memo.params [0][33].value.uStr, GS::UniString (tempStr.c_str ()).ToUStr ().Get ());

	// 북쪽 측면
	if (element->validNorth == true) {
		// 각도 및 시작점
		elem.object.angle = DegreeToRad (180.0);
		elem.object.pos.x = element->NorthLeftBottom.x;
		elem.object.pos.y = element->NorthLeftBottom.y;
		elem.object.level = element->NorthLeftBottom.z - workLevel;

		// 설치방향
		tempStr = "벽에 세우기";
		GS::ucscpy (memo.params [0][34].value.uStr, GS::UniString (tempStr.c_str ()).ToUStr ().Get ());

		// 가로길이
		horLen = abs (element->NorthLeftBottom.x - element->NorthRightTop.x);
		memo.params [0][38].value.real = horLen;

		// 세로길이
		verLen = abs (element->NorthRightTop.z - element->NorthLeftBottom.z);
		memo.params [0][42].value.real = verLen;

		// 객체 배치
		if ((horLen > EPS) && (verLen > EPS)) {
			ACAPI_Element_Create (&elem, &memo);
			element->qPlywoodGuids [element->nQPlywoods ++] = elem.header.guid;
		}
	}

	// 남쪽 측면
	if (element->validSouth == true) {
		// 각도 및 시작점
		elem.object.angle = DegreeToRad (0.0);
		elem.object.pos.x = element->SouthLeftBottom.x;
		elem.object.pos.y = element->SouthLeftBottom.y;
		elem.object.level = element->SouthLeftBottom.z - workLevel;

		// 설치방향
		tempStr = "벽에 세우기";
		GS::ucscpy (memo.params [0][34].value.uStr, GS::UniString (tempStr.c_str ()).ToUStr ().Get ());

		// 가로길이
		horLen = abs (element->SouthRightTop.x - element->SouthLeftBottom.x);
		memo.params [0][38].value.real = horLen;

		// 세로길이
		verLen = abs (element->SouthRightTop.z - element->SouthLeftBottom.z);
		memo.params [0][42].value.real = verLen;

		// 객체 배치
		if ((horLen > EPS) && (verLen > EPS)) {
			ACAPI_Element_Create (&elem, &memo);
			element->qPlywoodGuids [element->nQPlywoods ++] = elem.header.guid;
		}
	}

	// 동쪽 측면
	if (element->validEast == true) {
		// 각도 및 시작점
		elem.object.angle = DegreeToRad (90.0);
		elem.object.pos.x = element->EastLeftBottom.x;
		elem.object.pos.y = element->EastLeftBottom.y;
		elem.object.level = element->EastLeftBottom.z - workLevel;

		// 설치방향
		tempStr = "벽에 세우기";
		GS::ucscpy (memo.params [0][34].value.uStr, GS::UniString (tempStr.c_str ()).ToUStr ().Get ());

		// 가로길이
		horLen = abs (element->EastRightTop.y - element->EastLeftBottom.y);
		memo.params [0][38].value.real = horLen;

		// 세로길이
		verLen = abs (element->EastRightTop.z - element->EastLeftBottom.z);
		memo.params [0][42].value.real = verLen;

		// 객체 배치
		if ((horLen > EPS) && (verLen > EPS)) {
			ACAPI_Element_Create (&elem, &memo);
			element->qPlywoodGuids [element->nQPlywoods ++] = elem.header.guid;
		}
	}

	// 서쪽 측면
	if (element->validWest == true) {
		// 각도 및 시작점
		elem.object.angle = DegreeToRad (270.0);
		elem.object.pos.x = element->WestLeftBottom.x;
		elem.object.pos.y = element->WestLeftBottom.y;
		elem.object.level = element->WestLeftBottom.z - workLevel;

		// 설치방향
		tempStr = "벽에 세우기";
		GS::ucscpy (memo.params [0][34].value.uStr, GS::UniString (tempStr.c_str ()).ToUStr ().Get ());

		// 가로길이
		horLen = abs (element->WestLeftBottom.y - element->WestRightTop.y);
		memo.params [0][38].value.real = horLen;

		// 세로길이
		verLen = abs (element->WestRightTop.z - element->WestLeftBottom.z);
		memo.params [0][42].value.real = verLen;

		// 객체 배치
		if ((horLen > EPS) && (verLen > EPS)) {
			ACAPI_Element_Create (&elem, &memo);
			element->qPlywoodGuids [element->nQPlywoods ++] = elem.header.guid;
		}
	}

	// 밑면
	if (element->validBase == true) {
		// 각도 및 시작점
		elem.object.angle = DegreeToRad (0.0);
		elem.object.pos.x = element->BaseLeftBottom.x;
		elem.object.pos.y = element->BaseLeftBottom.y;
		elem.object.level = element->BaseLeftBottom.z - workLevel;

		// 설치방향
		tempStr = "바닥깔기";
		GS::ucscpy (memo.params [0][34].value.uStr, GS::UniString (tempStr.c_str ()).ToUStr ().Get ());

		// 가로길이
		horLen = abs (element->BaseRightTop.x - element->BaseLeftBottom.x);
		memo.params [0][38].value.real = horLen;

		// 세로길이
		verLen = abs (element->BaseRightTop.y - element->BaseLeftBottom.y);
		memo.params [0][42].value.real = verLen;

		// 객체 배치
		if ((horLen > EPS) && (verLen > EPS)) {
			ACAPI_Element_Create (&elem, &memo);
			element->qPlywoodGuids [element->nQPlywoods ++] = elem.header.guid;
		}
	}

	ACAPI_DisposeElemMemoHdls (&memo);
}

// 종류별 물량합판의 표면적 합계 값을 구함
GSErrCode	calcAreasOfQuantityPlywood (void)
{
	GSErrCode	err = NoError;
	short	xx;

	API_Element				elem;
	API_ElementMemo			memo;

	GS::Array<API_Guid>		elemList_QuantityPlywood;	// 물량합판들의 GUID

	qElemType		qElemType;	// 물량합판 정보 분류별 저장

	// 파라미터 값 저장
	const char*		typeStr;
	double			thk;

	// 수량 파라미터 가져오기
	API_ElementQuantity	quantity;
	API_Quantities		quantities;
	API_QuantitiesMask	mask;
	API_QuantityPar		params;

	// 텍스트 창
	API_NewWindowPars	windowPars;
	API_WindowInfo		windowInfo;
	char				buffer [256];


	// 면적 값 초기화
	qElemType.initAreas (&qElemType);

	// 텍스트 창 열기
	BNZeroMemory (&windowPars, sizeof (API_NewWindowPars));
	windowPars.typeID = APIWind_MyTextID;
	windowPars.userRefCon = 1;
	GS::snuprintf (windowPars.wTitle, sizeof (windowPars.wTitle) / sizeof (GS::uchar_t), L("물량합판 종류별 덮은 면적 정보"));
	err = ACAPI_Database (APIDb_NewWindowID, &windowPars, NULL);

	// 물량합판 객체를 가져옴
	ACAPI_Element_GetElemList (API_ObjectID, &elemList_QuantityPlywood, APIFilt_In3D);

	// 물량합판 객체가 갖고 있는 파라미터의 값들을 가져옴
	nElems = elemList_QuantityPlywood.GetSize ();
	for (xx = 0 ; xx < nElems ; ++xx) {
		// 요소 가져오기
		BNZeroMemory (&elem, sizeof (API_Element));
		BNZeroMemory (&memo, sizeof (API_ElementMemo));
		elem.header.guid = elemList_QuantityPlywood.Pop ();
		err = ACAPI_Element_Get (&elem);
		err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

		// 수량 정보 가져오기
		ACAPI_ELEMENT_QUANTITY_MASK_CLEAR (mask);
		ACAPI_ELEMENT_QUANTITY_MASK_SET (mask, symb, surface);
		ACAPI_ELEMENT_QUANTITY_MASK_SET (mask, symb, volume);
		quantities.elements = &quantity;
		params.minOpeningSize = EPS;
		err = ACAPI_Element_GetQuantities (elem.header.guid, &params, &quantities, &mask);

		// 분류 가져오기
		typeStr = getParameterStringByName (&memo, "m_type");

		if (strncmp (typeStr, "벽체(내벽)", strlen ("벽체(내벽)")) == 0) {
			thk = atoi (getParameterStringByName (&memo, "m_size"));
			thk = thk / 1000;	// mm 단위를 m 단위로 변환
			qElemType.areas_wall_in += quantity.symb.volume / thk;

		} else if (strncmp (typeStr, "벽체(외벽)", strlen ("벽체(외벽)")) == 0) {
			thk = atoi (getParameterStringByName (&memo, "m_size"));
			thk = thk / 1000;	// mm 단위를 m 단위로 변환
			qElemType.areas_wall_out += quantity.symb.volume / thk;

		} else if (strncmp (typeStr, "벽체(합벽)", strlen ("벽체(합벽)")) == 0) {
			thk = atoi (getParameterStringByName (&memo, "m_size"));
			thk = thk / 1000;	// mm 단위를 m 단위로 변환
			qElemType.areas_wall_composite += quantity.symb.volume / thk;

		} else if (strncmp (typeStr, "벽체(파라펫)", strlen ("벽체(파라펫)")) == 0) {
			thk = atoi (getParameterStringByName (&memo, "m_size"));
			thk = thk / 1000;	// mm 단위를 m 단위로 변환
			qElemType.areas_wall_parapet += quantity.symb.volume / thk;

		} else if (strncmp (typeStr, "벽체(방수턱)", strlen ("벽체(방수턱)")) == 0) {
			thk = atoi (getParameterStringByName (&memo, "m_size"));
			thk = thk / 1000;	// mm 단위를 m 단위로 변환
			qElemType.areas_wall_waterproof += quantity.symb.volume / thk;

		} else if (strncmp (typeStr, "스라브(기초)", strlen ("스라브(기초)")) == 0) {
			thk = atoi (getParameterStringByName (&memo, "m_size"));
			thk = thk / 1000;	// mm 단위를 m 단위로 변환
			qElemType.areas_slab_base += quantity.symb.volume / thk;

		} else if (strncmp (typeStr, "스라브(RC)", strlen ("스라브(RC)")) == 0) {
			thk = atoi (getParameterStringByName (&memo, "m_size"));
			thk = thk / 1000;	// mm 단위를 m 단위로 변환
			qElemType.areas_slab_rc += quantity.symb.volume / thk;

		} else if (strncmp (typeStr, "스라브(데크)", strlen ("스라브(데크)")) == 0) {
			thk = atoi (getParameterStringByName (&memo, "m_size"));
			thk = thk / 1000;	// mm 단위를 m 단위로 변환
			qElemType.areas_slab_deck += quantity.symb.volume / thk;

		} else if (strncmp (typeStr, "스라브(램프)", strlen ("스라브(램프)")) == 0) {
			thk = atoi (getParameterStringByName (&memo, "m_size"));
			thk = thk / 1000;	// mm 단위를 m 단위로 변환
			qElemType.areas_slab_ramp += quantity.symb.volume / thk;

		} else if (strncmp (typeStr, "보", strlen ("보")) == 0) {
			thk = atoi (getParameterStringByName (&memo, "m_size"));
			thk = thk / 1000;	// mm 단위를 m 단위로 변환
			qElemType.areas_beam += quantity.symb.volume / thk;

		} else if (strncmp (typeStr, "기둥(독립)", strlen ("기둥(독립)")) == 0) {
			thk = atoi (getParameterStringByName (&memo, "m_size"));
			thk = thk / 1000;	// mm 단위를 m 단위로 변환
			qElemType.areas_column_iso += quantity.symb.volume / thk;

		} else if (strncmp (typeStr, "기둥(벽체)", strlen ("기둥(벽체)")) == 0) {
			thk = atoi (getParameterStringByName (&memo, "m_size"));
			thk = thk / 1000;	// mm 단위를 m 단위로 변환
			qElemType.areas_column_inwall += quantity.symb.volume / thk;

		} else if (strncmp (typeStr, "기둥(원형)", strlen ("기둥(원형)")) == 0) {
			thk = atoi (getParameterStringByName (&memo, "m_size"));
			thk = thk / 1000;	// mm 단위를 m 단위로 변환
			qElemType.areas_column_circle += quantity.symb.volume / thk;

		} else if (strncmp (typeStr, "램프구간(벽)", strlen ("램프구간(벽)")) == 0) {
			thk = getParameterValueByName (&memo, "m_size");
			thk = thk / 1000;	// mm 단위를 m 단위로 변환
			qElemType.areas_ramp_wall += quantity.symb.volume / thk;
		}

		// memo 객체 해제
		ACAPI_DisposeElemMemoHdls (&memo);
	}

	// 텍스트 창으로 정리해서 보여주기
	BNZeroMemory (&windowInfo, sizeof (API_WindowInfo));
	windowInfo.typeID = APIWind_MyTextID;
	windowInfo.index = 1;

	ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, "[물량합판 분류별 면적]\n");

	sprintf (buffer, "벽체(내벽): %lf ㎡\n", qElemType.areas_wall_in);
	ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
	sprintf (buffer, "벽체(외벽): %lf ㎡\n", qElemType.areas_wall_out);
	ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
	sprintf (buffer, "벽체(합벽): %lf ㎡\n", qElemType.areas_wall_composite);
	ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
	sprintf (buffer, "벽체(파라펫): %lf ㎡\n", qElemType.areas_wall_parapet);
	ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
	sprintf (buffer, "벽체(방수턱): %lf ㎡\n", qElemType.areas_wall_waterproof);
	ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
	sprintf (buffer, "슬래브(기초): %lf ㎡\n", qElemType.areas_slab_base);
	ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
	sprintf (buffer, "슬래브(RC): %lf ㎡\n", qElemType.areas_slab_rc);
	ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
	sprintf (buffer, "슬래브(데크): %lf ㎡\n", qElemType.areas_slab_deck);
	ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
	sprintf (buffer, "슬래브(램프): %lf ㎡\n", qElemType.areas_slab_ramp);
	ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
	sprintf (buffer, "보: %lf ㎡\n", qElemType.areas_beam);
	ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
	sprintf (buffer, "기둥(독립): %lf ㎡\n", qElemType.areas_column_iso);
	ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
	sprintf (buffer, "기둥(벽체): %lf ㎡\n", qElemType.areas_column_inwall);
	ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
	sprintf (buffer, "기둥(원형): %lf ㎡\n", qElemType.areas_column_circle);
	ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
	sprintf (buffer, "램프구간(벽): %lf ㎡\n", qElemType.areas_ramp_wall);
	ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);

	return	err;
}

// 면적 값을 초기화
void	qElemType::initAreas (qElemType* elem)
{
	elem->areas_wall_in = 0.0;			// 면적: 벽체(내벽)
	elem->areas_wall_out = 0.0;			// 면적: 벽체(외벽)
	elem->areas_wall_composite = 0.0;	// 면적: 벽체(합벽)
	elem->areas_wall_parapet = 0.0;		// 면적: 벽체(파라펫)
	elem->areas_wall_waterproof = 0.0;	// 면적: 벽체(방수턱)
	elem->areas_slab_base = 0.0;		// 면적: 스라브(기초)
	elem->areas_slab_rc = 0.0;			// 면적: 스라브(RC)
	elem->areas_slab_deck = 0.0;		// 면적: 스라브(데크)
	elem->areas_slab_ramp = 0.0;		// 면적: 스라브(램프)
	elem->areas_beam = 0.0;				// 면적: 보
	elem->areas_column_iso = 0.0;		// 면적: 기둥(독립)
	elem->areas_column_inwall = 0.0;	// 면적: 기둥(벽체)
	elem->areas_column_circle = 0.0;	// 면적: 기둥(원형)
}