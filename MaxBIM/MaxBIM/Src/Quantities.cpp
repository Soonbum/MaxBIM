#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "Quantities.hpp"

using namespace quantitiesDG;

short				nLayers;		// 레이어 개수
GS::Array<short>	layerInds;		// 보이는 레이어 인덱스
GS::Array<string>	layerNames;		// 보이는 레이어 이름

short				selectedCombobox;	// 선택한 콤보박스 개수

// 부재(벽,슬래브,보,기둥)들의 가설재가 붙을 수 있는 면에 물량합판을 자동으로 부착함
GSErrCode	placeQuantityPlywood (void)
{
	GSErrCode	err = NoError;

	API_Element				elem;
	API_ElementMemo			memo;
	API_ElemInfo3D			info3D;

	short	xx, yy;
	long	nElems;		// 요소 개수
	qElem*	elems;		// 요소 배열
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
	nLayers = layerInds.GetSize ();

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

		// memo 객체 해제
		ACAPI_DisposeElemMemoHdls (&memo);
	}

	// [다이얼로그] 물량합판 레이어 설정
	result = DGBlankModalDialog (600, 100, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, quantityPlywoodUIHandler, 0);

	//for (xx = 0 ; xx < nElems ; ++xx) {
	//	for (yy = 0 ; yy < nElems ; ++yy) {
	// 단, 자기 자신과의 비교는 하지 않는다. (만약 xx번째 guid와 yy번째 guid가 동일하면 continue)

	// *** (중요) 각각의 요소는 측면 또는 밑면에 대한 X, Y, Z의 값 범위 정보를 갖고 있어야 함

	// 알고리즘
	// 1. xx가 벽일 경우
		// yy가 벽일 경우
			// 측면 1
			// 측면 2
				// yy에 의해 xx의 X-Y 범위가 깎임
		// yy가 슬래브일 경우
			// 측면 1
			// 측면 2
				// yy에 의해 xx의 Z 범위가 깎임
		// yy가 보일 경우
			// 측면 1
			// 측면 2
				// yy에 의해 xx의 Z 범위가 깎임
		// yy가 기둥일 경우
			// 측면 1
			// 측면 2
				// yy에 의해 xx의 X-Y 범위가 깎임
	// 2. xx가 슬래브일 경우
		// yy가 벽일 경우
			// 밑면
				// yy에 의해 xx의 X-Y 범위가 깎임
		// yy가 슬래브일 경우
			// 밑면
				// yy에 의해 xx의 X-Y 범위가 깎임
		// yy가 보일 경우
			// 밑면
				// yy에 의해 xx의 X-Y 범위가 깎임
		// yy가 기둥일 경우
			// 밑면
				// yy에 의해 xx의 X-Y 범위가 깎임
	// 3. xx가 보일 경우
		// yy가 벽일 경우
			// 밑면
				// yy에 의해 xx의 X-Y 범위가 깎임
			// 측면 1
			// 측면 2
				// yy에 의해 xx의 Z 범위가 깎임
		// yy가 슬래브일 경우
			// 밑면
				// yy에 의해 xx의 X-Y 범위가 깎임
			// 측면 1
			// 측면 2
				// yy에 의해 xx의 Z 범위가 깎임
		// yy가 보일 경우
			// 밑면
				// yy에 의해 xx의 X-Y 범위가 깎임
			// 측면 1
			// 측면 2
				// yy에 의해 xx의 Z 범위가 깎임
		// yy가 기둥일 경우
			// 밑면
				// yy에 의해 xx의 X-Y 범위가 깎임
			// 측면 1
			// 측면 2
				// yy에 의해 xx의 Z 범위가 깎임
	// 4. xx가 기둥일 경우
		// yy가 벽일 경우
			// 측면 1
			// 측면 2
			// 측면 3
			// 측면 4
				// yy에 의해 xx의 Z 범위가 깎임
		// yy가 슬래브일 경우
			// 측면 1
			// 측면 2
			// 측면 3
			// 측면 4
				// yy에 의해 xx의 Z 범위가 깎임
		// yy가 보일 경우
			// 측면 1
			// 측면 2
			// 측면 3
			// 측면 4
				// yy에 의해 xx의 Z 범위가 깎임
		// yy가 기둥일 경우
			// 측면 1
			// 측면 2
			// 측면 3
			// 측면 4
				// yy에 의해 xx의 Z 범위가 깎임

	// 화면 새로고침
	//ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
	//bool	regenerate = true;
	//ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

	delete []	elems;

	return	err;
}

// [다이얼로그] 물량산출 합판 레이어 지정
short DGCALLBACK quantityPlywoodUIHandler (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	short	itmIdx;
	short	itmPosX, itmPosY;
	short	xx;
	//char	tempStr [20];
	short	dialogSizeX, dialogSizeY;

	GSErrCode err = NoError;
	//API_ModulData  info;

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
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 120, 25, 150, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "부재 레이어");
			DGShowItem (dialogID, itmIdx);

			// 라벨: 물량합판 레이어
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 280, 25, 150, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "물량합판 레이어");
			DGShowItem (dialogID, itmIdx);

			itmPosX = 120;
			itmPosY = 50;

			// 기존 레이어: 처음에는 1행만 표시할 것
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 500, 25, 120, itmPosY, 150, 25);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, "없음");
			for (xx = 0 ; xx < nLayers ; ++xx) {
				DGPopUpInsertItem (dialogID, itmIdx, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, itmIdx, DG_POPUP_BOTTOM, layerNames.Get (xx).c_str ());
			}
			DGShowItem (dialogID, itmIdx);

			break;
		
		case DG_MSG_CHANGE:

			// ...
			// selectedCombobox : 선택한 콤보박스 개수?

			// itmPosX, itmPosY
			// dialogSizeX, dialogSizeY
			// DGSetDialogSize (dialogID, DG_FRAME, dialogSizeX, dialogSizeY, DG_TOPLEFT, true);

			// 레이어 개수만큼 for문 반복
			// 1열 (레이어 이름)		2열 (물량합판 레이어)
									//	팝업: 기존 레이어 / EditControl: 레이어 이름

			// UI (대상 레이어 - 물량합판 레이어는 1:1로 매칭시켜야 함)
			// 레이어 선택 방식은 기존 애드온 - 레이어 기능 참조
			// *** 대상 레이어는 현재 보이는 레이어를 다 읽어 들인 후 취사 선택할 것
				// 기존 레이어		물량합판 레이어				물량합판 타입
				//					없음 | 기존 | 커스텀 입력	벽, 슬래브, 보, 기둥

			// 아래는 예전 내용 (유효하지 않음)
			// 대상 레이어 선택(다중)	-->	물량합판 레이어 선택(다중)		(물량합판 부착면)
				// 벽 (내벽)					벽 (내벽)						측면(장)
				// 벽 (외벽)					벽 (외벽)						...
				// 벽 (합벽)					벽 (합벽)						...
				// 벽 (파라펫)					벽 (파라펫)						...
				// 벽 (방수턱)					벽 (방수턱)						...
				// 슬래브 (기초)				슬래브 (기초)					하부
				// 슬래브 (RC)					슬래브 (RC)						...
				// 슬래브 (데크)				슬래브 (데크)					...
				// 슬래브 (램프)				슬래브 (램프)					...
				// 보							보								측면(장), 하부
				// 기둥 (독립)					기둥 (독립)						측면(전)
				// 기둥 (벽체)					기둥 (벽체)						...
				// 기둥 (원형)					기둥 (원형)						...

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:

					// elems의 layerInd와 매칭하는 qLayerInd 저장할 것

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
