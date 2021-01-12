#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "Export.hpp"


const double DIST_BTW_COLUMN = 3.000;


// 배열 초기화 함수
void initArray (double arr [], short arrSize)
{
	short	xx;

	for (xx = 0 ; xx < arrSize ; ++xx)
		arr [xx] = 0.0;
}

// 오름차순으로 정렬할 때 사용하는 비교함수
int compare (const void* first, const void* second)
{
    if (*(double*)first > *(double*)second)
        return 1;
    else if (*(double*)first < *(double*)second)
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
	short	xx, yy, zz;
	short	i, j;
	short	iSel, jSel;

	// 객체 정보 가져오기
	API_Element			elem;
	API_ElementMemo		memo;
	API_ElemInfo3D		info3D;

	// 작업 층 정보
	API_StoryInfo	storyInfo;

	// 주열 번호를 선정하기 위한 변수들
	double	coords_hor [100];
	short	nCoords_hor;
	double	coords_ver [100];
	short	nCoords_ver;

	short	iHor, iVer;

	// 정보를 저장하기 위한 구조체
	ColumnPos		columnPos;
	

	// ================================================== 1. 기둥 정보 가져오기

	// 층 정보 가져오기
	BNZeroMemory (&storyInfo, sizeof (API_StoryInfo));
	ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo);

	columnPos.firstStory = storyInfo.firstStory;
	columnPos.lastStory = storyInfo.lastStory;
	columnPos.nStories = storyInfo.lastStory - storyInfo.firstStory;
	columnPos.nColumns = 0;

	// 층별로 기둥의 위치(주열)를 대략적으로 지정해 놓음
	for (xx = 0 ; xx < (storyInfo.lastStory - storyInfo.firstStory) ; ++xx) {

		nCoords_hor = 0;
		nCoords_ver = 0;

		ACAPI_Element_GetElemList (API_ColumnID, &elemList, APIFilt_OnVisLayer);	// 보이는 레이어에 있음
		for (yy = 0 ; yy < (short)elemList.GetSize () ; ++yy) {
			BNZeroMemory (&elem, sizeof (API_Element));
			BNZeroMemory (&memo, sizeof (API_ElementMemo));
			elem.header.guid = elemList.Pop ();
			err = ACAPI_Element_Get (&elem);
			err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

			if (storyInfo.data [0][xx].index == elem.header.floorInd) {
				coords_hor [nCoords_hor++] = elem.column.origoPos.x;
				coords_ver [nCoords_ver++] = elem.column.origoPos.y;
			}

			ACAPI_DisposeElemMemoHdls (&memo);
		}

		// 오름차순 정렬
		qsort (&coords_hor, nCoords_hor, sizeof (double), compare);
		qsort (&coords_ver, nCoords_ver, sizeof (double), compare);

		iHor = 0;
		iVer = 0;

		// 가로 방향 주열 정보 저장 (간격이 3000 초과하면 삽입)
		if (nCoords_hor >= 2) {
			columnPos.node_hor [xx][iHor++]	= coords_hor [0];

			for (zz = 1 ; zz < nCoords_hor ; ++zz) {
				if ( abs (coords_hor [zz] - columnPos.node_hor [xx][iHor - 1]) > DIST_BTW_COLUMN) {
					columnPos.node_hor [xx][iHor++]	= coords_hor [zz];
				}
			}
			columnPos.nNodes_hor [xx] = iHor;
		}

		// 세로 방향 주열 정보 저장 (간격이 3000 초과하면 삽입)
		if (nCoords_ver >= 2) {
			columnPos.node_ver [xx][iVer++]	= coords_ver [0];

			for (zz = 1 ; zz < nCoords_ver ; ++zz) {
				if ( abs (coords_ver [zz] - columnPos.node_ver [xx][iVer - 1]) > DIST_BTW_COLUMN) {
					columnPos.node_ver [xx][iVer++]	= coords_ver [zz];
				}
			}
			columnPos.nNodes_ver [xx] = iVer;
		}

		// 기둥 정보를 저장함 !!! -- iSel, jSel 값이 달라야 하는데?
		ACAPI_Element_GetElemList (API_ColumnID, &elemList, APIFilt_OnVisLayer);	// 보이는 레이어에 있음
		for (yy = 0 ; yy < (short)elemList.GetSize () ; ++yy) {
			BNZeroMemory (&elem, sizeof (API_Element));
			BNZeroMemory (&memo, sizeof (API_ElementMemo));
			elem.header.guid = elemList.Pop ();
			err = ACAPI_Element_Get (&elem);
			err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

			if (storyInfo.data [0][xx].index == elem.header.floorInd) {
				iSel = 0;
				jSel = 0;

				for (i = 0 ; i < iHor ; ++i) {
					if (abs (elem.column.origoPos.x - columnPos.node_hor [xx][i]) <= DIST_BTW_COLUMN) {
						iSel = i;
						break;
					}
				}

				for (j = 0; j < iVer ; ++j) {
					if (abs (elem.column.origoPos.y - columnPos.node_ver [xx][j]) <= DIST_BTW_COLUMN) {
						jSel = i;
						break;
					}
				}

				columnPos.columns [columnPos.nColumns].floorInd		= elem.header.floorInd;
				columnPos.columns [columnPos.nColumns].posX			= elem.column.origoPos.x;
				columnPos.columns [columnPos.nColumns].posY			= elem.column.origoPos.y;
				columnPos.columns [columnPos.nColumns].horLen		= elem.column.coreWidth + elem.column.venThick*2;
				columnPos.columns [columnPos.nColumns].verLen		= elem.column.coreDepth + elem.column.venThick*2;
				columnPos.columns [columnPos.nColumns].height		= elem.column.height;
				columnPos.columns [columnPos.nColumns].iHor			= iSel;
				columnPos.columns [columnPos.nColumns].iVer			= jSel;

				columnPos.nColumns ++;
			}

			ACAPI_DisposeElemMemoHdls (&memo);
		}
	}

	sprintf (msg, "기둥 개수: %d", columnPos.nColumns);
	ACAPI_WriteReport (msg, true);

	for (xx = 0 ; xx < 20 ; ++xx) {
		sprintf (msg, "[%d]: %d층, 번호(%d, %d), 위치(%.3f, %.3f), 크기(%.3f, %.3f, %.3f)",
			xx,
			columnPos.columns [xx].floorInd,
			columnPos.columns [xx].iHor,
			columnPos.columns [xx].iVer,
			columnPos.columns [xx].posX,
			columnPos.columns [xx].posY,
			columnPos.columns [xx].horLen,
			columnPos.columns [xx].verLen,
			columnPos.columns [xx].height);
		ACAPI_WriteReport (msg, true);
	}

	// 층 정보 폐기
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
	// ... columnPos
	return	err;
}