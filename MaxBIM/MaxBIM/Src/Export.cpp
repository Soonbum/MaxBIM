#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "Export.hpp"

using namespace exportDG;

double DIST_BTW_COLUMN = 2.000;		// 기둥 간 최소 간격 (기본값), 추후 다이얼로그에서 변경할 수 있음


// 배열 초기화 함수
void initArray (double arr [], short arrSize)
{
	short	xx;

	for (xx = 0 ; xx < arrSize ; ++xx)
		arr [xx] = 0.0;
}

// 오름차순으로 정렬할 때 사용하는 비교함수 (퀵소트)
int compare (const void* first, const void* second)
{
    if (*(double*)first > *(double*)second)
        return 1;
    else if (*(double*)first < *(double*)second)
        return -1;
    else
        return 0;
}

// 가로주열, 세로주열, 층 정보를 이용하여 기둥 찾기
ColumnInfo	findColumn (ColumnPos* columnPos, short iHor, short iVer, short floorInd)
{
	ColumnInfo	resultColumn;
	short	xx;

	resultColumn.floorInd = 0;
	resultColumn.iHor = 0;
	resultColumn.iVer = 0;
	resultColumn.posX = 0.0;
	resultColumn.posY = 0.0;
	resultColumn.horLen = 0.0;
	resultColumn.verLen = 0.0;
	resultColumn.height = 0.0;

	for (xx = 0 ; xx < columnPos->nColumns ; ++xx) {
		if ( (columnPos->columns [xx].iHor == iHor) && (columnPos->columns [xx].iVer == iVer) && (columnPos->columns [xx].floorInd == floorInd) ) {
			resultColumn.floorInd = columnPos->columns [xx].floorInd;
			resultColumn.iHor = columnPos->columns [xx].iHor;
			resultColumn.iVer = columnPos->columns [xx].iVer;
			resultColumn.posX = columnPos->columns [xx].posX;
			resultColumn.posY = columnPos->columns [xx].posY;
			resultColumn.horLen = columnPos->columns [xx].horLen;
			resultColumn.verLen = columnPos->columns [xx].verLen;
			resultColumn.height = columnPos->columns [xx].height;
		}
	}

	return	resultColumn;
}

// 부재(기둥,보,슬래브)들의 정보를 추출하고 정리해서 엑셀 파일로 내보내기
GSErrCode	exportGridElementInfo (void)
{
	GSErrCode	err = NoError;

	GS::Array<API_Guid> elemList;
	long	nElems;
	char	msg [512];
	char	buffer [256];
	char	piece [20];
	short	xx, yy, zz;
	short	i, j;
	short	result;

	// 객체 정보 가져오기
	API_Element			elem;
	API_ElementMemo		memo;
	//API_ElemInfo3D		info3D;

	// 작업 층 정보
	API_StoryInfo	storyInfo;

	// 주열 번호를 선정하기 위한 변수들 (가로와 세로 따로 정해야 하므로 나뉘어짐)
	double	coords_hor [100];
	short	nCoords_hor;
	double	coords_ver [100];
	short	nCoords_ver;

	short	iHor, iVer;
	short	iSel, jSel;
	short	maxHor, maxVer;

	// 파일 저장을 위한 변수
	API_SpecFolderID	specFolderID = API_ApplicationFolderID;
	IO::Location		location;
	GS::UniString		resultString;
	API_MiscAppInfo		miscAppInfo;

	FILE	*fp;
	char	file_column [256];
	bool	bFileCreationSuccess_column;

	// 정보를 저장하기 위한 구조체
	ColumnPos		columnPos;
	ColumnInfo		resultColumn;
	

	// ================================================== 기둥
	// [다이얼로그] 기둥 간 최소 간격 거리를 사용자에게 입력 받음 (기본값: 2000 mm)
	result = DGBlankModalDialog (250, 100, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, inputThresholdHandler, 0);

	if (result != DG_OK) {
		ACAPI_WriteReport ("내보내기를 중단합니다.", true);
		return	err;
	}

	// 층 정보 가져오기
	BNZeroMemory (&storyInfo, sizeof (API_StoryInfo));
	ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo);

	columnPos.firstStory = storyInfo.firstStory;	// 최하위 층 인덱스 (예: 지하 4층인 경우, -4)
	columnPos.lastStory = storyInfo.lastStory;		// 최상위 층 인덱스 (예: 지상 35층인 경우, 34)
	columnPos.nStories = storyInfo.lastStory - storyInfo.firstStory + (storyInfo.skipNullFloor) * 1;	// 0층을 생략했다면 +1
	columnPos.nColumns = 0;

	// 층별로 기둥의 위치(주열)를 대략적으로 지정해 놓음
	for (xx = 0 ; xx < columnPos.nStories ; ++xx) {

		nCoords_hor = 0;
		nCoords_ver = 0;

		// 좌표값을 일단 저장하되, X와 Y축 방향으로 나눠서 저장
		ACAPI_Element_GetElemList (API_ColumnID, &elemList, APIFilt_OnVisLayer);	// 보이는 레이어에 있음
		nElems = elemList.GetSize ();
		for (yy = 0 ; yy < nElems ; ++yy) {
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

		// 가로 방향 주열 정보 저장 (간격이 DIST_BTW_COLUMN 이상이면 삽입)
		if (nCoords_hor >= 2) {
			columnPos.node_hor [xx][iHor]	= coords_hor [0];
			iHor ++;

			for (zz = 1 ; zz < nCoords_hor ; ++zz) {
				if ( abs (coords_hor [zz] - columnPos.node_hor [xx][iHor - 1]) >= DIST_BTW_COLUMN) {
					columnPos.node_hor [xx][iHor]	= coords_hor [zz];
					iHor ++;
				}
			}
			columnPos.nNodes_hor [xx] = iHor;
		}

		// 세로 방향 주열 정보 저장 (간격이 DIST_BTW_COLUMN 이상이면 삽입)
		if (nCoords_ver >= 2) {
			columnPos.node_ver [xx][iVer]	= coords_ver [0];
			iVer ++;

			for (zz = 1 ; zz < nCoords_ver ; ++zz) {
				if ( abs (coords_ver [zz] - columnPos.node_ver [xx][iVer - 1]) >= DIST_BTW_COLUMN) {
					columnPos.node_ver [xx][iVer]	= coords_ver [zz];
					iVer ++;
				}
			}
			columnPos.nNodes_ver [xx] = iVer;
		}

		// 기둥 정보를 저장함
		ACAPI_Element_GetElemList (API_ColumnID, &elemList, APIFilt_OnVisLayer);	// 보이는 레이어에 있음
		nElems = elemList.GetSize ();
		for (yy = 0 ; yy < nElems ; ++yy) {
			BNZeroMemory (&elem, sizeof (API_Element));
			BNZeroMemory (&memo, sizeof (API_ElementMemo));
			elem.header.guid = elemList.Pop ();
			err = ACAPI_Element_Get (&elem);
			err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

			if (storyInfo.data [0][xx].index == elem.header.floorInd) {
				iSel = 0;
				jSel = 0;

				for (i = 0 ; i < columnPos.nNodes_hor [xx] ; ++i) {
					if (abs (elem.column.origoPos.x - columnPos.node_hor [xx][i]) < DIST_BTW_COLUMN) {
						iSel = i;
					}
				}

				for (j = 0; j < columnPos.nNodes_ver [xx] ; ++j) {
					if (abs (elem.column.origoPos.y - columnPos.node_ver [xx][j]) < DIST_BTW_COLUMN) {
						jSel = j;
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

	// 층 정보 폐기
	BMKillHandle ((GSHandle *) &storyInfo.data);

	// 가로, 세로 주열 최대 개수 찾기
	maxHor = 0;
	maxVer = 0;
	for (xx = 0 ; xx < columnPos.nStories ; ++xx) {
		if (columnPos.nNodes_hor [xx] > maxHor)
			maxHor = columnPos.nNodes_hor [xx];
		if (columnPos.nNodes_ver [xx] > maxVer) {
			maxVer = columnPos.nNodes_ver [xx];
		}
	}

	// 엑셀 파일로 기둥 정보 내보내기
	ACAPI_Environment (APIEnv_GetMiscAppInfoID, &miscAppInfo);
	sprintf (file_column, "%s - Column.csv", miscAppInfo.caption);
	fp = fopen (file_column, "w+");

	if (fp != NULL) {
		// 헤더행
		strcpy (buffer, "주열구분,구분");
		for (xx = storyInfo.firstStory ; xx <= storyInfo.lastStory ; ++xx) {
			if (xx < 0) {
				sprintf (piece, ",B%d", -xx);	// 지하층
			} else {
				sprintf (piece, ",F%d", xx+1);	// 지상층 (0층 제외)
			}
			strcat (buffer, piece);
		}
		strcat (buffer, "\n");
		fprintf (fp, buffer);

		for (xx = 0 ; xx < maxHor ; ++xx) {
			for (yy = 0 ; yy < maxVer ; ++yy) {
				// 헤더 및 가로
				sprintf (buffer, "X%d - Y%d,가로,", xx+1, yy+1);
				for (zz = columnPos.firstStory ; zz <= columnPos.lastStory ; ++zz) {
					resultColumn = findColumn (&columnPos, xx, yy, zz);
					sprintf (piece, "%.0f,", resultColumn.horLen * 1000);
					strcat (buffer, piece);
				}
				strcat (buffer, "\n");
				fprintf (fp, buffer);

				// 세로
				strcpy (buffer, ",세로,");
				for (zz = columnPos.firstStory ; zz <= columnPos.lastStory ; ++zz) {
					resultColumn = findColumn (&columnPos, xx, yy, zz);
					sprintf (piece, "%.0f,", resultColumn.verLen * 1000);
					strcat (buffer, piece);
				}
				strcat (buffer, "\n");
				fprintf (fp, buffer);

				// 높이
				strcpy (buffer, ",높이,");
				for (zz = columnPos.firstStory ; zz <= columnPos.lastStory ; ++zz) {
					resultColumn = findColumn (&columnPos, xx, yy, zz);
					sprintf (piece, "%.0f,", resultColumn.height * 1000);
					strcat (buffer, piece);
				}
				strcat (buffer, "\n");
				fprintf (fp, buffer);
			}
		}

		fclose (fp);

		bFileCreationSuccess_column = true;
	} else {
		bFileCreationSuccess_column = false;
	}

	if (bFileCreationSuccess_column == true) {
		ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);
		location.ToDisplayText (&resultString);
		sprintf (msg, "%s를 다음 위치에 저장했습니다.\n\n%s\n또는 프로젝트 파일이 있는 폴더", file_column, resultString.ToCStr ().Get ());
		ACAPI_WriteReport (msg, true);
	} else {
		sprintf (msg, "%s를 생성할 수 없습니다.", file_column);
		ACAPI_WriteReport (msg, true);
	}

	// ================================================== 보
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

	return	err;
}

// 선택한 부재 정보 내보내기
GSErrCode	exportSelectedElementInfo (void)
{
	GSErrCode	err = NoError;
	long		nSel;
	short		xx, yy, zz;

	// 선택한 요소가 없으면 오류
	API_SelectionInfo		selectionInfo;
	API_Neig				**selNeigs;
	API_Element				tElem;
	GS::Array<API_Guid>&	objects = GS::Array<API_Guid> ();
	GS::Array<API_Guid>&	beams = GS::Array<API_Guid> ();
	long					nObjects = 0;
	long					nBeams = 0;

	err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, true);
	BMKillHandle ((GSHandle *) &selectionInfo.marquee.coords);
	if (err != NoError) {
		BMKillHandle ((GSHandle *) &selNeigs);
		ACAPI_WriteReport ("요소들을 선택해야 합니다.", true);
		return err;
	}

	// 객체 타입의 요소 GUID만 저장함
	if (selectionInfo.typeID != API_SelEmpty) {
		nSel = BMGetHandleSize ((GSHandle) selNeigs) / sizeof (API_Neig);
		for (xx = 0 ; xx < nSel && err == NoError ; ++xx) {
			tElem.header.typeID = Neig_To_ElemID ((*selNeigs)[xx].neigID);

			tElem.header.guid = (*selNeigs)[xx].guid;
			if (ACAPI_Element_Get (&tElem) != NoError)	// 가져올 수 있는 요소인가?
				continue;

			if (tElem.header.typeID == API_ObjectID)	// 객체 타입 요소인가?
				objects.Push (tElem.header.guid);

			if (tElem.header.typeID == API_BeamID)		// 보 타입 요소인가?
				beams.Push (tElem.header.guid);
		}
	}
	BMKillHandle ((GSHandle *) &selNeigs);
	nObjects = objects.GetSize ();
	nBeams = beams.GetSize ();

	// 선택한 요소들을 3D로 보여줌
	err = ACAPI_Automate (APIDo_ShowSelectionIn3DID);

	// 선택한 요소들의 정보 요약하기
	API_Element			elem;
	API_ElementMemo		memo;
	SummaryOfSelectedObjects	summary;
	bool				foundExistValue;

	API_NewWindowPars	windowPars;
	API_WindowInfo		windowInfo;
	char				buffer [256];

	summary.sizeOfUformKinds = 0;				// 유로폼 개수 초기화
	summary.sizeOfSteelformKinds = 0;			// 스틸폼 개수 초기화
	summary.sizeOfIncornerPanelKinds = 0;		// 인코너판넬 개수 초기화
	summary.sizeOfOutcornerPanelKinds = 0;		// 아웃코너판넬 개수 초기화
	summary.sizeOfOutcornerAngleKinds = 0;		// 아웃코너앵글 개수 초기화
	summary.sizeOfWoodKinds = 0;				// 목재 개수 초기화
	summary.sizeOfFsKinds = 0;					// 휠러스페이서 개수 초기화
	summary.sizeOfSqrPipeKinds = 0;				// 사각파이프 개수 초기화
	summary.sizeOfPlywoodKinds = 0;				// 합판 개수 초기화
	summary.nHeadpiece = 0;						// 개수 초기화: RS Push-Pull Props 헤드피스 (인양고리 포함)
	summary.sizeOfPropsKinds = 0;				// RS Push-Pull Props 개수 초기화
	summary.nTie = 0;							// 개수 초기화: 벽체 타이
	summary.nClamp = 0;							// 개수 초기화: 직교클램프
	summary.sizeOfPinboltKinds = 0;				// 핀볼트세트 개수 초기화
	summary.nJoin = 0;							// 개수 초기화: 결합철물 (사각와셔활용)
	summary.sizeOfBeamYokeKinds = 0;			// 보 멍에제 개수 초기화
	
	summary.sizeOfBeamKinds = 0;				// 보 개수 초기화

	summary.nUnknownObjects = 0;				// 알 수 없는 객채 개수 초기화

	BNZeroMemory (&windowPars, sizeof (API_NewWindowPars));
	windowPars.typeID = APIWind_MyTextID;
	windowPars.userRefCon = 1;
	GS::snuprintf (windowPars.wTitle, sizeof (windowPars.wTitle) / sizeof (GS::uchar_t), L("선택한 객체들의 정보"));
	err = ACAPI_Database (APIDb_NewWindowID, &windowPars, NULL);	// 텍스트 창 열기

	for (xx = 0 ; xx < nObjects ; ++xx) {
		BNZeroMemory (&elem, sizeof (API_Element));
		BNZeroMemory (&memo, sizeof (API_ElementMemo));
		elem.header.guid = objects.Pop ();
		err = ACAPI_Element_Get (&elem);
		err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

		yy = 0;
		while (yy <= 50) {
			if (strncmp (memo.params [0][yy].name, "u_comp", strlen ("u_comp")) == 0) {
				if (GS::ucscmp (memo.params [0][yy].value.uStr, L("유로폼")) == 0) {
					foundExistValue = false;

					int width, height;

					// 규격폼의 경우
					if (memo.params [0][27].value.real == TRUE) {
						width = atoi (getParameterStringByName (&memo, "eu_wid"));
						height = atoi (getParameterStringByName (&memo, "eu_hei"));

					// 비규격폼의 경우
					} else {
						width = static_cast<int> (round (getParameterValueByName (&memo, "eu_wid2") * 1000, 0));
						height = static_cast<int> (round (getParameterValueByName (&memo, "eu_hei2") * 1000, 0));
					}

					// 중복 항목은 개수만 증가
					for (zz = 0 ; zz < summary.sizeOfUformKinds ; ++zz) {
						if ((summary.uformWidth [zz] == width) && (summary.uformHeight [zz] == height)) {
							summary.uformCount [zz] ++;
							foundExistValue = true;
							break;
						}
					}

					// 신규 항목 추가하고 개수도 증가
					if ( !foundExistValue ) {
						summary.uformWidth [summary.sizeOfUformKinds] = width;
						summary.uformHeight [summary.sizeOfUformKinds] = height;
						summary.uformCount [summary.sizeOfUformKinds] = 1;
						summary.sizeOfUformKinds ++;
					}

					break;
				} else if (GS::ucscmp (memo.params [0][yy].value.uStr, L("스틸폼")) == 0) {
					foundExistValue = false;

					int width, height;

					// 규격폼의 경우
					if (memo.params [0][27].value.real == TRUE) {
						width = atoi (getParameterStringByName (&memo, "eu_wid"));
						height = atoi (getParameterStringByName (&memo, "eu_hei"));

					// 비규격폼의 경우
					} else {
						width = static_cast<int> (round (getParameterValueByName (&memo, "eu_wid2") * 1000, 0));
						height = static_cast<int> (round (getParameterValueByName (&memo, "eu_hei2") * 1000, 0));
					}

					// 중복 항목은 개수만 증가
					for (zz = 0 ; zz < summary.sizeOfSteelformKinds ; ++zz) {
						if ((summary.steelformWidth [zz] == width) && (summary.steelformHeight [zz] == height)) {
							summary.steelformCount [zz] ++;
							foundExistValue = true;
							break;
						}
					}

					// 신규 항목 추가하고 개수도 증가
					if ( !foundExistValue ) {
						summary.steelformWidth [summary.sizeOfSteelformKinds] = width;
						summary.steelformHeight [summary.sizeOfSteelformKinds] = height;
						summary.steelformCount [summary.sizeOfSteelformKinds] = 1;
						summary.sizeOfSteelformKinds ++;
					}

					break;
				}
			} else if (strncmp (memo.params [0][yy].name, "in_comp", strlen ("in_comp")) == 0) {
				if (GS::ucscmp (memo.params [0][yy].value.uStr, L("인코너판넬")) == 0) {
					foundExistValue = false;

					int hor, ver, hei;

					hor = static_cast<int> (round (getParameterValueByName (&memo, "wid_s") * 1000, 0));
					ver = static_cast<int> (round (getParameterValueByName (&memo, "leng_s") * 1000, 0));
					hei = static_cast<int> (round (getParameterValueByName (&memo, "hei_s") * 1000, 0));

					// 중복 항목은 개수만 증가
					for (zz = 0 ; zz < summary.sizeOfIncornerPanelKinds ; ++zz) {
						if ((summary.incornerPanelHor [zz] == hor) && (summary.incornerPanelVer [zz] == ver) && (summary.incornerPanelHei [zz] == hei)) {
							summary.incornerPanelCount [zz] ++;
							foundExistValue = true;
							break;
						}
					}

					// 신규 항목 추가하고 개수도 증가
					if ( !foundExistValue ) {
						summary.incornerPanelHor [summary.sizeOfIncornerPanelKinds] = hor;
						summary.incornerPanelVer [summary.sizeOfIncornerPanelKinds] = ver;
						summary.incornerPanelHei [summary.sizeOfIncornerPanelKinds] = hei;
						summary.incornerPanelCount [summary.sizeOfIncornerPanelKinds] = 1;
						summary.sizeOfIncornerPanelKinds ++;
					}

					break;
				} else if (GS::ucscmp (memo.params [0][yy].value.uStr, L("아웃코너판넬")) == 0) {
					foundExistValue = false;

					int hor, ver, hei;

					hor = static_cast<int> (round (getParameterValueByName (&memo, "wid_s") * 1000, 0));
					ver = static_cast<int> (round (getParameterValueByName (&memo, "leng_s") * 1000, 0));
					hei = static_cast<int> (round (getParameterValueByName (&memo, "hei_s") * 1000, 0));

					// 중복 항목은 개수만 증가
					for (zz = 0 ; zz < summary.sizeOfOutcornerPanelKinds ; ++zz) {
						if ((summary.outcornerPanelHor [zz] == hor) && (summary.outcornerPanelVer [zz] == ver) && (summary.outcornerPanelHei [zz] == hei)) {
							summary.outcornerPanelCount [zz] ++;
							foundExistValue = true;
							break;
						}
					}

					// 신규 항목 추가하고 개수도 증가
					if ( !foundExistValue ) {
						summary.outcornerPanelHor [summary.sizeOfOutcornerPanelKinds] = hor;
						summary.outcornerPanelVer [summary.sizeOfOutcornerPanelKinds] = ver;
						summary.outcornerPanelHei [summary.sizeOfOutcornerPanelKinds] = hei;
						summary.outcornerPanelCount [summary.sizeOfOutcornerPanelKinds] = 1;
						summary.sizeOfOutcornerPanelKinds ++;
					}

					break;
				}
			} else if (strncmp (memo.params [0][yy].name, "w_comp", strlen ("w_comp")) == 0) {
				if (GS::ucscmp (memo.params [0][yy].value.uStr, L("아웃코너앵글")) == 0) {
					foundExistValue = false;

					int length;

					length = static_cast<int> (round (getParameterValueByName (&memo, "a_leng") * 1000, 0));

					// 중복 항목은 개수만 증가
					for (zz = 0 ; zz < summary.sizeOfOutcornerAngleKinds ; ++zz) {
						if (summary.outcornerAngleLength [zz] == length) {
							summary.outcornerAngleCount [zz] ++;
							foundExistValue = true;
							break;
						}
					}

					// 신규 항목 추가하고 개수도 증가
					if ( !foundExistValue ) {
						summary.outcornerAngleLength [summary.sizeOfOutcornerAngleKinds] = length;
						summary.outcornerAngleCount [summary.sizeOfOutcornerAngleKinds] = 1;
						summary.sizeOfOutcornerAngleKinds ++;
					}

					break;
				} else if (GS::ucscmp (memo.params [0][yy].value.uStr, L("목재")) == 0) {
					foundExistValue = false;

					int thk, wid, len;

					thk = static_cast<int> (round (getParameterValueByName (&memo, "w_w") * 1000, 0));
					wid = static_cast<int> (round (getParameterValueByName (&memo, "w_h") * 1000, 0));
					len = static_cast<int> (round (getParameterValueByName (&memo, "w_leng") * 1000, 0));

					// 중복 항목은 개수만 증가
					for (zz = 0 ; zz < summary.sizeOfWoodKinds ; ++zz) {
						if ((summary.woodThk [zz] == thk) && (summary.woodWidth [zz] == wid) && (summary.woodLength [zz] == len)) {
							summary.woodCount [zz] ++;
							foundExistValue = true;
							break;
						}
					}

					// 신규 항목 추가하고 개수도 증가
					if ( !foundExistValue ) {
						summary.woodThk [summary.sizeOfWoodKinds] = thk;
						summary.woodWidth [summary.sizeOfWoodKinds] = wid;
						summary.woodLength [summary.sizeOfWoodKinds] = len;
						summary.woodCount [summary.sizeOfWoodKinds] = 1;
						summary.sizeOfWoodKinds ++;
					}

					break;
				} else if (GS::ucscmp (memo.params [0][yy].value.uStr, L("휠러스페이서")) == 0) {
					foundExistValue = false;

					int thk, len;

					thk = static_cast<int> (round (getParameterValueByName (&memo, "f_thk") * 1000, 0));
					len = static_cast<int> (round (getParameterValueByName (&memo, "f_leng") * 1000, 0));

					// 중복 항목은 개수만 증가
					for (zz = 0 ; zz < summary.sizeOfFsKinds ; ++zz) {
						if ((summary.fsThk [zz] == thk) && (summary.fsLength [zz] == len)) {
							summary.fsCount [zz] ++;
							foundExistValue = true;
							break;
						}
					}

					// 신규 항목 추가하고 개수도 증가
					if ( !foundExistValue ) {
						summary.fsThk [summary.sizeOfFsKinds] = thk;
						summary.fsLength [summary.sizeOfFsKinds] = len;
						summary.fsCount [summary.sizeOfFsKinds] = 1;
						summary.sizeOfFsKinds ++;
					}

					break;
				}
			} else if (strncmp (memo.params [0][yy].name, "p_comp", strlen ("p_comp")) == 0) {
				if (GS::ucscmp (memo.params [0][yy].value.uStr, L("사각파이프")) == 0) {
					// 비계파이프의 경우
					if (memo.params [0][27].typeID == APIParT_Length) {
						foundExistValue = false;

						int hor, ver, len;

						hor = 50;
						ver = 50;
						len = static_cast<int> (round (getParameterValueByName (&memo, "p_leng") * 1000, 0));

						// 중복 항목은 개수만 증가
						for (zz = 0 ; zz < summary.sizeOfSqrPipeKinds ; ++zz) {
							if ((summary.sqrPipeHor [zz] == hor) && (summary.sqrPipeVer [zz] == ver) && (summary.sqrPipeLength [zz] == len)) {
								summary.sqrPipeCount [zz] ++;
								foundExistValue = true;
								break;
							}
						}

						// 신규 항목 추가하고 개수도 증가
						if ( !foundExistValue ) {
							summary.sqrPipeHor [summary.sizeOfSqrPipeKinds] = hor;
							summary.sqrPipeVer [summary.sizeOfSqrPipeKinds] = ver;
							summary.sqrPipeLength [summary.sizeOfSqrPipeKinds] = len;
							summary.sqrPipeCount [summary.sizeOfSqrPipeKinds] = 1;
							summary.sizeOfSqrPipeKinds ++;
						}

						break;
					}

					// 직사각파이프의 경우
					if (memo.params [0][27].typeID == APIParT_CString) {
						foundExistValue = false;

						char nom [20];
						int hor = 0, ver = 0, len;

						//sprintf (nom, "%s", GS::UniString (memo.params [0][27].value.uStr).ToCStr ().Get ());
						strcpy (nom, getParameterStringByName (&memo, "p_nom"));

						char	*token;
						int		tokCount;

						token = strtok (nom, "x");
						tokCount = 1;
						while (token != NULL) {
							if (strlen (token) > 0) {
								if (tokCount == 1)
									hor = atoi (token);
								if (tokCount == 2)
									ver = atoi (token);
								tokCount ++;
							}
							token = strtok (NULL, ",");
						}

						len = static_cast<int> (round (getParameterValueByName (&memo, "p_leng") * 1000, 0));

						// 중복 항목은 개수만 증가
						for (zz = 0 ; zz < summary.sizeOfSqrPipeKinds ; ++zz) {
							if ((summary.sqrPipeHor [zz] == hor) && (summary.sqrPipeVer [zz] == ver) && (summary.sqrPipeLength [zz] == len)) {
								summary.sqrPipeCount [zz] ++;
								foundExistValue = true;
								break;
							}
						}

						// 신규 항목 추가하고 개수도 증가
						if ( !foundExistValue ) {
							summary.sqrPipeHor [summary.sizeOfSqrPipeKinds] = hor;
							summary.sqrPipeVer [summary.sizeOfSqrPipeKinds] = ver;
							summary.sqrPipeLength [summary.sizeOfSqrPipeKinds] = len;
							summary.sqrPipeCount [summary.sizeOfSqrPipeKinds] = 1;
							summary.sizeOfSqrPipeKinds ++;
						}

						break;
					}
				}
			} else if (strncmp (memo.params [0][yy].name, "g_comp", strlen ("g_comp")) == 0) {
				if (GS::ucscmp (memo.params [0][yy].value.uStr, L("합판")) == 0) {
					foundExistValue = false;

					int hor, ver;
					double thk = 0.0;

					// 규격에 따른 가로, 세로
					if (strncmp (getParameterStringByName (&memo, "p_stan"), "3x6 [910x1820]", strlen ("3x6 [910x1820]")) == 0) {
						hor = 910;
						ver = 1820;
					} else if (strncmp (getParameterStringByName (&memo, "p_stan"), "4x8 [1220x2440]", strlen ("4x8 [1220x2440]")) == 0) {
						hor = 1220;
						ver = 2440;
					} else if (strncmp (getParameterStringByName (&memo, "p_stan"), "비규격", strlen ("비규격")) == 0) {
						hor = static_cast<int> (round (getParameterValueByName (&memo, "p_wid") * 1000, 0));
						ver = static_cast<int> (round (getParameterValueByName (&memo, "p_leng") * 1000, 0));
					} else {
						// 그 외(비정형)는 알 수 없는 객체로 취급함
						summary.nUnknownObjects ++;
						break;
					}
					
					// 두께
					const char* p_thk = getParameterStringByName (&memo, "p_thk");

					if (strncmp (p_thk, "2.7T", strlen ("2.7T")) == 0)		thk = 2.7;
					if (strncmp (p_thk, "4.8T", strlen ("4.8T")) == 0)		thk = 4.8;
					if (strncmp (p_thk, "7.5T", strlen ("7.5T")) == 0)		thk = 7.5;
					if (strncmp (p_thk, "8.5T", strlen ("8.5T")) == 0)		thk = 8.5;
					if (strncmp (p_thk, "11.5T", strlen ("11.5T")) == 0)	thk = 11.5;
					if (strncmp (p_thk, "14.5T", strlen ("14.5T")) == 0)	thk = 14.5;
					if (strncmp (p_thk, "17.5T", strlen ("17.5T")) == 0)	thk = 17.5;

					// 중복 항목은 개수만 증가
					for (zz = 0 ; zz < summary.sizeOfPlywoodKinds ; ++zz) {
						if ((summary.plywoodHor [zz] == hor) && (summary.plywoodVer [zz] == ver) && (abs (summary.plywoodThk [zz] - thk) < EPS)) {
							summary.plywoodCount [zz] ++;
							foundExistValue = true;
							break;
						}
					}

					// 신규 항목 추가하고 개수도 증가
					if ( !foundExistValue ) {
						summary.plywoodHor [summary.sizeOfPlywoodKinds] = hor;
						summary.plywoodVer [summary.sizeOfPlywoodKinds] = ver;
						summary.plywoodThk [summary.sizeOfPlywoodKinds] = thk;
						summary.plywoodCount [summary.sizeOfPlywoodKinds] = 1;
						summary.sizeOfPlywoodKinds ++;
					}

					break;
				}
			} else if (strncmp (memo.params [0][yy].name, "sup_type", strlen ("sup_type")) == 0) {
				if (GS::ucscmp (memo.params [0][yy].value.uStr, L("RS Push-Pull Props 헤드피스 (인양고리 포함)")) == 0) {
					summary.nHeadpiece ++;
					break;
				} else if (GS::ucscmp (memo.params [0][yy].value.uStr, L("RS Push-Pull Props")) == 0) {
					foundExistValue = false;

					bool bUpper = TRUE;
					char nomUpper [30];
					bool bLower;
					char nomLower [30];
					bool bBase;
					
					if (memo.params [0][17].value.real == TRUE)
						bLower = true;
					else
						bLower = false;

					if (memo.params [0][9].value.real == TRUE)
						bBase = true;
					else
						bBase = false;

					sprintf (nomUpper, "%s", getParameterStringByName (&memo, "mainType"));
					sprintf (nomLower, "%s", getParameterStringByName (&memo, "auxType"));

					// 중복 항목은 개수만 증가
					for (zz = 0 ; zz < summary.sizeOfPropsKinds ; ++zz) {
						if ((summary.bPropsUpperSupp [zz] == bUpper) && (strncmp (summary.PropsNomUpperSupp [xx], nomUpper, strlen(nomUpper)) == 0) && (summary.bPropsLowerSupp [zz] == bLower) && (strncmp (summary.PropsNomLowerSupp [xx], nomLower, strlen(nomLower)) == 0) && (summary.bPropsBase [zz] == bBase)) {
							summary.propsCount [zz] ++;
							foundExistValue = true;
							break;
						}
					}

					// 신규 항목 추가하고 개수도 증가
					if ( !foundExistValue ) {
						summary.bPropsUpperSupp [summary.sizeOfPropsKinds] = bUpper;
						sprintf (summary.PropsNomUpperSupp [summary.sizeOfPropsKinds], nomUpper);
						summary.bPropsLowerSupp [summary.sizeOfPropsKinds] = bLower;
						sprintf (summary.PropsNomLowerSupp [summary.sizeOfPropsKinds], nomLower);
						summary.bPropsBase [summary.sizeOfPropsKinds] = bBase;
						summary.propsCount [summary.sizeOfPropsKinds] = 1;
						summary.sizeOfPropsKinds ++;
					}

					break;
				} else if (GS::ucscmp (memo.params [0][yy].value.uStr, L("벽체 타이")) == 0) {
					summary.nTie ++;
					break;
				} else if (GS::ucscmp (memo.params [0][yy].value.uStr, L("직교클램프")) == 0) {
					summary.nClamp ++;
					break;
				} else if (GS::ucscmp (memo.params [0][yy].value.uStr, L("핀볼트세트")) == 0) {
					foundExistValue = false;

					int bolt_len, washer_pos;
					int len;

					bolt_len = static_cast<int> (round (getParameterValueByName (&memo, "bolt_len") * 1000, 0));
					washer_pos = static_cast<int> (round (getParameterValueByName (&memo, "washer_pos") * 1000, 0));
					len = bolt_len - washer_pos + 40;

					if (len < 100) {
						len = 100;
					} else if (len < 150) {
						len = 150;
					} else if (len < 180) {
						len = 180;
					}

					// 중복 항목은 개수만 증가
					for (zz = 0 ; zz < summary.sizeOfPinboltKinds ; ++zz) {
						if (summary.pinboltLen [zz] == len) {
							summary.pinboltCount [zz] ++;
							foundExistValue = true;
							break;
						}
					}

					// 신규 항목 추가하고 개수도 증가
					if ( !foundExistValue ) {
						summary.pinboltLen [summary.sizeOfPinboltKinds] = len;
						summary.pinboltCount [summary.sizeOfPinboltKinds] = 1;
						summary.sizeOfPinboltKinds ++;
					}

					break;
				} else if (GS::ucscmp (memo.params [0][yy].value.uStr, L("결합철물 (사각와셔활용)")) == 0) {
					summary.nJoin ++;
					break;
				} else if (GS::ucscmp (memo.params [0][yy].value.uStr, L("보 멍에제")) == 0) {
					foundExistValue = false;

					int len;

					len = static_cast<int> (round (getParameterValueByName (&memo, "beamLength") * 1000, 0));

					// 중복 항목은 개수만 증가
					for (zz = 0 ; zz < summary.sizeOfBeamYokeKinds ; ++zz) {
						if (summary.beamYokeLength [zz] == len) {
							summary.beamYokeCount [zz] ++;
							foundExistValue = true;
							break;
						}
					}

					// 신규 항목 추가하고 개수도 증가
					if ( !foundExistValue ) {
						summary.beamYokeLength [summary.sizeOfBeamYokeKinds] = len;
						summary.beamYokeCount [summary.sizeOfBeamYokeKinds] = 1;
						summary.sizeOfBeamYokeKinds ++;
					}

					break;
				}
			}

			if (yy == 50) {
				// 알 수 없는 객체

				summary.nUnknownObjects ++;
				break;
			}

			++ yy;
		}

		ACAPI_DisposeElemMemoHdls (&memo);
	}

	// 보 개수 세기
	for (xx = 0 ; xx < nBeams ; ++xx) {
		BNZeroMemory (&elem, sizeof (API_Element));
		BNZeroMemory (&memo, sizeof (API_ElementMemo));
		elem.header.guid = beams.Pop ();
		err = ACAPI_Element_Get (&elem);
		err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

		foundExistValue = false;

		int len;

		len = static_cast<int> (round (GetDistance (elem.beam.begC, elem.beam.endC) * 1000, 0));

		// 중복 항목은 개수만 증가
		for (zz = 0 ; zz < summary.sizeOfBeamKinds ; ++zz) {
			if (summary.beamLength [zz] == len) {
				summary.beamCount [zz] ++;
				foundExistValue = true;
			}
		}

		// 신규 항목 추가하고 개수도 증가
		if ( !foundExistValue ) {
			summary.beamLength [summary.sizeOfBeamKinds] = len;
			summary.beamCount [summary.sizeOfBeamKinds] = 1;
			summary.sizeOfBeamKinds ++;
		}

		ACAPI_DisposeElemMemoHdls (&memo);
	}

	// 최종 텍스트 표시
	BNZeroMemory (&windowInfo, sizeof (API_WindowInfo));
	windowInfo.typeID = APIWind_MyTextID;
	windowInfo.index = 1;

	for (xx = 0 ; xx < summary.sizeOfUformKinds ; ++xx) {
		if (xx == 0)
			ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, "[유로폼]\n");
		sprintf (buffer, "%d x %d : %d EA\n", summary.uformWidth [xx], summary.uformHeight [xx], summary.uformCount [xx]);
		ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
	}

	for (xx = 0 ; xx < summary.sizeOfSteelformKinds ; ++xx) {
		if (xx == 0)
			ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, "\n[스틸폼]\n");
		sprintf (buffer, "%d x %d : %d EA\n", summary.steelformWidth [xx], summary.steelformHeight [xx], summary.steelformCount [xx]);
		ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
	}

	for (xx = 0 ; xx < summary.sizeOfIncornerPanelKinds ; ++xx) {
		if (xx == 0)
			ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, "\n[인코너판넬]\n");
		sprintf (buffer, "%d x %d x %d : %d EA\n", summary.incornerPanelHor [xx], summary.incornerPanelVer [xx], summary.incornerPanelHei [xx], summary.incornerPanelCount [xx]);
		ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
	}

	for (xx = 0 ; xx < summary.sizeOfOutcornerPanelKinds ; ++xx) {
		if (xx == 0)
			ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, "\n[아웃코너판넬]\n");
		sprintf (buffer, "%d x %d x %d : %d EA\n", summary.outcornerPanelHor [xx], summary.outcornerPanelVer [xx], summary.outcornerPanelHei [xx], summary.outcornerPanelCount [xx]);
		ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
	}

	for (xx = 0 ; xx < summary.sizeOfOutcornerAngleKinds ; ++xx) {
		if (xx == 0)
			ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, "\n[아웃코너앵글]\n");
		sprintf (buffer, "%d : %d EA\n", summary.outcornerAngleLength [xx], summary.outcornerAngleCount [xx]);
		ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
	}

	for (xx = 0 ; xx < summary.sizeOfWoodKinds ; ++xx) {
		if (xx == 0)
			ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, "\n[목재]\n");
		sprintf (buffer, "%d x %d x %d : %d EA\n", summary.woodThk [xx], summary.woodWidth [xx], summary.woodLength [xx], summary.woodCount [xx]);
		ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
	}

	for (xx = 0 ; xx < summary.sizeOfFsKinds ; ++xx) {
		if (xx == 0)
			ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, "\n[휠러스페이서]\n");
		sprintf (buffer, "%d x %d : %d EA\n", summary.fsThk [xx], summary.fsLength [xx], summary.fsCount [xx]);
		ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
	}

	for (xx = 0 ; xx < summary.sizeOfSqrPipeKinds ; ++xx) {
		if (xx == 0)
			ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, "\n[사각파이프]\n");
		sprintf (buffer, "%d x %d x %d : %d EA\n", summary.sqrPipeHor [xx], summary.sqrPipeVer [xx], summary.sqrPipeLength [xx], summary.sqrPipeCount [xx]);
		ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
	}

	for (xx = 0 ; xx < summary.sizeOfPlywoodKinds ; ++xx) {
		if (xx == 0)
			ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, "\n[합판]\n");
		sprintf (buffer, "%d x %d x %.1fT : %d EA\n", summary.plywoodHor [xx], summary.plywoodVer [xx], summary.plywoodThk [xx], summary.plywoodCount [xx]);
		ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
	}

	if (summary.nHeadpiece > 0) {
		sprintf (buffer, "\n[RS Push-Pull Props 헤드피스]\n%d EA\n", summary.nHeadpiece);
		ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
	}

	for (xx = 0 ; xx < summary.sizeOfPropsKinds ; ++xx) {
		if (xx == 0)
			ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, "\n[RS Push-Pull Props]\n");

		char	buf1 [128];
		char	buf2 [128];
		char	buf3 [128];

		if (summary.bPropsUpperSupp [xx] == true)
			sprintf (buf1, "상부(%s)", summary.PropsNomUpperSupp [xx]);
		else
			sprintf (buf1, "상부(없음)");

		if (summary.bPropsLowerSupp [xx] == true)
			sprintf (buf2, "하부(%s)", summary.PropsNomLowerSupp [xx]);
		else
			sprintf (buf2, "하부(없음)");

		if (summary.bPropsBase [xx] == true)
			sprintf (buf3, "베이스(있음)");
		else
			sprintf (buf3, "베이스(없음)");

		sprintf (buffer, "%s %s %s : %d EA\n", buf1, buf2, buf3, summary.propsCount [xx]);
		ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
	}

	if (summary.nTie > 0) {
		sprintf (buffer, "\n[벽체 타이]\n%d EA\n", summary.nTie);
		ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
	}

	if (summary.nClamp > 0) {
		sprintf (buffer, "\n[직교클램프]\n%d EA\n", summary.nClamp);
		ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
	}

	for (xx = 0 ; xx < summary.sizeOfPinboltKinds ; ++xx) {
		if (xx == 0)
			ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, "\n[핀볼트세트]\n");
		sprintf (buffer, "%d : %d EA\n", summary.pinboltLen [xx], summary.pinboltCount [xx]);
		ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
	}

	if (summary.nJoin > 0) {
		sprintf (buffer, "\n[결합철물 (사각와셔활용)]\n%d EA\n", summary.nJoin);
		ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
	}

	for (xx = 0 ; xx < summary.sizeOfBeamYokeKinds ; ++xx) {
		if (xx == 0)
			ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, "\n[보 멍에제]\n");
		sprintf (buffer, "%d : %d EA\n", summary.beamYokeLength [xx], summary.beamYokeCount [xx]);
		ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
	}

	for (xx = 0 ; xx < summary.sizeOfBeamKinds ; ++xx) {
		if (xx == 0)
			ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, "\n[보]\n");
		sprintf (buffer, "%d : %d EA\n", summary.beamLength [xx], summary.beamCount [xx]);
		ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
	}

	if (summary.nUnknownObjects > 0) {
		sprintf (buffer, "\n[알 수 없는 객체]\n%d EA\n", summary.nUnknownObjects);
		ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
	}

	return	err;
}

// [다이얼로그] 기둥 간 최소 간격 거리를 사용자에게 입력 받음 (기본값: 2000 mm)
short DGCALLBACK inputThresholdHandler (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	short	itmIdx;
	short	itmPosX, itmPosY;

	switch (message) {
		case DG_MSG_INIT:
			// 다이얼로그 타이틀
			DGSetDialogTitle (dialogID, "주열 최소 간격 입력하기");

			// 확인 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 40, 60, 80, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, "확인");
			DGShowItem (dialogID, DG_OK);

			// 취소 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 140, 60, 80, 25);
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, "취소");
			DGShowItem (dialogID, DG_CANCEL);

			// 라벨: 주열 최소 간격
			itmPosX = 10;
			itmPosY = 15;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 120, 23);
			DGSetItemFont (dialogID, LABEL_DIST_BTW_COLUMN, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, LABEL_DIST_BTW_COLUMN, "주열 최소 간격 (mm)");
			DGShowItem (dialogID, LABEL_DIST_BTW_COLUMN);

			// Edit컨트롤: 주열 최소 간격
			itmPosX = 140;
			itmPosY = 10;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, itmPosX, itmPosY, 50, 25);
			DGSetItemFont (dialogID, EDITCONTROL_DIST_BTW_COLUMN, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemValDouble (dialogID, EDITCONTROL_DIST_BTW_COLUMN, DIST_BTW_COLUMN);
			DGShowItem (dialogID, EDITCONTROL_DIST_BTW_COLUMN);

			break;
		
		case DG_MSG_CHANGE:
			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					DIST_BTW_COLUMN = DGGetItemValDouble (dialogID, EDITCONTROL_DIST_BTW_COLUMN);
					break;

				case DG_CANCEL:
					break;
			}
		case DG_MSG_CLOSE:
			switch (item) {
				case DG_CLOSEBOX:
					break;
			}
	}

	result = item;

	return	result;
}
