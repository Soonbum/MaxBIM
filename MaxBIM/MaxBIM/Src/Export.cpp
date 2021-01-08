#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "Export.hpp"

// 부재(기둥,보,슬래브)들의 정보를 추출하고 정리해서 엑셀 파일로 내보내기
GSErrCode	exportElementInfo (void)
{
	GSErrCode	err = NoError;

	GS::Array<API_Guid> elemList;
	char	msg [200];
	short	xx, yy;

	// 객체 정보 가져오기
	API_Element				elem;
	API_ElementMemo			memo;
	API_ElemInfo3D			info3D;

	// 작업 층 정보
	API_StoryInfo			storyInfo;


	// 가정
	// 1. 활성된 레이어들의 부재들 정보만 가져옴

	// 기둥
	// 모든 기둥들의 중심점, 가로/세로/높이 값을 가져옴
	ACAPI_Element_GetElemList (API_ColumnID, &elemList, APIFilt_OnVisLayer);	// 보이는 레이어에 있음

	for (xx = 0 ; xx < elemList.GetSize () ; ++xx) {
		BNZeroMemory (&elem, sizeof (API_Element));
		BNZeroMemory (&memo, sizeof (API_ElementMemo));
		elem.header.guid = elemList.Pop ();
		err = ACAPI_Element_Get (&elem);
		err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

		sprintf (msg, "층 인덱스: %d, 중심점 좌표: (%.3f, %.3f), 가로: %.3f, 세로: %.3f, 높이: %.3f, 아래쪽 고도: %.3f",
			elem.header.floorInd,
			elem.column.origoPos.x,
			elem.column.origoPos.y,
			elem.column.coreWidth + elem.column.venThick*2,
			elem.column.coreDepth + elem.column.venThick*2,
			elem.column.height,
			elem.column.bottomOffset);
		ACAPI_WriteReport (msg, true);

		ACAPI_DisposeElemMemoHdls (&memo);
	}
	
	// 보
	// 모든 보들의 시작/끝점 좌표, 너비, 높이, 길이 값을 가져옴
	ACAPI_Element_GetElemList (API_BeamID, &elemList, APIFilt_OnVisLayer);	// 보이는 레이어에 있음

	for (xx = 0 ; xx < elemList.GetSize () ; ++xx) {
		BNZeroMemory (&elem, sizeof (API_Element));
		BNZeroMemory (&memo, sizeof (API_ElementMemo));
		elem.header.guid = elemList.Pop ();
		err = ACAPI_Element_Get (&elem);
		err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

		sprintf (msg, "층 인덱스: %d, 시작점: (%.3f, %.3f), 끝점: (%.3f, %.3f), 너비: %.3f, 높이: %.3f, 고도: %.3f",
			elem.header.floorInd,
			elem.beam.begC.x,
			elem.beam.begC.y,
			elem.beam.endC.x,
			elem.beam.endC.y,
			elem.beam.width,
			elem.beam.height,
			elem.beam.level);
		ACAPI_WriteReport (msg, true);

		ACAPI_DisposeElemMemoHdls (&memo);
	}

	// 슬래브
	// 모든 슬래브들의 꼭지점 좌표, 가로, 세로 값을 가져옴
	ACAPI_Element_GetElemList (API_SlabID, &elemList, APIFilt_OnVisLayer);	// 보이는 레이어에 있음

	for (xx = 0 ; xx < elemList.GetSize () ; ++xx) {
		BNZeroMemory (&elem, sizeof (API_Element));
		BNZeroMemory (&memo, sizeof (API_ElementMemo));
		elem.header.guid = elemList.Pop ();
		err = ACAPI_Element_Get (&elem);
		err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

		sprintf (msg, "층 인덱스: %d, 두께: %.3f, 레벨: %.3f",
			elem.header.floorInd,
			elem.slab.thickness,
			elem.slab.level);
		ACAPI_WriteReport (msg, true);

		/*
		for (yy = 0 ; yy < elem.slab.poly.nCoords ; ++yy) {
			memo.coords [0][yy].x;
			memo.coords [0][yy].y;
		}
		*/

		ACAPI_DisposeElemMemoHdls (&memo);
	}

	return	err;
}