#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "Export.hpp"

// 오름차순으로 정렬할 때 사용하는 비교함수
int compare (const void* first, const void* second)
{
    if (*(int*)first > *(int*)second)
        return 1;
    else if (*(int*)first < *(int*)second)
        return -1;
    else
        return 0;
}

// 부재(기둥,보,슬래브)들의 정보를 추출하고 정리해서 엑셀 파일로 내보내기
GSErrCode	exportElementInfo (void)
{
	GSErrCode	err = NoError;

	GS::Array<API_Guid> elemList;
	char	msg [200];
	char	buffer [200];
	char	piece [20];
	short	xx, yy;

	// 객체 정보 가져오기
	API_Element			elem;
	API_ElementMemo		memo;
	API_ElemInfo3D		info3D;

	// 작업 층 정보
	API_StoryInfo	storyInfo;

	// 주열 번호를 선정하기 위한 변수들
	double	coords1_hor [100];
	short	nCoords1_hor;
	double	coords1_ver [100];
	short	nCoords1_ver;

	double	coords2_hor [100];
	short	nCoords2_hor;
	double	coords2_ver [100];
	short	nCoords2_ver;

	// 정보를 저장하기 위한 구조체
	ColumnInfo		columnInfo;
	

	// 1. 기둥 정보 가져오기
	ACAPI_Element_GetElemList (API_ColumnID, &elemList, APIFilt_OnVisLayer);	// 보이는 레이어에 있음

	nCoords1_hor = 0;
	nCoords1_ver = 0;

	BNZeroMemory (&storyInfo, sizeof (API_StoryInfo));
	ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo);

	for (xx = 0 ; xx < (storyInfo.lastStory - storyInfo.firstStory) ; ++xx) {
		for (yy = 0 ; yy < elemList.GetSize () ; ++yy) {
			BNZeroMemory (&elem, sizeof (API_Element));
			BNZeroMemory (&memo, sizeof (API_ElementMemo));
			elem.header.guid = elemList.Pop ();
			err = ACAPI_Element_Get (&elem);
			err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

			if (storyInfo.data [0][yy].index == elem.header.floorInd) {
				coords1_hor [nCoords1_hor++] = elem.column.origoPos.x;
				coords1_ver [nCoords1_ver++] = elem.column.origoPos.y;
			}

			ACAPI_DisposeElemMemoHdls (&memo);
		}

		// ... 매 층별로 처리해야 함
		// 정렬 전 -- 출력!
		qsort (&coords1_hor, nCoords1_hor, sizeof (double), compare);
		// 정렬 후 -- 출력!
		// - traversing하면서 다른 배열에 또 저장: 간격이 3000 이하이면, 무시 초과하면 삽입
	}

	BMKillHandle ((GSHandle *) &storyInfo.data);



	//// 기둥
	//// 모든 기둥들의 중심점, 가로/세로/높이 값을 가져옴
	//ACAPI_Element_GetElemList (API_ColumnID, &elemList, APIFilt_OnVisLayer);	// 보이는 레이어에 있음

	//for (xx = 0 ; xx < elemList.GetSize () ; ++xx) {
	//	BNZeroMemory (&elem, sizeof (API_Element));
	//	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	//	elem.header.guid = elemList.Pop ();
	//	err = ACAPI_Element_Get (&elem);
	//	err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

	//	sprintf (msg, "층 인덱스: %d, 중심점 좌표: (%.3f, %.3f), 가로: %.3f, 세로: %.3f, 높이: %.3f, 아래쪽 고도: %.3f",
	//		elem.header.floorInd,
	//		elem.column.origoPos.x,
	//		elem.column.origoPos.y,
	//		elem.column.coreWidth + elem.column.venThick*2,
	//		elem.column.coreDepth + elem.column.venThick*2,
	//		elem.column.height,
	//		elem.column.bottomOffset);
	//	ACAPI_WriteReport (msg, true);

	//	ACAPI_DisposeElemMemoHdls (&memo);
	//}
	//
	//// 보
	//// 모든 보들의 시작/끝점 좌표, 너비, 높이, 길이 값을 가져옴
	//ACAPI_Element_GetElemList (API_BeamID, &elemList, APIFilt_OnVisLayer);	// 보이는 레이어에 있음

	//for (xx = 0 ; xx < elemList.GetSize () ; ++xx) {
	//	BNZeroMemory (&elem, sizeof (API_Element));
	//	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	//	elem.header.guid = elemList.Pop ();
	//	err = ACAPI_Element_Get (&elem);
	//	err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

	//	sprintf (msg, "층 인덱스: %d, 시작점: (%.3f, %.3f), 끝점: (%.3f, %.3f), 너비: %.3f, 높이: %.3f, 고도: %.3f",
	//		elem.header.floorInd,
	//		elem.beam.begC.x,
	//		elem.beam.begC.y,
	//		elem.beam.endC.x,
	//		elem.beam.endC.y,
	//		elem.beam.width,
	//		elem.beam.height,
	//		elem.beam.level);
	//	ACAPI_WriteReport (msg, true);

	//	ACAPI_DisposeElemMemoHdls (&memo);
	//}

	//// 슬래브
	//// 모든 슬래브들의 꼭지점 좌표, 가로, 세로 값을 가져옴
	//ACAPI_Element_GetElemList (API_SlabID, &elemList, APIFilt_OnVisLayer);	// 보이는 레이어에 있음

	//for (xx = 0 ; xx < elemList.GetSize () ; ++xx) {
	//	BNZeroMemory (&elem, sizeof (API_Element));
	//	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	//	elem.header.guid = elemList.Pop ();
	//	err = ACAPI_Element_Get (&elem);
	//	err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

	//	sprintf (msg, "층 인덱스: %d, 두께: %.3f, 레벨: %.3f",
	//		elem.header.floorInd,
	//		elem.slab.thickness,
	//		elem.slab.level);
	//	ACAPI_WriteReport (msg, true);

	//	/*
	//	for (yy = 0 ; yy < elem.slab.poly.nCoords ; ++yy) {
	//		memo.coords [0][yy].x;
	//		memo.coords [0][yy].y;
	//	}
	//	*/

	//	ACAPI_DisposeElemMemoHdls (&memo);
	//}




	/*
	// 층 범위 조사하기
	BNZeroMemory (&storyInfo, sizeof (API_StoryInfo));
	ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo);
	sprintf (msg, "처음 층: %d, 마지막 층: %d, 제로층 생략: %d", storyInfo.firstStory, storyInfo.lastStory, storyInfo.skipNullFloor);
	ACAPI_WriteReport (msg, true);
	for (xx = 0 ; xx < (storyInfo.lastStory - storyInfo.firstStory) ; ++xx) {
		sprintf (msg, "층 인덱스: %d, 레벨: %lf, 이름: %s", storyInfo.data [0][xx].index, storyInfo.data [0][xx].level, storyInfo.data [0][xx].name);
	}
	BMKillHandle ((GSHandle *) &storyInfo.data);

	// 파일이 저장되는 장소는 프로젝트 파일과 동일한 위치임
	FILE* fp = fopen ("Column.csv", "w+");

	// 헤더행
	strcpy (buffer, "주열구분,구분");
	for (xx = storyInfo.firstStory; xx <= storyInfo.lastStory ; ++xx) {
		if (xx < 0) {
			sprintf (piece, ",B%d", -xx);
		} else {
			sprintf (piece, ",F%d", xx);
		}
		strcat (buffer, piece);
	}
	fprintf (fp, buffer);

	// 2행
	// ...

	fclose(fp);
	*/

	// 기둥 정보를 구조체에 저장하기
	// ... columnInfo
	return	err;
}