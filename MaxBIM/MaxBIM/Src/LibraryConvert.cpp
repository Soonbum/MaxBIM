#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "LibraryConvert.hpp"

//const GS::uchar_t*	gsmUFOM = L("유로폼v2.0.gsm");
//const GS::uchar_t*	gsmSPIP = L("비계파이프v1.0.gsm");
//const GS::uchar_t*	gsmPINB = L("핀볼트세트v1.0.gsm");
//const GS::uchar_t*	gsmTIE = L("벽체 타이 v1.0.gsm");
//const GS::uchar_t*	gsmCLAM = L("직교클램프v1.0.gsm");
//const GS::uchar_t*	gsmPUSH = L("RS Push-Pull Props 헤드피스 v2.0 (인양고리 포함).gsm");
//const GS::uchar_t*	gsmJOIN = L("결합철물 (사각와셔활용) v1.0.gsm");
//const GS::uchar_t*	gsmPLYW = L("합판v1.0.gsm");
//const GS::uchar_t*	gsmTIMB = L("목재v1.0.gsm");

//const GS::uchar_t*	gsmSFOM = L("슬래브 테이블폼 (콘판넬) v1.0.gsm");
//const GS::uchar_t*	gsmFISP = L("휠러스페이서v1.0.gsm");
//const GS::uchar_t*	gsmOUTA = L("아웃코너앵글v1.0.gsm");
//const GS::uchar_t*	gsmOUTP = L("아웃코너판넬v1.0.gsm");
//const GS::uchar_t*	gsmINCO = L("인코너판넬v1.0.gsm");

// 모든 가상 가설재(TCO: Temporary Construction Object)를 실제 가설재로 변환함
GSErrCode	convertVirtualTCO (void)
{
	GSErrCode	err = NoError;

	API_Element				elem;
	API_ElementMemo			memo;

	short	xx;
	long	nSel;
	char	buffer [256];

	const char*		productName;	// 가상 가설재
	const char*		objType;		// 테이블폼(벽), 테이블폼(슬래브), 유로폼, 스틸폼, 합판, 휠러스페이서, 아웃코너앵글, 아웃코너판넬, 인코너판넬
	const char*		dir;			// 벽세우기, 바닥깔기, 바닥덮기
	const char*		coverSide;		// 양면, 단면
	double			oppSideOffset;	// 반대면까지의 거리
	bool			leftSide;		// 왼쪽(1), 오른쪽(0)
	bool			bRegularSize;	// 정규 크기(1)
	double unit_A, unit_B, unit_ZZYZX;	// 단위 길이
	int num_A, num_B, num_ZZYZX;		// 단위 개수


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

	for (xx = 0 ; xx < nObjects ; ++xx) {
		BNZeroMemory (&elem, sizeof (API_Element));
		BNZeroMemory (&memo, sizeof (API_ElementMemo));
		elem.header.guid = objects.Pop ();
		err = ACAPI_Element_Get (&elem);
		err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

		productName = getParameterStringByName (&memo, "productName");

		if (strncmp (productName, "가상 가설재", strlen ("가상 가설재")) == 0) {

			// 가상 가설재의 파라미터 값 불러오기
			objType = getParameterStringByName (&memo, "objType");
			dir = getParameterStringByName (&memo, "dir");						// 벽세우기, 바닥깔기, 바닥덮기
			coverSide = getParameterStringByName (&memo, "coverSide");			// 양면, 단면
			oppSideOffset = getParameterValueByName (&memo, "oppSideOffset");	// 반대면까지의 거리
			leftSide = getParameterValueByName (&memo, "leftSide");				// 왼쪽(1), 오른쪽(0)
			bRegularSize = getParameterValueByName (&memo, "bRegularSize");		// 정규 크기(1)
			unit_A = getParameterValueByName (&memo, "unit_A");					// 단위 길이 (A)
			unit_B = getParameterValueByName (&memo, "unit_B");					// 단위 길이 (B)
			unit_ZZYZX = getParameterValueByName (&memo, "unit_ZZYZX");			// 단위 길이 (ZZYZX)
			num_A = getParameterValueByName (&memo, "num_A");					// 단위 개수 (A)
			num_B = getParameterValueByName (&memo, "num_B");					// 단위 개수 (B)
			num_ZZYZX = getParameterValueByName (&memo, "num_ZZYZX");			// 단위 개수 (ZZYZX)
			
			// 가상 가설재 제거
			API_Elem_Head* headList = new API_Elem_Head [1];
			headList [0] = elem.header;
			err = ACAPI_Element_Delete (&headList, 1);
			delete headList;

			//GSErrCode	placeTableformOnWall (WallTableform params);		// 테이블폼(벽) 배치
			//GSErrCode	placeTableformOnSlabBottom (SlabTableform params);	// 테이블폼(슬래브) 배치
			//GSErrCode	placeEuroform (Euroform params);					// 유로폼/스틸폼 배치
			//GSErrCode	placePlywood (Plywood params);						// 합판 배치
			//GSErrCode	placeFillersp (FillerSpacer params);				// 휠러스페이서 배치
			//GSErrCode	placeOutcornerAngle (OutcornerAngle params);		// 아웃코너앵글 배치
			//GSErrCode	placeOutcornerPanel (OutcornerPanel params);		// 아웃코너판넬 배치
			//GSErrCode	placeIncornerPanel (IncornerPanel params);			// 인코너판넬 배치

			// ... 아래 조건문에서 객체 구조체 내용을 fill한 후 호출

			// 실제 가설재를 배치함
			if (strncmp (objType, "테이블폼(벽)", strlen ("테이블폼(벽)")) == 0) {
				// ...
			} else if (strncmp (objType, "테이블폼(슬래브)", strlen ("테이블폼(슬래브)")) == 0) {
				// ...
			} else if (strncmp (objType, "유로폼", strlen ("유로폼")) == 0) {
				// ...
			} else if (strncmp (objType, "스틸폼", strlen ("스틸폼")) == 0) {
				// ...
			} else if (strncmp (objType, "합판", strlen ("합판")) == 0) {
				// ...
			} else if (strncmp (objType, "휠러스페이서", strlen ("휠러스페이서")) == 0) {
				// ...
			} else if (strncmp (objType, "아웃코너앵글", strlen ("아웃코너앵글")) == 0) {
				// ...
			} else if (strncmp (objType, "아웃코너판넬", strlen ("아웃코너판넬")) == 0) {
				// ...
			} else if (strncmp (objType, "인코너판넬", strlen ("인코너판넬")) == 0) {
				// ...
			}
		}

		ACAPI_DisposeElemMemoHdls (&memo);
	}

	// 화면 새로고침
	ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
	bool	regenerate = true;
	ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

	return	err;
}

// 테이블폼(벽) 배치
GSErrCode	placeTableformOnWall (WallTableform params)
{
	GSErrCode	err = NoError;

	return	err;
}

// 테이블폼(슬래브) 배치
GSErrCode	placeTableformOnSlabBottom (SlabTableform params)
{
	GSErrCode	err = NoError;

	return	err;
}

// 유로폼/스틸폼 배치
GSErrCode	placeEuroform (Euroform params)
{
	GSErrCode	err = NoError;

	return	err;
}

// 합판 배치
GSErrCode	placePlywood (Plywood params)
{
	GSErrCode	err = NoError;

	return	err;
}

// 휠러스페이서 배치
GSErrCode	placeFillersp (FillerSpacer params)
{
	GSErrCode	err = NoError;

	return	err;
}

// 아웃코너앵글 배치
GSErrCode	placeOutcornerAngle (OutcornerAngle params)
{
	GSErrCode	err = NoError;

	return	err;
}

// 아웃코너판넬 배치
GSErrCode	placeOutcornerPanel (OutcornerPanel params)
{
	GSErrCode	err = NoError;

	return	err;
}

// 인코너판넬 배치
GSErrCode	placeIncornerPanel (IncornerPanel params)
{
	GSErrCode	err = NoError;

	return	err;
}

