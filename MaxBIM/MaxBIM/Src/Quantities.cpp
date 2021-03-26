#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "Quantities.hpp"

using namespace quantitiesDG;

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
	
	GS::Array<API_Guid>	elemList_All;		// 요소들의 GUID (전체 구조 요소)
	GS::Array<API_Guid>	elemList_Wall;		// 요소들의 GUID (벽)
	GS::Array<API_Guid>	elemList_Slab;		// 요소들의 GUID (슬래브)
	GS::Array<API_Guid>	elemList_Beam;		// 요소들의 GUID (보)
	GS::Array<API_Guid>	elemList_Column;	// 요소들의 GUID (기둥)


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

	// ... (진행중)
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

	delete []	elems;

	return	err;
}

// [다이얼로그] 물량산출 합판 레이어 지정
short DGCALLBACK quantityPlywoodUIHandler (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	short	itmIdx;
	//short	itmPosX, itmPosY;
	//short	xx, yy;
	//char	tempStr [20];
	//short	dialogSizeX, dialogSizeY;

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
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 120, 25, 90, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "부재 레이어");
			DGShowItem (dialogID, itmIdx);

			// 라벨: 물량합판 레이어
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 280, 25, 90, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "물량합판 레이어");
			DGShowItem (dialogID, itmIdx);

			// 라벨: 물량합판 타입
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_CENTER, DG_FT_NONE, 450, 25, 90, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "물량합판 타입");
			DGShowItem (dialogID, itmIdx);

			// ... 보이는 레이어를 불러올 것

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
			
			/*
			// 레이어 쉽게 선택하기
			GSErrCode	showLayersEasily (void)
			{
				API_Attribute	attrib;
				short			nLayers;

				// 구조체 초기화
				allocateMemory (&layerInfo);
				selectedInfo = layerInfo;	// selectedInfo에는 vector가 비어 있으므로 초기화를 위해 복사해 둠
				allocateMemory (&selectedInfo);

				// 프로젝트 내 레이어 개수를 알아냄
				BNZeroMemory (&attrib, sizeof (API_Attribute));
				attrib.header.typeID = API_LayerID;
				err = ACAPI_Attribute_GetNum (API_LayerID, &nLayers);

				for (xx = 1; xx <= nLayers && err == NoError ; ++xx) {
					attrib.header.index = xx;
					err = ACAPI_Attribute_Get (&attrib);
				}

				// OK 버튼이 아니면 메모리 해제하고 종료
				if (result != DG_OK) {
					deallocateMemory (&layerInfo);
					deallocateMemory (&selectedInfo);
					return	err;
				}

				// 모든 레이어 숨기기
				BNZeroMemory (&attrib, sizeof (API_Attribute));
				attrib.header.typeID = API_LayerID;
	
				for (xx = 1; xx <= nLayers ; ++xx) {
					attrib.header.index = xx;
					err = ACAPI_Attribute_Get (&attrib);
					if (err == NoError) {
						//if ((attrib.layer.head.flags & APILay_Hidden) == false) {
							attrib.layer.head.flags |= APILay_Hidden;
							ACAPI_Attribute_Modify (&attrib, NULL);
						//}
					}
				}

												// 조합한 레이어 이름 검색하기
												BNZeroMemory (&attrib, sizeof (API_Attribute));
												attrib.header.typeID = API_LayerID;
												CHCopyC (fullLayerName, attrib.header.name);
												err = ACAPI_Attribute_Get (&attrib);

												// 해당 레이어 보여주기
												if ((attrib.layer.head.flags & APILay_Hidden) == true) {
													attrib.layer.head.flags ^= APILay_Hidden;
													ACAPI_Attribute_Modify (&attrib, NULL);
												}
											}
										}
									}
								}
							}
						}
					}
				}
	
				// 화면 새로고침
				ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
				bool	regenerate = true;
				ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

				// 메모리 해제
				deallocateMemory (&layerInfo);
				deallocateMemory (&selectedInfo);

				return err;
			}
			*/

			//// 라벨: 코드 보여주기
			//itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 0, 120, 50, 23);
			//DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, itmIdx, "CODE");
			//DGShowItem (dialogID, itmIdx);
			//LABEL_CODE = itmIdx;

			//// 라벨: 공사 구분
			//itmPosX = 40;
			//itmPosY = 25;
			//itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 85, 23);
			//DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, itmIdx, "공사 구분");
			//DGShowItem (dialogID, itmIdx);

			//// 체크박스: 공사 구분 버튼
			//itmPosX = 150;
			//itmPosY = 20;
			//for (xx = 0 ; xx < layerInfo.code_name.size () ; ++xx) {
			//	if (layerInfo.code_state [xx] == true) {
			//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
			//		DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
			//		sprintf (tempStr, "%s %s", layerInfo.code_name [xx].c_str (), layerInfo.code_desc [xx].c_str ());
			//		DGSetItemText (dialogID, itmIdx, tempStr);
			//		DGShowItem (dialogID, itmIdx);
			//		layerInfo.code_idx [xx] = itmIdx;

			//		itmPosX += 100;
			//		if (itmPosX >= 600) {
			//			itmPosX = 150;
			//			itmPosY += 30;
			//		}
			//	}
			//}
			//// 모두 선택
			//itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
			//DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_BOLD);
			//DGSetItemText (dialogID, itmIdx, "모두 선택");
			//DGShowItem (dialogID, itmIdx);
			//SELECTALL_1_CONTYPE = itmIdx;
			//itmPosX += 100;
			//if (itmPosX >= 600) {
			//	itmPosX = 150;
			//	itmPosY += 30;
			//}

			//itmPosY += 30;

			//// 라벨: 동 구분
			//itmPosX = 40;
			//itmPosY += 10;
			//itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 85, 23);
			//DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, itmIdx, "동 구분");
			//DGShowItem (dialogID, itmIdx);

			//// 체크박스: 동 구분 버튼
			//itmPosX = 150;
			//itmPosY -= 5;
			//for (xx = 0 ; xx < layerInfo.dong_name.size () ; ++xx) {
			//	if (layerInfo.dong_state [xx] == true) {
			//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
			//		DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
			//		DGSetItemText (dialogID, itmIdx, layerInfo.dong_desc [xx].c_str ());
			//		DGShowItem (dialogID, itmIdx);
			//		layerInfo.dong_idx [xx] = itmIdx;

			//		itmPosX += 100;
			//		if (itmPosX >= 600) {
			//			itmPosX = 150;
			//			itmPosY += 30;
			//		}
			//	}
			//}
			//// 모두 선택
			//if (layerInfo.bDongAllShow) {
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_BOLD);
			//	DGSetItemText (dialogID, itmIdx, "모두 선택");
			//	DGShowItem (dialogID, itmIdx);
			//	SELECTALL_2_DONG = itmIdx;
			//	itmPosX += 100;
			//	if (itmPosX >= 600) {
			//		itmPosX = 150;
			//		itmPosY += 30;
			//	}
			//}

			//itmPosY += 30;

			//// 라벨: 층 구분
			//itmPosX = 40;
			//itmPosY += 10;
			//itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 85, 23);
			//DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, itmIdx, "층 구분");
			//DGShowItem (dialogID, itmIdx);

			//// 체크박스: 층 구분 버튼
			//itmPosX = 150;
			//itmPosY -= 5;
			//for (xx = 0 ; xx < layerInfo.floor_name.size () ; ++xx) {
			//	if (layerInfo.floor_state [xx] == true) {
			//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
			//		DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
			//		DGSetItemText (dialogID, itmIdx, layerInfo.floor_desc [xx].c_str ());
			//		DGShowItem (dialogID, itmIdx);
			//		layerInfo.floor_idx [xx] = itmIdx;

			//		itmPosX += 100;
			//		if (itmPosX >= 600) {
			//			itmPosX = 150;
			//			itmPosY += 30;
			//		}
			//	}
			//}
			//// 모두 선택
			//itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
			//DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_BOLD);
			//DGSetItemText (dialogID, itmIdx, "모두 선택");
			//DGShowItem (dialogID, itmIdx);
			//SELECTALL_3_FLOOR = itmIdx;
			//itmPosX += 100;
			//if (itmPosX >= 600) {
			//	itmPosX = 150;
			//	itmPosY += 30;
			//}

			//itmPosY += 30;

			//// 라벨: CJ 구간
			//itmPosX = 40;
			//itmPosY += 10;
			//itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 85, 23);
			//DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, itmIdx, "CJ 구간");
			//DGShowItem (dialogID, itmIdx);

			//// 체크박스: CJ 구간 버튼
			//itmPosX = 150;
			//itmPosY -= 5;
			//for (xx = 0 ; xx < layerInfo.CJ_name.size () ; ++xx) {
			//	if (layerInfo.CJ_state [xx] == true) {
			//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
			//		DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
			//		DGSetItemText (dialogID, itmIdx, layerInfo.CJ_name [xx].c_str ());
			//		DGShowItem (dialogID, itmIdx);
			//		layerInfo.CJ_idx [xx] = itmIdx;

			//		itmPosX += 100;
			//		if (itmPosX >= 600) {
			//			itmPosX = 150;
			//			itmPosY += 30;
			//		}
			//	}
			//}
			//// 모두 선택
			//if (layerInfo.bCJAllShow) {
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_BOLD);
			//	DGSetItemText (dialogID, itmIdx, "모두 선택");
			//	DGShowItem (dialogID, itmIdx);
			//	SELECTALL_4_CJ = itmIdx;
			//	itmPosX += 100;
			//	if (itmPosX >= 600) {
			//		itmPosX = 150;
			//		itmPosY += 30;
			//	}
			//}

			//itmPosY += 30;

			//// 라벨: CJ 속 시공순서
			//itmPosX = 40;
			//itmPosY += 10;
			//itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 85, 23);
			//DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, itmIdx, "시공순서");
			//DGShowItem (dialogID, itmIdx);

			//// 체크박스: CJ 속 시공순서 버튼
			//itmPosX = 150;
			//itmPosY -= 5;
			//for (xx = 0 ; xx < layerInfo.orderInCJ_name.size () ; ++xx) {
			//	if (layerInfo.orderInCJ_state [xx] == true) {
			//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
			//		DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
			//		DGSetItemText (dialogID, itmIdx, layerInfo.orderInCJ_name [xx].c_str ());
			//		DGShowItem (dialogID, itmIdx);
			//		layerInfo.orderInCJ_idx [xx] = itmIdx;

			//		itmPosX += 100;
			//		if (itmPosX >= 600) {
			//			itmPosX = 150;
			//			itmPosY += 30;
			//		}
			//	}
			//}
			//// 모두 선택
			//if (layerInfo.bOrderInCJAllShow) {
			//	itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
			//	DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_BOLD);
			//	DGSetItemText (dialogID, itmIdx, "모두 선택");
			//	DGShowItem (dialogID, itmIdx);
			//	SELECTALL_5_ORDER = itmIdx;
			//	itmPosX += 100;
			//	if (itmPosX >= 600) {
			//		itmPosX = 150;
			//		itmPosY += 30;
			//	}
			//}

			//itmPosY += 30;

			//// 라벨: 부재(구조)
			//itmPosX = 40;
			//itmPosY += 10;
			//itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 85, 23);
			//DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, itmIdx, "*구조");
			//DGShowItem (dialogID, itmIdx);

			//// 체크박스: 부재(구조)
			//itmPosX = 150;
			//itmPosY -= 5;
			//for (xx = 0 ; xx < layerInfo.obj_name.size () ; ++xx) {
			//	if (strncmp (layerInfo.obj_cat [xx].c_str (), "01-S", 4) == 0) {
			//		if (layerInfo.obj_state [xx] == true) {
			//			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
			//			DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
			//			DGSetItemText (dialogID, itmIdx, layerInfo.obj_desc [xx].c_str ());
			//			DGShowItem (dialogID, itmIdx);
			//			layerInfo.obj_idx [xx] = itmIdx;

			//			itmPosX += 100;
			//			if (itmPosX >= 600) {
			//				itmPosX = 150;
			//				itmPosY += 30;
			//			}
			//		}
			//	}
			//}

			//itmPosY += 30;

			//// 라벨: 부재(건축마감)
			//itmPosX = 40;
			//itmPosY += 10;
			//itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 85, 23);
			//DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, itmIdx, "*건축마감");
			//DGShowItem (dialogID, itmIdx);

			//// 체크박스: 부재(건축마감)
			//itmPosX = 150;
			//itmPosY -= 5;
			//for (xx = 0 ; xx < layerInfo.obj_name.size () ; ++xx) {
			//	if (strncmp (layerInfo.obj_cat [xx].c_str (), "02-A", 4) == 0) {
			//		if (layerInfo.obj_state [xx] == true) {
			//			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
			//			DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
			//			DGSetItemText (dialogID, itmIdx, layerInfo.obj_desc [xx].c_str ());
			//			DGShowItem (dialogID, itmIdx);
			//			layerInfo.obj_idx [xx] = itmIdx;

			//			itmPosX += 100;
			//			if (itmPosX >= 600) {
			//				itmPosX = 150;
			//				itmPosY += 30;
			//			}
			//		}
			//	}
			//}

			//itmPosY += 30;

			//// 라벨: 부재(가설재)
			//itmPosX = 40;
			//itmPosY += 10;
			//itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 85, 23);
			//DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, itmIdx, "*가설재");
			//DGShowItem (dialogID, itmIdx);

			//// 체크박스: 부재(가설재)
			//itmPosX = 150;
			//itmPosY -= 5;
			//for (xx = 0 ; xx < layerInfo.obj_name.size () ; ++xx) {
			//	if (strncmp (layerInfo.obj_cat [xx].c_str (), "05-T", 4) == 0) {
			//		if (layerInfo.obj_state [xx] == true) {
			//			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
			//			DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
			//			DGSetItemText (dialogID, itmIdx, layerInfo.obj_desc [xx].c_str ());
			//			DGShowItem (dialogID, itmIdx);
			//			layerInfo.obj_idx [xx] = itmIdx;

			//			itmPosX += 100;
			//			if (itmPosX >= 600) {
			//				itmPosX = 150;
			//				itmPosY += 30;
			//			}
			//		}
			//	}
			//}

			//itmPosY += 30;

			//// 라벨: 부재(가시설)
			//itmPosX = 40;
			//itmPosY += 10;
			//itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 85, 23);
			//DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, itmIdx, "*가시설");
			//DGShowItem (dialogID, itmIdx);

			//// 체크박스: 부재(가시설)
			//itmPosX = 150;
			//itmPosY -= 5;
			//for (xx = 0 ; xx < layerInfo.obj_name.size () ; ++xx) {
			//	if (strncmp (layerInfo.obj_cat [xx].c_str (), "06-F", 4) == 0) {
			//		if (layerInfo.obj_state [xx] == true) {
			//			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
			//			DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
			//			DGSetItemText (dialogID, itmIdx, layerInfo.obj_desc [xx].c_str ());
			//			DGShowItem (dialogID, itmIdx);
			//			layerInfo.obj_idx [xx] = itmIdx;

			//			itmPosX += 100;
			//			if (itmPosX >= 600) {
			//				itmPosX = 150;
			//				itmPosY += 30;
			//			}
			//		}
			//	}
			//}

			//itmPosY += 30;

			//// 라벨: 객체(가설재)
			//itmPosX = 40;
			//itmPosY += 10;
			//itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 85, 23);
			//DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			//DGSetItemText (dialogID, itmIdx, "**가설재(세부)");
			//DGShowItem (dialogID, itmIdx);

			//// 체크박스: 객체(가설재)
			//itmPosX = 150;
			//itmPosY -= 5;
			//for (xx = 0 ; xx < layerInfo.subObj_name.size () ; ++xx) {
			//	for (yy = 0 ; yy < layerInfo.code_name.size () ; ++yy) {
			//		if ((strncmp (layerInfo.subObj_cat [xx].c_str (), "05-T", 4) == 0) && (strncmp (layerInfo.code_name [yy].c_str (), layerInfo.subObj_cat [xx].c_str (), 4) == 0) && (layerInfo.code_state [yy] == true)) {
			//			if (layerInfo.obj_state [xx] == true) {
			//				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
			//				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
			//				DGSetItemText (dialogID, itmIdx, layerInfo.subObj_desc [xx].c_str ());
			//				DGShowItem (dialogID, itmIdx);
			//				layerInfo.subObj_idx [xx] = itmIdx;

			//				itmPosX += 100;
			//				if (itmPosX >= 600) {
			//					itmPosX = 150;
			//					itmPosY += 30;
			//				}
			//			}
			//		}
			//	}
			//}

			//dialogSizeX = 700;
			//dialogSizeY = itmPosY + 150;
			//DGSetDialogSize (dialogID, DG_FRAME, dialogSizeX, dialogSizeY, DG_TOPLEFT, true);

			break;
		
		case DG_MSG_CHANGE:
			//changedBtnItemIdx = item;

			//if (changedBtnItemIdx == SELECTALL_1_CONTYPE) {
			//	if (DGGetItemValLong (dialogID, SELECTALL_1_CONTYPE) == TRUE) {
			//		// 모두 선택
			//		for (xx = 0 ; xx < layerInfo.code_name.size () ; ++xx)
			//			DGSetItemValLong (dialogID, layerInfo.code_idx [xx], TRUE);
			//	} else {
			//		// 모두 제외
			//		for (xx = 0 ; xx < layerInfo.code_name.size () ; ++xx)
			//			DGSetItemValLong (dialogID, layerInfo.code_idx [xx], FALSE);
			//	}
			//}
			//if (changedBtnItemIdx == SELECTALL_2_DONG) {
			//	if (DGGetItemValLong (dialogID, SELECTALL_2_DONG) == TRUE) {
			//		// 모두 선택
			//		for (xx = 0 ; xx < layerInfo.dong_name.size () ; ++xx)
			//			DGSetItemValLong (dialogID, layerInfo.dong_idx [xx], TRUE);
			//	} else {
			//		// 모두 제외
			//		for (xx = 0 ; xx < layerInfo.dong_name.size () ; ++xx)
			//			DGSetItemValLong (dialogID, layerInfo.dong_idx [xx], FALSE);
			//	}
			//}
			//if (changedBtnItemIdx == SELECTALL_3_FLOOR) {
			//	if (DGGetItemValLong (dialogID, SELECTALL_3_FLOOR) == TRUE) {
			//		// 모두 선택
			//		for (xx = 0 ; xx < layerInfo.floor_name.size () ; ++xx)
			//			DGSetItemValLong (dialogID, layerInfo.floor_idx [xx], TRUE);
			//	} else {
			//		// 모두 제외
			//		for (xx = 0 ; xx < layerInfo.floor_name.size () ; ++xx)
			//			DGSetItemValLong (dialogID, layerInfo.floor_idx [xx], FALSE);
			//	}
			//}
			//if (changedBtnItemIdx == SELECTALL_4_CJ) {
			//	if (DGGetItemValLong (dialogID, SELECTALL_4_CJ) == TRUE) {
			//		// 모두 선택
			//		for (xx = 0 ; xx < layerInfo.CJ_name.size () ; ++xx)
			//			DGSetItemValLong (dialogID, layerInfo.CJ_idx [xx], TRUE);
			//	} else {
			//		// 모두 제외
			//		for (xx = 0 ; xx < layerInfo.CJ_name.size () ; ++xx)
			//			DGSetItemValLong (dialogID, layerInfo.CJ_idx [xx], FALSE);
			//	}
			//}
			//if (changedBtnItemIdx == SELECTALL_5_ORDER) {
			//	if (DGGetItemValLong (dialogID, SELECTALL_5_ORDER) == TRUE) {
			//		// 모두 선택
			//		for (xx = 0 ; xx < layerInfo.orderInCJ_name.size () ; ++xx)
			//			DGSetItemValLong (dialogID, layerInfo.orderInCJ_idx [xx], TRUE);
			//	} else {
			//		// 모두 제외
			//		for (xx = 0 ; xx < layerInfo.orderInCJ_name.size () ; ++xx)
			//			DGSetItemValLong (dialogID, layerInfo.orderInCJ_idx [xx], FALSE);
			//	}
			//}

			//// 부재를 선택하면 공사 코드를 자동 선택
			//for (xx = 0 ; xx < layerInfo.obj_name.size () ; ++xx) {
			//	if (DGGetItemValLong (dialogID, layerInfo.obj_idx [xx]) == TRUE) {
			//		for (yy = 0 ; yy < layerInfo.code_name.size () ; ++yy) {
			//			if (strncmp (layerInfo.obj_cat [xx].c_str (), layerInfo.code_name [yy].c_str (), 4) == 0) {
			//				DGSetItemValLong (dialogID, layerInfo.code_idx [yy], TRUE);
			//			}
			//		}
			//	}
			//}

			//// 객체를 선택하면 공사 코드를 자동 선택
			//for (xx = 0 ; xx < layerInfo.subObj_name.size () ; ++xx) {
			//	if (DGGetItemValLong (dialogID, layerInfo.subObj_idx [xx]) == TRUE) {
			//		for (yy = 0 ; yy < layerInfo.code_name.size () ; ++yy) {
			//			if (strncmp (layerInfo.subObj_cat [xx].c_str (), layerInfo.code_name [yy].c_str (), 4) == 0) {
			//				DGSetItemValLong (dialogID, layerInfo.code_idx [yy], TRUE);
			//			}
			//		}
			//	}
			//}

			//// 버튼의 이름 보여주기
			//for (xx = 0 ; xx < layerInfo.code_name.size () ; ++xx) {
			//	if ((layerInfo.code_idx [xx] == changedBtnItemIdx) && (layerInfo.code_state [xx] == true)) {
			//		sprintf (tempStr, "%s", layerInfo.code_name [xx].c_str ());
			//		DGSetItemText (dialogID, LABEL_CODE, tempStr);
			//	}
			//}
			//for (xx = 0 ; xx < layerInfo.dong_name.size () ; ++xx) {
			//	if ((layerInfo.dong_idx [xx] == changedBtnItemIdx) && (layerInfo.dong_state [xx] == true)) {
			//		sprintf (tempStr, "%s", layerInfo.dong_name [xx].c_str ());
			//		DGSetItemText (dialogID, LABEL_CODE, tempStr);
			//	}
			//}
			//for (xx = 0 ; xx < layerInfo.floor_name.size () ; ++xx) {
			//	if ((layerInfo.floor_idx [xx] == changedBtnItemIdx) && (layerInfo.floor_state [xx] == true)) {
			//		sprintf (tempStr, "%s", layerInfo.floor_name [xx].c_str ());
			//		DGSetItemText (dialogID, LABEL_CODE, tempStr);
			//	}
			//}
			//for (xx = 0 ; xx < layerInfo.CJ_name.size () ; ++xx) {
			//	if ((layerInfo.CJ_idx [xx] == changedBtnItemIdx) && (layerInfo.CJ_state [xx] == true)) {
			//		sprintf (tempStr, "%s", layerInfo.CJ_name [xx].c_str ());
			//		DGSetItemText (dialogID, LABEL_CODE, tempStr);
			//	}
			//}
			//for (xx = 0 ; xx < layerInfo.orderInCJ_name.size () ; ++xx) {
			//	if ((layerInfo.orderInCJ_idx [xx] == changedBtnItemIdx) && (layerInfo.orderInCJ_state [xx] == true)) {
			//		sprintf (tempStr, "%s", layerInfo.orderInCJ_name [xx].c_str ());
			//		DGSetItemText (dialogID, LABEL_CODE, tempStr);
			//	}
			//}
			//for (xx = 0 ; xx < layerInfo.obj_name.size () ; ++xx) {
			//	if ((layerInfo.obj_idx [xx] == changedBtnItemIdx) && (layerInfo.obj_state [xx] == true)) {
			//		sprintf (tempStr, "%s", layerInfo.obj_name [xx].c_str ());
			//		DGSetItemText (dialogID, LABEL_CODE, tempStr);
			//	}
			//}
			//for (xx = 0 ; xx < layerInfo.subObj_name.size () ; ++xx) {
			//	if ((layerInfo.subObj_idx [xx] == changedBtnItemIdx) && (layerInfo.subObj_state [xx] == true)) {
			//		sprintf (tempStr, "%s", layerInfo.subObj_name [xx].c_str ());
			//		DGSetItemText (dialogID, LABEL_CODE, tempStr);
			//	}
			//}

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					//// 공사 구분
					//for (xx = 0 ; xx < layerInfo.code_name.size () ; ++xx)
					//	(DGGetItemValLong (dialogID, layerInfo.code_idx [xx]) == TRUE) ? selectedInfo.code_state [xx] = true : selectedInfo.code_state [xx] = false;

					//// 동 구분
					//for (xx = 0 ; xx < layerInfo.dong_name.size () ; ++xx)
					//	(DGGetItemValLong (dialogID, layerInfo.dong_idx [xx]) == TRUE) ? selectedInfo.dong_state [xx] = true : selectedInfo.dong_state [xx] = false;

					//// 층 구분
					//for (xx = 0 ; xx < layerInfo.floor_name.size () ; ++xx)
					//	(DGGetItemValLong (dialogID, layerInfo.floor_idx [xx]) == TRUE) ? selectedInfo.floor_state [xx] = true : selectedInfo.floor_state [xx] = false;

					//// CJ
					//for (xx = 0 ; xx < layerInfo.CJ_name.size () ; ++xx)
					//	(DGGetItemValLong (dialogID, layerInfo.CJ_idx [xx]) == TRUE) ? selectedInfo.CJ_state [xx] = true : selectedInfo.CJ_state [xx] = false;

					//// CJ 속 시공순서
					//for (xx = 0 ; xx < layerInfo.orderInCJ_name.size () ; ++xx)
					//	(DGGetItemValLong (dialogID, layerInfo.orderInCJ_idx [xx]) == TRUE) ? selectedInfo.orderInCJ_state [xx] = true : selectedInfo.orderInCJ_state [xx] = false;

					//// 부재
					//for (xx = 0 ; xx < layerInfo.obj_name.size () ; ++xx)
					//	(DGGetItemValLong (dialogID, layerInfo.obj_idx [xx]) == TRUE) ? selectedInfo.obj_state [xx] = true : selectedInfo.obj_state [xx] = false;

					//// 객체
					//for (xx = 0 ; xx < layerInfo.subObj_name.size () ; ++xx)
					//	(DGGetItemValLong (dialogID, layerInfo.subObj_idx [xx]) == TRUE) ? selectedInfo.subObj_state [xx] = true : selectedInfo.subObj_state [xx] = false;

					//// 버튼 상태 저장
					//saveButtonStatus ();

					break;

				case DG_CANCEL:
					break;

				default:
					//clickedBtnItemIdx = item;
					//item = 0;

					//// 저장된 버튼 상태를 불러옴
					//if (clickedBtnItemIdx == BUTTON_LOAD) {
					//	BNZeroMemory (&info, sizeof (API_ModulData));
					//	err = ACAPI_ModulData_Get (&info, "ButtonStatus");

					//	if (err == NoError && info.dataVersion == 1) {
					//		selectedInfoSaved = *(reinterpret_cast<StatusOfLayerNameSystem*> (*info.dataHdl));
					//		
					//		for (xx = 0 ; xx < layerInfo.code_name.size () ; ++xx) {
					//			if (selectedInfoSaved.code_state [xx] == true)
					//				DGSetItemValLong (dialogID, layerInfo.code_idx [xx], TRUE);
					//		}
					//		for (xx = 0 ; xx < layerInfo.dong_name.size () ; ++xx) {
					//			if (selectedInfoSaved.dong_state [xx] == true)
					//				DGSetItemValLong (dialogID, layerInfo.dong_idx [xx], TRUE);
					//		}
					//		for (xx = 0 ; xx < layerInfo.floor_name.size () ; ++xx) {
					//			if (selectedInfoSaved.floor_state [xx] == true)
					//				DGSetItemValLong (dialogID, layerInfo.floor_idx [xx], TRUE);
					//		}
					//		for (xx = 0 ; xx < layerInfo.CJ_name.size () ; ++xx) {
					//			if (selectedInfoSaved.CJ_state [xx] == true)
					//				DGSetItemValLong (dialogID, layerInfo.CJ_idx [xx], TRUE);
					//		}
					//		for (xx = 0 ; xx < layerInfo.orderInCJ_name.size () ; ++xx) {
					//			if (selectedInfoSaved.orderInCJ_state [xx] == true)
					//				DGSetItemValLong (dialogID, layerInfo.orderInCJ_idx [xx], TRUE);
					//		}
					//		for (xx = 0 ; xx < layerInfo.obj_name.size () ; ++xx) {
					//			if (selectedInfoSaved.obj_state [xx] == true)
					//				DGSetItemValLong (dialogID, layerInfo.obj_idx [xx], TRUE);
					//		}
					//		for (xx = 0 ; xx < layerInfo.subObj_name.size () ; ++xx) {
					//			if (selectedInfoSaved.subObj_state [xx] == true)
					//				DGSetItemValLong (dialogID, layerInfo.subObj_idx [xx], TRUE);
					//		}
					//	}

					//	BMKillHandle (&info.dataHdl);
					//}

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
