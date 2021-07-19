#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "Export.hpp"

using namespace exportDG;

double DIST_BTW_COLUMN = 2.000;		// 기둥 간 최소 간격 (기본값), 추후 다이얼로그에서 변경할 수 있음
VisibleObjectInfo	visibleObjInfo;	// 보이는 레이어 상의 객체별 명칭, 존재 여부, 보이기 여부


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
		sprintf (buffer, "주열구분,구분");
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
				sprintf (buffer, ",세로,");
				for (zz = columnPos.firstStory ; zz <= columnPos.lastStory ; ++zz) {
					resultColumn = findColumn (&columnPos, xx, yy, zz);
					sprintf (piece, "%.0f,", resultColumn.verLen * 1000);
					strcat (buffer, piece);
				}
				strcat (buffer, "\n");
				fprintf (fp, buffer);

				// 높이
				sprintf (buffer, ",높이,");
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
	char	line [2048];	// 파일에서 읽어온 라인 하나
	char	*token;			// 읽어온 문자열의 토큰
	short	lineCount;		// 읽어온 라인 수
	short	tokCount;		// 읽어온 토큰 개수
	short	xx;
	int		count;

	char	nthToken [200][256];	// n번째 토큰

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

			// 토큰 개수가 2개 이상일 때 유효함
			if ((tokCount-1) >= 2) {
				// 토큰 개수가 2 + 3의 배수 개씩만 저장됨 (초과된 항목은 제외)
				if (((tokCount-1) - 2) % 3 != 0) {
					tokCount --;
				}

				this->keyName.push_back (nthToken [0]);		// 예: u_comp
				this->keyDesc.push_back (nthToken [1]);		// 예: 유로폼
				count = atoi (nthToken [2]);
				this->nInfo.push_back (count);				// 예: 5

				vector<string>	varNames;	// 해당 객체의 변수 이름들
				vector<string>	varDescs;	// 해당 객체의 변수 이름에 대한 설명들

				for (xx = 1 ; xx <= count ; ++xx) {
					varNames.push_back (nthToken [1 + xx*2]);
					varDescs.push_back (nthToken [1 + xx*2 + 1]);
				}

				this->varName.push_back (varNames);
				this->varDesc.push_back (varDescs);
			}
		}

		// 파일 닫기
		fclose (fp);

		// 객체 종류 개수
		this->nKnownObjects = lineCount - 1;
		this->nUnknownObjects = 0;
	}
}

// 객체의 레코드 수량 1 증가 (있으면 증가, 없으면 신규 추가)
int	SummaryOfObjectInfo::quantityPlus1 (vector<string> record)
{
	int		xx, yy;
	size_t	vecLen;
	size_t	inVecLen1, inVecLen2;
	int		diff;
	int		value;
	char	tempStr [128];

	vecLen = this->records.size ();

	for (xx = 0 ; xx < vecLen ; ++xx) {
		// 변수 값도 동일할 경우
		try {
			inVecLen1 = this->records.at(xx).size () - 1;		// 끝의 개수 필드를 제외한 길이
			inVecLen2 = record.size ();
		} catch (std::exception e) {
			DGAlert (DG_ERROR, "오류 발생", "objectInfo.records 변수 접근시 인덱싱 오류가 발생했습니다. (1)", "", "확인", "", "");
		}

		if (inVecLen1 == inVecLen2) {
			// 일치하지 않는 필드가 하나라도 있는지 찾아볼 것
			try {
				diff = 0;
				for (yy = 0 ; yy < inVecLen1 ; ++yy) {
					if (my_strcmp (this->records.at(xx).at(yy).c_str (), record.at(yy).c_str ()) != 0)
						diff++;
				}
			} catch (std::exception e) {
				DGAlert (DG_ERROR, "오류 발생", "objectInfo.records 변수 접근시 인덱싱 오류가 발생했습니다. (2)", "", "확인", "", "");
			}

			// 모든 필드가 일치하면
			try {
				if (diff == 0) {
					value = atoi (this->records.at(xx).at(inVecLen1).c_str ());
					value++;
					sprintf (tempStr, "%d", value);
					this->records.at(xx).pop_back ();
					this->records.at(xx).push_back (tempStr);
					return value;
				}
			} catch (std::exception e) {
				DGAlert (DG_ERROR, "오류 발생", "objectInfo.records 변수 접근시 인덱싱 오류가 발생했습니다. (3)", "", "확인", "", "");
			}
		}
	}

	// 없으면 신규 레코드 추가하고 1 리턴
	record.push_back ("1");
	this->records.push_back (record);
	return 1;
}

// 레코드 내용 지우기
void SummaryOfObjectInfo::clear ()
{
	int xx;
	size_t recordSize = this->records.size ();

	for (xx = 0 ; xx < recordSize ; ++xx)
		this->records.at (xx).clear ();

	this->records.clear ();
}

// 선택한 부재 정보 내보내기 (Single 모드)
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
	SummaryOfObjectInfo	objectInfo;

	char			buffer [256];
	char			filename [256];
	char			tempStr [256];
	const char*		foundStr;
	bool			foundObject;
	bool			foundExistValue;
	int				retVal;
	API_AddParID	varType;
	vector<string>	record;

	// 엑셀 파일로 기둥 정보 내보내기
	// 파일 저장을 위한 변수
	API_SpecFolderID	specFolderID = API_ApplicationFolderID;
	IO::Location		location;
	GS::UniString		resultString;
	API_MiscAppInfo		miscAppInfo;
	FILE				*fp;

	ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);
	ACAPI_Environment (APIEnv_GetMiscAppInfoID, &miscAppInfo);
	sprintf (filename, "%s - 선택한 부재 정보.csv", miscAppInfo.caption);
	fp = fopen (filename, "w+");

	if (fp == NULL) {
		ACAPI_WriteReport ("파일을 열 수 없습니다.", true);
		return err;
	}

	for (xx = 0 ; xx < nObjects ; ++xx) {
		foundObject = false;

		BNZeroMemory (&elem, sizeof (API_Element));
		BNZeroMemory (&memo, sizeof (API_ElementMemo));
		elem.header.guid = objects.Pop ();
		err = ACAPI_Element_Get (&elem);
		err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

		// 파라미터 스크립트를 강제로 실행시킴
		ACAPI_Goodies (APIAny_RunGDLParScriptID, &elem.header, 0);

		for (yy = 0 ; yy < objectInfo.keyName.size () ; ++yy) {
			try {
				strcpy (tempStr, objectInfo.keyName.at (yy).c_str ());
			} catch (std::exception e) {
				DGAlert (DG_ERROR, "오류 발생", "objectInfo.keyName 변수 접근시 인덱싱 오류가 발생했습니다.", "", "확인", "", "");
			}
			foundStr = getParameterStringByName (&memo, tempStr);

			// 객체 종류를 찾았다면,
			if (foundStr != NULL) {
				foundObject = true;

				try {
					retVal = my_strcmp (objectInfo.keyDesc.at (yy).c_str (), foundStr);
				} catch (std::exception e) {
					DGAlert (DG_ERROR, "오류 발생", "objectInfo.keyDesc 변수 접근시 인덱싱 오류가 발생했습니다. (1)", "", "확인", "", "");
				}

				if (retVal == 0) {
					foundExistValue = false;

					// 발견한 객체의 데이터를 기반으로 레코드 추가
					if (!record.empty ())
						record.clear ();

					try {
						record.push_back (objectInfo.keyDesc.at (yy));		// 객체 이름
					} catch (std::exception e) {
						DGAlert (DG_ERROR, "오류 발생", "objectInfo.keyDesc 변수 접근시 인덱싱 오류가 발생했습니다. (2)", "", "확인", "", "");
					}
					for (zz = 0 ; zz < objectInfo.nInfo [yy] ; ++zz) {
						try {
							sprintf (buffer, "%s", objectInfo.varName.at(yy).at(zz).c_str ());
						} catch (std::exception e) {
							DGAlert (DG_ERROR, "오류 발생", "objectInfo.varName 변수 접근시 인덱싱 오류가 발생했습니다.", "", "확인", "", "");
						}
						varType = getParameterTypeByName (&memo, buffer);

						if ((varType != APIParT_Separator) || (varType != APIParT_Title) || (varType != API_ZombieParT)) {
							if (varType == APIParT_CString)
								sprintf (tempStr, "%s", getParameterStringByName (&memo, buffer));	// 문자열
							else
								sprintf (tempStr, "%.3f", getParameterValueByName (&memo, buffer));	// 숫자
						}
						record.push_back (tempStr);		// 변수값
					}
					objectInfo.quantityPlus1 (record);

				}
			}
		}

		// 끝내 찾지 못하면 알 수 없는 객체로 취급함
		if (foundObject == false)
			objectInfo.nUnknownObjects ++;

		ACAPI_DisposeElemMemoHdls (&memo);
	}

	// 테스트 루틴: records 필드를 보여줌
	//char msg [1024];
	//char temp [256];
	//try {
	//	for (xx = 0 ; xx < objectInfo.records.size () ; ++xx) {
	//		strcpy (msg, "");
	//		for (yy = 0 ; yy < objectInfo.records.at(xx).size () ; ++yy) {
	//			sprintf (temp, " %s ", objectInfo.records.at(xx).at(yy).c_str ());
	//			strcat (msg, temp);
	//		}
	//		WriteReport_Alert (msg);
	//	}
	//} catch (std::exception e) {
	//	DGAlert (DG_ERROR, "오류 발생", "테스트 루틴에서 인덱싱 오류가 발생했습니다.", "", "확인", "", "");
	//}

	// 보 개수 세기
	//for (xx = 0 ; xx < nBeams ; ++xx) {
	//	BNZeroMemory (&elem, sizeof (API_Element));
	//	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	//	elem.header.guid = beams.Pop ();
	//	err = ACAPI_Element_Get (&elem);
	//	err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

	//	foundExistValue = false;

	//	int len;

	//	len = static_cast<int> (round (GetDistance (elem.beam.begC, elem.beam.endC) * 1000, 0));

	//	// 중복 항목은 개수만 증가
	//	for (zz = 0 ; zz < objectInfo.nCountsBeam ; ++zz) {
	//		if (objectInfo.beamLength [zz] == len) {
	//			objectInfo.beamCount [zz] ++;
	//			foundExistValue = true;
	//			break;
	//		}
	//	}

	//	// 신규 항목 추가하고 개수도 증가
	//	if ( !foundExistValue ) {
	//		objectInfo.beamLength.push_back (len);
	//		objectInfo.beamCount.push_back (1);
	//		objectInfo.nCountsBeam ++;
	//	}

	//	ACAPI_DisposeElemMemoHdls (&memo);
	//}

	// 최종 텍스트 표시
	// APIParT_Length인 경우 1000배 곱해서 표현
	// APIParT_Boolean인 경우 예/아니오 표현
	double	length, length2, length3;
	bool	bTitleAppeared;

	// 객체 종류별로 수량 출력
	for (xx = 0 ; xx < objectInfo.keyDesc.size () ; ++xx) {
		bTitleAppeared = false;

		// 레코드를 전부 순회
		for (yy = 0 ; yy < objectInfo.records.size () ; ++yy) {
			// 객체 종류 이름과 레코드의 1번 필드가 일치하는 경우만 찾아서 출력함
			try {
				retVal = my_strcmp (objectInfo.keyDesc.at(xx).c_str (), objectInfo.records.at(yy).at(0).c_str ());
			} catch (std::exception e) {
				DGAlert (DG_ERROR, "오류 발생", "레코드 순회 루틴에서 인덱싱 오류가 발생했습니다.", "", "확인", "", "");
			}
			if (retVal == 0) {
				// 제목 출력
				if (bTitleAppeared == false) {
					try {
						sprintf (buffer, "\n[%s]\n", objectInfo.keyDesc.at(xx).c_str ());
					} catch (std::exception e) {
						DGAlert (DG_ERROR, "오류 발생", "제목 출력 루틴에서 인덱싱 오류가 발생했습니다.", "", "확인", "", "");
					}
					fprintf (fp, buffer);
					bTitleAppeared = true;
				}

				// 변수별 값 출력
				if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "유로폼 후크") == 0) {
					try {
						// 원형
						if (objectInfo.records.at(yy).at(2).compare ("원형") == 0) {
							sprintf (buffer, "원형 / %s", objectInfo.records.at(yy).at(1));
						}

						// 사각
						if (objectInfo.records.at(yy).at(2).compare ("사각") == 0) {
							sprintf (buffer, "사각 / %s", objectInfo.records.at(yy).at(1));
						}
						fprintf (fp, buffer);
					} catch (std::exception e) {
						DGAlert (DG_ERROR, "오류 발생", "유로폼 후크 루틴에서 인덱싱 오류가 발생했습니다.", "", "확인", "", "");
					}

				} else if ((my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "유로폼") == 0) || (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "스틸폼") == 0)) {
					try {
						// 규격폼
						if (atoi (objectInfo.records.at(yy).at(2).c_str ()) > 0) {
							sprintf (buffer, "%s X %s ", objectInfo.records.at(yy).at(2), objectInfo.records.at(yy).at(3));

						// 비규격품
						} else {
							length = atof (objectInfo.records.at(yy).at(4).c_str ());
							length2 = atof (objectInfo.records.at(yy).at(5).c_str ());
							sprintf (buffer, "%.0f X %.0f ", round (length*1000, 0), round (length2*1000, 0));
						}
						fprintf (fp, buffer);
					} catch (std::exception e) {
						DGAlert (DG_ERROR, "오류 발생", "유로폼 | 스틸폼 루틴에서 인덱싱 오류가 발생했습니다.", "", "확인", "", "");
					}

				} else if (objectInfo.keyDesc.at(xx).compare ("목재") == 0) {
					try {
						length = atof (objectInfo.records.at(yy).at(2).c_str ());
						length2 = atof (objectInfo.records.at(yy).at(3).c_str ());
						length3 = atof (objectInfo.records.at(yy).at(4).c_str ());
						sprintf (buffer, "%.0f X %.0f X %.0f ", round (length*1000, 0), round (length2*1000, 0), round (length3*1000, 0));
						fprintf (fp, buffer);
					} catch (std::exception e) {
						DGAlert (DG_ERROR, "오류 발생", "목재 루틴에서 인덱싱 오류가 발생했습니다.", "", "확인", "", "");
					}

				} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "합판(다각형)") == 0) {
					try {
						sprintf (buffer, "합판(다각형) 넓이 %s ", objectInfo.records.at(yy).at(1).c_str ());
						fprintf (fp, buffer);

						if (atoi (objectInfo.records.at(yy).at(2).c_str ()) > 0) {
							length = atof (objectInfo.records.at(yy).at(3).c_str ());
							sprintf (buffer, "(각재 총길이: %s) ", length);
							fprintf (fp, buffer);
						}
					} catch (std::exception e) {
						DGAlert (DG_ERROR, "오류 발생", "합판(다각형) 루틴에서 인덱싱 오류가 발생했습니다.", "", "확인", "", "");
					}

				} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "합판") == 0) {
					try {
						if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "3x6 [910x1820]") == 0) {
							sprintf (buffer, "910 X 1820 X %s ", objectInfo.records.at(yy).at(2).c_str ());
							fprintf (fp, buffer);

							// 제작틀 ON
							if (atoi (objectInfo.records.at(yy).at(5).c_str ()) > 0) {
								sprintf (buffer, "(각재 총길이: %s) ", objectInfo.records.at(yy).at(6).c_str ());
								fprintf (fp, buffer);

								sprintf (buffer, "(각재 절단 길이: %s) ", objectInfo.records.at(yy).at(7).c_str ());
								fprintf (fp, buffer);
							}

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "4x8 [1220x2440]") == 0) {
							sprintf (buffer, "1220 X 2440 X %s ", objectInfo.records.at(yy).at(2).c_str ());
							fprintf (fp, buffer);

							// 제작틀 ON
							if (atoi (objectInfo.records.at(yy).at(5).c_str ()) > 0) {
								sprintf (buffer, "(각재 총길이: %s) ", objectInfo.records.at(yy).at(6).c_str ());
								fprintf (fp, buffer);

								sprintf (buffer, "(각재 절단 길이: %s) ", objectInfo.records.at(yy).at(7).c_str ());
								fprintf (fp, buffer);
							}

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "2x5 [606x1520]") == 0) {
							sprintf (buffer, "606 X 1520 X %s ", objectInfo.records.at(yy).at(2).c_str ());
							fprintf (fp, buffer);

							// 제작틀 ON
							if (atoi (objectInfo.records.at(yy).at(5).c_str ()) > 0) {
								sprintf (buffer, "(각재 총길이: %s) ", objectInfo.records.at(yy).at(6).c_str ());
								fprintf (fp, buffer);

								sprintf (buffer, "(각재 절단 길이: %s) ", objectInfo.records.at(yy).at(7).c_str ());
								fprintf (fp, buffer);
							}

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "2x6 [606x1820]") == 0) {
							sprintf (buffer, "606 X 1820 X %s ", objectInfo.records.at(yy).at(2).c_str ());
							fprintf (fp, buffer);

							// 제작틀 ON
							if (atoi (objectInfo.records.at(yy).at(5).c_str ()) > 0) {
								sprintf (buffer, "(각재 총길이: %s) ", objectInfo.records.at(yy).at(6).c_str ());
								fprintf (fp, buffer);

								sprintf (buffer, "(각재 절단 길이: %s) ", objectInfo.records.at(yy).at(7).c_str ());
								fprintf (fp, buffer);
							}

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "3x5 [910x1520]") == 0) {
							sprintf (buffer, "910 X 1520 X %s ", objectInfo.records.at(yy).at(2).c_str ());
							fprintf (fp, buffer);

							// 제작틀 ON
							if (atoi (objectInfo.records.at(yy).at(5).c_str ()) > 0) {
								sprintf (buffer, "(각재 총길이: %s) ", objectInfo.records.at(yy).at(6).c_str ());
								fprintf (fp, buffer);

								sprintf (buffer, "(각재 절단 길이: %s) ", objectInfo.records.at(yy).at(7).c_str ());
								fprintf (fp, buffer);
							}

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "비규격") == 0) {
							// 가로 X 세로 X 두께
							length = atof (objectInfo.records.at(yy).at(3).c_str ());
							length2 = atof (objectInfo.records.at(yy).at(4).c_str ());
							sprintf (buffer, "%.0f X %.0f X %s ", round (length*1000, 0), round (length2*1000, 0), objectInfo.records.at(yy).at(2).c_str ());
							fprintf (fp, buffer);

							// 제작틀 ON
							if (atoi (objectInfo.records.at(yy).at(5).c_str ()) > 0) {
								sprintf (buffer, "(각재 총길이: %s) ", objectInfo.records.at(yy).at(6).c_str ());
								fprintf (fp, buffer);

								sprintf (buffer, "(각재 절단 길이: %s) ", objectInfo.records.at(yy).at(7).c_str ());
								fprintf (fp, buffer);
							}

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "비정형") == 0) {
							sprintf (buffer, "비정형 ");
							fprintf (fp, buffer);

						} else {
							sprintf (buffer, "다각형 ");
							fprintf (fp, buffer);
						}
					} catch (std::exception e) {
						DGAlert (DG_ERROR, "오류 발생", "합판 루틴에서 인덱싱 오류가 발생했습니다.", "", "확인", "", "");
					}

				} else if (objectInfo.keyDesc.at(xx).compare ("RS Push-Pull Props") == 0) {
					try {
						// 베이스 플레이트 유무
						if (atoi (objectInfo.records.at(yy).at(1).c_str ()) == 1) {
							sprintf (buffer, "베이스 플레이트(있음) ");
						} else {
							sprintf (buffer, "베이스 플레이트(없음) ");
						}
						fprintf (fp, buffer);

						// 규격(상부)
						sprintf (buffer, "규격(상부): %s ", objectInfo.records.at(yy).at(2).c_str ());
						fprintf (fp, buffer);

						// 규격(하부) - 선택사항
						if (atoi (objectInfo.records.at(yy).at(4).c_str ()) == 1) {
							sprintf (buffer, "규격(하부): %s ", objectInfo.records.at(yy).at(3).c_str ());
						}
						fprintf (fp, buffer);
					} catch (std::exception e) {
						DGAlert (DG_ERROR, "오류 발생", "RS Push-Pull Props 루틴에서 인덱싱 오류가 발생했습니다.", "", "확인", "", "");
					}

				} else if (objectInfo.keyDesc.at(xx).compare ("사각파이프") == 0) {
					try {
						// 사각파이프
						if (atof (objectInfo.records.at(yy).at(1).c_str ()) < EPS) {
							length = atof (objectInfo.records.at(yy).at(2).c_str ());
							sprintf (buffer, "50 x 50 x %.0f ", round (length*1000, 0));

						// 직사각파이프
						} else {
							length = atof (objectInfo.records.at(yy).at(2).c_str ());
							sprintf (buffer, "%s x %.0f ", objectInfo.records.at(yy).at(1).c_str (), round (length*1000, 0));
						}
						fprintf (fp, buffer);
					} catch (std::exception e) {
						DGAlert (DG_ERROR, "오류 발생", "사각파이프 루틴에서 인덱싱 오류가 발생했습니다.", "", "확인", "", "");
					}

				} else if (objectInfo.keyDesc.at(xx).compare ("원형파이프") == 0) {
					try {
						length = atof (objectInfo.records.at(yy).at(1).c_str ());
						sprintf (buffer, "%.0f ", round (length*1000, 0));
						fprintf (fp, buffer);
					} catch (std::exception e) {
						DGAlert (DG_ERROR, "오류 발생", "원형파이프 루틴에서 인덱싱 오류가 발생했습니다.", "", "확인", "", "");
					}

				} else if (objectInfo.keyDesc.at(xx).compare ("아웃코너앵글") == 0) {
					try {
						length = atof (objectInfo.records.at(yy).at(1).c_str ());
						sprintf (buffer, "%.0f ", round (length*1000, 0));
						fprintf (fp, buffer);
					} catch (std::exception e) {
						DGAlert (DG_ERROR, "오류 발생", "아웃코너앵글 루틴에서 인덱싱 오류가 발생했습니다.", "", "확인", "", "");
					}

				} else if (objectInfo.keyDesc.at(xx).compare ("매직바") == 0) {
					try {
						if (atoi (objectInfo.records.at(yy).at(2).c_str ()) > 0) {
							length = atof (objectInfo.records.at(yy).at(3).c_str ());
							length2 = atof (objectInfo.records.at(yy).at(4).c_str ());
							length3 = atof (objectInfo.records.at(yy).at(5).c_str ());
							sprintf (buffer, "%.0f, 합판(%.0f X %.0f)", round (atof (objectInfo.records.at(yy).at(1).c_str ())*1000, 0), round ((length - length2)*1000, 0), round (length3*1000, 0));
						} else {
							length = atof (objectInfo.records.at(yy).at(1).c_str ());
							sprintf (buffer, "%.0f ", round (length*1000, 0));
						}
						fprintf (fp, buffer);
					} catch (std::exception e) {
						DGAlert (DG_ERROR, "오류 발생", "매직바 루틴에서 인덱싱 오류가 발생했습니다.", "", "확인", "", "");
					}

				} else if (objectInfo.keyDesc.at(xx).compare ("블루목심") == 0) {
					try {
						sprintf (buffer, "%s ", objectInfo.records.at(yy).at(1).c_str ());
						fprintf (fp, buffer);
					} catch (std::exception e) {
						DGAlert (DG_ERROR, "오류 발생", "블루목심 루틴에서 인덱싱 오류가 발생했습니다.", "", "확인", "", "");
					}

				} else if (objectInfo.keyDesc.at(xx).compare ("보 멍에제") == 0) {
					try {
						length = atof (objectInfo.records.at(yy).at(1).c_str ());
						sprintf (buffer, "%.0f ", round (length*1000, 0));
						fprintf (fp, buffer);
					} catch (std::exception e) {
						DGAlert (DG_ERROR, "오류 발생", "보 멍에제 루틴에서 인덱싱 오류가 발생했습니다.", "", "확인", "", "");
					}

				} else if (objectInfo.keyDesc.at(xx).compare ("물량합판") == 0) {
					try {
						sprintf (buffer, "%s ㎡ ", objectInfo.records.at(yy).at(1).c_str ());
						fprintf (fp, buffer);
					} catch (std::exception e) {
						DGAlert (DG_ERROR, "오류 발생", "물량합판 루틴에서 인덱싱 오류가 발생했습니다.", "", "확인", "", "");
					}
						
				} else {
					for (zz = 0 ; zz < objectInfo.nInfo.at(xx) ; ++zz) {
						// 변수별 값 출력
						try {
							sprintf (buffer, "%s(%s) ", objectInfo.varDesc.at(xx).at(zz).c_str (), objectInfo.records.at(yy).at(zz+1).c_str ());
						} catch (std::exception e) {
							DGAlert (DG_ERROR, "오류 발생", "변수별 값 출력에서 인덱싱 오류가 발생했습니다.", "", "확인", "", "");
						}
						fprintf (fp, buffer);
					}
				}

				// 수량 출력
				try {
					sprintf (buffer, ": %s EA\n", objectInfo.records.at(yy).at(objectInfo.records.at(yy).size ()-1).c_str ());
				} catch (std::exception e) {
					DGAlert (DG_ERROR, "오류 발생", "수량 출력에서 인덱싱 오류가 발생했습니다.", "", "확인", "", "");
				}
				fprintf (fp, buffer);
			}
		}
	}

	// 일반 요소 - 보
	//for (xx = 0 ; xx < objectInfo.nCountsBeam ; ++xx) {
	//	if (xx == 0) {
	//		fprintf (fp, "\n[보]\n");
	//	}
	//	sprintf (buffer, "%d : %d EA\n", objectInfo.beamLength [xx], objectInfo.beamCount [xx]);
	//	fprintf (fp, buffer);
	//}

	// 알 수 없는 객체
	if (objectInfo.nUnknownObjects > 0) {
		sprintf (buffer, "\n알 수 없는 객체 : %d EA\n", objectInfo.nUnknownObjects);
		fprintf (fp, buffer);
	}

	fclose (fp);

	// 화면 새로고침
	ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
	ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

	ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);
	location.ToDisplayText (&resultString);
	sprintf (buffer, "결과물을 다음 위치에 저장했습니다.\n\n%s\n또는 프로젝트 파일이 있는 폴더", resultString.ToCStr ().Get ());
	ACAPI_WriteReport (buffer, true);

	return	err;
}

// 선택한 부재 정보 내보내기 (Multi 모드)
GSErrCode	exportElementInfoOnVisibleLayers (void)
{
	GSErrCode	err = NoError;
	short		xx, yy, zz;
	short		mm;
	bool		regenerate = true;
	short		result;

	// 모든 객체들의 원점 좌표를 전부 저장함
	vector<API_Coord3D>	vecPos;

	// 모든 객체, 보 저장
	GS::Array<API_Guid>		elemList;
	GS::Array<API_Guid>&	objects = GS::Array<API_Guid> ();
	GS::Array<API_Guid>&	beams = GS::Array<API_Guid> ();
	long					nObjects = 0;
	long					nBeams = 0;

	// 레이어 관련 변수
	short			nLayers;
	API_Attribute	attrib;
	short			nVisibleLayers = 0;
	short			visLayerList [1024];
	char			fullLayerName [128];

	// 레이어 타입에 따라 캡쳐 방향 지정
	char*			foundLayerName;
	short			layerType = UNDEFINED;

	// 기타
	char			buffer [256];
	char			filename [256];

	// 작업 층 정보
	API_StoryInfo	storyInfo;
	double			workLevel_object;		// 벽의 작업 층 높이

	// 진행바를 표현하기 위한 변수
	GS::UniString       title ("내보내기 진행 상황");
	GS::UniString       subtitle ("진행중...");
	short	nPhase;
	Int32	cur, total;

	// 엑셀 파일로 기둥 정보 내보내기
	// 파일 저장을 위한 변수
	API_SpecFolderID	specFolderID = API_ApplicationFolderID;
	IO::Location		location;
	GS::UniString		resultString;
	API_MiscAppInfo		miscAppInfo;
	FILE				*fp;
	FILE				*fp_unite;

	// 선택한 요소들의 정보 요약하기
	API_Element			elem;
	API_ElementMemo		memo;
	SummaryOfObjectInfo	objectInfo;

	char			tempStr [256];
	const char*		foundStr;
	bool			foundObject;
	bool			foundExistValue;
	int				retVal;
	API_AddParID	varType;
	vector<string>	record;


	ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);

	// [DIALOG] 다이얼로그에서 객체 이미지를 캡쳐할지 여부를 물어봄
	//result = DGAlert (DG_INFORMATION, "캡쳐 여부 질문", "캡쳐 작업을 수행하시겠습니까?", "", "예", "아니오", "");
	result = DG_CANCEL;

	// 프로젝트 내 레이어 개수를 알아냄
	BNZeroMemory (&attrib, sizeof (API_Attribute));
	attrib.layer.head.typeID = API_LayerID;
	err = ACAPI_Attribute_GetNum (API_LayerID, &nLayers);

	// 보이는 레이어들의 목록 저장하기
	for (xx = 1 ; xx <= nLayers ; ++xx) {
		BNZeroMemory (&attrib, sizeof (API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = xx;
		err = ACAPI_Attribute_Get (&attrib);
		if (err == NoError) {
			if (!((attrib.layer.head.flags & APILay_Hidden) == true)) {
				visLayerList [nVisibleLayers++] = attrib.layer.head.index;
			}
		}
	}

	// 일시적으로 모든 레이어 숨기기
	for (xx = 1 ; xx <= nLayers ; ++xx) {
		BNZeroMemory (&attrib, sizeof (API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = xx;
		err = ACAPI_Attribute_Get (&attrib);
		if (err == NoError) {
			attrib.layer.head.flags |= APILay_Hidden;
			ACAPI_Attribute_Modify (&attrib, NULL);
		}
	}

	ACAPI_Environment (APIEnv_GetMiscAppInfoID, &miscAppInfo);
	sprintf (filename, "%s - 선택한 부재 정보 (통합).csv", miscAppInfo.caption);
	fp_unite = fopen (filename, "w+");

	if (fp_unite == NULL) {
		ACAPI_WriteReport ("통합 버전 엑셀파일을 만들 수 없습니다.", true);
		return	NoError;
	}

	// 진행 상황 표시하는 기능 - 초기화
	nPhase = 1;
	cur = 1;
	total = nVisibleLayers;
	ACAPI_Interface (APIIo_InitProcessWindowID, &title, &nPhase);
	ACAPI_Interface (APIIo_SetNextProcessPhaseID, &subtitle, &total);

	// 보이는 레이어들을 하나씩 순회하면서 전체 요소들을 선택한 후 "선택한 부재 정보 내보내기" 루틴 실행
	for (mm = 1 ; mm <= nVisibleLayers ; ++mm) {
		BNZeroMemory (&attrib, sizeof (API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = visLayerList [mm-1];
		err = ACAPI_Attribute_Get (&attrib);

		// 초기화
		objects.Clear ();
		beams.Clear ();
		vecPos.clear ();
		objectInfo.clear ();

		if (err == NoError) {
			// 레이어 보이기
			if ((attrib.layer.head.flags & APILay_Hidden) == true) {
				attrib.layer.head.flags ^= APILay_Hidden;
				ACAPI_Attribute_Modify (&attrib, NULL);
			}

			// 모든 요소 가져오기
			ACAPI_Element_GetElemList (API_ObjectID, &elemList, APIFilt_OnVisLayer | APIFilt_In3D);		// 보이는 레이어에 있음, 객체 타입만
			while (elemList.GetSize () > 0) {
				objects.Push (elemList.Pop ());
			}

			ACAPI_Element_GetElemList (API_BeamID, &elemList, APIFilt_OnVisLayer | APIFilt_In3D);		// 보이는 레이어에 있음, 보 타입만
			while (elemList.GetSize () > 0) {
				beams.Push (elemList.Pop ());
			}
			nObjects = objects.GetSize ();
			nBeams = beams.GetSize ();

			if ((nObjects == 0) && (nBeams == 0))
				continue;

			// 레이어 이름 가져옴
			sprintf (fullLayerName, "%s", attrib.layer.head.name);
			fullLayerName [strlen (fullLayerName)] = '\0';

			// 레이어 이름 식별하기 (WALL: 벽, SLAB: 슬래브, COLU: 기둥, BEAM: 보, WLBM: 눈썹보)
			layerType = UNDEFINED;
			foundLayerName = strstr (fullLayerName, "WALL");
			if (foundLayerName != NULL)	layerType = WALL;
			foundLayerName = strstr (fullLayerName, "SLAB");
			if (foundLayerName != NULL)	layerType = SLAB;
			foundLayerName = strstr (fullLayerName, "COLU");
			if (foundLayerName != NULL)	layerType = COLU;
			foundLayerName = strstr (fullLayerName, "BEAM");
			if (foundLayerName != NULL)	layerType = BEAM;
			foundLayerName = strstr (fullLayerName, "WLBM");
			if (foundLayerName != NULL)	layerType = WLBM;

			// 모든 요소들을 3D로 보여줌
			err = ACAPI_Automate (APIDo_ShowAllIn3DID);

			sprintf (filename, "%s - 선택한 부재 정보.csv", fullLayerName);
			fp = fopen (filename, "w+");

			if (fp == NULL) {
				sprintf (buffer, "레이어 %s는 파일명이 될 수 없으므로 생략합니다.", fullLayerName);
				ACAPI_WriteReport (buffer, true);
				continue;
			}

			// 레이어 이름 (통합 버전에만)
			sprintf (buffer, "\n\n<< 레이어 : %s >>\n", fullLayerName);
			fprintf (fp_unite, buffer);

			for (xx = 0 ; xx < nObjects ; ++xx) {
				foundObject = false;

				BNZeroMemory (&elem, sizeof (API_Element));
				BNZeroMemory (&memo, sizeof (API_ElementMemo));
				elem.header.guid = objects.Pop ();
				err = ACAPI_Element_Get (&elem);
				err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

				// 객체의 원점 수집하기 ==================================
				API_Coord3D	coord;

				coord.x = elem.object.pos.x;
				coord.y = elem.object.pos.y;
				coord.z = elem.object.level;
					
				vecPos.push_back (coord);
				// 객체의 원점 수집하기 ==================================

				// 작업 층 높이 반영 -- 객체
				if (xx == 0) {
					BNZeroMemory (&storyInfo, sizeof (API_StoryInfo));
					workLevel_object = 0.0;
					ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo);
					for (yy = 0 ; yy < (storyInfo.lastStory - storyInfo.firstStory) ; ++yy) {
						if (storyInfo.data [0][yy].index == elem.header.floorInd) {
							workLevel_object = storyInfo.data [0][yy].level;
							break;
						}
					}
					BMKillHandle ((GSHandle *) &storyInfo.data);
				}

				// 파라미터 스크립트를 강제로 실행시킴
				ACAPI_Goodies (APIAny_RunGDLParScriptID, &elem.header, 0);

				for (yy = 0 ; yy < objectInfo.keyName.size () ; ++yy) {
					try {
						strcpy (tempStr, objectInfo.keyName.at (yy).c_str ());
					} catch (std::exception e) {
						DGAlert (DG_ERROR, "오류 발생", "objectInfo.keyName 변수 접근시 인덱싱 오류가 발생했습니다.", "", "확인", "", "");
					}
					foundStr = getParameterStringByName (&memo, tempStr);

					// 객체 종류를 찾았다면,
					if (foundStr != NULL) {
						foundObject = true;

						try {
							retVal = my_strcmp (objectInfo.keyDesc.at (yy).c_str (), foundStr);
						} catch (std::exception e) {
							DGAlert (DG_ERROR, "오류 발생", "objectInfo.keyDesc 변수 접근시 인덱싱 오류가 발생했습니다. (1)", "", "확인", "", "");
						}

						if (retVal == 0) {
							foundExistValue = false;

							// 발견한 객체의 데이터를 기반으로 레코드 추가
							if (!record.empty ())
								record.clear ();

							try {
								record.push_back (objectInfo.keyDesc.at (yy));		// 객체 이름
							} catch (std::exception e) {
								DGAlert (DG_ERROR, "오류 발생", "objectInfo.keyDesc 변수 접근시 인덱싱 오류가 발생했습니다. (2)", "", "확인", "", "");
							}
							for (zz = 0 ; zz < objectInfo.nInfo [yy] ; ++zz) {
								try {
									sprintf (buffer, "%s", objectInfo.varName.at(yy).at(zz).c_str ());
								} catch (std::exception e) {
									DGAlert (DG_ERROR, "오류 발생", "objectInfo.varName 변수 접근시 인덱싱 오류가 발생했습니다.", "", "확인", "", "");
								}
								varType = getParameterTypeByName (&memo, buffer);

								if ((varType != APIParT_Separator) || (varType != APIParT_Title) || (varType != API_ZombieParT)) {
									if (varType == APIParT_CString)
										sprintf (tempStr, "%s", getParameterStringByName (&memo, buffer));	// 문자열
									else
										sprintf (tempStr, "%.3f", getParameterValueByName (&memo, buffer));	// 숫자
								}
								record.push_back (tempStr);		// 변수값
							}
							objectInfo.quantityPlus1 (record);

						}
					}
				}

				// 끝내 찾지 못하면 알 수 없는 객체로 취급함
				if (foundObject == false)
					objectInfo.nUnknownObjects ++;

				ACAPI_DisposeElemMemoHdls (&memo);
			}

			// 보 개수 세기
			//for (xx = 0 ; xx < nBeams ; ++xx) {
			//	BNZeroMemory (&elem, sizeof (API_Element));
			//	BNZeroMemory (&memo, sizeof (API_ElementMemo));
			//	elem.header.guid = beams.Pop ();
			//	err = ACAPI_Element_Get (&elem);
			//	err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

			//	foundExistValue = false;

			//	int len;

			//	len = static_cast<int> (round (GetDistance (elem.beam.begC, elem.beam.endC) * 1000, 0));

			//	// 중복 항목은 개수만 증가
			//	for (zz = 0 ; zz < objectInfo.nCountsBeam ; ++zz) {
			//		if (objectInfo.beamLength [zz] == len) {
			//			objectInfo.beamCount [zz] ++;
			//			foundExistValue = true;
			//			break;
			//		}
			//	}

			//	// 신규 항목 추가하고 개수도 증가
			//	if ( !foundExistValue ) {
			//		objectInfo.beamLength.push_back (len);
			//		objectInfo.beamCount.push_back (1);
			//		objectInfo.nCountsBeam ++;
			//	}

			//	ACAPI_DisposeElemMemoHdls (&memo);
			//}

			// APIParT_Length인 경우 1000배 곱해서 표현
			// APIParT_Boolean인 경우 예/아니오 표현
			double	length, length2, length3;
			bool	bTitleAppeared;

			// 객체 종류별로 수량 출력
			for (xx = 0 ; xx < objectInfo.keyDesc.size () ; ++xx) {
				bTitleAppeared = false;

				// 레코드를 전부 순회
				for (yy = 0 ; yy < objectInfo.records.size () ; ++yy) {
					// 객체 종류 이름과 레코드의 1번 필드가 일치하는 경우만 찾아서 출력함
					try {
						retVal = my_strcmp (objectInfo.keyDesc.at(xx).c_str (), objectInfo.records.at(yy).at(0).c_str ());
					} catch (std::exception e) {
						DGAlert (DG_ERROR, "오류 발생", "레코드 순회 루틴에서 인덱싱 오류가 발생했습니다.", "", "확인", "", "");
					}
					if (retVal == 0) {
						// 제목 출력
						if (bTitleAppeared == false) {
							try {
								sprintf (buffer, "\n[%s]\n", objectInfo.keyDesc.at(xx).c_str ());
							} catch (std::exception e) {
								DGAlert (DG_ERROR, "오류 발생", "제목 출력 루틴에서 인덱싱 오류가 발생했습니다.", "", "확인", "", "");
							}
							fprintf (fp, buffer);
							fprintf (fp_unite, buffer);
							bTitleAppeared = true;
						}

						// 변수별 값 출력
						if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "유로폼 후크") == 0) {
							try {
								// 원형
								if (objectInfo.records.at(yy).at(2).compare ("원형") == 0) {
									sprintf (buffer, "원형 / %s", objectInfo.records.at(yy).at(1));
								}

								// 사각
								if (objectInfo.records.at(yy).at(2).compare ("사각") == 0) {
									sprintf (buffer, "사각 / %s", objectInfo.records.at(yy).at(1));
								}
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							} catch (std::exception e) {
								DGAlert (DG_ERROR, "오류 발생", "유로폼 후크 루틴에서 인덱싱 오류가 발생했습니다.", "", "확인", "", "");
							}

						} else if ((my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "유로폼") == 0) || (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "스틸폼") == 0)) {
							try {
								// 규격폼
								if (atoi (objectInfo.records.at(yy).at(2).c_str ()) > 0) {
									sprintf (buffer, "%s X %s ", objectInfo.records.at(yy).at(2), objectInfo.records.at(yy).at(3));

								// 비규격품
								} else {
									length = atof (objectInfo.records.at(yy).at(4).c_str ());
									length2 = atof (objectInfo.records.at(yy).at(5).c_str ());
									sprintf (buffer, "%.0f X %.0f ", round (length*1000, 0), round (length2*1000, 0));
								}
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							} catch (std::exception e) {
								DGAlert (DG_ERROR, "오류 발생", "유로폼 | 스틸폼 루틴에서 인덱싱 오류가 발생했습니다.", "", "확인", "", "");
							}

						} else if (objectInfo.keyDesc.at(xx).compare ("목재") == 0) {
							try {
								length = atof (objectInfo.records.at(yy).at(2).c_str ());
								length2 = atof (objectInfo.records.at(yy).at(3).c_str ());
								length3 = atof (objectInfo.records.at(yy).at(4).c_str ());
								sprintf (buffer, "%.0f X %.0f X %.0f ", round (length*1000, 0), round (length2*1000, 0), round (length3*1000, 0));
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							} catch (std::exception e) {
								DGAlert (DG_ERROR, "오류 발생", "목재 루틴에서 인덱싱 오류가 발생했습니다.", "", "확인", "", "");
							}

						} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "합판(다각형)") == 0) {
							try {
								sprintf (buffer, "합판(다각형) 넓이 %s ", objectInfo.records.at(yy).at(1).c_str ());
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);

								if (atoi (objectInfo.records.at(yy).at(2).c_str ()) > 0) {
									length = atof (objectInfo.records.at(yy).at(3).c_str ());
									sprintf (buffer, "(각재 총길이: %s) ", length);
									fprintf (fp, buffer);
									fprintf (fp_unite, buffer);
								}
							} catch (std::exception e) {
								DGAlert (DG_ERROR, "오류 발생", "합판(다각형) 루틴에서 인덱싱 오류가 발생했습니다.", "", "확인", "", "");
							}

						} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "합판") == 0) {
							try {
								if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "3x6 [910x1820]") == 0) {
									sprintf (buffer, "910 X 1820 X %s ", objectInfo.records.at(yy).at(2).c_str ());
									fprintf (fp, buffer);
									fprintf (fp_unite, buffer);

									// 제작틀 ON
									if (atoi (objectInfo.records.at(yy).at(5).c_str ()) > 0) {
										sprintf (buffer, "(각재 총길이: %s) ", objectInfo.records.at(yy).at(6).c_str ());
										fprintf (fp, buffer);
										fprintf (fp_unite, buffer);

										sprintf (buffer, "(각재 절단 길이: %s) ", objectInfo.records.at(yy).at(7).c_str ());
										fprintf (fp, buffer);
										fprintf (fp_unite, buffer);
									}

								} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "4x8 [1220x2440]") == 0) {
									sprintf (buffer, "1220 X 2440 X %s ", objectInfo.records.at(yy).at(2).c_str ());
									fprintf (fp, buffer);
									fprintf (fp_unite, buffer);

									// 제작틀 ON
									if (atoi (objectInfo.records.at(yy).at(5).c_str ()) > 0) {
										sprintf (buffer, "(각재 총길이: %s) ", objectInfo.records.at(yy).at(6).c_str ());
										fprintf (fp, buffer);
										fprintf (fp_unite, buffer);

										sprintf (buffer, "(각재 절단 길이: %s) ", objectInfo.records.at(yy).at(7).c_str ());
										fprintf (fp, buffer);
										fprintf (fp_unite, buffer);
									}

								} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "2x5 [606x1520]") == 0) {
									sprintf (buffer, "606 X 1520 X %s ", objectInfo.records.at(yy).at(2).c_str ());
									fprintf (fp, buffer);
									fprintf (fp_unite, buffer);

									// 제작틀 ON
									if (atoi (objectInfo.records.at(yy).at(5).c_str ()) > 0) {
										sprintf (buffer, "(각재 총길이: %s) ", objectInfo.records.at(yy).at(6).c_str ());
										fprintf (fp, buffer);
										fprintf (fp_unite, buffer);

										sprintf (buffer, "(각재 절단 길이: %s) ", objectInfo.records.at(yy).at(7).c_str ());
										fprintf (fp, buffer);
										fprintf (fp_unite, buffer);
									}

								} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "2x6 [606x1820]") == 0) {
									sprintf (buffer, "606 X 1820 X %s ", objectInfo.records.at(yy).at(2).c_str ());
									fprintf (fp, buffer);
									fprintf (fp_unite, buffer);

									// 제작틀 ON
									if (atoi (objectInfo.records.at(yy).at(5).c_str ()) > 0) {
										sprintf (buffer, "(각재 총길이: %s) ", objectInfo.records.at(yy).at(6).c_str ());
										fprintf (fp, buffer);
										fprintf (fp_unite, buffer);

										sprintf (buffer, "(각재 절단 길이: %s) ", objectInfo.records.at(yy).at(7).c_str ());
										fprintf (fp, buffer);
										fprintf (fp_unite, buffer);
									}

								} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "3x5 [910x1520]") == 0) {
									sprintf (buffer, "910 X 1520 X %s ", objectInfo.records.at(yy).at(2).c_str ());
									fprintf (fp, buffer);
									fprintf (fp_unite, buffer);

									// 제작틀 ON
									if (atoi (objectInfo.records.at(yy).at(5).c_str ()) > 0) {
										sprintf (buffer, "(각재 총길이: %s) ", objectInfo.records.at(yy).at(6).c_str ());
										fprintf (fp, buffer);
										fprintf (fp_unite, buffer);

										sprintf (buffer, "(각재 절단 길이: %s) ", objectInfo.records.at(yy).at(7).c_str ());
										fprintf (fp, buffer);
										fprintf (fp_unite, buffer);
									}

								} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "비규격") == 0) {
									// 가로 X 세로 X 두께
									length = atof (objectInfo.records.at(yy).at(3).c_str ());
									length2 = atof (objectInfo.records.at(yy).at(4).c_str ());
									sprintf (buffer, "%.0f X %.0f X %s ", round (length*1000, 0), round (length2*1000, 0), objectInfo.records.at(yy).at(2).c_str ());
									fprintf (fp, buffer);
									fprintf (fp_unite, buffer);

									// 제작틀 ON
									if (atoi (objectInfo.records.at(yy).at(5).c_str ()) > 0) {
										sprintf (buffer, "(각재 총길이: %s) ", objectInfo.records.at(yy).at(6).c_str ());
										fprintf (fp, buffer);
										fprintf (fp_unite, buffer);

										sprintf (buffer, "(각재 절단 길이: %s) ", objectInfo.records.at(yy).at(7).c_str ());
										fprintf (fp, buffer);
										fprintf (fp_unite, buffer);
									}

								} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "비정형") == 0) {
									sprintf (buffer, "비정형 ");
									fprintf (fp, buffer);
									fprintf (fp_unite, buffer);

								} else {
									sprintf (buffer, "다각형 ");
									fprintf (fp, buffer);
									fprintf (fp_unite, buffer);
								}
							} catch (std::exception e) {
								DGAlert (DG_ERROR, "오류 발생", "합판 루틴에서 인덱싱 오류가 발생했습니다.", "", "확인", "", "");
							}

						} else if (objectInfo.keyDesc.at(xx).compare ("RS Push-Pull Props") == 0) {
							try {
								// 베이스 플레이트 유무
								if (atoi (objectInfo.records.at(yy).at(1).c_str ()) == 1) {
									sprintf (buffer, "베이스 플레이트(있음) ");
								} else {
									sprintf (buffer, "베이스 플레이트(없음) ");
								}
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);

								// 규격(상부)
								sprintf (buffer, "규격(상부): %s ", objectInfo.records.at(yy).at(2).c_str ());
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);

								// 규격(하부) - 선택사항
								if (atoi (objectInfo.records.at(yy).at(4).c_str ()) == 1) {
									sprintf (buffer, "규격(하부): %s ", objectInfo.records.at(yy).at(3).c_str ());
								}
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							} catch (std::exception e) {
								DGAlert (DG_ERROR, "오류 발생", "RS Push-Pull Props 루틴에서 인덱싱 오류가 발생했습니다.", "", "확인", "", "");
							}

						} else if (objectInfo.keyDesc.at(xx).compare ("사각파이프") == 0) {
							try {
								// 사각파이프
								if (atof (objectInfo.records.at(yy).at(1).c_str ()) < EPS) {
									length = atof (objectInfo.records.at(yy).at(2).c_str ());
									sprintf (buffer, "50 x 50 x %.0f ", round (length*1000, 0));

								// 직사각파이프
								} else {
									length = atof (objectInfo.records.at(yy).at(2).c_str ());
									sprintf (buffer, "%s x %.0f ", objectInfo.records.at(yy).at(1).c_str (), round (length*1000, 0));
								}
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							} catch (std::exception e) {
								DGAlert (DG_ERROR, "오류 발생", "사각파이프 루틴에서 인덱싱 오류가 발생했습니다.", "", "확인", "", "");
							}

						} else if (objectInfo.keyDesc.at(xx).compare ("원형파이프") == 0) {
							try {
								length = atof (objectInfo.records.at(yy).at(1).c_str ());
								sprintf (buffer, "%.0f ", round (length*1000, 0));
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							} catch (std::exception e) {
								DGAlert (DG_ERROR, "오류 발생", "원형파이프 루틴에서 인덱싱 오류가 발생했습니다.", "", "확인", "", "");
							}

						} else if (objectInfo.keyDesc.at(xx).compare ("아웃코너앵글") == 0) {
							try {
								length = atof (objectInfo.records.at(yy).at(1).c_str ());
								sprintf (buffer, "%.0f ", round (length*1000, 0));
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							} catch (std::exception e) {
								DGAlert (DG_ERROR, "오류 발생", "아웃코너앵글 루틴에서 인덱싱 오류가 발생했습니다.", "", "확인", "", "");
							}

						} else if (objectInfo.keyDesc.at(xx).compare ("매직바") == 0) {
							try {
								if (atoi (objectInfo.records.at(yy).at(2).c_str ()) > 0) {
									length = atof (objectInfo.records.at(yy).at(3).c_str ());
									length2 = atof (objectInfo.records.at(yy).at(4).c_str ());
									length3 = atof (objectInfo.records.at(yy).at(5).c_str ());
									sprintf (buffer, "%.0f, 합판(%.0f X %.0f)", round (atof (objectInfo.records.at(yy).at(1).c_str ())*1000, 0), round ((length - length2)*1000, 0), round (length3*1000, 0));
								} else {
									length = atof (objectInfo.records.at(yy).at(1).c_str ());
									sprintf (buffer, "%.0f ", round (length*1000, 0));
								}
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							} catch (std::exception e) {
								DGAlert (DG_ERROR, "오류 발생", "매직바 루틴에서 인덱싱 오류가 발생했습니다.", "", "확인", "", "");
							}

						} else if (objectInfo.keyDesc.at(xx).compare ("블루목심") == 0) {
							try {
								sprintf (buffer, "%s ", objectInfo.records.at(yy).at(1).c_str ());
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							} catch (std::exception e) {
								DGAlert (DG_ERROR, "오류 발생", "블루목심 루틴에서 인덱싱 오류가 발생했습니다.", "", "확인", "", "");
							}

						} else if (objectInfo.keyDesc.at(xx).compare ("보 멍에제") == 0) {
							try {
								length = atof (objectInfo.records.at(yy).at(1).c_str ());
								sprintf (buffer, "%.0f ", round (length*1000, 0));
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							} catch (std::exception e) {
								DGAlert (DG_ERROR, "오류 발생", "보 멍에제 루틴에서 인덱싱 오류가 발생했습니다.", "", "확인", "", "");
							}

						} else if (objectInfo.keyDesc.at(xx).compare ("물량합판") == 0) {
							try {
								sprintf (buffer, "%s ㎡ ", objectInfo.records.at(yy).at(1).c_str ());
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							} catch (std::exception e) {
								DGAlert (DG_ERROR, "오류 발생", "물량합판 루틴에서 인덱싱 오류가 발생했습니다.", "", "확인", "", "");
							}
						
						} else {
							for (zz = 0 ; zz < objectInfo.nInfo.at(xx) ; ++zz) {
								// 변수별 값 출력
								try {
									sprintf (buffer, "%s(%s) ", objectInfo.varDesc.at(xx).at(zz).c_str (), objectInfo.records.at(yy).at(zz+1).c_str ());
								} catch (std::exception e) {
									DGAlert (DG_ERROR, "오류 발생", "변수별 값 출력에서 인덱싱 오류가 발생했습니다.", "", "확인", "", "");
								}
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
							}
						}

						// 수량 출력
						try {
							sprintf (buffer, ": %s EA\n", objectInfo.records.at(yy).at(objectInfo.records.at(yy).size ()-1).c_str ());
						} catch (std::exception e) {
							DGAlert (DG_ERROR, "오류 발생", "수량 출력에서 인덱싱 오류가 발생했습니다.", "", "확인", "", "");
						}
						fprintf (fp, buffer);
						fprintf (fp_unite, buffer);
					}
				}
			}

			// 일반 요소 - 보
			//for (xx = 0 ; xx < objectInfo.nCountsBeam ; ++xx) {
			//	if (xx == 0) {
			//		fprintf (fp, "\n[보]\n");
			//	}
			//	sprintf (buffer, "%d : %d EA\n", objectInfo.beamLength [xx], objectInfo.beamCount [xx]);
			//	fprintf (fp, buffer);
			//	fprintf (fp_unite, buffer);
			//}

			// 알 수 없는 객체
			if (objectInfo.nUnknownObjects > 0) {
				sprintf (buffer, "\n알 수 없는 객체 : %d EA\n", objectInfo.nUnknownObjects);
				fprintf (fp, buffer);
				fprintf (fp_unite, buffer);
			}

			fclose (fp);

			// 3D 투영 정보 ==================================
			if (result == DG_OK) {
				API_3DProjectionInfo  proj3DInfo_beforeCapture;

				BNZeroMemory (&proj3DInfo_beforeCapture, sizeof (API_3DProjectionInfo));
				err = ACAPI_Environment (APIEnv_Get3DProjectionSetsID, &proj3DInfo_beforeCapture, NULL);


				API_3DProjectionInfo  proj3DInfo;

				BNZeroMemory (&proj3DInfo, sizeof (API_3DProjectionInfo));
				err = ACAPI_Environment (APIEnv_Get3DProjectionSetsID, &proj3DInfo, NULL);

				double	lowestZ, highestZ, cameraZ, targetZ;	// 가장 낮은 높이, 가장 높은 높이, 카메라 및 대상 높이
				API_Coord3D		p1, p2, p3, p4, p5;				// 점 좌표 저장
				double	distanceOfPoints;						// 두 점 간의 거리
				double	angleOfPoints;							// 두 점 간의 각도
				API_Coord3D		camPos1, camPos2;				// 카메라가 있을 수 있는 점 2개

				API_FileSavePars		fsp;			// 파일 저장을 위한 변수
				API_SavePars_Picture	pars_pict;		// 그림 파일에 대한 설명

				if (err == NoError && proj3DInfo.isPersp) {
					// 벽 타입 레이어의 경우
					if (layerType == WALL) {
						// 높이 값의 범위를 구함
						// 가장 작은 x값을 갖는 점 p1도 찾아냄
						lowestZ = highestZ = vecPos [0].z;
						p1 = vecPos [0];
						for (xx = 1 ; xx < vecPos.size () ; ++xx) {
							if (lowestZ > vecPos [xx].z)	lowestZ = vecPos [xx].z;
							if (highestZ < vecPos [xx].z)	highestZ = vecPos [xx].z;

							if (vecPos [xx].x < p1.x)	p1 = vecPos [xx];
						}
						cameraZ = (highestZ - lowestZ)/2 + workLevel_object;

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
						ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);
						BNZeroMemory (&fsp, sizeof (API_FileSavePars));
						fsp.fileTypeID = APIFType_PNGFile;
						sprintf (filename, "%s - 캡쳐 (1).png", fullLayerName);
						fsp.file = new IO::Location (location, IO::Name (filename));

						BNZeroMemory (&pars_pict, sizeof (API_SavePars_Picture));
						pars_pict.colorDepth	= APIColorDepth_TC24;
						pars_pict.dithered		= false;
						pars_pict.view2D		= false;
						pars_pict.crop			= false;
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
						ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);
						BNZeroMemory (&fsp, sizeof (API_FileSavePars));
						fsp.fileTypeID = APIFType_PNGFile;
						sprintf (filename, "%s - 캡쳐 (2).png", fullLayerName);
						fsp.file = new IO::Location (location, IO::Name (filename));

						BNZeroMemory (&pars_pict, sizeof (API_SavePars_Picture));
						pars_pict.colorDepth	= APIColorDepth_TC24;
						pars_pict.dithered		= false;
						pars_pict.view2D		= false;
						pars_pict.crop			= false;
						err = ACAPI_Automate (APIDo_SaveID, &fsp, &pars_pict);	// 데모 버전에서는 작동하지 않음
					
						delete fsp.file;
					}
					// 슬래브 타입 레이어의 경우
					else if (layerType == SLAB) {
						// p1: 가장 작은 x을 찾음
						// p2: 가장 작은 y를 찾음
						// p3: 가장 큰 x를 찾음
						// p4: 가장 큰 y를 찾음
						// lowestZ: 가장 작은 z를 찾음
						// highestZ: 가장 높은 z를 찾음
						p1 = vecPos [0];
						p2 = vecPos [0];
						p3 = vecPos [0];
						p4 = vecPos [0];
						lowestZ = highestZ = vecPos [0].z;
						for (xx = 1 ; xx < vecPos.size () ; ++xx) {
							if (lowestZ > vecPos [xx].z)	lowestZ = vecPos [xx].z;
							if (highestZ < vecPos [xx].z)	highestZ = vecPos [xx].z;
							if (vecPos [xx].x < p1.x)	p1 = vecPos [xx];
							if (vecPos [xx].y < p2.y)	p2 = vecPos [xx];
							if (vecPos [xx].x > p3.x)	p3 = vecPos [xx];
							if (vecPos [xx].y > p4.y)	p4 = vecPos [xx];
						}

						// p5: 면의 중심 찾기
						p5.x = (p1.x + p3.x) / 2;
						p5.y = (p2.y + p4.y) / 2;
						p5.z = lowestZ;

						// p1과 p3 간의 거리, p2와 p4 간의 거리 중 가장 먼 거리를 찾음
						if (GetDistance (p1, p3) > GetDistance (p2, p4))
							distanceOfPoints = GetDistance (p1, p3);
						else
							distanceOfPoints = GetDistance (p2, p4);

						// 슬래브 회전 각도를 구함 (p1과 p3 간의 각도 - 45도)
						angleOfPoints = RadToDegree (atan2 ((p3.y - p1.y), (p3.x - p1.x))) - 45.0;

						// 카메라 높이, 대상 높이 지정
						cameraZ = lowestZ - (distanceOfPoints * 10) + workLevel_object;		// 이 값에 의해 실질적으로 거리가 달라짐
						targetZ = highestZ + (distanceOfPoints * 2) + workLevel_object;

						// 카메라 및 대상 위치 설정
						proj3DInfo.isPersp = true;						// 퍼스펙티브 뷰
						proj3DInfo.u.persp.viewCone = 90.0;				// 카메라 시야각
						proj3DInfo.u.persp.rollAngle = 0.0;				// 카메라 롤 각도
						proj3DInfo.u.persp.azimuth = angleOfPoints;		// 카메라 방위각
						proj3DInfo.u.persp.distance = (targetZ - cameraZ) * 1000;	// 거리

						proj3DInfo.u.persp.pos.x = p5.x;
						proj3DInfo.u.persp.pos.y = p5.y;
						proj3DInfo.u.persp.cameraZ = cameraZ;

						proj3DInfo.u.persp.target.x = p5.x + 0.010;		// 카메라와 대상 간의 X, Y 좌표가 정확하게 일치한 채로 고도 차이만 있으면 캡쳐에 실패하므로 갭이 있어야 함
						proj3DInfo.u.persp.target.y = p5.y;
						proj3DInfo.u.persp.targetZ = targetZ;

						err = ACAPI_Environment (APIEnv_Change3DProjectionSetsID, &proj3DInfo, NULL, NULL);

						// ========== 1번째 캡쳐
						// 화면 새로고침
						ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
						ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

						// 화면 캡쳐
						ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);
						BNZeroMemory (&fsp, sizeof (API_FileSavePars));
						fsp.fileTypeID = APIFType_PNGFile;
						sprintf (filename, "%s - 캡쳐 (1).png", fullLayerName);
						fsp.file = new IO::Location (location, IO::Name (filename));

						BNZeroMemory (&pars_pict, sizeof (API_SavePars_Picture));
						pars_pict.colorDepth	= APIColorDepth_TC24;
						pars_pict.dithered		= false;
						pars_pict.view2D		= false;
						pars_pict.crop			= false;
						err = ACAPI_Automate (APIDo_SaveID, &fsp, &pars_pict);	// 데모 버전에서는 작동하지 않음
					
						delete fsp.file;

						// ========== 2번째 캡쳐
						proj3DInfo.u.persp.rollAngle = 90.0;			// 카메라 롤 각도

						err = ACAPI_Environment (APIEnv_Change3DProjectionSetsID, &proj3DInfo, NULL, NULL);

						// 화면 새로고침
						ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
						ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

						// 화면 캡쳐
						ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);
						BNZeroMemory (&fsp, sizeof (API_FileSavePars));
						fsp.fileTypeID = APIFType_PNGFile;
						sprintf (filename, "%s - 캡쳐 (2).png", fullLayerName);
						fsp.file = new IO::Location (location, IO::Name (filename));

						BNZeroMemory (&pars_pict, sizeof (API_SavePars_Picture));
						pars_pict.colorDepth	= APIColorDepth_TC24;
						pars_pict.dithered		= false;
						pars_pict.view2D		= false;
						pars_pict.crop			= false;
						err = ACAPI_Automate (APIDo_SaveID, &fsp, &pars_pict);	// 데모 버전에서는 작동하지 않음
					
						delete fsp.file;

						// 롤 각도 초기화
						proj3DInfo.u.persp.rollAngle = 0.0;			// 카메라 롤 각도

						err = ACAPI_Environment (APIEnv_Change3DProjectionSetsID, &proj3DInfo, NULL, NULL);
					}
					// 기둥 타입 레이어의 경우
					else if (layerType == COLU) {
						// p1: 가장 작은 x을 찾음
						// p2: 가장 작은 y를 찾음
						// p3: 가장 큰 x를 찾음
						// p4: 가장 큰 y를 찾음
						// lowestZ: 가장 작은 z를 찾음
						// highestZ: 가장 높은 z를 찾음
						p1 = vecPos [0];
						p2 = vecPos [0];
						p3 = vecPos [0];
						p4 = vecPos [0];
						lowestZ = highestZ = vecPos [0].z;
						for (xx = 1 ; xx < vecPos.size () ; ++xx) {
							if (lowestZ > vecPos [xx].z)	lowestZ = vecPos [xx].z;
							if (highestZ < vecPos [xx].z)	highestZ = vecPos [xx].z;
							if (vecPos [xx].x < p1.x)	p1 = vecPos [xx];
							if (vecPos [xx].y < p2.y)	p2 = vecPos [xx];
							if (vecPos [xx].x > p3.x)	p3 = vecPos [xx];
							if (vecPos [xx].y > p4.y)	p4 = vecPos [xx];
						}

						// p5: 면의 중심 찾기
						p5.x = (p1.x + p3.x) / 2;
						p5.y = (p2.y + p4.y) / 2;
						p5.z = lowestZ;

						// p1과 p3 간의 거리, p2와 p4 간의 거리 중 가장 먼 거리를 찾음
						if (GetDistance (p1, p3) > GetDistance (p2, p4))
							distanceOfPoints = GetDistance (p1, p3);
						else
							distanceOfPoints = GetDistance (p2, p4);

						// 기둥 회전 각도를 구함 (p1과 p3 간의 각도 - 45도)
						angleOfPoints = RadToDegree (atan2 ((p3.y - p1.y), (p3.x - p1.x))) - 45.0;

						// 카메라 높이, 대상 높이 지정
						targetZ = cameraZ = (highestZ - lowestZ)/2 + workLevel_object;

						// ========== 1번째 캡쳐 (북쪽에서)
						// 카메라 및 대상 위치 설정
						proj3DInfo.isPersp = true;						// 퍼스펙티브 뷰
						proj3DInfo.u.persp.viewCone = 90.0;				// 카메라 시야각
						proj3DInfo.u.persp.rollAngle = 0.0;				// 카메라 롤 각도
						proj3DInfo.u.persp.azimuth = 270.0;				// 카메라 방위각
						proj3DInfo.u.persp.distance = distanceOfPoints * 2;		// 거리

						proj3DInfo.u.persp.pos.x = p5.x;
						proj3DInfo.u.persp.pos.y = p5.y + distanceOfPoints;
						proj3DInfo.u.persp.cameraZ = cameraZ;

						proj3DInfo.u.persp.target.x = p5.x;
						proj3DInfo.u.persp.target.y = p5.y - distanceOfPoints;
						proj3DInfo.u.persp.targetZ = targetZ;

						err = ACAPI_Environment (APIEnv_Change3DProjectionSetsID, &proj3DInfo, NULL, NULL);

						// 화면 새로고침
						ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
						ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

						// 화면 캡쳐
						ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);
						BNZeroMemory (&fsp, sizeof (API_FileSavePars));
						fsp.fileTypeID = APIFType_PNGFile;
						sprintf (filename, "%s - 캡쳐 (1).png", fullLayerName);
						fsp.file = new IO::Location (location, IO::Name (filename));

						BNZeroMemory (&pars_pict, sizeof (API_SavePars_Picture));
						pars_pict.colorDepth	= APIColorDepth_TC24;
						pars_pict.dithered		= false;
						pars_pict.view2D		= false;
						pars_pict.crop			= false;
						err = ACAPI_Automate (APIDo_SaveID, &fsp, &pars_pict);	// 데모 버전에서는 작동하지 않음
					
						delete fsp.file;

						// ========== 2번째 캡쳐 (남쪽에서)
						// 카메라 및 대상 위치 설정
						proj3DInfo.isPersp = true;						// 퍼스펙티브 뷰
						proj3DInfo.u.persp.viewCone = 90.0;				// 카메라 시야각
						proj3DInfo.u.persp.rollAngle = 0.0;				// 카메라 롤 각도
						proj3DInfo.u.persp.azimuth = 90.0;				// 카메라 방위각
						proj3DInfo.u.persp.distance = distanceOfPoints * 2;		// 거리

						proj3DInfo.u.persp.pos.x = p5.x;
						proj3DInfo.u.persp.pos.y = p5.y - distanceOfPoints;
						proj3DInfo.u.persp.cameraZ = cameraZ;

						proj3DInfo.u.persp.target.x = p5.x;
						proj3DInfo.u.persp.target.y = p5.y + distanceOfPoints;
						proj3DInfo.u.persp.targetZ = targetZ;

						err = ACAPI_Environment (APIEnv_Change3DProjectionSetsID, &proj3DInfo, NULL, NULL);

						// 화면 새로고침
						ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
						ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

						// 화면 캡쳐
						ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);
						BNZeroMemory (&fsp, sizeof (API_FileSavePars));
						fsp.fileTypeID = APIFType_PNGFile;
						sprintf (filename, "%s - 캡쳐 (2).png", fullLayerName);
						fsp.file = new IO::Location (location, IO::Name (filename));

						BNZeroMemory (&pars_pict, sizeof (API_SavePars_Picture));
						pars_pict.colorDepth	= APIColorDepth_TC24;
						pars_pict.dithered		= false;
						pars_pict.view2D		= false;
						pars_pict.crop			= false;
						err = ACAPI_Automate (APIDo_SaveID, &fsp, &pars_pict);	// 데모 버전에서는 작동하지 않음
					
						delete fsp.file;

						// ========== 3번째 캡쳐 (동쪽에서)
						// 카메라 및 대상 위치 설정
						proj3DInfo.isPersp = true;						// 퍼스펙티브 뷰
						proj3DInfo.u.persp.viewCone = 90.0;				// 카메라 시야각
						proj3DInfo.u.persp.rollAngle = 0.0;				// 카메라 롤 각도
						proj3DInfo.u.persp.azimuth = 180.0;				// 카메라 방위각
						proj3DInfo.u.persp.distance = distanceOfPoints * 2;		// 거리

						proj3DInfo.u.persp.pos.x = p5.x + distanceOfPoints;
						proj3DInfo.u.persp.pos.y = p5.y;
						proj3DInfo.u.persp.cameraZ = cameraZ;

						proj3DInfo.u.persp.target.x = p5.x - distanceOfPoints;
						proj3DInfo.u.persp.target.y = p5.y;
						proj3DInfo.u.persp.targetZ = targetZ;

						err = ACAPI_Environment (APIEnv_Change3DProjectionSetsID, &proj3DInfo, NULL, NULL);

						// 화면 새로고침
						ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
						ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

						// 화면 캡쳐
						ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);
						BNZeroMemory (&fsp, sizeof (API_FileSavePars));
						fsp.fileTypeID = APIFType_PNGFile;
						sprintf (filename, "%s - 캡쳐 (3).png", fullLayerName);
						fsp.file = new IO::Location (location, IO::Name (filename));

						BNZeroMemory (&pars_pict, sizeof (API_SavePars_Picture));
						pars_pict.colorDepth	= APIColorDepth_TC24;
						pars_pict.dithered		= false;
						pars_pict.view2D		= false;
						pars_pict.crop			= false;
						err = ACAPI_Automate (APIDo_SaveID, &fsp, &pars_pict);	// 데모 버전에서는 작동하지 않음
					
						delete fsp.file;

						// ========== 4번째 캡쳐 (서쪽에서)
						// 카메라 및 대상 위치 설정
						proj3DInfo.isPersp = true;						// 퍼스펙티브 뷰
						proj3DInfo.u.persp.viewCone = 90.0;				// 카메라 시야각
						proj3DInfo.u.persp.rollAngle = 0.0;				// 카메라 롤 각도
						proj3DInfo.u.persp.azimuth = 0.0;				// 카메라 방위각
						proj3DInfo.u.persp.distance = distanceOfPoints * 2;		// 거리

						proj3DInfo.u.persp.pos.x = p5.x - distanceOfPoints;
						proj3DInfo.u.persp.pos.y = p5.y;
						proj3DInfo.u.persp.cameraZ = cameraZ;

						proj3DInfo.u.persp.target.x = p5.x + distanceOfPoints;
						proj3DInfo.u.persp.target.y = p5.y;
						proj3DInfo.u.persp.targetZ = targetZ;

						err = ACAPI_Environment (APIEnv_Change3DProjectionSetsID, &proj3DInfo, NULL, NULL);

						// 화면 새로고침
						ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
						ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

						// 화면 캡쳐
						ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);
						BNZeroMemory (&fsp, sizeof (API_FileSavePars));
						fsp.fileTypeID = APIFType_PNGFile;
						sprintf (filename, "%s - 캡쳐 (4).png", fullLayerName);
						fsp.file = new IO::Location (location, IO::Name (filename));

						BNZeroMemory (&pars_pict, sizeof (API_SavePars_Picture));
						pars_pict.colorDepth	= APIColorDepth_TC24;
						pars_pict.dithered		= false;
						pars_pict.view2D		= false;
						pars_pict.crop			= false;
						err = ACAPI_Automate (APIDo_SaveID, &fsp, &pars_pict);	// 데모 버전에서는 작동하지 않음
					
						delete fsp.file;
					}
					// 보, 눈썹보 타입 레이어의 경우
					else if ((layerType == BEAM) || (layerType == WLBM)) {
						// 높이 값의 범위를 구함
						// 가장 작은 x값을 갖는 점 p1도 찾아냄
						lowestZ = highestZ = vecPos [0].z;
						p1 = vecPos [0];
						for (xx = 1 ; xx < vecPos.size () ; ++xx) {
							if (lowestZ > vecPos [xx].z)	lowestZ = vecPos [xx].z;
							if (highestZ < vecPos [xx].z)	highestZ = vecPos [xx].z;

							if (vecPos [xx].x < p1.x)	p1 = vecPos [xx];
						}
						cameraZ = (highestZ - lowestZ)/2 + workLevel_object;

						distanceOfPoints = 0.0;
						for (xx = 0 ; xx < vecPos.size () ; ++xx) {
							if (distanceOfPoints < GetDistance (p1, vecPos [xx])) {
								distanceOfPoints = GetDistance (p1, vecPos [xx]);
								p2 = vecPos [xx];
							}
						}

						// 두 점(p1, p2) 간의 각도 구하기
						angleOfPoints = RadToDegree (atan2 ((p2.y - p1.y), (p2.x - p1.x)));

						// 중심점 구하기
						p3 = p1;
						moveIn3D ('x', DegreeToRad (angleOfPoints), distanceOfPoints/2, &p3.x, &p3.y, &p3.z);
						p3.z += workLevel_object;

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
						ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);
						BNZeroMemory (&fsp, sizeof (API_FileSavePars));
						fsp.fileTypeID = APIFType_PNGFile;
						sprintf (filename, "%s - 캡쳐 (1).png", fullLayerName);
						fsp.file = new IO::Location (location, IO::Name (filename));

						BNZeroMemory (&pars_pict, sizeof (API_SavePars_Picture));
						pars_pict.colorDepth	= APIColorDepth_TC24;
						pars_pict.dithered		= false;
						pars_pict.view2D		= false;
						pars_pict.crop			= false;
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
						ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);
						BNZeroMemory (&fsp, sizeof (API_FileSavePars));
						fsp.fileTypeID = APIFType_PNGFile;
						sprintf (filename, "%s - 캡쳐 (2).png", fullLayerName);
						fsp.file = new IO::Location (location, IO::Name (filename));

						BNZeroMemory (&pars_pict, sizeof (API_SavePars_Picture));
						pars_pict.colorDepth	= APIColorDepth_TC24;
						pars_pict.dithered		= false;
						pars_pict.view2D		= false;
						pars_pict.crop			= false;
						err = ACAPI_Automate (APIDo_SaveID, &fsp, &pars_pict);	// 데모 버전에서는 작동하지 않음
					
						delete fsp.file;

						// ========== 3번째 캡쳐
						// 카메라 및 대상 위치 설정
						proj3DInfo.isPersp = true;				// 퍼스펙티브 뷰
						proj3DInfo.u.persp.viewCone = 90.0;		// 카메라 시야각
						proj3DInfo.u.persp.rollAngle = 0.0;		// 카메라 롤 각도
						proj3DInfo.u.persp.azimuth = angleOfPoints - 90.0;	// 카메라 방위각

						proj3DInfo.u.persp.pos.x = p3.x;
						proj3DInfo.u.persp.pos.y = p3.y;
						proj3DInfo.u.persp.cameraZ = p3.z - distanceOfPoints;

						proj3DInfo.u.persp.target.x = p3.x - 0.001;
						proj3DInfo.u.persp.target.y = p3.y;
						proj3DInfo.u.persp.targetZ = p3.z;

						err = ACAPI_Environment (APIEnv_Change3DProjectionSetsID, &proj3DInfo, NULL, NULL);

						// 화면 새로고침
						ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
						ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

						// 화면 캡쳐
						ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);
						BNZeroMemory (&fsp, sizeof (API_FileSavePars));
						fsp.fileTypeID = APIFType_PNGFile;
						sprintf (filename, "%s - 캡쳐 (3).png", fullLayerName);
						fsp.file = new IO::Location (location, IO::Name (filename));

						BNZeroMemory (&pars_pict, sizeof (API_SavePars_Picture));
						pars_pict.colorDepth	= APIColorDepth_TC24;
						pars_pict.dithered		= false;
						pars_pict.view2D		= false;
						pars_pict.crop			= false;
						err = ACAPI_Automate (APIDo_SaveID, &fsp, &pars_pict);	// 데모 버전에서는 작동하지 않음
					
						delete fsp.file;
					}
				}

				// 화면을 캡쳐 이전 상태로 돌려놓음
				err = ACAPI_Environment (APIEnv_Change3DProjectionSetsID, &proj3DInfo_beforeCapture, NULL, NULL);

				// 화면 새로고침
				ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
				ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);
				// 3D 투영 정보 ==================================
			}


			// 레이어 숨기기
			attrib.layer.head.flags |= APILay_Hidden;
			ACAPI_Attribute_Modify (&attrib, NULL);
		}

		// 진행 상황 표시하는 기능 - 진행
		cur = mm;
		ACAPI_Interface (APIIo_SetProcessValueID, &cur, NULL);
		if (ACAPI_Interface (APIIo_IsProcessCanceledID, NULL, NULL) == APIERR_CANCEL)
			break;
	}

	// 진행 상황 표시하는 기능 - 마무리
	ACAPI_Interface (APIIo_CloseProcessWindowID, NULL, NULL);

	fclose (fp_unite);

	// 모든 프로세스를 마치면 처음에 수집했던 보이는 레이어들을 다시 켜놓을 것
	for (xx = 1 ; xx <= nVisibleLayers ; ++xx) {
		BNZeroMemory (&attrib, sizeof (API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = visLayerList [xx-1];
		err = ACAPI_Attribute_Get (&attrib);
		if (err == NoError) {
			if ((attrib.layer.head.flags & APILay_Hidden) == true) {
				attrib.layer.head.flags ^= APILay_Hidden;
				ACAPI_Attribute_Modify (&attrib, NULL);
			}
		}
	}

	// 화면 새로고침
	ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
	ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

	ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);
	location.ToDisplayText (&resultString);
	sprintf (buffer, "결과물을 다음 위치에 저장했습니다.\n\n%s\n또는 프로젝트 파일이 있는 폴더", resultString.ToCStr ().Get ());
	ACAPI_WriteReport (buffer, true);

	return	err;
}

// 부재별 선택 후 보여주기
GSErrCode	filterSelection (void)
{
	GSErrCode	err = NoError;
	short		xx, yy;
	short		result;
	const char*	tempStr;
	bool		foundObj;
	bool		suspGrp;

	short		selCount;
	API_Neig**	selNeig;

	FILE	*fp;				// 파일 포인터
	char	line [10240];		// 파일에서 읽어온 라인 하나
	char	*token;				// 읽어온 문자열의 토큰
	short	lineCount;			// 읽어온 라인 수
	short	tokCount;			// 읽어온 토큰 개수
	char	nthToken [200][50];	// n번째 토큰

	API_Element			elem;
	API_ElementMemo		memo;

	// GUID 저장을 위한 변수
	GS::Array<API_Guid>	objects	= GS::Array<API_Guid> ();	long nObjects	= 0;
	GS::Array<API_Guid>	walls	= GS::Array<API_Guid> ();	long nWalls		= 0;
	GS::Array<API_Guid>	columns	= GS::Array<API_Guid> ();	long nColumns	= 0;
	GS::Array<API_Guid>	beams	= GS::Array<API_Guid> ();	long nBeams		= 0;
	GS::Array<API_Guid>	slabs	= GS::Array<API_Guid> ();	long nSlabs		= 0;
	GS::Array<API_Guid>	roofs	= GS::Array<API_Guid> ();	long nRoofs		= 0;
	GS::Array<API_Guid>	meshes	= GS::Array<API_Guid> ();	long nMeshes	= 0;
	GS::Array<API_Guid>	morphs	= GS::Array<API_Guid> ();	long nMorphs	= 0;
	GS::Array<API_Guid>	shells	= GS::Array<API_Guid> ();	long nShells	= 0;

	// 조건에 맞는 객체들의 GUID 저장
	GS::Array<API_Guid> selection_known = GS::Array<API_Guid> ();
	GS::Array<API_Guid> selection_unknown = GS::Array<API_Guid> ();

	
	// 그룹화 일시정지 ON
	ACAPI_Environment (APIEnv_IsSuspendGroupOnID, &suspGrp);
	if (suspGrp == false)	ACAPI_Element_Tool (NULL, NULL, APITool_SuspendGroups, NULL);

	ACAPI_Element_GetElemList (API_ObjectID, &objects, APIFilt_OnVisLayer | APIFilt_In3D);	nObjects = objects.GetSize ();	// 보이는 레이어 상의 객체 타입만 가져오기
	ACAPI_Element_GetElemList (API_WallID, &walls, APIFilt_OnVisLayer | APIFilt_In3D);		nWalls = walls.GetSize ();		// 보이는 레이어 상의 벽 타입만 가져오기
	ACAPI_Element_GetElemList (API_ColumnID, &columns, APIFilt_OnVisLayer | APIFilt_In3D);	nColumns = columns.GetSize ();	// 보이는 레이어 상의 기둥 타입만 가져오기
	ACAPI_Element_GetElemList (API_BeamID, &beams, APIFilt_OnVisLayer | APIFilt_In3D);		nBeams = beams.GetSize ();		// 보이는 레이어 상의 보 타입만 가져오기
	ACAPI_Element_GetElemList (API_SlabID, &slabs, APIFilt_OnVisLayer | APIFilt_In3D);		nSlabs = slabs.GetSize ();		// 보이는 레이어 상의 슬래브 타입만 가져오기
	ACAPI_Element_GetElemList (API_RoofID, &roofs, APIFilt_OnVisLayer | APIFilt_In3D);		nRoofs = roofs.GetSize ();		// 보이는 레이어 상의 루프 타입만 가져오기
	ACAPI_Element_GetElemList (API_MeshID, &meshes, APIFilt_OnVisLayer | APIFilt_In3D);		nMeshes = meshes.GetSize ();	// 보이는 레이어 상의 메시 타입만 가져오기
	ACAPI_Element_GetElemList (API_MorphID, &morphs, APIFilt_OnVisLayer | APIFilt_In3D);	nMorphs = morphs.GetSize ();	// 보이는 레이어 상의 모프 타입만 가져오기
	ACAPI_Element_GetElemList (API_ShellID, &shells, APIFilt_OnVisLayer | APIFilt_In3D);	nShells = shells.GetSize ();	// 보이는 레이어 상의 셸 타입만 가져오기

	if (nObjects == 0 && nWalls == 0 && nColumns == 0 && nBeams == 0 && nSlabs == 0 && nRoofs == 0 && nMeshes == 0 && nMorphs == 0 && nShells == 0) {
		result = DGAlert (DG_INFORMATION, "종료 알림", "아무 객체도 존재하지 않습니다.", "", "확인", "", "");
		return	err;
	}

	// 객체 정보 파일 가져오기
	fp = fopen ("C:\\objectInfo.csv", "r");

	if (fp == NULL) {
		result = DGAlert (DG_WARNING, "파일 오류", "objectInfo.csv 파일을 C:\\로 복사하십시오.", "", "확인", "", "");
		return	err;
	}

	lineCount = 0;

	while (!feof (fp)) {
		tokCount = 0;
		fgets (line, sizeof (line), fp);

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

		sprintf (visibleObjInfo.varName [lineCount-1], "%s", nthToken [0]);
		sprintf (visibleObjInfo.objName [lineCount-1], "%s", nthToken [1]);
	}

	visibleObjInfo.nKinds = lineCount;

	// 끝에 같은 항목이 2번 들어갈 수 있으므로 중복 제거
	if (lineCount >= 2) {
		if (my_strcmp (visibleObjInfo.varName [lineCount-1], visibleObjInfo.varName [lineCount-2]) == 0) {
			visibleObjInfo.nKinds --;
		}
	}

	// 파일 닫기
	fclose (fp);

	// 존재 여부, 표시 여부 초기화
	for (xx = 0 ; xx < 50 ; ++xx) {
		visibleObjInfo.bExist [xx] = false;
		visibleObjInfo.bShow [xx] = false;
	}
	visibleObjInfo.bExist_Walls = false;
	visibleObjInfo.bShow_Walls = false;
	visibleObjInfo.bExist_Columns = false;
	visibleObjInfo.bShow_Columns = false;
	visibleObjInfo.bExist_Beams = false;
	visibleObjInfo.bShow_Beams = false;
	visibleObjInfo.bExist_Slabs = false;
	visibleObjInfo.bShow_Slabs = false;
	visibleObjInfo.bExist_Roofs = false;
	visibleObjInfo.bShow_Roofs = false;
	visibleObjInfo.bExist_Meshes = false;
	visibleObjInfo.bShow_Meshes = false;
	visibleObjInfo.bExist_Morphs = false;
	visibleObjInfo.bShow_Morphs = false;
	visibleObjInfo.bExist_Shells = false;
	visibleObjInfo.bShow_Shells = false;

	// 존재 여부 체크
	for (xx = 0 ; xx < nObjects ; ++xx) {
		BNZeroMemory (&elem, sizeof (API_Element));
		BNZeroMemory (&memo, sizeof (API_ElementMemo));
		elem.header.guid = objects [xx];
		err = ACAPI_Element_Get (&elem);
		err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

		foundObj = false;

		for (yy = 0 ; yy < visibleObjInfo.nKinds ; ++yy) {
			tempStr = getParameterStringByName (&memo, visibleObjInfo.varName [yy]);
			if (tempStr != NULL) {
				if (my_strcmp (tempStr, visibleObjInfo.objName [yy]) == 0) {
					visibleObjInfo.bExist [yy] = true;
					foundObj = true;
				}
			}
		}

		// 끝내 찾지 못하면 알려지지 않은 Object 타입 리스트에 추가
		if (foundObj == false)
			selection_unknown.Push (objects [xx]);

		ACAPI_DisposeElemMemoHdls (&memo);
	}
	
	visibleObjInfo.nUnknownObjects = selection_unknown.GetSize ();

	if (nWalls > 0)		visibleObjInfo.bExist_Walls = true;
	if (nColumns > 0)	visibleObjInfo.bExist_Columns = true;
	if (nBeams > 0)		visibleObjInfo.bExist_Beams = true;
	if (nSlabs > 0)		visibleObjInfo.bExist_Slabs = true;
	if (nRoofs > 0)		visibleObjInfo.bExist_Roofs = true;
	if (nMeshes > 0)	visibleObjInfo.bExist_Meshes = true;
	if (nMorphs > 0)	visibleObjInfo.bExist_Morphs = true;
	if (nShells > 0)	visibleObjInfo.bExist_Shells = true;

	visibleObjInfo.nItems = visibleObjInfo.nKinds +
		(visibleObjInfo.bExist_Walls * 1) +
		(visibleObjInfo.bExist_Columns * 1) +
		(visibleObjInfo.bExist_Beams * 1) +
		(visibleObjInfo.bExist_Slabs * 1) +
		(visibleObjInfo.bExist_Roofs * 1) +
		(visibleObjInfo.bExist_Meshes * 1) +
		(visibleObjInfo.bExist_Morphs * 1) +
		(visibleObjInfo.bExist_Shells * 1);

	// [DIALOG] 다이얼로그에서 보이는 레이어 상에 있는 객체들의 종류를 보여주고, 체크한 종류의 객체들만 선택 후 보여줌
	result = DGBlankModalDialog (750, 500, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, filterSelectionHandler, 0);

	if (result == DG_OK) {
		// 선택한 조건에 해당하는 객체들 선택하기
		for (xx = 0 ; xx < nObjects ; ++xx) {
			BNZeroMemory (&elem, sizeof (API_Element));
			BNZeroMemory (&memo, sizeof (API_ElementMemo));
			elem.header.guid = objects [xx];
			err = ACAPI_Element_Get (&elem);
			err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

			for (yy = 0 ; yy < visibleObjInfo.nKinds ; ++yy) {
				tempStr = getParameterStringByName (&memo, visibleObjInfo.varName [yy]);
				
				if (tempStr != NULL) {
					if ((my_strcmp (tempStr, visibleObjInfo.objName [yy]) == 0) && (visibleObjInfo.bShow [yy] == true)) {
						selection_known.Push (objects [xx]);
					}
				}
			}

			ACAPI_DisposeElemMemoHdls (&memo);
		}

		// 알려진 Object 타입 선택
		selCount = (short)selection_known.GetSize ();
		selNeig = (API_Neig**)(BMAllocateHandle (selCount * sizeof (API_Neig), ALLOCATE_CLEAR, 0));
		for (xx = 0 ; xx < selCount ; ++xx)
			(*selNeig)[xx].guid = selection_known [xx];

		ACAPI_Element_Select (selNeig, selCount, true);
		BMKillHandle (reinterpret_cast<GSHandle*> (&selNeig));

		// 알려지지 않은 Object 타입 선택
		if (visibleObjInfo.bShow_Unknown == true) {
			selCount = (short)selection_unknown.GetSize ();
			selNeig = (API_Neig**)(BMAllocateHandle (selCount * sizeof (API_Neig), ALLOCATE_CLEAR, 0));
			for (xx = 0 ; xx < selCount ; ++xx)
				(*selNeig)[xx].guid = selection_unknown [xx];

			ACAPI_Element_Select (selNeig, selCount, true);
			BMKillHandle (reinterpret_cast<GSHandle*> (&selNeig));
		}

		// 나머지 타입
		if (visibleObjInfo.bShow_Walls == true) {
			selCount = (short)nWalls;
			selNeig = (API_Neig**)(BMAllocateHandle (selCount * sizeof (API_Neig), ALLOCATE_CLEAR, 0));
			for (xx = 0 ; xx < selCount ; ++xx)
				(*selNeig)[xx].guid = walls [xx];

			ACAPI_Element_Select (selNeig, selCount, true);
			BMKillHandle (reinterpret_cast<GSHandle*> (&selNeig));
		}
		if (visibleObjInfo.bShow_Columns == true) {
			selCount = (short)nColumns;
			selNeig = (API_Neig**)(BMAllocateHandle (selCount * sizeof (API_Neig), ALLOCATE_CLEAR, 0));
			for (xx = 0 ; xx < selCount ; ++xx)
				(*selNeig)[xx].guid = columns [xx];

			ACAPI_Element_Select (selNeig, selCount, true);
			BMKillHandle (reinterpret_cast<GSHandle*> (&selNeig));
		}
		if (visibleObjInfo.bShow_Beams == true) {
			selCount = (short)nBeams;
			selNeig = (API_Neig**)(BMAllocateHandle (selCount * sizeof (API_Neig), ALLOCATE_CLEAR, 0));
			for (xx = 0 ; xx < selCount ; ++xx)
				(*selNeig)[xx].guid = beams [xx];

			ACAPI_Element_Select (selNeig, selCount, true);
			BMKillHandle (reinterpret_cast<GSHandle*> (&selNeig));
		}
		if (visibleObjInfo.bShow_Slabs == true) {
			selCount = (short)nSlabs;
			selNeig = (API_Neig**)(BMAllocateHandle (selCount * sizeof (API_Neig), ALLOCATE_CLEAR, 0));
			for (xx = 0 ; xx < selCount ; ++xx)
				(*selNeig)[xx].guid = slabs [xx];

			ACAPI_Element_Select (selNeig, selCount, true);
			BMKillHandle (reinterpret_cast<GSHandle*> (&selNeig));
		}
		if (visibleObjInfo.bShow_Roofs == true) {
			selCount = (short)nRoofs;
			selNeig = (API_Neig**)(BMAllocateHandle (selCount * sizeof (API_Neig), ALLOCATE_CLEAR, 0));
			for (xx = 0 ; xx < selCount ; ++xx)
				(*selNeig)[xx].guid = roofs [xx];

			ACAPI_Element_Select (selNeig, selCount, true);
			BMKillHandle (reinterpret_cast<GSHandle*> (&selNeig));
		}
		if (visibleObjInfo.bShow_Meshes == true) {
			selCount = (short)nMeshes;
			selNeig = (API_Neig**)(BMAllocateHandle (selCount * sizeof (API_Neig), ALLOCATE_CLEAR, 0));
			for (xx = 0 ; xx < selCount ; ++xx)
				(*selNeig)[xx].guid = meshes [xx];

			ACAPI_Element_Select (selNeig, selCount, true);
			BMKillHandle (reinterpret_cast<GSHandle*> (&selNeig));
		}
		if (visibleObjInfo.bShow_Morphs == true) {
			selCount = (short)nMorphs;
			selNeig = (API_Neig**)(BMAllocateHandle (selCount * sizeof (API_Neig), ALLOCATE_CLEAR, 0));
			for (xx = 0 ; xx < selCount ; ++xx)
				(*selNeig)[xx].guid = morphs [xx];

			ACAPI_Element_Select (selNeig, selCount, true);
			BMKillHandle (reinterpret_cast<GSHandle*> (&selNeig));
		}
		if (visibleObjInfo.bShow_Shells == true) {
			selCount = (short)nShells;
			selNeig = (API_Neig**)(BMAllocateHandle (selCount * sizeof (API_Neig), ALLOCATE_CLEAR, 0));
			for (xx = 0 ; xx < selCount ; ++xx)
				(*selNeig)[xx].guid = shells [xx];

			ACAPI_Element_Select (selNeig, selCount, true);
			BMKillHandle (reinterpret_cast<GSHandle*> (&selNeig));
		}
		
		// 선택한 것만 3D로 보여주기
		ACAPI_Automate (APIDo_ShowSelectionIn3DID, NULL, NULL);
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

// [다이얼로그] 다이얼로그에서 보이는 레이어 상에 있는 객체들의 종류를 보여주고, 체크한 종류의 객체들만 선택 후 보여줌
short DGCALLBACK filterSelectionHandler (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	short	xx;
	short	itmIdx;
	short	itmPosX, itmPosY;
	char	buffer [64];

	switch (message) {
		case DG_MSG_INIT:
			// 다이얼로그 타이틀
			DGSetDialogTitle (dialogID, "선택한 타입의 객체 선택 후 보여주기");

			// 확인 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 20, 10, 80, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, "확인");
			DGShowItem (dialogID, DG_OK);

			// 취소 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 120, 10, 80, 25);
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, "취소");
			DGShowItem (dialogID, DG_CANCEL);

			// 버튼: 전체선택
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 20, 50, 80, 25);
			DGSetItemFont (dialogID, BUTTON_ALL_SEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, BUTTON_ALL_SEL, "전체선택");
			DGShowItem (dialogID, BUTTON_ALL_SEL);

			// 버튼: 전체선택 해제
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 120, 50, 80, 25);
			DGSetItemFont (dialogID, BUTTON_ALL_UNSEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, BUTTON_ALL_UNSEL, "전체선택\n해제");
			DGShowItem (dialogID, BUTTON_ALL_UNSEL);

			// 체크박스: 알려지지 않은 객체 포함
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 220, 50, 250, 25);
			DGSetItemFont (dialogID, CHECKBOX_INCLUDE_UNKNOWN_OBJECT, DG_IS_LARGE | DG_IS_PLAIN);
			sprintf (buffer, "알려지지 않은 객체 포함 (%d)", visibleObjInfo.nUnknownObjects);
			DGSetItemText (dialogID, CHECKBOX_INCLUDE_UNKNOWN_OBJECT, buffer);
			DGShowItem (dialogID, CHECKBOX_INCLUDE_UNKNOWN_OBJECT);
			if (visibleObjInfo.nUnknownObjects > 0)
				DGEnableItem (dialogID, CHECKBOX_INCLUDE_UNKNOWN_OBJECT);
			else
				DGDisableItem (dialogID, CHECKBOX_INCLUDE_UNKNOWN_OBJECT);

			// 구분자
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 5, 90, 740, 1);
			DGShowItem (dialogID, itmIdx);

			// 체크박스 항목들 배치할 것
			itmPosX = 20;	itmPosY = 105;	// Y의 범위 105 ~ 500까지

			if (visibleObjInfo.bExist_Walls == true) {
				visibleObjInfo.itmIdx_Walls = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, itmPosX, itmPosY, 190, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_BOLD);
				DGSetItemText (dialogID, itmIdx, "벽");
				DGShowItem (dialogID, itmIdx);
				itmPosY += 30;
			}
			if (visibleObjInfo.bExist_Columns == true) {
				visibleObjInfo.itmIdx_Columns = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, itmPosX, itmPosY, 190, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_BOLD);
				DGSetItemText (dialogID, itmIdx, "기둥");
				DGShowItem (dialogID, itmIdx);
				itmPosY += 30;
			}
			if (visibleObjInfo.bExist_Beams == true) {
				visibleObjInfo.itmIdx_Beams = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, itmPosX, itmPosY, 190, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_BOLD);
				DGSetItemText (dialogID, itmIdx, "보");
				DGShowItem (dialogID, itmIdx);
				itmPosY += 30;
			}
			if (visibleObjInfo.bExist_Slabs == true) {
				visibleObjInfo.itmIdx_Slabs = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, itmPosX, itmPosY, 190, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_BOLD);
				DGSetItemText (dialogID, itmIdx, "슬래브");
				DGShowItem (dialogID, itmIdx);
				itmPosY += 30;
			}
			if (visibleObjInfo.bExist_Roofs == true) {
				visibleObjInfo.itmIdx_Roofs = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, itmPosX, itmPosY, 190, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_BOLD);
				DGSetItemText (dialogID, itmIdx, "루프");
				DGShowItem (dialogID, itmIdx);
				itmPosY += 30;
			}
			if (visibleObjInfo.bExist_Meshes == true) {
				visibleObjInfo.itmIdx_Meshes = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, itmPosX, itmPosY, 190, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_BOLD);
				DGSetItemText (dialogID, itmIdx, "메시");
				DGShowItem (dialogID, itmIdx);
				itmPosY += 30;
			}
			if (visibleObjInfo.bExist_Morphs == true) {
				visibleObjInfo.itmIdx_Morphs = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, itmPosX, itmPosY, 190, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_BOLD);
				DGSetItemText (dialogID, itmIdx, "모프");
				DGShowItem (dialogID, itmIdx);
				itmPosY += 30;
			}
			if (visibleObjInfo.bExist_Shells == true) {
				visibleObjInfo.itmIdx_Shells = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, itmPosX, itmPosY, 190, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_BOLD);
				DGSetItemText (dialogID, itmIdx, "셸");
				DGShowItem (dialogID, itmIdx);
				itmPosY += 30;
			}

			for (xx = 0 ; xx < visibleObjInfo.nKinds ; ++xx) {
				if (visibleObjInfo.bExist [xx] == true) {
					visibleObjInfo.itmIdx [xx] = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, itmPosX, itmPosY, 190, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, visibleObjInfo.objName [xx]);
					DGShowItem (dialogID, itmIdx);
					itmPosY += 30;

					// 1행에 12개
					if (itmPosY > 430) {
						itmPosX += 200;
						itmPosY = 105;
					}
				}
			}

			break;
		
		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					// Object 타입
					for (xx = 0 ; xx < visibleObjInfo.nKinds ; ++xx) {
						if (DGGetItemValLong (dialogID, visibleObjInfo.itmIdx [xx]) == TRUE) {
							visibleObjInfo.bShow [xx] = true;
						} else {
							visibleObjInfo.bShow [xx] = false;
						}
					}

					// 알려지지 않은 Object 타입의 객체 보이기 여부
					(DGGetItemValLong (dialogID, CHECKBOX_INCLUDE_UNKNOWN_OBJECT) == TRUE)	? visibleObjInfo.bShow_Unknown = true	: visibleObjInfo.bShow_Unknown = false;

					// 나머지 타입
					(DGGetItemValLong (dialogID, visibleObjInfo.itmIdx_Walls) == TRUE)		? visibleObjInfo.bShow_Walls = true		: visibleObjInfo.bShow_Walls = false;
					(DGGetItemValLong (dialogID, visibleObjInfo.itmIdx_Columns) == TRUE)	? visibleObjInfo.bShow_Columns = true	: visibleObjInfo.bShow_Columns = false;
					(DGGetItemValLong (dialogID, visibleObjInfo.itmIdx_Beams) == TRUE)		? visibleObjInfo.bShow_Beams = true		: visibleObjInfo.bShow_Beams = false;
					(DGGetItemValLong (dialogID, visibleObjInfo.itmIdx_Slabs) == TRUE)		? visibleObjInfo.bShow_Slabs = true		: visibleObjInfo.bShow_Slabs = false;
					(DGGetItemValLong (dialogID, visibleObjInfo.itmIdx_Roofs) == TRUE)		? visibleObjInfo.bShow_Roofs = true		: visibleObjInfo.bShow_Roofs = false;
					(DGGetItemValLong (dialogID, visibleObjInfo.itmIdx_Meshes) == TRUE)		? visibleObjInfo.bShow_Meshes = true	: visibleObjInfo.bShow_Meshes = false;
					(DGGetItemValLong (dialogID, visibleObjInfo.itmIdx_Morphs) == TRUE)		? visibleObjInfo.bShow_Morphs = true	: visibleObjInfo.bShow_Morphs = false;
					(DGGetItemValLong (dialogID, visibleObjInfo.itmIdx_Shells) == TRUE)		? visibleObjInfo.bShow_Shells = true	: visibleObjInfo.bShow_Shells = false;

					break;

				case DG_CANCEL:
					break;

				case BUTTON_ALL_SEL:
					item = 0;

					// 모든 체크박스를 켬
					if (visibleObjInfo.bExist_Walls == true)	DGSetItemValLong (dialogID, visibleObjInfo.itmIdx_Walls, TRUE);
					if (visibleObjInfo.bExist_Columns == true)	DGSetItemValLong (dialogID, visibleObjInfo.itmIdx_Columns, TRUE);
					if (visibleObjInfo.bExist_Beams == true)	DGSetItemValLong (dialogID, visibleObjInfo.itmIdx_Beams, TRUE);
					if (visibleObjInfo.bExist_Slabs == true)	DGSetItemValLong (dialogID, visibleObjInfo.itmIdx_Slabs, TRUE);
					if (visibleObjInfo.bExist_Roofs == true)	DGSetItemValLong (dialogID, visibleObjInfo.itmIdx_Roofs, TRUE);
					if (visibleObjInfo.bExist_Meshes == true)	DGSetItemValLong (dialogID, visibleObjInfo.itmIdx_Meshes, TRUE);
					if (visibleObjInfo.bExist_Morphs == true)	DGSetItemValLong (dialogID, visibleObjInfo.itmIdx_Morphs, TRUE);
					if (visibleObjInfo.bExist_Shells == true)	DGSetItemValLong (dialogID, visibleObjInfo.itmIdx_Shells, TRUE);
					for (xx = 0 ; xx < visibleObjInfo.nKinds ; ++xx) {
						if (visibleObjInfo.bExist [xx] == true) {
							DGSetItemValLong (dialogID, visibleObjInfo.itmIdx [xx], TRUE);
						}
					}

					break;

				case BUTTON_ALL_UNSEL:
					item = 0;

					// 모든 체크박스를 끔
					if (visibleObjInfo.bExist_Walls == true)	DGSetItemValLong (dialogID, visibleObjInfo.itmIdx_Walls, FALSE);
					if (visibleObjInfo.bExist_Columns == true)	DGSetItemValLong (dialogID, visibleObjInfo.itmIdx_Columns, FALSE);
					if (visibleObjInfo.bExist_Beams == true)	DGSetItemValLong (dialogID, visibleObjInfo.itmIdx_Beams, FALSE);
					if (visibleObjInfo.bExist_Slabs == true)	DGSetItemValLong (dialogID, visibleObjInfo.itmIdx_Slabs, FALSE);
					if (visibleObjInfo.bExist_Roofs == true)	DGSetItemValLong (dialogID, visibleObjInfo.itmIdx_Roofs, FALSE);
					if (visibleObjInfo.bExist_Meshes == true)	DGSetItemValLong (dialogID, visibleObjInfo.itmIdx_Meshes, FALSE);
					if (visibleObjInfo.bExist_Morphs == true)	DGSetItemValLong (dialogID, visibleObjInfo.itmIdx_Morphs, FALSE);
					if (visibleObjInfo.bExist_Shells == true)	DGSetItemValLong (dialogID, visibleObjInfo.itmIdx_Shells, FALSE);
					for (xx = 0 ; xx < visibleObjInfo.nKinds ; ++xx) {
						if (visibleObjInfo.bExist [xx] == true) {
							DGSetItemValLong (dialogID, visibleObjInfo.itmIdx [xx], FALSE);
						}
					}

					break;

				default:
					item = 0;
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
