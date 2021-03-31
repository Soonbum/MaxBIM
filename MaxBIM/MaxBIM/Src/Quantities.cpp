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

		// 요소 타입 지정
		if (elem.header.typeID == API_WallID) {
			elems [xx].typeOfElem = ELEM_WALL;

			// 벽: 짧은 면과 밑면은 유효하지 않음
			invalidateShortTwoSide (&elems [xx]);
			invalidateBase (&elems [xx]);
		} else if (elem.header.typeID == API_SlabID) {
			elems [xx].typeOfElem = ELEM_SLAB;

			// 슬래브: 모든 측면은 유효하지 않음
			invalidateAllSide (&elems [xx]);
		} else if (elem.header.typeID == API_BeamID) {
			elems [xx].typeOfElem = ELEM_BEAM;

			// 보: 짧은 면은 유효하지 않음
			invalidateShortTwoSide (&elems [xx]);
		} else if (elem.header.typeID == API_ColumnID) {
			elems [xx].typeOfElem = ELEM_COLUMN;

			// 기둥: 밑면은 유효하지 않음
			invalidateBase (&elems [xx]);
		} else {
			elems [xx].typeOfElem = ELEM_UNKNOWN;

			// 다른 객체들은 모든 측면과 밑면이 유효하지 않음
			invalidateAllSide (&elems [xx]);
			invalidateBase (&elems [xx]);
		}

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

	for (xx = 0 ; xx < nElems ; ++xx) {
		for (yy = 0 ; yy < nElems ; ++yy) {
			// 동일한 요소끼리는 비교하지 않음
			if (xx == yy)	continue;

			// 간섭 영역 피하기
			subtractArea (&elems [xx], elems [yy]);
		}
	}

	// ... 객체 배치
	// ... 비규격 타입 + 측면은 벽에 세우기, 밑면은 바닥깔기
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

// 4개의 측면 중 짧은 2개의 측면을 무효화하고, 유효한 면 정보를 리턴함. 리턴 값은 다음 값의 조합입니다. 북쪽/남쪽(2), 동쪽/서쪽(1), 오류(0)
short invalidateShortTwoSide (qElem* element)
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

// 4개의 측면을 모두 무효화함
void invalidateAllSide (qElem* element)
{
	element->validNorth = false;
	element->validSouth = false;
	element->validEast = false;
	element->validWest = false;
}

// 밑면을 무효화함
void invalidateBase (qElem* element)
{
	element->validBase = false;
}

// src 요소의 측면, 밑면 영역이 operand 요소에 의해 감소됨
bool subtractArea (qElem* src, qElem operand)
{
	bool precondition = false;
	bool result = false;

	// 전제 조건: src, operand는 벽, 슬래브, 보, 기둥이어야 함
	if ((src->typeOfElem == ELEM_WALL) || (src->typeOfElem == ELEM_SLAB) || (src->typeOfElem == ELEM_BEAM) || (src->typeOfElem == ELEM_COLUMN))
		if ((operand.typeOfElem == ELEM_WALL) || (operand.typeOfElem == ELEM_SLAB) || (operand.typeOfElem == ELEM_BEAM) || (operand.typeOfElem == ELEM_COLUMN))
			precondition = true;

	// 전제 조건을 만족하지 않으면 실패
	if (precondition == false)
		return result;

	// 북쪽 측면에 물량합판을 붙일 경우
	if (src->validNorth == true) {
		//선행조건
		//	(1) src 측면의 X값 범위 안에 operand의 X값 범위가 침범하는가?
		//	(2) src 측면의 Y값이 operand의 Y값 범위 안에 들어가는가?
		//	(3) src 측면의 Z값 범위 안에 operand의 Z값 범위가 침범하는가?
		//선행 조건이 모두 참이면,
		//	(1) src 측면의 X값 범위에서 operand의 X값 범위를 제외한다.
		//	(2) src 측면의 Z값 범위에서 operand의 Z값 범위를 제외한다.

		if (overlapRange (src->NorthRightTop.x, src->NorthLeftBottom.x, operand.bottomPoint.x, operand.topPoint.x) &&
			inRange (src->NorthLeftBottom.y, operand.bottomPoint.y, operand.topPoint.y) &&
			overlapRange (src->NorthLeftBottom.z, src->NorthRightTop.z, operand.bottomPoint.z, operand.topPoint.z)) {

				excludeRange (&src->NorthLeftBottom, &src->NorthRightTop, NORTH_SIDE, MODE_X, operand.NorthRightTop.x, operand.NorthLeftBottom.x);
				excludeRange (&src->NorthLeftBottom, &src->NorthRightTop, NORTH_SIDE, MODE_Z, operand.NorthLeftBottom.z, operand.NorthRightTop.z);
		}

		result = true;
	}

	// 남쪽 측면에 물량합판을 붙일 경우
	if (src->validSouth == true) {
		//선행조건
		//	(1) src 측면의 X값 범위 안에 operand의 X값 범위가 침범하는가?
		//	(2) src 측면의 Y값이 operand의 Y값 범위 안에 들어가는가?
		//	(3) src 측면의 Z값 범위 안에 operand의 Z값 범위가 침범하는가?
		//선행 조건이 모두 참이면,
		//	(1) src 측면의 X값 범위에서 operand의 X값 범위를 제외한다.
		//	(2) src 측면의 Z값 범위에서 operand의 Z값 범위를 제외한다.

		if (overlapRange (src->SouthLeftBottom.x, src->SouthRightTop.x, operand.bottomPoint.x, operand.topPoint.x) &&
			inRange (src->SouthLeftBottom.y, operand.bottomPoint.y, operand.topPoint.y) &&
			overlapRange (src->SouthLeftBottom.z, src->SouthRightTop.z, operand.bottomPoint.z, operand.topPoint.z)) {

				excludeRange (&src->SouthLeftBottom, &src->SouthRightTop, SOUTH_SIDE, MODE_X, operand.SouthLeftBottom.x, operand.SouthRightTop.x);
				excludeRange (&src->SouthLeftBottom, &src->SouthRightTop, SOUTH_SIDE, MODE_Z, operand.SouthLeftBottom.z, operand.SouthRightTop.z);
		}

		result = true;
	}

	// 동쪽 측면에 물량합판을 붙일 경우
	if (src->validEast == true) {
		//선행조건
		//	(1) src 측면의 X값이 operand의 X값 범위 안에 들어가는가?
		//	(2) src 측면의 Y값 범위 안에 operand의 Y값 범위가 침범하는가?
		//	(3) src 측면의 Z값 범위 안에 operand의 Z값 범위가 침범하는가?
		//선행 조건이 모두 참이면,
		//	(1) src 측면의 Y값 범위에서 operand의 Y값 범위를 제외한다.
		//	(2) src 측면의 Z값 범위에서 operand의 Z값 범위를 제외한다.

		if (inRange (src->EastLeftBottom.x, operand.bottomPoint.x, operand.topPoint.x) &&
			overlapRange (src->EastLeftBottom.y, src->EastRightTop.y, operand.bottomPoint.y, operand.topPoint.y) &&
			overlapRange (src->EastLeftBottom.z, src->EastRightTop.z, operand.bottomPoint.z, operand.topPoint.z)) {

				excludeRange (&src->EastLeftBottom, &src->EastRightTop, EAST_SIDE, MODE_X, operand.EastLeftBottom.x, operand.EastRightTop.x);
				excludeRange (&src->EastLeftBottom, &src->EastRightTop, EAST_SIDE, MODE_Z, operand.EastLeftBottom.z, operand.EastRightTop.z);
		}

		result = true;
	}

	// 서쪽 측면에 물량합판을 붙일 경우
	if (src->validWest == true) {
		//선행조건
		//	(1) src 측면의 X값이 operand의 X값 범위 안에 들어가는가?
		//	(2) src 측면의 Y값 범위 안에 operand의 Y값 범위가 침범하는가?
		//	(3) src 측면의 Z값 범위 안에 operand의 Z값 범위가 침범하는가?
		//선행 조건이 모두 참이면,
		//	(1) src 측면의 Y값 범위에서 operand의 Y값 범위를 제외한다.
		//	(2) src 측면의 Z값 범위에서 operand의 Z값 범위를 제외한다.

		if (inRange (src->WestLeftBottom.x, operand.bottomPoint.x, operand.topPoint.x) &&
			overlapRange (src->WestRightTop.y, src->WestLeftBottom.y, operand.bottomPoint.y, operand.topPoint.y) &&
			overlapRange (src->WestLeftBottom.z, src->WestRightTop.z, operand.bottomPoint.z, operand.topPoint.z)) {

				excludeRange (&src->WestLeftBottom, &src->WestRightTop, WEST_SIDE, MODE_X, operand.WestRightTop.x, operand.WestLeftBottom.x);
				excludeRange (&src->WestLeftBottom, &src->WestRightTop, WEST_SIDE, MODE_Z, operand.WestLeftBottom.z, operand.WestRightTop.z);
		}

		result = true;
	}

	// 밑면에 물량합판을 붙일 경우
	if (src->validBase == true) {
		//선행조건
		//	(1) src 밑면의 X값 범위 안에 operand의 X값 범위가 침범하는가?
		//	(2) src 밑면의 Y값 범위 안에 operand의 Y값 범위가 침범하는가?
		//	(3) src 밑면의 Z값이 operand의 Z값 범위 안에 들어가는가?
		//선행 조건이 모두 참이면,
		//	(1) src 밑면의 X값 범위에서 operand의 X값 범위를 제외한다.
		//	(2) src 밑면의 Y값 범위에서 operand의 Y값 범위를 제외한다.

		if (overlapRange (src->BaseLeftBottom.x, src->BaseRightTop.x, operand.bottomPoint.x, operand.topPoint.x) &&
			overlapRange (src->BaseLeftBottom.y, src->BaseRightTop.y, operand.bottomPoint.y, operand.topPoint.y) &&
			inRange (src->BaseLeftBottom.z, operand.bottomPoint.z, operand.topPoint.z)) {

				excludeRange (&src->BaseLeftBottom, &src->BaseRightTop, BASE_SIDE, MODE_X, operand.BaseLeftBottom.x, operand.BaseRightTop.x);
				excludeRange (&src->BaseLeftBottom, &src->BaseRightTop, BASE_SIDE, MODE_Z, operand.BaseLeftBottom.z, operand.BaseRightTop.z);
		}

		result = true;
	}

	return	result;
}

// srcPoint 값이 target 범위 안에 들어 있는가?
bool	inRange (double srcPoint, double targetMin, double targetMax)
{
	if ((srcPoint > targetMin - EPS) && (srcPoint < targetMax + EPS))
		return true;
	else
		return false;
}

// src 범위와 target 범위가 겹치는가?
bool	overlapRange (double srcMin, double srcMax, double targetMin, double targetMax)
{
	bool	result = false;

	// srcMin과 srcMax 중 하나라도 target 범위 안에 들어가는 경우
	if (inRange (srcMin, targetMin, targetMax) || inRange (srcMax, targetMin, targetMax))
		result = true;
	// srcMin이 targetMin보다 작고 srcMax가 targetMax보다 큰 경우
	if ((srcMin < targetMin + EPS) && (srcMax > targetMax - EPS))
		result = true;
	// targetMin이 srcMin보다 작고 targetMax가 srcMax보다 큰 경우
	if ((targetMin < srcMin + EPS) && (targetMax > srcMax - EPS))
		result = true;

	return result;
}

// 측면 또는 밑면의 범위를 축소함, side는 적용할 면 선택 (북쪽(0), 남쪽(1), 동쪽(2), 서쪽(3), 밑면(5)), mode에 따라 X(0), Y(1), Z(2)에 적용함, minVal ~ maxVal에 의해 범위가 깎임
bool	excludeRange (API_Coord3D* srcLeftBottom, API_Coord3D* srcRightTop, short side, short mode, double minVal, double maxVal)
{
	bool	result = false;

	// 최소값, 최대값이 반대로 되어 있으면 오류
	if (minVal > maxVal)
		return false;

	// 북쪽 측면
	if (side == NORTH_SIDE) {
		if (mode == MODE_X) {
			// srcRightTop.x ~ srcLeftBottom.x 범위 안
				// minVal과 maxVal이 모두 들어 있는 경우 -> 범위 모두 깎음
				// minVal이 들어 있으나 maxVal은 srcLeftBottom.x - EPS보다 큰 경우 -> srcLeftBottom.x = minVal
				// maxVal이 들어 있으나 minVal은 srcRightTop.x + EPS보다 작은 경우 -> srcRightTop.x = maxVal
				// minVal이 srcRightTop.x + EPS보다 작고 maxVal이 srcLeftBottom.x - EPS보다 큰 경우 -> 범위 모두 깎음

			if ((srcRightTop->x < minVal) && (minVal < srcLeftBottom->x) && (srcRightTop->x < maxVal) && (maxVal < srcLeftBottom->x)) {
				srcRightTop->x = srcLeftBottom->x;
				srcRightTop->z = srcLeftBottom->z;
			}
			if ((srcRightTop->x < minVal) && (minVal < srcLeftBottom->x) && (srcLeftBottom->x - EPS < maxVal)) {
				srcLeftBottom->x = minVal;
			}
			if ((srcRightTop->x < maxVal) && (maxVal < srcRightTop->x) && (minVal < srcRightTop->x + EPS)) {
				srcRightTop->x = maxVal;
			}
			if ((minVal < srcRightTop->x + EPS) && (srcLeftBottom->x - EPS < maxVal)) {
				srcRightTop->x = srcLeftBottom->x;
				srcRightTop->z = srcLeftBottom->z;
			}

			result = true;
		} else if (mode == MODE_Y) {
			// 무효
			result = false;
		} else if (mode == MODE_Z) {
			// srcLeftBottom.z ~ srcRightTop.z 범위 안
				// minVal과 maxVal이 모두 들어 있는 경우 -> 범위 모두 깎음
				// minVal이 들어 있으나 maxVal은 srcRightTop.z - EPS보다 큰 경우 -> srcRightTop.z = minVal
				// maxVal이 들어 있으나 minVal은 srcLeftBottom.z + EPS보다 작은 경우 -> srcLeftBottom.z = maxVal
				// minVal이 srcLeftBottom.z + EPS보다 작고 maxVal이 srcRightTop.z - EPS보다 큰 경우 -> 범위 모두 깎음

			if ((srcLeftBottom->z < minVal) && (minVal < srcRightTop->z) && (srcLeftBottom->z < maxVal) && (maxVal < srcRightTop->z)) {
				srcRightTop->x = srcLeftBottom->x;
				srcRightTop->z = srcLeftBottom->z;
			}
			if ((srcLeftBottom->z < minVal) && (minVal < srcRightTop->z) && (srcRightTop->z - EPS < maxVal)) {
				srcRightTop->z = minVal;
			}
			if ((srcLeftBottom->z < maxVal) && (maxVal < srcRightTop->z) && (minVal < srcLeftBottom->z + EPS)) {
				srcLeftBottom->z = maxVal;
			}
			if ((minVal < srcLeftBottom->z + EPS) && (srcRightTop->z - EPS < maxVal)) {
				srcRightTop->x = srcLeftBottom->x;
				srcRightTop->z = srcLeftBottom->z;
			}

			result = true;
		}

	// 남쪽 측면
	} else if (side == SOUTH_SIDE) {
		if (mode == MODE_X) {
			// srcLeftBottom.x ~ srcRightTop.x 범위 안
				// minVal과 maxVal이 모두 들어 있는 경우 -> 범위 모두 깎음
				// minVal이 들어 있으나 maxVal은 srcRightTop.x - EPS보다 큰 경우 -> srcRightTop.x = minVal
				// maxVal이 들어 있으나 minVal은 srcLeftBottom.x + EPS보다 작은 경우 -> srcLeftBottom.x = maxVal
				// minVal이 srcLeftBottom.x + EPS보다 작고 maxVal이 srcRightTop.x - EPS보다 큰 경우 -> 범위 모두 깎음

			if ((srcLeftBottom->x < minVal) && (minVal < srcRightTop->x) && (srcLeftBottom->x < maxVal) && (maxVal < srcRightTop->x)) {
				srcRightTop->x = srcLeftBottom->x;
				srcRightTop->z = srcLeftBottom->z;
			}
			if ((srcRightTop->x < minVal) && (minVal < srcLeftBottom->x) && (srcLeftBottom->x - EPS < maxVal)) {
				srcLeftBottom->x = minVal;
			}
			if ((srcRightTop->x < maxVal) && (maxVal < srcRightTop->x) && (minVal < srcRightTop->x + EPS)) {
				srcRightTop->x = maxVal;
			}
			if ((minVal < srcRightTop->x + EPS) && (srcLeftBottom->x - EPS < maxVal)) {
				srcRightTop->x = srcLeftBottom->x;
				srcRightTop->z = srcLeftBottom->z;
			}

			result = true;
		} else if (mode == MODE_Y) {
			// 무효
			result = false;
		} else if (mode == MODE_Z) {
			// srcLeftBottom.z ~ srcRightTop.z 범위 안
				// minVal과 maxVal이 모두 들어 있는 경우 -> 범위 모두 깎음
				// minVal이 들어 있으나 maxVal은 srcRightTop.z - EPS보다 큰 경우 -> srcRightTop.z = minVal
				// maxVal이 들어 있으나 minVal은 srcLeftBottom.z + EPS보다 작은 경우 -> srcLeftBottom.z = maxVal
				// minVal이 srcLeftBottom.z + EPS보다 작고 maxVal이 srcRightTop.z - EPS보다 큰 경우 -> 범위 모두 깎음

			if ((srcLeftBottom->z < minVal) && (minVal < srcRightTop->z) && (srcLeftBottom->z < maxVal) && (maxVal < srcRightTop->z)) {
				srcRightTop->x = srcLeftBottom->x;
				srcRightTop->z = srcLeftBottom->z;
			}
			if ((srcLeftBottom->z < minVal) && (minVal < srcRightTop->z) && (srcRightTop->z - EPS < maxVal)) {
				srcRightTop->z = minVal;
			}
			if ((srcLeftBottom->z < maxVal) && (maxVal < srcRightTop->z) && (minVal < srcLeftBottom->z + EPS)) {
				srcLeftBottom->z = maxVal;
			}
			if ((minVal < srcLeftBottom->z + EPS) && (srcRightTop->z - EPS < maxVal)) {
				srcRightTop->x = srcLeftBottom->x;
				srcRightTop->z = srcLeftBottom->z;
			}

			result = true;
		}

	// 동쪽 측면
	} else if (side == EAST_SIDE) {
		if (mode == MODE_X) {
			// 무효
			result = false;
		} else if (mode == MODE_Y) {
			// srcLeftBottom.y ~ srcRightTop.y 범위 안
				// minVal과 maxVal이 모두 들어 있는 경우 -> 범위 모두 깎음
				// minVal이 들어 있으나 maxVal은 srcRightTop.y - EPS보다 큰 경우 -> srcRightTop.y = minVal
				// maxVal이 들어 있으나 minVal은 srcLeftBottom.y + EPS보다 작은 경우 -> srcLeftBottom.y = maxVal
				// minVal이 srcLeftBottom.y + EPS보다 작고 maxVal이 srcRightTop.y - EPS보다 큰 경우 -> 범위 모두 깎음

			if ((srcLeftBottom->y < minVal) && (minVal < srcRightTop->y) && (srcLeftBottom->y < maxVal) && (maxVal < srcRightTop->y)) {
				srcRightTop->y = srcLeftBottom->y;
				srcRightTop->z = srcLeftBottom->z;
			}
			if ((srcRightTop->y < minVal) && (minVal < srcLeftBottom->y) && (srcLeftBottom->y - EPS < maxVal)) {
				srcLeftBottom->y = minVal;
			}
			if ((srcRightTop->y < maxVal) && (maxVal < srcRightTop->y) && (minVal < srcRightTop->y + EPS)) {
				srcRightTop->y = maxVal;
			}
			if ((minVal < srcRightTop->y + EPS) && (srcLeftBottom->y - EPS < maxVal)) {
				srcRightTop->y = srcLeftBottom->y;
				srcRightTop->z = srcLeftBottom->z;
			}

			result = true;
		} else if (mode == MODE_Z) {
			// srcLeftBottom.z ~ srcRightTop.z 범위 안
				// minVal과 maxVal이 모두 들어 있는 경우 -> 범위 모두 깎음
				// minVal이 들어 있으나 maxVal은 srcRightTop.z - EPS보다 큰 경우 -> srcRightTop.z = minVal
				// maxVal이 들어 있으나 minVal은 srcLeftBottom.z + EPS보다 작은 경우 -> srcLeftBottom.z = maxVal
				// minVal이 srcLeftBottom.z + EPS보다 작고 maxVal이 srcRightTop.z - EPS보다 큰 경우 -> 범위 모두 깎음

			if ((srcLeftBottom->z < minVal) && (minVal < srcRightTop->z) && (srcLeftBottom->z < maxVal) && (maxVal < srcRightTop->z)) {
				srcRightTop->x = srcLeftBottom->x;
				srcRightTop->z = srcLeftBottom->z;
			}
			if ((srcLeftBottom->z < minVal) && (minVal < srcRightTop->z) && (srcRightTop->z - EPS < maxVal)) {
				srcRightTop->z = minVal;
			}
			if ((srcLeftBottom->z < maxVal) && (maxVal < srcRightTop->z) && (minVal < srcLeftBottom->z + EPS)) {
				srcLeftBottom->z = maxVal;
			}
			if ((minVal < srcLeftBottom->z + EPS) && (srcRightTop->z - EPS < maxVal)) {
				srcRightTop->x = srcLeftBottom->x;
				srcRightTop->z = srcLeftBottom->z;
			}

			result = true;
		}
	
	// 서쪽 측면
	} else if (side == WEST_SIDE) {
		if (mode == MODE_X) {
			// 무효
			result = false;
		} else if (mode == MODE_Y) {
			// srcRightTop.y ~ srcLeftBottom.y 범위 안
				// minVal과 maxVal이 모두 들어 있는 경우 -> 범위 모두 깎음
				// minVal이 들어 있으나 maxVal은 srcLeftBottom.y - EPS보다 큰 경우 -> srcLeftBottom.y = minVal
				// maxVal이 들어 있으나 minVal은 srcRightTop.y + EPS보다 작은 경우 -> srcRightTop.y = maxVal
				// minVal이 srcRightTop.y + EPS보다 작고 maxVal이 srcLeftBottom.y - EPS보다 큰 경우 -> 범위 모두 깎음

			if ((srcRightTop->y < minVal) && (minVal < srcLeftBottom->y) && (srcRightTop->y < maxVal) && (maxVal < srcLeftBottom->y)) {
				srcRightTop->y = srcLeftBottom->y;
				srcRightTop->z = srcLeftBottom->z;
			}
			if ((srcRightTop->y < minVal) && (minVal < srcLeftBottom->y) && (srcLeftBottom->y - EPS < maxVal)) {
				srcLeftBottom->y = minVal;
			}
			if ((srcRightTop->y < maxVal) && (maxVal < srcRightTop->y) && (minVal < srcRightTop->y + EPS)) {
				srcRightTop->y = maxVal;
			}
			if ((minVal < srcRightTop->y + EPS) && (srcLeftBottom->y - EPS < maxVal)) {
				srcRightTop->y = srcLeftBottom->y;
				srcRightTop->z = srcLeftBottom->z;
			}

			result = true;
		} else if (mode == MODE_Z) {
			// srcLeftBottom.z ~ srcRightTop.z 범위 안
				// minVal과 maxVal이 모두 들어 있는 경우 -> 범위 모두 깎음
				// minVal이 들어 있으나 maxVal은 srcRightTop.z - EPS보다 큰 경우 -> srcRightTop.z = minVal
				// maxVal이 들어 있으나 minVal은 srcLeftBottom.z + EPS보다 작은 경우 -> srcLeftBottom.z = maxVal
				// minVal이 srcLeftBottom.z + EPS보다 작고 maxVal이 srcRightTop.z - EPS보다 큰 경우 -> 범위 모두 깎음

			if ((srcLeftBottom->z < minVal) && (minVal < srcRightTop->z) && (srcLeftBottom->z < maxVal) && (maxVal < srcRightTop->z)) {
				srcRightTop->x = srcLeftBottom->x;
				srcRightTop->z = srcLeftBottom->z;
			}
			if ((srcLeftBottom->z < minVal) && (minVal < srcRightTop->z) && (srcRightTop->z - EPS < maxVal)) {
				srcRightTop->z = minVal;
			}
			if ((srcLeftBottom->z < maxVal) && (maxVal < srcRightTop->z) && (minVal < srcLeftBottom->z + EPS)) {
				srcLeftBottom->z = maxVal;
			}
			if ((minVal < srcLeftBottom->z + EPS) && (srcRightTop->z - EPS < maxVal)) {
				srcRightTop->x = srcLeftBottom->x;
				srcRightTop->z = srcLeftBottom->z;
			}

			result = true;
		}
	
	// 밑면
	} else if (side == BASE_SIDE) {
		if (mode == MODE_X) {
			// srcLeftBottom.x ~ srcRightTop.x 범위 안
				// minVal과 maxVal이 모두 들어 있는 경우 -> 범위 모두 깎음
				// minVal이 들어 있으나 maxVal은 srcRightTop.x - EPS보다 큰 경우 -> srcRightTop.x = minVal
				// maxVal이 들어 있으나 minVal은 srcLeftBottom.x + EPS보다 작은 경우 -> srcLeftBottom.x = maxVal
				// minVal이 srcLeftBottom.x + EPS보다 작고 maxVal이 srcRightTop.x - EPS보다 큰 경우 -> 범위 모두 깎음

			if ((srcLeftBottom->x < minVal) && (minVal < srcRightTop->x) && (srcLeftBottom->x < maxVal) && (maxVal < srcRightTop->x)) {
				srcRightTop->x = srcLeftBottom->x;
				srcRightTop->z = srcLeftBottom->z;
			}
			if ((srcRightTop->x < minVal) && (minVal < srcLeftBottom->x) && (srcLeftBottom->x - EPS < maxVal)) {
				srcLeftBottom->x = minVal;
			}
			if ((srcRightTop->x < maxVal) && (maxVal < srcRightTop->x) && (minVal < srcRightTop->x + EPS)) {
				srcRightTop->x = maxVal;
			}
			if ((minVal < srcRightTop->x + EPS) && (srcLeftBottom->x - EPS < maxVal)) {
				srcRightTop->x = srcLeftBottom->x;
				srcRightTop->z = srcLeftBottom->z;
			}

			result = true;
		} else if (mode == MODE_Y) {
			// srcLeftBottom.y ~ srcRightTop.y 범위 안
				// minVal과 maxVal이 모두 들어 있는 경우 -> 범위 모두 깎음
				// minVal이 들어 있으나 maxVal은 srcRightTop.y - EPS보다 큰 경우 -> srcRightTop.y = minVal
				// maxVal이 들어 있으나 minVal은 srcLeftBottom.y + EPS보다 작은 경우 -> srcLeftBottom.y = maxVal
				// minVal이 srcLeftBottom.y + EPS보다 작고 maxVal이 srcRightTop.y - EPS보다 큰 경우 -> 범위 모두 깎음

			if ((srcLeftBottom->y < minVal) && (minVal < srcRightTop->y) && (srcLeftBottom->y < maxVal) && (maxVal < srcRightTop->y)) {
				srcRightTop->y = srcLeftBottom->y;
				srcRightTop->z = srcLeftBottom->z;
			}
			if ((srcRightTop->y < minVal) && (minVal < srcLeftBottom->y) && (srcLeftBottom->y - EPS < maxVal)) {
				srcLeftBottom->y = minVal;
			}
			if ((srcRightTop->y < maxVal) && (maxVal < srcRightTop->y) && (minVal < srcRightTop->y + EPS)) {
				srcRightTop->y = maxVal;
			}
			if ((minVal < srcRightTop->y + EPS) && (srcLeftBottom->y - EPS < maxVal)) {
				srcRightTop->y = srcLeftBottom->y;
				srcRightTop->z = srcLeftBottom->z;
			}

			result = true;
		} else if (mode == MODE_Z) {
			// 무효
			result = false;
		}
	}

	return result;
}