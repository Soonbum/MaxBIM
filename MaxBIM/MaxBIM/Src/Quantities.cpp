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
	// ...

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

	//for (xx = 0 ; xx < nElems ; ++xx) {
	//	for (yy = 0 ; yy < nElems ; ++yy) {
	// 단, 자기 자신과의 비교는 하지 않는다. (만약 xx번째 guid와 yy번째 guid가 동일하면 continue)

	// *** (중요) 각각의 요소는 측면 또는 밑면에 대한 X, Y, Z의 값 범위와 회전 각도 정보를 구조체로 갖고 있어야 함

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

	// 직육면체 형태의 꼭지점 8곳에 좌표 객체 배치 (라벨: 기둥 xx - 점 ?)
	// ...
	//BNZeroMemory (&elem, sizeof (API_Element));
	//BNZeroMemory (&memo, sizeof (API_ElementMemo));
	//ACAPI_Element_Get (&elem);
	//ACAPI_Element_GetMemo (elem.header.guid, &memo);

	//// 폴리라인일 경우,
	//if (elem.header.typeID == API_PolyLineID) {
	//	for (int yy = 1 ; yy <= elem.polyLine.poly.nCoords ; ++yy) {
	//		err = placeCoordinateLabel (memo.coords [0][yy].x, memo.coords [0][yy].y, 0, false, "", layerInd);
	//	}
	//}

	//// 모프일 경우,
	//if (elem.header.typeID == API_MorphID) {
	//	ACAPI_Element_Get3DInfo (elem.header, &info3D);

	//	// 모프의 3D 바디를 가져옴
	//	BNZeroMemory (&component, sizeof (API_Component3D));
	//	component.header.typeID = API_BodyID;
	//	component.header.index = info3D.fbody;
	//	err = ACAPI_3D_GetComponent (&component);

	//	// 모프의 3D 모델을 가져오지 못하면 종료
	//	if (err != NoError) {
	//		ACAPI_WriteReport ("모프의 3D 모델을 가져오지 못했습니다.", true);
	//		return err;
	//	}

	//	nVert = component.body.nVert;
	//	nEdge = component.body.nEdge;
	//	nPgon = component.body.nPgon;
	//	tm = component.body.tranmat;
	//	elemIdx = component.body.head.elemIndex - 1;
	//	bodyIdx = component.body.head.bodyIndex - 1;
	//
	//	// 정점 좌표를 임의 순서대로 저장함
	//	for (xx = 1 ; xx <= nVert ; ++xx) {
	//		component.header.typeID	= API_VertID;
	//		component.header.index	= xx;
	//		err = ACAPI_3D_GetComponent (&component);
	//		if (err == NoError) {
	//			trCoord.x = tm.tmx[0]*component.vert.x + tm.tmx[1]*component.vert.y + tm.tmx[2]*component.vert.z + tm.tmx[3];
	//			trCoord.y = tm.tmx[4]*component.vert.x + tm.tmx[5]*component.vert.y + tm.tmx[6]*component.vert.z + tm.tmx[7];
	//			trCoord.z = tm.tmx[8]*component.vert.x + tm.tmx[9]*component.vert.y + tm.tmx[10]*component.vert.z + tm.tmx[11];
	//			coords.Push (trCoord);
	//		}
	//	}
	//	nNodes = coords.GetSize ();

	//	tempString = format_string ("%s", "MIN 값");
	//	err = placeCoordinateLabel (info3D.bounds.xMin, info3D.bounds.yMin, info3D.bounds.zMin, true, tempString, layerInd);
	//	tempString = format_string ("%s", "MAX 값");
	//	err = placeCoordinateLabel (info3D.bounds.xMax, info3D.bounds.yMax, info3D.bounds.zMax, true, tempString, layerInd);

	//	for (xx = 1 ; xx <= nNodes ; ++xx) {
	//		point3D = coords.Pop ();

	//		tempString = format_string ("%d번", xx);
	//		err = placeCoordinateLabel (point3D.x, point3D.y, point3D.z, true, tempString, layerInd);
	//	}
	//}

	//// 보일 경우,
	//if (elem.header.typeID == API_BeamID) {
	//	tempString = format_string ("시작");
	//	err = placeCoordinateLabel (elem.beam.begC.x, elem.beam.begC.y, elem.beam.level, true, tempString, layerInd);
	//	tempString = format_string ("끝");
	//	err = placeCoordinateLabel (elem.beam.endC.x, elem.beam.endC.y, elem.beam.level, true, tempString, layerInd);
	//}

	//ACAPI_DisposeElemMemoHdls (&memo);

	return	err;
}