#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "Export.hpp"

using namespace exportDG;

VisibleObjectInfo	visibleObjInfo;			// 보이는 레이어 상의 객체별 명칭, 존재 여부, 보이기 여부
static short	EDITCONTROL_SCALE_VALUE;	// 축척 값을 입력하는 Edit컨트롤의 ID 값


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

// vector 내 자재 정보 구조체 정렬을 위한 비교 함수 (좌표값 X 기준)
bool comparePosX (const objectInBeamTableform& a, const objectInBeamTableform& b)
{
	return	(a.minPos.x + (a.maxPos.x - a.minPos.x)/2) < (b.minPos.x + (b.maxPos.x - b.minPos.x)/2);
}

// vector 내 자재 정보 구조체 정렬을 위한 비교 함수 (좌표값 Y 기준)
bool comparePosY (const objectInBeamTableform& a, const objectInBeamTableform& b)
{
	return	(a.minPos.y + (a.maxPos.y - a.minPos.y)/2) < (b.minPos.y + (b.maxPos.y - b.minPos.y)/2);
}

// vector 내 레이어 정보 구조체 정렬을 위한 비교 함수 (레이어 이름 기준)
bool compareLayerName (const LayerList& a, const LayerList& b)
{
	if (a.layerName.compare (b.layerName) < 0)
		return true;
	else
		return false;
}

// vector 내 레코드 내 필드를 기준으로 내림차순 정렬을 위한 비교 함수
bool compareVectorString (const vector<string>& a, const vector<string>& b)
{
	int	xx;
	size_t	aLen = a.size ();
	size_t	bLen = b.size ();

	const char* aStr;
	const char* bStr;

	// a와 b의 0번째 문자열이 같고, 벡터의 길이도 같을 경우 계속 진행
	if (aLen == bLen) {
		// 1번째 문자열부터 n번째 문자열까지 순차적으로 비교함
		for (xx = 0 ; xx < aLen ; ++xx) {
			aStr = a.at(xx).c_str ();
			bStr = b.at(xx).c_str ();

			// 숫자로 된 문자열이면,
			if ((isStringDouble (aStr) == TRUE) && (isStringDouble (bStr) == TRUE)) {
				if (atof (aStr) - atof (bStr) > EPS)
					return true;
				else if (atof (bStr) - atof (aStr) > EPS)
					return false;
				else
					continue;
			} else {
				if (my_strcmp (aStr, bStr) > 0)
					return true;
				else if (my_strcmp (aStr, bStr) < 0)
					return false;
				else
					continue;
			}
		}
	} else {
		if (aLen > bLen)
			return true;
		else
			return false;
	}

	return	true;
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
		DGAlert (DG_ERROR, L"오류", L"objectInfo.csv 파일을 C:\\로 복사하십시오.", "", L"확인", "", "");
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
	char	tempStr [512];

	vecLen = this->records.size ();

	try {
		for (xx = 0 ; xx < vecLen ; ++xx) {
			// 변수 값도 동일할 경우
			inVecLen1 = this->records.at(xx).size () - 1;		// 끝의 개수 필드를 제외한 길이
			inVecLen2 = record.size ();

			if (inVecLen1 == inVecLen2) {
				// 일치하지 않는 필드가 하나라도 있는지 찾아볼 것
				diff = 0;
				for (yy = 0 ; yy < inVecLen1 ; ++yy) {
					if (my_strcmp (this->records.at(xx).at(yy).c_str (), record.at(yy).c_str ()) != 0)
						diff++;
				}

				// 모든 필드가 일치하면
				if (diff == 0) {
					value = atoi (this->records.at(xx).back ().c_str ());
					value ++;
					sprintf (tempStr, "%d", value);
					this->records.at(xx).pop_back ();
					this->records.at(xx).push_back (tempStr);
					return value;
				}
			}
		}
	} catch (exception& ex) {
		WriteReport_Alert ("quantityPlus1 함수에서 오류 발생: %s", ex.what ());
	}

	// 없으면 신규 레코드 추가하고 1 리턴
	record.push_back ("1");
	this->records.push_back (record);

	return 1;
}

// 레코드 내용 지우기
void SummaryOfObjectInfo::clear ()
{
	unsigned int xx;
	
	try {
		for (xx = 0 ; xx < this->records.size () ; ++xx) {
			this->records.at(xx).clear ();
		}
	} catch (exception& ex) {
		WriteReport_Alert ("clear 함수에서 오류 발생: %s", ex.what ());
	}
	this->records.clear ();
}

// 선택한 부재 정보 내보내기 (Single 모드)
GSErrCode	exportSelectedElementInfo (void)
{
	GSErrCode	err = NoError;
	unsigned short		xx, yy, zz;
	//bool		regenerate = true;
	
	// 선택한 요소가 없으면 오류
	GS::Array<API_Guid>		objects;
	//GS::Array<API_Guid>		beams;
	long					nObjects = 0;
	//long					nBeams = 0;

	// 선택한 요소들의 정보 요약하기
	API_Element			elem;
	API_ElementMemo		memo;
	SummaryOfObjectInfo	objectInfo;

	char			buffer [512];
	char			filename [512];
	char			tempStr [512];
	const char*		foundStr;
	bool			foundObject;
	bool			foundExistValue;
	int				retVal;
	int				nInfo;
	API_AddParID	varType;
	vector<string>	record;

	// GS::Array 반복자
	GS::Array<API_Guid>::Iterator	iterObj;
	API_Guid	curGuid;

	// Single 모드 전용
	vector<vector<string>>	plywoodInfo;	// 합판 종류별 정보
	vector<vector<string>>	darukiInfo;		// 각재 종류별 정보 (합판에 부착된 제작틀만)
	char*	token;
	char	infoText [256];


	// 그룹화 일시정지 ON
	suspendGroups (true);

	// 선택한 요소 가져오기
	if (getGuidsOfSelection (&objects, API_ObjectID, &nObjects) != NoError) {
		DGAlert (DG_ERROR, L"오류", L"요소들을 선택해야 합니다.", "", L"확인", "", "");
		return err;
	}

	// 엑셀 파일로 기둥 정보 내보내기
	// 파일 저장을 위한 변수
	API_SpecFolderID	specFolderID = API_ApplicationFolderID;
	IO::Location		location;
	GS::UniString		resultString;
	API_MiscAppInfo		miscAppInfo;
	FILE				*fp;
	FILE				*fp_interReport;

	ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);
	ACAPI_Environment (APIEnv_GetMiscAppInfoID, &miscAppInfo);
	sprintf (filename, "%s - 선택한 부재 정보.csv", miscAppInfo.caption);
	fp = fopen (filename, "w+");
	sprintf (filename, "%s - 선택한 부재 정보 (중간보고서).txt", miscAppInfo.caption);
	fp_interReport = fopen (filename, "w+");

	if ((fp == NULL) || (fp_interReport == NULL)) {
		DGAlert (DG_ERROR, L"오류", L"파일을 열 수 없습니다.", "", L"확인", "", "");
		return err;
	}

	iterObj = objects.Enumerate ();

	while (iterObj != NULL) {
		foundObject = false;

		BNZeroMemory (&elem, sizeof (API_Element));
		BNZeroMemory (&memo, sizeof (API_ElementMemo));
		curGuid = *iterObj;
		elem.header.guid = curGuid;
		err = ACAPI_Element_Get (&elem);

		if (err == NoError && elem.header.hasMemo) {
			err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

			if (err == NoError) {
				// 파라미터 스크립트를 강제로 실행시킴
				ACAPI_Goodies (APIAny_RunGDLParScriptID, &elem.header, 0);
				bool	bForce = true;
				ACAPI_Database (APIDb_RefreshElementID, &elem.header, &bForce);

				try {
					for (yy = 0 ; yy < objectInfo.keyName.size () ; ++yy) {
						strcpy (tempStr, objectInfo.keyName.at(yy).c_str ());
						foundStr = getParameterStringByName (&memo, tempStr);

						// 객체 종류를 찾았다면,
						if (my_strcmp (foundStr, "") != 0) {
							retVal = my_strcmp (objectInfo.keyDesc.at(yy).c_str (), foundStr);

							if (retVal == 0) {
								foundObject = true;
								foundExistValue = false;

								// 발견한 객체의 데이터를 기반으로 레코드 추가
								if (!record.empty ())
									record.clear ();

								record.push_back (objectInfo.keyDesc.at(yy));		// 객체 이름
								nInfo = objectInfo.nInfo.at(yy);
								for (zz = 0 ; zz < nInfo ; ++zz) {
									sprintf (buffer, "%s", objectInfo.varName.at(yy).at(zz).c_str ());
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
				} catch (exception& ex) {
					WriteReport_Alert ("객체 정보 수집에서 오류 발생: %s", ex.what ());
				}

				// 끝내 찾지 못하면 알 수 없는 객체로 취급함
				if (foundObject == false)
					objectInfo.nUnknownObjects ++;
			}

			ACAPI_DisposeElemMemoHdls (&memo);
		}

		++iterObj;
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

	// 최종 텍스트 표시
	// APIParT_Length인 경우 1000배 곱해서 표현
	// APIParT_Boolean인 경우 예/아니오 표현
	double	length, length2, length3;
	bool	bTitleAppeared;

	// 레코드 정렬 (내림차순 정렬)
	sort (objectInfo.records.begin (), objectInfo.records.end (), compareVectorString);

	// *합판, 각재 구매 수량을 계산하기 위한 변수
	double	totalAreaOfPlywoods = 0.0;
	double	totalLengthOfTimbers_40x50 = 0.0;	// 다루끼 (40*50)
	double	totalLengthOfTimbers_50x80 = 0.0;	// 투바이 (50*80)
	double	totalLengthOfTimbers_80x80 = 0.0;	// 산승각 (80*80)
	double	totalLengthOfTimbersEtc = 0.0;		// 비규격
	int		count;	// 개수

	// *합판, 다루끼 제작 수량을 계산하기 위한 변수
	plywoodInfo.clear ();
	darukiInfo.clear ();

	// 객체 종류별로 수량 출력
	try {
		for (xx = 0 ; xx < objectInfo.keyDesc.size () ; ++xx) {
			bTitleAppeared = false;

			// 레코드를 전부 순회
			for (yy = 0 ; yy < objectInfo.records.size () ; ++yy) {
				// 객체 종류 이름과 레코드의 1번 필드가 일치하는 경우만 찾아서 출력함
				retVal = my_strcmp (objectInfo.keyDesc.at(xx).c_str (), objectInfo.records.at(yy).at(0).c_str ());

				count = atoi (objectInfo.records.at(yy).at(objectInfo.records.at(yy).size ()-1).c_str ());

				if (retVal == 0) {
					// 제목 출력
					if (bTitleAppeared == false) {
						sprintf (buffer, "\n[%s]\n", objectInfo.keyDesc.at(xx).c_str ());
						fprintf (fp, buffer);
						bTitleAppeared = true;
					}

					// 변수별 값 출력
					if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "유로폼 후크") == 0) {
						// 원형
						if (objectInfo.records.at(yy).at(2).compare ("원형") == 0) {
							sprintf (buffer, "원형 / %s", objectInfo.records.at(yy).at(1));
						}

						// 사각
						if (objectInfo.records.at(yy).at(2).compare ("사각") == 0) {
							sprintf (buffer, "사각 / %s", objectInfo.records.at(yy).at(1));
						}
						fprintf (fp, buffer);

					} else if ((my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "유로폼") == 0) || (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "스틸폼") == 0)) {
						// 규격폼
						if (atoi (objectInfo.records.at(yy).at(1).c_str ()) > 0) {
							sprintf (buffer, "%s X %s ", objectInfo.records.at(yy).at(2), objectInfo.records.at(yy).at(3));

						// 비규격품
						} else {
							length = atof (objectInfo.records.at(yy).at(4).c_str ());
							length2 = atof (objectInfo.records.at(yy).at(5).c_str ());
							sprintf (buffer, "%.0f X %.0f ", round (length*1000, 0), round (length2*1000, 0));
						}
						fprintf (fp, buffer);

					} else if (objectInfo.keyDesc.at(xx).compare ("목재") == 0) {
						length = atof (objectInfo.records.at(yy).at(1).c_str ());
						length2 = atof (objectInfo.records.at(yy).at(2).c_str ());
						length3 = atof (objectInfo.records.at(yy).at(3).c_str ());
						sprintf (buffer, "%.0f X %.0f X %.0f ", round (length*1000, 0), round (length2*1000, 0), round (length3*1000, 0));
						if ( ((abs (length - 0.040) < EPS) && (abs (length2 - 0.050) < EPS)) || ((abs (length - 0.050) < EPS) && (abs (length2 - 0.040) < EPS)) )
							totalLengthOfTimbers_40x50 += (length3 * count);
						else if ( ((abs (length - 0.050) < EPS) && (abs (length2 - 0.080) < EPS)) || ((abs (length - 0.080) < EPS) && (abs (length2 - 0.050) < EPS)) )
							totalLengthOfTimbers_50x80 += (length3 * count);
						else if ((abs (length - 0.080) < EPS) && (abs (length2 - 0.080) < EPS))
							totalLengthOfTimbers_80x80 += (length3 * count);
						else
							totalLengthOfTimbersEtc += (length3 * count);
						fprintf (fp, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "콘판넬") == 0) {
						if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "3x6 [910x1820]") == 0) {
							sprintf (buffer, "910 X 1820 X %s ", objectInfo.records.at(yy).at(2).c_str ());
							fprintf (fp, buffer);

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "4x8 [1220x2440]") == 0) {
							sprintf (buffer, "1220 X 2440 X %s ", objectInfo.records.at(yy).at(2).c_str ());
							fprintf (fp, buffer);

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "2x5 [606x1520]") == 0) {
							sprintf (buffer, "606 X 1520 X %s ", objectInfo.records.at(yy).at(2).c_str ());
							fprintf (fp, buffer);

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "2x6 [606x1820]") == 0) {
							sprintf (buffer, "606 X 1820 X %s ", objectInfo.records.at(yy).at(2).c_str ());
							fprintf (fp, buffer);

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "3x5 [910x1520]") == 0) {
							sprintf (buffer, "910 X 1520 X %s ", objectInfo.records.at(yy).at(2).c_str ());
							fprintf (fp, buffer);

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "비규격") == 0) {
							// 가로 X 세로 X 두께
							length = atof (objectInfo.records.at(yy).at(3).c_str ());
							length2 = atof (objectInfo.records.at(yy).at(4).c_str ());
							sprintf (buffer, "%.0f X %.0f X %s ", round (length*1000, 0), round (length2*1000, 0), objectInfo.records.at(yy).at(2).c_str ());
							fprintf (fp, buffer);
						}

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "합판") == 0) {
						if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "3x6 [910x1820]") == 0) {
							sprintf (buffer, "910 X 1820 X %s ", objectInfo.records.at(yy).at(2).c_str ());
							totalAreaOfPlywoods += (0.900 * 1.800 * count);
							fprintf (fp, buffer);
							
							// 합판 정보 DB에 삽입
							record.clear ();
							record.push_back (buffer);
							quantityPlusN (&plywoodInfo, record, count);

							// 제작틀 ON
							if (atoi (objectInfo.records.at(yy).at(5).c_str ()) > 0) {
								sprintf (buffer, "(각재 총길이: %s) ", objectInfo.records.at(yy).at(6).c_str ());
								totalLengthOfTimbers_40x50 += ((atof (objectInfo.records.at(yy).at(6).c_str ()) / 1000) * count);
								fprintf (fp, buffer);

								sprintf (buffer, "(각재 절단 길이: %s) ", objectInfo.records.at(yy).at(7).c_str ());
								fprintf (fp, buffer);
								
								// 다루끼 DB에 삽입
								strcpy (infoText, objectInfo.records.at(yy).at(7).c_str ());
								token = strtok (infoText, " /");
								while (token != NULL) {
									// 숫자이면 push
									if (atoi (token) != 0) {
										record.clear ();
										record.push_back (token);
										quantityPlusN (&darukiInfo, record, 1);
									}
									token = strtok (NULL, " /");
								}
							}

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "4x8 [1220x2440]") == 0) {
							sprintf (buffer, "1220 X 2440 X %s ", objectInfo.records.at(yy).at(2).c_str ());
							totalAreaOfPlywoods += (1.200 * 2.400 * count);
							fprintf (fp, buffer);

							// 합판 정보 DB에 삽입
							record.clear ();
							record.push_back (buffer);
							quantityPlusN (&plywoodInfo, record, count);

							// 제작틀 ON
							if (atoi (objectInfo.records.at(yy).at(5).c_str ()) > 0) {
								sprintf (buffer, "(각재 총길이: %s) ", objectInfo.records.at(yy).at(6).c_str ());
								totalLengthOfTimbers_40x50 += ((atof (objectInfo.records.at(yy).at(6).c_str ()) / 1000) * count);
								fprintf (fp, buffer);

								sprintf (buffer, "(각재 절단 길이: %s) ", objectInfo.records.at(yy).at(7).c_str ());
								fprintf (fp, buffer);

								// 다루끼 DB에 삽입
								strcpy (infoText, objectInfo.records.at(yy).at(7).c_str ());
								token = strtok (infoText, " /");
								while (token != NULL) {
									// 숫자이면 push
									if (atoi (token) != 0) {
										record.clear ();
										record.push_back (token);
										quantityPlusN (&darukiInfo, record, 1);
									}
									token = strtok (NULL, " /");
								}
							}

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "2x5 [606x1520]") == 0) {
							sprintf (buffer, "606 X 1520 X %s ", objectInfo.records.at(yy).at(2).c_str ());
							totalAreaOfPlywoods += (0.600 * 1.500 * count);
							fprintf (fp, buffer);

							// 합판 정보 DB에 삽입
							record.clear ();
							record.push_back (buffer);
							quantityPlusN (&plywoodInfo, record, count);

							// 제작틀 ON
							if (atoi (objectInfo.records.at(yy).at(5).c_str ()) > 0) {
								sprintf (buffer, "(각재 총길이: %s) ", objectInfo.records.at(yy).at(6).c_str ());
								totalLengthOfTimbers_40x50 += ((atof (objectInfo.records.at(yy).at(6).c_str ()) / 1000) * count);
								fprintf (fp, buffer);

								sprintf (buffer, "(각재 절단 길이: %s) ", objectInfo.records.at(yy).at(7).c_str ());
								fprintf (fp, buffer);

								// 다루끼 DB에 삽입
								strcpy (infoText, objectInfo.records.at(yy).at(7).c_str ());
								token = strtok (infoText, " /");
								while (token != NULL) {
									// 숫자이면 push
									if (atoi (token) != 0) {
										record.clear ();
										record.push_back (token);
										quantityPlusN (&darukiInfo, record, 1);
									}
									token = strtok (NULL, " /");
								}
							}

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "2x6 [606x1820]") == 0) {
							sprintf (buffer, "606 X 1820 X %s ", objectInfo.records.at(yy).at(2).c_str ());
							totalAreaOfPlywoods += (0.600 * 1.800 * count);
							fprintf (fp, buffer);

							// 합판 정보 DB에 삽입
							record.clear ();
							record.push_back (buffer);
							quantityPlusN (&plywoodInfo, record, count);

							// 제작틀 ON
							if (atoi (objectInfo.records.at(yy).at(5).c_str ()) > 0) {
								sprintf (buffer, "(각재 총길이: %s) ", objectInfo.records.at(yy).at(6).c_str ());
								totalLengthOfTimbers_40x50 += ((atof (objectInfo.records.at(yy).at(6).c_str ()) / 1000) * count);
								fprintf (fp, buffer);

								sprintf (buffer, "(각재 절단 길이: %s) ", objectInfo.records.at(yy).at(7).c_str ());
								fprintf (fp, buffer);

								// 다루끼 DB에 삽입
								strcpy (infoText, objectInfo.records.at(yy).at(7).c_str ());
								token = strtok (infoText, " /");
								while (token != NULL) {
									// 숫자이면 push
									if (atoi (token) != 0) {
										record.clear ();
										record.push_back (token);
										quantityPlusN (&darukiInfo, record, 1);
									}
									token = strtok (NULL, " /");
								}
							}

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "3x5 [910x1520]") == 0) {
							sprintf (buffer, "910 X 1520 X %s ", objectInfo.records.at(yy).at(2).c_str ());
							totalAreaOfPlywoods += (0.900 * 1.500 * count);
							fprintf (fp, buffer);

							// 합판 정보 DB에 삽입
							record.clear ();
							record.push_back (buffer);
							quantityPlusN (&plywoodInfo, record, count);

							// 제작틀 ON
							if (atoi (objectInfo.records.at(yy).at(5).c_str ()) > 0) {
								sprintf (buffer, "(각재 총길이: %s) ", objectInfo.records.at(yy).at(6).c_str ());
								totalLengthOfTimbers_40x50 += ((atof (objectInfo.records.at(yy).at(6).c_str ()) / 1000) * count);
								fprintf (fp, buffer);

								sprintf (buffer, "(각재 절단 길이: %s) ", objectInfo.records.at(yy).at(7).c_str ());
								fprintf (fp, buffer);

								// 다루끼 DB에 삽입
								strcpy (infoText, objectInfo.records.at(yy).at(7).c_str ());
								token = strtok (infoText, " /");
								while (token != NULL) {
									// 숫자이면 push
									if (atoi (token) != 0) {
										record.clear ();
										record.push_back (token);
										quantityPlusN (&darukiInfo, record, 1);
									}
									token = strtok (NULL, " /");
								}
							}

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "비규격") == 0) {
							// 가로 X 세로 X 두께
							length = atof (objectInfo.records.at(yy).at(3).c_str ());
							length2 = atof (objectInfo.records.at(yy).at(4).c_str ());
							sprintf (buffer, "%.0f X %.0f X %s ", round (length*1000, 0), round (length2*1000, 0), objectInfo.records.at(yy).at(2).c_str ());
							totalAreaOfPlywoods += (length * length2 * count);
							fprintf (fp, buffer);

							// 합판 정보 DB에 삽입
							record.clear ();
							record.push_back (buffer);
							quantityPlusN (&plywoodInfo, record, count);

							// 제작틀 ON
							if (atoi (objectInfo.records.at(yy).at(5).c_str ()) > 0) {
								sprintf (buffer, "(각재 총길이: %s) ", objectInfo.records.at(yy).at(6).c_str ());
								totalLengthOfTimbers_40x50 += ((atof (objectInfo.records.at(yy).at(6).c_str ()) / 1000) * count);
								fprintf (fp, buffer);

								sprintf (buffer, "(각재 절단 길이: %s) ", objectInfo.records.at(yy).at(7).c_str ());
								fprintf (fp, buffer);

								// 다루끼 DB에 삽입
								strcpy (infoText, objectInfo.records.at(yy).at(7).c_str ());
								token = strtok (infoText, " /");
								while (token != NULL) {
									// 숫자이면 push
									if (atoi (token) != 0) {
										record.clear ();
										record.push_back (token);
										quantityPlusN (&darukiInfo, record, 1);
									}
									token = strtok (NULL, " /");
								}
							}

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "비정형") == 0) {
							sprintf (buffer, "비정형 ");
							fprintf (fp, buffer);

						} else {
							sprintf (buffer, "다각형 ");
							fprintf (fp, buffer);
						}

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "합판(다각형)") == 0) {
						// 합판 면적
						sprintf (buffer, "면적: %.2f ", atof (objectInfo.records.at(yy).at(1).c_str ()));
						totalAreaOfPlywoods += (atof (objectInfo.records.at(yy).at(1).c_str ()) * count);
						fprintf (fp, buffer);

						// 제작틀 ON
						if (atoi (objectInfo.records.at(yy).at(2).c_str ()) > 0) {
							sprintf (buffer, "(각재 총길이: %.0f) ", round (atof (objectInfo.records.at(yy).at(3).c_str ())*1000, 0));
							totalLengthOfTimbers_40x50 += (atof (objectInfo.records.at(yy).at(3).c_str ()) * count);
							fprintf (fp, buffer);
						}

					} else if (objectInfo.keyDesc.at(xx).compare ("RS Push-Pull Props") == 0) {
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
				
					} else if (objectInfo.keyDesc.at(xx).compare ("Push-Pull Props (기성품 및 당사제작품)") == 0) {
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

					} else if (objectInfo.keyDesc.at(xx).compare ("사각파이프") == 0) {
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

					} else if (objectInfo.keyDesc.at(xx).compare ("원형파이프") == 0) {
						length = atof (objectInfo.records.at(yy).at(1).c_str ());
						sprintf (buffer, "%.0f ", round (length*1000, 0));
						fprintf (fp, buffer);

					} else if (objectInfo.keyDesc.at(xx).compare ("아웃코너앵글") == 0) {
						length = atof (objectInfo.records.at(yy).at(1).c_str ());
						sprintf (buffer, "%.0f ", round (length*1000, 0));
						fprintf (fp, buffer);

					} else if (objectInfo.keyDesc.at(xx).compare ("매직바") == 0) {
						if (atoi (objectInfo.records.at(yy).at(2).c_str ()) > 0) {
							length = atof (objectInfo.records.at(yy).at(3).c_str ());
							length2 = atof (objectInfo.records.at(yy).at(4).c_str ());
							length3 = atof (objectInfo.records.at(yy).at(5).c_str ());
							totalAreaOfPlywoods += (atof (objectInfo.records.at(yy).at(6).c_str ()) * count);
							sprintf (buffer, "%.0f / 합판(%.0f X %.0f) ", round (atof (objectInfo.records.at(yy).at(1).c_str ())*1000, 0), round ((length - length2)*1000, 0), round (length3*1000, 0));
						} else {
							length = atof (objectInfo.records.at(yy).at(1).c_str ());
							sprintf (buffer, "%.0f ", round (length*1000, 0));
						}
						fprintf (fp, buffer);

					} else if (objectInfo.keyDesc.at(xx).compare ("매직아웃코너") == 0) {
						sprintf (buffer, "타입(%s) %.0f ", objectInfo.records.at(yy).at(1).c_str (), round (atof (objectInfo.records.at(yy).at(2).c_str ())*1000, 0));
						fprintf (fp, buffer);
						if (atoi (objectInfo.records.at(yy).at(3).c_str ()) > 0) {
							totalAreaOfPlywoods += (atof (objectInfo.records.at(yy).at(6).c_str ()) * count);
							sprintf (buffer, "합판1(%.0f X %.0f) ", round (atof (objectInfo.records.at(yy).at(2).c_str ())*1000, 0), round (atof (objectInfo.records.at(yy).at(4).c_str ())*1000, 0));
							fprintf (fp, buffer);
							sprintf (buffer, "합판2(%.0f X %.0f) ", round (atof (objectInfo.records.at(yy).at(2).c_str ())*1000, 0), round (atof (objectInfo.records.at(yy).at(5).c_str ())*1000, 0));
							fprintf (fp, buffer);
						}

					} else if (objectInfo.keyDesc.at(xx).compare ("매직인코너") == 0) {
						if (atoi (objectInfo.records.at(yy).at(3).c_str ()) > 0) {
							length = atof (objectInfo.records.at(yy).at(4).c_str ());
							length2 = atof (objectInfo.records.at(yy).at(5).c_str ());
							length3 = atof (objectInfo.records.at(yy).at(6).c_str ());
							totalAreaOfPlywoods += (atof (objectInfo.records.at(yy).at(7).c_str ()) * count);
							sprintf (buffer, "%.0f / 합판(%.0f X %.0f) ", round (atof (objectInfo.records.at(yy).at(2).c_str ())*1000, 0), round ((length - length2)*1000, 0), round (length3*1000, 0));
						} else {
							length = atof (objectInfo.records.at(yy).at(2).c_str ());
							sprintf (buffer, "%.0f ", round (length*1000, 0));
						}
						fprintf (fp, buffer);

					} else if (objectInfo.keyDesc.at(xx).compare ("눈썹보 브라켓 v2") == 0) {
						if (atoi (objectInfo.records.at(yy).at(1).c_str ()) > 0) {
							length = atof (objectInfo.records.at(yy).at(2).c_str ()) / 1000;
							totalLengthOfTimbers_40x50 += (length * count);
							sprintf (buffer, "각재(%.0f) ", round (length*1000, 0));
						}
						fprintf (fp, buffer);

					} else if (objectInfo.keyDesc.at(xx).compare ("단열재") == 0) {
						sprintf (buffer, "원장크기: %.0f X %.0f / 실제크기: %.0f X %.0f (ㄱ형상으로 자름: %s)",
							round (atof (objectInfo.records.at(yy).at(2).c_str ())*1000, 0), round (atof (objectInfo.records.at(yy).at(3).c_str ())*1000, 0),
							round (atof (objectInfo.records.at(yy).at(4).c_str ())*1000, 0), round (atof (objectInfo.records.at(yy).at(5).c_str ())*1000, 0),
							(atoi (objectInfo.records.at(yy).at(5).c_str ()) ? "자름" : "자르지 않음"));
						fprintf (fp, buffer);

					} else if (objectInfo.keyDesc.at(xx).compare ("PERI동바리 수직재") == 0) {
						length = atof (objectInfo.records.at(yy).at(2).c_str ());
						if (atoi (objectInfo.records.at(yy).at(3).c_str ()) == 1) {
							sprintf (buffer, "규격(%s) 길이(%.0f) 크로스헤드(%s) ", objectInfo.records.at(yy).at(1).c_str (), round (length*1000, 0), objectInfo.records.at(yy).at(4).c_str ());
						} else {
							sprintf (buffer, "규격(%s) 길이(%.0f) ", objectInfo.records.at(yy).at(1).c_str (), round (length*1000, 0));
						}
						fprintf (fp, buffer);

					} else if (objectInfo.keyDesc.at(xx).compare ("서포트") == 0) {
						length = atof (objectInfo.records.at(yy).at(2).c_str ());
						if (atoi (objectInfo.records.at(yy).at(3).c_str ()) == 1) {
							sprintf (buffer, "규격(%s) 길이(%.0f) 크로스헤드(%s) ", objectInfo.records.at(yy).at(1).c_str (), round (length*1000, 0), objectInfo.records.at(yy).at(4).c_str ());
						} else {
							sprintf (buffer, "규격(%s) 길이(%.0f) ", objectInfo.records.at(yy).at(1).c_str (), round (length*1000, 0));
						}
						fprintf (fp, buffer);

					} else if (objectInfo.keyDesc.at(xx).compare ("창문 개구부 합판거푸집") == 0) {
						// 너비 X 높이 X 벽 두께
						length = atof (objectInfo.records.at(yy).at(2).c_str ());
						length2 = atof (objectInfo.records.at(yy).at(3).c_str ());
						length3 = atof (objectInfo.records.at(yy).at(4).c_str ());
						sprintf (buffer, "%.0f X %.0f X %.0f ", round (length*1000, 0), round (length2*1000, 0), round (length3*1000, 0));
						fprintf (fp, buffer);

						// 합판 면적
						totalAreaOfPlywoods += ((atof (objectInfo.records.at(yy).at(5).c_str ())) * count);

						// 다루끼 길이
						totalLengthOfTimbers_40x50 += ((atof (objectInfo.records.at(yy).at(6).c_str ())) * count);
						
					} else {
						for (zz = 0 ; zz < objectInfo.nInfo.at(xx) ; ++zz) {
							// 변수별 값 출력
							sprintf (buffer, "%s(%s) ", objectInfo.varDesc.at(xx).at(zz).c_str (), objectInfo.records.at(yy).at(zz+1).c_str ());
							fprintf (fp, buffer);
						}
					}

					// 수량 출력
					sprintf (buffer, ": %s EA\n", objectInfo.records.at(yy).at(objectInfo.records.at(yy).size ()-1).c_str ());
					fprintf (fp, buffer);
				}
			}
		}
	} catch (exception& ex) {
		WriteReport_Alert ("출력 함수에서 오류 발생: %s", ex.what ());
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

	// *합판, 다루끼 제작 수량 계산
	sort (plywoodInfo.begin (), plywoodInfo.end (), compareVectorString);
	sort (darukiInfo.begin (), darukiInfo.end (), compareVectorString);

	if (plywoodInfo.size () > 0) {
		sprintf (buffer, "\n합판 규격별 제작 수량은 다음과 같습니다.");
		fprintf (fp, buffer);

		for (xx = 0 ; xx < plywoodInfo.size () ; ++xx) {
			sprintf (buffer, "\n%s : %s EA", plywoodInfo.at(xx).at(0).c_str (), plywoodInfo.at(xx).at(1).c_str ());
			fprintf (fp, buffer);
		}
		fprintf (fp, "\n");
	}

	if (darukiInfo.size () > 0) {
		sprintf (buffer, "\n합판 제작틀 다루끼 (40*50) 규격별 제작 수량은 다음과 같습니다.");
		fprintf (fp, buffer);
		
		for (xx = 0 ; xx < darukiInfo.size () ; ++xx) {
			sprintf (buffer, "\n%s : %s EA", darukiInfo.at(xx).at(0).c_str (), darukiInfo.at(xx).at(1).c_str ());
			fprintf (fp, buffer);
		}
		fprintf (fp, "\n");
	}

	// *합판, 각재 구매 수량 계산
	// 합판 4x8 규격 (1200*2400) 기준으로 총 면적을 나누면 합판 구매 수량이 나옴
	if (totalAreaOfPlywoods > EPS) {
		sprintf (buffer, "\n합판 구매 수량은 다음과 같습니다.\n총 면적 (%.2f ㎡) ÷ 합판 4x8 규격 (1200*2400) = %.0f 개 (할증 5퍼센트 적용됨)\n", totalAreaOfPlywoods, ceil ((totalAreaOfPlywoods / 2.88)*1.05));
		fprintf (fp, buffer);
	}
	// 각재 다루끼(40*50), 투바이(50*80), 산승각(80*80), 1본은 3600mm
	if ((totalLengthOfTimbers_40x50 > EPS) || (totalLengthOfTimbers_50x80 > EPS) || (totalLengthOfTimbers_80x80 > EPS) || (totalLengthOfTimbersEtc > EPS)) {
		sprintf (buffer, "\n각재 구매 수량은 다음과 같습니다.\n");
		fprintf (fp, buffer);
		if (totalLengthOfTimbers_40x50 > EPS) {
			sprintf (buffer, "다루끼 (40*50) : 총 길이 (%.3f) ÷ 1본 (3600) = %.0f 개 (할증 5퍼센트 적용됨)\n", totalLengthOfTimbers_40x50, ceil ((totalLengthOfTimbers_40x50 / 3.6)*1.05));
			fprintf (fp, buffer);
		}
		if (totalLengthOfTimbers_50x80 > EPS) {
			sprintf (buffer, "투바이 (50*80) : 총 길이 (%.3f) ÷ 1본 (3600) = %.0f 개 (할증 5퍼센트 적용됨)\n", totalLengthOfTimbers_50x80, ceil ((totalLengthOfTimbers_50x80 / 3.6)*1.05));
			fprintf (fp, buffer);
		}
		if (totalLengthOfTimbers_80x80 > EPS) {
			sprintf (buffer, "산승각 (80*80) : 총 길이 (%.3f) ÷ 1본 (3600) = %.0f 개 (할증 5퍼센트 적용됨)\n", totalLengthOfTimbers_80x80, ceil ((totalLengthOfTimbers_80x80 / 3.6)*1.05));
			fprintf (fp, buffer);
		}
		if (totalLengthOfTimbersEtc > EPS) {
			sprintf (buffer, "비규격 : 총 길이 (%.3f) ÷ 1본 (3600) = %.0f 개 (할증 5퍼센트 적용됨)\n", totalLengthOfTimbersEtc, ceil ((totalLengthOfTimbersEtc / 3.6)*1.05));
			fprintf (fp, buffer);
		}
	}
	if ((totalAreaOfPlywoods > EPS) || (totalLengthOfTimbers_40x50 > EPS) || (totalLengthOfTimbers_50x80 > EPS) || (totalLengthOfTimbers_80x80 > EPS) || (totalLengthOfTimbersEtc > EPS)) {
		sprintf (buffer, "\n주의사항: 합판/목재 구매 수량은 다음 객체에 대해서만 계산되었습니다. 추가될 객체가 있다면 개발자에게 문의하십시오.\n합판 / 합판(다각형) / 목재 / 매직바 / 매직아웃코너 / 매직인코너 / 눈썹보 브라켓 v2 / 창문 개구부 합판거푸집\n");
		fprintf (fp, buffer);
	}

	fclose (fp);

	// 객체 종류별로 수량 출력 (중간보고서)
	try {
		for (xx = 0 ; xx < objectInfo.keyDesc.size () ; ++xx) {
			// 레코드를 전부 순회
			for (yy = 0 ; yy < objectInfo.records.size () ; ++yy) {
				// 객체 종류 이름과 레코드의 1번 필드가 일치하는 경우만 찾아서 출력함
				retVal = my_strcmp (objectInfo.keyDesc.at(xx).c_str (), objectInfo.records.at(yy).at(0).c_str ());

				if (retVal == 0) {
					// 품목
					sprintf (buffer, "%s | ", objectInfo.keyDesc.at(xx).c_str ());
					fprintf (fp_interReport, buffer);

					// 변수별 값 출력
					if ((my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "유로폼") == 0) || (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "스틸폼") == 0)) {
						// 규격
						if (atoi (objectInfo.records.at(yy).at(1).c_str ()) > 0) {
							// 규격폼
							sprintf (buffer, "%s X %s | ", objectInfo.records.at(yy).at(2), objectInfo.records.at(yy).at(3));
						} else {
							// 비규격품
							length = atof (objectInfo.records.at(yy).at(4).c_str ());
							length2 = atof (objectInfo.records.at(yy).at(5).c_str ());
							sprintf (buffer, "%.0f X %.0f | ", round (length*1000, 0), round (length2*1000, 0));
						}
						fprintf (fp_interReport, buffer);

						// 길이
						sprintf (buffer, "- | ");
						fprintf (fp_interReport, buffer);
						
						// 단위
						sprintf (buffer, "개수(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if ((my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "인코너판넬") == 0) || (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "아웃코너판넬") == 0)) {
						length = atof (objectInfo.records.at(yy).at(1).c_str ());
						length2 = atof (objectInfo.records.at(yy).at(2).c_str ());
						length3 = atof (objectInfo.records.at(yy).at(3).c_str ());

						// 규격
						sprintf (buffer, "%.0f X %.0f | ", round (length*1000, 0), round (length2*1000, 0));
						fprintf (fp_interReport, buffer);

						// 길이
						sprintf (buffer, "%.0f | ", round (length3*1000, 0));
						fprintf (fp_interReport, buffer);

						// 단위
						sprintf (buffer, "개수(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "아웃코너앵글") == 0) {
						length = atof (objectInfo.records.at(yy).at(1).c_str ());

						// 규격
						sprintf (buffer, "- | ");
						fprintf (fp_interReport, buffer);

						// 길이
						sprintf (buffer, "%.0f | ", round (length*1000, 0));
						fprintf (fp_interReport, buffer);

						// 단위
						sprintf (buffer, "개수(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "목재") == 0) {
						length = atof (objectInfo.records.at(yy).at(1).c_str ());
						length2 = atof (objectInfo.records.at(yy).at(2).c_str ());
						length3 = atof (objectInfo.records.at(yy).at(3).c_str ());

						// 규격
						sprintf (buffer, "%.0f X %.0f | ", round (length*1000, 0), round (length2*1000, 0));
						fprintf (fp_interReport, buffer);

						// 길이
						sprintf (buffer, "%.0f | ", round (length3*1000, 0));
						fprintf (fp_interReport, buffer);

						// 단위
						sprintf (buffer, "본 | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "휠러스페이서") == 0) {
						length = atof (objectInfo.records.at(yy).at(1).c_str ());
						length2 = atof (objectInfo.records.at(yy).at(2).c_str ());

						// 규격
						sprintf (buffer, "%.0f | ", round (length*1000, 0));
						fprintf (fp_interReport, buffer);

						// 길이
						sprintf (buffer, "%.0f | ", round (length2*1000, 0));
						fprintf (fp_interReport, buffer);

						// 단위
						sprintf (buffer, "개수(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "원형파이프") == 0) {
						length = atof (objectInfo.records.at(yy).at(1).c_str ());

						// 규격
						sprintf (buffer, "- | ");
						fprintf (fp_interReport, buffer);

						// 길이
						sprintf (buffer, "%.0f | ", round (length*1000, 0));
						fprintf (fp_interReport, buffer);

						// 단위
						sprintf (buffer, "개수(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "사각파이프") == 0) {
						length = atof (objectInfo.records.at(yy).at(2).c_str ());

						// 규격
						if (atof (objectInfo.records.at(yy).at(1).c_str ()) < EPS) {
							// 사각파이프
							sprintf (buffer, "50 x 50 | ");
						} else {
							// 직사각파이프
							sprintf (buffer, "%s | ", objectInfo.records.at(yy).at(1).c_str ());
						}
						fprintf (fp_interReport, buffer);

						// 길이
						sprintf (buffer, "%.0f | ", round (length*1000, 0));
						fprintf (fp_interReport, buffer);

						// 단위
						sprintf (buffer, "본 | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "콘판넬") == 0) {
						// 규격
						if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "3x6 [910x1820]") == 0) {
							sprintf (buffer, "910 X 1820 X %s | ", objectInfo.records.at(yy).at(2).c_str ());

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "4x8 [1220x2440]") == 0) {
							sprintf (buffer, "1220 X 2440 X %s | ", objectInfo.records.at(yy).at(2).c_str ());

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "2x5 [606x1520]") == 0) {
							sprintf (buffer, "606 X 1520 X %s | ", objectInfo.records.at(yy).at(2).c_str ());

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "2x6 [606x1820]") == 0) {
							sprintf (buffer, "606 X 1820 X %s | ", objectInfo.records.at(yy).at(2).c_str ());

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "3x5 [910x1520]") == 0) {
							sprintf (buffer, "910 X 1520 X %s | ", objectInfo.records.at(yy).at(2).c_str ());

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "비규격") == 0) {
							// 가로 X 세로 X 두께
							length = atof (objectInfo.records.at(yy).at(3).c_str ());
							length2 = atof (objectInfo.records.at(yy).at(4).c_str ());
							sprintf (buffer, "%.0f X %.0f X %s | ", round (length*1000, 0), round (length2*1000, 0), objectInfo.records.at(yy).at(2).c_str ());
						}
						fprintf (fp_interReport, buffer);

						// 길이
						sprintf (buffer, "- | ");
						fprintf (fp_interReport, buffer);

						// 단위
						sprintf (buffer, "장 | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "합판") == 0) {
						// 규격
						if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "3x6 [910x1820]") == 0) {
							sprintf (buffer, "910 X 1820 X %s | ", objectInfo.records.at(yy).at(2).c_str ());

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "4x8 [1220x2440]") == 0) {
							sprintf (buffer, "1220 X 2440 X %s | ", objectInfo.records.at(yy).at(2).c_str ());

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "2x5 [606x1520]") == 0) {
							sprintf (buffer, "606 X 1520 X %s | ", objectInfo.records.at(yy).at(2).c_str ());

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "2x6 [606x1820]") == 0) {
							sprintf (buffer, "606 X 1820 X %s | ", objectInfo.records.at(yy).at(2).c_str ());

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "3x5 [910x1520]") == 0) {
							sprintf (buffer, "910 X 1520 X %s | ", objectInfo.records.at(yy).at(2).c_str ());

						} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "비규격") == 0) {
							// 가로 X 세로 X 두께
							length = atof (objectInfo.records.at(yy).at(3).c_str ());
							length2 = atof (objectInfo.records.at(yy).at(4).c_str ());
							sprintf (buffer, "%.0f X %.0f X %s | ", round (length*1000, 0), round (length2*1000, 0), objectInfo.records.at(yy).at(2).c_str ());
						}
						fprintf (fp_interReport, buffer);

						// 길이
						sprintf (buffer, "- | ");
						fprintf (fp_interReport, buffer);

						// 단위
						sprintf (buffer, "장 | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "RS Push-Pull Props 헤드피스 (인양고리 포함)") == 0) {
						// 규격
						sprintf (buffer, "%s | ", objectInfo.records.at(yy).at(1).c_str ());
						fprintf (fp_interReport, buffer);

						// 길이
						sprintf (buffer, "- | ");
						fprintf (fp_interReport, buffer);

						// 단위
						sprintf (buffer, "개수(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "RS Push-Pull Props") == 0) {
						// 규격
						if (atoi (objectInfo.records.at(yy).at(4).c_str ()) == 1) {
							// 하부 지지대 있을 경우
							sprintf (buffer, "%s, %s | ", objectInfo.records.at(yy).at(2).c_str (), objectInfo.records.at(yy).at(3).c_str ());
						} else {
							// 하부 지지대 없을 경우
							sprintf (buffer, "%s, - | ", objectInfo.records.at(yy).at(2).c_str ());
						}
						fprintf (fp_interReport, buffer);

						// 길이
						sprintf (buffer, "- | ");
						fprintf (fp_interReport, buffer);

						// 단위
						sprintf (buffer, "개수(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "핀볼트세트") == 0) {
						// 규격
						length = atof (objectInfo.records.at(yy).at(2).c_str ());
						sprintf (buffer, "%.0f X %.0f | ", round (length*1000, 0), round (length*1000, 0));
						fprintf (fp_interReport, buffer);

						// 길이
						length = atof (objectInfo.records.at(yy).at(1).c_str ());
						sprintf (buffer, "%.0f | ", round (length*1000, 0));
						fprintf (fp_interReport, buffer);

						// 단위
						sprintf (buffer, "개수(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "결합철물 (사각와셔활용)") == 0) {
						// 규격
						length = atof (objectInfo.records.at(yy).at(2).c_str ());
						sprintf (buffer, "%.0f X %.0f | ", round (length*1000, 0), round (length*1000, 0));
						fprintf (fp_interReport, buffer);

						// 길이
						length = atof (objectInfo.records.at(yy).at(1).c_str ());
						sprintf (buffer, "%.0f | ", round (length*1000, 0));
						fprintf (fp_interReport, buffer);

						// 단위
						sprintf (buffer, "개수(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "보 멍에제") == 0) {
						// 규격
						sprintf (buffer, "- | ");
						fprintf (fp_interReport, buffer);

						// 길이
						sprintf (buffer, "%s | ", objectInfo.records.at(yy).at(1).c_str ());
						fprintf (fp_interReport, buffer);

						// 단위
						sprintf (buffer, "개수(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "PERI동바리 수직재") == 0) {
						// 규격
						sprintf (buffer, "%s | ", objectInfo.records.at(yy).at(1).c_str ());
						fprintf (fp_interReport, buffer);

						// 길이
						sprintf (buffer, "%s | ", objectInfo.records.at(yy).at(2).c_str ());
						fprintf (fp_interReport, buffer);

						// 단위
						sprintf (buffer, "개수(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "PERI동바리 수평재") == 0) {
						// 규격
						sprintf (buffer, "- | ");
						fprintf (fp_interReport, buffer);

						// 길이
						sprintf (buffer, "%s | ", objectInfo.records.at(yy).at(1).c_str ());
						fprintf (fp_interReport, buffer);

						// 단위
						sprintf (buffer, "개수(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "GT24 거더") == 0) {
						// 규격
						sprintf (buffer, "- | ");
						fprintf (fp_interReport, buffer);

						// 길이
						sprintf (buffer, "%s | ", objectInfo.records.at(yy).at(1).c_str ());
						fprintf (fp_interReport, buffer);

						// 단위
						sprintf (buffer, "개수(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "매직바") == 0) {
						// 규격 (매직바 전체 길이)
						sprintf (buffer, "%s | ", objectInfo.records.at(yy).at(1).c_str ());
						fprintf (fp_interReport, buffer);

						// 길이 (합판 너비 X 길이)
						length = atof (objectInfo.records.at(yy).at(3).c_str ());
						length2 = atof (objectInfo.records.at(yy).at(4).c_str ());
						if (atoi (objectInfo.records.at(yy).at(2).c_str ()) == 1) {
							sprintf (buffer, "%s X %.0f | ", objectInfo.records.at(yy).at(5).c_str (), abs (round (length*1000, 0) - round (length2*1000, 0)));
						} else {
							sprintf (buffer, "- | ");
						}
						fprintf (fp_interReport, buffer);

						// 단위
						sprintf (buffer, "개수(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "매직인코너") == 0) {
						// 규격 (매직바 너비 X 길이)
						sprintf (buffer, "%s X %s | ", objectInfo.records.at(yy).at(1).c_str (), objectInfo.records.at(yy).at(2).c_str ());
						fprintf (fp_interReport, buffer);

						// 길이 (합판 너비 X 길이)
						length = atof (objectInfo.records.at(yy).at(4).c_str ());
						length2 = atof (objectInfo.records.at(yy).at(5).c_str ());
						if (atoi (objectInfo.records.at(yy).at(3).c_str ()) == 1) {
							sprintf (buffer, "%s X %.0f | ", objectInfo.records.at(yy).at(6).c_str (), abs (round (length*1000, 0) - round (length2*1000, 0)));
						} else {
							sprintf (buffer, "- | ");
						}
						fprintf (fp_interReport, buffer);

						// 단위
						sprintf (buffer, "개수(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "유로폼 후크") == 0) {
						// 규격
						if (objectInfo.records.at(yy).at(2).compare ("원형") == 0) {
							sprintf (buffer, "%s, 원형 | ", objectInfo.records.at(yy).at(1).c_str ());
						} else if (objectInfo.records.at(yy).at(2).compare ("사각") == 0) {
							sprintf (buffer, "%s, 사각 | ", objectInfo.records.at(yy).at(1).c_str ());
						}
						fprintf (fp_interReport, buffer);

						// 길이
						sprintf (buffer, "- | ");
						fprintf (fp_interReport, buffer);
						
						// 단위
						sprintf (buffer, "개수(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "블루목심") == 0) {
						// 규격
						sprintf (buffer, "%s | ", objectInfo.records.at(yy).at(1).c_str ());
						fprintf (fp_interReport, buffer);

						// 길이
						sprintf (buffer, "- | ");
						fprintf (fp_interReport, buffer);
						
						// 단위
						sprintf (buffer, "개수(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "서포트") == 0) {
						// 규격
						sprintf (buffer, "%s | ", objectInfo.records.at(yy).at(1).c_str ());
						fprintf (fp_interReport, buffer);

						// 길이
						sprintf (buffer, "- | ");
						fprintf (fp_interReport, buffer);
						
						// 단위
						sprintf (buffer, "개수(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "슬래브 테이블폼 (콘판넬)") == 0) {
						// 규격
						sprintf (buffer, "%s | ", objectInfo.records.at(yy).at(1).c_str ());
						fprintf (fp_interReport, buffer);

						// 길이
						sprintf (buffer, "- | ");
						fprintf (fp_interReport, buffer);

						// 단위
						sprintf (buffer, "개수(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "블루클램프") == 0) {
						// 규격
						sprintf (buffer, "%s | ", objectInfo.records.at(yy).at(1).c_str ());
						fprintf (fp_interReport, buffer);

						// 길이
						sprintf (buffer, "- | ");
						fprintf (fp_interReport, buffer);
						
						// 단위
						sprintf (buffer, "개수(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "빔조인트용 Push-Pull Props 헤드피스") == 0) {
						// 규격
						sprintf (buffer, "%s | ", objectInfo.records.at(yy).at(1).c_str ());
						fprintf (fp_interReport, buffer);

						// 길이 
						sprintf (buffer, "- | ");
						fprintf (fp_interReport, buffer);

						// 단위
						sprintf (buffer, "개수(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "블루 보 브라켓") == 0) {
						// 규격
						sprintf (buffer, "%s | ", objectInfo.records.at(yy).at(1).c_str ());
						fprintf (fp_interReport, buffer);

						// 길이
						sprintf (buffer, "- | ");
						fprintf (fp_interReport, buffer);

						// 단위
						sprintf (buffer, "개수(EA) | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "단열재") == 0) {
						// 규격
						if (atoi (objectInfo.records.at(yy).at(1).c_str ()) == 1) {
							// 원장 가로 X 세로
							sprintf (buffer, "%s X %s | ", objectInfo.records.at(yy).at(2).c_str (), objectInfo.records.at(yy).at(3).c_str ());
						} else {
							// 실제 가로 X 세로
							sprintf (buffer, "%s X %s | ", objectInfo.records.at(yy).at(4).c_str (), objectInfo.records.at(yy).at(5).c_str ());
						}
						fprintf (fp_interReport, buffer);

						// 길이
						sprintf (buffer, "- | ");
						fprintf (fp_interReport, buffer);

						// 단위
						sprintf (buffer, "장 | ");
						fprintf (fp_interReport, buffer);

					} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "Push-Pull Props (기성품 및 당사제작품)") == 0) {
						// 규격
						if (atoi (objectInfo.records.at(yy).at(4).c_str ()) == 1) {
							// 하부 지지대 있을 경우
							sprintf (buffer, "%s, %s | ", objectInfo.records.at(yy).at(2).c_str (), objectInfo.records.at(yy).at(3).c_str ());
						} else {
							// 하부 지지대 없을 경우
							sprintf (buffer, "%s, - | ", objectInfo.records.at(yy).at(2).c_str ());
						}
						fprintf (fp_interReport, buffer);

						// 길이
						sprintf (buffer, "- | ");
						fprintf (fp_interReport, buffer);

						// 단위
						sprintf (buffer, "개수(EA) | ");
						fprintf (fp_interReport, buffer);

					} else {
						// 규격, 길이 없고 수량만 표현할 경우

						// 규격
						sprintf (buffer, "- | ");
						fprintf (fp_interReport, buffer);

						// 길이
						sprintf (buffer, "- | ");
						fprintf (fp_interReport, buffer);

						// 단위
						sprintf (buffer, "개수(EA) | ");
						fprintf (fp_interReport, buffer);
					}

					// 수량 출력
					sprintf (buffer, "%s\n", objectInfo.records.at(yy).at(objectInfo.records.at(yy).size ()-1).c_str ());
					fprintf (fp_interReport, buffer);
				}
			}
		}
	} catch (exception& ex) {
		WriteReport_Alert ("출력 함수에서 오류 발생: %s", ex.what ());
	}

	// 알 수 없는 객체
	if (objectInfo.nUnknownObjects > 0) {
		sprintf (buffer, "알 수 없는 객체 | - | - | - | %d\n", objectInfo.nUnknownObjects);
		fprintf (fp_interReport, buffer);
	}

	fclose (fp_interReport);

	// 그룹화 일시정지 OFF
	suspendGroups (false);

	ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);
	location.ToDisplayText (&resultString);
	WriteReport_Alert ("결과물을 다음 위치에 저장했습니다.\n\n%s\n또는 프로젝트 파일이 있는 폴더", resultString.ToCStr ().Get ());

	return	err;
}

// 선택한 부재 정보 내보내기 (Multi 모드)
GSErrCode	exportElementInfoOnVisibleLayers (void)
{
	GSErrCode	err = NoError;
	unsigned short		xx, yy, zz;
	short		mm;
	//bool		regenerate = true;

	// 모든 객체들의 원점 좌표를 전부 저장함
	vector<API_Coord3D>	vecPos;

	// 모든 객체, 보 저장
	GS::Array<API_Guid>		elemList;
	GS::Array<API_Guid>		objects;
	GS::Array<API_Guid>		beams;
	long					nObjects = 0;
	long					nBeams = 0;

	// 선택한 요소들의 정보 요약하기
	API_Element			elem;
	API_ElementMemo		memo;
	SummaryOfObjectInfo	objectInfo;

	char			tempStr [512];
	const char*		foundStr;
	bool			foundObject;
	bool			foundExistValue;
	int				retVal;
	int				nInfo;
	API_AddParID	varType;
	vector<string>	record;

	// GS::Array 반복자
	//GS::Array<API_Guid>::Iterator	iterObj;
	//API_Guid	curGuid;

	// 레이어 관련 변수
	short			nLayers;
	API_Attribute	attrib;
	short			nVisibleLayers = 0;
	short			visLayerList [1024];
	char			fullLayerName [512];
	vector<LayerList>	layerList;

	// 레이어 타입에 따라 캡쳐 방향 지정
	char*			foundLayerName;
	short			layerType = UNDEFINED;

	// 기타
	char			buffer [512];
	char			filename [512];

	// 작업 층 정보
	API_StoryInfo	storyInfo;
	double			workLevel_object;		// 객체의 작업 층 높이


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


	// 그룹화 일시정지 ON
	suspendGroups (true);

	ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);

	// [경고] 다이얼로그에서 객체 이미지를 캡쳐할지 여부를 물어봄
	//result = DGAlert (DG_INFORMATION, "캡쳐 여부 질문", "캡쳐 작업을 수행하시겠습니까?", "", "예", "아니오", "");
	//result = DG_CANCEL;

	// 프로젝트 내 레이어 개수를 알아냄
	nLayers = getLayerCount ();

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

	// 레이어 이름과 인덱스 저장
	for (xx = 0 ; xx < nVisibleLayers ; ++xx) {
		BNZeroMemory (&attrib, sizeof (API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = visLayerList [xx];
		err = ACAPI_Attribute_Get (&attrib);

		sprintf (fullLayerName, "%s", attrib.layer.head.name);
		fullLayerName [strlen (fullLayerName)] = '\0';

		LayerList newLayerItem;
		newLayerItem.layerInd = visLayerList [xx];
		newLayerItem.layerName = fullLayerName;

		layerList.push_back (newLayerItem);
	}

	// 레이어 이름 기준으로 정렬하여 레이어 인덱스 순서 변경
	sort (layerList.begin (), layerList.end (), compareLayerName);		// 레이어 이름 기준 오름차순 정렬

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
		DGAlert (DG_ERROR, L"오류", L"통합 버전 엑셀파일을 만들 수 없습니다.", "", L"확인", "", "");
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
		//attrib.layer.head.index = visLayerList [mm-1];
		attrib.layer.head.index = layerList [mm-1].layerInd;
		err = ACAPI_Attribute_Get (&attrib);

		// 초기화
		objects.Clear ();
		beams.Clear ();
		vecPos.clear ();
		objectInfo.clear ();
		objectInfo.nUnknownObjects = 0;

		if (err == NoError) {
			// 레이어 보이기
			if ((attrib.layer.head.flags & APILay_Hidden) == true) {
				attrib.layer.head.flags ^= APILay_Hidden;
				ACAPI_Attribute_Modify (&attrib, NULL);
			}

			// 모든 요소 가져오기
			ACAPI_Element_GetElemList (API_ObjectID, &elemList, APIFilt_OnVisLayer);	// 보이는 레이어에 있음, 객체 타입만
			while (elemList.GetSize () > 0) {
				objects.Push (elemList.Pop ());
			}

			ACAPI_Element_GetElemList (API_BeamID, &elemList, APIFilt_OnVisLayer);		// 보이는 레이어에 있음, 보 타입만
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

			sprintf (filename, "%s - 선택한 부재 정보.csv", fullLayerName);
			fp = fopen (filename, "w+");

			if (fp == NULL) {
				WriteReport_Alert ("레이어 %s는 파일명이 될 수 없으므로 생략합니다.", fullLayerName);
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

				if (err == NoError && elem.header.hasMemo) {
					err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

					if (err == NoError) {
						// 객체의 원점 수집하기 ==================================
						//API_Coord3D	coord;

						//coord.x = elem.object.pos.x;
						//coord.y = elem.object.pos.y;
						//coord.z = elem.object.level;
					
						//vecPos.push_back (coord);
						// 객체의 원점 수집하기 ==================================

						// 작업 층 높이 반영 -- 객체
						if (xx == 0) {
							BNZeroMemory (&storyInfo, sizeof (API_StoryInfo));
							workLevel_object = 0.0;
							ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo);
							for (yy = 0 ; yy <= (storyInfo.lastStory - storyInfo.firstStory) ; ++yy) {
								if (storyInfo.data [0][yy].index == elem.header.floorInd) {
									workLevel_object = storyInfo.data [0][yy].level;
									break;
								}
							}
							BMKillHandle ((GSHandle *) &storyInfo.data);
						}

						// 파라미터 스크립트를 강제로 실행시킴
						ACAPI_Goodies (APIAny_RunGDLParScriptID, &elem.header, 0);
						bool	bForce = true;
						ACAPI_Database (APIDb_RefreshElementID, &elem.header, &bForce);

						try {
							for (yy = 0 ; yy < objectInfo.keyName.size () ; ++yy) {
								strcpy (tempStr, objectInfo.keyName.at(yy).c_str ());
								foundStr = getParameterStringByName (&memo, tempStr);

								// 객체 종류를 찾았다면,
								if (my_strcmp (foundStr, "") != 0) {
									retVal = my_strcmp (objectInfo.keyDesc.at(yy).c_str (), foundStr);

									if (retVal == 0) {
										foundObject = true;
										foundExistValue = false;

										// 발견한 객체의 데이터를 기반으로 레코드 추가
										if (!record.empty ())
											record.clear ();

										record.push_back (objectInfo.keyDesc.at(yy));		// 객체 이름
										nInfo = objectInfo.nInfo.at(yy);
										for (zz = 0 ; zz < nInfo ; ++zz) {
											sprintf (buffer, "%s", objectInfo.varName.at(yy).at(zz).c_str ());
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
						} catch (exception& ex) {
							WriteReport_Alert ("객체 정보 수집에서 오류 발생: %s", ex.what ());
						}

						// 끝내 찾지 못하면 알 수 없는 객체로 취급함
						if (foundObject == false)
							objectInfo.nUnknownObjects ++;
					}

					ACAPI_DisposeElemMemoHdls (&memo);
				}
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

			// 레코드 정렬 (내림차순 정렬)
			sort (objectInfo.records.begin (), objectInfo.records.end (), compareVectorString);

			// 객체 종류별로 수량 출력
			try {
				for (xx = 0 ; xx < objectInfo.keyDesc.size () ; ++xx) {
					bTitleAppeared = false;

					// 레코드를 전부 순회
					for (yy = 0 ; yy < objectInfo.records.size () ; ++yy) {
						// 객체 종류 이름과 레코드의 1번 필드가 일치하는 경우만 찾아서 출력함
						retVal = my_strcmp (objectInfo.keyDesc.at(xx).c_str (), objectInfo.records.at(yy).at(0).c_str ());

						if (retVal == 0) {
							// 제목 출력
							if (bTitleAppeared == false) {
								sprintf (buffer, "\n[%s]\n", objectInfo.keyDesc.at(xx).c_str ());
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
								bTitleAppeared = true;
							}

							// 변수별 값 출력
							if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "유로폼 후크") == 0) {
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

							} else if ((my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "유로폼") == 0) || (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "스틸폼") == 0)) {
								// 규격폼
								if (atoi (objectInfo.records.at(yy).at(1).c_str ()) > 0) {
									sprintf (buffer, "%s X %s ", objectInfo.records.at(yy).at(2), objectInfo.records.at(yy).at(3));

								// 비규격품
								} else {
									length = atof (objectInfo.records.at(yy).at(4).c_str ());
									length2 = atof (objectInfo.records.at(yy).at(5).c_str ());
									sprintf (buffer, "%.0f X %.0f ", round (length*1000, 0), round (length2*1000, 0));
								}
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);

							} else if (objectInfo.keyDesc.at(xx).compare ("목재") == 0) {
								length = atof (objectInfo.records.at(yy).at(1).c_str ());
								length2 = atof (objectInfo.records.at(yy).at(2).c_str ());
								length3 = atof (objectInfo.records.at(yy).at(3).c_str ());
								sprintf (buffer, "%.0f X %.0f X %.0f ", round (length*1000, 0), round (length2*1000, 0), round (length3*1000, 0));
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);

							} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "콘판넬") == 0) {
								if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "3x6 [910x1820]") == 0) {
									sprintf (buffer, "910 X 1820 X %s ", objectInfo.records.at(yy).at(2).c_str ());
									fprintf (fp, buffer);
									fprintf (fp_unite, buffer);

								} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "4x8 [1220x2440]") == 0) {
									sprintf (buffer, "1220 X 2440 X %s ", objectInfo.records.at(yy).at(2).c_str ());
									fprintf (fp, buffer);
									fprintf (fp_unite, buffer);

								} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "2x5 [606x1520]") == 0) {
									sprintf (buffer, "606 X 1520 X %s ", objectInfo.records.at(yy).at(2).c_str ());
									fprintf (fp, buffer);
									fprintf (fp_unite, buffer);

								} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "2x6 [606x1820]") == 0) {
									sprintf (buffer, "606 X 1820 X %s ", objectInfo.records.at(yy).at(2).c_str ());
									fprintf (fp, buffer);
									fprintf (fp_unite, buffer);

								} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "3x5 [910x1520]") == 0) {
									sprintf (buffer, "910 X 1520 X %s ", objectInfo.records.at(yy).at(2).c_str ());
									fprintf (fp, buffer);
									fprintf (fp_unite, buffer);

								} else if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "비규격") == 0) {
									// 가로 X 세로 X 두께
									length = atof (objectInfo.records.at(yy).at(3).c_str ());
									length2 = atof (objectInfo.records.at(yy).at(4).c_str ());
									sprintf (buffer, "%.0f X %.0f X %s ", round (length*1000, 0), round (length2*1000, 0), objectInfo.records.at(yy).at(2).c_str ());
									fprintf (fp, buffer);
									fprintf (fp_unite, buffer);
								}

							} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "합판") == 0) {
								if (my_strcmp (objectInfo.records.at(yy).at(1).c_str (), "3x6 [910x1820]") == 0) {
									sprintf (buffer, "910 X 1820 X %s ", objectInfo.records.at(yy).at(2).c_str ());
									fprintf (fp, buffer);
									fprintf (fp_unite, buffer);

									// 제작틀 ON
									if (atoi (objectInfo.records.at(yy).at(5).c_str ()) > 0) {
										sprintf (buffer, "(각재 총길이: %s) \n", objectInfo.records.at(yy).at(6).c_str ());
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
										sprintf (buffer, "(각재 총길이: %s) \n", objectInfo.records.at(yy).at(6).c_str ());
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
										sprintf (buffer, "(각재 총길이: %s) \n", objectInfo.records.at(yy).at(6).c_str ());
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
										sprintf (buffer, "(각재 총길이: %s) \n", objectInfo.records.at(yy).at(6).c_str ());
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
										sprintf (buffer, "(각재 총길이: %s) \n", objectInfo.records.at(yy).at(6).c_str ());
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
										sprintf (buffer, "(각재 총길이: %s) \n", objectInfo.records.at(yy).at(6).c_str ());
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

							} else if (my_strcmp (objectInfo.keyDesc.at(xx).c_str (), "합판(다각형)") == 0) {
								// 합판 면적
								sprintf (buffer, "면적: %.2f ", atof (objectInfo.records.at(yy).at(1).c_str ()));
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);

								// 제작틀 ON
								if (atoi (objectInfo.records.at(yy).at(2).c_str ()) > 0) {
									sprintf (buffer, "(각재 총길이: %.0f) ", round (atof (objectInfo.records.at(yy).at(3).c_str ())*1000, 0));
									fprintf (fp, buffer);
									fprintf (fp_unite, buffer);
								}

							} else if (objectInfo.keyDesc.at(xx).compare ("RS Push-Pull Props") == 0) {
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

								// 규격(하부) - 선택사항
								if (atoi (objectInfo.records.at(yy).at(4).c_str ()) == 1) {
									sprintf (buffer, "규격(하부): %s ", objectInfo.records.at(yy).at(3).c_str ());
								}
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
				
							} else if (objectInfo.keyDesc.at(xx).compare ("Push-Pull Props (기성품 및 당사제작품)") == 0) {
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

							} else if (objectInfo.keyDesc.at(xx).compare ("사각파이프") == 0) {
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

							} else if (objectInfo.keyDesc.at(xx).compare ("원형파이프") == 0) {
								length = atof (objectInfo.records.at(yy).at(1).c_str ());
								sprintf (buffer, "%.0f ", round (length*1000, 0));
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);

							} else if (objectInfo.keyDesc.at(xx).compare ("아웃코너앵글") == 0) {
								length = atof (objectInfo.records.at(yy).at(1).c_str ());
								sprintf (buffer, "%.0f ", round (length*1000, 0));
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);

							} else if (objectInfo.keyDesc.at(xx).compare ("매직바") == 0) {
								if (atoi (objectInfo.records.at(yy).at(2).c_str ()) > 0) {
									length = atof (objectInfo.records.at(yy).at(3).c_str ());
									length2 = atof (objectInfo.records.at(yy).at(4).c_str ());
									length3 = atof (objectInfo.records.at(yy).at(5).c_str ());
									sprintf (buffer, "%.0f / 합판(%.0f X %.0f) ", round (atof (objectInfo.records.at(yy).at(1).c_str ())*1000, 0), round ((length - length2)*1000, 0), round (length3*1000, 0));
								} else {
									length = atof (objectInfo.records.at(yy).at(1).c_str ());
									sprintf (buffer, "%.0f ", round (length*1000, 0));
								}
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);

							} else if (objectInfo.keyDesc.at(xx).compare ("매직아웃코너") == 0) {
								sprintf (buffer, "타입(%s) %.0f ", objectInfo.records.at(yy).at(1).c_str (), round (atof (objectInfo.records.at(yy).at(2).c_str ())*1000, 0));
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
								if (atoi (objectInfo.records.at(yy).at(3).c_str ()) > 0) {
									sprintf (buffer, "합판1(%.0f X %.0f) ", round (atof (objectInfo.records.at(yy).at(2).c_str ())*1000, 0), round (atof (objectInfo.records.at(yy).at(4).c_str ())*1000, 0));
									fprintf (fp, buffer);
									fprintf (fp_unite, buffer);
									sprintf (buffer, "합판2(%.0f X %.0f) ", round (atof (objectInfo.records.at(yy).at(2).c_str ())*1000, 0), round (atof (objectInfo.records.at(yy).at(5).c_str ())*1000, 0));
									fprintf (fp, buffer);
									fprintf (fp_unite, buffer);
								}

							} else if (objectInfo.keyDesc.at(xx).compare ("매직인코너") == 0) {
								if (atoi (objectInfo.records.at(yy).at(3).c_str ()) > 0) {
									length = atof (objectInfo.records.at(yy).at(4).c_str ());
									length2 = atof (objectInfo.records.at(yy).at(5).c_str ());
									length3 = atof (objectInfo.records.at(yy).at(6).c_str ());
									sprintf (buffer, "%.0f / 합판(%.0f X %.0f) ", round (atof (objectInfo.records.at(yy).at(2).c_str ())*1000, 0), round ((length - length2)*1000, 0), round (length3*1000, 0));
								} else {
									length = atof (objectInfo.records.at(yy).at(2).c_str ());
									sprintf (buffer, "%.0f ", round (length*1000, 0));
								}
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);

							} else if (objectInfo.keyDesc.at(xx).compare ("눈썹보 브라켓 v2") == 0) {
								if (atoi (objectInfo.records.at(yy).at(1).c_str ()) > 0) {
									length = atof (objectInfo.records.at(yy).at(2).c_str ());
									sprintf (buffer, "각재(%.0f) ", round (length*1000, 0));
								}
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);

							} else if (objectInfo.keyDesc.at(xx).compare ("단열재") == 0) {
								sprintf (buffer, "원장크기: %.0f X %.0f / 실제크기: %.0f X %.0f (ㄱ형상으로 자름: %s)",
									round (atof (objectInfo.records.at(yy).at(2).c_str ())*1000, 0), round (atof (objectInfo.records.at(yy).at(3).c_str ())*1000, 0),
									round (atof (objectInfo.records.at(yy).at(4).c_str ())*1000, 0), round (atof (objectInfo.records.at(yy).at(5).c_str ())*1000, 0),
									(atoi (objectInfo.records.at(yy).at(5).c_str ()) ? "자름" : "자르지 않음"));
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);
						
							} else if (objectInfo.keyDesc.at(xx).compare ("PERI동바리 수직재") == 0) {
								length = atof (objectInfo.records.at(yy).at(2).c_str ());
								if (atoi (objectInfo.records.at(yy).at(3).c_str ()) == 1) {
									sprintf (buffer, "규격(%s) 길이(%.0f) 크로스헤드(%s) ", objectInfo.records.at(yy).at(1).c_str (), round (length*1000, 0), objectInfo.records.at(yy).at(4).c_str ());
								} else {
									sprintf (buffer, "규격(%s) 길이(%.0f) ", objectInfo.records.at(yy).at(1).c_str (), round (length*1000, 0));
								}
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);

							} else if (objectInfo.keyDesc.at(xx).compare ("서포트") == 0) {
								length = atof (objectInfo.records.at(yy).at(2).c_str ());
								if (atoi (objectInfo.records.at(yy).at(3).c_str ()) == 1) {
									sprintf (buffer, "규격(%s) 길이(%.0f) 크로스헤드(%s) ", objectInfo.records.at(yy).at(1).c_str (), round (length*1000, 0), objectInfo.records.at(yy).at(4).c_str ());
								} else {
									sprintf (buffer, "규격(%s) 길이(%.0f) ", objectInfo.records.at(yy).at(1).c_str (), round (length*1000, 0));
								}
								fprintf (fp, buffer);
								fprintf (fp_unite, buffer);

							} else {
								for (zz = 0 ; zz < objectInfo.nInfo.at(xx) ; ++zz) {
									// 변수별 값 출력
									sprintf (buffer, "%s(%s) ", objectInfo.varDesc.at(xx).at(zz).c_str (), objectInfo.records.at(yy).at(zz+1).c_str ());
									fprintf (fp, buffer);
									fprintf (fp_unite, buffer);
								}
							}

							// 수량 출력
							sprintf (buffer, ": %s EA\n", objectInfo.records.at(yy).at(objectInfo.records.at(yy).size ()-1).c_str ());
							fprintf (fp, buffer);
							fprintf (fp_unite, buffer);
						}
					}
				}
			} catch (exception& ex) {
				WriteReport_Alert ("출력 함수에서 오류 발생: %s", ex.what ());
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

	// 그룹화 일시정지 OFF
	suspendGroups (false);

	ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);
	location.ToDisplayText (&resultString);
	WriteReport_Alert ("결과물을 다음 위치에 저장했습니다.\n\n%s\n또는 프로젝트 파일이 있는 폴더", resultString.ToCStr ().Get ());

	return	err;
}

// 객체의 레코드 수량 n 증가
int		quantityPlusN (vector<vector<string>> *db, vector<string> record, int n)
{
	int		xx, yy;
	size_t	vecLen;
	size_t	inVecLen1, inVecLen2;
	int		diff;
	int		value;
	char	tempStr [512];

	vecLen = db->size ();

	try {
		for (xx = 0 ; xx < vecLen ; ++xx) {
			// 변수 값도 동일할 경우
			inVecLen1 = db->at(xx).size () - 1;		// 끝의 개수 필드를 제외한 길이
			inVecLen2 = record.size ();

			if (inVecLen1 == inVecLen2) {
				// 일치하지 않는 필드가 하나라도 있는지 찾아볼 것
				diff = 0;
				for (yy = 0 ; yy < inVecLen1 ; ++yy) {
					if (my_strcmp (db->at(xx).at(yy).c_str (), record.at(yy).c_str ()) != 0)
						diff++;
				}

				// 모든 필드가 일치하면
				if (diff == 0) {
					value = atoi (db->at(xx).back ().c_str ());
					value += n;
					sprintf (tempStr, "%d", value);
					db->at(xx).pop_back ();
					db->at(xx).push_back (tempStr);
					return value;
				}
			}
		}
	} catch (exception& ex) {
		WriteReport_Alert ("quantityPlusN 함수에서 오류 발생: %s", ex.what ());
	}

	// 없으면 신규 레코드 추가하고 n 리턴
	sprintf (tempStr, "%d", n);
	record.push_back (tempStr);
	db->push_back (record);

	return n;
}

// 부재별 선택 후 보여주기
GSErrCode	filterSelection (void)
{
	GSErrCode	err = NoError;
	short		xx, yy;
	short		result;
	const char*	tempStr;
	bool		foundObj;

	FILE	*fp;				// 파일 포인터
	char	line [10240];		// 파일에서 읽어온 라인 하나
	char	*token;				// 읽어온 문자열의 토큰
	short	lineCount;			// 읽어온 라인 수
	short	tokCount;			// 읽어온 토큰 개수
	char	nthToken [200][50];	// n번째 토큰

	API_Element			elem;
	API_ElementMemo		memo;

	// GUID 저장을 위한 변수
	GS::Array<API_Guid>	objects;	long nObjects	= 0;
	GS::Array<API_Guid>	walls;		long nWalls		= 0;
	GS::Array<API_Guid>	columns;	long nColumns	= 0;
	GS::Array<API_Guid>	beams;		long nBeams		= 0;
	GS::Array<API_Guid>	slabs;		long nSlabs		= 0;
	GS::Array<API_Guid>	roofs;		long nRoofs		= 0;
	GS::Array<API_Guid>	meshes;		long nMeshes	= 0;
	GS::Array<API_Guid>	morphs;		long nMorphs	= 0;
	GS::Array<API_Guid>	shells;		long nShells	= 0;

	// 조건에 맞는 객체들의 GUID 저장
	GS::Array<API_Guid> selection_known;
	GS::Array<API_Guid> selection_unknown;

	
	// 그룹화 일시정지 ON
	suspendGroups (true);

	ACAPI_Element_GetElemList (API_ObjectID, &objects, APIFilt_OnVisLayer);	nObjects = objects.GetSize ();	// 보이는 레이어 상의 객체 타입만 가져오기
	ACAPI_Element_GetElemList (API_WallID, &walls, APIFilt_OnVisLayer);		nWalls = walls.GetSize ();		// 보이는 레이어 상의 벽 타입만 가져오기
	ACAPI_Element_GetElemList (API_ColumnID, &columns, APIFilt_OnVisLayer);	nColumns = columns.GetSize ();	// 보이는 레이어 상의 기둥 타입만 가져오기
	ACAPI_Element_GetElemList (API_BeamID, &beams, APIFilt_OnVisLayer);		nBeams = beams.GetSize ();		// 보이는 레이어 상의 보 타입만 가져오기
	ACAPI_Element_GetElemList (API_SlabID, &slabs, APIFilt_OnVisLayer);		nSlabs = slabs.GetSize ();		// 보이는 레이어 상의 슬래브 타입만 가져오기
	ACAPI_Element_GetElemList (API_RoofID, &roofs, APIFilt_OnVisLayer);		nRoofs = roofs.GetSize ();		// 보이는 레이어 상의 루프 타입만 가져오기
	ACAPI_Element_GetElemList (API_MeshID, &meshes, APIFilt_OnVisLayer);	nMeshes = meshes.GetSize ();	// 보이는 레이어 상의 메시 타입만 가져오기
	ACAPI_Element_GetElemList (API_MorphID, &morphs, APIFilt_OnVisLayer);	nMorphs = morphs.GetSize ();	// 보이는 레이어 상의 모프 타입만 가져오기
	ACAPI_Element_GetElemList (API_ShellID, &shells, APIFilt_OnVisLayer);	nShells = shells.GetSize ();	// 보이는 레이어 상의 셸 타입만 가져오기

	if (nObjects == 0 && nWalls == 0 && nColumns == 0 && nBeams == 0 && nSlabs == 0 && nRoofs == 0 && nMeshes == 0 && nMorphs == 0 && nShells == 0) {
		result = DGAlert (DG_INFORMATION, L"종료 알림", L"아무 객체도 존재하지 않습니다.", "", L"확인", "", "");
		return	err;
	}

	// 객체 정보 파일 가져오기
	fp = fopen ("C:\\objectInfo.csv", "r");

	if (fp == NULL) {
		result = DGAlert (DG_ERROR, L"파일 오류", L"objectInfo.csv 파일을 C:\\로 복사하십시오.", "", L"확인", "", "");
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

		if (err == NoError && elem.header.hasMemo) {
			err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

			if (err == NoError) {
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
			}

			ACAPI_DisposeElemMemoHdls (&memo);
		}
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
		selectElements (selection_known);

		// 알려지지 않은 Object 타입 선택
		if (visibleObjInfo.bShow_Unknown == true)	selectElements (selection_unknown);

		// 나머지 타입
		if (visibleObjInfo.bShow_Walls == true)		selectElements (walls);
		if (visibleObjInfo.bShow_Columns == true)	selectElements (columns);
		if (visibleObjInfo.bShow_Beams == true)		selectElements (beams);
		if (visibleObjInfo.bShow_Slabs == true)		selectElements (slabs);
		if (visibleObjInfo.bShow_Roofs == true)		selectElements (roofs);
		if (visibleObjInfo.bShow_Meshes == true)	selectElements (meshes);
		if (visibleObjInfo.bShow_Morphs == true)	selectElements (morphs);
		if (visibleObjInfo.bShow_Shells == true)	selectElements (shells);
		
		// 선택한 것만 3D로 보여주기
		ACAPI_Automate (APIDo_ShowSelectionIn3DID, NULL, NULL);
	}

	// 그룹화 일시정지 OFF
	suspendGroups (false);

	return	err;
}

// 보 테이블폼 물량표 작성을 위한 클래스 생성자 (초기화)
BeamTableformInfo::BeamTableformInfo ()
{
	this->init ();
}

// 초기화
void BeamTableformInfo::init ()
{
	this->iBeamDirection = 0;
	this->nCells = 0;

	for (short xx = 0 ; xx < 50 ; ++xx) {
		this->cells [xx].euroform_leftHeight = 0.0;
		this->cells [xx].euroform_rightHeight = 0.0;
		this->cells [xx].euroform_bottomWidth = 0.0;

		this->cells [xx].plywoodOnly_leftHeight = 0.0;
		this->cells [xx].plywoodOnly_rightHeight = 0.0;
		this->cells [xx].plywoodOnly_bottomWidth = 0.0;

		this->cells [xx].length_left = 0.0;
		this->cells [xx].length_right = 0.0;
		this->cells [xx].length_bottom = 0.0;
	}
}

// 보 테이블폼 물량 정보 내보내기
GSErrCode	exportBeamTableformInformation (void)
{
	GSErrCode	err = NoError;
	unsigned short		xx, yy, zz;
	short	mm;
	//bool	regenerate = true;

	GS::Array<API_Guid>		objects;
	long					nObjects = 0;

	API_Element			elem;
	API_ElementMemo		memo;
	API_ElemInfo3D		info3D;

	vector<objectInBeamTableform>	objectList;				// 객체 리스트
	objectInBeamTableform			newObject;
	BeamTableformInfo				tableformInfo;			// 보 테이블폼 정보
	BeamTableformInfo				tableformInfoSummary;	// 보 테이블폼 정보 (요약판)
	BeamTableformEuroformCellType	euroformCellType;		// 유로폼 셀 타입 정보

	double				xmin, xmax, ymin, ymax;
	int					ang_x, ang_y;
	bool				bValid, bFirst;
	double				xcenter, ycenter;

	// 레이어 관련 변수
	short			nLayers;
	API_Attribute	attrib;
	short			nVisibleLayers = 0;
	short			visLayerList [1024];
	char			fullLayerName [512];
	vector<LayerList>	layerList;

	// 기타
	char			tempStr [512];
	char			buffer [512];
	char			tempBuffer [512];
	char			filename [512];

	// 엑셀 파일로 기둥 정보 내보내기
	// 파일 저장을 위한 변수
	API_SpecFolderID	specFolderID = API_ApplicationFolderID;
	IO::Location		location;
	GS::UniString		resultString;
	API_MiscAppInfo		miscAppInfo;
	FILE				*fp;


	// 그룹화 일시정지 ON
	suspendGroups (true);

	ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);

	// 프로젝트 내 레이어 개수를 알아냄
	nLayers = getLayerCount ();

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

	// 레이어 이름과 인덱스 저장
	for (xx = 0 ; xx < nVisibleLayers ; ++xx) {
		BNZeroMemory (&attrib, sizeof (API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = visLayerList [xx];
		err = ACAPI_Attribute_Get (&attrib);

		sprintf (fullLayerName, "%s", attrib.layer.head.name);
		fullLayerName [strlen (fullLayerName)] = '\0';

		LayerList newLayerItem;
		newLayerItem.layerInd = visLayerList [xx];
		newLayerItem.layerName = fullLayerName;

		layerList.push_back (newLayerItem);
	}

	// 레이어 이름 기준으로 정렬하여 레이어 인덱스 순서 변경
	sort (layerList.begin (), layerList.end (), compareLayerName);		// 레이어 이름 기준 오름차순 정렬

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
	sprintf (filename, "%s - 보 테이블폼 물량표.csv", miscAppInfo.caption);
	fp = fopen (filename, "w+");

	if (fp == NULL) {
		DGAlert (DG_ERROR, L"오류", L"엑셀파일을 만들 수 없습니다.", "", L"확인", "", "");
		return	NoError;
	}

	// 유로폼 셀 타입 정보 초기화
	euroformCellType.nCells = 0;

	// 보이는 레이어들을 하나씩 순회하면서 전체 요소들을 선택한 후 "보 테이블폼 물량표" 루틴 실행
	for (mm = 1 ; mm <= nVisibleLayers ; ++mm) {
		BNZeroMemory (&attrib, sizeof (API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		//attrib.layer.head.index = visLayerList [mm-1];
		attrib.layer.head.index = layerList [mm-1].layerInd;
		err = ACAPI_Attribute_Get (&attrib);

		if (err == NoError) {
			// 레이어 보이기
			if ((attrib.layer.head.flags & APILay_Hidden) == true) {
				attrib.layer.head.flags ^= APILay_Hidden;
				ACAPI_Attribute_Modify (&attrib, NULL);
			}

			// 초기화
			tableformInfo.init ();
			tableformInfoSummary.init ();
			objectList.clear ();

			// 모든 요소 가져오기
			ACAPI_Element_GetElemList (API_ObjectID, &objects, APIFilt_OnVisLayer);	// 보이는 레이어에 있음, 객체 타입만
			nObjects = objects.GetSize ();

			if (nObjects == 0)
				continue;

			// 레이어 이름 가져옴
			sprintf (fullLayerName, "%s", attrib.layer.head.name);
			fullLayerName [strlen (fullLayerName)] = '\0';

			for (xx = 0 ; xx < nObjects ; ++xx) {
				BNZeroMemory (&elem, sizeof (API_Element));
				BNZeroMemory (&memo, sizeof (API_ElementMemo));
				elem.header.guid = objects [xx];
				err = ACAPI_Element_Get (&elem);
				err = ACAPI_Element_Get3DInfo (elem.header, &info3D);

				if (err == NoError && elem.header.hasMemo) {
					err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

					if (err == NoError) {
						bValid = false;

						// 초기화
						newObject.attachPosition = NO_POSITION;
						newObject.objType = NONE;
						newObject.length = 0.0;
						newObject.width = 0.0;
						newObject.length = 0.0;
						newObject.origin.x = 0.0;
						newObject.origin.y = 0.0;
						newObject.origin.z = 0.0;
						newObject.minPos.x = 0.0;
						newObject.minPos.y = 0.0;
						newObject.minPos.z = 0.0;
						newObject.maxPos.x = 0.0;
						newObject.maxPos.y = 0.0;
						newObject.maxPos.z = 0.0;

						// 원점 좌표 저장
						newObject.origin.x = elem.object.pos.x;
						newObject.origin.y = elem.object.pos.y;
						newObject.origin.z = elem.object.level;

						// 최소점, 최대점 좌표 저장
						newObject.minPos.x = info3D.bounds.xMin;
						newObject.minPos.y = info3D.bounds.yMin;
						newObject.minPos.z = info3D.bounds.zMin;
						newObject.maxPos.x = info3D.bounds.xMax;
						newObject.maxPos.y = info3D.bounds.yMax;
						newObject.maxPos.z = info3D.bounds.zMax;

						// 셀에 넣기 전에는 false 값을 넣어둠
						newObject.bUsed = false;
						
						// 객체의 타입, 너비와 길이를 저장
						if (my_strcmp (getParameterStringByName (&memo, "u_comp"), "유로폼") == 0) {
							if (my_strcmp (getParameterStringByName (&memo, "u_ins"), "벽눕히기") == 0) {
								newObject.objType = EUROFORM;

								sprintf (tempStr, "%s", getParameterStringByName (&memo, "eu_wid"));
								newObject.width = atof (tempStr) / 1000.0;
								sprintf (tempStr, "%s", getParameterStringByName (&memo, "eu_hei"));
								newObject.length = atof (tempStr) / 1000.0;

								ang_x = (int)round (RadToDegree (getParameterValueByName (&memo, "ang_x")), 0);
								ang_y = (int)round (RadToDegree (getParameterValueByName (&memo, "ang_y")), 0);

								if ( ((ang_x ==   0) && (ang_y ==  0)) ||
									 ((ang_x ==  90) && (ang_y ==  0)) ||
									 ((ang_x == 180) && (ang_y ==  0)) ||
									 ((ang_x ==  90) && (ang_y == 90)) )
									newObject.attachPosition = LEFT_SIDE;
								else if ( ((ang_x ==   0) && (ang_y == 90)) ||
										  ((ang_x == 180) && (ang_y == 90)) )
									newObject.attachPosition = BOTTOM_SIDE;

								bValid = true;
							} else if (my_strcmp (getParameterStringByName (&memo, "u_ins"), "벽세우기") == 0) {
								newObject.objType = EUROFORM;

								sprintf (tempStr, "%s", getParameterStringByName (&memo, "eu_wid"));
								newObject.width = atof (tempStr) / 1000.0;
								sprintf (tempStr, "%s", getParameterStringByName (&memo, "eu_hei"));
								newObject.length = atof (tempStr) / 1000.0;

								ang_x = (int)round (RadToDegree (getParameterValueByName (&memo, "ang_x")), 0);
								ang_y = (int)round (RadToDegree (getParameterValueByName (&memo, "ang_y")), 0);

								if ( ((ang_x ==   0) && (ang_y ==  0)) ||
									 ((ang_x ==  90) && (ang_y ==  0)) ||
									 ((ang_x == 180) && (ang_y ==  0)) )
									newObject.attachPosition = LEFT_SIDE;
								else if ( ((ang_x ==   0) && (ang_y == 90)) ||
										  ((ang_x == 180) && (ang_y == 90)) )
									newObject.attachPosition = BOTTOM_SIDE;

								bValid = true;
							}
						}

						else if (my_strcmp (getParameterStringByName (&memo, "g_comp"), "합판") == 0) {
							if (abs (getParameterValueByName (&memo, "sogak") - 1.0) < EPS) {
								newObject.objType = PLYWOOD;

								ang_x = (int)round (RadToDegree (getParameterValueByName (&memo, "p_ang")), 0);

								if (my_strcmp (getParameterStringByName (&memo, "w_dir"), "벽눕히기") == 0) {
									newObject.width = getParameterValueByName (&memo, "p_wid");
									newObject.length = getParameterValueByName (&memo, "p_leng");
									if ( (ang_x == 0) || (ang_x == 180) )
										newObject.attachPosition = LEFT_SIDE;
									else
										newObject.attachPosition = BOTTOM_SIDE;
								} else if (my_strcmp (getParameterStringByName (&memo, "w_dir"), "벽세우기") == 0) {
									newObject.width = getParameterValueByName (&memo, "p_leng");
									newObject.length = getParameterValueByName (&memo, "p_wid");
									if ( (ang_x == 0) || (ang_x == 180) )
										newObject.attachPosition = LEFT_SIDE;
									else
										newObject.attachPosition = BOTTOM_SIDE;
								} else if (my_strcmp (getParameterStringByName (&memo, "w_dir"), "바닥깔기") == 0) {
									newObject.width = getParameterValueByName (&memo, "p_wid");
									newObject.length = getParameterValueByName (&memo, "p_leng");
									if ( (ang_x == 90) || (ang_x == 270) )
										newObject.attachPosition = LEFT_SIDE;
									else
										newObject.attachPosition = BOTTOM_SIDE;
								} else if (my_strcmp (getParameterStringByName (&memo, "w_dir"), "바닥덮기") == 0) {
									newObject.width = getParameterValueByName (&memo, "p_wid");
									newObject.length = getParameterValueByName (&memo, "p_leng");
									if ( (ang_x == 90) || (ang_x == 270) )
										newObject.attachPosition = LEFT_SIDE;
									else
										newObject.attachPosition = BOTTOM_SIDE;
								}

								// 단, 전체를 덮는 합판은 너비가 200mm 이상이어야 함
								if (newObject.width > 0.200 - EPS)
									bValid = true;
								else
									bValid = false;
							}
						}

						if (bValid == true)
							objectList.push_back (newObject);
					}

					ACAPI_DisposeElemMemoHdls (&memo);
				}
			}

			nObjects = (long)objectList.size ();

			// 보 방향을 찾아냄 (가로인가, 세로인가?)
			bFirst = false;
			for (xx = 0 ; xx < nObjects ; ++xx) {
				if (objectList [xx].attachPosition != BOTTOM_SIDE) {
					if (bFirst == false) {
						xmin = xmax = objectList [xx].origin.x;
						ymin = ymax = objectList [xx].origin.y;
						bFirst = true;
					} else {
						if (xmin > objectList [xx].origin.x)	xmin = objectList [xx].origin.x;
						if (ymin > objectList [xx].origin.y)	ymin = objectList [xx].origin.y;
						if (xmax < objectList [xx].origin.x)	xmax = objectList [xx].origin.x;
						if (ymax < objectList [xx].origin.y)	ymax = objectList [xx].origin.y;
					}
				}
			}
			if (abs (xmax - xmin) > abs (ymax - ymin))
				tableformInfo.iBeamDirection = HORIZONTAL_DIRECTION;
			else
				tableformInfo.iBeamDirection = VERTICAL_DIRECTION;

			// 객체 정렬하기
			for (xx = 0 ; xx < nObjects ; ++xx) {
				if (tableformInfo.iBeamDirection == HORIZONTAL_DIRECTION)
					sort (objectList.begin (), objectList.end (), comparePosX);		// X 오름차순 정렬
				else
					sort (objectList.begin (), objectList.end (), comparePosY);		// Y 오름차순 정렬
			}

			// 센터 위치 찾기
			xcenter = (xmax - xmin) / 2 + xmin;
			ycenter = (ymax - ymin) / 2 + ymin;

			// 수집된 정보 분류하기
			for (xx = 0 ; xx < nObjects ; ++xx) {
				// 왼쪽/아래쪽(최소)과 오른쪽/위쪽(최대) 측판 사이의 중간점을 기준으로 구분
				if (tableformInfo.iBeamDirection == HORIZONTAL_DIRECTION) {
					if (objectList [xx].origin.y > ycenter) {
						if (objectList [xx].attachPosition == LEFT_SIDE)
							objectList [xx].attachPosition = RIGHT_SIDE;	// 위쪽
					}
				} else {
					if (objectList [xx].origin.x > xcenter) {
						if (objectList [xx].attachPosition == LEFT_SIDE)
							objectList [xx].attachPosition = RIGHT_SIDE;	// 오른쪽
					}
				}
			}

			// 자재들의 시작점/끝점 좌표를 업데이트
			for (xx = 0 ; xx < nObjects ; ++xx) {
				if (tableformInfo.iBeamDirection == HORIZONTAL_DIRECTION) {
					// 가로 방향
					objectList [xx].beginPos = objectList [xx].minPos.x;
					objectList [xx].endPos = objectList [xx].beginPos + objectList [xx].length;
				} else {
					// 세로 방향
					objectList [xx].beginPos = objectList [xx].minPos.y;
					objectList [xx].endPos = objectList [xx].beginPos + objectList [xx].length;
				}
			}

			// 시작점/끝점 위치와 일치하는 객체들을 묶어서 셀로 저장함 (유로폼 또는 유로폼을 3개 또는 2개씩 그룹화)
			short nCells_left = 0;
			short nCells_right = 0;
			short nCells_bottom = 0;

			for (xx = 0 ; xx < nObjects ; ++xx) {
				// 연속된 3개의 객체가 유로폼인가?
				if ((objectList [xx].objType == EUROFORM) && (objectList [xx+1].objType == EUROFORM) && (objectList [xx+2].objType == EUROFORM) &&
					(objectList [xx].bUsed == false) && (objectList [xx+1].bUsed == false) && (objectList [xx+2].bUsed == false)) {

					for (yy = 0 ; yy < 3 ; ++yy) {
						if (objectList [xx + yy].objType == EUROFORM) {
							if (objectList [xx + yy].attachPosition == LEFT_SIDE) {
								tableformInfo.cells [nCells_left].euroform_leftHeight = objectList [xx + yy].width;
								tableformInfo.cells [nCells_left].length_left = objectList [xx + yy].length;
								++ nCells_left;
							}
							else if (objectList [xx + yy].attachPosition == RIGHT_SIDE) {
								tableformInfo.cells [nCells_right].euroform_rightHeight = objectList [xx + yy].width;
								tableformInfo.cells [nCells_right].length_right = objectList [xx + yy].length;
								++ nCells_right;
							}
							else if (objectList [xx + yy].attachPosition == BOTTOM_SIDE) {
								tableformInfo.cells [nCells_bottom].euroform_bottomWidth = objectList [xx + yy].width;
								tableformInfo.cells [nCells_bottom].length_bottom = objectList [xx + yy].length;
								++ nCells_bottom;
							}
						}
							
						objectList [xx + yy].bUsed = true;
					}
					continue;
				}

				// 연속된 2개의 객체가 유로폼인가?
				else if ((objectList [xx].objType == EUROFORM) && (objectList [xx+1].objType == EUROFORM) &&
					(objectList [xx].bUsed == false) && (objectList [xx+1].bUsed == false)) {

					for (yy = 0 ; yy < 2 ; ++yy) {
						if (objectList [xx + yy].objType == EUROFORM) {
							if (objectList [xx + yy].attachPosition == LEFT_SIDE) {
								tableformInfo.cells [nCells_left].euroform_leftHeight = objectList [xx + yy].width;
								tableformInfo.cells [nCells_left].length_left = objectList [xx + yy].length;
								++ nCells_left;
							}
							else if (objectList [xx + yy].attachPosition == RIGHT_SIDE) {
								tableformInfo.cells [nCells_right].euroform_rightHeight = objectList [xx + yy].width;
								tableformInfo.cells [nCells_right].length_right = objectList [xx + yy].length;
								++ nCells_right;
							}
							else if (objectList [xx + yy].attachPosition == BOTTOM_SIDE) {
								tableformInfo.cells [nCells_bottom].euroform_bottomWidth = objectList [xx + yy].width;
								tableformInfo.cells [nCells_bottom].length_bottom = objectList [xx + yy].length;
								++ nCells_bottom;
							}
						}
							
						objectList [xx + yy].bUsed = true;
					}
					continue;
				}

				// 연속된 3개의 객체가 합판인가?
				else if ((objectList [xx].objType == PLYWOOD) && (objectList [xx+1].objType == PLYWOOD) && (objectList [xx+2].objType == PLYWOOD) &&
					(objectList [xx].bUsed == false) && (objectList [xx+1].bUsed == false) && (objectList [xx+2].bUsed == false)) {

					for (yy = 0 ; yy < 3 ; ++yy) {
						if (objectList [xx + yy].objType == PLYWOOD) {
							if (objectList [xx + yy].attachPosition == LEFT_SIDE) {
								tableformInfo.cells [nCells_left].plywoodOnly_leftHeight = objectList [xx + yy].width;
								tableformInfo.cells [nCells_left].length_left = objectList [xx + yy].length;
								++ nCells_left;
							}
							else if (objectList [xx + yy].attachPosition == RIGHT_SIDE) {
								tableformInfo.cells [nCells_right].plywoodOnly_rightHeight = objectList [xx + yy].width;
								tableformInfo.cells [nCells_right].length_right = objectList [xx + yy].length;
								++ nCells_right;
							}
							else if (objectList [xx + yy].attachPosition == BOTTOM_SIDE) {
								tableformInfo.cells [nCells_bottom].plywoodOnly_bottomWidth = objectList [xx + yy].width;
								tableformInfo.cells [nCells_bottom].length_bottom = objectList [xx + yy].length;
								++ nCells_bottom;
							}
						}
							
						objectList [xx + yy].bUsed = true;
					}
					continue;
				}

				// 연속된 2개의 객체가 합판인가?
				else if ((objectList [xx].objType == PLYWOOD) && (objectList [xx+1].objType == PLYWOOD) &&
					(objectList [xx].bUsed == false) && (objectList [xx+1].bUsed == false)) {

					for (yy = 0 ; yy < 2 ; ++yy) {
						if (objectList [xx + yy].objType == PLYWOOD) {
							if (objectList [xx + yy].attachPosition == LEFT_SIDE) {
								tableformInfo.cells [nCells_left].plywoodOnly_leftHeight = objectList [xx + yy].width;
								tableformInfo.cells [nCells_left].length_left = objectList [xx + yy].length;
								++ nCells_left;
							}
							else if (objectList [xx + yy].attachPosition == RIGHT_SIDE) {
								tableformInfo.cells [nCells_right].plywoodOnly_rightHeight = objectList [xx + yy].width;
								tableformInfo.cells [nCells_right].length_right = objectList [xx + yy].length;
								++ nCells_right;
							}
							else if (objectList [xx + yy].attachPosition == BOTTOM_SIDE) {
								tableformInfo.cells [nCells_bottom].plywoodOnly_bottomWidth = objectList [xx + yy].width;
								tableformInfo.cells [nCells_bottom].length_bottom = objectList [xx + yy].length;
								++ nCells_bottom;
							}
						}
							
						objectList [xx + yy].bUsed = true;
					}
					continue;
				}
			}

			// 보 테이블폼 정보 (요약판) 만들기
			tableformInfoSummary.iBeamDirection = tableformInfo.iBeamDirection;
			bool	bBeforeCellEuroform;	// 이전 셀이 유로폼입니까?

			if (nObjects != 0) {
				// 성공한 경우
				tableformInfo.nCells = nCells_left;
				tableformInfoSummary.nCells = 0;

				for (xx = 0 ; xx < tableformInfo.nCells ; ++xx) {
					if (xx == 0) {
						// 0번 셀은 무조건 저장
						tableformInfoSummary.cells [0] = tableformInfo.cells [0];
						++ tableformInfoSummary.nCells;
							
						// 이전 셀 정보 저장함
						if (tableformInfo.cells [0].euroform_leftHeight > EPS)
							bBeforeCellEuroform = true;
						else
							bBeforeCellEuroform = false;
					} else {
						if (tableformInfo.cells [xx].euroform_leftHeight > EPS) {
							// 현재 셀이 유로폼이면,
							if (bBeforeCellEuroform == true) {
								// 이전 셀이 유로폼인 경우, 이전 셀에 현재 셀의 길이 합산
								tableformInfoSummary.cells [tableformInfoSummary.nCells - 1].length_left += tableformInfo.cells [xx].length_left;
								tableformInfoSummary.cells [tableformInfoSummary.nCells - 1].length_right += tableformInfo.cells [xx].length_right;
								tableformInfoSummary.cells [tableformInfoSummary.nCells - 1].length_bottom += tableformInfo.cells [xx].length_bottom;
							} else {
								// 이전 셀이 합판인 경우, 새로운 셀 추가
								tableformInfoSummary.cells [tableformInfoSummary.nCells] = tableformInfo.cells [xx];
								++ tableformInfoSummary.nCells;
							}

							bBeforeCellEuroform = true;
						} else {
							// 현재 셀이 합판이면,
							if (bBeforeCellEuroform == true) {
								// 이전 셀이 유로폼인 경우, 새로운 셀 추가
								tableformInfoSummary.cells [tableformInfoSummary.nCells] = tableformInfo.cells [xx];
								++ tableformInfoSummary.nCells;
							} else {
								// 이전 셀이 합판인 경우, 이전 셀에 현재 셀의 길이 합산
								tableformInfoSummary.cells [tableformInfoSummary.nCells - 1].length_left += tableformInfo.cells [xx].length_left;
								tableformInfoSummary.cells [tableformInfoSummary.nCells - 1].length_right += tableformInfo.cells [xx].length_right;
								tableformInfoSummary.cells [tableformInfoSummary.nCells - 1].length_bottom += tableformInfo.cells [xx].length_bottom;
							}

							bBeforeCellEuroform = false;
						}
					}
				}
			}

			// 유일한 유로폼 셀을 저장함
			bool	bFoundSameCell = false;		// 동일한 셀을 찾았는지 여부를 체크함
			for (xx = 0 ; xx < tableformInfoSummary.nCells ; ++xx) {
				if (tableformInfoSummary.cells [xx].euroform_leftHeight > EPS) {
					// euroformCellType 내부에 현재 셀(유로폼 셀만)과 일치하는 셀이 있는지 확인할 것
					for (yy = 0 ; yy < euroformCellType.nCells ; ++yy) {
						if (tableformInfoSummary.cells [xx] == euroformCellType.cells [yy])
							bFoundSameCell = true;
					}

					// 동일한 타입의 유로폼 셀 타입을 찾지 못했을 경우,
					if (bFoundSameCell == false) {
						// 새로운 유로폼 셀 타입 추가
						euroformCellType.cells [euroformCellType.nCells] = tableformInfoSummary.cells [xx];
						++ euroformCellType.nCells;
					}
				}
			}

			// 유로폼 셀 타입별 레이어 이름 저장하기
			bool	bDuplicatedLayername = false;
			for (xx = 0 ; xx < tableformInfoSummary.nCells ; ++xx) {
				if (tableformInfoSummary.cells [xx].euroform_leftHeight > EPS) {
					for (yy = 0 ; yy < euroformCellType.nCells ; ++yy) {
						if (tableformInfoSummary.cells [xx] == euroformCellType.cells [yy]) {
							// 레이어 이름이 중복된 것은 없는지 확인할 것
							for (zz = 0 ; zz < euroformCellType.layerNames [yy].size () ; ++zz) {
								if (euroformCellType.layerNames [yy].at (zz).compare (fullLayerName) == 0)
									bDuplicatedLayername = true;
							}

							if (bDuplicatedLayername == false)
								euroformCellType.layerNames [yy].push_back (fullLayerName);
						}
					}
				}
			}

			// 정보 출력
			if (tableformInfoSummary.nCells > 0) {
				sprintf (buffer, ",,<< 레이어 : %s >>\n", fullLayerName);
				fprintf (fp, buffer);

				// 해당 레이어에 적용된 타입 코드를 1번째 필드에 출력
				strcpy (buffer, "");
				for (xx = 0 ; xx < tableformInfoSummary.nCells ; ++xx) {
					if (tableformInfoSummary.cells [xx].euroform_leftHeight > EPS) {
						for (yy = 0 ; yy < euroformCellType.nCells ; ++yy) {
							if (tableformInfoSummary.cells [xx] == euroformCellType.cells [yy]) {
								sprintf (tempBuffer, "%.0f x %.0f x %.0f x %.0f ", euroformCellType.cells [yy].euroform_bottomWidth * 1000, euroformCellType.cells [yy].euroform_leftHeight * 1000, euroformCellType.cells [yy].euroform_rightHeight * 1000, euroformCellType.cells [yy].length_left * 1000);
								strcat (buffer, tempBuffer);
							}
						}
					}
				}
				strcat (buffer, ",");
				fprintf (fp, buffer);

				// 레이어 이름 끝에 2개 혹은 1개 필드가 숫자로 되어 있으면 2번째 필드에 출력 (숫자 필드가 없으면 빈 필드로 둠)
				strcpy (tempBuffer, fullLayerName);

				char* token = strtok (tempBuffer, "-");
				string insElem;
				vector<string> layerNameComp;

				while (token != NULL) {
					if (strlen (token) > 0) {
						insElem = token;
						layerNameComp.push_back (insElem);
					}
					token = strtok (NULL, "-");
				}

				sprintf (buffer, ",");
				if (layerNameComp.size () >= 2) {
					// 마지막 요소와 마지막 직전 요소가 모두 숫자일 경우
					int lastNum = atoi (layerNameComp.at (layerNameComp.size () - 1).c_str ());
					int lastBeforeNum = atoi (layerNameComp.at (layerNameComp.size () - 2).c_str ());

					if ((lastNum != 0) && (lastBeforeNum != 0)) {
						sprintf (buffer, "%s-%s,", layerNameComp.at (layerNameComp.size () - 2).c_str (), layerNameComp.at (layerNameComp.size () - 1).c_str ());
					} else if (lastNum != 0) {
						sprintf (buffer, "\'%s,", layerNameComp.at (layerNameComp.size () - 1).c_str ());
					} else {
						sprintf (buffer, ",");
					}
				}
				fprintf (fp, buffer);

				// 레이어에 배치된 보 테이블폼 정보를 출력함
				strcpy (buffer, "");
				for (xx = 0 ; xx < tableformInfoSummary.nCells ; ++xx) {
					if (tableformInfoSummary.cells [xx].euroform_leftHeight > EPS)
						strcat (buffer, "유로폼,,,,");
					else
						strcat (buffer, "합판,,,,");
				}
				fprintf (fp, buffer);

				strcpy (buffer, "\n,,");
				for (xx = 0 ; xx < tableformInfoSummary.nCells ; ++xx) {
					strcat (buffer, "밑면,측면1,측면2,,");
				}
				fprintf (fp, buffer);

				strcpy (buffer, "\n,,");
				for (xx = 0 ; xx < tableformInfoSummary.nCells ; ++xx) {
					if (tableformInfoSummary.cells [xx].euroform_leftHeight > EPS)
						sprintf (tempBuffer, "%.0f,%.0f,%.0f,,", tableformInfoSummary.cells [xx].euroform_bottomWidth * 1000, tableformInfoSummary.cells [xx].euroform_leftHeight * 1000, tableformInfoSummary.cells [xx].euroform_rightHeight * 1000);
					else
						sprintf (tempBuffer, "%.0f,%.0f,%.0f,,", tableformInfoSummary.cells [xx].plywoodOnly_bottomWidth * 1000, tableformInfoSummary.cells [xx].plywoodOnly_leftHeight * 1000, tableformInfoSummary.cells [xx].plywoodOnly_rightHeight * 1000);
					strcat (buffer, tempBuffer);
				}
				fprintf (fp, buffer);

				strcpy (buffer, "\n,,");
				for (xx = 0 ; xx < tableformInfoSummary.nCells ; ++xx) {
					strcat (buffer, "길이,길이,길이,,");
				}
				fprintf (fp, buffer);

				strcpy (buffer, "\n,,");
				for (xx = 0 ; xx < tableformInfoSummary.nCells ; ++xx) {
					sprintf (tempBuffer, "%.0f,%.0f,%.0f,,", tableformInfoSummary.cells [xx].length_bottom * 1000, tableformInfoSummary.cells [xx].length_left * 1000, tableformInfoSummary.cells [xx].length_right * 1000);
					strcat (buffer, tempBuffer);
				}
				fprintf (fp, buffer);

				strcpy (buffer, "\n\n");
				fprintf (fp, buffer);

			} else {
				// 실패한 경우
				sprintf (buffer, ",,<< 레이어 : %s >>\n", fullLayerName);
				fprintf (fp, buffer);

				sprintf (buffer, "\n,,정규화된 보 테이블폼 레이어가 아닙니다.\n");
				fprintf (fp, buffer);

				strcpy (buffer, "\n\n");
				fprintf (fp, buffer);
			}

			// 객체 비우기
			if (!objects.IsEmpty ())
				objects.Clear ();
			if (!objectList.empty ())
				objectList.clear ();

			// 레이어 숨기기
			attrib.layer.head.flags |= APILay_Hidden;
			ACAPI_Attribute_Modify (&attrib, NULL);
		}
	}

	// 유로폼 셀 타입 정보를 파일 끝 부분에 기록할 것
	if (euroformCellType.nCells > 0) {
		sprintf (buffer, "\n\n,,========== 테이블폼(유로폼) 타입 ==========\n");
		fprintf (fp, buffer);

		for (xx = 0 ; xx < euroformCellType.nCells ; ++xx) {
			sprintf (buffer, ",,순번,밑면,측면1,측면2,길이\n");
			fprintf (fp, buffer);

			sprintf (buffer, ",,%d,%.0f,%.0f,%.0f,%.0f\n", xx+1, euroformCellType.cells [xx].euroform_bottomWidth * 1000, euroformCellType.cells [xx].euroform_leftHeight * 1000, euroformCellType.cells [xx].euroform_rightHeight * 1000, euroformCellType.cells [xx].length_left * 1000);
			fprintf (fp, buffer);

			for (yy = 0 ; yy < euroformCellType.layerNames [xx].size () ; ++yy) {
				strcpy (tempBuffer, euroformCellType.layerNames [xx].at (yy).c_str ());
				char* token = strtok (tempBuffer, "-");
				string insElem;
				vector<string> layerNameComp;

				while (token != NULL) {
					if (strlen (token) > 0) {
						insElem = token;
						layerNameComp.push_back (insElem);
					}
					token = strtok (NULL, "-");
				}

				if (layerNameComp.size () >= 2) {
					// 마지막 요소와 마지막 직전 요소가 모두 숫자일 경우
					int lastNum = atoi (layerNameComp.at (layerNameComp.size () - 1).c_str ());
					int lastBeforeNum = atoi (layerNameComp.at (layerNameComp.size () - 2).c_str ());

					if ((lastNum != 0) && (lastBeforeNum != 0)) {
						sprintf (buffer, ",,,%s-%s,%s\n", layerNameComp.at (layerNameComp.size () - 2).c_str (), layerNameComp.at (layerNameComp.size () - 1).c_str (), euroformCellType.layerNames [xx].at (yy).c_str ());
					} else if (lastNum != 0) {
						sprintf (buffer, ",,,\'%s,%s\n", layerNameComp.at (layerNameComp.size () - 1).c_str (), euroformCellType.layerNames [xx].at (yy).c_str ());
					} else {
						sprintf (buffer, ",,,%s\n", euroformCellType.layerNames [xx].at (yy).c_str ());
					}
				} else {
					sprintf (buffer, ",,,%s\n", euroformCellType.layerNames [xx].at (yy).c_str ()); 
				}
				fprintf (fp, buffer);
			}
		}
	}

	fclose (fp);

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

	// 그룹화 일시정지 OFF
	suspendGroups (false);

	ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);
	location.ToDisplayText (&resultString);
	sprintf (buffer, "결과물을 다음 위치에 저장했습니다.\n\n%s\n또는 프로젝트 파일이 있는 폴더", resultString.ToCStr ().Get ());
	WriteReport_Alert (buffer);

	return err;
}

// 테이블폼 면적 계산
GSErrCode	calcTableformArea (void)
{
	GSErrCode	err = NoError;
	unsigned short		xx;
	short		mm;
	//bool		regenerate = true;

	// 모든 객체, 보 저장
	GS::Array<API_Guid>		elemList;
	GS::Array<API_Guid>		objects;
	long					nObjects = 0;

	// 선택한 요소들의 정보 요약하기
	API_Element			elem;
	API_ElementMemo		memo;

	// 레이어 관련 변수
	short			nLayers;
	API_Attribute	attrib;
	short			nVisibleLayers = 0;
	short			visLayerList [1024];
	char			fullLayerName [512];
	vector<LayerList>	layerList;

	// 기타
	char			buffer [512];
	char			filename [512];

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
	FILE				*fp_unite;

	bool	bTargetObject;		// 대상이 되는 객체인가?
	double	totalArea;			// 총 면적값


	// 그룹화 일시정지 ON
	suspendGroups (true);

	ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);

	// 프로젝트 내 레이어 개수를 알아냄
	nLayers = getLayerCount ();

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

	// 레이어 이름과 인덱스 저장
	for (xx = 0 ; xx < nVisibleLayers ; ++xx) {
		BNZeroMemory (&attrib, sizeof (API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = visLayerList [xx];
		err = ACAPI_Attribute_Get (&attrib);

		sprintf (fullLayerName, "%s", attrib.layer.head.name);
		fullLayerName [strlen (fullLayerName)] = '\0';

		LayerList newLayerItem;
		newLayerItem.layerInd = visLayerList [xx];
		newLayerItem.layerName = fullLayerName;

		layerList.push_back (newLayerItem);
	}

	// 레이어 이름 기준으로 정렬하여 레이어 인덱스 순서 변경
	sort (layerList.begin (), layerList.end (), compareLayerName);		// 레이어 이름 기준 오름차순 정렬

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
	sprintf (filename, "%s - 레이어별 테이블폼 면적 (통합).csv", miscAppInfo.caption);
	fp_unite = fopen (filename, "w+");

	if (fp_unite == NULL) {
		DGAlert (DG_ERROR, L"오류", L"통합 버전 엑셀파일을 만들 수 없습니다.", "", L"확인", "", "");
		return	NoError;
	}

	// 진행 상황 표시하는 기능 - 초기화
	nPhase = 1;
	cur = 1;
	total = nVisibleLayers;
	ACAPI_Interface (APIIo_InitProcessWindowID, &title, &nPhase);
	ACAPI_Interface (APIIo_SetNextProcessPhaseID, &subtitle, &total);

	sprintf (buffer, "안내: 면적 값의 단위는 m2(제곱미터)입니다.\n고려되는 객체: 유로폼 / 합판 / 아웃코너판넬 / 인코너판넬 / 변각인코너판넬 / 인코너M형판넬 / 목재\n\n");
	fprintf (fp_unite, buffer);

	// 보이는 레이어들을 하나씩 순회하면서 전체 요소들을 선택한 후 테이블폼의 면적 값을 가진 객체들의 변수 값을 가져와서 계산함
	for (mm = 1 ; mm <= nVisibleLayers ; ++mm) {
		BNZeroMemory (&attrib, sizeof (API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		//attrib.layer.head.index = visLayerList [mm-1];
		attrib.layer.head.index = layerList [mm-1].layerInd;
		err = ACAPI_Attribute_Get (&attrib);

		// 초기화
		objects.Clear ();

		if (err == NoError) {
			// 레이어 보이기
			if ((attrib.layer.head.flags & APILay_Hidden) == true) {
				attrib.layer.head.flags ^= APILay_Hidden;
				ACAPI_Attribute_Modify (&attrib, NULL);
			}

			// 모든 요소 가져오기
			ACAPI_Element_GetElemList (API_ObjectID, &elemList, APIFilt_OnVisLayer);	// 보이는 레이어에 있음, 객체 타입만
			while (elemList.GetSize () > 0) {
				objects.Push (elemList.Pop ());
			}
			nObjects = objects.GetSize ();

			if (nObjects == 0)
				continue;

			// 레이어 이름 가져옴
			sprintf (fullLayerName, "%s", attrib.layer.head.name);
			fullLayerName [strlen (fullLayerName)] = '\0';

			// 레이어 이름
			sprintf (buffer, "<< 레이어 : %s >> : ", fullLayerName);
			fprintf (fp_unite, buffer);

			totalArea = 0.0;

			// 객체에서 면적 값 가져와서 합산하기
			for (xx = 0 ; xx < nObjects ; ++xx) {
				BNZeroMemory (&elem, sizeof (API_Element));
				BNZeroMemory (&memo, sizeof (API_ElementMemo));
				elem.header.guid = objects [xx];
				err = ACAPI_Element_Get (&elem);

				if (err == NoError && elem.header.hasMemo) {
					err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

					// 파라미터 스크립트를 강제로 실행시킴
					ACAPI_Goodies (APIAny_RunGDLParScriptID, &elem.header, 0);
					bool	bForce = true;
					ACAPI_Database (APIDb_RefreshElementID, &elem.header, &bForce);

					if (err == NoError) {
						bTargetObject = false;

						if (my_strcmp (getParameterStringByName (&memo, "u_comp"), "유로폼") == 0) {
							bTargetObject = true;
						} else if (my_strcmp (getParameterStringByName (&memo, "g_comp"), "합판") == 0) {
							bTargetObject = true;
						} else if (my_strcmp (getParameterStringByName (&memo, "in_comp"), "아웃코너판넬") == 0) {
							bTargetObject = true;
						} else if (my_strcmp (getParameterStringByName (&memo, "in_comp"), "인코너판넬") == 0) {
							bTargetObject = true;
						} else if (my_strcmp (getParameterStringByName (&memo, "in_comp"), "변각인코너판넬") == 0) {
							bTargetObject = true;
						} else if (my_strcmp (getParameterStringByName (&memo, "in_comp"), "인코너M형판넬") == 0) {
							bTargetObject = true;
						} else if (my_strcmp (getParameterStringByName (&memo, "w_comp"), "목재") == 0) {
							bTargetObject = true;
						}

						if (bTargetObject == true) {
							sprintf (buffer, "%s", getParameterStringByName (&memo, "gs_list_custom04"));
							removeCharInStr (buffer, ',');
							totalArea += atof (buffer);
						}
					}

					ACAPI_DisposeElemMemoHdls (&memo);
				}
			}

			// 면적 값 출력하기
			sprintf (buffer, "%lf\n", totalArea);
			fprintf (fp_unite, buffer);

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

	// 그룹화 일시정지 OFF
	suspendGroups (false);

	ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);
	location.ToDisplayText (&resultString);
	WriteReport_Alert ("결과물을 다음 위치에 저장했습니다.\n\n%s\n또는 프로젝트 파일이 있는 폴더", resultString.ToCStr ().Get ());

	return err;
}

// 콘크리트 물량 계산 (Single 모드)
GSErrCode	calcConcreteVolumeSingleMode (void)
{
	GSErrCode	err = NoError;
	unsigned short		xx;
	//bool		regenerate = true;
	
	GS::Array<API_Guid>		walls;
	GS::Array<API_Guid>		columns;
	GS::Array<API_Guid>		beams;
	GS::Array<API_Guid>		slabs;
	GS::Array<API_Guid>		morphs;
	GS::Array<API_Guid>		objects;

	long					nWalls = 0;
	long					nColumns = 0;
	long					nBeams = 0;
	long					nSlabs = 0;
	long					nMorphs = 0;
	long					nObjects = 0;

	double					volume_walls = 0.0;
	double					volume_columns = 0.0;
	double					volume_beams = 0.0;
	double					volume_slabs = 0.0;
	double					volume_morphs = 0.0;
	double					volume_objects = 0.0;

	double					volume_total = 0.0;

	// 선택한 요소들의 정보 요약하기
	API_ElementQuantity	quantity;
	API_Quantities		quantities;
	API_QuantitiesMask	mask;
	API_QuantityPar		params;
	char				reportStr [512];


	// 그룹화 일시정지 ON
	suspendGroups (true);

	// 선택한 요소 가져오기 ( 벽, 기둥, 보, 슬래브, 모프, 객체)
	getGuidsOfSelection (&walls, API_WallID, &nWalls);
	getGuidsOfSelection (&columns, API_ColumnID, &nColumns);
	getGuidsOfSelection (&beams, API_BeamID, &nBeams);
	getGuidsOfSelection (&slabs, API_SlabID, &nSlabs);
	getGuidsOfSelection (&morphs, API_MorphID, &nMorphs);
	getGuidsOfSelection (&objects, API_ObjectID, &nObjects);

	if ( (nWalls == 0) && (nColumns == 0) && (nBeams == 0) && (nSlabs == 0) && (nMorphs == 0) && (nObjects == 0) ) {
		DGAlert (DG_ERROR, L"오류", L"요소들을 선택해야 합니다.", "", L"확인", "", "");
		return err;
	}

	params.minOpeningSize = EPS;

	// 벽에 대한 물량 정보 추출
	ACAPI_ELEMENT_QUANTITY_MASK_CLEAR (mask);
	ACAPI_ELEMENT_QUANTITY_MASK_SET (mask, wall, volume);
	for (xx = 0 ; xx < nWalls ; ++xx) {
		quantities.elements = &quantity;
		err = ACAPI_Element_GetQuantities (walls [xx], &params, &quantities, &mask);

		if (err == NoError) {
			volume_walls += quantity.wall.volume;
		}
	}

	// 기둥에 대한 물량 정보 추출
	ACAPI_ELEMENT_QUANTITY_MASK_CLEAR (mask);
	ACAPI_ELEMENT_QUANTITY_MASK_SET (mask, column, coreVolume);
	ACAPI_ELEMENT_QUANTITY_MASK_SET (mask, column, veneVolume);
	for (xx = 0 ; xx < nColumns ; ++xx) {
		quantities.elements = &quantity;
		err = ACAPI_Element_GetQuantities (columns [xx], &params, &quantities, &mask);

		if (err == NoError) {
			volume_columns += quantity.column.coreVolume;
			volume_columns += quantity.column.veneVolume;
		}
	}

	// 보에 대한 물량 정보 추출
	ACAPI_ELEMENT_QUANTITY_MASK_CLEAR (mask);
	ACAPI_ELEMENT_QUANTITY_MASK_SET (mask, beam, volume);
	for (xx = 0 ; xx < nBeams ; ++xx) {
		quantities.elements = &quantity;
		err = ACAPI_Element_GetQuantities (beams [xx], &params, &quantities, &mask);

		if (err == NoError) {
			volume_beams += quantity.beam.volume;
		}
	}

	// 슬래브에 대한 물량 정보 추출
	ACAPI_ELEMENT_QUANTITY_MASK_CLEAR (mask);
	ACAPI_ELEMENT_QUANTITY_MASK_SET (mask, slab, volume);
	for (xx = 0 ; xx < nSlabs ; ++xx) {
		quantities.elements = &quantity;
		err = ACAPI_Element_GetQuantities (slabs [xx], &params, &quantities, &mask);

		if (err == NoError) {
			volume_slabs += quantity.slab.volume;
		}
	}

	// 모프에 대한 물량 정보 추출
	ACAPI_ELEMENT_QUANTITY_MASK_CLEAR (mask);
	ACAPI_ELEMENT_QUANTITY_MASK_SET (mask, morph, volume);
	for (xx = 0 ; xx < nMorphs ; ++xx) {
		quantities.elements = &quantity;
		err = ACAPI_Element_GetQuantities (morphs [xx], &params, &quantities, &mask);

		if (err == NoError) {
			volume_morphs += quantity.morph.volume;
		}
	}

	// 객체에 대한 물량 정보 추출
	ACAPI_ELEMENT_QUANTITY_MASK_CLEAR (mask);
	ACAPI_ELEMENT_QUANTITY_MASK_SET (mask, symb, volume);
	for (xx = 0 ; xx < nObjects ; ++xx) {
		quantities.elements = &quantity;
		err = ACAPI_Element_GetQuantities (objects [xx], &params, &quantities, &mask);

		if (err == NoError) {
			volume_objects += quantity.symb.volume;
		}
	}

	// 총 부피 계산
	volume_total = volume_walls + volume_columns + volume_beams + volume_slabs + volume_morphs + volume_objects;

	sprintf (reportStr, "%12s: %lf ㎥\n%12s: %lf ㎥\n%12s: %lf ㎥\n%12s: %lf ㎥\n%12s: %lf ㎥\n%12s: %lf ㎥\n\n%12s: %lf ㎥",
						"벽 부피", volume_walls,
						"기둥 부피", volume_columns,
						"보 부피", volume_beams,
						"슬래브 부피", volume_slabs,
						"모프 부피", volume_morphs,
						"객체 부피", volume_objects,
						"총 부피", volume_total);
	WriteReport_Alert (reportStr);

	// 그룹화 일시정지 OFF
	suspendGroups (false);

	return	err;
}

// 콘크리트 물량 계산 (Multi 모드)
GSErrCode	calcConcreteVolumeMultiMode (void)
{
	GSErrCode	err = NoError;
	unsigned short		xx;
	short		mm;
	//bool		regenerate = true;
	
	// 모든 객체 및 벽, 기둥, 보, 슬래브, 모프 저장
	GS::Array<API_Guid>		elemList;

	GS::Array<API_Guid>		walls;
	GS::Array<API_Guid>		columns;
	GS::Array<API_Guid>		beams;
	GS::Array<API_Guid>		slabs;
	GS::Array<API_Guid>		morphs;
	GS::Array<API_Guid>		objects;

	long					nWalls = 0;
	long					nColumns = 0;
	long					nBeams = 0;
	long					nSlabs = 0;
	long					nMorphs = 0;
	long					nObjects = 0;

	double					volume_walls = 0.0;
	double					volume_columns = 0.0;
	double					volume_beams = 0.0;
	double					volume_slabs = 0.0;
	double					volume_morphs = 0.0;
	double					volume_objects = 0.0;

	double					volume_total = 0.0;

	// 선택한 요소들의 정보 요약하기
	API_ElementQuantity	quantity;
	API_Quantities		quantities;
	API_QuantitiesMask	mask;
	API_QuantityPar		params;
	char				reportStr [512];

	// 레이어 관련 변수
	short			nLayers;
	API_Attribute	attrib;
	short			nVisibleLayers = 0;
	short			visLayerList [1024];
	char			fullLayerName [512];
	vector<LayerList>	layerList;

	// 기타
	char			filename [512];

	// 엑셀 파일로 기둥 정보 내보내기
	// 파일 저장을 위한 변수
	API_SpecFolderID	specFolderID = API_ApplicationFolderID;
	IO::Location		location;
	GS::UniString		resultString;
	API_MiscAppInfo		miscAppInfo;
	FILE				*fp_unite;


	// 그룹화 일시정지 ON
	suspendGroups (true);

	ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);

	// 프로젝트 내 레이어 개수를 알아냄
	nLayers = getLayerCount ();

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

	// 레이어 이름과 인덱스 저장
	for (xx = 0 ; xx < nVisibleLayers ; ++xx) {
		BNZeroMemory (&attrib, sizeof (API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = visLayerList [xx];
		err = ACAPI_Attribute_Get (&attrib);

		sprintf (fullLayerName, "%s", attrib.layer.head.name);
		fullLayerName [strlen (fullLayerName)] = '\0';

		LayerList newLayerItem;
		newLayerItem.layerInd = visLayerList [xx];
		newLayerItem.layerName = fullLayerName;

		layerList.push_back (newLayerItem);
	}

	// 레이어 이름 기준으로 정렬하여 레이어 인덱스 순서 변경
	sort (layerList.begin (), layerList.end (), compareLayerName);		// 레이어 이름 기준 오름차순 정렬

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
	sprintf (filename, "%s - 선택한 구조 정보 (통합).csv", miscAppInfo.caption);
	fp_unite = fopen (filename, "w+");

	if (fp_unite == NULL) {
		DGAlert (DG_ERROR, L"오류", L"통합 버전 엑셀파일을 만들 수 없습니다.", "", L"확인", "", "");
		return	NoError;
	}

	sprintf (reportStr, "단위: ㎥\n\n");
	fprintf (fp_unite, reportStr);

	// 보이는 레이어들을 하나씩 순회하면서 전체 요소들을 선택한 후 "선택한 부재 정보 내보내기" 루틴 실행
	for (mm = 1 ; mm <= nVisibleLayers ; ++mm) {
		BNZeroMemory (&attrib, sizeof (API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		//attrib.layer.head.index = visLayerList [mm-1];
		attrib.layer.head.index = layerList [mm-1].layerInd;
		err = ACAPI_Attribute_Get (&attrib);

		// 초기화
		walls.Clear ();
		columns.Clear ();
		beams.Clear ();
		slabs.Clear ();
		morphs.Clear ();

		if (err == NoError) {
			// 레이어 보이기
			if ((attrib.layer.head.flags & APILay_Hidden) == true) {
				attrib.layer.head.flags ^= APILay_Hidden;
				ACAPI_Attribute_Modify (&attrib, NULL);
			}

			// 모든 요소 가져오기
			ACAPI_Element_GetElemList (API_WallID, &elemList, APIFilt_OnVisLayer);		// 보이는 레이어에 있음, 벽 타입만
			while (elemList.GetSize () > 0) {
				walls.Push (elemList.Pop ());
			}

			ACAPI_Element_GetElemList (API_ColumnID, &elemList, APIFilt_OnVisLayer);	// 보이는 레이어에 있음, 기둥 타입만
			while (elemList.GetSize () > 0) {
				columns.Push (elemList.Pop ());
			}

			ACAPI_Element_GetElemList (API_BeamID, &elemList, APIFilt_OnVisLayer);		// 보이는 레이어에 있음, 보 타입만
			while (elemList.GetSize () > 0) {
				beams.Push (elemList.Pop ());
			}

			ACAPI_Element_GetElemList (API_SlabID, &elemList, APIFilt_OnVisLayer);		// 보이는 레이어에 있음, 슬래브 타입만
			while (elemList.GetSize () > 0) {
				slabs.Push (elemList.Pop ());
			}

			ACAPI_Element_GetElemList (API_MorphID, &elemList, APIFilt_OnVisLayer);		// 보이는 레이어에 있음, 모프 타입만
			while (elemList.GetSize () > 0) {
				morphs.Push (elemList.Pop ());
			}

			ACAPI_Element_GetElemList (API_ObjectID, &elemList, APIFilt_OnVisLayer);	// 보이는 레이어에 있음, 객체 타입만
			while (elemList.GetSize () > 0) {
				objects.Push (elemList.Pop ());
			}

			nWalls = walls.GetSize ();
			nColumns = columns.GetSize ();
			nBeams = beams.GetSize ();
			nSlabs = slabs.GetSize ();
			nMorphs = morphs.GetSize ();
			nObjects = objects.GetSize ();

			params.minOpeningSize = EPS;

			if ((nWalls == 0) && (nColumns == 0) && (nBeams == 0) && (nSlabs == 0) && (nMorphs == 0) && (nObjects == 0))
				continue;

			// 레이어 이름 가져옴
			sprintf (fullLayerName, "%s", attrib.layer.head.name);
			fullLayerName [strlen (fullLayerName)] = '\0';

			// 레이어 이름 (통합 버전에만)
			sprintf (reportStr, "<< 레이어 : %s >>\n\n", fullLayerName);
			fprintf (fp_unite, reportStr);

			volume_walls = 0.0;
			volume_columns = 0.0;
			volume_beams = 0.0;
			volume_slabs = 0.0;
			volume_morphs = 0.0;
			volume_objects = 0.0;
			volume_total = 0.0;

			// 벽에 대한 물량 정보 추출
			ACAPI_ELEMENT_QUANTITY_MASK_CLEAR (mask);
			ACAPI_ELEMENT_QUANTITY_MASK_SET (mask, wall, volume);
			for (xx = 0 ; xx < nWalls ; ++xx) {
				quantities.elements = &quantity;
				err = ACAPI_Element_GetQuantities (walls [xx], &params, &quantities, &mask);

				if (err == NoError) {
					volume_walls += quantity.wall.volume;
				}
			}

			// 기둥에 대한 물량 정보 추출
			ACAPI_ELEMENT_QUANTITY_MASK_CLEAR (mask);
			ACAPI_ELEMENT_QUANTITY_MASK_SET (mask, column, coreVolume);
			ACAPI_ELEMENT_QUANTITY_MASK_SET (mask, column, veneVolume);
			for (xx = 0 ; xx < nColumns ; ++xx) {
				quantities.elements = &quantity;
				err = ACAPI_Element_GetQuantities (columns [xx], &params, &quantities, &mask);

				if (err == NoError) {
					volume_columns += quantity.column.coreVolume;
					volume_columns += quantity.column.veneVolume;
				}
			}

			// 보에 대한 물량 정보 추출
			ACAPI_ELEMENT_QUANTITY_MASK_CLEAR (mask);
			ACAPI_ELEMENT_QUANTITY_MASK_SET (mask, beam, volume);
			for (xx = 0 ; xx < nBeams ; ++xx) {
				quantities.elements = &quantity;
				err = ACAPI_Element_GetQuantities (beams [xx], &params, &quantities, &mask);

				if (err == NoError) {
					volume_beams += quantity.beam.volume;
				}
			}

			// 슬래브에 대한 물량 정보 추출
			ACAPI_ELEMENT_QUANTITY_MASK_CLEAR (mask);
			ACAPI_ELEMENT_QUANTITY_MASK_SET (mask, slab, volume);
			for (xx = 0 ; xx < nSlabs ; ++xx) {
				quantities.elements = &quantity;
				err = ACAPI_Element_GetQuantities (slabs [xx], &params, &quantities, &mask);

				if (err == NoError) {
					volume_slabs += quantity.slab.volume;
				}
			}

			// 모프에 대한 물량 정보 추출
			ACAPI_ELEMENT_QUANTITY_MASK_CLEAR (mask);
			ACAPI_ELEMENT_QUANTITY_MASK_SET (mask, morph, volume);
			for (xx = 0 ; xx < nMorphs ; ++xx) {
				quantities.elements = &quantity;
				err = ACAPI_Element_GetQuantities (morphs [xx], &params, &quantities, &mask);

				if (err == NoError) {
					volume_morphs += quantity.morph.volume;
				}
			}

			// 객체에 대한 물량 정보 추출
			ACAPI_ELEMENT_QUANTITY_MASK_CLEAR (mask);
			ACAPI_ELEMENT_QUANTITY_MASK_SET (mask, symb, volume);
			for (xx = 0 ; xx < nObjects ; ++xx) {
				quantities.elements = &quantity;
				err = ACAPI_Element_GetQuantities (objects [xx], &params, &quantities, &mask);

				if (err == NoError) {
					volume_objects += quantity.symb.volume;
				}
			}

			// 총 부피 계산
			volume_total = volume_walls + volume_columns + volume_beams + volume_slabs + volume_morphs + volume_objects;

			sprintf (reportStr, "%s,%lf\n%s,%lf\n%s,%lf\n%s,%lf\n%s,%lf\n%s,%lf\n\n%s,%lf\n\n\n",
								"벽 부피", volume_walls,
								"기둥 부피", volume_columns,
								"보 부피", volume_beams,
								"슬래브 부피", volume_slabs,
								"모프 부피", volume_morphs,
								"객체 부피", volume_objects,
								"총 부피", volume_total);
			fprintf (fp_unite, reportStr);

			// 레이어 숨기기
			attrib.layer.head.flags |= APILay_Hidden;
			ACAPI_Attribute_Modify (&attrib, NULL);
		}
	}

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

	// 그룹화 일시정지 OFF
	suspendGroups (false);

	ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);
	location.ToDisplayText (&resultString);
	WriteReport_Alert ("결과물을 다음 위치에 저장했습니다.\n\n%s\n또는 프로젝트 파일이 있는 폴더", resultString.ToCStr ().Get ());

	return err;
}

// 슬래브 수량/하부면적 계산 (Single 모드)
GSErrCode	calcSlabQuantityAndAreaSingleMode (void)
{
	GSErrCode err = NoError;

	unsigned short		xx;
	//bool		regenerate = true;
	
	GS::Array<API_Guid>		slabs;

	long					nSlabs = 0;
	double					surface_slabs = 0.0;

	// 선택한 요소들의 정보 요약하기
	API_ElementQuantity	quantity;
	API_Quantities		quantities;
	API_QuantitiesMask	mask;
	API_QuantityPar		params;
	char				reportStr [512];

	// 그룹화 일시정지 ON
	suspendGroups (true);

	// 선택한 요소 가져오기
	err = getGuidsOfSelection (&slabs, API_SlabID, &nSlabs);
	if (err != NoError) {
		DGAlert (DG_ERROR, L"오류", L"요소들을 선택해야 합니다.", "", L"확인", "", "");
		return err;
	}

	params.minOpeningSize = EPS;

	// 슬래브에 대한 물량 정보 추출
	ACAPI_ELEMENT_QUANTITY_MASK_CLEAR (mask);
	ACAPI_ELEMENT_QUANTITY_MASK_SET (mask, slab, bottomSurface);
	for (xx = 0 ; xx < nSlabs ; ++xx) {
		quantities.elements = &quantity;
		err = ACAPI_Element_GetQuantities (slabs [xx], &params, &quantities, &mask);

		if (err == NoError) {
			surface_slabs += quantity.slab.bottomSurface;
		}
	}

	sprintf (reportStr, "슬래브 수량: %d\n슬래브 밑면 넓이(m2): %lf", nSlabs, surface_slabs);
	WriteReport_Alert (reportStr);

	// 그룹화 일시정지 OFF
	suspendGroups (false);

	return	err;
}

// 슬래브 수량/하부면적 계산 (Multi 모드)
GSErrCode	calcSlabQuantityAndAreaMultiMode (void)
{
	GSErrCode err = NoError;
	unsigned short		xx;
	short		mm;
	//bool		regenerate = true;
	
	// 슬래브 저장
	GS::Array<API_Guid>		elemList;
	GS::Array<API_Guid>		slabs;

	long					nSlabs = 0;
	double					surface_slabs = 0.0;

	// 선택한 요소들의 정보 요약하기
	API_ElementQuantity	quantity;
	API_Quantities		quantities;
	API_QuantitiesMask	mask;
	API_QuantityPar		params;
	char				reportStr [512];

	// 레이어 관련 변수
	short			nLayers;
	API_Attribute	attrib;
	short			nVisibleLayers = 0;
	short			visLayerList [1024];
	char			fullLayerName [512];
	vector<LayerList>	layerList;

	// 기타
	char			filename [512];

	// 엑셀 파일로 기둥 정보 내보내기
	// 파일 저장을 위한 변수
	API_SpecFolderID	specFolderID = API_ApplicationFolderID;
	IO::Location		location;
	GS::UniString		resultString;
	API_MiscAppInfo		miscAppInfo;
	FILE				*fp_unite;


	// 그룹화 일시정지 ON
	suspendGroups (true);

	ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);

	// 프로젝트 내 레이어 개수를 알아냄
	nLayers = getLayerCount ();

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

	// 레이어 이름과 인덱스 저장
	for (xx = 0 ; xx < nVisibleLayers ; ++xx) {
		BNZeroMemory (&attrib, sizeof (API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = visLayerList [xx];
		err = ACAPI_Attribute_Get (&attrib);

		sprintf (fullLayerName, "%s", attrib.layer.head.name);
		fullLayerName [strlen (fullLayerName)] = '\0';

		LayerList newLayerItem;
		newLayerItem.layerInd = visLayerList [xx];
		newLayerItem.layerName = fullLayerName;

		layerList.push_back (newLayerItem);
	}

	// 레이어 이름 기준으로 정렬하여 레이어 인덱스 순서 변경
	sort (layerList.begin (), layerList.end (), compareLayerName);		// 레이어 이름 기준 오름차순 정렬

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
	sprintf (filename, "%s - 슬래브 수량 및 면적 정보 (통합).csv", miscAppInfo.caption);
	fp_unite = fopen (filename, "w+");

	if (fp_unite == NULL) {
		DGAlert (DG_ERROR, L"오류", L"통합 버전 엑셀파일을 만들 수 없습니다.", "", L"확인", "", "");
		return	NoError;
	}

	sprintf (reportStr, "단위: ㎡\n\n");
	fprintf (fp_unite, reportStr);

	// 보이는 레이어들을 하나씩 순회하면서 전체 요소들을 선택한 후 "선택한 부재 정보 내보내기" 루틴 실행
	for (mm = 1 ; mm <= nVisibleLayers ; ++mm) {
		BNZeroMemory (&attrib, sizeof (API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		//attrib.layer.head.index = visLayerList [mm-1];
		attrib.layer.head.index = layerList [mm-1].layerInd;
		err = ACAPI_Attribute_Get (&attrib);

		// 초기화
		slabs.Clear ();

		if (err == NoError) {
			// 레이어 보이기
			if ((attrib.layer.head.flags & APILay_Hidden) == true) {
				attrib.layer.head.flags ^= APILay_Hidden;
				ACAPI_Attribute_Modify (&attrib, NULL);
			}

			// 모든 요소 가져오기
			ACAPI_Element_GetElemList (API_SlabID, &elemList, APIFilt_OnVisLayer);		// 보이는 레이어에 있음, 슬래브 타입만
			while (elemList.GetSize () > 0) {
				slabs.Push (elemList.Pop ());
			}

			nSlabs = slabs.GetSize ();

			params.minOpeningSize = EPS;

			if (nSlabs == 0)
				continue;

			// 레이어 이름 가져옴
			sprintf (fullLayerName, "%s", attrib.layer.head.name);
			fullLayerName [strlen (fullLayerName)] = '\0';

			// 레이어 이름 (통합 버전에만)
			sprintf (reportStr, "<< 레이어 : %s >>\n\n", fullLayerName);
			fprintf (fp_unite, reportStr);

			surface_slabs = 0.0;

			// 슬래브에 대한 물량 정보 추출
			ACAPI_ELEMENT_QUANTITY_MASK_CLEAR (mask);
			ACAPI_ELEMENT_QUANTITY_MASK_SET (mask, slab, bottomSurface);
			for (xx = 0 ; xx < nSlabs ; ++xx) {
				quantities.elements = &quantity;
				err = ACAPI_Element_GetQuantities (slabs [xx], &params, &quantities, &mask);

				if (err == NoError) {
					surface_slabs += quantity.slab.bottomSurface;
				}
			}

			sprintf (reportStr, "슬래브 수량,%d\n슬래브 밑면 넓이,%lf\n\n", nSlabs, surface_slabs);
			fprintf (fp_unite, reportStr);

			// 레이어 숨기기
			attrib.layer.head.flags |= APILay_Hidden;
			ACAPI_Attribute_Modify (&attrib, NULL);
		}
	}

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

	ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);
	location.ToDisplayText (&resultString);
	WriteReport_Alert ("결과물을 다음 위치에 저장했습니다.\n\n%s\n또는 프로젝트 파일이 있는 폴더", resultString.ToCStr ().Get ());

	// 그룹화 일시정지 OFF
	suspendGroups (false);

	return	err;
}

// 단열재 수량/면적 계산 (Single 모드)
GSErrCode	calcInsulationQuantityAndAreaSingleMode (void)
{
	GSErrCode err = NoError;

	unsigned short		xx;
	//bool		regenerate = true;

	GS::Array<API_Guid>		objects;

	long					nObjects = 0;
	double					surface_objects = 0.0;
	char					buffer [512];

	// 선택한 요소들의 정보 요약하기
	API_Element			elem;
	API_ElementMemo		memo;

	// 그룹화 일시정지 ON
	suspendGroups (true);

	// 선택한 요소 가져오기
	err = getGuidsOfSelection (&objects, API_ObjectID, &nObjects);
	if (err != NoError) {
		DGAlert (DG_ERROR, L"오류", L"요소들을 선택해야 합니다.", "", L"확인", "", "");
		return err;
	}

	for (xx = 0 ; xx < nObjects ; ++xx) {
		BNZeroMemory (&elem, sizeof (API_Element));
		BNZeroMemory (&memo, sizeof (API_ElementMemo));
		elem.header.guid = objects [xx];
		err = ACAPI_Element_Get (&elem);

		if (err == NoError && elem.header.hasMemo) {
			err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

			// 파라미터 스크립트를 강제로 실행시킴
			ACAPI_Goodies (APIAny_RunGDLParScriptID, &elem.header, 0);
			bool	bForce = true;
			ACAPI_Database (APIDb_RefreshElementID, &elem.header, &bForce);

			if (err == NoError) {
				if (my_strcmp (getParameterStringByName (&memo, "sup_type"), "단열재") == 0) {
					sprintf (buffer, "%s", getParameterStringByName (&memo, "gs_list_custom2"));
					removeCharInStr (buffer, ',');
					surface_objects += atof (buffer);
				}
			}

			ACAPI_DisposeElemMemoHdls (&memo);
		}
	}

	sprintf (buffer, "단열재 수량: %d\n단열재 넓이(m2): %lf", nObjects, surface_objects);
	WriteReport_Alert (buffer);

	// 그룹화 일시정지 OFF
	suspendGroups (false);

	return	err;
}

// 단열재 수량/면적 계산 (Multi 모드)
GSErrCode	calcInsulationQuantityAndAreaMultiMode (void)
{
	GSErrCode	err = NoError;
	unsigned short		xx;
	short		mm;
	//bool		regenerate = true;

	// 모든 객체, 보 저장
	GS::Array<API_Guid>		elemList;
	GS::Array<API_Guid>		objects;
	long					nObjects = 0;

	// 선택한 요소들의 정보 요약하기
	API_Element			elem;
	API_ElementMemo		memo;

	// 레이어 관련 변수
	short			nLayers;
	API_Attribute	attrib;
	short			nVisibleLayers = 0;
	short			visLayerList [1024];
	char			fullLayerName [512];
	vector<LayerList>	layerList;

	// 기타
	char			buffer [512];
	char			filename [512];

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
	FILE				*fp_unite;

	bool	bTargetObject;		// 대상이 되는 객체인가?
	int		nTarget;			// 총 수량
	double	totalArea;			// 총 면적값


	// 그룹화 일시정지 ON
	suspendGroups (true);

	ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);

	// 프로젝트 내 레이어 개수를 알아냄
	nLayers = getLayerCount ();

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

	// 레이어 이름과 인덱스 저장
	for (xx = 0 ; xx < nVisibleLayers ; ++xx) {
		BNZeroMemory (&attrib, sizeof (API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = visLayerList [xx];
		err = ACAPI_Attribute_Get (&attrib);

		sprintf (fullLayerName, "%s", attrib.layer.head.name);
		fullLayerName [strlen (fullLayerName)] = '\0';

		LayerList newLayerItem;
		newLayerItem.layerInd = visLayerList [xx];
		newLayerItem.layerName = fullLayerName;

		layerList.push_back (newLayerItem);
	}

	// 레이어 이름 기준으로 정렬하여 레이어 인덱스 순서 변경
	sort (layerList.begin (), layerList.end (), compareLayerName);		// 레이어 이름 기준 오름차순 정렬

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
	sprintf (filename, "%s - 단열재 수량 및 면적 (통합).csv", miscAppInfo.caption);
	fp_unite = fopen (filename, "w+");

	if (fp_unite == NULL) {
		DGAlert (DG_ERROR, L"오류", L"통합 버전 엑셀파일을 만들 수 없습니다.", "", L"확인", "", "");
		return	NoError;
	}

	// 진행 상황 표시하는 기능 - 초기화
	nPhase = 1;
	cur = 1;
	total = nVisibleLayers;
	ACAPI_Interface (APIIo_InitProcessWindowID, &title, &nPhase);
	ACAPI_Interface (APIIo_SetNextProcessPhaseID, &subtitle, &total);

	sprintf (buffer, "안내: 면적 값의 단위는 m2(제곱미터)입니다.\n고려되는 객체: 단열재\n\n");
	fprintf (fp_unite, buffer);

	// 보이는 레이어들을 하나씩 순회하면서 전체 요소들을 선택한 후 테이블폼의 면적 값을 가진 객체들의 변수 값을 가져와서 계산함
	for (mm = 1 ; mm <= nVisibleLayers ; ++mm) {
		BNZeroMemory (&attrib, sizeof (API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		//attrib.layer.head.index = visLayerList [mm-1];
		attrib.layer.head.index = layerList [mm-1].layerInd;
		err = ACAPI_Attribute_Get (&attrib);

		// 초기화
		objects.Clear ();

		if (err == NoError) {
			// 레이어 보이기
			if ((attrib.layer.head.flags & APILay_Hidden) == true) {
				attrib.layer.head.flags ^= APILay_Hidden;
				ACAPI_Attribute_Modify (&attrib, NULL);
			}

			// 모든 요소 가져오기
			ACAPI_Element_GetElemList (API_ObjectID, &elemList, APIFilt_OnVisLayer);	// 보이는 레이어에 있음, 객체 타입만
			while (elemList.GetSize () > 0) {
				objects.Push (elemList.Pop ());
			}
			nObjects = objects.GetSize ();

			if (nObjects == 0)
				continue;

			// 레이어 이름 가져옴
			sprintf (fullLayerName, "%s", attrib.layer.head.name);
			fullLayerName [strlen (fullLayerName)] = '\0';

			// 레이어 이름
			sprintf (buffer, "<< 레이어 : %s >>\n\n", fullLayerName);
			fprintf (fp_unite, buffer);

			totalArea = 0.0;
			nTarget = 0;

			// 객체에서 면적 값 가져와서 합산하기
			for (xx = 0 ; xx < nObjects ; ++xx) {
				BNZeroMemory (&elem, sizeof (API_Element));
				BNZeroMemory (&memo, sizeof (API_ElementMemo));
				elem.header.guid = objects [xx];
				err = ACAPI_Element_Get (&elem);

				if (err == NoError && elem.header.hasMemo) {
					err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

					// 파라미터 스크립트를 강제로 실행시킴
					ACAPI_Goodies (APIAny_RunGDLParScriptID, &elem.header, 0);
					bool	bForce = true;
					ACAPI_Database (APIDb_RefreshElementID, &elem.header, &bForce);

					if (err == NoError) {
						bTargetObject = false;

						if (my_strcmp (getParameterStringByName (&memo, "sup_type"), "단열재") == 0) {
							bTargetObject = true;
						}

						if (bTargetObject == true) {
							nTarget ++;
							sprintf (buffer, "%s", getParameterStringByName (&memo, "gs_list_custom2"));
							removeCharInStr (buffer, ',');
							totalArea += atof (buffer);
						}
					}

					ACAPI_DisposeElemMemoHdls (&memo);
				}
			}

			// 면적 값 출력하기
			sprintf (buffer, "수량,%d\n면적,%lf\n\n", nTarget, totalArea);
			fprintf (fp_unite, buffer);

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

	ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);
	location.ToDisplayText (&resultString);
	WriteReport_Alert ("결과물을 다음 위치에 저장했습니다.\n\n%s\n또는 프로젝트 파일이 있는 폴더", resultString.ToCStr ().Get ());

	// 그룹화 일시정지 OFF
	suspendGroups (false);

	return err;
}

// 모든 입면도 PDF로 내보내기 (Single 모드)
GSErrCode	exportAllElevationsToPDFSingleMode (void)
{
	GSErrCode	err = NoError;
	bool		regenerate = true;
	char		filename [256];
	bool		bAsked = false;

	// 입면도 DB를 가져오기 위한 변수
	API_DatabaseUnId*	dbases = NULL;
	GSSize				nDbases = 0;
	API_WindowInfo		windowInfo;
	API_DatabaseInfo	currentDB;

	// 파일 내보내기를 위한 변수
	API_FileSavePars	fsp;
	API_SavePars_Pdf	pars_pdf;

	// 입면도 확대를 위한 변수
	API_Box		extent;
	double		scale = 100.0;
	bool		zoom = true;

	API_SpecFolderID	specFolderID = API_ApplicationFolderID;
	IO::Location		location;
	GS::UniString		resultString;

	ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
	ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);
						
	// PDF 파일 페이지 설정
	BNZeroMemory (&pars_pdf, sizeof (API_SavePars_Pdf));
	pars_pdf.leftMargin = 5.0;
	pars_pdf.rightMargin = 5.0;
	pars_pdf.topMargin = 5.0;
	pars_pdf.bottomMargin = 5.0;
	pars_pdf.sizeX = 297.0;
	pars_pdf.sizeY = 210.0;

	// 입면 뷰 DB의 ID들을 획득함
	err = ACAPI_Database (APIDb_GetElevationDatabasesID, &dbases, NULL);
	if (err == NoError)
		nDbases = BMpGetSize (reinterpret_cast<GSPtr>(dbases)) / Sizeof32 (API_DatabaseUnId);

	// 입면 뷰들을 하나씩 순회함
	for (GSIndex i = 0; i < nDbases; i++) {
		API_DatabaseInfo dbPars;
		BNZeroMemory (&dbPars, sizeof (API_DatabaseInfo));
		dbPars.databaseUnId = dbases [i];

		// 창을 변경함
		BNZeroMemory (&windowInfo, sizeof (API_WindowInfo));
		windowInfo.typeID = APIWind_ElevationID;
		windowInfo.databaseUnId = dbPars.databaseUnId;
		ACAPI_Automate (APIDo_ChangeWindowID, &windowInfo, NULL);

		// 현재 데이터베이스를 가져옴
		ACAPI_Database (APIDb_GetCurrentDatabaseID, &currentDB, NULL);

		// 현재 도면의 드로잉 범위를 가져옴
		ACAPI_Database (APIDb_GetExtentID, &extent, NULL);

		// 축척 변경하기
		if (bAsked == false) {
			scale = (abs (extent.xMax - extent.xMin) < abs (extent.yMax - extent.yMin)) ? abs (extent.xMax - extent.xMin) : abs (extent.yMax - extent.yMin);
			scale *= 2;
			DGBlankModalDialog (300, 150, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, scaleQuestionHandler, (DGUserData) &scale);
			bAsked = true;
		}
		ACAPI_Database (APIDb_ChangeDrawingScaleID, &scale, &zoom);

		// 저장하기
		BNZeroMemory (&fsp, sizeof (API_FileSavePars));
		fsp.fileTypeID = APIFType_PdfFile;
		ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);
		sprintf (filename, "%s.pdf", GS::UniString (currentDB.title).ToCStr ().Get ());
		fsp.file = new IO::Location (location, IO::Name (filename));

		err = ACAPI_Automate (APIDo_SaveID, &fsp, &pars_pdf);

		delete	fsp.file;
	}

	if (dbases != NULL)
		BMpFree (reinterpret_cast<GSPtr>(dbases));

	ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);
	location.ToDisplayText (&resultString);
	WriteReport_Alert ("결과물을 다음 위치에 저장했습니다.\n\n%s\n또는 프로젝트 파일이 있는 폴더", resultString.ToCStr ().Get ());

	return err;
}

// 모든 입면도 PDF로 내보내기 (Multi 모드)
GSErrCode	exportAllElevationsToPDFMultiMode (void)
{
	GSErrCode	err = NoError;
	short		xx, mm;
	bool		regenerate = true;
	char		filename [256];
	bool		bAsked = false;

	// 레이어 관련 변수
	short			nLayers;
	API_Attribute	attrib;
	short			nVisibleLayers = 0;
	short			visLayerList [1024];
	char			fullLayerName [512];
	vector<LayerList>	layerList;

	// 진행바를 표현하기 위한 변수
	GS::UniString       title ("내보내기 진행 상황");
	GS::UniString       subtitle ("진행중...");
	short	nPhase;
	Int32	cur, total;

	// 입면도 DB를 가져오기 위한 변수
	API_DatabaseUnId*	dbases = NULL;
	GSSize				nDbases = 0;
	API_WindowInfo		windowInfo;
	API_DatabaseInfo	currentDB;

	// 파일 내보내기를 위한 변수
	API_FileSavePars	fsp;
	API_SavePars_Pdf	pars_pdf;

	// 입면도 확대를 위한 변수
	API_Box		extent;
	double		scale = 100.0;
	bool		zoom = true;

	API_SpecFolderID	specFolderID = API_ApplicationFolderID;
	IO::Location		location;
	GS::UniString		resultString;

	ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
	ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);
						
	// 프로젝트 내 레이어 개수를 알아냄
	nLayers = getLayerCount ();

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

	// 레이어 이름과 인덱스 저장
	for (xx = 0 ; xx < nVisibleLayers ; ++xx) {
		BNZeroMemory (&attrib, sizeof (API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		attrib.layer.head.index = visLayerList [xx];
		err = ACAPI_Attribute_Get (&attrib);

		sprintf (fullLayerName, "%s", attrib.layer.head.name);
		fullLayerName [strlen (fullLayerName)] = '\0';

		LayerList newLayerItem;
		newLayerItem.layerInd = visLayerList [xx];
		newLayerItem.layerName = fullLayerName;

		layerList.push_back (newLayerItem);
	}

	// 레이어 이름 기준으로 정렬하여 레이어 인덱스 순서 변경
	sort (layerList.begin (), layerList.end (), compareLayerName);		// 레이어 이름 기준 오름차순 정렬

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

	// 입면 뷰 DB의 ID들을 획득함
	err = ACAPI_Database (APIDb_GetElevationDatabasesID, &dbases, NULL);
	if (err == NoError)
		nDbases = BMpGetSize (reinterpret_cast<GSPtr>(dbases)) / Sizeof32 (API_DatabaseUnId);

	// PDF 파일 페이지 설정
	BNZeroMemory (&pars_pdf, sizeof (API_SavePars_Pdf));
	pars_pdf.leftMargin = 5.0;
	pars_pdf.rightMargin = 5.0;
	pars_pdf.topMargin = 5.0;
	pars_pdf.bottomMargin = 5.0;
	pars_pdf.sizeX = 297.0;
	pars_pdf.sizeY = 210.0;

	// 진행 상황 표시하는 기능 - 초기화
	nPhase = 1;
	cur = 1;
	total = nVisibleLayers;
	ACAPI_Interface (APIIo_InitProcessWindowID, &title, &nPhase);
	ACAPI_Interface (APIIo_SetNextProcessPhaseID, &subtitle, &total);

	// 보이는 레이어들을 하나씩 순회하면서 전체 요소들을 선택한 후 테이블폼의 면적 값을 가진 객체들의 변수 값을 가져와서 계산함
	for (mm = 1 ; mm <= nVisibleLayers ; ++mm) {
		BNZeroMemory (&attrib, sizeof (API_Attribute));
		attrib.layer.head.typeID = API_LayerID;
		//attrib.layer.head.index = visLayerList [mm-1];
		attrib.layer.head.index = layerList [mm-1].layerInd;
		err = ACAPI_Attribute_Get (&attrib);

		if (err == NoError) {
			// 레이어 보이기
			if ((attrib.layer.head.flags & APILay_Hidden) == true) {
				attrib.layer.head.flags ^= APILay_Hidden;
				ACAPI_Attribute_Modify (&attrib, NULL);
			}

			// 레이어 이름 가져옴
			sprintf (fullLayerName, "%s", attrib.layer.head.name);
			fullLayerName [strlen (fullLayerName)] = '\0';

			// 입면 뷰들을 하나씩 순회함
			for (GSIndex i = 0; i < nDbases; i++) {
				API_DatabaseInfo dbPars;
				BNZeroMemory (&dbPars, sizeof (API_DatabaseInfo));
				dbPars.databaseUnId = dbases [i];

				// 창을 변경함
				BNZeroMemory (&windowInfo, sizeof (API_WindowInfo));
				windowInfo.typeID = APIWind_ElevationID;
				windowInfo.databaseUnId = dbPars.databaseUnId;
				ACAPI_Automate (APIDo_ChangeWindowID, &windowInfo, NULL);

				// 현재 데이터베이스를 가져옴
				ACAPI_Database (APIDb_GetCurrentDatabaseID, &currentDB, NULL);

				// 현재 도면의 드로잉 범위를 가져옴
				ACAPI_Database (APIDb_GetExtentID, &extent, NULL);

				// 축척 변경하기
				if (bAsked == false) {
					scale = (abs (extent.xMax - extent.xMin) < abs (extent.yMax - extent.yMin)) ? abs (extent.xMax - extent.xMin) : abs (extent.yMax - extent.yMin);
					scale *= 2;
					DGBlankModalDialog (300, 150, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, scaleQuestionHandler, (DGUserData) &scale);
					bAsked = true;
				}
				ACAPI_Database (APIDb_ChangeDrawingScaleID, &scale, &zoom);

				// 저장하기
				BNZeroMemory (&fsp, sizeof (API_FileSavePars));
				fsp.fileTypeID = APIFType_PdfFile;
				ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);
				sprintf (filename, "%s - %s.pdf", fullLayerName, GS::UniString (currentDB.title).ToCStr ().Get ());
				fsp.file = new IO::Location (location, IO::Name (filename));

				err = ACAPI_Automate (APIDo_SaveID, &fsp, &pars_pdf);

				delete	fsp.file;
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

	if (dbases != NULL)
		BMpFree (reinterpret_cast<GSPtr>(dbases));

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

	ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);
	location.ToDisplayText (&resultString);
	WriteReport_Alert ("결과물을 다음 위치에 저장했습니다.\n\n%s\n또는 프로젝트 파일이 있는 폴더", resultString.ToCStr ().Get ());

	return err;
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
			DGSetDialogTitle (dialogID, L"선택한 타입의 객체 선택 후 보여주기");

			// 확인 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 20, 10, 80, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, L"확인");
			DGShowItem (dialogID, DG_OK);

			// 취소 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 120, 10, 80, 25);
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, L"취소");
			DGShowItem (dialogID, DG_CANCEL);

			// 버튼: 전체선택
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 20, 50, 80, 25);
			DGSetItemFont (dialogID, BUTTON_ALL_SEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, BUTTON_ALL_SEL, L"전체선택");
			DGShowItem (dialogID, BUTTON_ALL_SEL);

			// 버튼: 전체선택 해제
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 120, 50, 80, 25);
			DGSetItemFont (dialogID, BUTTON_ALL_UNSEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, BUTTON_ALL_UNSEL, L"전체선택\n해제");
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
				DGSetItemText (dialogID, itmIdx, L"벽");
				DGShowItem (dialogID, itmIdx);
				itmPosY += 30;
			}
			if (visibleObjInfo.bExist_Columns == true) {
				visibleObjInfo.itmIdx_Columns = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, itmPosX, itmPosY, 190, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_BOLD);
				DGSetItemText (dialogID, itmIdx, L"기둥");
				DGShowItem (dialogID, itmIdx);
				itmPosY += 30;
			}
			if (visibleObjInfo.bExist_Beams == true) {
				visibleObjInfo.itmIdx_Beams = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, itmPosX, itmPosY, 190, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_BOLD);
				DGSetItemText (dialogID, itmIdx, L"보");
				DGShowItem (dialogID, itmIdx);
				itmPosY += 30;
			}
			if (visibleObjInfo.bExist_Slabs == true) {
				visibleObjInfo.itmIdx_Slabs = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, itmPosX, itmPosY, 190, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_BOLD);
				DGSetItemText (dialogID, itmIdx, L"슬래브");
				DGShowItem (dialogID, itmIdx);
				itmPosY += 30;
			}
			if (visibleObjInfo.bExist_Roofs == true) {
				visibleObjInfo.itmIdx_Roofs = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, itmPosX, itmPosY, 190, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_BOLD);
				DGSetItemText (dialogID, itmIdx, L"루프");
				DGShowItem (dialogID, itmIdx);
				itmPosY += 30;
			}
			if (visibleObjInfo.bExist_Meshes == true) {
				visibleObjInfo.itmIdx_Meshes = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, itmPosX, itmPosY, 190, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_BOLD);
				DGSetItemText (dialogID, itmIdx, L"메시");
				DGShowItem (dialogID, itmIdx);
				itmPosY += 30;
			}
			if (visibleObjInfo.bExist_Morphs == true) {
				visibleObjInfo.itmIdx_Morphs = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, itmPosX, itmPosY, 190, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_BOLD);
				DGSetItemText (dialogID, itmIdx, L"모프");
				DGShowItem (dialogID, itmIdx);
				itmPosY += 30;
			}
			if (visibleObjInfo.bExist_Shells == true) {
				visibleObjInfo.itmIdx_Shells = itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, itmPosX, itmPosY, 190, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_BOLD);
				DGSetItemText (dialogID, itmIdx, L"셸");
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

// [다이얼로그] 사용자가 축척 값을 직접 입력할 수 있도록 함
short DGCALLBACK scaleQuestionHandler (short message, short dialogID, short item, DGUserData userData, DGMessageData /* msgData */)
{
	short	result;
	short	idxItem;
	double	*scale = (double*) userData;

	switch (message) {
		case DG_MSG_INIT:
			// 다이얼로그 타이틀
			DGSetDialogTitle (dialogID, L"입면도 축척값 입력하기");

			// 적용 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 70, 110, 70, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, L"예");
			DGShowItem (dialogID, DG_OK);

			// 종료 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 160, 110, 70, 25);
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, L"아니오");
			DGShowItem (dialogID, DG_CANCEL);

			// 라벨: 안내문
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 30, 15, 200, 25);
			DGSetItemFont (dialogID, idxItem, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, idxItem, L"입면도의 축척을 변경하시겠습니까?\n값이 작아질수록 도면이 확대됩니다.");
			DGShowItem (dialogID, idxItem);

			// 라벨: 축척
			idxItem = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 130, 60, 30, 23);
			DGSetItemFont (dialogID, idxItem, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, idxItem, L"1 : ");
			DGShowItem (dialogID, idxItem);

			// Edit 컨트롤: 축척
			EDITCONTROL_SCALE_VALUE = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_LENGTH, 0, 161, 54, 60, 25);
			DGSetItemFont (dialogID, EDITCONTROL_SCALE_VALUE, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemValDouble (dialogID, EDITCONTROL_SCALE_VALUE, *scale);
			DGShowItem (dialogID, EDITCONTROL_SCALE_VALUE);

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					*scale = DGGetItemValDouble (dialogID, EDITCONTROL_SCALE_VALUE);
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