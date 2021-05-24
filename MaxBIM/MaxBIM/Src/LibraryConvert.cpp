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

using namespace libraryConvertDG;

short	floorInd;		// 가상 가설재의 층 인덱스 저장

// 해당 레이어를 사용하는지 여부
static bool		bLayerInd_Euroform;			// 레이어 번호: 유로폼
static bool		bLayerInd_RectPipe;			// 레이어 번호: 비계 파이프
static bool		bLayerInd_PinBolt;			// 레이어 번호: 핀볼트 세트
static bool		bLayerInd_WallTie;			// 레이어 번호: 빅체 타이
static bool		bLayerInd_Clamp;			// 레이어 번호: 직교 클램프
static bool		bLayerInd_HeadPiece;		// 레이어 번호: 헤드피스
static bool		bLayerInd_Join;				// 레이어 번호: 결합철물
static bool		bLayerInd_Plywood;			// 레이어 번호: 합판
static bool		bLayerInd_Wood;				// 레이어 번호: 목재

static bool		bLayerInd_SlabTableform;	// 레이어 번호: 슬래브 테이블폼
//static bool		bLayerInd_Plywood;			// 레이어 번호: 합판
//static bool		bLayerInd_Wood;				// 레이어 번호: 목재
static bool		bLayerInd_Profile;			// 레이어 번호: KS프로파일
static bool		bLayerInd_Fittings;			// 레이어 번호: 결합철물

static bool		bLayerInd_Steelform;		// 레이어 번호: 스틸폼
//static bool		bLayerInd_Plywood;			// 레이어 번호: 합판
static bool		bLayerInd_Fillersp;			// 레이어 번호: 휠러스페이서
static bool		bLayerInd_OutcornerAngle;	// 레이어 번호: 아웃코너앵글
static bool		bLayerInd_OutcornerPanel;	// 레이어 번호: 아웃코너앵글
static bool		bLayerInd_IncornerPanel;	// 레이어 번호: 인코너앵글

// 해당 레이어의 번호
static short	layerInd_Euroform;			// 레이어 번호: 유로폼
static short	layerInd_RectPipe;			// 레이어 번호: 비계 파이프
static short	layerInd_PinBolt;			// 레이어 번호: 핀볼트 세트
static short	layerInd_WallTie;			// 레이어 번호: 빅체 타이
static short	layerInd_Clamp;				// 레이어 번호: 직교 클램프
static short	layerInd_HeadPiece;			// 레이어 번호: 헤드피스
static short	layerInd_Join;				// 레이어 번호: 결합철물
static short	layerInd_Plywood;			// 레이어 번호: 합판
static short	layerInd_Wood;				// 레이어 번호: 목재

static short	layerInd_SlabTableform;		// 레이어 번호: 슬래브 테이블폼
//static short	layerInd_Plywood;			// 레이어 번호: 합판
//static short	layerInd_Wood;				// 레이어 번호: 목재
static short	layerInd_Profile;			// 레이어 번호: KS프로파일
static short	layerInd_Fittings;			// 레이어 번호: 결합철물

static short	layerInd_Steelform;			// 레이어 번호: 스틸폼
//static short	layerInd_Plywood;			// 레이어 번호: 합판
static short	layerInd_Fillersp;			// 레이어 번호: 휠러스페이서
static short	layerInd_OutcornerAngle;	// 레이어 번호: 아웃코너앵글
static short	layerInd_OutcornerPanel;	// 레이어 번호: 아웃코너앵글
static short	layerInd_IncornerPanel;		// 레이어 번호: 인코너앵글

static GS::Array<API_Guid>	elemList;		// 그룹화를 위해 생성된 결과물들의 GUID를 전부 저장함


// 모든 가상 가설재(TCO: Temporary Construction Object)를 실제 가설재로 변환함
GSErrCode	convertVirtualTCO (void)
{
	GSErrCode	err = NoError;

	API_Element				elem;
	API_ElementMemo			memo;
	API_Elem_Head*			headList;

	short	xx, yy;
	short	result;
	long	nSel;

	const char*		tempStr;
	char			productName [16];	// 가상 가설재
	char			objType [32];		// 테이블폼(벽), 테이블폼(슬래브), 유로폼, 스틸폼, 합판, 휠러스페이서, 아웃코너앵글, 아웃코너판넬, 인코너판넬
	char			dir [16];			// 벽세우기, 바닥깔기, 바닥덮기
	char			coverSide [8];		// 양면, 단면
	double			oppSideOffset;		// 반대면까지의 거리
	bool			leftSide;			// 왼쪽(1), 오른쪽(0)
	bool			bRegularSize;		// 정규 크기(1)
	double unit_A, unit_B, unit_ZZYZX;	// 단위 길이
	int num_A, num_B, num_ZZYZX;		// 단위 개수


	// 선택한 요소가 없으면 오류
	API_SelectionInfo		selectionInfo;
	API_Neig				**selNeigs;
	API_Element				tElem;
	GS::Array<API_Guid>&	objects = GS::Array<API_Guid> ();
	GS::Array<API_Guid>&	objectsRetry = GS::Array<API_Guid> ();


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

			if (tElem.header.typeID == API_ObjectID) {	// 객체 타입 요소인가?
				objects.Push (tElem.header.guid);
				objectsRetry.Push (tElem.header.guid);
			}
		}
	}
	BMKillHandle ((GSHandle *) &selNeigs);

	// 요소의 유무 확인
	bLayerInd_Euroform = false;
	bLayerInd_RectPipe = false;
	bLayerInd_PinBolt = false;
	bLayerInd_WallTie = false;
	bLayerInd_Clamp = false;
	bLayerInd_HeadPiece = false;
	bLayerInd_Join = false;
	bLayerInd_Plywood = false;
	bLayerInd_Wood = false;

	bLayerInd_SlabTableform = false;
	bLayerInd_Profile = false;
	bLayerInd_Fittings = false;

	bLayerInd_Steelform = false;
	bLayerInd_Fillersp = false;
	bLayerInd_OutcornerAngle = false;
	bLayerInd_OutcornerPanel = false;
	bLayerInd_IncornerPanel = false;

	// 미리 가상 가설재를 확인하여 설정해야 할 레이어를 선정할 것
	while (objects.GetSize () > 0) {
		BNZeroMemory (&elem, sizeof (API_Element));
		BNZeroMemory (&memo, sizeof (API_ElementMemo));
		elem.header.guid = objects.Pop ();
		err = ACAPI_Element_Get (&elem);
		err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

		tempStr = getParameterStringByName (&memo, "productName");
		if (tempStr == NULL) {
			ACAPI_DisposeElemMemoHdls (&memo);
			continue;
		}
		strncpy (productName, tempStr, strlen (tempStr));
		productName [strlen (tempStr)] = '\0';

		if (strncmp (productName, "가상 가설재", strlen ("가상 가설재")) == 0) {
			
			// 가상 가설재의 파라미터 값 불러오기
			tempStr = getParameterStringByName (&memo, "objType");
			strncpy (objType, tempStr, strlen (tempStr));
			objType [strlen (tempStr)] = '\0';

			if (strncmp (objType, "테이블폼(벽)", strlen ("테이블폼(벽)")) == 0) {
				bLayerInd_Euroform = true;
				bLayerInd_RectPipe = true;
				bLayerInd_PinBolt = true;
				bLayerInd_WallTie = true;
				bLayerInd_Clamp = true;
				bLayerInd_HeadPiece = true;
				bLayerInd_Join = true;
				bLayerInd_Plywood = true;
				bLayerInd_Wood = true;

			} else if (strncmp (objType, "테이블폼(슬래브)", strlen ("테이블폼(슬래브)")) == 0) {
				bLayerInd_SlabTableform = true;
				bLayerInd_Plywood = true;
				bLayerInd_Wood = true;
				bLayerInd_Profile = true;
				bLayerInd_Fittings = true;

			} else if (strncmp (objType, "유로폼", strlen ("유로폼")) == 0) {
				bLayerInd_Euroform = true;

			} else if (strncmp (objType, "스틸폼", strlen ("스틸폼")) == 0) {
				bLayerInd_Steelform = true;

			} else if (strncmp (objType, "합판", strlen ("합판")) == 0) {
				bLayerInd_Plywood = true;

			} else if (strncmp (objType, "휠러스페이서", strlen ("휠러스페이서")) == 0) {
				bLayerInd_Fillersp = true;

			} else if (strncmp (objType, "아웃코너앵글", strlen ("아웃코너앵글")) == 0) {
				bLayerInd_OutcornerAngle = true;

			} else if (strncmp (objType, "아웃코너판넬", strlen ("아웃코너판넬")) == 0) {
				bLayerInd_OutcornerPanel = true;

			} else if (strncmp (objType, "인코너판넬", strlen ("인코너판넬")) == 0) {
				bLayerInd_IncornerPanel = true;
			}
		}

		ACAPI_DisposeElemMemoHdls (&memo);
	}
	
	// [DIALOG] 레이어 설정
	result = DGModalDialog (ACAPI_GetOwnResModule (), 32519, ACAPI_GetOwnResModule (), convertVirtualTCOHandler1, 0);

	// 가상 가설재만 추려내서 배치함
	while (objectsRetry.GetSize () > 0) {
		BNZeroMemory (&elem, sizeof (API_Element));
		BNZeroMemory (&memo, sizeof (API_ElementMemo));
		elem.header.guid = objectsRetry.Pop ();
		err = ACAPI_Element_Get (&elem);
		err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

		tempStr = getParameterStringByName (&memo, "productName");
		if (tempStr == NULL) {
			ACAPI_DisposeElemMemoHdls (&memo);
			continue;
		}
		strncpy (productName, tempStr, strlen (tempStr));
		productName [strlen (tempStr)] = '\0';

		if (strncmp (productName, "가상 가설재", strlen ("가상 가설재")) == 0) {

			// 가상 가설재의 층 인덱스 가져옴
			floorInd = elem.header.floorInd;

			tempStr = getParameterStringByName (&memo, "objType");						// 객체 유형
			strncpy (objType, tempStr, strlen (tempStr));
			objType [strlen (tempStr)] = '\0';
			tempStr = getParameterStringByName (&memo, "dir");							// 벽세우기, 벽눕히기, 바닥깔기, 바닥덮기
			strncpy (dir, tempStr, strlen (tempStr));
			dir [strlen (tempStr)] = '\0';
			tempStr = getParameterStringByName (&memo, "coverSide");					// 양면, 단면
			strncpy (coverSide, tempStr, strlen (tempStr));
			coverSide [strlen (tempStr)] = '\0';
			oppSideOffset = getParameterValueByName (&memo, "oppSideOffset");			// 반대면까지의 거리
			leftSide = getParameterValueByName (&memo, "leftSide");						// 왼쪽(1), 오른쪽(0)
			bRegularSize = getParameterValueByName (&memo, "bRegularSize");				// 정규 크기(1)
			unit_A = getParameterValueByName (&memo, "unit_A");							// 단위 길이 (A)
			unit_B = getParameterValueByName (&memo, "unit_B");							// 단위 길이 (B)
			unit_ZZYZX = getParameterValueByName (&memo, "unit_ZZYZX");					// 단위 길이 (ZZYZX)
			num_A = (int)round (getParameterValueByName (&memo, "num_A"), 0);			// 단위 개수 (A)
			num_B = (int)round (getParameterValueByName (&memo, "num_B"), 0);			// 단위 개수 (B)
			num_ZZYZX = (int)round (getParameterValueByName (&memo, "num_ZZYZX"), 0);	// 단위 개수 (ZZYZX)

			// 가상 가설재 제거
			headList = new API_Elem_Head [1];
			headList [0] = elem.header;
			err = ACAPI_Element_Delete (&headList, 1);
			delete headList;

			// 가상 가설재의 정보를 토대로 실제 가설재의 데이터를 구성함
			WallTableform	walltableform;
			SlabTableform	slabtableform;
			Euroform		euroform;
			Plywood			plywood;
			FillerSpacer	fillersp;
			OutcornerAngle	outcornerAngle;
			OutcornerPanel	outcornerPanel;
			IncornerPanel	incornerPanel;

			// 실제 가설재를 배치함
			if (strncmp (objType, "테이블폼(벽)", strlen ("테이블폼(벽)")) == 0) {
				// ...
				//GSErrCode	placeTableformOnWall (WallTableform params);
			} else if (strncmp (objType, "테이블폼(슬래브)", strlen ("테이블폼(슬래브)")) == 0) {

				//const char* typeStr = getParameterStringByName (&memo, "type");
				//strncpy (slabtableform.type, typeStr, strlen (typeStr));
				//slabtableform.type [strlen (typeStr)] = '\0';
				//ACAPI_WriteReport (slabtableform.type, true);

				//if (strncmp (dir, "바닥깔기", strlen ("바닥깔기")) == 0) {

				//	placeTableformOnSlabBottom (slabtableform);

				//} else if (strncmp (dir, "바닥덮기", strlen ("바닥덮기")) == 0) {

				//	placeTableformOnSlabBottom (slabtableform);
				//}

			} else if (strncmp (objType, "유로폼", strlen ("유로폼")) == 0) {

				euroform.ang = elem.object.angle;
				euroform.eu_stan_onoff = bRegularSize;
				if (bRegularSize == true) {
					euroform.eu_wid = unit_A;
					euroform.eu_hei = unit_ZZYZX;
				} else {
					euroform.eu_wid2 = unit_A;
					euroform.eu_hei2 = unit_ZZYZX;
				}

				if (strncmp (dir, "벽세우기", strlen ("벽세우기")) == 0) {
					euroform.leftBottomX = elem.object.pos.x;
					euroform.leftBottomY = elem.object.pos.y;
					euroform.leftBottomZ = elem.object.level;
					euroform.u_ins_wall = true;					// true: 벽세우기, false: 벽눕히기
					euroform.ang_x = DegreeToRad (90.0);		// 벽(90)

					for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
						for (yy = 0 ; yy < num_A ; ++yy) {
							placeEuroform (euroform);
							moveIn3D ('x', elem.object.angle, unit_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						}
						moveIn3D ('x', elem.object.angle, -unit_A * num_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						moveIn3D ('z', elem.object.angle, unit_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
					}

					// 양면
					if (strncmp (coverSide, "양면", strlen ("양면")) == 0) {
						euroform.leftBottomX = elem.object.pos.x;
						euroform.leftBottomY = elem.object.pos.y;
						euroform.leftBottomZ = elem.object.level;
						euroform.ang = elem.object.angle + DegreeToRad (180.0);
						moveIn3D ('x', elem.object.angle, unit_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						moveIn3D ('y', elem.object.angle, oppSideOffset, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						
						for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
							for (yy = 0 ; yy < num_A ; ++yy) {
								placeEuroform (euroform);
								moveIn3D ('x', elem.object.angle, unit_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
							}
							moveIn3D ('x', elem.object.angle, -unit_A * num_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
							moveIn3D ('z', elem.object.angle, unit_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						}
					}
				} else if (strncmp (dir, "벽눕히기", strlen ("벽눕히기")) == 0) {
					euroform.leftBottomX = elem.object.pos.x;
					euroform.leftBottomY = elem.object.pos.y;
					euroform.leftBottomZ = elem.object.level;
					moveIn3D ('x', elem.object.angle, unit_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
					euroform.u_ins_wall = false;				// true: 벽세우기, false: 벽눕히기
					euroform.ang_x = DegreeToRad (90.0);		// 벽(90)

					for (xx = 0 ; xx < num_A ; ++xx) {
						for (yy = 0 ; yy < num_ZZYZX ; ++yy) {
							placeEuroform (euroform);
							moveIn3D ('x', elem.object.angle, unit_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						}
						moveIn3D ('x', elem.object.angle, -unit_ZZYZX * num_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						moveIn3D ('z', elem.object.angle, unit_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
					}

					// 양면
					if (strncmp (coverSide, "양면", strlen ("양면")) == 0) {
						euroform.leftBottomX = elem.object.pos.x;
						euroform.leftBottomY = elem.object.pos.y;
						euroform.leftBottomZ = elem.object.level;
						euroform.ang = elem.object.angle + DegreeToRad (180.0);
						moveIn3D ('y', elem.object.angle, oppSideOffset, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						
						for (xx = 0 ; xx < num_A ; ++xx) {
							for (yy = 0 ; yy < num_ZZYZX ; ++yy) {
								placeEuroform (euroform);
								moveIn3D ('x', elem.object.angle, unit_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
							}
							moveIn3D ('x', elem.object.angle, -unit_ZZYZX * num_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
							moveIn3D ('z', elem.object.angle, unit_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						}
					}
				} else if (strncmp (dir, "바닥깔기", strlen ("바닥깔기")) == 0) {
					euroform.leftBottomX = elem.object.pos.x;
					euroform.leftBottomY = elem.object.pos.y;
					euroform.leftBottomZ = elem.object.level;
					moveIn3D ('y', elem.object.angle, unit_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
					euroform.u_ins_wall = true;					// true: 벽세우기, false: 벽눕히기
					euroform.ang_x = DegreeToRad (0.0);			// 천장(0)
					
					for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
						for (yy = 0 ; yy < num_A ; ++yy) {
							placeEuroform (euroform);
							moveIn3D ('x', elem.object.angle, unit_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						}
						moveIn3D ('x', elem.object.angle, -unit_A * num_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						moveIn3D ('y', elem.object.angle, unit_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
					}
				} else if (strncmp (dir, "바닥덮기", strlen ("바닥덮기")) == 0) {
					euroform.leftBottomX = elem.object.pos.x;
					euroform.leftBottomY = elem.object.pos.y;
					euroform.leftBottomZ = elem.object.level;
					euroform.u_ins_wall = true;					// true: 벽세우기, false: 벽눕히기
					euroform.ang_x = DegreeToRad (180.0);		// 바닥(180)

					for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
						for (yy = 0 ; yy < num_A ; ++yy) {
							placeEuroform (euroform);
							moveIn3D ('x', elem.object.angle, unit_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						}
						moveIn3D ('x', elem.object.angle, -unit_A * num_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						moveIn3D ('y', elem.object.angle, unit_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
					}
				}
			} else if (strncmp (objType, "스틸폼", strlen ("스틸폼")) == 0) {

				euroform.ang = elem.object.angle;
				euroform.eu_stan_onoff = bRegularSize;
				if (bRegularSize == true) {
					euroform.eu_wid = unit_A;
					euroform.eu_hei = unit_ZZYZX;
				} else {
					euroform.eu_wid2 = unit_A;
					euroform.eu_hei2 = unit_ZZYZX;
				}

				if (strncmp (dir, "벽세우기", strlen ("벽세우기")) == 0) {
					euroform.leftBottomX = elem.object.pos.x;
					euroform.leftBottomY = elem.object.pos.y;
					euroform.leftBottomZ = elem.object.level;
					euroform.u_ins_wall = true;					// true: 벽세우기, false: 벽눕히기
					euroform.ang_x = DegreeToRad (90.0);		// 벽(90)

					for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
						for (yy = 0 ; yy < num_A ; ++yy) {
							placeSteelform (euroform);
							moveIn3D ('x', elem.object.angle, unit_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						}
						moveIn3D ('x', elem.object.angle, -unit_A * num_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						moveIn3D ('z', elem.object.angle, unit_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
					}

					// 양면
					if (strncmp (coverSide, "양면", strlen ("양면")) == 0) {
						euroform.leftBottomX = elem.object.pos.x;
						euroform.leftBottomY = elem.object.pos.y;
						euroform.leftBottomZ = elem.object.level;
						euroform.ang = elem.object.angle + DegreeToRad (180.0);
						moveIn3D ('x', elem.object.angle, unit_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						moveIn3D ('y', elem.object.angle, oppSideOffset, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						
						for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
							for (yy = 0 ; yy < num_A ; ++yy) {
								placeSteelform (euroform);
								moveIn3D ('x', elem.object.angle, unit_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
							}
							moveIn3D ('x', elem.object.angle, -unit_A * num_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
							moveIn3D ('z', elem.object.angle, unit_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						}
					}
				} else if (strncmp (dir, "벽눕히기", strlen ("벽눕히기")) == 0) {
					euroform.leftBottomX = elem.object.pos.x;
					euroform.leftBottomY = elem.object.pos.y;
					euroform.leftBottomZ = elem.object.level;
					moveIn3D ('x', elem.object.angle, unit_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
					euroform.u_ins_wall = false;				// true: 벽세우기, false: 벽눕히기
					euroform.ang_x = DegreeToRad (90.0);		// 벽(90)

					for (xx = 0 ; xx < num_A ; ++xx) {
						for (yy = 0 ; yy < num_ZZYZX ; ++yy) {
							placeSteelform (euroform);
							moveIn3D ('x', elem.object.angle, unit_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						}
						moveIn3D ('x', elem.object.angle, -unit_ZZYZX * num_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						moveIn3D ('z', elem.object.angle, unit_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
					}

					// 양면
					if (strncmp (coverSide, "양면", strlen ("양면")) == 0) {
						euroform.leftBottomX = elem.object.pos.x;
						euroform.leftBottomY = elem.object.pos.y;
						euroform.leftBottomZ = elem.object.level;
						euroform.ang = elem.object.angle + DegreeToRad (180.0);
						moveIn3D ('y', elem.object.angle, oppSideOffset, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						
						for (xx = 0 ; xx < num_A ; ++xx) {
							for (yy = 0 ; yy < num_ZZYZX ; ++yy) {
								placeSteelform (euroform);
								moveIn3D ('x', elem.object.angle, unit_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
							}
							moveIn3D ('x', elem.object.angle, -unit_ZZYZX * num_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
							moveIn3D ('z', elem.object.angle, unit_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						}
					}
				} else if (strncmp (dir, "바닥깔기", strlen ("바닥깔기")) == 0) {
					euroform.leftBottomX = elem.object.pos.x;
					euroform.leftBottomY = elem.object.pos.y;
					euroform.leftBottomZ = elem.object.level;
					moveIn3D ('y', elem.object.angle, unit_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
					euroform.u_ins_wall = true;					// true: 벽세우기, false: 벽눕히기
					euroform.ang_x = DegreeToRad (0.0);			// 천장(0)
					
					for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
						for (yy = 0 ; yy < num_A ; ++yy) {
							placeSteelform (euroform);
							moveIn3D ('x', elem.object.angle, unit_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						}
						moveIn3D ('x', elem.object.angle, -unit_A * num_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						moveIn3D ('y', elem.object.angle, unit_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
					}
				} else if (strncmp (dir, "바닥덮기", strlen ("바닥덮기")) == 0) {
					euroform.leftBottomX = elem.object.pos.x;
					euroform.leftBottomY = elem.object.pos.y;
					euroform.leftBottomZ = elem.object.level;
					euroform.u_ins_wall = true;					// true: 벽세우기, false: 벽눕히기
					euroform.ang_x = DegreeToRad (180.0);		// 바닥(180)

					for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
						for (yy = 0 ; yy < num_A ; ++yy) {
							placeSteelform (euroform);
							moveIn3D ('x', elem.object.angle, unit_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						}
						moveIn3D ('x', elem.object.angle, -unit_A * num_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						moveIn3D ('y', elem.object.angle, unit_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
					}
				}
			} else if (strncmp (objType, "합판", strlen ("합판")) == 0) {

				plywood.ang = elem.object.angle;
				plywood.leftBottomX = elem.object.pos.x;
				plywood.leftBottomY = elem.object.pos.y;
				plywood.leftBottomZ = elem.object.level;
				plywood.p_wid = unit_A;
				plywood.p_leng = unit_ZZYZX;

				if (strncmp (dir, "벽세우기", strlen ("벽세우기")) == 0) {
					plywood.w_dir = 1;

					for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
						for (yy = 0 ; yy < num_A ; ++yy) {
							placePlywood (plywood);
							moveIn3D ('x', elem.object.angle, unit_A, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
						}
						moveIn3D ('x', elem.object.angle, -unit_A * num_A, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
						moveIn3D ('z', elem.object.angle, unit_ZZYZX, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
					}

					// 양면
					if (strncmp (coverSide, "양면", strlen ("양면")) == 0) {
						plywood.leftBottomX = elem.object.pos.x;
						plywood.leftBottomY = elem.object.pos.y;
						plywood.leftBottomZ = elem.object.level;
						plywood.ang = elem.object.angle + DegreeToRad (180.0);
						moveIn3D ('x', elem.object.angle, unit_A, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
						moveIn3D ('y', elem.object.angle, oppSideOffset, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);

						for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
							for (yy = 0 ; yy < num_A ; ++yy) {
								placePlywood (plywood);
								moveIn3D ('x', elem.object.angle, unit_A, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
							}
							moveIn3D ('x', elem.object.angle, -unit_A * num_A, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
							moveIn3D ('z', elem.object.angle, unit_ZZYZX, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
						}
					}
				} else if (strncmp (dir, "벽눕히기", strlen ("벽눕히기")) == 0) {
					plywood.w_dir = 2;

					for (xx = 0 ; xx < num_A ; ++xx) {
						for (yy = 0 ; yy < num_ZZYZX ; ++yy) {
							placePlywood (plywood);
							moveIn3D ('x', elem.object.angle, unit_ZZYZX, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
						}
						moveIn3D ('x', elem.object.angle, -unit_ZZYZX * num_ZZYZX, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
						moveIn3D ('z', elem.object.angle, unit_A, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
					}

					// 양면
					if (strncmp (coverSide, "양면", strlen ("양면")) == 0) {
						plywood.leftBottomX = elem.object.pos.x;
						plywood.leftBottomY = elem.object.pos.y;
						plywood.leftBottomZ = elem.object.level;
						plywood.ang = elem.object.angle + DegreeToRad (180.0);
						moveIn3D ('x', elem.object.angle, unit_ZZYZX, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
						moveIn3D ('y', elem.object.angle, oppSideOffset, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);

						for (xx = 0 ; xx < num_A ; ++xx) {
							for (yy = 0 ; yy < num_ZZYZX ; ++yy) {
								placePlywood (plywood);
								moveIn3D ('x', elem.object.angle, unit_ZZYZX, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
							}
							moveIn3D ('x', elem.object.angle, -unit_ZZYZX * num_ZZYZX, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
							moveIn3D ('z', elem.object.angle, unit_A, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
						}
					}
				} else if (strncmp (dir, "바닥깔기", strlen ("바닥깔기")) == 0) {
					plywood.w_dir = 3;

					moveIn3D ('x', elem.object.angle, unit_A, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
					plywood.ang = elem.object.angle + DegreeToRad (90.0);

					for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
						for (yy = 0 ; yy < num_A ; ++yy) {
							placePlywood (plywood);
							moveIn3D ('x', elem.object.angle, unit_A, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
						}
						moveIn3D ('x', elem.object.angle, -unit_A * num_A, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
						moveIn3D ('y', elem.object.angle, unit_ZZYZX, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
					}
				} else if (strncmp (dir, "바닥덮기", strlen ("바닥덮기")) == 0) {
					plywood.w_dir = 4;

					moveIn3D ('x', elem.object.angle, unit_A, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
					plywood.ang = elem.object.angle + DegreeToRad (90.0);

					for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
						for (yy = 0 ; yy < num_A ; ++yy) {
							placePlywood (plywood);
							moveIn3D ('x', elem.object.angle, unit_A, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
						}
						moveIn3D ('x', elem.object.angle, -unit_A * num_A, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
						moveIn3D ('y', elem.object.angle, unit_ZZYZX, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
					}
				}
			} else if (strncmp (objType, "휠러스페이서", strlen ("휠러스페이서")) == 0) {

				fillersp.ang = elem.object.angle;
				fillersp.leftBottomX = elem.object.pos.x;
				fillersp.leftBottomY = elem.object.pos.y;
				fillersp.leftBottomZ = elem.object.level;
				fillersp.f_thk = unit_A;
				fillersp.f_leng = unit_ZZYZX;

				if (strncmp (dir, "벽세우기", strlen ("벽세우기")) == 0) {
					fillersp.f_ang = DegreeToRad (90.0);
					fillersp.f_rota = DegreeToRad (0.0);
					moveIn3D ('x', elem.object.angle, unit_A, &fillersp.leftBottomX, &fillersp.leftBottomY, &fillersp.leftBottomZ);

					for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
						for (yy = 0 ; yy < num_A ; ++yy) {
							placeFillersp (fillersp);
							moveIn3D ('x', elem.object.angle, unit_A, &fillersp.leftBottomX, &fillersp.leftBottomY, &fillersp.leftBottomZ);
						}
						moveIn3D ('x', elem.object.angle, -unit_A * num_A, &fillersp.leftBottomX, &fillersp.leftBottomY, &fillersp.leftBottomZ);
						moveIn3D ('z', elem.object.angle, unit_ZZYZX, &fillersp.leftBottomX, &fillersp.leftBottomY, &fillersp.leftBottomZ);
					}

					// 양면
					if (strncmp (coverSide, "양면", strlen ("양면")) == 0) {
						fillersp.leftBottomX = elem.object.pos.x;
						fillersp.leftBottomY = elem.object.pos.y;
						fillersp.leftBottomZ = elem.object.level;
						fillersp.ang = elem.object.angle + DegreeToRad (180.0);
						moveIn3D ('y', elem.object.angle, oppSideOffset, &fillersp.leftBottomX, &fillersp.leftBottomY, &fillersp.leftBottomZ);

						for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
							for (yy = 0 ; yy < num_A ; ++yy) {
								placeFillersp (fillersp);
								moveIn3D ('x', elem.object.angle, unit_A, &fillersp.leftBottomX, &fillersp.leftBottomY, &fillersp.leftBottomZ);
							}
							moveIn3D ('x', elem.object.angle, -unit_A * num_A, &fillersp.leftBottomX, &fillersp.leftBottomY, &fillersp.leftBottomZ);
							moveIn3D ('z', elem.object.angle, unit_ZZYZX, &fillersp.leftBottomX, &fillersp.leftBottomY, &fillersp.leftBottomZ);
						}
					}
				} else if (strncmp (dir, "벽눕히기", strlen ("벽눕히기")) == 0) {
					fillersp.f_ang = DegreeToRad (0.0);
					fillersp.f_rota = DegreeToRad (0.0);

					for (xx = 0 ; xx < num_A ; ++xx) {
						for (yy = 0 ; yy < num_ZZYZX ; ++yy) {
							placeFillersp (fillersp);
							moveIn3D ('x', elem.object.angle, unit_ZZYZX, &fillersp.leftBottomX, &fillersp.leftBottomY, &fillersp.leftBottomZ);
						}
						moveIn3D ('x', elem.object.angle, -unit_ZZYZX * num_ZZYZX, &fillersp.leftBottomX, &fillersp.leftBottomY, &fillersp.leftBottomZ);
						moveIn3D ('z', elem.object.angle, unit_A, &fillersp.leftBottomX, &fillersp.leftBottomY, &fillersp.leftBottomZ);
					}

					// 양면
					if (strncmp (coverSide, "양면", strlen ("양면")) == 0) {
						fillersp.leftBottomX = elem.object.pos.x;
						fillersp.leftBottomY = elem.object.pos.y;
						fillersp.leftBottomZ = elem.object.level;
						fillersp.ang = elem.object.angle + DegreeToRad (180.0);
						moveIn3D ('x', elem.object.angle, unit_ZZYZX, &fillersp.leftBottomX, &fillersp.leftBottomY, &fillersp.leftBottomZ);
						moveIn3D ('y', elem.object.angle, oppSideOffset, &fillersp.leftBottomX, &fillersp.leftBottomY, &fillersp.leftBottomZ);

						for (xx = 0 ; xx < num_A ; ++xx) {
							for (yy = 0 ; yy < num_ZZYZX ; ++yy) {
								placeFillersp (fillersp);
								moveIn3D ('x', elem.object.angle, unit_ZZYZX, &fillersp.leftBottomX, &fillersp.leftBottomY, &fillersp.leftBottomZ);
							}
							moveIn3D ('x', elem.object.angle, -unit_ZZYZX * num_ZZYZX, &fillersp.leftBottomX, &fillersp.leftBottomY, &fillersp.leftBottomZ);
							moveIn3D ('z', elem.object.angle, unit_A, &fillersp.leftBottomX, &fillersp.leftBottomY, &fillersp.leftBottomZ);
						}
					}
				}
			} else if (strncmp (objType, "아웃코너앵글", strlen ("아웃코너앵글")) == 0) {

				outcornerAngle.ang = elem.object.angle;
				outcornerAngle.leftBottomX = elem.object.pos.x;
				outcornerAngle.leftBottomY = elem.object.pos.y;
				outcornerAngle.leftBottomZ = elem.object.level;
				outcornerAngle.a_leng = unit_ZZYZX;
				
				if (leftSide == true) {
					if (strncmp (dir, "벽세우기", strlen ("벽세우기")) == 0) {
						moveIn3D ('x', outcornerAngle.ang, unit_A, &outcornerAngle.leftBottomX, &outcornerAngle.leftBottomY, &outcornerAngle.leftBottomZ);
						outcornerAngle.a_ang = DegreeToRad (90.0);

						for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
							outcornerAngle.ang = elem.object.angle + DegreeToRad (180.0);
							placeOutcornerAngle (outcornerAngle);
							outcornerAngle.ang = elem.object.angle;
							moveIn3D ('z', elem.object.angle, unit_ZZYZX, &outcornerAngle.leftBottomX, &outcornerAngle.leftBottomY, &outcornerAngle.leftBottomZ);
						}

						// 양면
						if (strncmp (coverSide, "양면", strlen ("양면")) == 0) {
							outcornerAngle.leftBottomX = elem.object.pos.x;
							outcornerAngle.leftBottomY = elem.object.pos.y;
							outcornerAngle.leftBottomZ = elem.object.level;
							outcornerAngle.a_ang = DegreeToRad (90.0);
							moveIn3D ('x', outcornerAngle.ang, unit_A, &outcornerAngle.leftBottomX, &outcornerAngle.leftBottomY, &outcornerAngle.leftBottomZ);
							moveIn3D ('y', elem.object.angle, oppSideOffset, &outcornerAngle.leftBottomX, &outcornerAngle.leftBottomY, &outcornerAngle.leftBottomZ);

							for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
								outcornerAngle.ang = elem.object.angle + DegreeToRad (90.0);
								placeOutcornerAngle (outcornerAngle);
								outcornerAngle.ang = elem.object.angle;
								moveIn3D ('z', elem.object.angle, unit_ZZYZX, &outcornerAngle.leftBottomX, &outcornerAngle.leftBottomY, &outcornerAngle.leftBottomZ);
							}
						}
					} else if (strncmp (dir, "바닥깔기", strlen ("바닥깔기")) == 0) {
						moveIn3D ('x', outcornerAngle.ang, unit_A, &outcornerAngle.leftBottomX, &outcornerAngle.leftBottomY, &outcornerAngle.leftBottomZ);
						outcornerAngle.a_ang = DegreeToRad (0.0);

						for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
							outcornerAngle.ang = elem.object.angle + DegreeToRad (90.0);
							placeOutcornerAngle (outcornerAngle);
							outcornerAngle.ang = elem.object.angle;
							moveIn3D ('y', elem.object.angle, unit_ZZYZX, &outcornerAngle.leftBottomX, &outcornerAngle.leftBottomY, &outcornerAngle.leftBottomZ);
						}
					} else if (strncmp (dir, "바닥덮기", strlen ("바닥덮기")) == 0) {
						moveIn3D ('x', elem.object.angle, unit_A, &outcornerAngle.leftBottomX, &outcornerAngle.leftBottomY, &outcornerAngle.leftBottomZ);
						moveIn3D ('y', elem.object.angle, unit_ZZYZX, &outcornerAngle.leftBottomX, &outcornerAngle.leftBottomY, &outcornerAngle.leftBottomZ);
						outcornerAngle.a_ang = DegreeToRad (180.0);

						for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
							outcornerAngle.ang = elem.object.angle + DegreeToRad (90.0);
							placeOutcornerAngle (outcornerAngle);
							outcornerAngle.ang = elem.object.angle;
							moveIn3D ('y', elem.object.angle, unit_ZZYZX, &outcornerAngle.leftBottomX, &outcornerAngle.leftBottomY, &outcornerAngle.leftBottomZ);
						}
					}
				} else {
					if (strncmp (dir, "벽세우기", strlen ("벽세우기")) == 0) {
						outcornerAngle.a_ang = DegreeToRad (90.0);

						for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
							outcornerAngle.ang = elem.object.angle + DegreeToRad (270.0);
							placeOutcornerAngle (outcornerAngle);
							outcornerAngle.ang = elem.object.angle;
							moveIn3D ('z', elem.object.angle, unit_ZZYZX, &outcornerAngle.leftBottomX, &outcornerAngle.leftBottomY, &outcornerAngle.leftBottomZ);
						}

						// 양면
						if (strncmp (coverSide, "양면", strlen ("양면")) == 0) {
							outcornerAngle.leftBottomX = elem.object.pos.x;
							outcornerAngle.leftBottomY = elem.object.pos.y;
							outcornerAngle.leftBottomZ = elem.object.level;
							outcornerAngle.a_ang = DegreeToRad (90.0);
							moveIn3D ('y', elem.object.angle, oppSideOffset, &outcornerAngle.leftBottomX, &outcornerAngle.leftBottomY, &outcornerAngle.leftBottomZ);

							for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
								outcornerAngle.ang = elem.object.angle;
								placeOutcornerAngle (outcornerAngle);
								moveIn3D ('z', elem.object.angle, unit_ZZYZX, &outcornerAngle.leftBottomX, &outcornerAngle.leftBottomY, &outcornerAngle.leftBottomZ);
							}
						}
					} else if (strncmp (dir, "바닥깔기", strlen ("바닥깔기")) == 0) {
						moveIn3D ('y', outcornerAngle.ang, unit_ZZYZX, &outcornerAngle.leftBottomX, &outcornerAngle.leftBottomY, &outcornerAngle.leftBottomZ);
						outcornerAngle.a_ang = DegreeToRad (0.0);

						for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
							outcornerAngle.ang = elem.object.angle + DegreeToRad (270.0);
							placeOutcornerAngle (outcornerAngle);
							outcornerAngle.ang = elem.object.angle;
							moveIn3D ('y', elem.object.angle, unit_ZZYZX, &outcornerAngle.leftBottomX, &outcornerAngle.leftBottomY, &outcornerAngle.leftBottomZ);
						}
					} else if (strncmp (dir, "바닥덮기", strlen ("바닥덮기")) == 0) {
						outcornerAngle.a_ang = DegreeToRad (180.0);

						for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
							outcornerAngle.ang = elem.object.angle + DegreeToRad (270.0);
							placeOutcornerAngle (outcornerAngle);
							outcornerAngle.ang = elem.object.angle;
							moveIn3D ('y', elem.object.angle, unit_ZZYZX, &outcornerAngle.leftBottomX, &outcornerAngle.leftBottomY, &outcornerAngle.leftBottomZ);
						}
					}
				}
			} else if (strncmp (objType, "아웃코너판넬", strlen ("아웃코너판넬")) == 0) {

				outcornerPanel.ang = elem.object.angle;
				outcornerPanel.leftBottomX = elem.object.pos.x;
				outcornerPanel.leftBottomY = elem.object.pos.y;
				outcornerPanel.leftBottomZ = elem.object.level;

				if (leftSide == true) {
					if (strncmp (dir, "벽세우기", strlen ("벽세우기")) == 0) {
						outcornerPanel.wid_s = unit_A;
						outcornerPanel.leng_s = unit_B;
						outcornerPanel.hei_s = unit_ZZYZX;

						for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
							placeOutcornerPanel (outcornerPanel);
							moveIn3D ('z', elem.object.angle, unit_ZZYZX, &outcornerPanel.leftBottomX, &outcornerPanel.leftBottomY, &outcornerPanel.leftBottomZ);
						}

						// 양면
						if (strncmp (coverSide, "양면", strlen ("양면")) == 0) {
							outcornerPanel.leftBottomX = elem.object.pos.x;
							outcornerPanel.leftBottomY = elem.object.pos.y;
							outcornerPanel.leftBottomZ = elem.object.level;

							outcornerPanel.wid_s = unit_B;
							outcornerPanel.leng_s = unit_A;
							outcornerPanel.hei_s = unit_ZZYZX;

							moveIn3D ('y', elem.object.angle, oppSideOffset, &outcornerPanel.leftBottomX, &outcornerPanel.leftBottomY, &outcornerPanel.leftBottomZ);

							for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
								outcornerPanel.ang = elem.object.angle - DegreeToRad (90.0);
								placeOutcornerPanel (outcornerPanel);
								outcornerPanel.ang = elem.object.angle;
								moveIn3D ('z', elem.object.angle, unit_ZZYZX, &outcornerPanel.leftBottomX, &outcornerPanel.leftBottomY, &outcornerPanel.leftBottomZ);
							}
						}
					}
				} else {
					if (strncmp (dir, "벽세우기", strlen ("벽세우기")) == 0) {
						outcornerPanel.wid_s = unit_B;
						outcornerPanel.leng_s = unit_A;
						outcornerPanel.hei_s = unit_ZZYZX;

						for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
							outcornerPanel.ang = elem.object.angle + DegreeToRad (90.0);
							placeOutcornerPanel (outcornerPanel);
							outcornerPanel.ang = elem.object.angle;
							moveIn3D ('z', elem.object.angle, unit_ZZYZX, &outcornerPanel.leftBottomX, &outcornerPanel.leftBottomY, &outcornerPanel.leftBottomZ);
						}

						// 양면
						if (strncmp (coverSide, "양면", strlen ("양면")) == 0) {
							outcornerPanel.leftBottomX = elem.object.pos.x;
							outcornerPanel.leftBottomY = elem.object.pos.y;
							outcornerPanel.leftBottomZ = elem.object.level;

							outcornerPanel.wid_s = unit_A;
							outcornerPanel.leng_s = unit_B;
							outcornerPanel.hei_s = unit_ZZYZX;

							moveIn3D ('y', elem.object.angle, oppSideOffset, &outcornerPanel.leftBottomX, &outcornerPanel.leftBottomY, &outcornerPanel.leftBottomZ);

							for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
								outcornerPanel.ang = elem.object.angle + DegreeToRad (180.0);
								placeOutcornerPanel (outcornerPanel);
								outcornerPanel.ang = elem.object.angle;
								moveIn3D ('z', elem.object.angle, unit_ZZYZX, &outcornerPanel.leftBottomX, &outcornerPanel.leftBottomY, &outcornerPanel.leftBottomZ);
							}
						}
					}
				}
			} else if (strncmp (objType, "인코너판넬", strlen ("인코너판넬")) == 0) {

				incornerPanel.ang = elem.object.angle;
				incornerPanel.leftBottomX = elem.object.pos.x;
				incornerPanel.leftBottomY = elem.object.pos.y;
				incornerPanel.leftBottomZ = elem.object.level;

				if (leftSide == true) {
					if (strncmp (dir, "벽세우기", strlen ("벽세우기")) == 0) {
						incornerPanel.wid_s = unit_B;
						incornerPanel.leng_s = unit_A;
						incornerPanel.hei_s = unit_ZZYZX;

						for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
							incornerPanel.ang = elem.object.angle + DegreeToRad (270.0);
							placeIncornerPanel (incornerPanel);
							incornerPanel.ang = elem.object.angle;
							moveIn3D ('z', elem.object.angle, unit_ZZYZX, &incornerPanel.leftBottomX, &incornerPanel.leftBottomY, &incornerPanel.leftBottomZ);
						}

						// 양면
						if (strncmp (coverSide, "양면", strlen ("양면")) == 0) {
							incornerPanel.leftBottomX = elem.object.pos.x;
							incornerPanel.leftBottomY = elem.object.pos.y;
							incornerPanel.leftBottomZ = elem.object.level;

							incornerPanel.wid_s = unit_A;
							incornerPanel.leng_s = unit_B;
							incornerPanel.hei_s = unit_ZZYZX;

							moveIn3D ('y', elem.object.angle, oppSideOffset, &incornerPanel.leftBottomX, &incornerPanel.leftBottomY, &incornerPanel.leftBottomZ);

							for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
								incornerPanel.ang = elem.object.angle;
								placeIncornerPanel (incornerPanel);
								moveIn3D ('z', elem.object.angle, unit_ZZYZX, &incornerPanel.leftBottomX, &incornerPanel.leftBottomY, &incornerPanel.leftBottomZ);
							}
						}
					}
				} else {
					if (strncmp (dir, "벽세우기", strlen ("벽세우기")) == 0) {
						incornerPanel.wid_s = unit_A;
						incornerPanel.leng_s = unit_B;
						incornerPanel.hei_s = unit_ZZYZX;

						for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
							incornerPanel.ang = elem.object.angle + DegreeToRad (180.0);
							placeIncornerPanel (incornerPanel);
							incornerPanel.ang = elem.object.angle;
							moveIn3D ('z', elem.object.angle, unit_ZZYZX, &incornerPanel.leftBottomX, &incornerPanel.leftBottomY, &incornerPanel.leftBottomZ);
						}

						// 양면
						if (strncmp (coverSide, "양면", strlen ("양면")) == 0) {
							incornerPanel.leftBottomX = elem.object.pos.x;
							incornerPanel.leftBottomY = elem.object.pos.y;
							incornerPanel.leftBottomZ = elem.object.level;

							incornerPanel.wid_s = unit_B;
							incornerPanel.leng_s = unit_A;
							incornerPanel.hei_s = unit_ZZYZX;

							moveIn3D ('y', elem.object.angle, oppSideOffset, &incornerPanel.leftBottomX, &incornerPanel.leftBottomY, &incornerPanel.leftBottomZ);

							for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
								incornerPanel.ang = elem.object.angle + DegreeToRad (90.0);
								placeIncornerPanel (incornerPanel);
								incornerPanel.ang = elem.object.angle;
								moveIn3D ('z', elem.object.angle, unit_ZZYZX, &incornerPanel.leftBottomX, &incornerPanel.leftBottomY, &incornerPanel.leftBottomZ);
							}
						}
					}
				}
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

	API_Element			element;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	double	aParam;
	double	bParam;
	Int32	addParNum;

	// GUID 변수 초기화
	element.header.guid.clock_seq_hi_and_reserved = 0;
	element.header.guid.clock_seq_low = 0;
	element.header.guid.node[0] = 0;
	element.header.guid.node[1] = 0;
	element.header.guid.node[2] = 0;
	element.header.guid.node[3] = 0;
	element.header.guid.node[4] = 0;
	element.header.guid.node[5] = 0;
	element.header.guid.time_hi_and_version = 0;
	element.header.guid.time_low = 0;
	element.header.guid.time_mid = 0;

	// 객체 로드
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, L("슬래브 테이블폼 (콘판넬) v1.0.gsm"));
	err = ACAPI_LibPart_Search (&libPart, false);
	if (err != NoError)
		return err;
	if (libPart.location != NULL)
		delete libPart.location;

	ACAPI_LibPart_Get (&libPart);

	BNZeroMemory (&element, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));

	element.header.typeID = API_ObjectID;
	element.header.guid = GSGuid2APIGuid (GS::Guid (libPart.ownUnID));

	ACAPI_Element_GetDefaults (&element, &memo);
	ACAPI_LibPart_GetParams (libPart.index, &aParam, &bParam, &addParNum, &memo.params);

	// 라이브러리의 파라미터 값 입력
	element.object.libInd = libPart.index;
	element.object.pos.x = params.leftBottomX;
	element.object.pos.y = params.leftBottomY;
	element.object.level = params.leftBottomZ;
	element.object.xRatio = aParam;
	element.object.yRatio = bParam;
	element.object.angle = params.ang;
	element.header.floorInd = floorInd;
	element.header.layer = layerInd_SlabTableform;

	// 타입
	setParameterByName (&memo, "type", params.type);

	// 객체 배치
	ACAPI_Element_Create (&element, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	err;
}

// 유로폼 배치
GSErrCode	placeEuroform (Euroform params)
{
	GSErrCode	err = NoError;

	API_Element			element;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	double	aParam;
	double	bParam;
	Int32	addParNum;

	char	tempString [20];

	// GUID 변수 초기화
	element.header.guid.clock_seq_hi_and_reserved = 0;
	element.header.guid.clock_seq_low = 0;
	element.header.guid.node[0] = 0;
	element.header.guid.node[1] = 0;
	element.header.guid.node[2] = 0;
	element.header.guid.node[3] = 0;
	element.header.guid.node[4] = 0;
	element.header.guid.node[5] = 0;
	element.header.guid.time_hi_and_version = 0;
	element.header.guid.time_low = 0;
	element.header.guid.time_mid = 0;

	// 객체 로드
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, L("유로폼v2.0.gsm"));
	err = ACAPI_LibPart_Search (&libPart, false);
	if (err != NoError)
		return err;
	if (libPart.location != NULL)
		delete libPart.location;

	ACAPI_LibPart_Get (&libPart);

	BNZeroMemory (&element, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));

	element.header.typeID = API_ObjectID;
	element.header.guid = GSGuid2APIGuid (GS::Guid (libPart.ownUnID));

	ACAPI_Element_GetDefaults (&element, &memo);
	ACAPI_LibPart_GetParams (libPart.index, &aParam, &bParam, &addParNum, &memo.params);

	// 라이브러리의 파라미터 값 입력
	element.object.libInd = libPart.index;
	element.object.pos.x = params.leftBottomX;
	element.object.pos.y = params.leftBottomY;
	element.object.level = params.leftBottomZ;
	element.object.xRatio = aParam;
	element.object.yRatio = bParam;
	element.object.angle = params.ang;
	element.header.floorInd = floorInd;
	element.header.layer = layerInd_Euroform;

	// 유로폼
	setParameterByName (&memo, "u_comp", "유로폼");

	// 규격품일 경우,
	if (params.eu_stan_onoff == true) {
		setParameterByName (&memo, "eu_stan_onoff", 1.0);	// 규격폼 On/Off

		// 너비
		sprintf (tempString, "%.0f", params.eu_wid * 1000);
		setParameterByName (&memo, "eu_wid", tempString);

		// 높이
		sprintf (tempString, "%.0f", params.eu_hei * 1000);
		setParameterByName (&memo, "eu_hei", tempString);

	// 비규격품일 경우,
	} else {
		setParameterByName (&memo, "eu_stan_onoff", 0.0);		// 규격폼 On/Off
		setParameterByName (&memo, "eu_wid2", params.eu_wid2);	// 너비
		setParameterByName (&memo, "eu_hei2", params.eu_hei2);	// 높이
	}

	// 설치방향
	if (params.u_ins_wall == true) {
		strcpy (tempString, "벽세우기");
	} else {
		strcpy (tempString, "벽눕히기");
	}
	setParameterByName (&memo, "u_ins", tempString);
	setParameterByName (&memo, "ang_x", params.ang_x);	// 회전X

	// 객체 배치
	ACAPI_Element_Create (&element, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	err;
}

// 스틸폼 배치
GSErrCode	placeSteelform (Euroform params)
{
	GSErrCode	err = NoError;

	API_Element			element;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	double	aParam;
	double	bParam;
	Int32	addParNum;

	char	tempString [20];

	// GUID 변수 초기화
	element.header.guid.clock_seq_hi_and_reserved = 0;
	element.header.guid.clock_seq_low = 0;
	element.header.guid.node[0] = 0;
	element.header.guid.node[1] = 0;
	element.header.guid.node[2] = 0;
	element.header.guid.node[3] = 0;
	element.header.guid.node[4] = 0;
	element.header.guid.node[5] = 0;
	element.header.guid.time_hi_and_version = 0;
	element.header.guid.time_low = 0;
	element.header.guid.time_mid = 0;

	// 객체 로드
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, L("유로폼v2.0.gsm"));
	err = ACAPI_LibPart_Search (&libPart, false);
	if (err != NoError)
		return err;
	if (libPart.location != NULL)
		delete libPart.location;

	ACAPI_LibPart_Get (&libPart);

	BNZeroMemory (&element, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));

	element.header.typeID = API_ObjectID;
	element.header.guid = GSGuid2APIGuid (GS::Guid (libPart.ownUnID));

	ACAPI_Element_GetDefaults (&element, &memo);
	ACAPI_LibPart_GetParams (libPart.index, &aParam, &bParam, &addParNum, &memo.params);

	// 라이브러리의 파라미터 값 입력
	element.object.libInd = libPart.index;
	element.object.pos.x = params.leftBottomX;
	element.object.pos.y = params.leftBottomY;
	element.object.level = params.leftBottomZ;
	element.object.xRatio = aParam;
	element.object.yRatio = bParam;
	element.object.angle = params.ang;
	element.header.floorInd = floorInd;
	element.header.layer = layerInd_Steelform;

	// 스틸폼
	setParameterByName (&memo, "u_comp", "스틸폼");

	// 규격품일 경우,
	if (params.eu_stan_onoff == true) {
		setParameterByName (&memo, "eu_stan_onoff", 1.0);	// 규격폼 On/Off

		// 너비
		sprintf (tempString, "%.0f", params.eu_wid * 1000);
		setParameterByName (&memo, "eu_wid", tempString);

		// 높이
		sprintf (tempString, "%.0f", params.eu_hei * 1000);
		setParameterByName (&memo, "eu_hei", tempString);

	// 비규격품일 경우,
	} else {
		setParameterByName (&memo, "eu_stan_onoff", 0.0);		// 규격폼 On/Off
		setParameterByName (&memo, "eu_wid2", params.eu_wid2);	// 너비
		setParameterByName (&memo, "eu_hei2", params.eu_hei2);	// 높이
	}

	// 설치방향
	if (params.u_ins_wall == true) {
		strcpy (tempString, "벽세우기");
	} else {
		strcpy (tempString, "벽눕히기");
	}
	setParameterByName (&memo, "u_ins", tempString);
	setParameterByName (&memo, "ang_x", params.ang_x);	// 회전X

	// 객체 배치
	ACAPI_Element_Create (&element, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	err;
}

// 합판 배치
GSErrCode	placePlywood (Plywood params)
{
	GSErrCode	err = NoError;

	API_Element			element;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	double	aParam;
	double	bParam;
	Int32	addParNum;

	// GUID 변수 초기화
	element.header.guid.clock_seq_hi_and_reserved = 0;
	element.header.guid.clock_seq_low = 0;
	element.header.guid.node[0] = 0;
	element.header.guid.node[1] = 0;
	element.header.guid.node[2] = 0;
	element.header.guid.node[3] = 0;
	element.header.guid.node[4] = 0;
	element.header.guid.node[5] = 0;
	element.header.guid.time_hi_and_version = 0;
	element.header.guid.time_low = 0;
	element.header.guid.time_mid = 0;

	// 객체 로드
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, L("합판v1.0.gsm"));
	err = ACAPI_LibPart_Search (&libPart, false);
	if (err != NoError)
		return err;
	if (libPart.location != NULL)
		delete libPart.location;

	ACAPI_LibPart_Get (&libPart);

	BNZeroMemory (&element, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));

	element.header.typeID = API_ObjectID;
	element.header.guid = GSGuid2APIGuid (GS::Guid (libPart.ownUnID));

	ACAPI_Element_GetDefaults (&element, &memo);
	ACAPI_LibPart_GetParams (libPart.index, &aParam, &bParam, &addParNum, &memo.params);

	// 라이브러리의 파라미터 값 입력
	element.object.libInd = libPart.index;
	element.object.pos.x = params.leftBottomX;
	element.object.pos.y = params.leftBottomY;
	element.object.level = params.leftBottomZ;
	element.object.xRatio = aParam;
	element.object.yRatio = bParam;
	element.object.angle = params.ang;
	element.header.floorInd = floorInd;
	element.header.layer = layerInd_Plywood;

	// 합판
	setParameterByName (&memo, "g_comp", "합판");

	// 규격
	setParameterByName (&memo, "p_stan", "비규격");

	// 설치방향
	if (params.w_dir == 1)
		setParameterByName (&memo, "w_dir", "벽세우기");
	else if (params.w_dir == 2)
		setParameterByName (&memo, "w_dir", "벽눕히기");
	else if (params.w_dir == 3)
		setParameterByName (&memo, "w_dir", "바닥깔기");
	else if (params.w_dir == 4)
		setParameterByName (&memo, "w_dir", "바닥덮기");

	// 두께
	setParameterByName (&memo, "p_thk", "11.5T");

	// 가로
	setParameterByName (&memo, "p_wid", params.p_wid);

	// 세로
	setParameterByName (&memo, "p_leng", params.p_leng);

	// 제작틀
	setParameterByName (&memo, "sogak", TRUE);
	setParameterByName (&memo, "prof", "소각");

	// 객체 배치
	ACAPI_Element_Create (&element, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	err;
}

// 휠러스페이서 배치
GSErrCode	placeFillersp (FillerSpacer params)
{
	GSErrCode	err = NoError;

	API_Element			element;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	double	aParam;
	double	bParam;
	Int32	addParNum;

	// GUID 변수 초기화
	element.header.guid.clock_seq_hi_and_reserved = 0;
	element.header.guid.clock_seq_low = 0;
	element.header.guid.node[0] = 0;
	element.header.guid.node[1] = 0;
	element.header.guid.node[2] = 0;
	element.header.guid.node[3] = 0;
	element.header.guid.node[4] = 0;
	element.header.guid.node[5] = 0;
	element.header.guid.time_hi_and_version = 0;
	element.header.guid.time_low = 0;
	element.header.guid.time_mid = 0;

	// 객체 로드
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, L("휠러스페이서v1.0.gsm"));
	err = ACAPI_LibPart_Search (&libPart, false);
	if (err != NoError)
		return err;
	if (libPart.location != NULL)
		delete libPart.location;

	ACAPI_LibPart_Get (&libPart);

	BNZeroMemory (&element, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));

	element.header.typeID = API_ObjectID;
	element.header.guid = GSGuid2APIGuid (GS::Guid (libPart.ownUnID));

	ACAPI_Element_GetDefaults (&element, &memo);
	ACAPI_LibPart_GetParams (libPart.index, &aParam, &bParam, &addParNum, &memo.params);

	// 라이브러리의 파라미터 값 입력
	element.object.libInd = libPart.index;
	element.object.pos.x = params.leftBottomX;
	element.object.pos.y = params.leftBottomY;
	element.object.level = params.leftBottomZ;
	element.object.xRatio = aParam;
	element.object.yRatio = bParam;
	element.object.angle = params.ang;
	element.header.floorInd = floorInd;
	element.header.layer = layerInd_Fillersp;

	// 두께
	setParameterByName (&memo, "f_thk", params.f_thk);

	// 길이
	setParameterByName (&memo, "f_leng", params.f_leng);

	// 각도
	setParameterByName (&memo, "f_ang", params.f_ang);

	// 회전
	setParameterByName (&memo, "f_rota", params.f_rota);

	// 객체 배치
	ACAPI_Element_Create (&element, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	err;
}

// 아웃코너앵글 배치
GSErrCode	placeOutcornerAngle (OutcornerAngle params)
{
	GSErrCode	err = NoError;

	API_Element			element;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	double	aParam;
	double	bParam;
	Int32	addParNum;

	// GUID 변수 초기화
	element.header.guid.clock_seq_hi_and_reserved = 0;
	element.header.guid.clock_seq_low = 0;
	element.header.guid.node[0] = 0;
	element.header.guid.node[1] = 0;
	element.header.guid.node[2] = 0;
	element.header.guid.node[3] = 0;
	element.header.guid.node[4] = 0;
	element.header.guid.node[5] = 0;
	element.header.guid.time_hi_and_version = 0;
	element.header.guid.time_low = 0;
	element.header.guid.time_mid = 0;

	// 객체 로드
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, L("아웃코너앵글v1.0.gsm"));
	err = ACAPI_LibPart_Search (&libPart, false);
	if (err != NoError)
		return err;
	if (libPart.location != NULL)
		delete libPart.location;

	ACAPI_LibPart_Get (&libPart);

	BNZeroMemory (&element, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));

	element.header.typeID = API_ObjectID;
	element.header.guid = GSGuid2APIGuid (GS::Guid (libPart.ownUnID));

	ACAPI_Element_GetDefaults (&element, &memo);
	ACAPI_LibPart_GetParams (libPart.index, &aParam, &bParam, &addParNum, &memo.params);

	// 라이브러리의 파라미터 값 입력
	element.object.libInd = libPart.index;
	element.object.pos.x = params.leftBottomX;
	element.object.pos.y = params.leftBottomY;
	element.object.level = params.leftBottomZ;
	element.object.xRatio = aParam;
	element.object.yRatio = bParam;
	element.object.angle = params.ang;
	element.header.floorInd = floorInd;
	element.header.layer = layerInd_OutcornerAngle;

	// 길이
	setParameterByName (&memo, "a_leng", params.a_leng);

	// 각도
	setParameterByName (&memo, "a_ang", params.a_ang);

	// 객체 배치
	ACAPI_Element_Create (&element, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	err;
}

// 아웃코너판넬 배치
GSErrCode	placeOutcornerPanel (OutcornerPanel params)
{
	GSErrCode	err = NoError;

	API_Element			element;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	double	aParam;
	double	bParam;
	Int32	addParNum;

	// GUID 변수 초기화
	element.header.guid.clock_seq_hi_and_reserved = 0;
	element.header.guid.clock_seq_low = 0;
	element.header.guid.node[0] = 0;
	element.header.guid.node[1] = 0;
	element.header.guid.node[2] = 0;
	element.header.guid.node[3] = 0;
	element.header.guid.node[4] = 0;
	element.header.guid.node[5] = 0;
	element.header.guid.time_hi_and_version = 0;
	element.header.guid.time_low = 0;
	element.header.guid.time_mid = 0;

	// 객체 로드
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, L("아웃코너판넬v1.0.gsm"));
	err = ACAPI_LibPart_Search (&libPart, false);
	if (err != NoError)
		return err;
	if (libPart.location != NULL)
		delete libPart.location;

	ACAPI_LibPart_Get (&libPart);

	BNZeroMemory (&element, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));

	element.header.typeID = API_ObjectID;
	element.header.guid = GSGuid2APIGuid (GS::Guid (libPart.ownUnID));

	ACAPI_Element_GetDefaults (&element, &memo);
	ACAPI_LibPart_GetParams (libPart.index, &aParam, &bParam, &addParNum, &memo.params);

	// 라이브러리의 파라미터 값 입력
	element.object.libInd = libPart.index;
	element.object.pos.x = params.leftBottomX;
	element.object.pos.y = params.leftBottomY;
	element.object.level = params.leftBottomZ;
	element.object.xRatio = aParam;
	element.object.yRatio = bParam;
	element.object.angle = params.ang;
	element.header.floorInd = floorInd;
	element.header.layer = layerInd_OutcornerPanel;

	setParameterByName (&memo, "wid_s", params.wid_s);		// 가로
	setParameterByName (&memo, "leng_s", params.leng_s);	// 세로
	setParameterByName (&memo, "hei_s", params.hei_s);		// 높이
	setParameterByName (&memo, "dir_s", "세우기");			// 설치방향

	// 객체 배치
	ACAPI_Element_Create (&element, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	err;
}

// 인코너판넬 배치
GSErrCode	placeIncornerPanel (IncornerPanel params)
{
	GSErrCode	err = NoError;

	API_Element			element;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	double	aParam;
	double	bParam;
	Int32	addParNum;

	// GUID 변수 초기화
	element.header.guid.clock_seq_hi_and_reserved = 0;
	element.header.guid.clock_seq_low = 0;
	element.header.guid.node[0] = 0;
	element.header.guid.node[1] = 0;
	element.header.guid.node[2] = 0;
	element.header.guid.node[3] = 0;
	element.header.guid.node[4] = 0;
	element.header.guid.node[5] = 0;
	element.header.guid.time_hi_and_version = 0;
	element.header.guid.time_low = 0;
	element.header.guid.time_mid = 0;

	// 객체 로드
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, L("인코너판넬v1.0.gsm"));
	err = ACAPI_LibPart_Search (&libPart, false);
	if (err != NoError)
		return err;
	if (libPart.location != NULL)
		delete libPart.location;

	ACAPI_LibPart_Get (&libPart);

	BNZeroMemory (&element, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));

	element.header.typeID = API_ObjectID;
	element.header.guid = GSGuid2APIGuid (GS::Guid (libPart.ownUnID));

	ACAPI_Element_GetDefaults (&element, &memo);
	ACAPI_LibPart_GetParams (libPart.index, &aParam, &bParam, &addParNum, &memo.params);

	// 라이브러리의 파라미터 값 입력
	element.object.libInd = libPart.index;
	element.object.pos.x = params.leftBottomX;
	element.object.pos.y = params.leftBottomY;
	element.object.level = params.leftBottomZ;
	element.object.xRatio = aParam;
	element.object.yRatio = bParam;
	element.object.angle = params.ang;
	element.header.floorInd = floorInd;
	element.header.layer = layerInd_IncornerPanel;

	setParameterByName (&memo, "wid_s", params.wid_s);		// 가로
	setParameterByName (&memo, "leng_s", params.leng_s);	// 세로
	setParameterByName (&memo, "hei_s", params.hei_s);		// 높이
	setParameterByName (&memo, "dir_s", "세우기");			// 설치방향

	// 객체 배치
	ACAPI_Element_Create (&element, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	err;
}

// 객체의 레이어를 선택하기 위한 다이얼로그
short DGCALLBACK convertVirtualTCOHandler1 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	API_UCCallbackType	ucb;

	switch (message) {
		case DG_MSG_INIT:
			// 다이얼로그 타이틀
			DGSetDialogTitle (dialogID, "가설재 레이어 선택하기");

			//////////////////////////////////////////////////////////// 아이템 배치 (기본 버튼)
			// 적용 버튼
			DGSetItemText (dialogID, DG_OK, "확 인");

			// 종료 버튼
			DGSetItemText (dialogID, DG_CANCEL, "취 소");

			// 레이어 관련 라벨
			DGSetItemText (dialogID, LABEL_LAYER_SETTINGS, "부재별 레이어 설정");
			DGSetItemText (dialogID, LABEL_LAYER_SLABTABLEFORM, "슬래브 테이블폼");
			DGSetItemText (dialogID, LABEL_LAYER_PROFILE, "C형강");
			DGSetItemText (dialogID, LABEL_LAYER_FITTINGS, "결합철물");
			DGSetItemText (dialogID, LABEL_LAYER_EUROFORM, "유로폼");
			DGSetItemText (dialogID, LABEL_LAYER_RECTPIPE, "비계 파이프");
			DGSetItemText (dialogID, LABEL_LAYER_PINBOLT, "핀볼트 세트");
			DGSetItemText (dialogID, LABEL_LAYER_WALLTIE, "벽체 타이");
			DGSetItemText (dialogID, LABEL_LAYER_JOIN, "결합철물");
			DGSetItemText (dialogID, LABEL_LAYER_HEADPIECE, "헤드피스");
			DGSetItemText (dialogID, LABEL_LAYER_STEELFORM, "스틸폼");
			DGSetItemText (dialogID, LABEL_LAYER_PLYWOOD, "합판");
			DGSetItemText (dialogID, LABEL_LAYER_WOOD, "목재");
			DGSetItemText (dialogID, LABEL_LAYER_FILLERSP, "휠러스페이서");
			DGSetItemText (dialogID, LABEL_LAYER_OUTCORNER_ANGLE, "아웃코너앵글");
			DGSetItemText (dialogID, LABEL_LAYER_OUTCORNER_PANEL, "아웃코너판넬");
			DGSetItemText (dialogID, LABEL_LAYER_INCORNER_PANEL, "인코너판넬");

			// 유저 컨트롤 초기화
			BNZeroMemory (&ucb, sizeof (ucb));
			ucb.dialogID = dialogID;
			ucb.type	 = APIUserControlType_Layer;
			ucb.itemID	 = USERCONTROL_LAYER_SLABTABLEFORM;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM, 1);
			if (bLayerInd_SlabTableform == true) {
				DGEnableItem (dialogID, LABEL_LAYER_SLABTABLEFORM);
				DGEnableItem (dialogID, USERCONTROL_LAYER_SLABTABLEFORM);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_SLABTABLEFORM);
				DGDisableItem (dialogID, USERCONTROL_LAYER_SLABTABLEFORM);
			}

			ucb.itemID	 = USERCONTROL_LAYER_PROFILE;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE, 1);
			if (bLayerInd_Profile == true) {
				DGEnableItem (dialogID, LABEL_LAYER_PROFILE);
				DGEnableItem (dialogID, USERCONTROL_LAYER_PROFILE);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_PROFILE);
				DGDisableItem (dialogID, USERCONTROL_LAYER_PROFILE);
			}

			ucb.itemID	 = USERCONTROL_LAYER_FITTINGS;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_FITTINGS, 1);
			if (bLayerInd_Fittings == true) {
				DGEnableItem (dialogID, LABEL_LAYER_FITTINGS);
				DGEnableItem (dialogID, USERCONTROL_LAYER_FITTINGS);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_FITTINGS);
				DGDisableItem (dialogID, USERCONTROL_LAYER_FITTINGS);
			}

			ucb.itemID	 = USERCONTROL_LAYER_EUROFORM;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, 1);
			if (bLayerInd_Euroform == true) {
				DGEnableItem (dialogID, LABEL_LAYER_EUROFORM);
				DGEnableItem (dialogID, USERCONTROL_LAYER_EUROFORM);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_EUROFORM);
				DGDisableItem (dialogID, USERCONTROL_LAYER_EUROFORM);
			}

			ucb.itemID	 = USERCONTROL_LAYER_RECTPIPE;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, 1);
			if (bLayerInd_RectPipe == true) {
				DGEnableItem (dialogID, LABEL_LAYER_RECTPIPE);
				DGEnableItem (dialogID, USERCONTROL_LAYER_RECTPIPE);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_RECTPIPE);
				DGDisableItem (dialogID, USERCONTROL_LAYER_RECTPIPE);
			}

			ucb.itemID	 = USERCONTROL_LAYER_PINBOLT;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, 1);
			if (bLayerInd_PinBolt == true) {
				DGEnableItem (dialogID, LABEL_LAYER_PINBOLT);
				DGEnableItem (dialogID, USERCONTROL_LAYER_PINBOLT);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_PINBOLT);
				DGDisableItem (dialogID, USERCONTROL_LAYER_PINBOLT);
			}

			ucb.itemID	 = USERCONTROL_LAYER_WALLTIE;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, 1);
			if (bLayerInd_WallTie == true) {
				DGEnableItem (dialogID, LABEL_LAYER_WALLTIE);
				DGEnableItem (dialogID, USERCONTROL_LAYER_WALLTIE);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_WALLTIE);
				DGDisableItem (dialogID, USERCONTROL_LAYER_WALLTIE);
			}

			ucb.itemID	 = USERCONTROL_LAYER_JOIN;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, 1);
			if (bLayerInd_Join == true) {
				DGEnableItem (dialogID, LABEL_LAYER_JOIN);
				DGEnableItem (dialogID, USERCONTROL_LAYER_JOIN);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_JOIN);
				DGDisableItem (dialogID, USERCONTROL_LAYER_JOIN);
			}

			ucb.itemID	 = USERCONTROL_LAYER_HEADPIECE;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, 1);
			if (bLayerInd_HeadPiece == true) {
				DGEnableItem (dialogID, LABEL_LAYER_HEADPIECE);
				DGEnableItem (dialogID, USERCONTROL_LAYER_HEADPIECE);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_HEADPIECE);
				DGDisableItem (dialogID, USERCONTROL_LAYER_HEADPIECE);
			}

			ucb.itemID	 = USERCONTROL_LAYER_STEELFORM;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM, 1);
			if (bLayerInd_Steelform == true) {
				DGEnableItem (dialogID, LABEL_LAYER_STEELFORM);
				DGEnableItem (dialogID, USERCONTROL_LAYER_STEELFORM);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_STEELFORM);
				DGDisableItem (dialogID, USERCONTROL_LAYER_STEELFORM);
			}

			ucb.itemID	 = USERCONTROL_LAYER_PLYWOOD;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, 1);
			if (bLayerInd_Plywood == true) {
				DGEnableItem (dialogID, LABEL_LAYER_PLYWOOD);
				DGEnableItem (dialogID, USERCONTROL_LAYER_PLYWOOD);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_PLYWOOD);
				DGDisableItem (dialogID, USERCONTROL_LAYER_PLYWOOD);
			}

			ucb.itemID	 = USERCONTROL_LAYER_WOOD;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_WOOD, 1);
			if (bLayerInd_Wood == true) {
				DGEnableItem (dialogID, LABEL_LAYER_WOOD);
				DGEnableItem (dialogID, USERCONTROL_LAYER_WOOD);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_WOOD);
				DGDisableItem (dialogID, USERCONTROL_LAYER_WOOD);
			}

			ucb.itemID	 = USERCONTROL_LAYER_FILLERSP;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP, 1);
			if (bLayerInd_Fillersp == true) {
				DGEnableItem (dialogID, LABEL_LAYER_FILLERSP);
				DGEnableItem (dialogID, USERCONTROL_LAYER_FILLERSP);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_FILLERSP);
				DGDisableItem (dialogID, USERCONTROL_LAYER_FILLERSP);
			}

			ucb.itemID	 = USERCONTROL_LAYER_OUTCORNER_ANGLE;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, 1);
			if (bLayerInd_OutcornerAngle == true) {
				DGEnableItem (dialogID, LABEL_LAYER_OUTCORNER_ANGLE);
				DGEnableItem (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_OUTCORNER_ANGLE);
				DGDisableItem (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE);
			}

			ucb.itemID	 = USERCONTROL_LAYER_OUTCORNER_PANEL;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, 1);
			if (bLayerInd_OutcornerPanel == true) {
				DGEnableItem (dialogID, LABEL_LAYER_OUTCORNER_PANEL);
				DGEnableItem (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_OUTCORNER_PANEL);
				DGDisableItem (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL);
			}

			ucb.itemID	 = USERCONTROL_LAYER_INCORNER_PANEL;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL, 1);
			if (bLayerInd_IncornerPanel == true) {
				DGEnableItem (dialogID, LABEL_LAYER_INCORNER_PANEL);
				DGEnableItem (dialogID, USERCONTROL_LAYER_INCORNER_PANEL);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_INCORNER_PANEL);
				DGDisableItem (dialogID, USERCONTROL_LAYER_INCORNER_PANEL);
			}

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					// 레이어 번호 저장
					if (bLayerInd_SlabTableform == true)	layerInd_SlabTableform	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM);
					if (bLayerInd_Profile == true)			layerInd_Profile		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE);
					if (bLayerInd_Fittings == true)			layerInd_Fittings		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_FITTINGS);
					if (bLayerInd_Euroform == true)			layerInd_Euroform		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM);
					if (bLayerInd_RectPipe == true)			layerInd_RectPipe		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE);
					if (bLayerInd_PinBolt == true)			layerInd_PinBolt		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT);
					if (bLayerInd_WallTie == true)			layerInd_WallTie		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE);
					if (bLayerInd_Join == true)				layerInd_Join			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN);
					if (bLayerInd_HeadPiece == true)		layerInd_HeadPiece		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE);
					if (bLayerInd_Steelform == true)		layerInd_Steelform		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM);
					if (bLayerInd_Plywood == true)			layerInd_Plywood		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD);
					if (bLayerInd_Wood == true)				layerInd_Wood			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_WOOD);
					if (bLayerInd_Fillersp == true)			layerInd_Fillersp		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP);
					if (bLayerInd_OutcornerAngle == true)	layerInd_OutcornerAngle	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE);
					if (bLayerInd_OutcornerPanel == true)	layerInd_OutcornerPanel	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL);
					if (bLayerInd_IncornerPanel == true)	layerInd_IncornerPanel	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL);

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
