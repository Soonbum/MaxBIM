#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "Export.hpp"

using namespace exportDG;

double DIST_BTW_COLUMN = 2.000;		// 기둥 간 최소 간격 (기본값), 추후 다이얼로그에서 변경할 수 있음

vector<API_Coord3D>	vecPos;		// 모든 객체들의 원점 좌표를 전부 저장함


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

// 선택한 부재들의 요약 정보: 생성자
SummaryOfObjectInfo::SummaryOfObjectInfo ()
{
	FILE	*fp;			// 파일 포인터
	char	line [10240];	// 파일에서 읽어온 라인 하나
	char	*token;			// 읽어온 문자열의 토큰
	short	lineCount;		// 읽어온 라인 수
	short	tokCount;		// 읽어온 토큰 개수
	short	xx;

	char	nthToken [50][50];	// n번째 토큰

	// 객체 정보 파일 가져오기
	fp = fopen ("C:\\objectInfo.csv", "r");

	if (fp == NULL) {
		ACAPI_WriteReport ("objectInfo.csv 파일을 C:\\로 복사하십시오.", true);
	} else {
		lineCount = 0;

		while (!feof (fp)) {
			tokCount = 0;
			fgets (line, sizeof(line), fp);

			token = strtok (line, ",");
			tokCount ++;
			lineCount ++;

			// 한 라인씩 처리
			while (token != NULL) {
				if (strlen (token) > 0) {
					strncpy (nthToken [tokCount-1], token, strlen (token)+1);
				}
				token = strtok (NULL, ",");
				tokCount ++;
			}

			// 표시할 정보 필드 개수
			this->nInfo.push_back ((short)(((tokCount-1) - 2) / 3));

			// 토큰 개수가 2개 이상일 때 유효함
			if ((tokCount-1) >= 2) {
				// 토큰 개수가 2 + 3의 배수 개씩만 저장됨 (초과된 항목은 제외)
				if (((tokCount-1) - 2) % 3 != 0) {
					tokCount --;
				}

				this->nameKey.push_back (nthToken [0]);
				this->nameVal.push_back (nthToken [1]);

				if ((tokCount-1) >= 3)	this->var1name.push_back (nthToken [2]);				else this->var1name.push_back ("");
				if ((tokCount-1) >= 4)	this->var1desc.push_back (nthToken [3]);				else this->var1desc.push_back ("");
				if ((tokCount-1) >= 5)	this->var1showFlag.push_back ((short)atoi (nthToken [4]));		else this->var1showFlag.push_back (0);

				if ((tokCount-1) >= 6)	this->var2name.push_back (nthToken [5]);				else this->var2name.push_back ("");
				if ((tokCount-1) >= 7)	this->var2desc.push_back (nthToken [6]);				else this->var2desc.push_back ("");
				if ((tokCount-1) >= 8)	this->var2showFlag.push_back ((short)atoi (nthToken [7]));		else this->var2showFlag.push_back (0);
				
				if ((tokCount-1) >= 9)	this->var3name.push_back (nthToken [8]);				else this->var3name.push_back ("");
				if ((tokCount-1) >= 10)	this->var3desc.push_back (nthToken [9]);				else this->var3desc.push_back ("");
				if ((tokCount-1) >= 11)	this->var3showFlag.push_back ((short)atoi (nthToken [10]));	else this->var3showFlag.push_back (0);

				if ((tokCount-1) >= 12)	this->var4name.push_back (nthToken [11]);				else this->var4name.push_back ("");
				if ((tokCount-1) >= 13)	this->var4desc.push_back (nthToken [12]);				else this->var4desc.push_back ("");
				if ((tokCount-1) >= 14)	this->var4showFlag.push_back ((short)atoi (nthToken [13]));	else this->var4showFlag.push_back (0);

				if ((tokCount-1) >= 15)	this->var5name.push_back (nthToken [14]);				else this->var5name.push_back ("");
				if ((tokCount-1) >= 16)	this->var5desc.push_back (nthToken [15]);				else this->var5desc.push_back ("");
				if ((tokCount-1) >= 17)	this->var5showFlag.push_back ((short)atoi (nthToken [16]));	else this->var5showFlag.push_back (0);

				if ((tokCount-1) >= 18)	this->var6name.push_back (nthToken [17]);				else this->var6name.push_back ("");
				if ((tokCount-1) >= 19)	this->var6desc.push_back (nthToken [18]);				else this->var6desc.push_back ("");
				if ((tokCount-1) >= 20)	this->var6showFlag.push_back ((short)atoi (nthToken [19]));	else this->var6showFlag.push_back (0);

				if ((tokCount-1) >= 21)	this->var7name.push_back (nthToken [20]);				else this->var7name.push_back ("");
				if ((tokCount-1) >= 22)	this->var7desc.push_back (nthToken [21]);				else this->var7desc.push_back ("");
				if ((tokCount-1) >= 23)	this->var7showFlag.push_back ((short)atoi (nthToken [22]));	else this->var7showFlag.push_back (0);

				if ((tokCount-1) >= 24)	this->var8name.push_back (nthToken [23]);				else this->var8name.push_back ("");
				if ((tokCount-1) >= 25)	this->var8desc.push_back (nthToken [24]);				else this->var8desc.push_back ("");
				if ((tokCount-1) >= 26)	this->var8showFlag.push_back ((short)atoi (nthToken [25]));	else this->var8showFlag.push_back (0);

				if ((tokCount-1) >= 27)	this->var9name.push_back (nthToken [26]);				else this->var9name.push_back ("");
				if ((tokCount-1) >= 28)	this->var9desc.push_back (nthToken [27]);				else this->var9desc.push_back ("");
				if ((tokCount-1) >= 29)	this->var9showFlag.push_back ((short)atoi (nthToken [28]));	else this->var9showFlag.push_back (0);
			}
		}

		// 파일 닫기
		fclose (fp);

		// 객체 종류 개수
		this->nKnownObjects = lineCount - 1;

		// 다른 멤버 변수 초기화
		vector<string>			vec_empty_string = vector<string> (200, "");
		vector<short>			vec_empty_short = vector<short> (200, 0);
		vector<API_AddParID>	vec_empty_type = vector<API_AddParID> (200, API_ZombieParT);

		this->var1type = vec_empty_type;
		this->var2type = vec_empty_type;
		this->var3type = vec_empty_type;
		this->var4type = vec_empty_type;
		this->var5type = vec_empty_type;
		this->var6type = vec_empty_type;
		this->var7type = vec_empty_type;
		this->var8type = vec_empty_type;
		this->var9type = vec_empty_type;

		this->nCounts = vec_empty_short;
		this->nCountsBeam = 0;
		this->nUnknownObjects = 0;

		for (xx = 0 ; xx < lineCount-1 ; ++xx) {
			this->var1value.push_back (vec_empty_string);
			this->var2value.push_back (vec_empty_string);
			this->var3value.push_back (vec_empty_string);
			this->var4value.push_back (vec_empty_string);
			this->var5value.push_back (vec_empty_string);
			this->var6value.push_back (vec_empty_string);
			this->var7value.push_back (vec_empty_string);
			this->var8value.push_back (vec_empty_string);
			this->var9value.push_back (vec_empty_string);
			this->combinationCount.push_back (vec_empty_short);
		}
	}
}

// 선택한 부재 정보 내보내기
GSErrCode	exportSelectedElementInfo (void)
{
	GSErrCode	err = NoError;
	long		nSel;
	short		xx, yy, zz;
	bool		regenerate = true;

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
	bool				foundExistValue;

	SummaryOfObjectInfo		objectInfo;

	//API_NewWindowPars	windowPars;
	//API_WindowInfo		windowInfo;
	char				buffer [256];
	char				filename [256];

	//BNZeroMemory (&windowPars, sizeof (API_NewWindowPars));
	//windowPars.typeID = APIWind_MyTextID;
	//windowPars.userRefCon = 1;
	//GS::snuprintf (windowPars.wTitle, sizeof (windowPars.wTitle) / sizeof (GS::uchar_t), L("선택한 객체들의 정보"));
	//err = ACAPI_Database (APIDb_NewWindowID, &windowPars, NULL);	// 텍스트 창 열기

	// 엑셀 파일로 기둥 정보 내보내기
	// 파일 저장을 위한 변수
	API_SpecFolderID	specFolderID = API_ApplicationFolderID;
	IO::Location		location;
	GS::UniString		resultString;
	API_MiscAppInfo		miscAppInfo;
	FILE				*fp;

	ACAPI_Environment (APIEnv_GetMiscAppInfoID, &miscAppInfo);
	sprintf (filename, "%s - 선택한 부재 정보.csv", miscAppInfo.caption);
	fp = fopen (filename, "w+");

	if (fp == NULL)
		return err;

	double			value_numeric [9];
	string			value_string [9];
	API_AddParID	value_type [9];
	char			tempStr [50];
	const char*		foundStr;
	bool			foundObject;

	for (xx = 0 ; xx < nObjects ; ++xx) {
		foundObject = false;

		BNZeroMemory (&elem, sizeof (API_Element));
		BNZeroMemory (&memo, sizeof (API_ElementMemo));
		elem.header.guid = objects.Pop ();
		err = ACAPI_Element_Get (&elem);
		err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

		// !!! 객체의 원점 수집하기 ==================================
		API_Coord3D	coord;

		coord.x = elem.object.pos.x;
		coord.y = elem.object.pos.y;
		coord.z = elem.object.level;
		
		vecPos.push_back (coord);
		// !!! 객체의 원점 수집하기 ==================================

		// 파라미터 스크립트를 강제로 실행시킴
		ACAPI_Goodies (APIAny_RunGDLParScriptID, &elem.header, 0);

		for (yy = 0 ; yy < objectInfo.nameKey.size () ; ++yy) {

			strcpy (tempStr, objectInfo.nameKey [yy].c_str ());
			foundStr = getParameterStringByName (&memo, tempStr);

			// 객체 종류를 찾았다면,
			if (foundStr != NULL) {
				foundObject = true;

				if (strncmp (foundStr, objectInfo.nameVal [yy].c_str (), strlen (objectInfo.nameVal [yy].c_str ())) == 0) {
					foundExistValue = false;

					value_string [0] = "";
					value_string [1] = "";
					value_string [2] = "";
					value_string [3] = "";
					value_string [4] = "";
					value_string [5] = "";
					value_string [6] = "";
					value_string [7] = "";
					value_string [8] = "";

					// 변수 1
					strcpy (tempStr, objectInfo.var1name [yy].c_str ());
					value_type [0] = getParameterTypeByName (&memo, tempStr);
					objectInfo.var1type [yy] = value_type [0];

					if ((value_type [0] != APIParT_Separator) || (value_type [0] != APIParT_Title) || (value_type [0] != API_ZombieParT)) {
						if (value_type [0] == APIParT_CString) {
							// 문자열
							value_string [0] = getParameterStringByName (&memo, tempStr);
							value_numeric [0] = 0.0;
						} else {
							// 숫자
							value_numeric [0] = getParameterValueByName (&memo, tempStr);
							sprintf (tempStr, "%f", value_numeric [0]);
							value_string [0] = tempStr;
						}
					}

					// 변수 2
					strcpy (tempStr, objectInfo.var2name [yy].c_str ());
					value_type [1] = getParameterTypeByName (&memo, tempStr);
					objectInfo.var2type [yy] = value_type [1];

					if ((value_type [1] != APIParT_Separator) || (value_type [1] != APIParT_Title) || (value_type [1] != API_ZombieParT)) {
						if (value_type [1] == APIParT_CString) {
							// 문자열
							value_string [1] = getParameterStringByName (&memo, tempStr);
							value_numeric [1] = 0.0;
						} else {
							// 숫자
							value_numeric [1] = getParameterValueByName (&memo, tempStr);
							sprintf (tempStr, "%f", value_numeric [1]);
							value_string [1] = tempStr;
						}
					}

					// 변수 3
					strcpy (tempStr, objectInfo.var3name [yy].c_str ());
					value_type [2] = getParameterTypeByName (&memo, tempStr);
					objectInfo.var3type [yy] = value_type [2];

					if ((value_type [2] != APIParT_Separator) || (value_type [2] != APIParT_Title) || (value_type [2] != API_ZombieParT)) {
						if (value_type [2] == APIParT_CString) {
							// 문자열
							value_string [2] = getParameterStringByName (&memo, tempStr);
							value_numeric [2] = 0.0;
						} else {
							// 숫자
							value_numeric [2] = getParameterValueByName (&memo, tempStr);
							sprintf (tempStr, "%f", value_numeric [2]);
							value_string [2] = tempStr;
						}
					}

					// 변수 4
					strcpy (tempStr, objectInfo.var4name [yy].c_str ());
					value_type [3] = getParameterTypeByName (&memo, tempStr);
					objectInfo.var4type [yy] = value_type [3];

					if ((value_type [3] != APIParT_Separator) || (value_type [3] != APIParT_Title) || (value_type [3] != API_ZombieParT)) {
						if (value_type [3] == APIParT_CString) {
							// 문자열
							value_string [3] = getParameterStringByName (&memo, tempStr);
							value_numeric [3] = 0.0;
						} else {
							// 숫자
							value_numeric [3] = getParameterValueByName (&memo, tempStr);
							sprintf (tempStr, "%f", value_numeric [3]);
							value_string [3] = tempStr;
						}
					}

					// 변수 5
					strcpy (tempStr, objectInfo.var5name [yy].c_str ());
					value_type [4] = getParameterTypeByName (&memo, tempStr);
					objectInfo.var5type [yy] = value_type [4];

					if ((value_type [4] != APIParT_Separator) || (value_type [4] != APIParT_Title) || (value_type [4] != API_ZombieParT)) {
						if (value_type [4] == APIParT_CString) {
							// 문자열
							value_string [4] = getParameterStringByName (&memo, tempStr);
							value_numeric [4] = 0.0;
						} else {
							// 숫자
							value_numeric [4] = getParameterValueByName (&memo, tempStr);
							sprintf (tempStr, "%f", value_numeric [4]);
							value_string [4] = tempStr;
						}
					}

					// 변수 6
					strcpy (tempStr, objectInfo.var6name [yy].c_str ());
					value_type [5] = getParameterTypeByName (&memo, tempStr);
					objectInfo.var6type [yy] = value_type [5];

					if ((value_type [5] != APIParT_Separator) || (value_type [5] != APIParT_Title) || (value_type [5] != API_ZombieParT)) {
						if (value_type [5] == APIParT_CString) {
							// 문자열
							value_string [5] = getParameterStringByName (&memo, tempStr);
							value_numeric [5] = 0.0;
						} else {
							// 숫자
							value_numeric [5] = getParameterValueByName (&memo, tempStr);
							sprintf (tempStr, "%f", value_numeric [5]);
							value_string [5] = tempStr;
						}
					}

					// 변수 7
					strcpy (tempStr, objectInfo.var7name [yy].c_str ());
					value_type [6] = getParameterTypeByName (&memo, tempStr);
					objectInfo.var7type [yy] = value_type [6];

					if ((value_type [6] != APIParT_Separator) || (value_type [6] != APIParT_Title) || (value_type [6] != API_ZombieParT)) {
						if (value_type [6] == APIParT_CString) {
							// 문자열
							value_string [6] = getParameterStringByName (&memo, tempStr);
							value_numeric [6] = 0.0;
						} else {
							// 숫자
							value_numeric [6] = getParameterValueByName (&memo, tempStr);
							sprintf (tempStr, "%f", value_numeric [6]);
							value_string [6] = tempStr;
						}
					}

					// 변수 8
					strcpy (tempStr, objectInfo.var8name [yy].c_str ());
					value_type [7] = getParameterTypeByName (&memo, tempStr);
					objectInfo.var8type [yy] = value_type [7];

					if ((value_type [7] != APIParT_Separator) || (value_type [7] != APIParT_Title) || (value_type [7] != API_ZombieParT)) {
						if (value_type [7] == APIParT_CString) {
							// 문자열
							value_string [7] = getParameterStringByName (&memo, tempStr);
							value_numeric [7] = 0.0;
						} else {
							// 숫자
							value_numeric [7] = getParameterValueByName (&memo, tempStr);
							sprintf (tempStr, "%f", value_numeric [7]);
							value_string [7] = tempStr;
						}
					}

					// 변수 9
					strcpy (tempStr, objectInfo.var9name [yy].c_str ());
					value_type [8] = getParameterTypeByName (&memo, tempStr);
					objectInfo.var9type [yy] = value_type [8];

					if ((value_type [8] != APIParT_Separator) || (value_type [8] != APIParT_Title) || (value_type [8] != API_ZombieParT)) {
						if (value_type [8] == APIParT_CString) {
							// 문자열
							value_string [8] = getParameterStringByName (&memo, tempStr);
							value_numeric [8] = 0.0;
						} else {
							// 숫자
							value_numeric [8] = getParameterValueByName (&memo, tempStr);
							sprintf (tempStr, "%f", value_numeric [8]);
							value_string [8] = tempStr;
						}
					}

					// 중복 항목은 개수만 증가
					for (zz = 0 ; zz < objectInfo.nCounts [yy] ; ++zz) {
						if ((objectInfo.var1value [yy][zz].compare (value_string [0]) == 0) && (objectInfo.var2value [yy][zz].compare (value_string [1]) == 0) && (objectInfo.var3value [yy][zz].compare (value_string [2]) == 0) && 
							(objectInfo.var4value [yy][zz].compare (value_string [3]) == 0) && (objectInfo.var5value [yy][zz].compare (value_string [4]) == 0) && (objectInfo.var6value [yy][zz].compare (value_string [5]) == 0) && 
							(objectInfo.var7value [yy][zz].compare (value_string [6]) == 0) && (objectInfo.var8value [yy][zz].compare (value_string [7]) == 0) && (objectInfo.var9value [yy][zz].compare (value_string [8]) == 0)) {

								objectInfo.combinationCount [yy][zz] ++;
								foundExistValue = true;
								break;
						}
					}

					// 신규 항목이면
					if (!foundExistValue) {
						objectInfo.var1value [yy][objectInfo.nCounts [yy]] = value_string [0];
						objectInfo.var2value [yy][objectInfo.nCounts [yy]] = value_string [1];
						objectInfo.var3value [yy][objectInfo.nCounts [yy]] = value_string [2];
						objectInfo.var4value [yy][objectInfo.nCounts [yy]] = value_string [3];
						objectInfo.var5value [yy][objectInfo.nCounts [yy]] = value_string [4];
						objectInfo.var6value [yy][objectInfo.nCounts [yy]] = value_string [5];
						objectInfo.var7value [yy][objectInfo.nCounts [yy]] = value_string [6];
						objectInfo.var8value [yy][objectInfo.nCounts [yy]] = value_string [7];
						objectInfo.var9value [yy][objectInfo.nCounts [yy]] = value_string [8];
						objectInfo.combinationCount [yy][objectInfo.nCounts [yy]] = 1;
						objectInfo.nCounts [yy] ++;
					}
				}
			}
		}

		// 끝내 찾지 못하면 알 수 없는 객체로 취급함
		if (foundObject == false)
			objectInfo.nUnknownObjects ++;

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
		for (zz = 0 ; zz < objectInfo.nCountsBeam ; ++zz) {
			if (objectInfo.beamLength [zz] == len) {
				objectInfo.beamCount [zz] ++;
				foundExistValue = true;
				break;
			}
		}

		// 신규 항목 추가하고 개수도 증가
		if ( !foundExistValue ) {
			objectInfo.beamLength.push_back (len);
			objectInfo.beamCount.push_back (1);
			objectInfo.nCountsBeam ++;
		}

		ACAPI_DisposeElemMemoHdls (&memo);
	}

	// 최종 텍스트 표시
	//BNZeroMemory (&windowInfo, sizeof (API_WindowInfo));
	//windowInfo.typeID = APIWind_MyTextID;
	//windowInfo.index = 1;

	// APIParT_Length인 경우 1000배 곱해서 표현
	// APIParT_Boolean인 경우 예/아니오 표현
	double	length, length2, length3;
	bool	bShow;

	for (xx = 0 ; xx < objectInfo.nKnownObjects ; ++xx) {
		for (yy = 0 ; yy < objectInfo.nCounts [xx] ; ++yy) {
			// 제목
			if (yy == 0) {
				sprintf (buffer, "\n[%s]\n", objectInfo.nameVal [xx].c_str ());
				//ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
				fprintf (fp, buffer);
			}

			if (strncmp (objectInfo.nameVal [xx].c_str (), "유로폼 후크", strlen ("유로폼 후크")) == 0) {
				// 원형
				if (strncmp (objectInfo.var2value [xx][yy].c_str (), "원형", strlen ("원형")) == 0) {
					sprintf (buffer, "원형, %s ", objectInfo.var1value [xx][yy].c_str ());

				// 사각
				} else {
					sprintf (buffer, "사각, %s ", objectInfo.var1value [xx][yy].c_str ());
				}
				//ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
				fprintf (fp, buffer);

			} else if (strncmp (objectInfo.nameVal [xx].c_str (), "유로폼", strlen ("유로폼")) == 0) {
				// 규격폼
				if (atoi (objectInfo.var1value [xx][yy].c_str ()) > 0) {
					sprintf (buffer, "%s X %s ", objectInfo.var2value [xx][yy], objectInfo.var3value [xx][yy]);
				// 비규격폼
				} else {
					// 4열 X 5열
					length = atof (objectInfo.var4value [xx][yy].c_str ());
					length2 = atof (objectInfo.var5value [xx][yy].c_str ());
					sprintf (buffer, "%.0f X %.0f ", round (length*1000, 0), round (length2*1000, 0));
				}
				//ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
				fprintf (fp, buffer);

			} else if (strncmp (objectInfo.nameVal [xx].c_str (), "스틸폼", strlen ("스틸폼")) == 0) {
				// 규격폼
				if (atoi (objectInfo.var1value [xx][yy].c_str ()) > 0) {
					sprintf (buffer, "%s X %s ", objectInfo.var2value [xx][yy], objectInfo.var3value [xx][yy]);
				// 비규격폼
				} else {
					// 4열 X 5열
					length = atof (objectInfo.var4value [xx][yy].c_str ());
					length2 = atof (objectInfo.var5value [xx][yy].c_str ());
					sprintf (buffer, "%.0f X %.0f ", round (length*1000, 0), round (length2*1000, 0));
				}
				//ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
				fprintf (fp, buffer);

			} else if (strncmp (objectInfo.nameVal [xx].c_str (), "목재", strlen ("목재")) == 0) {
				length = atof (objectInfo.var1value [xx][yy].c_str ());
				length2 = atof (objectInfo.var2value [xx][yy].c_str ());
				length3 = atof (objectInfo.var3value [xx][yy].c_str ());
				sprintf (buffer, "%.0f X %.0f X %.0f ", round (length*1000, 0), round (length2*1000, 0), round (length3*1000, 0));
				//ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
				fprintf (fp, buffer);

			} else if (strncmp (objectInfo.nameVal [xx].c_str (), "합판", strlen ("합판")) == 0) {
				if (strncmp (objectInfo.var1value [xx][yy].c_str (), "3x6 [910x1820]", strlen ("3x6 [910x1820]")) == 0) {
					sprintf (buffer, "910 X 1820 X %s ", objectInfo.var2value [xx][yy].c_str ());
				} else if (strncmp (objectInfo.var1value [xx][yy].c_str (), "4x8 [1220x2440]", strlen ("4x8 [1220x2440]")) == 0) {
					sprintf (buffer, "1220 X 2440 X %s ", objectInfo.var2value [xx][yy].c_str ());
				} else if (strncmp (objectInfo.var1value [xx][yy].c_str (), "2x5 [606x1520]", strlen ("2x5 [606x1520]")) == 0) {
					sprintf (buffer, "606 X 1520 X %s ", objectInfo.var2value [xx][yy].c_str ());
				} else if (strncmp (objectInfo.var1value [xx][yy].c_str (), "2x6 [606x1820]", strlen ("2x6 [606x1820]")) == 0) {
					sprintf (buffer, "606 X 1820 X %s ", objectInfo.var2value [xx][yy].c_str ());
				} else if (strncmp (objectInfo.var1value [xx][yy].c_str (), "3x5 [910x1520]", strlen ("3x5 [910x1520]")) == 0) {
					sprintf (buffer, "910 X 1520 X %s ", objectInfo.var2value [xx][yy].c_str ());
				} else if (strncmp (objectInfo.var1value [xx][yy].c_str (), "비규격", strlen ("비규격")) == 0) {
					// 가로 X 세로 X 두께
					length = atof (objectInfo.var3value [xx][yy].c_str ());
					length2 = atof (objectInfo.var4value [xx][yy].c_str ());
					sprintf (buffer, "%.0f X %.0f X %s ", round (length*1000, 0), round (length2*1000, 0), objectInfo.var2value [xx][yy].c_str ());
				} else if (strncmp (objectInfo.var1value [xx][yy].c_str (), "비정형", strlen ("비정형")) == 0) {
					sprintf (buffer, "비정형 ");
				}
				//ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
				fprintf (fp, buffer);

				// 제작틀 ON
				if (atoi (objectInfo.var5value [xx][yy].c_str ()) > 0) {
					sprintf (buffer, "(각재 총길이: %s) ", objectInfo.var6value [xx][yy].c_str ());
					//ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
					fprintf (fp, buffer);
				}

			} else if (strncmp (objectInfo.nameVal [xx].c_str (), "사각파이프", strlen ("사각파이프")) == 0) {
				// 사각파이프
				if (atoi (objectInfo.var1value [xx][yy].c_str ()) == 0) {
					length = atof (objectInfo.var2value [xx][yy].c_str ());
					sprintf (buffer, "50 x 50 x %.0f ", round (length*1000, 0));

				// 직사각파이프
				} else {
					length = atof (objectInfo.var2value [xx][yy].c_str ());
					sprintf (buffer, "%s x %.0f ", objectInfo.var1value [xx][yy].c_str (), round (length*1000, 0));
				}
				//ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
				fprintf (fp, buffer);

			} else if (strncmp (objectInfo.nameVal [xx].c_str (), "원형파이프", strlen ("원형파이프")) == 0) {
				length = atof (objectInfo.var1value [xx][yy].c_str ());
				sprintf (buffer, "%.0f ", round (length*1000, 0));
				//ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
				fprintf (fp, buffer);

			} else if (strncmp (objectInfo.nameVal [xx].c_str (), "아웃코너앵글", strlen ("아웃코너앵글")) == 0) {
				length = atof (objectInfo.var1value [xx][yy].c_str ());
				sprintf (buffer, "%.0f ", round (length*1000, 0));
				//ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
				fprintf (fp, buffer);

			} else if (strncmp (objectInfo.nameVal [xx].c_str (), "매직바", strlen ("매직바")) == 0) {
				if (atoi (objectInfo.var2value [xx][yy].c_str ()) > 0) {
					length = atof (objectInfo.var1value [xx][yy].c_str ());
					length2 = atof (objectInfo.var5value [xx][yy].c_str ());
					sprintf (buffer, "%.0f, 합판 %.0f", round (length*1000, 0), round (length2*1000, 0));
				} else {
					length = atof (objectInfo.var1value [xx][yy].c_str ());
					sprintf (buffer, "%.0f ", round (length*1000, 0));
				}
				//ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
				fprintf (fp, buffer);

			} else if (strncmp (objectInfo.nameVal [xx].c_str (), "블루목심", strlen ("블루목심")) == 0) {
				sprintf (buffer, "%s ", objectInfo.var1value [xx][yy].c_str ());
				//ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
				fprintf (fp, buffer);

			} else if (strncmp (objectInfo.nameVal [xx].c_str (), "보 멍에제", strlen ("보 멍에제")) == 0) {
				length = atof (objectInfo.var1value [xx][yy].c_str ());
				sprintf (buffer, "%.0f ", round (length*1000, 0));
				//ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
				fprintf (fp, buffer);

			} else if (strncmp (objectInfo.nameVal [xx].c_str (), "물량합판", strlen ("물량합판")) == 0) {
				sprintf (buffer, "%s ㎡ ", objectInfo.var1value [xx][yy].c_str ());
				//ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
				fprintf (fp, buffer);

			} else {
				// 변수별 값 표현
				if (objectInfo.var1name [xx].size () > 1) {
					bShow = false;
					if (objectInfo.var1showFlag [xx] == 0)																bShow = true;
					if ((objectInfo.var1showFlag [xx] ==  2) && (atoi (objectInfo.var2value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var1showFlag [xx] == -2) && (atoi (objectInfo.var2value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var1showFlag [xx] ==  3) && (atoi (objectInfo.var3value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var1showFlag [xx] == -3) && (atoi (objectInfo.var3value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var1showFlag [xx] ==  4) && (atoi (objectInfo.var4value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var1showFlag [xx] == -4) && (atoi (objectInfo.var4value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var1showFlag [xx] ==  5) && (atoi (objectInfo.var5value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var1showFlag [xx] == -5) && (atoi (objectInfo.var5value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var1showFlag [xx] ==  6) && (atoi (objectInfo.var6value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var1showFlag [xx] == -6) && (atoi (objectInfo.var6value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var1showFlag [xx] ==  7) && (atoi (objectInfo.var7value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var1showFlag [xx] == -7) && (atoi (objectInfo.var7value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var1showFlag [xx] ==  8) && (atoi (objectInfo.var8value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var1showFlag [xx] == -8) && (atoi (objectInfo.var8value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var1showFlag [xx] ==  9) && (atoi (objectInfo.var9value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var1showFlag [xx] == -9) && (atoi (objectInfo.var9value [xx][yy].c_str ()) == 0))	bShow = true;

					if (objectInfo.var1type [xx] == APIParT_Length) {
						length = atof (objectInfo.var1value [xx][yy].c_str ());
						sprintf (buffer, "%s(%.0f) ", objectInfo.var1desc [xx].c_str (), round (length*1000, 0));
					} else if (objectInfo.var1type [xx] == APIParT_Boolean) {
						if (atoi (objectInfo.var1value [xx][yy].c_str ()) > 0) {
							sprintf (buffer, "%s(%s) ", objectInfo.var1desc [xx].c_str (), "예");
						} else {
							sprintf (buffer, "%s(%s) ", objectInfo.var1desc [xx].c_str (), "아니오");
						}
					} else {
						sprintf (buffer, "%s(%s) ", objectInfo.var1desc [xx].c_str (), objectInfo.var1value [xx][yy]);
					}
					if (bShow) fprintf (fp, buffer);	//ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
				}
				if (objectInfo.var2name [xx].size () > 1) {
					bShow = false;
					if (objectInfo.var2showFlag [xx] == 0)																bShow = true;
					if ((objectInfo.var2showFlag [xx] ==  1) && (atoi (objectInfo.var1value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var2showFlag [xx] == -1) && (atoi (objectInfo.var1value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var2showFlag [xx] ==  3) && (atoi (objectInfo.var3value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var2showFlag [xx] == -3) && (atoi (objectInfo.var3value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var2showFlag [xx] ==  4) && (atoi (objectInfo.var4value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var2showFlag [xx] == -4) && (atoi (objectInfo.var4value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var2showFlag [xx] ==  5) && (atoi (objectInfo.var5value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var2showFlag [xx] == -5) && (atoi (objectInfo.var5value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var2showFlag [xx] ==  6) && (atoi (objectInfo.var6value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var2showFlag [xx] == -6) && (atoi (objectInfo.var6value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var2showFlag [xx] ==  7) && (atoi (objectInfo.var7value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var2showFlag [xx] == -7) && (atoi (objectInfo.var7value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var2showFlag [xx] ==  8) && (atoi (objectInfo.var8value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var2showFlag [xx] == -8) && (atoi (objectInfo.var8value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var2showFlag [xx] ==  9) && (atoi (objectInfo.var9value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var2showFlag [xx] == -9) && (atoi (objectInfo.var9value [xx][yy].c_str ()) == 0))	bShow = true;

					if (objectInfo.var2type [xx] == APIParT_Length) {
						length = atof (objectInfo.var2value [xx][yy].c_str ());
						sprintf (buffer, "%s(%.0f) ", objectInfo.var2desc [xx].c_str (), round (length*1000, 0));
					} else if (objectInfo.var2type [xx] == APIParT_Boolean) {
						if (atoi (objectInfo.var2value [xx][yy].c_str ()) > 0) {
							sprintf (buffer, "%s(%s) ", objectInfo.var2desc [xx].c_str (), "예");
						} else {
							sprintf (buffer, "%s(%s) ", objectInfo.var2desc [xx].c_str (), "아니오");
						}
					} else {
						sprintf (buffer, "%s(%s) ", objectInfo.var2desc [xx].c_str (), objectInfo.var2value [xx][yy]);
					}
					if (bShow) fprintf (fp, buffer);	//ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
				}
				if (objectInfo.var3name [xx].size () > 1) {
					bShow = false;
					if (objectInfo.var3showFlag [xx] == 0)																bShow = true;
					if ((objectInfo.var3showFlag [xx] ==  1) && (atoi (objectInfo.var1value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var3showFlag [xx] == -1) && (atoi (objectInfo.var1value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var3showFlag [xx] ==  2) && (atoi (objectInfo.var2value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var3showFlag [xx] == -2) && (atoi (objectInfo.var2value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var3showFlag [xx] ==  4) && (atoi (objectInfo.var4value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var3showFlag [xx] == -4) && (atoi (objectInfo.var4value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var3showFlag [xx] ==  5) && (atoi (objectInfo.var5value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var3showFlag [xx] == -5) && (atoi (objectInfo.var5value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var3showFlag [xx] ==  6) && (atoi (objectInfo.var6value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var3showFlag [xx] == -6) && (atoi (objectInfo.var6value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var3showFlag [xx] ==  7) && (atoi (objectInfo.var7value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var3showFlag [xx] == -7) && (atoi (objectInfo.var7value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var3showFlag [xx] ==  8) && (atoi (objectInfo.var8value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var3showFlag [xx] == -8) && (atoi (objectInfo.var8value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var3showFlag [xx] ==  9) && (atoi (objectInfo.var9value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var3showFlag [xx] == -9) && (atoi (objectInfo.var9value [xx][yy].c_str ()) == 0))	bShow = true;

					if (objectInfo.var3type [xx] == APIParT_Length) {
						length = atof (objectInfo.var3value [xx][yy].c_str ());
						sprintf (buffer, "%s(%.0f) ", objectInfo.var3desc [xx].c_str (), round (length*1000, 0));
					} else if (objectInfo.var3type [xx] == APIParT_Boolean) {
						if (atoi (objectInfo.var3value [xx][yy].c_str ()) > 0) {
							sprintf (buffer, "%s(%s) ", objectInfo.var3desc [xx].c_str (), "예");
						} else {
							sprintf (buffer, "%s(%s) ", objectInfo.var3desc [xx].c_str (), "아니오");
						}
					} else {
						sprintf (buffer, "%s(%s) ", objectInfo.var3desc [xx].c_str (), objectInfo.var3value [xx][yy]);
					}
					if (bShow) fprintf (fp, buffer);	//ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
				}
				if (objectInfo.var4name [xx].size () > 1) {
					bShow = false;
					if (objectInfo.var4showFlag [xx] == 0)																bShow = true;
					if ((objectInfo.var4showFlag [xx] ==  1) && (atoi (objectInfo.var1value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var4showFlag [xx] == -1) && (atoi (objectInfo.var1value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var4showFlag [xx] ==  2) && (atoi (objectInfo.var2value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var4showFlag [xx] == -2) && (atoi (objectInfo.var2value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var4showFlag [xx] ==  3) && (atoi (objectInfo.var3value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var4showFlag [xx] == -3) && (atoi (objectInfo.var3value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var4showFlag [xx] ==  5) && (atoi (objectInfo.var5value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var4showFlag [xx] == -5) && (atoi (objectInfo.var5value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var4showFlag [xx] ==  6) && (atoi (objectInfo.var6value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var4showFlag [xx] == -6) && (atoi (objectInfo.var6value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var4showFlag [xx] ==  7) && (atoi (objectInfo.var7value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var4showFlag [xx] == -7) && (atoi (objectInfo.var7value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var4showFlag [xx] ==  8) && (atoi (objectInfo.var8value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var4showFlag [xx] == -8) && (atoi (objectInfo.var8value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var4showFlag [xx] ==  9) && (atoi (objectInfo.var9value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var4showFlag [xx] == -9) && (atoi (objectInfo.var9value [xx][yy].c_str ()) == 0))	bShow = true;

					if (objectInfo.var4type [xx] == APIParT_Length) {
						length = atof (objectInfo.var4value [xx][yy].c_str ());
						sprintf (buffer, "%s(%.0f) ", objectInfo.var4desc [xx].c_str (), round (length*1000, 0));
					} else if (objectInfo.var4type [xx] == APIParT_Boolean) {
						if (atoi (objectInfo.var4value [xx][yy].c_str ()) > 0) {
							sprintf (buffer, "%s(%s) ", objectInfo.var4desc [xx].c_str (), "예");
						} else {
							sprintf (buffer, "%s(%s) ", objectInfo.var4desc [xx].c_str (), "아니오");
						}
					} else {
						sprintf (buffer, "%s(%s) ", objectInfo.var4desc [xx].c_str (), objectInfo.var4value [xx][yy]);
					}
					if (bShow) fprintf (fp, buffer);	//ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
				}
				if (objectInfo.var5name [xx].size () > 1) {
					bShow = false;
					if (objectInfo.var5showFlag [xx] == 0)																bShow = true;
					if ((objectInfo.var5showFlag [xx] ==  1) && (atoi (objectInfo.var1value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var5showFlag [xx] == -1) && (atoi (objectInfo.var1value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var5showFlag [xx] ==  2) && (atoi (objectInfo.var2value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var5showFlag [xx] == -2) && (atoi (objectInfo.var2value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var5showFlag [xx] ==  3) && (atoi (objectInfo.var3value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var5showFlag [xx] == -3) && (atoi (objectInfo.var3value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var5showFlag [xx] ==  4) && (atoi (objectInfo.var4value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var5showFlag [xx] == -4) && (atoi (objectInfo.var4value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var5showFlag [xx] ==  6) && (atoi (objectInfo.var6value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var5showFlag [xx] == -6) && (atoi (objectInfo.var6value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var5showFlag [xx] ==  7) && (atoi (objectInfo.var7value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var5showFlag [xx] == -7) && (atoi (objectInfo.var7value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var5showFlag [xx] ==  8) && (atoi (objectInfo.var8value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var5showFlag [xx] == -8) && (atoi (objectInfo.var8value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var5showFlag [xx] ==  9) && (atoi (objectInfo.var9value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var5showFlag [xx] == -9) && (atoi (objectInfo.var9value [xx][yy].c_str ()) == 0))	bShow = true;

					if (objectInfo.var5type [xx] == APIParT_Length) {
						length = atof (objectInfo.var5value [xx][yy].c_str ());
						sprintf (buffer, "%s(%.0f) ", objectInfo.var5desc [xx].c_str (), round (length*1000, 0));
					} else if (objectInfo.var5type [xx] == APIParT_Boolean) {
						if (atoi (objectInfo.var5value [xx][yy].c_str ()) > 0) {
							sprintf (buffer, "%s(%s) ", objectInfo.var5desc [xx].c_str (), "예");
						} else {
							sprintf (buffer, "%s(%s) ", objectInfo.var5desc [xx].c_str (), "아니오");
						}
					} else {
						sprintf (buffer, "%s(%s) ", objectInfo.var5desc [xx].c_str (), objectInfo.var5value [xx][yy]);
					}
					if (bShow) fprintf (fp, buffer);	//ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
				}
				if (objectInfo.var6name [xx].size () > 1) {
					bShow = false;
					if (objectInfo.var6showFlag [xx] == 0)																bShow = true;
					if ((objectInfo.var6showFlag [xx] ==  1) && (atoi (objectInfo.var1value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var6showFlag [xx] == -1) && (atoi (objectInfo.var1value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var6showFlag [xx] ==  2) && (atoi (objectInfo.var2value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var6showFlag [xx] == -2) && (atoi (objectInfo.var2value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var6showFlag [xx] ==  3) && (atoi (objectInfo.var3value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var6showFlag [xx] == -3) && (atoi (objectInfo.var3value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var6showFlag [xx] ==  4) && (atoi (objectInfo.var4value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var6showFlag [xx] == -4) && (atoi (objectInfo.var4value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var6showFlag [xx] ==  5) && (atoi (objectInfo.var5value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var6showFlag [xx] == -5) && (atoi (objectInfo.var5value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var6showFlag [xx] ==  7) && (atoi (objectInfo.var7value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var6showFlag [xx] == -7) && (atoi (objectInfo.var7value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var6showFlag [xx] ==  8) && (atoi (objectInfo.var8value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var6showFlag [xx] == -8) && (atoi (objectInfo.var8value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var6showFlag [xx] ==  9) && (atoi (objectInfo.var9value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var6showFlag [xx] == -9) && (atoi (objectInfo.var9value [xx][yy].c_str ()) == 0))	bShow = true;

					if (objectInfo.var6type [xx] == APIParT_Length) {
						length = atof (objectInfo.var6value [xx][yy].c_str ());
						sprintf (buffer, "%s(%.0f) ", objectInfo.var6desc [xx].c_str (), round (length*1000, 0));
					} else if (objectInfo.var6type [xx] == APIParT_Boolean) {
						if (atoi (objectInfo.var6value [xx][yy].c_str ()) > 0) {
							sprintf (buffer, "%s(%s) ", objectInfo.var6desc [xx].c_str (), "예");
						} else {
							sprintf (buffer, "%s(%s) ", objectInfo.var6desc [xx].c_str (), "아니오");
						}
					} else {
						sprintf (buffer, "%s(%s) ", objectInfo.var6desc [xx].c_str (), objectInfo.var6value [xx][yy]);
					}
					if (bShow) fprintf (fp, buffer);	//ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
				}
				if (objectInfo.var7name [xx].size () > 1) {
					bShow = false;
					if (objectInfo.var7showFlag [xx] == 0)																bShow = true;
					if ((objectInfo.var7showFlag [xx] ==  1) && (atoi (objectInfo.var1value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var7showFlag [xx] == -1) && (atoi (objectInfo.var1value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var7showFlag [xx] ==  2) && (atoi (objectInfo.var2value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var7showFlag [xx] == -2) && (atoi (objectInfo.var2value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var7showFlag [xx] ==  3) && (atoi (objectInfo.var3value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var7showFlag [xx] == -3) && (atoi (objectInfo.var3value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var7showFlag [xx] ==  4) && (atoi (objectInfo.var4value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var7showFlag [xx] == -4) && (atoi (objectInfo.var4value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var7showFlag [xx] ==  5) && (atoi (objectInfo.var5value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var7showFlag [xx] == -5) && (atoi (objectInfo.var5value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var7showFlag [xx] ==  6) && (atoi (objectInfo.var6value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var7showFlag [xx] == -6) && (atoi (objectInfo.var6value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var7showFlag [xx] ==  8) && (atoi (objectInfo.var8value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var7showFlag [xx] == -8) && (atoi (objectInfo.var8value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var7showFlag [xx] ==  9) && (atoi (objectInfo.var9value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var7showFlag [xx] == -9) && (atoi (objectInfo.var9value [xx][yy].c_str ()) == 0))	bShow = true;

					if (objectInfo.var7type [xx] == APIParT_Length) {
						length = atof (objectInfo.var7value [xx][yy].c_str ());
						sprintf (buffer, "%s(%.0f) ", objectInfo.var7desc [xx].c_str (), round (length*1000, 0));
					} else if (objectInfo.var7type [xx] == APIParT_Boolean) {
						if (atoi (objectInfo.var7value [xx][yy].c_str ()) > 0) {
							sprintf (buffer, "%s(%s) ", objectInfo.var7desc [xx].c_str (), "예");
						} else {
							sprintf (buffer, "%s(%s) ", objectInfo.var7desc [xx].c_str (), "아니오");
						}
					} else {
						sprintf (buffer, "%s(%s) ", objectInfo.var7desc [xx].c_str (), objectInfo.var7value [xx][yy]);
					}
					if (bShow) fprintf (fp, buffer);	//ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
				}
				if (objectInfo.var8name [xx].size () > 1) {
					bShow = false;
					if (objectInfo.var8showFlag [xx] == 0)																bShow = true;
					if ((objectInfo.var8showFlag [xx] ==  1) && (atoi (objectInfo.var1value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var8showFlag [xx] == -1) && (atoi (objectInfo.var1value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var8showFlag [xx] ==  2) && (atoi (objectInfo.var2value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var8showFlag [xx] == -2) && (atoi (objectInfo.var2value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var8showFlag [xx] ==  3) && (atoi (objectInfo.var3value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var8showFlag [xx] == -3) && (atoi (objectInfo.var3value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var8showFlag [xx] ==  4) && (atoi (objectInfo.var4value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var8showFlag [xx] == -4) && (atoi (objectInfo.var4value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var8showFlag [xx] ==  5) && (atoi (objectInfo.var5value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var8showFlag [xx] == -5) && (atoi (objectInfo.var5value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var8showFlag [xx] ==  6) && (atoi (objectInfo.var6value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var8showFlag [xx] == -6) && (atoi (objectInfo.var6value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var8showFlag [xx] ==  7) && (atoi (objectInfo.var7value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var8showFlag [xx] == -7) && (atoi (objectInfo.var7value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var8showFlag [xx] ==  9) && (atoi (objectInfo.var9value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var8showFlag [xx] == -9) && (atoi (objectInfo.var9value [xx][yy].c_str ()) == 0))	bShow = true;

					if (objectInfo.var8type [xx] == APIParT_Length) {
						length = atof (objectInfo.var8value [xx][yy].c_str ());
						sprintf (buffer, "%s(%.0f) ", objectInfo.var8desc [xx].c_str (), round (length*1000, 0));
					} else if (objectInfo.var8type [xx] == APIParT_Boolean) {
						if (atoi (objectInfo.var8value [xx][yy].c_str ()) > 0) {
							sprintf (buffer, "%s(%s) ", objectInfo.var8desc [xx].c_str (), "예");
						} else {
							sprintf (buffer, "%s(%s) ", objectInfo.var8desc [xx].c_str (), "아니오");
						}
					} else {
						sprintf (buffer, "%s(%s) ", objectInfo.var8desc [xx].c_str (), objectInfo.var8value [xx][yy]);
					}
					if (bShow) fprintf (fp, buffer);	//ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
				}
				if (objectInfo.var9name [xx].size () > 1) {
					bShow = false;
					if (objectInfo.var9showFlag [xx] == 0)																bShow = true;
					if ((objectInfo.var9showFlag [xx] ==  1) && (atoi (objectInfo.var1value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var9showFlag [xx] == -1) && (atoi (objectInfo.var1value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var9showFlag [xx] ==  2) && (atoi (objectInfo.var2value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var9showFlag [xx] == -2) && (atoi (objectInfo.var2value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var9showFlag [xx] ==  3) && (atoi (objectInfo.var3value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var9showFlag [xx] == -3) && (atoi (objectInfo.var3value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var9showFlag [xx] ==  4) && (atoi (objectInfo.var4value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var9showFlag [xx] == -4) && (atoi (objectInfo.var4value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var9showFlag [xx] ==  5) && (atoi (objectInfo.var5value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var9showFlag [xx] == -5) && (atoi (objectInfo.var5value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var9showFlag [xx] ==  6) && (atoi (objectInfo.var6value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var9showFlag [xx] == -6) && (atoi (objectInfo.var6value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var9showFlag [xx] ==  7) && (atoi (objectInfo.var7value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var9showFlag [xx] == -7) && (atoi (objectInfo.var7value [xx][yy].c_str ()) == 0))	bShow = true;
					if ((objectInfo.var9showFlag [xx] ==  8) && (atoi (objectInfo.var8value [xx][yy].c_str ()) == 1))	bShow = true;
					if ((objectInfo.var9showFlag [xx] == -8) && (atoi (objectInfo.var8value [xx][yy].c_str ()) == 0))	bShow = true;

					if (objectInfo.var9type [xx] == APIParT_Length) {
						length = atof (objectInfo.var9value [xx][yy].c_str ());
						sprintf (buffer, "%s(%.0f) ", objectInfo.var9desc [xx].c_str (), round (length*1000, 0));
					} else if (objectInfo.var9type [xx] == APIParT_Boolean) {
						if (atoi (objectInfo.var9value [xx][yy].c_str ()) > 0) {
							sprintf (buffer, "%s(%s) ", objectInfo.var9desc [xx].c_str (), "예");
						} else {
							sprintf (buffer, "%s(%s) ", objectInfo.var9desc [xx].c_str (), "아니오");
						}
					} else {
						sprintf (buffer, "%s(%s) ", objectInfo.var9desc [xx].c_str (), objectInfo.var9value [xx][yy]);
					}
					if (bShow) fprintf (fp, buffer);	//ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
				}
			}

			// 수량 표현
			if (objectInfo.combinationCount [xx][yy] > 0) {
				sprintf (buffer, ": %d EA", objectInfo.combinationCount [xx][yy]);
				//ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
				fprintf (fp, buffer);
			}

			// 줄넘김
			//ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, "\n");
			fprintf (fp, "\n");
		}
	}

	// 일반 요소 - 보
	for (xx = 0 ; xx < objectInfo.nCountsBeam ; ++xx) {
		if (xx == 0) {
			//ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, "\n[보]\n");
			fprintf (fp, "\n[보]\n");
		}
		sprintf (buffer, "%d : %d EA\n", objectInfo.beamLength [xx], objectInfo.beamCount [xx]);
		//ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
		fprintf (fp, buffer);
	}

	// 알 수 없는 객체
	if (objectInfo.nUnknownObjects > 0) {
		sprintf (buffer, "\n알 수 없는 객체 : %d EA", objectInfo.nUnknownObjects);
		//ACAPI_Database (APIDb_AddTextWindowContentID, &windowInfo, buffer);
		fprintf (fp, buffer);
	}

	fclose (fp);

	ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);
	location.ToDisplayText (&resultString);
	sprintf (buffer, "%s를 다음 위치에 저장했습니다.\n\n%s\n또는 프로젝트 파일이 있는 폴더", filename, resultString.ToCStr ().Get ());
	ACAPI_WriteReport (buffer, true);

	// !!! 3D 투영 정보 ==================================
	API_3DProjectionInfo  proj3DInfo;

	BNZeroMemory (&proj3DInfo, sizeof (API_3DProjectionInfo));
	err = ACAPI_Environment (APIEnv_Get3DProjectionSetsID, &proj3DInfo, NULL);

	double	lowestZ, highestZ, cameraZ;		// 가장 낮은 높이, 가장 높은 높이, 카메라 및 대상 높이
	API_Coord3D		p1, p2;					// 서로 가장 멀리 떨어져 있는 두 점
	double	distanceOfPoints;				// 두 점 간의 거리
	double	angleOfPoints;					// 두 점 간의 각도
	API_Coord3D		camPos1, camPos2;		// 카메라가 있을 수 있는 점 2개

	API_FileSavePars		fsp;			// 파일 저장을 위한 변수
	API_SavePars_Picture	pars_pict;		// 그림 파일에 대한 설명

	if (err == NoError && proj3DInfo.isPersp) {
		// 높이 값의 범위를 구함
		// 가장 작은 x값을 갖는 점 p1도 찾아냄
		lowestZ = highestZ = vecPos [0].z;
		p1 = vecPos [0];
		for (xx = 1 ; xx < vecPos.size () ; ++xx) {
			if (lowestZ > vecPos [xx].z)	lowestZ = vecPos [xx].z;
			if (highestZ < vecPos [xx].z)	highestZ = vecPos [xx].z;

			if (vecPos [xx].x < p1.x)	p1 = vecPos [xx];
		}
		cameraZ = (highestZ - lowestZ) * 0.5;

		distanceOfPoints = 0.0;
		for (xx = 0 ; xx < vecPos.size () ; ++xx) {
			if (distanceOfPoints < GetDistance (p1, vecPos [xx])) {
				distanceOfPoints = GetDistance (p1, vecPos [xx]);
				p2 = vecPos [xx];
			}
		}

		// 두 점(p1, p2) 간의 각도 구하기
		angleOfPoints = RadToDegree (atan2 ((p2.y - p1.y), (p2.x - p1.x)));

		// 카메라와 대상이 있을 수 있는 위치 2개를 찾음
		camPos1 = p1;
		moveIn3D ('x', DegreeToRad (angleOfPoints), distanceOfPoints/2, &camPos1.x, &camPos1.y, &camPos1.z);
		if (distanceOfPoints > (highestZ - lowestZ))
			moveIn3D ('y', DegreeToRad (angleOfPoints), -distanceOfPoints * 1.5, &camPos1.x, &camPos1.y, &camPos1.z);
		else
			moveIn3D ('y', DegreeToRad (angleOfPoints), -(highestZ - lowestZ) * 1.5, &camPos1.x, &camPos1.y, &camPos1.z);
		camPos2 = p1;
		moveIn3D ('x', DegreeToRad (angleOfPoints), distanceOfPoints/2, &camPos2.x, &camPos2.y, &camPos2.z);
		if (distanceOfPoints > (highestZ - lowestZ))
			moveIn3D ('y', DegreeToRad (angleOfPoints), distanceOfPoints * 1.5, &camPos2.x, &camPos2.y, &camPos2.z);
		else
			moveIn3D ('y', DegreeToRad (angleOfPoints), (highestZ - lowestZ) * 1.5, &camPos2.x, &camPos2.y, &camPos2.z);

		camPos1.z = cameraZ;
		camPos2.z = cameraZ;

		// 모든 요소들을 Deselect
		ACAPI_Element_Select (NULL, 0, false);

		// ========== 1번째 캡쳐
		// 카메라 및 대상 위치 설정
		proj3DInfo.isPersp = true;				// 퍼스펙티브 뷰
		proj3DInfo.u.persp.viewCone = 90.0;		// 카메라 시야각
		proj3DInfo.u.persp.rollAngle = 0.0;		// 카메라 롤 각도
		proj3DInfo.u.persp.azimuth = angleOfPoints + 90.0;	// 카메라 방위각

		proj3DInfo.u.persp.pos.x = camPos1.x;
		proj3DInfo.u.persp.pos.y = camPos1.y;
		proj3DInfo.u.persp.cameraZ = camPos1.z;

		proj3DInfo.u.persp.target.x = camPos2.x;
		proj3DInfo.u.persp.target.y = camPos2.y;
		proj3DInfo.u.persp.targetZ = camPos2.z;

		err = ACAPI_Environment (APIEnv_Change3DProjectionSetsID, &proj3DInfo, NULL, NULL);

		// 화면 새로고침
		ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
		ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

		// 화면 캡쳐
		BNZeroMemory (&fsp, sizeof (API_FileSavePars));
		fsp.fileTypeID = APIFType_PNGFile;
		sprintf (filename, "%s - 캡쳐 (1).png", miscAppInfo.caption);
		fsp.file = new IO::Location (location, IO::Name (filename));

		BNZeroMemory (&pars_pict, sizeof (API_SavePars_Picture));
		pars_pict.colorDepth	= APIColorDepth_TC24;
		pars_pict.dithered		= false;
		pars_pict.view2D		= false;
		pars_pict.crop			= true;
		err = ACAPI_Automate (APIDo_SaveID, &fsp, &pars_pict);	// 데모 버전에서는 작동하지 않음
		
		delete fsp.file;

		// ========== 2번째 캡쳐
		// 카메라 및 대상 위치 설정
		proj3DInfo.isPersp = true;				// 퍼스펙티브 뷰
		proj3DInfo.u.persp.viewCone = 90.0;		// 카메라 시야각
		proj3DInfo.u.persp.rollAngle = 0.0;		// 카메라 롤 각도
		proj3DInfo.u.persp.azimuth = angleOfPoints - 90.0;	// 카메라 방위각

		proj3DInfo.u.persp.pos.x = camPos2.x;
		proj3DInfo.u.persp.pos.y = camPos2.y;
		proj3DInfo.u.persp.cameraZ = camPos2.z;

		proj3DInfo.u.persp.target.x = camPos1.x;
		proj3DInfo.u.persp.target.y = camPos1.y;
		proj3DInfo.u.persp.targetZ = camPos1.z;

		err = ACAPI_Environment (APIEnv_Change3DProjectionSetsID, &proj3DInfo, NULL, NULL);

		// 화면 새로고침
		ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
		ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

		// 화면 캡쳐
		BNZeroMemory (&fsp, sizeof (API_FileSavePars));
		fsp.fileTypeID = APIFType_PNGFile;
		sprintf (filename, "%s - 캡쳐 (2).png", miscAppInfo.caption);
		fsp.file = new IO::Location (location, IO::Name (filename));

		BNZeroMemory (&pars_pict, sizeof (API_SavePars_Picture));
		pars_pict.colorDepth	= APIColorDepth_TC24;
		pars_pict.dithered		= false;
		pars_pict.view2D		= false;
		pars_pict.crop			= true;
		err = ACAPI_Automate (APIDo_SaveID, &fsp, &pars_pict);	// 데모 버전에서는 작동하지 않음
		
		delete fsp.file;
	}
	// !!! 3D 투영 정보 ==================================

	// 화면 새로고침
	ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
	ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

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
