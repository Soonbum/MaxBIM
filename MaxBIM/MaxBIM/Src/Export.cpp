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
	long					nObjects = 0;

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
		}
	}
	BMKillHandle ((GSHandle *) &selNeigs);
	nObjects = objects.GetSize ();

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

	summary.sizeOfUformVectors = 0;				// 유로폼 개수 초기화
	summary.sizeOfSteelformVectors = 0;			// 스틸폼 개수 초기화
	summary.sizeOfIncornerPanelVectors = 0;		// 인코너판넬 개수 초기화

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
						width = atoi (GS::UniString (memo.params [0][28].value.uStr).ToCStr ().Get ());
						height = atoi (GS::UniString (memo.params [0][29].value.uStr).ToCStr ().Get ());

					// 비규격폼의 경우
					} else {
						width = static_cast<int> (memo.params [0][30].value.real * 1000);
						height = static_cast<int> (memo.params [0][31].value.real * 1000);
					}

					// 중복 항목은 개수만 증가
					for (zz = 0 ; zz < summary.sizeOfUformVectors ; ++zz) {
						if ((summary.uformWidth [zz] == width) && (summary.uformHeight [zz] == height)) {
							summary.uformCount [zz] ++;
							foundExistValue = true;
						}
					}

					// 신규 항목은 너비, 높이 추가하고 개수도 증가
					if ( !foundExistValue ) {
						summary.uformWidth.push_back (width);
						summary.uformHeight.push_back (height);
						summary.uformCount.push_back (1);
						summary.sizeOfUformVectors ++;
					}

					break;
				} else if (GS::ucscmp (memo.params [0][yy].value.uStr, L("스틸폼")) == 0) {
					foundExistValue = false;

					int width, height;

					// 규격폼의 경우
					if (memo.params [0][27].value.real == TRUE) {
						width = atoi (GS::UniString (memo.params [0][28].value.uStr).ToCStr ().Get ());
						height = atoi (GS::UniString (memo.params [0][29].value.uStr).ToCStr ().Get ());

					// 비규격폼의 경우
					} else {
						width = static_cast<int> (memo.params [0][30].value.real * 1000);
						height = static_cast<int> (memo.params [0][31].value.real * 1000);
					}

					// 중복 항목은 개수만 증가
					for (zz = 0 ; zz < summary.sizeOfSteelformVectors ; ++zz) {
						if ((summary.steelformWidth [zz] == width) && (summary.steelformHeight [zz] == height)) {
							summary.steelformCount [zz] ++;
							foundExistValue = true;
						}
					}

					// 신규 항목은 너비, 높이 추가하고 개수도 증가
					if ( !foundExistValue ) {
						summary.steelformWidth.push_back (width);
						summary.steelformHeight.push_back (height);
						summary.steelformCount.push_back (1);
						summary.sizeOfSteelformVectors ++;
					}

					break;
				}
			} else if (strncmp (memo.params [0][yy].name, "in_comp", strlen ("in_comp")) == 0) {
				if (GS::ucscmp (memo.params [0][yy].value.uStr, L("인코너판넬")) == 0) {
					foundExistValue = false;

					int hor, ver, hei;

					hor = static_cast<int> (memo.params [0][27].value.real * 1000);
					ver = static_cast<int> (memo.params [0][28].value.real * 1000);
					hei = static_cast<int> (memo.params [0][29].value.real * 1000);

					// 중복 항목은 개수만 증가
					for (zz = 0 ; zz < summary.sizeOfIncornerPanelVectors ; ++zz) {
						if ((summary.incornerPanelHor [zz] == hor) && (summary.incornerPanelVer [zz] == ver) && (summary.incornerPanelHei [zz] == hei)) {
							summary.incornerPanelCount [zz] ++;
							foundExistValue = true;
						}
					}

					// 신규 항목은 가로, 세로, 높이 추가하고 개수도 증가
					if ( !foundExistValue ) {
						summary.incornerPanelHor.push_back (hor);
						summary.incornerPanelVer.push_back (ver);
						summary.incornerPanelHei.push_back (hei);
						summary.incornerPanelCount.push_back (1);
						summary.sizeOfIncornerPanelVectors ++;
					}

					break;
				} else if (GS::ucscmp (memo.params [0][yy].value.uStr, L("아웃코너판넬")) == 0) {
					//
					break;
				}
			} else if (strncmp (memo.params [0][yy].name, "w_comp", strlen ("w_comp")) == 0) {
				if (GS::ucscmp (memo.params [0][yy].value.uStr, L("아웃코너앵글")) == 0) {
					//
					break;
				} else if (GS::ucscmp (memo.params [0][yy].value.uStr, L("목재")) == 0) {
					//
					break;
				} else if (GS::ucscmp (memo.params [0][yy].value.uStr, L("휠러스페이서")) == 0) {
					//
					break;
				}
			} else if (strncmp (memo.params [0][yy].name, "p_comp", strlen ("p_comp")) == 0) {
				if (GS::ucscmp (memo.params [0][yy].value.uStr, L("사각파이프")) == 0) {
					//
					break;
				}
			} else if (strncmp (memo.params [0][yy].name, "g_comp", strlen ("g_comp")) == 0) {
				if (GS::ucscmp (memo.params [0][yy].value.uStr, L("합판")) == 0) {
					//
					break;
				}
			} else if (strncmp (memo.params [0][yy].name, "sup_type", strlen ("sup_type")) == 0) {
				if (GS::ucscmp (memo.params [0][yy].value.uStr, L("RS Push-Pull Props 헤드피스 (인양고리 포함)")) == 0) {
					//
					break;
				} else if (GS::ucscmp (memo.params [0][yy].value.uStr, L("RS Push-Pull Props")) == 0) {
					//
					break;
				} else if (GS::ucscmp (memo.params [0][yy].value.uStr, L("벽체 타이")) == 0) {
					//
					break;
				} else if (GS::ucscmp (memo.params [0][yy].value.uStr, L("직교클램프")) == 0) {
					//
					break;
				} else if (GS::ucscmp (memo.params [0][yy].value.uStr, L("핀볼트세트")) == 0) {
					//
					break;
				}
			}

			if (yy == 50) {
				// 알 수 없는 객체
				// ...
			}

			++ yy;
		}

		ACAPI_DisposeElemMemoHdls (&memo);
	}

	// 최종 텍스트 표시
	BNZeroMemory (&windowInfo, sizeof (API_WindowInfo));
	windowInfo.typeID = APIWind_MyTextID;
	windowInfo.index = 1;

	ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, "[유로폼]\n");
	for (xx = 0 ; xx < summary.sizeOfUformVectors ; ++xx) {
		sprintf (buffer, "%d x %d : %d EA\n", summary.uformWidth [xx], summary.uformHeight [xx], summary.uformCount [xx]);
		ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
	}

	ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, "\n[스틸폼]\n");
	for (xx = 0 ; xx < summary.sizeOfSteelformVectors ; ++xx) {
		sprintf (buffer, "%d x %d : %d EA\n", summary.steelformWidth [xx], summary.steelformHeight [xx], summary.steelformCount [xx]);
		ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
	}

	ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, "\n[인코너판넬]\n");
	for (xx = 0 ; xx < summary.sizeOfIncornerPanelVectors ; ++xx) {
		sprintf (buffer, "%d x %d x %d : %d EA\n", summary.incornerPanelHor [xx], summary.incornerPanelVer [xx], summary.incornerPanelHei [xx], summary.steelformCount [xx]);
		ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
	}

	// ...

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
