#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "LibraryConvert.hpp"

using namespace libraryConvertDG;

short	floorInd;		// 가상 가설재의 층 인덱스 저장

// 해당 레이어를 사용하는지 여부
static bool		bLayerInd_Euroform;			// 레이어 번호: 유로폼
static bool		bLayerInd_RectPipe;			// 레이어 번호: 비계 파이프
static bool		bLayerInd_PinBolt;			// 레이어 번호: 핀볼트 세트
static bool		bLayerInd_WallTie;			// 레이어 번호: 벽체 타이
static bool		bLayerInd_HeadPiece;		// 레이어 번호: 헤드피스
static bool		bLayerInd_Join;				// 레이어 번호: 결합철물

static bool		bLayerInd_SlabTableform;	// 레이어 번호: 슬래브 테이블폼
static bool		bLayerInd_Profile;			// 레이어 번호: KS프로파일

static bool		bLayerInd_Steelform;		// 레이어 번호: 스틸폼
static bool		bLayerInd_Plywood;			// 레이어 번호: 합판
static bool		bLayerInd_Fillersp;			// 레이어 번호: 휠러스페이서
static bool		bLayerInd_OutcornerAngle;	// 레이어 번호: 아웃코너앵글
static bool		bLayerInd_OutcornerPanel;	// 레이어 번호: 아웃코너앵글
static bool		bLayerInd_IncornerPanel;	// 레이어 번호: 인코너앵글
static bool		bLayerInd_RectpipeHanger;	// 레이어 번호: 각파이프 행거
static bool		bLayerInd_EuroformHook;		// 레이어 번호: 유로폼 후크
static bool		bLayerInd_Hidden;			// 레이어 번호: 숨김

// 해당 레이어의 번호
static short	layerInd_Euroform;			// 레이어 번호: 유로폼
static short	layerInd_RectPipe;			// 레이어 번호: 비계 파이프
static short	layerInd_PinBolt;			// 레이어 번호: 핀볼트 세트
static short	layerInd_WallTie;			// 레이어 번호: 벽체 타이
static short	layerInd_HeadPiece;			// 레이어 번호: 헤드피스
static short	layerInd_Join;				// 레이어 번호: 결합철물

static short	layerInd_SlabTableform;		// 레이어 번호: 슬래브 테이블폼
static short	layerInd_Profile;			// 레이어 번호: KS프로파일

static short	layerInd_Steelform;			// 레이어 번호: 스틸폼
static short	layerInd_Plywood;			// 레이어 번호: 합판
static short	layerInd_Fillersp;			// 레이어 번호: 휠러스페이서
static short	layerInd_OutcornerAngle;	// 레이어 번호: 아웃코너앵글
static short	layerInd_OutcornerPanel;	// 레이어 번호: 아웃코너앵글
static short	layerInd_IncornerPanel;		// 레이어 번호: 인코너앵글
static short	layerInd_RectpipeHanger;	// 레이어 번호: 각파이프 행거
static short	layerInd_EuroformHook;		// 레이어 번호: 유로폼 후크
static short	layerInd_Hidden;			// 레이어 번호: 숨김

static GS::Array<API_Guid>	elemList;		// 그룹화를 위해 생성된 결과물들의 GUID를 전부 저장함
static GS::Array<API_Guid>	elemListBack;	// 그룹화를 위해 생성된 결과물들의 GUID를 전부 저장함

bool	bDoubleSide;	// 테이블폼(벽): 양면(true), 단면(false)
double	wallThk;		// 벽 두께


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
	char			objType [32];		// 테이블폼(벽) 타입1, 테이블폼(벽) 타입2, 테이블폼(슬래브), 유로폼, 스틸폼, 합판, 휠러스페이서, 아웃코너앵글, 아웃코너판넬, 인코너판넬
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
	GS::Array<API_Guid>		objects;
	GS::Array<API_Guid>		objectsRetry;


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
	bLayerInd_HeadPiece = false;
	bLayerInd_Join = false;

	bLayerInd_SlabTableform = false;
	bLayerInd_Profile = false;

	bLayerInd_Steelform = false;
	bLayerInd_Plywood = false;
	bLayerInd_Fillersp = false;
	bLayerInd_OutcornerAngle = false;
	bLayerInd_OutcornerPanel = false;
	bLayerInd_IncornerPanel = false;
	bLayerInd_RectpipeHanger = false;
	bLayerInd_EuroformHook = false;
	bLayerInd_Hidden = false;

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

		if (my_strcmp (productName, "가상 가설재") == 0) {
			
			// 가상 가설재의 파라미터 값 불러오기
			tempStr = getParameterStringByName (&memo, "objType");
			strncpy (objType, tempStr, strlen (tempStr));
			objType [strlen (tempStr)] = '\0';

			if (my_strcmp (objType, "테이블폼(벽) 타입1") == 0) {
				bLayerInd_Euroform = true;
				bLayerInd_RectPipe = true;
				bLayerInd_PinBolt = true;
				bLayerInd_HeadPiece = true;
				bLayerInd_Join = true;
			}
			if (my_strcmp (objType, "테이블폼(벽) 타입2") == 0) {
				bLayerInd_Euroform = true;
				bLayerInd_RectPipe = true;
				bLayerInd_RectpipeHanger = true;
				bLayerInd_EuroformHook = true;
				bLayerInd_HeadPiece = true;
				bLayerInd_Join = true;
				bLayerInd_Hidden = true;
			}
			if (my_strcmp (objType, "테이블폼(슬래브)") == 0) {
				bLayerInd_SlabTableform = true;
				bLayerInd_Profile = true;
				bLayerInd_Join = true;
			}
			if (my_strcmp (objType, "유로폼") == 0) {
				bLayerInd_Euroform = true;

			}
			if (my_strcmp (objType, "스틸폼") == 0) {
				bLayerInd_Steelform = true;

			}
			if (my_strcmp (objType, "합판") == 0) {
				bLayerInd_Plywood = true;

			}
			if (my_strcmp (objType, "휠러스페이서") == 0) {
				bLayerInd_Fillersp = true;

			}
			if (my_strcmp (objType, "아웃코너앵글") == 0) {
				bLayerInd_OutcornerAngle = true;

			}
			if (my_strcmp (objType, "아웃코너판넬") == 0) {
				bLayerInd_OutcornerPanel = true;

			}
			if (my_strcmp (objType, "인코너판넬") == 0) {
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

		if (my_strcmp (productName, "가상 가설재") == 0) {

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
			if (getParameterValueByName (&memo, "leftSide") > 0)						// 왼쪽(1), 오른쪽(0)
				leftSide = true;
			else
				leftSide = false;
			if (getParameterValueByName (&memo, "bRegularSize") > 0)					// 정규 크기(1)
				bRegularSize = true;
			else
				bRegularSize = false;
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
				
				walltableform.leftBottomX = elem.object.pos.x;
				walltableform.leftBottomY = elem.object.pos.y;
				walltableform.leftBottomZ = elem.object.level;
				walltableform.ang = elem.object.angle;
				walltableform.width = unit_A;
				walltableform.height = unit_ZZYZX;

				// 벽 두께
				wallThk = oppSideOffset;

				if (my_strcmp (coverSide, "양면") == 0) {
					bDoubleSide = true;
				} else {
					bDoubleSide = false;
				}

				if (my_strcmp (dir, "벽세우기") == 0) {
					for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
						for (yy = 0 ; yy < num_A ; ++yy) {
							if (my_strcmp (objType, "테이블폼(벽) 타입1") == 0)
								placeTableformOnWall_portrait_Type1 (walltableform);
							else if (my_strcmp (objType, "테이블폼(벽) 타입2") == 0)
								placeTableformOnWall_portrait_Type2 (walltableform);
							moveIn3D ('x', elem.object.angle, unit_A, &walltableform.leftBottomX, &walltableform.leftBottomY, &walltableform.leftBottomZ);
						}
						moveIn3D ('x', elem.object.angle, -unit_A * num_A, &walltableform.leftBottomX, &walltableform.leftBottomY, &walltableform.leftBottomZ);
						moveIn3D ('z', elem.object.angle, unit_ZZYZX, &walltableform.leftBottomX, &walltableform.leftBottomY, &walltableform.leftBottomZ);
					}
				} else if (my_strcmp (dir, "벽눕히기") == 0) {
					for (xx = 0 ; xx < num_A ; ++xx) {
						for (yy = 0 ; yy < num_ZZYZX ; ++yy) {
							if (my_strcmp (objType, "테이블폼(벽) 타입1") == 0)
								placeTableformOnWall_landscape_Type1 (walltableform);
							else if (my_strcmp (objType, "테이블폼(벽) 타입2") == 0)
								placeTableformOnWall_landscape_Type2 (walltableform);
							moveIn3D ('x', elem.object.angle, unit_ZZYZX, &walltableform.leftBottomX, &walltableform.leftBottomY, &walltableform.leftBottomZ);
						}
						moveIn3D ('x', elem.object.angle, -unit_ZZYZX * num_ZZYZX, &walltableform.leftBottomX, &walltableform.leftBottomY, &walltableform.leftBottomZ);
						moveIn3D ('z', elem.object.angle, unit_A, &walltableform.leftBottomX, &walltableform.leftBottomY, &walltableform.leftBottomZ);
					}
				}
			} else if (my_strcmp (objType, "테이블폼(슬래브)") == 0) {

				slabtableform.leftBottomX = elem.object.pos.x;
				slabtableform.leftBottomY = elem.object.pos.y;
				slabtableform.leftBottomZ = elem.object.level;
				slabtableform.ang = elem.object.angle;

				slabtableform.direction = true;
				slabtableform.horLen = unit_A;
				slabtableform.verLen = unit_B;
				sprintf (slabtableform.type, "%.0f x %.0f", round (unit_A * 1000, 0), round (unit_B * 1000, 0));
				
				if (my_strcmp (dir, "바닥깔기") == 0) {

					// 슬래브폼 배치
					for (xx = 0 ; xx < num_B ; ++xx) {
						for (yy = 0 ; yy < num_A ; ++yy) {
							elemList.Push (placeTableformOnSlabBottom (slabtableform));
							moveIn3D ('x', elem.object.angle, unit_A, &slabtableform.leftBottomX, &slabtableform.leftBottomY, &slabtableform.leftBottomZ);
						}
						moveIn3D ('x', elem.object.angle, -unit_A * num_A, &slabtableform.leftBottomX, &slabtableform.leftBottomY, &slabtableform.leftBottomZ);
						moveIn3D ('y', elem.object.angle, unit_B, &slabtableform.leftBottomX, &slabtableform.leftBottomY, &slabtableform.leftBottomZ);
					}
				}

				// 그룹화하기
				if (!elemList.IsEmpty ()) {
					GSSize nElems = elemList.GetSize ();
					API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
					if (elemHead != NULL) {
						for (GSIndex i = 0; i < nElems; i++)
							(*elemHead)[i].guid = elemList[i];

						ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

						BMKillHandle ((GSHandle *) &elemHead);
					}
				}
				elemList.Clear (false);
			} else if (my_strcmp (objType, "유로폼") == 0) {

				euroform.ang = elem.object.angle;
				euroform.eu_stan_onoff = bRegularSize;
				if (bRegularSize == true) {
					euroform.eu_wid = unit_A;
					euroform.eu_hei = unit_ZZYZX;
				} else {
					euroform.eu_wid2 = unit_A;
					euroform.eu_hei2 = unit_ZZYZX;
				}

				if (my_strcmp (dir, "벽세우기") == 0) {
					euroform.leftBottomX = elem.object.pos.x;
					euroform.leftBottomY = elem.object.pos.y;
					euroform.leftBottomZ = elem.object.level;
					euroform.u_ins_wall = true;					// true: 벽세우기, false: 벽눕히기
					euroform.ang_x = DegreeToRad (90.0);		// 벽(90)

					for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
						for (yy = 0 ; yy < num_A ; ++yy) {
							elemList.Push (placeEuroform (euroform));
							moveIn3D ('x', elem.object.angle, unit_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						}
						moveIn3D ('x', elem.object.angle, -unit_A * num_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						moveIn3D ('z', elem.object.angle, unit_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
					}

					// 양면
					if (my_strcmp (coverSide, "양면") == 0) {
						euroform.leftBottomX = elem.object.pos.x;
						euroform.leftBottomY = elem.object.pos.y;
						euroform.leftBottomZ = elem.object.level;
						euroform.ang = elem.object.angle + DegreeToRad (180.0);
						moveIn3D ('x', elem.object.angle, unit_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						moveIn3D ('y', elem.object.angle, oppSideOffset, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						
						for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
							for (yy = 0 ; yy < num_A ; ++yy) {
								elemListBack.Push (placeEuroform (euroform));
								moveIn3D ('x', elem.object.angle, unit_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
							}
							moveIn3D ('x', elem.object.angle, -unit_A * num_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
							moveIn3D ('z', elem.object.angle, unit_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						}
					}
				} else if (my_strcmp (dir, "벽눕히기") == 0) {
					euroform.leftBottomX = elem.object.pos.x;
					euroform.leftBottomY = elem.object.pos.y;
					euroform.leftBottomZ = elem.object.level;
					moveIn3D ('x', elem.object.angle, unit_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
					euroform.u_ins_wall = false;				// true: 벽세우기, false: 벽눕히기
					euroform.ang_x = DegreeToRad (90.0);		// 벽(90)

					for (xx = 0 ; xx < num_A ; ++xx) {
						for (yy = 0 ; yy < num_ZZYZX ; ++yy) {
							elemList.Push (placeEuroform (euroform));
							moveIn3D ('x', elem.object.angle, unit_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						}
						moveIn3D ('x', elem.object.angle, -unit_ZZYZX * num_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						moveIn3D ('z', elem.object.angle, unit_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
					}

					// 양면
					if (my_strcmp (coverSide, "양면") == 0) {
						euroform.leftBottomX = elem.object.pos.x;
						euroform.leftBottomY = elem.object.pos.y;
						euroform.leftBottomZ = elem.object.level;
						euroform.ang = elem.object.angle + DegreeToRad (180.0);
						moveIn3D ('y', elem.object.angle, oppSideOffset, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						
						for (xx = 0 ; xx < num_A ; ++xx) {
							for (yy = 0 ; yy < num_ZZYZX ; ++yy) {
								elemListBack.Push (placeEuroform (euroform));
								moveIn3D ('x', elem.object.angle, unit_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
							}
							moveIn3D ('x', elem.object.angle, -unit_ZZYZX * num_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
							moveIn3D ('z', elem.object.angle, unit_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						}
					}
				} else if (my_strcmp (dir, "바닥깔기") == 0) {
					euroform.leftBottomX = elem.object.pos.x;
					euroform.leftBottomY = elem.object.pos.y;
					euroform.leftBottomZ = elem.object.level;
					moveIn3D ('y', elem.object.angle, unit_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
					euroform.u_ins_wall = true;					// true: 벽세우기, false: 벽눕히기
					euroform.ang_x = DegreeToRad (0.0);			// 천장(0)
					
					for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
						for (yy = 0 ; yy < num_A ; ++yy) {
							elemList.Push (placeEuroform (euroform));
							moveIn3D ('x', elem.object.angle, unit_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						}
						moveIn3D ('x', elem.object.angle, -unit_A * num_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						moveIn3D ('y', elem.object.angle, unit_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
					}
				} else if (my_strcmp (dir, "바닥덮기") == 0) {
					euroform.leftBottomX = elem.object.pos.x;
					euroform.leftBottomY = elem.object.pos.y;
					euroform.leftBottomZ = elem.object.level;
					euroform.u_ins_wall = true;					// true: 벽세우기, false: 벽눕히기
					euroform.ang_x = DegreeToRad (180.0);		// 바닥(180)

					for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
						for (yy = 0 ; yy < num_A ; ++yy) {
							elemList.Push (placeEuroform (euroform));
							moveIn3D ('x', elem.object.angle, unit_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						}
						moveIn3D ('x', elem.object.angle, -unit_A * num_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						moveIn3D ('y', elem.object.angle, unit_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
					}
				}

				// 그룹화하기
				if (!elemList.IsEmpty ()) {
					GSSize nElems = elemList.GetSize ();
					API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
					if (elemHead != NULL) {
						for (GSIndex i = 0; i < nElems; i++)
							(*elemHead)[i].guid = elemList[i];

						ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

						BMKillHandle ((GSHandle *) &elemHead);
					}
				}
				elemList.Clear (false);

				// 그룹화하기
				if (!elemListBack.IsEmpty ()) {
					GSSize nElems = elemListBack.GetSize ();
					API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
					if (elemHead != NULL) {
						for (GSIndex i = 0; i < nElems; i++)
							(*elemHead)[i].guid = elemListBack[i];

						ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

						BMKillHandle ((GSHandle *) &elemHead);
					}
				}
				elemListBack.Clear (false);
			} else if (my_strcmp (objType, "스틸폼") == 0) {

				euroform.ang = elem.object.angle;
				euroform.eu_stan_onoff = bRegularSize;
				if (bRegularSize == true) {
					euroform.eu_wid = unit_A;
					euroform.eu_hei = unit_ZZYZX;
				} else {
					euroform.eu_wid2 = unit_A;
					euroform.eu_hei2 = unit_ZZYZX;
				}

				if (my_strcmp (dir, "벽세우기") == 0) {
					euroform.leftBottomX = elem.object.pos.x;
					euroform.leftBottomY = elem.object.pos.y;
					euroform.leftBottomZ = elem.object.level;
					euroform.u_ins_wall = true;					// true: 벽세우기, false: 벽눕히기
					euroform.ang_x = DegreeToRad (90.0);		// 벽(90)

					for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
						for (yy = 0 ; yy < num_A ; ++yy) {
							elemList.Push (placeSteelform (euroform));
							moveIn3D ('x', elem.object.angle, unit_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						}
						moveIn3D ('x', elem.object.angle, -unit_A * num_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						moveIn3D ('z', elem.object.angle, unit_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
					}

					// 양면
					if (my_strcmp (coverSide, "양면") == 0) {
						euroform.leftBottomX = elem.object.pos.x;
						euroform.leftBottomY = elem.object.pos.y;
						euroform.leftBottomZ = elem.object.level;
						euroform.ang = elem.object.angle + DegreeToRad (180.0);
						moveIn3D ('x', elem.object.angle, unit_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						moveIn3D ('y', elem.object.angle, oppSideOffset, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						
						for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
							for (yy = 0 ; yy < num_A ; ++yy) {
								elemListBack.Push (placeSteelform (euroform));
								moveIn3D ('x', elem.object.angle, unit_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
							}
							moveIn3D ('x', elem.object.angle, -unit_A * num_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
							moveIn3D ('z', elem.object.angle, unit_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						}
					}
				} else if (my_strcmp (dir, "벽눕히기") == 0) {
					euroform.leftBottomX = elem.object.pos.x;
					euroform.leftBottomY = elem.object.pos.y;
					euroform.leftBottomZ = elem.object.level;
					moveIn3D ('x', elem.object.angle, unit_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
					euroform.u_ins_wall = false;				// true: 벽세우기, false: 벽눕히기
					euroform.ang_x = DegreeToRad (90.0);		// 벽(90)

					for (xx = 0 ; xx < num_A ; ++xx) {
						for (yy = 0 ; yy < num_ZZYZX ; ++yy) {
							elemList.Push (placeSteelform (euroform));
							moveIn3D ('x', elem.object.angle, unit_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						}
						moveIn3D ('x', elem.object.angle, -unit_ZZYZX * num_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						moveIn3D ('z', elem.object.angle, unit_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
					}

					// 양면
					if (my_strcmp (coverSide, "양면") == 0) {
						euroform.leftBottomX = elem.object.pos.x;
						euroform.leftBottomY = elem.object.pos.y;
						euroform.leftBottomZ = elem.object.level;
						euroform.ang = elem.object.angle + DegreeToRad (180.0);
						moveIn3D ('y', elem.object.angle, oppSideOffset, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						
						for (xx = 0 ; xx < num_A ; ++xx) {
							for (yy = 0 ; yy < num_ZZYZX ; ++yy) {
								elemListBack.Push (placeSteelform (euroform));
								moveIn3D ('x', elem.object.angle, unit_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
							}
							moveIn3D ('x', elem.object.angle, -unit_ZZYZX * num_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
							moveIn3D ('z', elem.object.angle, unit_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						}
					}
				} else if (my_strcmp (dir, "바닥깔기") == 0) {
					euroform.leftBottomX = elem.object.pos.x;
					euroform.leftBottomY = elem.object.pos.y;
					euroform.leftBottomZ = elem.object.level;
					moveIn3D ('y', elem.object.angle, unit_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
					euroform.u_ins_wall = true;					// true: 벽세우기, false: 벽눕히기
					euroform.ang_x = DegreeToRad (0.0);			// 천장(0)
					
					for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
						for (yy = 0 ; yy < num_A ; ++yy) {
							elemList.Push (placeSteelform (euroform));
							moveIn3D ('x', elem.object.angle, unit_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						}
						moveIn3D ('x', elem.object.angle, -unit_A * num_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						moveIn3D ('y', elem.object.angle, unit_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
					}
				} else if (my_strcmp (dir, "바닥덮기") == 0) {
					euroform.leftBottomX = elem.object.pos.x;
					euroform.leftBottomY = elem.object.pos.y;
					euroform.leftBottomZ = elem.object.level;
					euroform.u_ins_wall = true;					// true: 벽세우기, false: 벽눕히기
					euroform.ang_x = DegreeToRad (180.0);		// 바닥(180)

					for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
						for (yy = 0 ; yy < num_A ; ++yy) {
							elemList.Push (placeSteelform (euroform));
							moveIn3D ('x', elem.object.angle, unit_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						}
						moveIn3D ('x', elem.object.angle, -unit_A * num_A, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
						moveIn3D ('y', elem.object.angle, unit_ZZYZX, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
					}
				}

				// 그룹화하기
				if (!elemList.IsEmpty ()) {
					GSSize nElems = elemList.GetSize ();
					API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
					if (elemHead != NULL) {
						for (GSIndex i = 0; i < nElems; i++)
							(*elemHead)[i].guid = elemList[i];

						ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

						BMKillHandle ((GSHandle *) &elemHead);
					}
				}
				elemList.Clear (false);

				// 그룹화하기
				if (!elemListBack.IsEmpty ()) {
					GSSize nElems = elemListBack.GetSize ();
					API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
					if (elemHead != NULL) {
						for (GSIndex i = 0; i < nElems; i++)
							(*elemHead)[i].guid = elemListBack[i];

						ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

						BMKillHandle ((GSHandle *) &elemHead);
					}
				}
				elemListBack.Clear (false);
			} else if (my_strcmp (objType, "합판") == 0) {

				plywood.ang = elem.object.angle;
				plywood.leftBottomX = elem.object.pos.x;
				plywood.leftBottomY = elem.object.pos.y;
				plywood.leftBottomZ = elem.object.level;
				plywood.p_wid = unit_A;
				plywood.p_leng = unit_ZZYZX;

				if (my_strcmp (dir, "벽세우기") == 0) {
					plywood.w_dir = 1;

					for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
						for (yy = 0 ; yy < num_A ; ++yy) {
							elemList.Push (placePlywood (plywood));
							moveIn3D ('x', elem.object.angle, unit_A, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
						}
						moveIn3D ('x', elem.object.angle, -unit_A * num_A, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
						moveIn3D ('z', elem.object.angle, unit_ZZYZX, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
					}

					// 양면
					if (my_strcmp (coverSide, "양면") == 0) {
						plywood.leftBottomX = elem.object.pos.x;
						plywood.leftBottomY = elem.object.pos.y;
						plywood.leftBottomZ = elem.object.level;
						plywood.ang = elem.object.angle + DegreeToRad (180.0);
						moveIn3D ('x', elem.object.angle, unit_A, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
						moveIn3D ('y', elem.object.angle, oppSideOffset, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);

						for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
							for (yy = 0 ; yy < num_A ; ++yy) {
								elemListBack.Push (placePlywood (plywood));
								moveIn3D ('x', elem.object.angle, unit_A, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
							}
							moveIn3D ('x', elem.object.angle, -unit_A * num_A, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
							moveIn3D ('z', elem.object.angle, unit_ZZYZX, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
						}
					}
				} else if (my_strcmp (dir, "벽눕히기") == 0) {
					plywood.w_dir = 2;

					for (xx = 0 ; xx < num_A ; ++xx) {
						for (yy = 0 ; yy < num_ZZYZX ; ++yy) {
							elemList.Push (placePlywood (plywood));
							moveIn3D ('x', elem.object.angle, unit_ZZYZX, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
						}
						moveIn3D ('x', elem.object.angle, -unit_ZZYZX * num_ZZYZX, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
						moveIn3D ('z', elem.object.angle, unit_A, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
					}

					// 양면
					if (my_strcmp (coverSide, "양면") == 0) {
						plywood.leftBottomX = elem.object.pos.x;
						plywood.leftBottomY = elem.object.pos.y;
						plywood.leftBottomZ = elem.object.level;
						plywood.ang = elem.object.angle + DegreeToRad (180.0);
						moveIn3D ('x', elem.object.angle, unit_ZZYZX, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
						moveIn3D ('y', elem.object.angle, oppSideOffset, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);

						for (xx = 0 ; xx < num_A ; ++xx) {
							for (yy = 0 ; yy < num_ZZYZX ; ++yy) {
								elemListBack.Push (placePlywood (plywood));
								moveIn3D ('x', elem.object.angle, unit_ZZYZX, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
							}
							moveIn3D ('x', elem.object.angle, -unit_ZZYZX * num_ZZYZX, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
							moveIn3D ('z', elem.object.angle, unit_A, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
						}
					}
				} else if (my_strcmp (dir, "바닥깔기") == 0) {
					plywood.w_dir = 3;

					moveIn3D ('x', elem.object.angle, unit_A, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
					plywood.ang = elem.object.angle + DegreeToRad (90.0);

					for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
						for (yy = 0 ; yy < num_A ; ++yy) {
							elemList.Push (placePlywood (plywood));
							moveIn3D ('x', elem.object.angle, unit_A, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
						}
						moveIn3D ('x', elem.object.angle, -unit_A * num_A, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
						moveIn3D ('y', elem.object.angle, unit_ZZYZX, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
					}
				} else if (my_strcmp (dir, "바닥덮기") == 0) {
					plywood.w_dir = 4;

					moveIn3D ('x', elem.object.angle, unit_A, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
					plywood.ang = elem.object.angle + DegreeToRad (90.0);

					for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
						for (yy = 0 ; yy < num_A ; ++yy) {
							elemList.Push (placePlywood (plywood));
							moveIn3D ('x', elem.object.angle, unit_A, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
						}
						moveIn3D ('x', elem.object.angle, -unit_A * num_A, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
						moveIn3D ('y', elem.object.angle, unit_ZZYZX, &plywood.leftBottomX, &plywood.leftBottomY, &plywood.leftBottomZ);
					}
				}

				// 그룹화하기
				if (!elemList.IsEmpty ()) {
					GSSize nElems = elemList.GetSize ();
					API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
					if (elemHead != NULL) {
						for (GSIndex i = 0; i < nElems; i++)
							(*elemHead)[i].guid = elemList[i];

						ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

						BMKillHandle ((GSHandle *) &elemHead);
					}
				}
				elemList.Clear (false);

				// 그룹화하기
				if (!elemListBack.IsEmpty ()) {
					GSSize nElems = elemListBack.GetSize ();
					API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
					if (elemHead != NULL) {
						for (GSIndex i = 0; i < nElems; i++)
							(*elemHead)[i].guid = elemListBack[i];

						ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

						BMKillHandle ((GSHandle *) &elemHead);
					}
				}
				elemListBack.Clear (false);
			} else if (my_strcmp (objType, "휠러스페이서") == 0) {

				fillersp.ang = elem.object.angle;
				fillersp.leftBottomX = elem.object.pos.x;
				fillersp.leftBottomY = elem.object.pos.y;
				fillersp.leftBottomZ = elem.object.level;
				fillersp.f_thk = unit_A;
				fillersp.f_leng = unit_ZZYZX;

				if (my_strcmp (dir, "벽세우기") == 0) {
					fillersp.f_ang = DegreeToRad (90.0);
					fillersp.f_rota = DegreeToRad (0.0);
					moveIn3D ('x', elem.object.angle, unit_A, &fillersp.leftBottomX, &fillersp.leftBottomY, &fillersp.leftBottomZ);

					for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
						for (yy = 0 ; yy < num_A ; ++yy) {
							elemList.Push (placeFillersp (fillersp));
							moveIn3D ('x', elem.object.angle, unit_A, &fillersp.leftBottomX, &fillersp.leftBottomY, &fillersp.leftBottomZ);
						}
						moveIn3D ('x', elem.object.angle, -unit_A * num_A, &fillersp.leftBottomX, &fillersp.leftBottomY, &fillersp.leftBottomZ);
						moveIn3D ('z', elem.object.angle, unit_ZZYZX, &fillersp.leftBottomX, &fillersp.leftBottomY, &fillersp.leftBottomZ);
					}

					// 양면
					if (my_strcmp (coverSide, "양면") == 0) {
						fillersp.leftBottomX = elem.object.pos.x;
						fillersp.leftBottomY = elem.object.pos.y;
						fillersp.leftBottomZ = elem.object.level;
						fillersp.ang = elem.object.angle + DegreeToRad (180.0);
						moveIn3D ('y', elem.object.angle, oppSideOffset, &fillersp.leftBottomX, &fillersp.leftBottomY, &fillersp.leftBottomZ);

						for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
							for (yy = 0 ; yy < num_A ; ++yy) {
								elemListBack.Push (placeFillersp (fillersp));
								moveIn3D ('x', elem.object.angle, unit_A, &fillersp.leftBottomX, &fillersp.leftBottomY, &fillersp.leftBottomZ);
							}
							moveIn3D ('x', elem.object.angle, -unit_A * num_A, &fillersp.leftBottomX, &fillersp.leftBottomY, &fillersp.leftBottomZ);
							moveIn3D ('z', elem.object.angle, unit_ZZYZX, &fillersp.leftBottomX, &fillersp.leftBottomY, &fillersp.leftBottomZ);
						}
					}
				} else if (my_strcmp (dir, "벽눕히기") == 0) {
					fillersp.f_ang = DegreeToRad (0.0);
					fillersp.f_rota = DegreeToRad (0.0);

					for (xx = 0 ; xx < num_A ; ++xx) {
						for (yy = 0 ; yy < num_ZZYZX ; ++yy) {
							elemList.Push (placeFillersp (fillersp));
							moveIn3D ('x', elem.object.angle, unit_ZZYZX, &fillersp.leftBottomX, &fillersp.leftBottomY, &fillersp.leftBottomZ);
						}
						moveIn3D ('x', elem.object.angle, -unit_ZZYZX * num_ZZYZX, &fillersp.leftBottomX, &fillersp.leftBottomY, &fillersp.leftBottomZ);
						moveIn3D ('z', elem.object.angle, unit_A, &fillersp.leftBottomX, &fillersp.leftBottomY, &fillersp.leftBottomZ);
					}

					// 양면
					if (my_strcmp (coverSide, "양면") == 0) {
						fillersp.leftBottomX = elem.object.pos.x;
						fillersp.leftBottomY = elem.object.pos.y;
						fillersp.leftBottomZ = elem.object.level;
						fillersp.ang = elem.object.angle + DegreeToRad (180.0);
						moveIn3D ('x', elem.object.angle, unit_ZZYZX, &fillersp.leftBottomX, &fillersp.leftBottomY, &fillersp.leftBottomZ);
						moveIn3D ('y', elem.object.angle, oppSideOffset, &fillersp.leftBottomX, &fillersp.leftBottomY, &fillersp.leftBottomZ);

						for (xx = 0 ; xx < num_A ; ++xx) {
							for (yy = 0 ; yy < num_ZZYZX ; ++yy) {
								elemListBack.Push (placeFillersp (fillersp));
								moveIn3D ('x', elem.object.angle, unit_ZZYZX, &fillersp.leftBottomX, &fillersp.leftBottomY, &fillersp.leftBottomZ);
							}
							moveIn3D ('x', elem.object.angle, -unit_ZZYZX * num_ZZYZX, &fillersp.leftBottomX, &fillersp.leftBottomY, &fillersp.leftBottomZ);
							moveIn3D ('z', elem.object.angle, unit_A, &fillersp.leftBottomX, &fillersp.leftBottomY, &fillersp.leftBottomZ);
						}
					}
				}

				// 그룹화하기
				if (!elemList.IsEmpty ()) {
					GSSize nElems = elemList.GetSize ();
					API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
					if (elemHead != NULL) {
						for (GSIndex i = 0; i < nElems; i++)
							(*elemHead)[i].guid = elemList[i];

						ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

						BMKillHandle ((GSHandle *) &elemHead);
					}
				}
				elemList.Clear (false);

				// 그룹화하기
				if (!elemListBack.IsEmpty ()) {
					GSSize nElems = elemListBack.GetSize ();
					API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
					if (elemHead != NULL) {
						for (GSIndex i = 0; i < nElems; i++)
							(*elemHead)[i].guid = elemListBack[i];

						ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

						BMKillHandle ((GSHandle *) &elemHead);
					}
				}
				elemListBack.Clear (false);
			} else if (my_strcmp (objType, "아웃코너앵글") == 0) {

				outcornerAngle.ang = elem.object.angle;
				outcornerAngle.leftBottomX = elem.object.pos.x;
				outcornerAngle.leftBottomY = elem.object.pos.y;
				outcornerAngle.leftBottomZ = elem.object.level;
				outcornerAngle.a_leng = unit_ZZYZX;
				
				if (leftSide == true) {
					if (my_strcmp (dir, "벽세우기") == 0) {
						moveIn3D ('x', outcornerAngle.ang, unit_A, &outcornerAngle.leftBottomX, &outcornerAngle.leftBottomY, &outcornerAngle.leftBottomZ);
						outcornerAngle.a_ang = DegreeToRad (90.0);

						for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
							outcornerAngle.ang = elem.object.angle + DegreeToRad (180.0);
							elemList.Push (placeOutcornerAngle (outcornerAngle));
							outcornerAngle.ang = elem.object.angle;
							moveIn3D ('z', elem.object.angle, unit_ZZYZX, &outcornerAngle.leftBottomX, &outcornerAngle.leftBottomY, &outcornerAngle.leftBottomZ);
						}

						// 양면
						if (my_strcmp (coverSide, "양면") == 0) {
							outcornerAngle.leftBottomX = elem.object.pos.x;
							outcornerAngle.leftBottomY = elem.object.pos.y;
							outcornerAngle.leftBottomZ = elem.object.level;
							outcornerAngle.a_ang = DegreeToRad (90.0);
							moveIn3D ('x', outcornerAngle.ang, unit_A, &outcornerAngle.leftBottomX, &outcornerAngle.leftBottomY, &outcornerAngle.leftBottomZ);
							moveIn3D ('y', elem.object.angle, oppSideOffset, &outcornerAngle.leftBottomX, &outcornerAngle.leftBottomY, &outcornerAngle.leftBottomZ);

							for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
								outcornerAngle.ang = elem.object.angle + DegreeToRad (90.0);
								elemListBack.Push (placeOutcornerAngle (outcornerAngle));
								outcornerAngle.ang = elem.object.angle;
								moveIn3D ('z', elem.object.angle, unit_ZZYZX, &outcornerAngle.leftBottomX, &outcornerAngle.leftBottomY, &outcornerAngle.leftBottomZ);
							}
						}
					} else if (my_strcmp (dir, "바닥깔기") == 0) {
						moveIn3D ('x', outcornerAngle.ang, unit_A, &outcornerAngle.leftBottomX, &outcornerAngle.leftBottomY, &outcornerAngle.leftBottomZ);
						outcornerAngle.a_ang = DegreeToRad (0.0);

						for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
							outcornerAngle.ang = elem.object.angle + DegreeToRad (90.0);
							elemList.Push (placeOutcornerAngle (outcornerAngle));
							outcornerAngle.ang = elem.object.angle;
							moveIn3D ('y', elem.object.angle, unit_ZZYZX, &outcornerAngle.leftBottomX, &outcornerAngle.leftBottomY, &outcornerAngle.leftBottomZ);
						}
					} else if (my_strcmp (dir, "바닥덮기") == 0) {
						moveIn3D ('x', elem.object.angle, unit_A, &outcornerAngle.leftBottomX, &outcornerAngle.leftBottomY, &outcornerAngle.leftBottomZ);
						moveIn3D ('y', elem.object.angle, unit_ZZYZX, &outcornerAngle.leftBottomX, &outcornerAngle.leftBottomY, &outcornerAngle.leftBottomZ);
						outcornerAngle.a_ang = DegreeToRad (180.0);

						for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
							outcornerAngle.ang = elem.object.angle + DegreeToRad (90.0);
							elemList.Push (placeOutcornerAngle (outcornerAngle));
							outcornerAngle.ang = elem.object.angle;
							moveIn3D ('y', elem.object.angle, unit_ZZYZX, &outcornerAngle.leftBottomX, &outcornerAngle.leftBottomY, &outcornerAngle.leftBottomZ);
						}
					}
				} else {
					if (my_strcmp (dir, "벽세우기") == 0) {
						outcornerAngle.a_ang = DegreeToRad (90.0);

						for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
							outcornerAngle.ang = elem.object.angle + DegreeToRad (270.0);
							elemList.Push (placeOutcornerAngle (outcornerAngle));
							outcornerAngle.ang = elem.object.angle;
							moveIn3D ('z', elem.object.angle, unit_ZZYZX, &outcornerAngle.leftBottomX, &outcornerAngle.leftBottomY, &outcornerAngle.leftBottomZ);
						}

						// 양면
						if (my_strcmp (coverSide, "양면") == 0) {
							outcornerAngle.leftBottomX = elem.object.pos.x;
							outcornerAngle.leftBottomY = elem.object.pos.y;
							outcornerAngle.leftBottomZ = elem.object.level;
							outcornerAngle.a_ang = DegreeToRad (90.0);
							moveIn3D ('y', elem.object.angle, oppSideOffset, &outcornerAngle.leftBottomX, &outcornerAngle.leftBottomY, &outcornerAngle.leftBottomZ);

							for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
								outcornerAngle.ang = elem.object.angle;
								elemListBack.Push (placeOutcornerAngle (outcornerAngle));
								moveIn3D ('z', elem.object.angle, unit_ZZYZX, &outcornerAngle.leftBottomX, &outcornerAngle.leftBottomY, &outcornerAngle.leftBottomZ);
							}
						}
					} else if (my_strcmp (dir, "바닥깔기") == 0) {
						moveIn3D ('y', outcornerAngle.ang, unit_ZZYZX, &outcornerAngle.leftBottomX, &outcornerAngle.leftBottomY, &outcornerAngle.leftBottomZ);
						outcornerAngle.a_ang = DegreeToRad (0.0);

						for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
							outcornerAngle.ang = elem.object.angle + DegreeToRad (270.0);
							elemList.Push (placeOutcornerAngle (outcornerAngle));
							outcornerAngle.ang = elem.object.angle;
							moveIn3D ('y', elem.object.angle, unit_ZZYZX, &outcornerAngle.leftBottomX, &outcornerAngle.leftBottomY, &outcornerAngle.leftBottomZ);
						}
					} else if (my_strcmp (dir, "바닥덮기") == 0) {
						outcornerAngle.a_ang = DegreeToRad (180.0);

						for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
							outcornerAngle.ang = elem.object.angle + DegreeToRad (270.0);
							elemList.Push (placeOutcornerAngle (outcornerAngle));
							outcornerAngle.ang = elem.object.angle;
							moveIn3D ('y', elem.object.angle, unit_ZZYZX, &outcornerAngle.leftBottomX, &outcornerAngle.leftBottomY, &outcornerAngle.leftBottomZ);
						}
					}
				}

				// 그룹화하기
				if (!elemList.IsEmpty ()) {
					GSSize nElems = elemList.GetSize ();
					API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
					if (elemHead != NULL) {
						for (GSIndex i = 0; i < nElems; i++)
							(*elemHead)[i].guid = elemList[i];

						ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

						BMKillHandle ((GSHandle *) &elemHead);
					}
				}
				elemList.Clear (false);

				// 그룹화하기
				if (!elemListBack.IsEmpty ()) {
					GSSize nElems = elemListBack.GetSize ();
					API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
					if (elemHead != NULL) {
						for (GSIndex i = 0; i < nElems; i++)
							(*elemHead)[i].guid = elemListBack[i];

						ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

						BMKillHandle ((GSHandle *) &elemHead);
					}
				}
				elemListBack.Clear (false);
			} else if (my_strcmp (objType, "아웃코너판넬") == 0) {

				outcornerPanel.ang = elem.object.angle;
				outcornerPanel.leftBottomX = elem.object.pos.x;
				outcornerPanel.leftBottomY = elem.object.pos.y;
				outcornerPanel.leftBottomZ = elem.object.level;

				if (leftSide == true) {
					if (my_strcmp (dir, "벽세우기") == 0) {
						outcornerPanel.wid_s = unit_A;
						outcornerPanel.leng_s = unit_B;
						outcornerPanel.hei_s = unit_ZZYZX;

						for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
							elemList.Push (placeOutcornerPanel (outcornerPanel));
							moveIn3D ('z', elem.object.angle, unit_ZZYZX, &outcornerPanel.leftBottomX, &outcornerPanel.leftBottomY, &outcornerPanel.leftBottomZ);
						}

						// 양면
						if (my_strcmp (coverSide, "양면") == 0) {
							outcornerPanel.leftBottomX = elem.object.pos.x;
							outcornerPanel.leftBottomY = elem.object.pos.y;
							outcornerPanel.leftBottomZ = elem.object.level;

							outcornerPanel.wid_s = unit_B;
							outcornerPanel.leng_s = unit_A;
							outcornerPanel.hei_s = unit_ZZYZX;

							moveIn3D ('y', elem.object.angle, oppSideOffset, &outcornerPanel.leftBottomX, &outcornerPanel.leftBottomY, &outcornerPanel.leftBottomZ);

							for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
								outcornerPanel.ang = elem.object.angle - DegreeToRad (90.0);
								elemListBack.Push (placeOutcornerPanel (outcornerPanel));
								outcornerPanel.ang = elem.object.angle;
								moveIn3D ('z', elem.object.angle, unit_ZZYZX, &outcornerPanel.leftBottomX, &outcornerPanel.leftBottomY, &outcornerPanel.leftBottomZ);
							}
						}
					}
				} else {
					if (my_strcmp (dir, "벽세우기") == 0) {
						outcornerPanel.wid_s = unit_B;
						outcornerPanel.leng_s = unit_A;
						outcornerPanel.hei_s = unit_ZZYZX;

						for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
							outcornerPanel.ang = elem.object.angle + DegreeToRad (90.0);
							elemList.Push (placeOutcornerPanel (outcornerPanel));
							outcornerPanel.ang = elem.object.angle;
							moveIn3D ('z', elem.object.angle, unit_ZZYZX, &outcornerPanel.leftBottomX, &outcornerPanel.leftBottomY, &outcornerPanel.leftBottomZ);
						}

						// 양면
						if (my_strcmp (coverSide, "양면") == 0) {
							outcornerPanel.leftBottomX = elem.object.pos.x;
							outcornerPanel.leftBottomY = elem.object.pos.y;
							outcornerPanel.leftBottomZ = elem.object.level;

							outcornerPanel.wid_s = unit_A;
							outcornerPanel.leng_s = unit_B;
							outcornerPanel.hei_s = unit_ZZYZX;

							moveIn3D ('y', elem.object.angle, oppSideOffset, &outcornerPanel.leftBottomX, &outcornerPanel.leftBottomY, &outcornerPanel.leftBottomZ);

							for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
								outcornerPanel.ang = elem.object.angle + DegreeToRad (180.0);
								elemListBack.Push (placeOutcornerPanel (outcornerPanel));
								outcornerPanel.ang = elem.object.angle;
								moveIn3D ('z', elem.object.angle, unit_ZZYZX, &outcornerPanel.leftBottomX, &outcornerPanel.leftBottomY, &outcornerPanel.leftBottomZ);
							}
						}
					}
				}

				// 그룹화하기
				if (!elemList.IsEmpty ()) {
					GSSize nElems = elemList.GetSize ();
					API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
					if (elemHead != NULL) {
						for (GSIndex i = 0; i < nElems; i++)
							(*elemHead)[i].guid = elemList[i];

						ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

						BMKillHandle ((GSHandle *) &elemHead);
					}
				}
				elemList.Clear (false);

				// 그룹화하기
				if (!elemListBack.IsEmpty ()) {
					GSSize nElems = elemListBack.GetSize ();
					API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
					if (elemHead != NULL) {
						for (GSIndex i = 0; i < nElems; i++)
							(*elemHead)[i].guid = elemListBack[i];

						ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

						BMKillHandle ((GSHandle *) &elemHead);
					}
				}
				elemListBack.Clear (false);
			} else if (my_strcmp (objType, "인코너판넬") == 0) {

				incornerPanel.ang = elem.object.angle;
				incornerPanel.leftBottomX = elem.object.pos.x;
				incornerPanel.leftBottomY = elem.object.pos.y;
				incornerPanel.leftBottomZ = elem.object.level;

				if (leftSide == true) {
					if (my_strcmp (dir, "벽세우기") == 0) {
						incornerPanel.wid_s = unit_B;
						incornerPanel.leng_s = unit_A;
						incornerPanel.hei_s = unit_ZZYZX;

						for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
							incornerPanel.ang = elem.object.angle + DegreeToRad (270.0);
							elemList.Push (placeIncornerPanel (incornerPanel));
							incornerPanel.ang = elem.object.angle;
							moveIn3D ('z', elem.object.angle, unit_ZZYZX, &incornerPanel.leftBottomX, &incornerPanel.leftBottomY, &incornerPanel.leftBottomZ);
						}

						// 양면
						if (my_strcmp (coverSide, "양면") == 0) {
							incornerPanel.leftBottomX = elem.object.pos.x;
							incornerPanel.leftBottomY = elem.object.pos.y;
							incornerPanel.leftBottomZ = elem.object.level;

							incornerPanel.wid_s = unit_A;
							incornerPanel.leng_s = unit_B;
							incornerPanel.hei_s = unit_ZZYZX;

							moveIn3D ('y', elem.object.angle, oppSideOffset, &incornerPanel.leftBottomX, &incornerPanel.leftBottomY, &incornerPanel.leftBottomZ);

							for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
								incornerPanel.ang = elem.object.angle;
								elemListBack.Push (placeIncornerPanel (incornerPanel));
								moveIn3D ('z', elem.object.angle, unit_ZZYZX, &incornerPanel.leftBottomX, &incornerPanel.leftBottomY, &incornerPanel.leftBottomZ);
							}
						}
					}
				} else {
					if (my_strcmp (dir, "벽세우기") == 0) {
						incornerPanel.wid_s = unit_A;
						incornerPanel.leng_s = unit_B;
						incornerPanel.hei_s = unit_ZZYZX;

						for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
							incornerPanel.ang = elem.object.angle + DegreeToRad (180.0);
							elemList.Push (placeIncornerPanel (incornerPanel));
							incornerPanel.ang = elem.object.angle;
							moveIn3D ('z', elem.object.angle, unit_ZZYZX, &incornerPanel.leftBottomX, &incornerPanel.leftBottomY, &incornerPanel.leftBottomZ);
						}

						// 양면
						if (my_strcmp (coverSide, "양면") == 0) {
							incornerPanel.leftBottomX = elem.object.pos.x;
							incornerPanel.leftBottomY = elem.object.pos.y;
							incornerPanel.leftBottomZ = elem.object.level;

							incornerPanel.wid_s = unit_B;
							incornerPanel.leng_s = unit_A;
							incornerPanel.hei_s = unit_ZZYZX;

							moveIn3D ('y', elem.object.angle, oppSideOffset, &incornerPanel.leftBottomX, &incornerPanel.leftBottomY, &incornerPanel.leftBottomZ);

							for (xx = 0 ; xx < num_ZZYZX ; ++xx) {
								incornerPanel.ang = elem.object.angle + DegreeToRad (90.0);
								elemListBack.Push (placeIncornerPanel (incornerPanel));
								incornerPanel.ang = elem.object.angle;
								moveIn3D ('z', elem.object.angle, unit_ZZYZX, &incornerPanel.leftBottomX, &incornerPanel.leftBottomY, &incornerPanel.leftBottomZ);
							}
						}
					}
				}

				// 그룹화하기
				if (!elemList.IsEmpty ()) {
					GSSize nElems = elemList.GetSize ();
					API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
					if (elemHead != NULL) {
						for (GSIndex i = 0; i < nElems; i++)
							(*elemHead)[i].guid = elemList[i];

						ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

						BMKillHandle ((GSHandle *) &elemHead);
					}
				}
				elemList.Clear (false);

				// 그룹화하기
				if (!elemListBack.IsEmpty ()) {
					GSSize nElems = elemListBack.GetSize ();
					API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
					if (elemHead != NULL) {
						for (GSIndex i = 0; i < nElems; i++)
							(*elemHead)[i].guid = elemListBack[i];

						ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

						BMKillHandle ((GSHandle *) &elemHead);
					}
				}
				elemListBack.Clear (false);
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

// 테이블폼(벽) 배치 (벽세우기) 타입1
GSErrCode	placeTableformOnWall_portrait_Type1 (WallTableform params)
{
	GSErrCode	err = NoError;

	short	nHorEuroform;			// 수평 방향 유로폼 개수
	short	nVerEuroform;			// 수직 방향 유로폼 개수
	double	width [7];				// 수평 방향 각 유로폼 너비
	double	height [7];				// 수직 방향 각 유로폼 높이

	short		xx, yy;
	double		width_t, height_t;
	double		elev_headpiece;
	double		horizontalGap = 0.050;	// 수평재 양쪽 이격거리

	Euroform					euroform;
	SquarePipe					sqrPipe;
	PinBoltSet					pinbolt;
	//WallTie						walltie;
	HeadpieceOfPushPullProps	headpiece;
	MetalFittingsWithRectWasher	fittings;

	nHorEuroform = 0;
	nVerEuroform = 0;
	for (xx = 0 ; xx < 7 ; ++xx) {
		width [xx] = 0.0;
		height [xx] = 0.0;
	}

	if (abs (params.width - 2.300) < EPS) {
		nHorEuroform = 4;
		width [0] = 0.600;	width [1] = 0.600;	width [2] = 0.500;	width [3] = 0.600;
		horizontalGap = 0.050;
	} else if (abs (params.width - 2.250) < EPS) {
		nHorEuroform = 4;
		width [0] = 0.600;	width [1] = 0.600;	width [2] = 0.450;	width [3] = 0.600;
		horizontalGap = 0.025;
	} else if (abs (params.width - 2.200) < EPS) {
		nHorEuroform = 4;
		width [0] = 0.600;	width [1] = 0.600;	width [2] = 0.400;	width [3] = 0.600;
		horizontalGap = 0.050;
	} else if (abs (params.width - 2.150) < EPS) {
		nHorEuroform = 4;
		width [0] = 0.600;	width [1] = 0.500;	width [2] = 0.450;	width [3] = 0.600;
		horizontalGap = 0.025;
	} else if (abs (params.width - 2.100) < EPS) {
		nHorEuroform = 4;
		width [0] = 0.600;	width [1] = 0.600;	width [2] = 0.300;	width [3] = 0.600;
		horizontalGap = 0.050;
	} else if (abs (params.width - 2.050) < EPS) {
		nHorEuroform = 4;
		width [0] = 0.600;	width [1] = 0.450;	width [2] = 0.400;	width [3] = 0.600;
		horizontalGap = 0.025;
	} else if (abs (params.width - 2.000) < EPS) {
		nHorEuroform = 4;
		width [0] = 0.600;	width [1] = 0.600;	width [2] = 0.200;	width [3] = 0.600;
		horizontalGap = 0.050;
	} else if (abs (params.width - 1.950) < EPS) {
		nHorEuroform = 4;
		width [0] = 0.600;	width [1] = 0.450;	width [2] = 0.300;	width [3] = 0.600;
		horizontalGap = 0.025;
	} else if (abs (params.width - 1.900) < EPS) {
		nHorEuroform = 4;
		width [0] = 0.600;	width [1] = 0.500;	width [2] = 0.200;	width [3] = 0.600;
		horizontalGap = 0.050;
	} else if (abs (params.width - 1.850) < EPS) {
		nHorEuroform = 4;
		width [0] = 0.600;	width [1] = 0.450;	width [2] = 0.200;	width [3] = 0.600;
		horizontalGap = 0.025;
	} else if (abs (params.width - 1.800) < EPS) {
		nHorEuroform = 3;
		width [0] = 0.600;	width [1] = 0.600;	width [2] = 0.600;	width [3] = 0.0;
		horizontalGap = 0.050;
	} else if (abs (params.width - 1.750) < EPS) {
		nHorEuroform = 4;
		width [0] = 0.600;	width [1] = 0.200;	width [2] = 0.450;	width [3] = 0.500;
		horizontalGap = 0.025;
	} else if (abs (params.width - 1.700) < EPS) {
		nHorEuroform = 3;
		width [0] = 0.600;	width [1] = 0.500;	width [2] = 0.600;	width [3] = 0.0;
		horizontalGap = 0.050;
	} else if (abs (params.width - 1.650) < EPS) {
		nHorEuroform = 3;
		width [0] = 0.600;	width [1] = 0.450;	width [2] = 0.600;	width [3] = 0.0;
		horizontalGap = 0.025;
	} else if (abs (params.width - 1.600) < EPS) {
		nHorEuroform = 3;
		width [0] = 0.600;	width [1] = 0.400;	width [2] = 0.600;	width [3] = 0.0;
		horizontalGap = 0.050;
	} else if (abs (params.width - 1.550) < EPS) {
		nHorEuroform = 3;
		width [0] = 0.600;	width [1] = 0.450;	width [2] = 0.500;	width [3] = 0.0;
		horizontalGap = 0.025;
	} else if (abs (params.width - 1.500) < EPS) {
		nHorEuroform = 3;
		width [0] = 0.600;	width [1] = 0.300;	width [2] = 0.600;	width [3] = 0.0;
		horizontalGap = 0.050;
	} else if (abs (params.width - 1.450) < EPS) {
		nHorEuroform = 3;
		width [0] = 0.500;	width [1] = 0.450;	width [2] = 0.500;	width [3] = 0.0;
		horizontalGap = 0.025;
	} else if (abs (params.width - 1.400) < EPS) {
		nHorEuroform = 3;
		width [0] = 0.500;	width [1] = 0.400;	width [2] = 0.500;	width [3] = 0.0;
		horizontalGap = 0.050;
	} else if (abs (params.width - 1.350) < EPS) {
		nHorEuroform = 3;
		width [0] = 0.600;	width [1] = 0.300;	width [2] = 0.450;	width [3] = 0.0;
		horizontalGap = 0.025;
	} else if (abs (params.width - 1.300) < EPS) {
		nHorEuroform = 3;
		width [0] = 0.600;	width [1] = 0.200;	width [2] = 0.500;	width [3] = 0.0;
		horizontalGap = 0.050;
	} else if (abs (params.width - 1.250) < EPS) {
		nHorEuroform = 3;
		width [0] = 0.600;	width [1] = 0.200;	width [2] = 0.450;	width [3] = 0.0;
		horizontalGap = 0.025;
	} else if (abs (params.width - 1.200) < EPS) {
		nHorEuroform = 2;
		width [0] = 0.600;	width [1] = 0.600;	width [2] = 0.0;	width [3] = 0.0;
		horizontalGap = 0.050;
	} else if (abs (params.width - 1.150) < EPS) {
		nHorEuroform = 3;
		width [0] = 0.450;	width [1] = 0.300;	width [2] = 0.400;	width [3] = 0.0;
		horizontalGap = 0.025;
	} else if (abs (params.width - 1.100) < EPS) {
		nHorEuroform = 3;
		width [0] = 0.400;	width [1] = 0.300;	width [2] = 0.400;	width [3] = 0.0;
		horizontalGap = 0.050;
	} else if (abs (params.width - 1.050) < EPS) {
		nHorEuroform = 3;
		width [0] = 0.450;	width [1] = 0.300;	width [2] = 0.300;	width [3] = 0.0;
		horizontalGap = 0.025;
	} else if (abs (params.width - 1.000) < EPS) {
		nHorEuroform = 2;
		width [0] = 0.600;	width [1] = 0.400;	width [2] = 0.0;	width [3] = 0.0;
		horizontalGap = 0.050;
	} else if (abs (params.width - 0.950) < EPS) {
		nHorEuroform = 2;
		width [0] = 0.450;	width [1] = 0.500;	width [2] = 0.0;	width [3] = 0.0;
		horizontalGap = 0.025;
	} else if (abs (params.width - 0.900) < EPS) {
		nHorEuroform = 2;
		width [0] = 0.600;	width [1] = 0.300;	width [2] = 0.0;	width [3] = 0.0;
		horizontalGap = 0.050;
	} else if (abs (params.width - 0.850) < EPS) {
		nHorEuroform = 2;
		width [0] = 0.400;	width [1] = 0.450;	width [2] = 0.0;	width [3] = 0.0;
		horizontalGap = 0.025;
	} else if (abs (params.width - 0.800) < EPS) {
		nHorEuroform = 2;
		width [0] = 0.400;	width [1] = 0.400;	width [2] = 0.0;	width [3] = 0.0;
		horizontalGap = 0.050;
	} else if (abs (params.width - 0.750) < EPS) {
		nHorEuroform = 2;
		width [0] = 0.450;	width [1] = 0.300;	width [2] = 0.0;	width [3] = 0.0;
		horizontalGap = 0.025;
	} else if (abs (params.width - 0.700) < EPS) {
		nHorEuroform = 2;
		width [0] = 0.400;	width [1] = 0.300;	width [2] = 0.0;	width [3] = 0.0;
		horizontalGap = 0.050;
	} else if (abs (params.width - 0.650) < EPS) {
		nHorEuroform = 2;
		width [0] = 0.450;	width [1] = 0.200;	width [2] = 0.0;	width [3] = 0.0;
		horizontalGap = 0.025;
	} else if (abs (params.width - 0.600) < EPS) {
		nHorEuroform = 1;
		width [0] = 0.600;	width [1] = 0.0;	width [2] = 0.0;	width [3] = 0.0;
		horizontalGap = 0.050;
	} else if (abs (params.width - 0.500) < EPS) {
		nHorEuroform = 1;
		width [0] = 0.500;	width [1] = 0.0;	width [2] = 0.0;	width [3] = 0.0;
		horizontalGap = 0.050;
	} else if (abs (params.width - 0.450) < EPS) {
		nHorEuroform = 1;
		width [0] = 0.450;	width [1] = 0.0;	width [2] = 0.0;	width [3] = 0.0;
		horizontalGap = 0.025;
	} else if (abs (params.width - 0.400) < EPS) {
		nHorEuroform = 1;
		width [0] = 0.400;	width [1] = 0.0;	width [2] = 0.0;	width [3] = 0.0;
		horizontalGap = 0.050;
	} else {
		nHorEuroform = 0;
		width [0] = 0.0;	width [1] = 0.0;	width [2] = 0.0;	width [3] = 0.0;
	}

	if (abs (params.height - 6.000) < EPS) {
		nVerEuroform = 5;
		height [0] = 1.200;
		height [1] = 1.200;
		height [2] = 1.200;
		height [3] = 1.200;
		height [4] = 1.200;
	} else if (abs (params.height - 5.700) < EPS) {
		nVerEuroform = 5;
		height [0] = 1.200;
		height [1] = 1.200;
		height [2] = 1.200;
		height [3] = 1.200;
		height [4] = 0.900;
	} else if (abs (params.height - 5.400) < EPS) {
		nVerEuroform = 5;
		height [0] = 1.200;
		height [1] = 1.200;
		height [2] = 1.200;
		height [3] = 0.900;
		height [4] = 0.900;
	} else if (abs (params.height - 5.100) < EPS) {
		nVerEuroform = 5;
		height [0] = 1.200;
		height [1] = 1.200;
		height [2] = 1.200;
		height [3] = 0.900;
		height [4] = 0.600;
	} else if (abs (params.height - 4.800) < EPS) {
		nVerEuroform = 4;
		height [0] = 1.200;
		height [1] = 1.200;
		height [2] = 1.200;
		height [3] = 1.200;
		height [4] = 0.0;
	} else if (abs (params.height - 4.500) < EPS) {
		nVerEuroform = 4;
		height [0] = 1.200;
		height [1] = 1.200;
		height [2] = 1.200;
		height [3] = 0.900;
		height [4] = 0.0;
	} else if (abs (params.height - 4.200) < EPS) {
		nVerEuroform = 4;
		height [0] = 1.200;
		height [1] = 1.200;
		height [2] = 0.900;
		height [3] = 0.900;
		height [4] = 0.0;
	} else if (abs (params.height - 3.900) < EPS) {
		nVerEuroform = 4;
		height [0] = 1.200;
		height [1] = 1.200;
		height [2] = 0.900;
		height [3] = 0.600;
		height [4] = 0.0;
	} else if (abs (params.height - 3.600) < EPS) {
		nVerEuroform = 3;
		height [0] = 1.200;
		height [1] = 1.200;
		height [2] = 1.200;
		height [3] = 0.0;
		height [4] = 0.0;
	} else if (abs (params.height - 3.300) < EPS) {
		nVerEuroform = 3;
		height [0] = 1.200;
		height [1] = 1.200;
		height [2] = 0.900;
		height [3] = 0.0;
		height [4] = 0.0;
	} else if (abs (params.height - 3.000) < EPS) {
		nVerEuroform = 3;
		height [0] = 1.200;
		height [1] = 1.200;
		height [2] = 0.600;
		height [3] = 0.0;
		height [4] = 0.0;
	} else if (abs (params.height - 2.700) < EPS) {
		nVerEuroform = 3;
		height [0] = 1.200;
		height [1] = 0.900;
		height [2] = 0.600;
		height [3] = 0.0;
		height [4] = 0.0;
	} else if (abs (params.height - 2.400) < EPS) {
		nVerEuroform = 2;
		height [0] = 1.200;
		height [1] = 1.200;
		height [2] = 0.0;
		height [3] = 0.0;
		height [4] = 0.0;
	} else if (abs (params.height - 2.100) < EPS) {
		nVerEuroform = 2;
		height [0] = 1.200;
		height [1] = 0.900;
		height [2] = 0.0;
		height [3] = 0.0;
		height [4] = 0.0;
	} else if (abs (params.height - 1.800) < EPS) {
		nVerEuroform = 2;
		height [0] = 0.900;
		height [1] = 0.900;
		height [2] = 0.0;
		height [3] = 0.0;
		height [4] = 0.0;
	} else if (abs (params.height - 1.500) < EPS) {
		nVerEuroform = 2;
		height [0] = 0.900;
		height [1] = 0.600;
		height [2] = 0.0;
		height [3] = 0.0;
		height [4] = 0.0;
	} else {
		nVerEuroform = 0;
		height [0] = 0.0;
		height [1] = 0.0;
		height [2] = 0.0;
		height [3] = 0.0;
		height [4] = 0.0;
	}

	// 너비나 높이가 0이면 아무것도 배치하지 않음
	if ((nHorEuroform == 0) || (nVerEuroform == 0))
		return	NoError;

	//////////////////////////////////////////////////////////////// 현재면
	// 유로폼 설치
	euroform.leftBottomX = params.leftBottomX;
	euroform.leftBottomY = params.leftBottomY;
	euroform.leftBottomZ = params.leftBottomZ;
	euroform.ang = params.ang;
	euroform.eu_stan_onoff = true;
	euroform.u_ins_wall = true;
	euroform.ang_x = DegreeToRad (90.0);

	for (xx = 0 ; xx < nHorEuroform ; ++xx) {
		height_t = 0.0;
		for (yy = 0 ; yy < nVerEuroform ; ++yy) {
			euroform.eu_wid = euroform.width = width [xx];
			euroform.eu_hei = euroform.height = height [yy];
			height_t += height [yy];
			elemList.Push (placeEuroform (euroform));
			moveIn3D ('z', euroform.ang, height [yy], &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
		}
		moveIn3D ('x', euroform.ang, width [xx], &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
		moveIn3D ('z', euroform.ang, -height_t, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
	}

	// 비계 파이프 (수평) 배치
	sqrPipe.leftBottomX = params.leftBottomX;
	sqrPipe.leftBottomY = params.leftBottomY;
	sqrPipe.leftBottomZ = params.leftBottomZ;
	sqrPipe.ang = params.ang;
	sqrPipe.length = params.width - (horizontalGap * 2);
	sqrPipe.pipeAng = DegreeToRad (0);

	moveIn3D ('x', sqrPipe.ang, horizontalGap, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
	moveIn3D ('y', sqrPipe.ang, -(0.0635 + 0.025), &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
	moveIn3D ('z', sqrPipe.ang, 0.150 - 0.031, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);

	for (xx = 0 ; xx <= nVerEuroform ; ++xx) {
		if (xx == 0) {
			// 1행
			elemList.Push (placeSqrPipe (sqrPipe));
			moveIn3D ('z', sqrPipe.ang, 0.062, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
			elemList.Push (placeSqrPipe (sqrPipe));
			moveIn3D ('z', sqrPipe.ang, -0.031 - 0.150 + height [xx] - 0.031, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
		} else if (xx == nVerEuroform) {
			// 마지막 행
			moveIn3D ('z', sqrPipe.ang, -0.150, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
			elemList.Push (placeSqrPipe (sqrPipe));
			moveIn3D ('z', sqrPipe.ang, 0.062, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
			elemList.Push (placeSqrPipe (sqrPipe));
		} else {
			// 나머지 행
			elemList.Push (placeSqrPipe (sqrPipe));
			moveIn3D ('z', sqrPipe.ang, 0.062, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
			elemList.Push (placeSqrPipe (sqrPipe));
			moveIn3D ('z', sqrPipe.ang, -0.031 + height [xx] - 0.031, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
		}
	}

	// 비계 파이프 (수직) 배치
	sqrPipe.leftBottomX = params.leftBottomX;
	sqrPipe.leftBottomY = params.leftBottomY;
	sqrPipe.leftBottomZ = params.leftBottomZ;
	sqrPipe.ang = params.ang;
	sqrPipe.length = params.height - 0.100;
	sqrPipe.pipeAng = DegreeToRad (90);

	moveIn3D ('x', sqrPipe.ang, width [0] - 0.150 - 0.035, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
	moveIn3D ('y', sqrPipe.ang, -(0.0635 + 0.075), &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
	moveIn3D ('z', sqrPipe.ang, 0.050, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);

	// 1열
	elemList.Push (placeSqrPipe (sqrPipe));	moveIn3D ('x', sqrPipe.ang, 0.070, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
	elemList.Push (placeSqrPipe (sqrPipe));	moveIn3D ('x', sqrPipe.ang, -0.070 - (width [0] - 0.150) + params.width + (-width [nHorEuroform-1] + 0.150), &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
	// 2열
	elemList.Push (placeSqrPipe (sqrPipe));	moveIn3D ('x', sqrPipe.ang, 0.070, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
	elemList.Push (placeSqrPipe (sqrPipe));

	// 핀볼트 배치 (수평 - 최하단, 최상단)
	pinbolt.leftBottomX = params.leftBottomX;
	pinbolt.leftBottomY = params.leftBottomY;
	pinbolt.leftBottomZ = params.leftBottomZ;
	pinbolt.ang = params.ang;
	pinbolt.bPinBoltRot90 = TRUE;
	pinbolt.boltLen = 0.100;
	pinbolt.angX = DegreeToRad (270.0);
	pinbolt.angY = DegreeToRad (0.0);

	moveIn3D ('y', pinbolt.ang, -(0.1635), &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);

	// 최하단 행
	moveIn3D ('z', pinbolt.ang, 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
	width_t = 0.0;
	for (xx = 0 ; xx < nHorEuroform - 1 ; ++xx) {
		width_t += width [xx];
		moveIn3D ('x', pinbolt.ang, width [xx], &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);

		elemList.Push (placePinbolt (pinbolt));
	}
	// 최상단 행
	moveIn3D ('x', pinbolt.ang, -width_t, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
	moveIn3D ('z', pinbolt.ang, params.height - 0.300, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
	for (xx = 0 ; xx < nHorEuroform - 1 ; ++xx) {
		moveIn3D ('x', pinbolt.ang, width [xx], &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);

		elemList.Push (placePinbolt (pinbolt));
	}

	// 핀볼트 배치 (수평 - 나머지)
	pinbolt.leftBottomX = params.leftBottomX;
	pinbolt.leftBottomY = params.leftBottomY;
	pinbolt.leftBottomZ = params.leftBottomZ;
	pinbolt.ang = params.ang;
	pinbolt.bPinBoltRot90 = FALSE;
	pinbolt.boltLen = 0.100;
	pinbolt.angX = DegreeToRad (270.0);
	pinbolt.angY = DegreeToRad (0.0);

	moveIn3D ('y', pinbolt.ang, -(0.1635), &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);

	// 2 ~ [n-1]행
	if (nHorEuroform >= 3) {
		moveIn3D ('x', pinbolt.ang, 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
		moveIn3D ('z', pinbolt.ang, height [0], &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
		for (xx = 1 ; xx < nVerEuroform ; ++xx) {
			width_t = 0.0;
			for (yy = 0 ; yy < nHorEuroform ; ++yy) {
				// 1열
				if (yy == 0) {
					elemList.Push (placePinbolt (pinbolt));
					moveIn3D ('x', pinbolt.ang, width [0] - 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
					width_t += width [0] - 0.150;
				// 마지막 열
				} else if (yy == nHorEuroform - 1) {
					width_t += width [nHorEuroform-1] - 0.150;
					moveIn3D ('x', pinbolt.ang, width [nHorEuroform-1] - 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
					elemList.Push (placePinbolt (pinbolt));
				// 나머지 열
				} else {
					width_t += width [yy];
					if (abs (width [yy] - 0.600) < EPS) {
						moveIn3D ('x', pinbolt.ang, 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						elemList.Push (placePinbolt (pinbolt));
						moveIn3D ('x', pinbolt.ang, 0.300, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						elemList.Push (placePinbolt (pinbolt));
						moveIn3D ('x', pinbolt.ang, 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
					} else if (abs (width [yy] - 0.500) < EPS) {
						moveIn3D ('x', pinbolt.ang, 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						elemList.Push (placePinbolt (pinbolt));
						moveIn3D ('x', pinbolt.ang, 0.200, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						elemList.Push (placePinbolt (pinbolt));
						moveIn3D ('x', pinbolt.ang, 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
					} else if (abs (width [yy] - 0.450) < EPS) {
						moveIn3D ('x', pinbolt.ang, 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						elemList.Push (placePinbolt (pinbolt));
						moveIn3D ('x', pinbolt.ang, 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						elemList.Push (placePinbolt (pinbolt));
						moveIn3D ('x', pinbolt.ang, 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
					} else if (abs (width [yy] - 0.400) < EPS) {
						moveIn3D ('x', pinbolt.ang, 0.100, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						elemList.Push (placePinbolt (pinbolt));
						moveIn3D ('x', pinbolt.ang, 0.200, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						elemList.Push (placePinbolt (pinbolt));
						moveIn3D ('x', pinbolt.ang, 0.100, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
					} else if (abs (width [yy] - 0.300) < EPS) {
						moveIn3D ('x', pinbolt.ang, 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						elemList.Push (placePinbolt (pinbolt));
						moveIn3D ('x', pinbolt.ang, 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
					} else if (abs (width [yy] - 0.200) < EPS) {
						moveIn3D ('x', pinbolt.ang, 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						elemList.Push (placePinbolt (pinbolt));
						moveIn3D ('x', pinbolt.ang, 0.050, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
					}
				}
			}
			moveIn3D ('x', pinbolt.ang, -width_t, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
			moveIn3D ('z', pinbolt.ang, height [xx], &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
		}
	}

	// 핀볼트 배치 (수직)
	pinbolt.leftBottomX = params.leftBottomX;
	pinbolt.leftBottomY = params.leftBottomY;
	pinbolt.leftBottomZ = params.leftBottomZ;
	pinbolt.ang = params.ang;
	pinbolt.bPinBoltRot90 = FALSE;
	pinbolt.boltLen = 0.150;
	pinbolt.angX = DegreeToRad (270.0);
	pinbolt.angY = DegreeToRad (0.0);

	moveIn3D ('x', pinbolt.ang, width [0] - 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
	moveIn3D ('y', pinbolt.ang, -(0.2135), &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
	moveIn3D ('z', pinbolt.ang, height [0], &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);

	// 1열
	height_t = 0.0;
	for (xx = 1 ; xx < nVerEuroform ; ++xx) {
		elemList.Push (placePinbolt (pinbolt));
		moveIn3D ('z', pinbolt.ang, height [xx], &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
		height_t += height [xx];
	}
	// 2열
	moveIn3D ('x', pinbolt.ang, -(width [0] - 0.150) + params.width + (-width [nHorEuroform-1] + 0.150), &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
	moveIn3D ('z', pinbolt.ang, -height_t, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
	for (xx = 1 ; xx < nVerEuroform ; ++xx) {
		elemList.Push (placePinbolt (pinbolt));
		moveIn3D ('z', pinbolt.ang, height [xx], &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
		height_t += height [xx];
	}

	// 벽체 타이
	//if (bDoubleSide == true) {
	//	walltie.leftBottomX = params.leftBottomX;
	//	walltie.leftBottomY = params.leftBottomY;
	//	walltie.leftBottomZ = params.leftBottomZ;
	//	walltie.ang = params.ang;
	//	remainder = fmod ((wallThk + 0.327), 0.100);
	//	walltie.boltLen = (wallThk + 0.327 + (0.100 - remainder));
	//	walltie.pipeBeg = 0.0365 + 0.1635;
	//	walltie.pipeEnd = 0.0365 + 0.1635 + wallThk;
	//	walltie.clampBeg = 0.0365;
	//	walltie.clampEnd = 0.0365 + wallThk + 0.327;

	//	moveIn3D ('x', walltie.ang, width [0] - 0.150, &walltie.leftBottomX, &walltie.leftBottomY, &walltie.leftBottomZ);
	//	moveIn3D ('y', walltie.ang, -(0.1635 + 0.0365), &walltie.leftBottomX, &walltie.leftBottomY, &walltie.leftBottomZ);
	//	moveIn3D ('z', walltie.ang, 0.350, &walltie.leftBottomX, &walltie.leftBottomY, &walltie.leftBottomZ);

	//	for (xx = 0 ; xx < 2 ; ++xx) {
	//		for (yy = 0 ; yy < nVerEuroform ; ++yy) {
	//			// 최하위 행
	//			if (yy == 0) {
	//				elemList.Push (placeWalltie (walltie));
	//				moveIn3D ('z', walltie.ang, height [yy], &walltie.leftBottomX, &walltie.leftBottomY, &walltie.leftBottomZ);
	//	
	//			// 최상위 행
	//			} else if (yy == nVerEuroform - 1) {
	//				moveIn3D ('z', walltie.ang, height [yy] - 0.350*2, &walltie.leftBottomX, &walltie.leftBottomY, &walltie.leftBottomZ);
	//				elemList.Push (placeWalltie (walltie));
	//				moveIn3D ('x', walltie.ang, -(width [0] - 0.150) + params.width + (-width [nHorEuroform-1] + 0.150), &walltie.leftBottomX, &walltie.leftBottomY, &walltie.leftBottomZ);
	//				moveIn3D ('z', walltie.ang, 0.350 - params.height + 0.350, &walltie.leftBottomX, &walltie.leftBottomY, &walltie.leftBottomZ);
	//			
	//			// 2 ~ [n-1]행
	//			} else {
	//				elemList.Push (placeWalltie (walltie));
	//				moveIn3D ('z', walltie.ang, height [yy], &walltie.leftBottomX, &walltie.leftBottomY, &walltie.leftBottomZ);
	//			}
	//		}
	//	}
	//}

	// 헤드 피스
	headpiece.leftBottomX = params.leftBottomX;
	headpiece.leftBottomY = params.leftBottomY;
	headpiece.leftBottomZ = params.leftBottomZ;
	headpiece.ang = params.ang;

	moveIn3D ('x', headpiece.ang, width [0] - 0.150 - 0.100, &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
	moveIn3D ('y', headpiece.ang, -0.1725, &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
	moveIn3D ('z', headpiece.ang, 0.231, &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);

	// 처음 행
	elemList.Push (placeHeadpiece_ver (headpiece));
	moveIn3D ('x', headpiece.ang, -(width [0] - 0.150) + params.width + (-width [nHorEuroform-1] + 0.150), &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
	elemList.Push (placeHeadpiece_ver (headpiece));
	moveIn3D ('x', headpiece.ang, (width [0] - 0.150) - params.width - (-width [nHorEuroform-1] + 0.150), &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
	elev_headpiece = 2.100;
	moveIn3D ('z', headpiece.ang, -0.231 + elev_headpiece, &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
	// 마지막 행
	elemList.Push (placeHeadpiece_ver (headpiece));
	moveIn3D ('x', headpiece.ang, -(width [0] - 0.150) + params.width + (-width [nHorEuroform-1] + 0.150), &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
	elemList.Push (placeHeadpiece_ver (headpiece));
	
	// 결합철물
	fittings.leftBottomX = params.leftBottomX;
	fittings.leftBottomY = params.leftBottomY;
	fittings.leftBottomZ = params.leftBottomZ;
	fittings.ang = params.ang;

	fittings.angX = DegreeToRad (0.0);
	fittings.angY = DegreeToRad (0.0);
	fittings.bolt_len = 0.150;
	fittings.bolt_dia = 0.012;
	fittings.bWasher1 = true;
	fittings.bWasher2 = true;
	fittings.washer_pos1 = 0.0;
	fittings.washer_pos2 = 0.108;
	fittings.washer_size = 0.100;
	strncpy (fittings.nutType, "육각너트", strlen ("육각너트"));

	moveIn3D ('x', fittings.ang, width [0] - 0.150, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
	moveIn3D ('y', fittings.ang, -0.0455, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
	moveIn3D ('z', fittings.ang, 0.150, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);

	// 처음 행
	fittings.ang += DegreeToRad (180.0);
	elemList.Push (placeFittings (fittings));
	fittings.ang -= DegreeToRad (180.0);
	moveIn3D ('x', fittings.ang, -(width [0] - 0.150) + params.width + (-width [nHorEuroform-1] + 0.150), &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
	fittings.ang += DegreeToRad (180.0);
	elemList.Push (placeFittings (fittings));
	fittings.ang -= DegreeToRad (180.0);
	moveIn3D ('x', fittings.ang, (width [0] - 0.150) - params.width - (-width [nHorEuroform-1] + 0.150), &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
	moveIn3D ('z', fittings.ang, params.height - 0.300, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);

	// 마지막 행
	fittings.ang += DegreeToRad (180.0);
	elemList.Push (placeFittings (fittings));
	fittings.ang -= DegreeToRad (180.0);
	moveIn3D ('x', fittings.ang, -(width [0] - 0.150) + params.width + (-width [nHorEuroform-1] + 0.150), &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
	fittings.ang += DegreeToRad (180.0);
	elemList.Push (placeFittings (fittings));
	fittings.ang -= DegreeToRad (180.0);

	// 그룹화하기
	if (!elemList.IsEmpty ()) {
		GSSize nElems = elemList.GetSize ();
		API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
		if (elemHead != NULL) {
			for (GSIndex i = 0; i < nElems; i++)
				(*elemHead)[i].guid = elemList[i];

			ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

			BMKillHandle ((GSHandle *) &elemHead);
		}
	}
	elemList.Clear (false);

	//////////////////////////////////////////////////////////////// 반대면
	if (bDoubleSide == true) {
		moveIn3D ('x', params.ang, params.width, &params.leftBottomX, &params.leftBottomY, &params.leftBottomY);
		moveIn3D ('y', params.ang, wallThk, &params.leftBottomX, &params.leftBottomY, &params.leftBottomY);
		params.ang += DegreeToRad (180.0);

		// 유로폼 설치 (반대편에서 변경됨)
		euroform.leftBottomX = params.leftBottomX;
		euroform.leftBottomY = params.leftBottomY;
		euroform.leftBottomZ = params.leftBottomZ;
		euroform.ang = params.ang;

		for (xx = nHorEuroform - 1 ; xx >= 0 ; --xx) {
			height_t = 0.0;
			for (yy = 0 ; yy < nVerEuroform ; ++yy) {
				euroform.eu_wid = euroform.width = width [xx];
				euroform.eu_hei = euroform.height = height [yy];
				height_t += height [yy];
				elemList.Push (placeEuroform (euroform));
				moveIn3D ('z', euroform.ang, height [yy], &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
			}
			moveIn3D ('x', euroform.ang, width [xx], &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
			moveIn3D ('z', euroform.ang, -height_t, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
		}

		// 비계 파이프 (수평) 배치
		sqrPipe.leftBottomX = params.leftBottomX;
		sqrPipe.leftBottomY = params.leftBottomY;
		sqrPipe.leftBottomZ = params.leftBottomZ;
		sqrPipe.ang = params.ang;
		sqrPipe.length = params.width - (horizontalGap * 2);
		sqrPipe.pipeAng = DegreeToRad (0);

		moveIn3D ('x', sqrPipe.ang, horizontalGap, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
		moveIn3D ('y', sqrPipe.ang, -(0.0635 + 0.025), &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
		moveIn3D ('z', sqrPipe.ang, 0.150 - 0.031, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);

		for (xx = 0 ; xx <= nVerEuroform ; ++xx) {
			if (xx == 0) {
				// 1행
				elemList.Push (placeSqrPipe (sqrPipe));
				moveIn3D ('z', sqrPipe.ang, 0.062, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
				elemList.Push (placeSqrPipe (sqrPipe));
				moveIn3D ('z', sqrPipe.ang, -0.031 - 0.150 + height [xx] - 0.031, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
			} else if (xx == nVerEuroform) {
				// 마지막 행
				moveIn3D ('z', sqrPipe.ang, -0.150, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
				elemList.Push (placeSqrPipe (sqrPipe));
				moveIn3D ('z', sqrPipe.ang, 0.062, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
				elemList.Push (placeSqrPipe (sqrPipe));
			} else {
				// 나머지 행
				elemList.Push (placeSqrPipe (sqrPipe));
				moveIn3D ('z', sqrPipe.ang, 0.062, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
				elemList.Push (placeSqrPipe (sqrPipe));
				moveIn3D ('z', sqrPipe.ang, -0.031 + height [xx] - 0.031, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
			}
		}

		// 비계 파이프 (수직) 배치
		sqrPipe.leftBottomX = params.leftBottomX;
		sqrPipe.leftBottomY = params.leftBottomY;
		sqrPipe.leftBottomZ = params.leftBottomZ;
		sqrPipe.ang = params.ang;
		sqrPipe.length = params.height - 0.100;
		sqrPipe.pipeAng = DegreeToRad (90);

		moveIn3D ('x', sqrPipe.ang, width [nHorEuroform-1] - 0.150 - 0.035, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
		moveIn3D ('y', sqrPipe.ang, -(0.0635 + 0.075), &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
		moveIn3D ('z', sqrPipe.ang, 0.050, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);

		// 1열
		elemList.Push (placeSqrPipe (sqrPipe));	moveIn3D ('x', sqrPipe.ang, 0.070, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
		elemList.Push (placeSqrPipe (sqrPipe));	moveIn3D ('x', sqrPipe.ang, -0.070 - (width [nHorEuroform-1] - 0.150) + params.width + (-width [0] + 0.150), &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
		// 2열
		elemList.Push (placeSqrPipe (sqrPipe));	moveIn3D ('x', sqrPipe.ang, 0.070, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
		elemList.Push (placeSqrPipe (sqrPipe));

		// 핀볼트 배치 (수평 - 최하단, 최상단) (반대편에서 변경됨)
		pinbolt.leftBottomX = params.leftBottomX;
		pinbolt.leftBottomY = params.leftBottomY;
		pinbolt.leftBottomZ = params.leftBottomZ;
		pinbolt.ang = params.ang;
		pinbolt.bPinBoltRot90 = TRUE;
		pinbolt.boltLen = 0.100;
		pinbolt.angX = DegreeToRad (270.0);
		pinbolt.angY = DegreeToRad (0.0);

		moveIn3D ('y', pinbolt.ang, -(0.1635), &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);

		// 최하단 행
		moveIn3D ('z', pinbolt.ang, 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
		width_t = 0.0;
		for (xx = nHorEuroform - 1 ; xx > 0 ; --xx) {
			width_t += width [xx];
			moveIn3D ('x', pinbolt.ang, width [xx], &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);

			elemList.Push (placePinbolt (pinbolt));
		}
		// 최상단 행
		moveIn3D ('x', pinbolt.ang, -width_t, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
		moveIn3D ('z', pinbolt.ang, params.height - 0.300, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
		for (xx = nHorEuroform - 1 ; xx > 0 ; --xx) {
			moveIn3D ('x', pinbolt.ang, width [xx], &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);

			elemList.Push (placePinbolt (pinbolt));
		}

		// 핀볼트 배치 (수평 - 나머지) (반대편에서 변경됨)
		pinbolt.leftBottomX = params.leftBottomX;
		pinbolt.leftBottomY = params.leftBottomY;
		pinbolt.leftBottomZ = params.leftBottomZ;
		pinbolt.ang = params.ang;
		pinbolt.bPinBoltRot90 = FALSE;
		pinbolt.boltLen = 0.100;
		pinbolt.angX = DegreeToRad (270.0);
		pinbolt.angY = DegreeToRad (0.0);

		moveIn3D ('y', pinbolt.ang, -(0.1635), &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);

		if (nHorEuroform >= 3) {
			moveIn3D ('x', pinbolt.ang, 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
			moveIn3D ('z', pinbolt.ang, height [0], &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
			for (xx = 1 ; xx < nVerEuroform ; ++xx) {
				width_t = 0.0;
				for (yy = nHorEuroform - 1 ; yy >= 0 ; --yy) {
					// 1열
					if (yy == nHorEuroform - 1) {
						elemList.Push (placePinbolt (pinbolt));
						moveIn3D ('x', pinbolt.ang, width [nHorEuroform-1] - 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						width_t += width [nHorEuroform-1] - 0.150;
					// 마지막 열
					} else if (yy == 0) {
						width_t += width [0] - 0.150;
						moveIn3D ('x', pinbolt.ang, width [0] - 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						elemList.Push (placePinbolt (pinbolt));
					// 나머지 열
					} else {
						width_t += width [yy];
						if (abs (width [yy] - 0.600) < EPS) {
							moveIn3D ('x', pinbolt.ang, 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
							elemList.Push (placePinbolt (pinbolt));
							moveIn3D ('x', pinbolt.ang, 0.300, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
							elemList.Push (placePinbolt (pinbolt));
							moveIn3D ('x', pinbolt.ang, 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						} else if (abs (width [yy] - 0.500) < EPS) {
							moveIn3D ('x', pinbolt.ang, 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
							elemList.Push (placePinbolt (pinbolt));
							moveIn3D ('x', pinbolt.ang, 0.200, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
							elemList.Push (placePinbolt (pinbolt));
							moveIn3D ('x', pinbolt.ang, 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						} else if (abs (width [yy] - 0.450) < EPS) {
							moveIn3D ('x', pinbolt.ang, 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
							elemList.Push (placePinbolt (pinbolt));
							moveIn3D ('x', pinbolt.ang, 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
							elemList.Push (placePinbolt (pinbolt));
							moveIn3D ('x', pinbolt.ang, 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						} else if (abs (width [yy] - 0.400) < EPS) {
							moveIn3D ('x', pinbolt.ang, 0.100, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
							elemList.Push (placePinbolt (pinbolt));
							moveIn3D ('x', pinbolt.ang, 0.200, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
							elemList.Push (placePinbolt (pinbolt));
							moveIn3D ('x', pinbolt.ang, 0.100, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						} else if (abs (width [yy] - 0.300) < EPS) {
							moveIn3D ('x', pinbolt.ang, 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
							elemList.Push (placePinbolt (pinbolt));
							moveIn3D ('x', pinbolt.ang, 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						} else if (abs (width [yy] - 0.200) < EPS) {
							moveIn3D ('x', pinbolt.ang, 0.050, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
							elemList.Push (placePinbolt (pinbolt));
							moveIn3D ('x', pinbolt.ang, 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						}
					}
				}
				moveIn3D ('x', pinbolt.ang, -width_t, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
				moveIn3D ('z', pinbolt.ang, height [xx], &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
			}
		}

		// 핀볼트 배치 (수직)
		pinbolt.leftBottomX = params.leftBottomX;
		pinbolt.leftBottomY = params.leftBottomY;
		pinbolt.leftBottomZ = params.leftBottomZ;
		pinbolt.ang = params.ang;
		pinbolt.bPinBoltRot90 = FALSE;
		pinbolt.boltLen = 0.150;
		pinbolt.angX = DegreeToRad (270.0);
		pinbolt.angY = DegreeToRad (0.0);

		moveIn3D ('x', pinbolt.ang, width [nHorEuroform-1] - 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
		moveIn3D ('y', pinbolt.ang, -(0.2135), &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
		moveIn3D ('z', pinbolt.ang, height [0], &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);

		// 1열
		height_t = 0.0;
		for (xx = 1 ; xx < nVerEuroform ; ++xx) {
			elemList.Push (placePinbolt (pinbolt));
			moveIn3D ('z', pinbolt.ang, height [xx], &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
			height_t += height [xx];
		}
		// 2열
		moveIn3D ('x', pinbolt.ang, -(width [nHorEuroform-1] - 0.150) + params.width + (-width [0] + 0.150), &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
		moveIn3D ('z', pinbolt.ang, -height_t, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
		for (xx = 1 ; xx < nVerEuroform ; ++xx) {
			elemList.Push (placePinbolt (pinbolt));
			moveIn3D ('z', pinbolt.ang, height [xx], &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
			height_t += height [xx];
		}

		// 헤드 피스
		headpiece.leftBottomX = params.leftBottomX;
		headpiece.leftBottomY = params.leftBottomY;
		headpiece.leftBottomZ = params.leftBottomZ;
		headpiece.ang = params.ang;

		moveIn3D ('x', headpiece.ang, width [nHorEuroform-1] - 0.150 - 0.100, &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
		moveIn3D ('y', headpiece.ang, -0.1725, &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
		moveIn3D ('z', headpiece.ang, 0.231, &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);

		// 처음 행
		elemList.Push (placeHeadpiece_hor (headpiece));
		moveIn3D ('x', headpiece.ang, -(width [nHorEuroform-1] - 0.150) + params.width + (-width [0] + 0.150), &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
		elemList.Push (placeHeadpiece_hor (headpiece));
		moveIn3D ('x', headpiece.ang, (width [nHorEuroform-1] - 0.150) - params.width - (-width [0] + 0.150), &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
		elev_headpiece = 2.100;
		moveIn3D ('z', headpiece.ang, -0.231 + elev_headpiece, &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
		// 마지막 행
		elemList.Push (placeHeadpiece_hor (headpiece));
		moveIn3D ('x', headpiece.ang, -(width [nHorEuroform-1] - 0.150) + params.width + (-width [0] + 0.150), &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
		elemList.Push (placeHeadpiece_hor (headpiece));

		// 결합철물
		fittings.leftBottomX = params.leftBottomX;
		fittings.leftBottomY = params.leftBottomY;
		fittings.leftBottomZ = params.leftBottomZ;
		fittings.ang = params.ang;

		fittings.angX = DegreeToRad (0.0);
		fittings.angY = DegreeToRad (0.0);
		fittings.bolt_len = 0.150;
		fittings.bolt_dia = 0.012;
		fittings.bWasher1 = true;
		fittings.bWasher2 = true;
		fittings.washer_pos1 = 0.0;
		fittings.washer_pos2 = 0.108;
		fittings.washer_size = 0.100;
		strncpy (fittings.nutType, "육각너트", strlen ("육각너트"));

		moveIn3D ('x', fittings.ang, width [nHorEuroform-1] - 0.150, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
		moveIn3D ('y', fittings.ang, -0.0455, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
		moveIn3D ('z', fittings.ang, 0.150, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);

		// 처음 행
		fittings.ang += DegreeToRad (180.0);
		elemList.Push (placeFittings (fittings));
		fittings.ang -= DegreeToRad (180.0);
		moveIn3D ('x', fittings.ang, -(width [nHorEuroform-1] - 0.150) + params.width + (-width [0] + 0.150), &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
		fittings.ang += DegreeToRad (180.0);
		elemList.Push (placeFittings (fittings));
		fittings.ang -= DegreeToRad (180.0);
		moveIn3D ('x', fittings.ang, (width [nHorEuroform-1] - 0.150) - params.width - (-width [0] + 0.150), &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
		moveIn3D ('z', fittings.ang, params.height - 0.300, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);

		// 마지막 행
		fittings.ang += DegreeToRad (180.0);
		elemList.Push (placeFittings (fittings));
		fittings.ang -= DegreeToRad (180.0);
		moveIn3D ('x', fittings.ang, -(width [nHorEuroform-1] - 0.150) + params.width + (-width [0] + 0.150), &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
		fittings.ang += DegreeToRad (180.0);
		elemList.Push (placeFittings (fittings));
		fittings.ang -= DegreeToRad (180.0);

		// 그룹화하기
		if (!elemList.IsEmpty ()) {
			GSSize nElems = elemList.GetSize ();
			API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
			if (elemHead != NULL) {
				for (GSIndex i = 0; i < nElems; i++)
					(*elemHead)[i].guid = elemList[i];

				ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

				BMKillHandle ((GSHandle *) &elemHead);
			}
		}
		elemList.Clear (false);
	}

	return	err;
}

// 테이블폼(벽) 배치 (벽눕히기) 타입1
GSErrCode	placeTableformOnWall_landscape_Type1 (WallTableform params)
{
	// 가로, 세로가 뒤바뀌어야 함
	exchangeDoubles (&params.width, &params.height);

	GSErrCode	err = NoError;

	short	nHorEuroform;			// 수평 방향 유로폼 개수
	short	nVerEuroform;			// 수직 방향 유로폼 개수
	double	width [7];				// 수평 방향 각 유로폼 너비
	double	height [7];				// 수직 방향 각 유로폼 높이

	short		xx, yy;
	double		width_t, height_t;
	double		elev_headpiece;
	double		verticalGap = 0.050;	// 수직재 양쪽 이격거리

	Euroform					euroform;
	SquarePipe					sqrPipe;
	PinBoltSet					pinbolt;
	//WallTie						walltie;
	HeadpieceOfPushPullProps	headpiece;
	MetalFittingsWithRectWasher	fittings;

	nHorEuroform = 0;
	nVerEuroform = 0;
	for (xx = 0 ; xx < 7 ; ++xx) {
		width [xx] = 0.0;
		height [xx] = 0.0;
	}

	if (abs (params.height - 2.300) < EPS) {
		nVerEuroform = 4;
		//height [0] = 0.600;		height [1] = 0.600;		height [2] = 0.500;		height [3] = 0.600;
		height [0] = 0.500;		height [1] = 0.600;		height [2] = 0.600;		height [3] = 0.600;
		verticalGap = 0.050;
	} else if (abs (params.height - 2.250) < EPS) {
		nVerEuroform = 4;
		height [0] = 0.600;		height [1] = 0.600;		height [2] = 0.450;		height [3] = 0.600;
		verticalGap = 0.025;
	} else if (abs (params.height - 2.200) < EPS) {
		nVerEuroform = 4;
		//height [0] = 0.600;		height [1] = 0.600;		height [2] = 0.400;		height [3] = 0.600;
		height [0] = 0.400;		height [1] = 0.600;		height [2] = 0.600;		height [3] = 0.600;
		verticalGap = 0.050;
	} else if (abs (params.height - 2.150) < EPS) {
		nVerEuroform = 4;
		//height [0] = 0.600;		height [1] = 0.500;		height [2] = 0.450;		height [3] = 0.600;
		height [0] = 0.500;		height [1] = 0.600;		height [2] = 0.450;		height [3] = 0.600;
		verticalGap = 0.025;
	} else if (abs (params.height - 2.100) < EPS) {
		nVerEuroform = 4;
		height [0] = 0.600;		height [1] = 0.600;		height [2] = 0.300;		height [3] = 0.600;
		verticalGap = 0.050;
	} else if (abs (params.height - 2.050) < EPS) {
		nVerEuroform = 4;
		//height [0] = 0.600;		height [1] = 0.450;		height [2] = 0.400;		height [3] = 0.600;
		height [0] = 0.400;		height [1] = 0.450;		height [2] = 0.600;		height [3] = 0.600;
		verticalGap = 0.025;
	} else if (abs (params.height - 2.000) < EPS) {
		nVerEuroform = 4;
		//height [0] = 0.600;		height [1] = 0.600;		height [2] = 0.200;		height [3] = 0.600;
		height [0] = 0.200;		height [1] = 0.600;		height [2] = 0.600;		height [3] = 0.600;
		verticalGap = 0.050;
	} else if (abs (params.height - 1.950) < EPS) {
		nVerEuroform = 4;
		height [0] = 0.600;		height [1] = 0.450;		height [2] = 0.300;		height [3] = 0.600;
		verticalGap = 0.025;
	} else if (abs (params.height - 1.900) < EPS) {
		nVerEuroform = 4;
		//height [0] = 0.600;		height [1] = 0.500;		height [2] = 0.200;		height [3] = 0.600;
		height [0] = 0.500;		height [1] = 0.200;		height [2] = 0.600;		height [3] = 0.600;
		verticalGap = 0.050;
	} else if (abs (params.height - 1.850) < EPS) {
		nVerEuroform = 4;
		//height [0] = 0.600;		height [1] = 0.450;		height [2] = 0.200;		height [3] = 0.600;
		height [0] = 0.200;		height [1] = 0.450;		height [2] = 0.600;		height [3] = 0.600;
		verticalGap = 0.025;
	} else if (abs (params.height - 1.800) < EPS) {
		nVerEuroform = 3;
		height [0] = 0.600;		height [1] = 0.600;		height [2] = 0.600;		height [3] = 0.0;
		verticalGap = 0.050;
	} else if (abs (params.height - 1.750) < EPS) {
		nVerEuroform = 4;
		//height [0] = 0.600;		height [1] = 0.200;		height [2] = 0.450;		height [3] = 0.500;
		height [0] = 0.500;		height [1] = 0.200;		height [2] = 0.450;		height [3] = 0.600;
		verticalGap = 0.025;
	} else if (abs (params.height - 1.700) < EPS) {
		nVerEuroform = 3;
		//height [0] = 0.600;		height [1] = 0.500;		height [2] = 0.600;		height [3] = 0.0;
		height [0] = 0.500;		height [1] = 0.600;		height [2] = 0.600;		height [3] = 0.0;
		verticalGap = 0.050;
	} else if (abs (params.height - 1.650) < EPS) {
		nVerEuroform = 3;
		height [0] = 0.600;		height [1] = 0.450;		height [2] = 0.600;		height [3] = 0.0;
		verticalGap = 0.025;
	} else if (abs (params.height - 1.600) < EPS) {
		nVerEuroform = 3;
		//height [0] = 0.600;		height [1] = 0.400;		height [2] = 0.600;		height [3] = 0.0;
		height [0] = 0.400;		height [1] = 0.600;		height [2] = 0.600;		height [3] = 0.0;
		verticalGap = 0.050;
	} else if (abs (params.height - 1.550) < EPS) {
		nVerEuroform = 3;
		//height [0] = 0.600;		height [1] = 0.450;		height [2] = 0.500;		height [3] = 0.0;
		height [0] = 0.500;		height [1] = 0.450;		height [2] = 0.600;		height [3] = 0.0;
		verticalGap = 0.025;
	} else if (abs (params.height - 1.500) < EPS) {
		nVerEuroform = 3;
		height [0] = 0.600;		height [1] = 0.300;		height [2] = 0.600;		height [3] = 0.0;
		verticalGap = 0.050;
	} else if (abs (params.height - 1.450) < EPS) {
		nVerEuroform = 3;
		//height [0] = 0.500;		height [1] = 0.450;		height [2] = 0.500;		height [3] = 0.0;
		height [0] = 0.400;		height [1] = 0.450;		height [2] = 0.600;		height [3] = 0.0;
		verticalGap = 0.025;
	} else if (abs (params.height - 1.400) < EPS) {
		nVerEuroform = 3;
		//height [0] = 0.500;		height [1] = 0.400;		height [2] = 0.500;		height [3] = 0.0;
		height [0] = 0.500;		height [1] = 0.300;		height [2] = 0.600;		height [3] = 0.0;
		verticalGap = 0.050;
	} else if (abs (params.height - 1.350) < EPS) {
		nVerEuroform = 3;
		height [0] = 0.600;		height [1] = 0.300;		height [2] = 0.450;		height [3] = 0.0;
		verticalGap = 0.025;
	} else if (abs (params.height - 1.300) < EPS) {
		nVerEuroform = 3;
		//height [0] = 0.600;		height [1] = 0.200;		height [2] = 0.500;		height [3] = 0.0;
		height [0] = 0.500;		height [1] = 0.200;		height [2] = 0.600;		height [3] = 0.0;
		verticalGap = 0.050;
	} else if (abs (params.height - 1.250) < EPS) {
		nVerEuroform = 3;
		//height [0] = 0.600;		height [1] = 0.200;		height [2] = 0.450;		height [3] = 0.0;
		height [0] = 0.200;		height [1] = 0.600;		height [2] = 0.450;		height [3] = 0.0;
		verticalGap = 0.025;
	} else if (abs (params.height - 1.200) < EPS) {
		nVerEuroform = 2;
		height [0] = 0.600;		height [1] = 0.600;		height [2] = 0.0;		height [3] = 0.0;
		verticalGap = 0.050;
	} else if (abs (params.height - 1.150) < EPS) {
		nVerEuroform = 3;
		//height [0] = 0.450;		height [1] = 0.300;		height [2] = 0.400;		height [3] = 0.0;
		height [0] = 0.400;		height [1] = 0.300;		height [2] = 0.450;		height [3] = 0.0;
		verticalGap = 0.025;
	} else if (abs (params.height - 1.100) < EPS) {
		//nVerEuroform = 3;
		nVerEuroform = 2;
		//height [0] = 0.400;		height [1] = 0.300;		height [2] = 0.400;		height [3] = 0.0;
		height [0] = 0.500;		height [1] = 0.600;		height [2] = 0.0;		height [3] = 0.0;
		verticalGap = 0.050;
	} else if (abs (params.height - 1.050) < EPS) {
		nVerEuroform = 3;
		height [0] = 0.450;		height [1] = 0.300;		height [2] = 0.300;		height [3] = 0.0;
		verticalGap = 0.025;
	} else if (abs (params.height - 1.000) < EPS) {
		nVerEuroform = 2;
		//height [0] = 0.600;		height [1] = 0.400;		height [2] = 0.0;		height [3] = 0.0;
		height [0] = 0.400;		height [1] = 0.600;		height [2] = 0.0;		height [3] = 0.0;
		verticalGap = 0.050;
	} else if (abs (params.height - 0.950) < EPS) {
		nVerEuroform = 2;
		//height [0] = 0.450;		height [1] = 0.500;		height [2] = 0.0;		height [3] = 0.0;
		height [0] = 0.500;		height [1] = 0.450;		height [2] = 0.0;		height [3] = 0.0;
		verticalGap = 0.025;
	} else if (abs (params.height - 0.900) < EPS) {
		nVerEuroform = 2;
		height [0] = 0.600;		height [1] = 0.300;		height [2] = 0.0;		height [3] = 0.0;
		verticalGap = 0.050;
	} else if (abs (params.height - 0.850) < EPS) {
		nVerEuroform = 2;
		height [0] = 0.400;		height [1] = 0.450;		height [2] = 0.0;		height [3] = 0.0;
		verticalGap = 0.025;
	} else if (abs (params.height - 0.800) < EPS) {
		nVerEuroform = 2;
		height [0] = 0.400;		height [1] = 0.400;		height [2] = 0.0;		height [3] = 0.0;
		verticalGap = 0.050;
	} else if (abs (params.height - 0.750) < EPS) {
		nVerEuroform = 2;
		height [0] = 0.450;		height [1] = 0.300;		height [2] = 0.0;		height [3] = 0.0;
		verticalGap = 0.025;
	} else if (abs (params.height - 0.700) < EPS) {
		nVerEuroform = 2;
		height [0] = 0.400;		height [1] = 0.300;		height [2] = 0.0;		height [3] = 0.0;
		verticalGap = 0.050;
	} else if (abs (params.height - 0.650) < EPS) {
		nVerEuroform = 2;
		//height [0] = 0.450;		height [1] = 0.200;		height [2] = 0.0;		height [3] = 0.0;
		height [0] = 0.200;		height [1] = 0.450;		height [2] = 0.0;		height [3] = 0.0;
		verticalGap = 0.025;
	} else if (abs (params.height - 0.600) < EPS) {
		nVerEuroform = 1;
		height [0] = 0.600;		height [1] = 0.0;		height [2] = 0.0;		height [3] = 0.0;
		verticalGap = 0.050;
	} else if (abs (params.height - 0.500) < EPS) {
		nVerEuroform = 1;
		height [0] = 0.500;		height [1] = 0.0;		height [2] = 0.0;		height [3] = 0.0;
		verticalGap = 0.050;
	} else if (abs (params.height - 0.450) < EPS) {
		nVerEuroform = 1;
		height [0] = 0.450;		height [1] = 0.0;		height [2] = 0.0;		height [3] = 0.0;
		verticalGap = 0.025;
	} else if (abs (params.height - 0.400) < EPS) {
		nVerEuroform = 1;
		height [0] = 0.400;		height [1] = 0.0;		height [2] = 0.0;		height [3] = 0.0;
		verticalGap = 0.050;
	} else {
		nVerEuroform = 0;
		height [0] = 0.0;		height [1] = 0.0;		height [2] = 0.0;		height [3] = 0.0;
	}

	if (abs (params.width - 6.000) < EPS) {
		nHorEuroform = 5;
		width [0] = 1.200;
		width [1] = 1.200;
		width [2] = 1.200;
		width [3] = 1.200;
		width [4] = 1.200;
	} else if (abs (params.width - 5.700) < EPS) {
		nHorEuroform = 5;
		width [0] = 1.200;
		width [1] = 1.200;
		width [2] = 1.200;
		width [3] = 1.200;
		width [4] = 0.900;
	} else if (abs (params.width - 5.400) < EPS) {
		nHorEuroform = 5;
		width [0] = 1.200;
		width [1] = 1.200;
		width [2] = 1.200;
		width [3] = 0.900;
		width [4] = 0.900;
	} else if (abs (params.width - 5.100) < EPS) {
		nHorEuroform = 5;
		width [0] = 1.200;
		width [1] = 1.200;
		width [2] = 1.200;
		width [3] = 0.900;
		width [4] = 0.600;
	} else if (abs (params.width - 4.800) < EPS) {
		nHorEuroform = 4;
		width [0] = 1.200;
		width [1] = 1.200;
		width [2] = 1.200;
		width [3] = 1.200;
		width [4] = 0.0;
	} else if (abs (params.width - 4.500) < EPS) {
		nHorEuroform = 4;
		width [0] = 1.200;
		width [1] = 1.200;
		width [2] = 1.200;
		width [3] = 0.900;
		width [4] = 0.0;
	} else if (abs (params.width - 4.200) < EPS) {
		nHorEuroform = 4;
		width [0] = 1.200;
		width [1] = 1.200;
		width [2] = 0.900;
		width [3] = 0.900;
		width [4] = 0.0;
	} else if (abs (params.width - 3.900) < EPS) {
		nHorEuroform = 4;
		width [0] = 1.200;
		width [1] = 1.200;
		width [2] = 0.900;
		width [3] = 0.600;
		width [4] = 0.0;
	} else if (abs (params.width - 3.600) < EPS) {
		nHorEuroform = 3;
		width [0] = 1.200;
		width [1] = 1.200;
		width [2] = 1.200;
		width [3] = 0.0;
		width [4] = 0.0;
	} else if (abs (params.width - 3.300) < EPS) {
		nHorEuroform = 3;
		width [0] = 1.200;
		width [1] = 1.200;
		width [2] = 0.900;
		width [3] = 0.0;
		width [4] = 0.0;
	} else if (abs (params.width - 3.000) < EPS) {
		nHorEuroform = 3;
		width [0] = 1.200;
		width [1] = 1.200;
		width [2] = 0.600;
		width [3] = 0.0;
		width [4] = 0.0;
	} else if (abs (params.width - 2.700) < EPS) {
		nHorEuroform = 3;
		width [0] = 1.200;
		width [1] = 0.900;
		width [2] = 0.600;
		width [3] = 0.0;
		width [4] = 0.0;
	} else if (abs (params.width - 2.400) < EPS) {
		nHorEuroform = 2;
		width [0] = 1.200;
		width [1] = 1.200;
		width [2] = 0.0;
		width [3] = 0.0;
		width [4] = 0.0;
	} else if (abs (params.width - 2.100) < EPS) {
		nHorEuroform = 2;
		width [0] = 1.200;
		width [1] = 0.900;
		width [2] = 0.0;
		width [3] = 0.0;
		width [4] = 0.0;
	} else if (abs (params.width - 1.800) < EPS) {
		nHorEuroform = 2;
		width [0] = 0.900;
		width [1] = 0.900;
		width [2] = 0.0;
		width [3] = 0.0;
		width [4] = 0.0;
	} else if (abs (params.width - 1.500) < EPS) {
		nHorEuroform = 2;
		width [0] = 0.900;
		width [1] = 0.600;
		width [2] = 0.0;
		width [3] = 0.0;
		width [4] = 0.0;
	} else {
		nHorEuroform = 0;
		width [0] = 0.0;
		width [1] = 0.0;
		width [2] = 0.0;
		width [3] = 0.0;
		width [4] = 0.0;
	}

	// 너비나 높이가 0이면 아무것도 배치하지 않음
	if ((nHorEuroform == 0) || (nVerEuroform == 0))
		return	NoError;

	//////////////////////////////////////////////////////////////// 현재면
	// 유로폼 설치
	euroform.leftBottomX = params.leftBottomX;
	euroform.leftBottomY = params.leftBottomY;
	euroform.leftBottomZ = params.leftBottomZ;
	euroform.ang = params.ang;
	euroform.eu_stan_onoff = true;
	euroform.u_ins_wall = false;
	euroform.ang_x = DegreeToRad (90.0);

	for (xx = 0 ; xx < nHorEuroform ; ++xx) {
		height_t = 0.0;
		for (yy = nVerEuroform-1 ; yy >= 0 ; --yy) {
			euroform.eu_wid = euroform.width	= height [yy];
			euroform.eu_hei = euroform.height	= width [xx];
			height_t += height [yy];
			elemList.Push (placeEuroform (euroform));
			moveIn3D ('z', euroform.ang, height [yy], &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
		}
		moveIn3D ('x', euroform.ang, width [xx], &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
		moveIn3D ('z', euroform.ang, -height_t, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
	}

	// 비계 파이프 (수직) 배치
	sqrPipe.leftBottomX = params.leftBottomX;
	sqrPipe.leftBottomY = params.leftBottomY;
	sqrPipe.leftBottomZ = params.leftBottomZ;
	sqrPipe.ang = params.ang;
	sqrPipe.length = params.height - (verticalGap * 2);
	sqrPipe.pipeAng = DegreeToRad (90.0);

	moveIn3D ('z', sqrPipe.ang, verticalGap, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
	moveIn3D ('y', sqrPipe.ang, -(0.0635 + 0.025), &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
	moveIn3D ('x', sqrPipe.ang, 0.150 - 0.031, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);

	for (xx = 0 ; xx <= nHorEuroform ; ++xx) {
		if (xx == 0) {
			// 1행
			elemList.Push (placeSqrPipe (sqrPipe));
			moveIn3D ('x', sqrPipe.ang, 0.062, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
			elemList.Push (placeSqrPipe (sqrPipe));
			moveIn3D ('x', sqrPipe.ang, -0.031 - 0.150 + width [xx] - 0.031, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
		} else if (xx == nHorEuroform) {
			// 마지막 행
			moveIn3D ('x', sqrPipe.ang, -0.150, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
			elemList.Push (placeSqrPipe (sqrPipe));
			moveIn3D ('x', sqrPipe.ang, 0.062, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
			elemList.Push (placeSqrPipe (sqrPipe));
		} else {
			// 나머지 행
			elemList.Push (placeSqrPipe (sqrPipe));
			moveIn3D ('x', sqrPipe.ang, 0.062, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
			elemList.Push (placeSqrPipe (sqrPipe));
			moveIn3D ('x', sqrPipe.ang, -0.031 + width [xx] - 0.031, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
		}
	}

	// 비계 파이프 (수평) 배치
	sqrPipe.leftBottomX = params.leftBottomX;
	sqrPipe.leftBottomY = params.leftBottomY;
	sqrPipe.leftBottomZ = params.leftBottomZ;
	sqrPipe.ang = params.ang;
	sqrPipe.length = params.width - 0.100;
	sqrPipe.pipeAng = DegreeToRad (0);

	height_t = 0.0;
	for (xx = nVerEuroform-1 ; xx >= 0 ; --xx) {
		height_t += height [xx];
	}
	moveIn3D ('z', sqrPipe.ang, height_t - height [0] + 0.150 + 0.035, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
	moveIn3D ('y', sqrPipe.ang, -(0.0635 + 0.075), &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
	moveIn3D ('x', sqrPipe.ang, 0.050, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);

	// 1열
	elemList.Push (placeSqrPipe (sqrPipe));	moveIn3D ('z', sqrPipe.ang, -0.070, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
	elemList.Push (placeSqrPipe (sqrPipe));	moveIn3D ('z', sqrPipe.ang, 0.070 + (height [nVerEuroform-1] - 0.150) - params.height - (-height [0] + 0.150), &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
	// 2열
	elemList.Push (placeSqrPipe (sqrPipe));	moveIn3D ('z', sqrPipe.ang, -0.070, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
	elemList.Push (placeSqrPipe (sqrPipe));

	// 핀볼트 배치 (수직 - 최하단, 최상단)
	pinbolt.leftBottomX = params.leftBottomX;
	pinbolt.leftBottomY = params.leftBottomY;
	pinbolt.leftBottomZ = params.leftBottomZ;
	pinbolt.ang = params.ang;
	pinbolt.bPinBoltRot90 = FALSE;
	pinbolt.boltLen = 0.100;
	pinbolt.angX = DegreeToRad (270.0);
	pinbolt.angY = DegreeToRad (0.0);

	moveIn3D ('y', pinbolt.ang, -(0.1635), &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);

	// 최하단 행
	moveIn3D ('x', pinbolt.ang, 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
	height_t = 0.0;
	for (xx = 0 ; xx < nVerEuroform ; ++xx) {
		height_t += height [xx];
	}
	moveIn3D ('z', pinbolt.ang, height_t, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
	height_t = 0.0;
	for (xx = 0 ; xx < nVerEuroform-1 ; ++xx) {
		height_t += height [xx];
		moveIn3D ('z', pinbolt.ang, -height [xx], &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);

		elemList.Push (placePinbolt (pinbolt));
	}
	// 최상단 행
	moveIn3D ('z', pinbolt.ang, height_t, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
	moveIn3D ('x', pinbolt.ang, params.width - 0.300, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
	for (xx = 0 ; xx < nVerEuroform-1 ; ++xx) {
		moveIn3D ('z', pinbolt.ang, -height [xx], &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);

		elemList.Push (placePinbolt (pinbolt));
	}

	// 핀볼트 배치 (수직 - 나머지)
	pinbolt.leftBottomX = params.leftBottomX;
	pinbolt.leftBottomY = params.leftBottomY;
	pinbolt.leftBottomZ = params.leftBottomZ;
	pinbolt.ang = params.ang;
	pinbolt.bPinBoltRot90 = TRUE;
	pinbolt.boltLen = 0.100;
	pinbolt.angX = DegreeToRad (270.0);
	pinbolt.angY = DegreeToRad (0.0);

	moveIn3D ('y', pinbolt.ang, -(0.1635), &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);

	// 2 ~ [n-1]행
	if (nVerEuroform >= 3) {
		height_t = 0.0;
		for (xx = 0 ; xx < nVerEuroform ; ++xx) {
			height_t += height [xx];
		}
		moveIn3D ('z', pinbolt.ang, height_t - 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
		moveIn3D ('x', pinbolt.ang, width [0], &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
		for (xx = 1 ; xx < nHorEuroform ; ++xx) {
			height_t = 0.0;
			for (yy = 0 ; yy < nVerEuroform ; ++yy) {
				// 1열
				if (yy == 0) {
					elemList.Push (placePinbolt (pinbolt));
					moveIn3D ('z', pinbolt.ang, -(height [0] - 0.150), &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
					height_t += height [0] - 0.150;
				// 마지막 열
				} else if (yy == nVerEuroform - 1) {
					height_t += height [nVerEuroform-1] - 0.150;
					moveIn3D ('z', pinbolt.ang, -(height [nVerEuroform-1] - 0.150), &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
					elemList.Push (placePinbolt (pinbolt));
				// 나머지 열
				} else {
					height_t += height [yy];
					if (abs (height [yy] - 0.600) < EPS) {
						moveIn3D ('z', pinbolt.ang, -0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						elemList.Push (placePinbolt (pinbolt));
						moveIn3D ('z', pinbolt.ang, -0.300, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						elemList.Push (placePinbolt (pinbolt));
						moveIn3D ('z', pinbolt.ang, -0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
					} else if (abs (height [yy] - 0.500) < EPS) {
						moveIn3D ('z', pinbolt.ang, -0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						elemList.Push (placePinbolt (pinbolt));
						moveIn3D ('z', pinbolt.ang, -0.200, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						elemList.Push (placePinbolt (pinbolt));
						moveIn3D ('z', pinbolt.ang, -0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
					} else if (abs (height [yy] - 0.450) < EPS) {
						moveIn3D ('z', pinbolt.ang, -0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						elemList.Push (placePinbolt (pinbolt));
						moveIn3D ('z', pinbolt.ang, -0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						elemList.Push (placePinbolt (pinbolt));
						moveIn3D ('z', pinbolt.ang, -0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
					} else if (abs (height [yy] - 0.400) < EPS) {
						moveIn3D ('z', pinbolt.ang, -0.100, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						elemList.Push (placePinbolt (pinbolt));
						moveIn3D ('z', pinbolt.ang, -0.200, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						elemList.Push (placePinbolt (pinbolt));
						moveIn3D ('z', pinbolt.ang, -0.100, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
					} else if (abs (height [yy] - 0.300) < EPS) {
						moveIn3D ('z', pinbolt.ang, -0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						elemList.Push (placePinbolt (pinbolt));
						moveIn3D ('z', pinbolt.ang, -0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
					} else if (abs (height [yy] - 0.200) < EPS) {
						moveIn3D ('z', pinbolt.ang, -0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						elemList.Push (placePinbolt (pinbolt));
						moveIn3D ('z', pinbolt.ang, -0.050, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
					}
				}
			}
			moveIn3D ('z', pinbolt.ang, height_t, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
			moveIn3D ('x', pinbolt.ang, width [xx], &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
		}
	}

	// 핀볼트 배치 (수평)
	pinbolt.leftBottomX = params.leftBottomX;
	pinbolt.leftBottomY = params.leftBottomY;
	pinbolt.leftBottomZ = params.leftBottomZ;
	pinbolt.ang = params.ang;
	pinbolt.bPinBoltRot90 = TRUE;
	pinbolt.boltLen = 0.150;
	pinbolt.angX = DegreeToRad (270.0);
	pinbolt.angY = DegreeToRad (0.0);

	height_t = 0.0;
	for (xx = 0 ; xx < nVerEuroform ; ++xx) {
		height_t += height [xx];
	}
	moveIn3D ('z', pinbolt.ang, height_t - (height [0] - 0.150), &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
	moveIn3D ('y', pinbolt.ang, -(0.2135), &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
	moveIn3D ('x', pinbolt.ang, width [0], &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);

	// 1열
	width_t = 0.0;
	for (xx = 1 ; xx < nHorEuroform ; ++xx) {
		elemList.Push (placePinbolt (pinbolt));
		moveIn3D ('x', pinbolt.ang, width [xx], &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
		width_t += width [xx];
	}
	// 2열
	moveIn3D ('z', pinbolt.ang, (height [0] - 0.150) - params.height - (-height [nVerEuroform-1] + 0.150), &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
	moveIn3D ('x', pinbolt.ang, -width_t, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
	for (xx = 1 ; xx < nHorEuroform ; ++xx) {
		elemList.Push (placePinbolt (pinbolt));
		moveIn3D ('x', pinbolt.ang, width [xx], &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
		width_t += width [xx];
	}

	// 벽체 타이
	//if (bDoubleSide == true) {
	//	walltie.leftBottomX = params.leftBottomX;
	//	walltie.leftBottomY = params.leftBottomY;
	//	walltie.leftBottomZ = params.leftBottomZ;
	//	walltie.ang = params.ang;
	//	remainder = fmod ((wallThk + 0.327), 0.100);
	//	walltie.boltLen = (wallThk + 0.327 + (0.100 - remainder));
	//	walltie.pipeBeg = 0.0365 + 0.1635;
	//	walltie.pipeEnd = 0.0365 + 0.1635 + wallThk;
	//	walltie.clampBeg = 0.0365;
	//	walltie.clampEnd = 0.0365 + wallThk + 0.327;

	//	height_t = 0.0;
	//	for (xx = 0 ; xx < nVerEuroform ; ++xx) {
	//		height_t += height [xx];
	//	}
	//	moveIn3D ('z', walltie.ang, height_t - (height [0] - 0.150), &walltie.leftBottomX, &walltie.leftBottomY, &walltie.leftBottomZ);
	//	moveIn3D ('y', walltie.ang, -(0.1635 + 0.0365), &walltie.leftBottomX, &walltie.leftBottomY, &walltie.leftBottomZ);
	//	moveIn3D ('x', walltie.ang, 0.350, &walltie.leftBottomX, &walltie.leftBottomY, &walltie.leftBottomZ);

	//	for (xx = 0 ; xx < 2 ; ++xx) {
	//		for (yy = 0 ; yy < nHorEuroform ; ++yy) {
	//			// 최하위 행
	//			if (yy == 0) {
	//				elemList.Push (placeWalltie (walltie));
	//				moveIn3D ('x', walltie.ang, width [yy], &walltie.leftBottomX, &walltie.leftBottomY, &walltie.leftBottomZ);
	//	
	//			// 최상위 행
	//			} else if (yy == nHorEuroform - 1) {
	//				moveIn3D ('x', walltie.ang, width [yy] - 0.350*2, &walltie.leftBottomX, &walltie.leftBottomY, &walltie.leftBottomZ);
	//				elemList.Push (placeWalltie (walltie));
	//				moveIn3D ('z', walltie.ang, (height [0] - 0.150) - params.height - (-height [nVerEuroform-1] + 0.150), &walltie.leftBottomX, &walltie.leftBottomY, &walltie.leftBottomZ);
	//				moveIn3D ('x', walltie.ang, (0.350 - params.width + 0.350), &walltie.leftBottomX, &walltie.leftBottomY, &walltie.leftBottomZ);
	//			
	//			// 2 ~ [n-1]행
	//			} else {
	//				elemList.Push (placeWalltie (walltie));
	//				moveIn3D ('x', walltie.ang, width [yy], &walltie.leftBottomX, &walltie.leftBottomY, &walltie.leftBottomZ);
	//			}
	//		}
	//	}
	//}

	// 헤드 피스
	headpiece.leftBottomX = params.leftBottomX;
	headpiece.leftBottomY = params.leftBottomY;
	headpiece.leftBottomZ = params.leftBottomZ;
	headpiece.ang = params.ang;

	height_t = 0.0;
	for (xx = 0 ; xx < nVerEuroform ; ++xx) {
		height_t += height [xx];
	}
	width_t = 0.0;
	for (xx = 0 ; xx < nHorEuroform ; ++xx) {
		width_t += width [xx];
	}
	moveIn3D ('z', headpiece.ang, height_t - (height [0] - 0.150 - 0.100) - 0.200, &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
	moveIn3D ('y', headpiece.ang, -0.1725, &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
	moveIn3D ('x', headpiece.ang, 0.600, &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);

	// 처음 행
	elemList.Push (placeHeadpiece_hor (headpiece));
	moveIn3D ('z', headpiece.ang, (height [0] - 0.150) - params.height - (-height [nVerEuroform-1] + 0.150), &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
	elemList.Push (placeHeadpiece_hor (headpiece));
	moveIn3D ('z', headpiece.ang, -(height [nVerEuroform-1] - 0.150) + params.height + (-height [0] + 0.150), &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
	elev_headpiece = width_t - 0.800;
	moveIn3D ('x', headpiece.ang, -0.600 + elev_headpiece, &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
	// 마지막 행
	elemList.Push (placeHeadpiece_hor (headpiece));
	moveIn3D ('z', headpiece.ang, (height [nVerEuroform-1] - 0.150) - params.height - (-height [0] + 0.150), &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
	elemList.Push (placeHeadpiece_hor (headpiece));

	// 결합철물
	fittings.leftBottomX = params.leftBottomX;
	fittings.leftBottomY = params.leftBottomY;
	fittings.leftBottomZ = params.leftBottomZ;
	fittings.ang = params.ang;

	fittings.angX = DegreeToRad (0.0);
	fittings.angY = DegreeToRad (0.0);
	fittings.bolt_len = 0.150;
	fittings.bolt_dia = 0.012;
	fittings.bWasher1 = true;
	fittings.bWasher2 = true;
	fittings.washer_pos1 = 0.0;
	fittings.washer_pos2 = 0.108;
	fittings.washer_size = 0.100;
	strncpy (fittings.nutType, "육각너트", strlen ("육각너트"));

	height_t = 0.0;
	for (xx = 0 ; xx < nVerEuroform ; ++xx) {
		height_t += height [xx];
	}
	moveIn3D ('z', fittings.ang, height_t - (height [0] - 0.150), &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
	moveIn3D ('y', fittings.ang, -0.0455, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
	moveIn3D ('x', fittings.ang, 0.150, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);

	// 처음 행
	fittings.ang += DegreeToRad (180.0);
	elemList.Push (placeFittings (fittings));
	fittings.ang -= DegreeToRad (180.0);
	moveIn3D ('z', fittings.ang, (height [0] - 0.150) - params.height - (-height [nVerEuroform-1] + 0.150), &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
	fittings.ang += DegreeToRad (180.0);
	elemList.Push (placeFittings (fittings));
	fittings.ang -= DegreeToRad (180.0);
	moveIn3D ('z', fittings.ang, -(height [nVerEuroform-1] - 0.150) + params.height + (-height [0] + 0.150), &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
	moveIn3D ('x', fittings.ang, params.width - 0.300, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);

	// 마지막 행
	fittings.ang += DegreeToRad (180.0);
	elemList.Push (placeFittings (fittings));
	fittings.ang -= DegreeToRad (180.0);
	moveIn3D ('z', fittings.ang, (height [nVerEuroform-1] - 0.150) - params.height - (-height [0] + 0.150), &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
	fittings.ang += DegreeToRad (180.0);
	elemList.Push (placeFittings (fittings));
	fittings.ang -= DegreeToRad (180.0);

	// 그룹화하기
	if (!elemList.IsEmpty ()) {
		GSSize nElems = elemList.GetSize ();
		API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
		if (elemHead != NULL) {
			for (GSIndex i = 0; i < nElems; i++)
				(*elemHead)[i].guid = elemList[i];

			ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

			BMKillHandle ((GSHandle *) &elemHead);
		}
	}
	elemList.Clear (false);

	//////////////////////////////////////////////////////////////// 반대면
	if (bDoubleSide == true) {
		moveIn3D ('x', params.ang, params.width, &params.leftBottomX, &params.leftBottomY, &params.leftBottomY);
		moveIn3D ('y', params.ang, wallThk, &params.leftBottomX, &params.leftBottomY, &params.leftBottomY);
		params.ang += DegreeToRad (180.0);

		// 유로폼 설치 (반대편에서 변경됨)
		euroform.leftBottomX = params.leftBottomX;
		euroform.leftBottomY = params.leftBottomY;
		euroform.leftBottomZ = params.leftBottomZ;
		euroform.ang = params.ang;

		for (xx = nHorEuroform-1 ; xx >= 0 ; --xx) {
			height_t = 0.0;
			for (yy = nVerEuroform-1 ; yy >= 0 ; --yy) {
				euroform.eu_wid = euroform.width	= height [yy];
				euroform.eu_hei = euroform.height	= width [xx];
				height_t += height [yy];
				elemList.Push (placeEuroform (euroform));
				moveIn3D ('z', euroform.ang, height [yy], &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
			}
			moveIn3D ('x', euroform.ang, width [xx], &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
			moveIn3D ('z', euroform.ang, -height_t, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
		}

		// 비계 파이프 (수직) 배치
		sqrPipe.leftBottomX = params.leftBottomX;
		sqrPipe.leftBottomY = params.leftBottomY;
		sqrPipe.leftBottomZ = params.leftBottomZ;
		sqrPipe.ang = params.ang;
		sqrPipe.length = params.height - (verticalGap * 2);
		sqrPipe.pipeAng = DegreeToRad (90.0);

		moveIn3D ('z', sqrPipe.ang, verticalGap, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
		moveIn3D ('y', sqrPipe.ang, -(0.0635 + 0.025), &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
		moveIn3D ('x', sqrPipe.ang, params.width - (0.150 - 0.031), &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);

		for (xx = 0 ; xx <= nHorEuroform ; ++xx) {
			if (xx == 0) {
				// 1행
				elemList.Push (placeSqrPipe (sqrPipe));
				moveIn3D ('x', sqrPipe.ang, -0.062, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
				elemList.Push (placeSqrPipe (sqrPipe));
				moveIn3D ('x', sqrPipe.ang, 0.031 + 0.150 - width [xx] + 0.031, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
			} else if (xx == nHorEuroform) {
				// 마지막 행
				moveIn3D ('x', sqrPipe.ang, 0.150, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
				elemList.Push (placeSqrPipe (sqrPipe));
				moveIn3D ('x', sqrPipe.ang, -0.062, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
				elemList.Push (placeSqrPipe (sqrPipe));
			} else {
				// 나머지 행
				elemList.Push (placeSqrPipe (sqrPipe));
				moveIn3D ('x', sqrPipe.ang, -0.062, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
				elemList.Push (placeSqrPipe (sqrPipe));
				moveIn3D ('x', sqrPipe.ang, 0.031 - width [xx] + 0.031, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
			}
		}

		// 비계 파이프 (수평) 배치
		sqrPipe.leftBottomX = params.leftBottomX;
		sqrPipe.leftBottomY = params.leftBottomY;
		sqrPipe.leftBottomZ = params.leftBottomZ;
		sqrPipe.ang = params.ang;
		sqrPipe.length = params.width - 0.100;
		sqrPipe.pipeAng = DegreeToRad (0);

		height_t = 0.0;
		for (xx = nVerEuroform-1 ; xx >= 0 ; --xx) {
			height_t += height [xx];
		}
		moveIn3D ('z', sqrPipe.ang, height_t - height [0] + 0.150 + 0.035, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
		moveIn3D ('y', sqrPipe.ang, -(0.0635 + 0.075), &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
		moveIn3D ('x', sqrPipe.ang, 0.050, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);

		// 1열
		elemList.Push (placeSqrPipe (sqrPipe));	moveIn3D ('z', sqrPipe.ang, -0.070, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
		elemList.Push (placeSqrPipe (sqrPipe));	moveIn3D ('z', sqrPipe.ang, 0.070 + (height [nVerEuroform-1] - 0.150) - params.height - (-height [0] + 0.150), &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
		// 2열
		elemList.Push (placeSqrPipe (sqrPipe));	moveIn3D ('z', sqrPipe.ang, -0.070, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
		elemList.Push (placeSqrPipe (sqrPipe));

		// 핀볼트 배치 (수직 - 최하단, 최상단)
		pinbolt.leftBottomX = params.leftBottomX;
		pinbolt.leftBottomY = params.leftBottomY;
		pinbolt.leftBottomZ = params.leftBottomZ;
		pinbolt.ang = params.ang;
		pinbolt.bPinBoltRot90 = FALSE;
		pinbolt.boltLen = 0.100;
		pinbolt.angX = DegreeToRad (270.0);
		pinbolt.angY = DegreeToRad (0.0);

		moveIn3D ('y', pinbolt.ang, -(0.1635), &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);

		// 최하단 행
		moveIn3D ('x', pinbolt.ang, params.width - 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
		height_t = 0.0;
		for (xx = 0 ; xx < nVerEuroform ; ++xx) {
			height_t += height [xx];
		}
		moveIn3D ('z', pinbolt.ang, height_t, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
		height_t = 0.0;
		for (xx = 0 ; xx < nVerEuroform-1 ; ++xx) {
			height_t += height [xx];
			moveIn3D ('z', pinbolt.ang, -height [xx], &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);

			elemList.Push (placePinbolt (pinbolt));
		}
		// 최상단 행
		moveIn3D ('z', pinbolt.ang, height_t, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
		moveIn3D ('x', pinbolt.ang, -params.width + 0.300, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
		for (xx = 0 ; xx < nVerEuroform-1 ; ++xx) {
			moveIn3D ('z', pinbolt.ang, -height [xx], &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);

			elemList.Push (placePinbolt (pinbolt));
		}

		// 핀볼트 배치 (수직 - 나머지)
		pinbolt.leftBottomX = params.leftBottomX;
		pinbolt.leftBottomY = params.leftBottomY;
		pinbolt.leftBottomZ = params.leftBottomZ;
		pinbolt.ang = params.ang;
		pinbolt.bPinBoltRot90 = TRUE;
		pinbolt.boltLen = 0.100;
		pinbolt.angX = DegreeToRad (270.0);
		pinbolt.angY = DegreeToRad (0.0);

		moveIn3D ('y', pinbolt.ang, -(0.1635), &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);

		// 2 ~ [n-1]행
		if (nVerEuroform >= 3) {
			height_t = 0.0;
			for (xx = 0 ; xx < nVerEuroform ; ++xx) {
				height_t += height [xx];
			}
			moveIn3D ('z', pinbolt.ang, height_t - 0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
			moveIn3D ('x', pinbolt.ang, params.width - width [0], &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
			for (xx = 1 ; xx < nHorEuroform ; ++xx) {
				height_t = 0.0;
				for (yy = 0 ; yy < nVerEuroform ; ++yy) {
					// 1열
					if (yy == 0) {
						elemList.Push (placePinbolt (pinbolt));
						moveIn3D ('z', pinbolt.ang, -(height [0] - 0.150), &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						height_t += height [0] - 0.150;
					// 마지막 열
					} else if (yy == nVerEuroform - 1) {
						height_t += height [nVerEuroform-1] - 0.150;
						moveIn3D ('z', pinbolt.ang, -(height [nVerEuroform-1] - 0.150), &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						elemList.Push (placePinbolt (pinbolt));
					// 나머지 열
					} else {
						height_t += height [yy];
						if (abs (height [yy] - 0.600) < EPS) {
							moveIn3D ('z', pinbolt.ang, -0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
							elemList.Push (placePinbolt (pinbolt));
							moveIn3D ('z', pinbolt.ang, -0.300, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
							elemList.Push (placePinbolt (pinbolt));
							moveIn3D ('z', pinbolt.ang, -0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						} else if (abs (height [yy] - 0.500) < EPS) {
							moveIn3D ('z', pinbolt.ang, -0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
							elemList.Push (placePinbolt (pinbolt));
							moveIn3D ('z', pinbolt.ang, -0.200, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
							elemList.Push (placePinbolt (pinbolt));
							moveIn3D ('z', pinbolt.ang, -0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						} else if (abs (height [yy] - 0.450) < EPS) {
							moveIn3D ('z', pinbolt.ang, -0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
							elemList.Push (placePinbolt (pinbolt));
							moveIn3D ('z', pinbolt.ang, -0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
							elemList.Push (placePinbolt (pinbolt));
							moveIn3D ('z', pinbolt.ang, -0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						} else if (abs (height [yy] - 0.400) < EPS) {
							moveIn3D ('z', pinbolt.ang, -0.100, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
							elemList.Push (placePinbolt (pinbolt));
							moveIn3D ('z', pinbolt.ang, -0.200, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
							elemList.Push (placePinbolt (pinbolt));
							moveIn3D ('z', pinbolt.ang, -0.100, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						} else if (abs (height [yy] - 0.300) < EPS) {
							moveIn3D ('z', pinbolt.ang, -0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
							elemList.Push (placePinbolt (pinbolt));
							moveIn3D ('z', pinbolt.ang, -0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						} else if (abs (height [yy] - 0.200) < EPS) {
							moveIn3D ('z', pinbolt.ang, -0.150, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
							elemList.Push (placePinbolt (pinbolt));
							moveIn3D ('z', pinbolt.ang, -0.050, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
						}
					}
				}
				moveIn3D ('z', pinbolt.ang, height_t, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
				moveIn3D ('x', pinbolt.ang, -width [xx], &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
			}
		}

		// 핀볼트 배치 (수평)
		pinbolt.leftBottomX = params.leftBottomX;
		pinbolt.leftBottomY = params.leftBottomY;
		pinbolt.leftBottomZ = params.leftBottomZ;
		pinbolt.ang = params.ang;
		pinbolt.bPinBoltRot90 = TRUE;
		pinbolt.boltLen = 0.150;
		pinbolt.angX = DegreeToRad (270.0);
		pinbolt.angY = DegreeToRad (0.0);

		height_t = 0.0;
		for (xx = 0 ; xx < nVerEuroform ; ++xx) {
			height_t += height [xx];
		}
		moveIn3D ('z', pinbolt.ang, height_t - (height [0] - 0.150), &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
		moveIn3D ('y', pinbolt.ang, -(0.2135), &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
		moveIn3D ('x', pinbolt.ang, params.width - width [0], &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);

		// 1열
		width_t = 0.0;
		for (xx = 1 ; xx < nHorEuroform ; ++xx) {
			elemList.Push (placePinbolt (pinbolt));
			moveIn3D ('x', pinbolt.ang, -width [xx], &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
			width_t += width [xx];
		}
		// 2열
		moveIn3D ('z', pinbolt.ang, (height [0] - 0.150) - params.height - (-height [nVerEuroform-1] + 0.150), &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
		moveIn3D ('x', pinbolt.ang, width_t, &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
		for (xx = 1 ; xx < nHorEuroform ; ++xx) {
			elemList.Push (placePinbolt (pinbolt));
			moveIn3D ('x', pinbolt.ang, -width [xx], &pinbolt.leftBottomX, &pinbolt.leftBottomY, &pinbolt.leftBottomZ);
			width_t += width [xx];
		}

		// 벽체 타이 (현재면에서 했으므로 생략)

		// 헤드 피스
		headpiece.leftBottomX = params.leftBottomX;
		headpiece.leftBottomY = params.leftBottomY;
		headpiece.leftBottomZ = params.leftBottomZ;
		headpiece.ang = params.ang;

		height_t = 0.0;
		for (xx = 0 ; xx < nVerEuroform ; ++xx) {
			height_t += height [xx];
		}
		width_t = 0.0;
		for (xx = 0 ; xx < nHorEuroform ; ++xx) {
			width_t += width [xx];
		}
		moveIn3D ('z', headpiece.ang, height_t - (height [0] - 0.150 - 0.100) - 0.200, &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
		moveIn3D ('y', headpiece.ang, -0.1725, &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
		moveIn3D ('x', headpiece.ang, params.width - 0.800, &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);

		// 처음 행
		elemList.Push (placeHeadpiece_hor (headpiece));
		moveIn3D ('z', headpiece.ang, (height [0] - 0.150) - params.height - (-height [nVerEuroform-1] + 0.150), &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
		elemList.Push (placeHeadpiece_hor (headpiece));
		moveIn3D ('z', headpiece.ang, -(height [nVerEuroform-1] - 0.150) + params.height + (-height [0] + 0.150), &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
		elev_headpiece = width_t - 0.800;
		moveIn3D ('x', headpiece.ang, 0.600 - elev_headpiece, &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
		// 마지막 행
		elemList.Push (placeHeadpiece_hor (headpiece));
		moveIn3D ('z', headpiece.ang, (height [nVerEuroform-1] - 0.150) - params.height - (-height [0] + 0.150), &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
		elemList.Push (placeHeadpiece_hor (headpiece));

		// 결합철물
		fittings.leftBottomX = params.leftBottomX;
		fittings.leftBottomY = params.leftBottomY;
		fittings.leftBottomZ = params.leftBottomZ;
		fittings.ang = params.ang;

		fittings.angX = DegreeToRad (0.0);
		fittings.angY = DegreeToRad (0.0);
		fittings.bolt_len = 0.150;
		fittings.bolt_dia = 0.012;
		fittings.bWasher1 = true;
		fittings.bWasher2 = true;
		fittings.washer_pos1 = 0.0;
		fittings.washer_pos2 = 0.108;
		fittings.washer_size = 0.100;
		strncpy (fittings.nutType, "육각너트", strlen ("육각너트"));

		height_t = 0.0;
		for (xx = 0 ; xx < nVerEuroform ; ++xx) {
			height_t += height [xx];
		}
		moveIn3D ('z', fittings.ang, height_t - (height [0] - 0.150), &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
		moveIn3D ('y', fittings.ang, -0.0455, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
		moveIn3D ('x', fittings.ang, params.width - 0.150, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);

		// 처음 행
		fittings.ang += DegreeToRad (180.0);
		elemList.Push (placeFittings (fittings));
		fittings.ang -= DegreeToRad (180.0);
		moveIn3D ('z', fittings.ang, (height [0] - 0.150) - params.height - (-height [nVerEuroform-1] + 0.150), &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
		fittings.ang += DegreeToRad (180.0);
		elemList.Push (placeFittings (fittings));
		fittings.ang -= DegreeToRad (180.0);
		moveIn3D ('z', fittings.ang, -(height [nVerEuroform-1] - 0.150) + params.height + (-height [0] + 0.150), &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
		moveIn3D ('x', fittings.ang, -params.width + 0.300, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);

		// 마지막 행
		fittings.ang += DegreeToRad (180.0);
		elemList.Push (placeFittings (fittings));
		fittings.ang -= DegreeToRad (180.0);
		moveIn3D ('z', fittings.ang, (height [nVerEuroform-1] - 0.150) - params.height - (-height [0] + 0.150), &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
		fittings.ang += DegreeToRad (180.0);
		elemList.Push (placeFittings (fittings));
		fittings.ang -= DegreeToRad (180.0);

		// 그룹화하기
		if (!elemList.IsEmpty ()) {
			GSSize nElems = elemList.GetSize ();
			API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
			if (elemHead != NULL) {
				for (GSIndex i = 0; i < nElems; i++)
					(*elemHead)[i].guid = elemList[i];

				ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

				BMKillHandle ((GSHandle *) &elemHead);
			}
		}
		elemList.Clear (false);
	}

	return	err;
}

// 테이블폼(벽) 배치 (벽세우기) 타입2
GSErrCode	placeTableformOnWall_portrait_Type2 (WallTableform params)
{
	GSErrCode	err = NoError;

	short	nHorEuroform;			// 수평 방향 유로폼 개수
	short	nVerEuroform;			// 수직 방향 유로폼 개수
	double	width [7];				// 수평 방향 각 유로폼 너비
	double	height [7];				// 수직 방향 각 유로폼 높이

	short	nVerticalBar;
	double	verticalBarLeftOffset;
	double	verticalBarRightOffset;

	short		xx, yy;
	double		width_t, height_t;
	double		elev_headpiece;
	double		horizontalGap = 0.050;	// 수평재 양쪽 이격거리
	API_Guid	tempGuid;
	Cylinder	cylinder;

	Euroform		euroform;
	SquarePipe		sqrPipe;
	HeadpieceOfPushPullProps	headpiece;
	MetalFittings	fittings;
	EuroformHook	hook;
	RectPipeHanger	hanger;

	nHorEuroform = 0;
	nVerEuroform = 0;
	for (xx = 0 ; xx < 7 ; ++xx) {
		width [xx] = 0.0;
		height [xx] = 0.0;
	}

	if (abs (params.width - 2.300) < EPS) {
		nHorEuroform = 4;
		width [0] = 0.600;	width [1] = 0.600;	width [2] = 0.500;	width [3] = 0.600;
		horizontalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.width - 2.250) < EPS) {
		nHorEuroform = 4;
		width [0] = 0.600;	width [1] = 0.600;	width [2] = 0.450;	width [3] = 0.600;
		horizontalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.width - 2.200) < EPS) {
		nHorEuroform = 4;
		width [0] = 0.600;	width [1] = 0.600;	width [2] = 0.400;	width [3] = 0.600;
		horizontalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.width - 2.150) < EPS) {
		nHorEuroform = 4;
		width [0] = 0.600;	width [1] = 0.500;	width [2] = 0.450;	width [3] = 0.600;
		horizontalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.width - 2.100) < EPS) {
		nHorEuroform = 4;
		width [0] = 0.600;	width [1] = 0.600;	width [2] = 0.300;	width [3] = 0.600;
		horizontalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.width - 2.050) < EPS) {
		nHorEuroform = 4;
		width [0] = 0.600;	width [1] = 0.450;	width [2] = 0.400;	width [3] = 0.600;
		horizontalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.width - 2.000) < EPS) {
		nHorEuroform = 4;
		width [0] = 0.600;	width [1] = 0.600;	width [2] = 0.200;	width [3] = 0.600;
		horizontalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.width - 1.950) < EPS) {
		nHorEuroform = 4;
		width [0] = 0.600;	width [1] = 0.450;	width [2] = 0.300;	width [3] = 0.600;
		horizontalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.width - 1.900) < EPS) {
		nHorEuroform = 4;
		width [0] = 0.600;	width [1] = 0.500;	width [2] = 0.200;	width [3] = 0.600;
		horizontalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.width - 1.850) < EPS) {
		nHorEuroform = 4;
		width [0] = 0.600;	width [1] = 0.450;	width [2] = 0.200;	width [3] = 0.600;
		horizontalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.width - 1.800) < EPS) {
		nHorEuroform = 3;
		width [0] = 0.600;	width [1] = 0.600;	width [2] = 0.600;	width [3] = 0.0;
		horizontalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.width - 1.750) < EPS) {
		nHorEuroform = 4;
		width [0] = 0.600;	width [1] = 0.200;	width [2] = 0.450;	width [3] = 0.500;
		horizontalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.width - 1.700) < EPS) {
		nHorEuroform = 3;
		width [0] = 0.600;	width [1] = 0.500;	width [2] = 0.600;	width [3] = 0.0;
		horizontalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.width - 1.650) < EPS) {
		nHorEuroform = 3;
		width [0] = 0.600;	width [1] = 0.450;	width [2] = 0.600;	width [3] = 0.0;
		horizontalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.width - 1.600) < EPS) {
		nHorEuroform = 3;
		width [0] = 0.600;	width [1] = 0.400;	width [2] = 0.600;	width [3] = 0.0;
		horizontalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.width - 1.550) < EPS) {
		nHorEuroform = 3;
		width [0] = 0.600;	width [1] = 0.450;	width [2] = 0.500;	width [3] = 0.0;
		horizontalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.width - 1.500) < EPS) {
		nHorEuroform = 3;
		width [0] = 0.600;	width [1] = 0.300;	width [2] = 0.600;	width [3] = 0.0;
		horizontalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.width - 1.450) < EPS) {
		nHorEuroform = 3;
		width [0] = 0.500;	width [1] = 0.450;	width [2] = 0.500;	width [3] = 0.0;
		horizontalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.width - 1.400) < EPS) {
		nHorEuroform = 3;
		width [0] = 0.500;	width [1] = 0.400;	width [2] = 0.500;	width [3] = 0.0;
		horizontalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.width - 1.350) < EPS) {
		nHorEuroform = 3;
		width [0] = 0.600;	width [1] = 0.300;	width [2] = 0.450;	width [3] = 0.0;
		horizontalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.width - 1.300) < EPS) {
		nHorEuroform = 3;
		width [0] = 0.600;	width [1] = 0.200;	width [2] = 0.500;	width [3] = 0.0;
		horizontalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.width - 1.250) < EPS) {
		nHorEuroform = 3;
		width [0] = 0.600;	width [1] = 0.200;	width [2] = 0.450;	width [3] = 0.0;
		horizontalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.width - 1.200) < EPS) {
		nHorEuroform = 2;
		width [0] = 0.600;	width [1] = 0.600;	width [2] = 0.0;	width [3] = 0.0;
		horizontalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.width - 1.150) < EPS) {
		nHorEuroform = 3;
		width [0] = 0.450;	width [1] = 0.300;	width [2] = 0.400;	width [3] = 0.0;
		horizontalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.width - 1.100) < EPS) {
		nHorEuroform = 3;
		width [0] = 0.400;	width [1] = 0.300;	width [2] = 0.400;	width [3] = 0.0;
		horizontalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.width - 1.050) < EPS) {
		nHorEuroform = 3;
		width [0] = 0.450;	width [1] = 0.300;	width [2] = 0.300;	width [3] = 0.0;
		horizontalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.width - 1.000) < EPS) {
		nHorEuroform = 2;
		width [0] = 0.600;	width [1] = 0.400;	width [2] = 0.0;	width [3] = 0.0;
		horizontalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.width - 0.950) < EPS) {
		nHorEuroform = 2;
		width [0] = 0.450;	width [1] = 0.500;	width [2] = 0.0;	width [3] = 0.0;
		horizontalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.width - 0.900) < EPS) {
		nHorEuroform = 2;
		width [0] = 0.600;	width [1] = 0.300;	width [2] = 0.0;	width [3] = 0.0;
		horizontalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.width - 0.850) < EPS) {
		nHorEuroform = 2;
		width [0] = 0.400;	width [1] = 0.450;	width [2] = 0.0;	width [3] = 0.0;
		horizontalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.width - 0.800) < EPS) {
		nHorEuroform = 2;
		width [0] = 0.400;	width [1] = 0.400;	width [2] = 0.0;	width [3] = 0.0;
		horizontalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.width - 0.750) < EPS) {
		nHorEuroform = 2;
		width [0] = 0.450;	width [1] = 0.300;	width [2] = 0.0;	width [3] = 0.0;
		horizontalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.200;
	} else if (abs (params.width - 0.700) < EPS) {
		nHorEuroform = 2;
		width [0] = 0.400;	width [1] = 0.300;	width [2] = 0.0;	width [3] = 0.0;
		horizontalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.150;
	} else if (abs (params.width - 0.650) < EPS) {
		nHorEuroform = 2;
		width [0] = 0.450;	width [1] = 0.200;	width [2] = 0.0;	width [3] = 0.0;
		horizontalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.200;
		verticalBarRightOffset = 0.150;
	} else if (abs (params.width - 0.600) < EPS) {
		nHorEuroform = 1;
		width [0] = 0.600;	width [1] = 0.0;	width [2] = 0.0;	width [3] = 0.0;
		horizontalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.150;
		verticalBarRightOffset = 0.150;
	} else if (abs (params.width - 0.500) < EPS) {
		nHorEuroform = 1;
		width [0] = 0.500;	width [1] = 0.0;	width [2] = 0.0;	width [3] = 0.0;
		horizontalGap = 0.050;
		nVerticalBar = 1;
		verticalBarLeftOffset = 0.200;
		verticalBarRightOffset = 0.200;
	} else if (abs (params.width - 0.450) < EPS) {
		nHorEuroform = 1;
		width [0] = 0.450;	width [1] = 0.0;	width [2] = 0.0;	width [3] = 0.0;
		horizontalGap = 0.025;
		nVerticalBar = 1;
		verticalBarLeftOffset = 0.200;
		verticalBarRightOffset = 0.200;
	} else if (abs (params.width - 0.400) < EPS) {
		nHorEuroform = 1;
		width [0] = 0.400;	width [1] = 0.0;	width [2] = 0.0;	width [3] = 0.0;
		horizontalGap = 0.050;
		nVerticalBar = 1;
		verticalBarLeftOffset = 0.150;
		verticalBarRightOffset = 0.150;
	} else {
		nHorEuroform = 0;
		width [0] = 0.0;	width [1] = 0.0;	width [2] = 0.0;	width [3] = 0.0;
		nVerticalBar = 0;
		verticalBarLeftOffset = 0.0;
		verticalBarRightOffset = 0.0;
	}

	if (abs (params.height - 6.000) < EPS) {
		nVerEuroform = 5;
		height [0] = 1.200;
		height [1] = 1.200;
		height [2] = 1.200;
		height [3] = 1.200;
		height [4] = 1.200;
	} else if (abs (params.height - 5.700) < EPS) {
		nVerEuroform = 5;
		height [0] = 1.200;
		height [1] = 1.200;
		height [2] = 1.200;
		height [3] = 1.200;
		height [4] = 0.900;
	} else if (abs (params.height - 5.400) < EPS) {
		nVerEuroform = 5;
		height [0] = 1.200;
		height [1] = 1.200;
		height [2] = 1.200;
		height [3] = 0.900;
		height [4] = 0.900;
	} else if (abs (params.height - 5.100) < EPS) {
		nVerEuroform = 5;
		height [0] = 1.200;
		height [1] = 1.200;
		height [2] = 1.200;
		height [3] = 0.900;
		height [4] = 0.600;
	} else if (abs (params.height - 4.800) < EPS) {
		nVerEuroform = 4;
		height [0] = 1.200;
		height [1] = 1.200;
		height [2] = 1.200;
		height [3] = 1.200;
		height [4] = 0.0;
	} else if (abs (params.height - 4.500) < EPS) {
		nVerEuroform = 4;
		height [0] = 1.200;
		height [1] = 1.200;
		height [2] = 1.200;
		height [3] = 0.900;
		height [4] = 0.0;
	} else if (abs (params.height - 4.200) < EPS) {
		nVerEuroform = 4;
		height [0] = 1.200;
		height [1] = 1.200;
		height [2] = 0.900;
		height [3] = 0.900;
		height [4] = 0.0;
	} else if (abs (params.height - 3.900) < EPS) {
		nVerEuroform = 4;
		height [0] = 1.200;
		height [1] = 1.200;
		height [2] = 0.900;
		height [3] = 0.600;
		height [4] = 0.0;
	} else if (abs (params.height - 3.600) < EPS) {
		nVerEuroform = 3;
		height [0] = 1.200;
		height [1] = 1.200;
		height [2] = 1.200;
		height [3] = 0.0;
		height [4] = 0.0;
	} else if (abs (params.height - 3.300) < EPS) {
		nVerEuroform = 3;
		height [0] = 1.200;
		height [1] = 1.200;
		height [2] = 0.900;
		height [3] = 0.0;
		height [4] = 0.0;
	} else if (abs (params.height - 3.000) < EPS) {
		nVerEuroform = 3;
		height [0] = 1.200;
		height [1] = 1.200;
		height [2] = 0.600;
		height [3] = 0.0;
		height [4] = 0.0;
	} else if (abs (params.height - 2.700) < EPS) {
		nVerEuroform = 3;
		height [0] = 1.200;
		height [1] = 0.900;
		height [2] = 0.600;
		height [3] = 0.0;
		height [4] = 0.0;
	} else if (abs (params.height - 2.400) < EPS) {
		nVerEuroform = 2;
		height [0] = 1.200;
		height [1] = 1.200;
		height [2] = 0.0;
		height [3] = 0.0;
		height [4] = 0.0;
	} else if (abs (params.height - 2.100) < EPS) {
		nVerEuroform = 2;
		height [0] = 1.200;
		height [1] = 0.900;
		height [2] = 0.0;
		height [3] = 0.0;
		height [4] = 0.0;
	} else if (abs (params.height - 1.800) < EPS) {
		nVerEuroform = 2;
		height [0] = 0.900;
		height [1] = 0.900;
		height [2] = 0.0;
		height [3] = 0.0;
		height [4] = 0.0;
	} else if (abs (params.height - 1.500) < EPS) {
		nVerEuroform = 2;
		height [0] = 0.900;
		height [1] = 0.600;
		height [2] = 0.0;
		height [3] = 0.0;
		height [4] = 0.0;
	} else {
		nVerEuroform = 0;
		height [0] = 0.0;
		height [1] = 0.0;
		height [2] = 0.0;
		height [3] = 0.0;
		height [4] = 0.0;
	}

	// 너비나 높이가 0이면 아무것도 배치하지 않음
	if ((nHorEuroform == 0) || (nVerEuroform == 0))
		return	NoError;

	//////////////////////////////////////////////////////////////// 현재면
	// 유로폼 설치
	euroform.leftBottomX = params.leftBottomX;
	euroform.leftBottomY = params.leftBottomY;
	euroform.leftBottomZ = params.leftBottomZ;
	euroform.ang = params.ang;
	euroform.eu_stan_onoff = true;
	euroform.u_ins_wall = true;
	euroform.ang_x = DegreeToRad (90.0);

	for (xx = 0 ; xx < nHorEuroform ; ++xx) {
		height_t = 0.0;
		for (yy = 0 ; yy < nVerEuroform ; ++yy) {
			euroform.eu_wid	= euroform.width = width [xx];
			euroform.eu_hei	= euroform.height = height [yy];
			height_t += height [yy];
			elemList.Push (placeEuroform (euroform));
			moveIn3D ('z', euroform.ang, height [yy], &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
		}
		moveIn3D ('x', euroform.ang, width [xx], &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
		moveIn3D ('z', euroform.ang, -height_t, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
	}

	// 비계 파이프 (수평) 배치
	sqrPipe.leftBottomX = params.leftBottomX;
	sqrPipe.leftBottomY = params.leftBottomY;
	sqrPipe.leftBottomZ = params.leftBottomZ;
	sqrPipe.ang = params.ang;
	sqrPipe.length = params.width - (horizontalGap * 2);
	sqrPipe.pipeAng = DegreeToRad (0);

	moveIn3D ('x', sqrPipe.ang, horizontalGap, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
	moveIn3D ('y', sqrPipe.ang, -(0.0635 + 0.025), &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
	moveIn3D ('z', sqrPipe.ang, 0.150 + 0.031, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);

	cylinder.angleFromPlane = DegreeToRad (90.0);
	cylinder.length = 0.050;
	cylinder.radius = 0.013/2;
	cylinder.ang = params.ang;
	cylinder.leftBottomX = sqrPipe.leftBottomX;
	cylinder.leftBottomY = sqrPipe.leftBottomY;
	cylinder.leftBottomZ = sqrPipe.leftBottomZ;

	for (xx = 0 ; xx <= nVerEuroform ; ++xx) {
		if (xx == 0) {
			// 1행
			tempGuid = placeSqrPipe (sqrPipe);
			elemList.Push (tempGuid);
			cylinder.leftBottomX = sqrPipe.leftBottomX;
			cylinder.leftBottomY = sqrPipe.leftBottomY;
			cylinder.leftBottomZ = sqrPipe.leftBottomZ;
			moveIn3D ('z', cylinder.ang, -0.025, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			for (yy = 0 ; yy < 6 ; ++yy) {
				moveIn3D ('x', cylinder.ang, 0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				elemList.Push (placeHole (tempGuid, cylinder));
			}
			moveIn3D ('x', cylinder.ang, -0.300 + params.width - (horizontalGap * 2), &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			for (yy = 0 ; yy < 6 ; ++yy) {
				moveIn3D ('x', cylinder.ang, -0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				elemList.Push (placeHole (tempGuid, cylinder));
			}
			moveIn3D ('z', sqrPipe.ang, -0.031 - 0.150 + height [xx], &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
			moveIn3D ('x', cylinder.ang, 0.300 - params.width + (horizontalGap * 2), &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
		} else if (xx == nVerEuroform) {
			// 마지막 행
			moveIn3D ('z', sqrPipe.ang, -0.150 + 0.031, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
			tempGuid = placeSqrPipe (sqrPipe);
			elemList.Push (tempGuid);
			cylinder.leftBottomX = sqrPipe.leftBottomX;
			cylinder.leftBottomY = sqrPipe.leftBottomY;
			cylinder.leftBottomZ = sqrPipe.leftBottomZ;
			moveIn3D ('z', cylinder.ang, -0.025, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			for (yy = 0 ; yy < 6 ; ++yy) {
				moveIn3D ('x', cylinder.ang, 0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				elemList.Push (placeHole (tempGuid, cylinder));
			}
			moveIn3D ('x', cylinder.ang, -0.300 + params.width - (horizontalGap * 2), &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			for (yy = 0 ; yy < 6 ; ++yy) {
				moveIn3D ('x', cylinder.ang, -0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				elemList.Push (placeHole (tempGuid, cylinder));
			}
		} else {
			// 나머지 행
			tempGuid = placeSqrPipe (sqrPipe);
			elemList.Push (tempGuid);
			cylinder.leftBottomX = sqrPipe.leftBottomX;
			cylinder.leftBottomY = sqrPipe.leftBottomY;
			cylinder.leftBottomZ = sqrPipe.leftBottomZ;
			moveIn3D ('z', cylinder.ang, -0.025, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			for (yy = 0 ; yy < 6 ; ++yy) {
				moveIn3D ('x', cylinder.ang, 0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				elemList.Push (placeHole (tempGuid, cylinder));
			}
			moveIn3D ('x', cylinder.ang, -0.300 + params.width - (horizontalGap * 2), &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			for (yy = 0 ; yy < 6 ; ++yy) {
				moveIn3D ('x', cylinder.ang, -0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				elemList.Push (placeHole (tempGuid, cylinder));
			}
			moveIn3D ('z', sqrPipe.ang, height [xx], &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
			moveIn3D ('x', cylinder.ang, 0.300 - params.width + (horizontalGap * 2), &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
		}
	}

	// 비계 파이프 (수직) 배치
	sqrPipe.leftBottomX = params.leftBottomX;
	sqrPipe.leftBottomY = params.leftBottomY;
	sqrPipe.leftBottomZ = params.leftBottomZ;
	sqrPipe.ang = params.ang;
	sqrPipe.length = params.height - 0.100;
	sqrPipe.pipeAng = DegreeToRad (90);

	moveIn3D ('x', sqrPipe.ang, horizontalGap + verticalBarLeftOffset, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
	moveIn3D ('y', sqrPipe.ang, -(0.0635 + 0.075), &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
	moveIn3D ('z', sqrPipe.ang, 0.050, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);

	// 1열
	cylinder.angleFromPlane = DegreeToRad (0.0);
	cylinder.length = 0.050;
	cylinder.radius = 0.013/2;
	cylinder.ang = params.ang;
	cylinder.leftBottomX = sqrPipe.leftBottomX;
	cylinder.leftBottomY = sqrPipe.leftBottomY;
	cylinder.leftBottomZ = sqrPipe.leftBottomZ;

	tempGuid = placeSqrPipe (sqrPipe);
	elemList.Push (tempGuid);
	moveIn3D ('x', cylinder.ang, -0.025, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
	for (xx = 0 ; xx < 6 ; ++xx) {
		moveIn3D ('z', cylinder.ang, 0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
		elemList.Push (placeHole (tempGuid, cylinder));
	}
	moveIn3D ('z', cylinder.ang, -0.300 + params.height - 0.100, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
	for (xx = 0 ; xx < 6 ; ++xx) {
		moveIn3D ('z', cylinder.ang, -0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
		elemList.Push (placeHole (tempGuid, cylinder));
	}
	moveIn3D ('x', sqrPipe.ang, -(horizontalGap + verticalBarLeftOffset) + params.width - (horizontalGap + verticalBarRightOffset), &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);

	if (nVerticalBar > 1) {
		// 2열
		tempGuid = placeSqrPipe (sqrPipe);
		elemList.Push (tempGuid);
		cylinder.leftBottomX = sqrPipe.leftBottomX;
		cylinder.leftBottomY = sqrPipe.leftBottomY;
		cylinder.leftBottomZ = sqrPipe.leftBottomZ;
		moveIn3D ('x', cylinder.ang, -0.025, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
		for (xx = 0 ; xx < 6 ; ++xx) {
			moveIn3D ('z', cylinder.ang, 0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			elemList.Push (placeHole (tempGuid, cylinder));
		}
		moveIn3D ('z', cylinder.ang, -0.300 + params.height - 0.100, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
		for (xx = 0 ; xx < 6 ; ++xx) {
			moveIn3D ('z', cylinder.ang, -0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			elemList.Push (placeHole (tempGuid, cylinder));
		}
	}

	// 유로폼 후크 배치 (수평 - 최하단, 최상단)
	hook.leftBottomX = params.leftBottomX;
	hook.leftBottomY = params.leftBottomY;
	hook.leftBottomZ = params.leftBottomZ;
	hook.ang = params.ang;
	hook.iHookType = 2;
	hook.iHookShape = 2;
	hook.angX = DegreeToRad (0.0);
	hook.angY = DegreeToRad (90.0);

	moveIn3D ('y', hook.ang, -0.0885, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);

	if (nHorEuroform >= 2) {
		moveIn3D ('x', hook.ang, width [0], &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);
		moveIn3D ('z', hook.ang, 0.030 + 0.150, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);
		// 1행
		width_t = 0.0;
		for (xx = 1 ; xx < nHorEuroform ; ++xx) {
			width_t += width [xx];
			elemList.Push (placeEuroformHook (hook));
			moveIn3D ('x', hook.ang, width [xx], &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);
		}
		moveIn3D ('x', hook.ang, -width_t, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);
		moveIn3D ('z', hook.ang, -0.150 + params.height - 0.150, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);

		// 마지막 행
		for (xx = 1 ; xx < nHorEuroform ; ++xx) {
			width_t += width [xx];
			elemList.Push (placeEuroformHook (hook));
			moveIn3D ('x', hook.ang, width [xx], &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);
		}
	}

	// 각파이프행거 배치 (수평 - 최하단, 최상단을 제외한 나머지)
	hanger.leftBottomX = params.leftBottomX;
	hanger.leftBottomY = params.leftBottomY;
	hanger.leftBottomZ = params.leftBottomZ;
	hanger.ang = params.ang;
	hanger.angX = DegreeToRad (0.0);
	hanger.angY = DegreeToRad (270.0);

	moveIn3D ('y', hanger.ang, -0.0635, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);

	// 2 ~ [n-1]행
	if (nHorEuroform >= 2) {
		moveIn3D ('x', hanger.ang, 0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
		moveIn3D ('z', hanger.ang, height [0], &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
		for (xx = 1 ; xx < nVerEuroform ; ++xx) {
			width_t = 0.0;
			for (yy = 0 ; yy < nHorEuroform ; ++yy) {
				// 1열
				if (yy == 0) {
					elemList.Push (placeRectpipeHanger (hanger));
					moveIn3D ('x', hanger.ang, width [0] - 0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
					width_t += width [0] - 0.150;
				// 마지막 열
				} else if (yy == nHorEuroform - 1) {
					width_t += width [nHorEuroform-1] - 0.150;
					moveIn3D ('x', hanger.ang, width [nHorEuroform-1] - 0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
					elemList.Push (placeRectpipeHanger (hanger));
				// 나머지 열
				} else {
					width_t += width [yy];
					if (abs (width [yy] - 0.600) < EPS) {
						moveIn3D ('x', hanger.ang, 0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						elemList.Push (placeRectpipeHanger (hanger));
						moveIn3D ('x', hanger.ang, 0.300, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						elemList.Push (placeRectpipeHanger (hanger));
						moveIn3D ('x', hanger.ang, 0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
					} else if (abs (width [yy] - 0.500) < EPS) {
						moveIn3D ('x', hanger.ang, 0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						elemList.Push (placeRectpipeHanger (hanger));
						moveIn3D ('x', hanger.ang, 0.200, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						elemList.Push (placeRectpipeHanger (hanger));
						moveIn3D ('x', hanger.ang, 0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
					} else if (abs (width [yy] - 0.450) < EPS) {
						moveIn3D ('x', hanger.ang, 0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						elemList.Push (placeRectpipeHanger (hanger));
						moveIn3D ('x', hanger.ang, 0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						elemList.Push (placeRectpipeHanger (hanger));
						moveIn3D ('x', hanger.ang, 0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
					} else if (abs (width [yy] - 0.400) < EPS) {
						moveIn3D ('x', hanger.ang, 0.100, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						elemList.Push (placeRectpipeHanger (hanger));
						moveIn3D ('x', hanger.ang, 0.200, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						elemList.Push (placeRectpipeHanger (hanger));
						moveIn3D ('x', hanger.ang, 0.100, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
					} else if (abs (width [yy] - 0.300) < EPS) {
						moveIn3D ('x', hanger.ang, 0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						elemList.Push (placeRectpipeHanger (hanger));
						moveIn3D ('x', hanger.ang, 0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
					} else if (abs (width [yy] - 0.200) < EPS) {
						moveIn3D ('x', hanger.ang, 0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						elemList.Push (placeRectpipeHanger (hanger));
						moveIn3D ('x', hanger.ang, 0.050, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
					}
				}
			}
			moveIn3D ('x', hanger.ang, -width_t, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
			moveIn3D ('z', hanger.ang, height [xx], &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
		}
	}

	// 헤드 피스
	headpiece.leftBottomX = params.leftBottomX;
	headpiece.leftBottomY = params.leftBottomY;
	headpiece.leftBottomZ = params.leftBottomZ;
	headpiece.ang = params.ang;

	moveIn3D ('x', headpiece.ang, horizontalGap + verticalBarLeftOffset - 0.0475, &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
	moveIn3D ('y', headpiece.ang, -0.2685, &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
	moveIn3D ('z', headpiece.ang, 0.291, &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);

	// 처음 행
	elemList.Push (placeJointHeadpeace_hor (headpiece));
	moveIn3D ('x', headpiece.ang, -(horizontalGap + verticalBarLeftOffset - 0.0475) + params.width - (horizontalGap + verticalBarRightOffset + 0.0475), &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
	if (nVerticalBar > 1)
		elemList.Push (placeJointHeadpeace_ver (headpiece));
	moveIn3D ('x', headpiece.ang, (horizontalGap + verticalBarLeftOffset - 0.0475) - params.width + (horizontalGap + verticalBarRightOffset + 0.0475), &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
	//if (cell.verLen > 4.000) {
	//	elev_headpiece = 4.000 * 0.80;
	//} else {
	//	elev_headpiece = cell.verLen * 0.80;
	//}
	elev_headpiece = 1.900;
	moveIn3D ('z', headpiece.ang, elev_headpiece, &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
	// 마지막 행
		elemList.Push (placeJointHeadpeace_ver (headpiece));
		moveIn3D ('x', headpiece.ang, -(horizontalGap + verticalBarLeftOffset - 0.0475) + params.width - (horizontalGap + verticalBarRightOffset + 0.0475), &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
	if (nVerticalBar > 1)
		elemList.Push (placeJointHeadpeace_ver (headpiece));

	// 결합철물
	fittings.leftBottomX = params.leftBottomX;
	fittings.leftBottomY = params.leftBottomY;
	fittings.leftBottomZ = params.leftBottomZ;
	fittings.ang = params.ang;
	fittings.angX = DegreeToRad (180.0);
	fittings.angY = DegreeToRad (0.0);
	
	moveIn3D ('x', fittings.ang, horizontalGap + verticalBarLeftOffset - 0.081, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
	moveIn3D ('y', fittings.ang, -0.1155, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
	moveIn3D ('z', fittings.ang, 0.230, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);

	// 처음 열
	for (xx = 0 ; xx <= nVerEuroform ; ++xx) {
		// 1행
		if (xx == 0) {
			elemList.Push (placeFittings (fittings));
			moveIn3D ('z', fittings.ang, height [xx] - 0.180, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
		// 마지막 행
		} else if (xx == nVerEuroform) {
			moveIn3D ('z', fittings.ang, -0.120, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
			elemList.Push (placeFittings (fittings));
		// 나머지 행
		} else {
			elemList.Push (placeFittings (fittings));
			moveIn3D ('z', fittings.ang, height [xx], &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
		}
	}
	moveIn3D ('x', fittings.ang, -(horizontalGap + verticalBarLeftOffset - 0.081) + params.width - (horizontalGap + verticalBarRightOffset + 0.081), &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
	moveIn3D ('z', fittings.ang, 0.300 - params.height, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);

	if (nVerticalBar > 1) {
		// 마지막 열
		for (xx = 0 ; xx <= nVerEuroform ; ++xx) {
			// 1행
			if (xx == 0) {
				elemList.Push (placeFittings (fittings));
				moveIn3D ('z', fittings.ang, height [xx] - 0.180, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
			// 마지막 행
			} else if (xx == nVerEuroform) {
				moveIn3D ('z', fittings.ang, -0.120, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
				elemList.Push (placeFittings (fittings));
			// 나머지 행
			} else {
				elemList.Push (placeFittings (fittings));
				moveIn3D ('z', fittings.ang, height [xx], &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
			}
		}
	}

	// 그룹화하기
	if (!elemList.IsEmpty ()) {
		GSSize nElems = elemList.GetSize ();
		API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
		if (elemHead != NULL) {
			for (GSIndex i = 0; i < nElems; i++)
				(*elemHead)[i].guid = elemList[i];

			ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

			BMKillHandle ((GSHandle *) &elemHead);
		}
	}
	elemList.Clear (false);

	//////////////////////////////////////////////////////////////// 반대면
	if (bDoubleSide) {
		moveIn3D ('x', params.ang, params.width, &params.leftBottomX, &params.leftBottomY, &params.leftBottomY);
		moveIn3D ('y', params.ang, wallThk, &params.leftBottomX, &params.leftBottomY, &params.leftBottomY);
		params.ang += DegreeToRad (180.0);

		// 유로폼 설치 (반대편에서 변경됨)
		euroform.leftBottomX = params.leftBottomX;
		euroform.leftBottomY = params.leftBottomY;
		euroform.leftBottomZ = params.leftBottomZ;
		euroform.ang = params.ang;
		euroform.u_ins_wall = true;

		for (xx = nHorEuroform - 1 ; xx >= 0 ; --xx) {
			height_t = 0.0;
			for (yy = 0 ; yy < nVerEuroform ; ++yy) {
				euroform.eu_wid = euroform.width	= width [xx];
				euroform.eu_hei = euroform.height	= height [yy];
				height_t += height [yy];
				elemList.Push (placeEuroform (euroform));
				moveIn3D ('z', euroform.ang, height [yy], &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
			}
			moveIn3D ('x', euroform.ang, width [xx], &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
			moveIn3D ('z', euroform.ang, -height_t, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
		}

		// 비계 파이프 (수평) 배치
		sqrPipe.leftBottomX = params.leftBottomX;
		sqrPipe.leftBottomY = params.leftBottomY;
		sqrPipe.leftBottomZ = params.leftBottomZ;
		sqrPipe.ang = params.ang;
		sqrPipe.length = params.width - (horizontalGap * 2);
		sqrPipe.pipeAng = DegreeToRad (0);

		moveIn3D ('x', sqrPipe.ang, horizontalGap, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
		moveIn3D ('y', sqrPipe.ang, -(0.0635 + 0.025), &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
		moveIn3D ('z', sqrPipe.ang, 0.150 + 0.031, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);

		cylinder.angleFromPlane = DegreeToRad (90.0);
		cylinder.length = 0.050;
		cylinder.radius = 0.013/2;
		cylinder.ang = params.ang;
		cylinder.leftBottomX = sqrPipe.leftBottomX;
		cylinder.leftBottomY = sqrPipe.leftBottomY;
		cylinder.leftBottomZ = sqrPipe.leftBottomZ;
		moveIn3D ('z', cylinder.ang, -0.025, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);

		for (xx = 0 ; xx <= nVerEuroform ; ++xx) {
			if (xx == 0) {
				// 1행
				tempGuid = placeSqrPipe (sqrPipe);
				elemList.Push (tempGuid);
				cylinder.leftBottomX = sqrPipe.leftBottomX;
				cylinder.leftBottomY = sqrPipe.leftBottomY;
				cylinder.leftBottomZ = sqrPipe.leftBottomZ;
				moveIn3D ('z', cylinder.ang, -0.025, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				for (yy = 0 ; yy < 6 ; ++yy) {
					moveIn3D ('x', cylinder.ang, 0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
					elemList.Push (placeHole (tempGuid, cylinder));
				}
				moveIn3D ('x', cylinder.ang, -0.300 + params.width - (horizontalGap * 2), &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				for (yy = 0 ; yy < 6 ; ++yy) {
					moveIn3D ('x', cylinder.ang, -0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
					elemList.Push (placeHole (tempGuid, cylinder));
				}
				moveIn3D ('z', sqrPipe.ang, -0.031 - 0.150 + height [xx], &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
				moveIn3D ('x', cylinder.ang, 0.300 - params.width + (horizontalGap * 2), &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			} else if (xx == nVerEuroform) {
				// 마지막 행
				moveIn3D ('z', sqrPipe.ang, -0.150 + 0.031, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
				tempGuid = placeSqrPipe (sqrPipe);
				elemList.Push (tempGuid);
				cylinder.leftBottomX = sqrPipe.leftBottomX;
				cylinder.leftBottomY = sqrPipe.leftBottomY;
				cylinder.leftBottomZ = sqrPipe.leftBottomZ;
				moveIn3D ('z', cylinder.ang, -0.025, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				for (yy = 0 ; yy < 6 ; ++yy) {
					moveIn3D ('x', cylinder.ang, 0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
					elemList.Push (placeHole (tempGuid, cylinder));
				}
				moveIn3D ('x', cylinder.ang, -0.300 + params.width - (horizontalGap * 2), &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				for (yy = 0 ; yy < 6 ; ++yy) {
					moveIn3D ('x', cylinder.ang, -0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
					elemList.Push (placeHole (tempGuid, cylinder));
				}
			} else {
				// 나머지 행
				tempGuid = placeSqrPipe (sqrPipe);
				elemList.Push (tempGuid);
				cylinder.leftBottomX = sqrPipe.leftBottomX;
				cylinder.leftBottomY = sqrPipe.leftBottomY;
				cylinder.leftBottomZ = sqrPipe.leftBottomZ;
				moveIn3D ('z', cylinder.ang, -0.025, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				for (yy = 0 ; yy < 6 ; ++yy) {
					moveIn3D ('x', cylinder.ang, 0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
					elemList.Push (placeHole (tempGuid, cylinder));
				}
				moveIn3D ('x', cylinder.ang, -0.300 + params.width - (horizontalGap * 2), &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				for (yy = 0 ; yy < 6 ; ++yy) {
					moveIn3D ('x', cylinder.ang, -0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
					elemList.Push (placeHole (tempGuid, cylinder));
				}
				moveIn3D ('z', sqrPipe.ang, height [xx], &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
				moveIn3D ('x', cylinder.ang, 0.300 - params.width + (horizontalGap * 2), &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			}
		}

		// 비계 파이프 (수직) 배치
		sqrPipe.leftBottomX = params.leftBottomX;
		sqrPipe.leftBottomY = params.leftBottomY;
		sqrPipe.leftBottomZ = params.leftBottomZ;
		sqrPipe.ang = params.ang;
		sqrPipe.length = params.height - 0.100;
		sqrPipe.pipeAng = DegreeToRad (90);

		moveIn3D ('x', sqrPipe.ang, horizontalGap + verticalBarRightOffset, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
		moveIn3D ('y', sqrPipe.ang, -(0.0635 + 0.075), &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
		moveIn3D ('z', sqrPipe.ang, 0.050, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);

		// 1열
		cylinder.angleFromPlane = DegreeToRad (0.0);
		cylinder.length = 0.050;
		cylinder.radius = 0.013/2;
		cylinder.ang = params.ang;
		cylinder.leftBottomX = sqrPipe.leftBottomX;
		cylinder.leftBottomY = sqrPipe.leftBottomY;
		cylinder.leftBottomZ = sqrPipe.leftBottomZ;

		tempGuid = placeSqrPipe (sqrPipe);
		elemList.Push (tempGuid);
		moveIn3D ('x', cylinder.ang, -0.025, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
		for (xx = 0 ; xx < 6 ; ++xx) {
			moveIn3D ('z', cylinder.ang, 0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			elemList.Push (placeHole (tempGuid, cylinder));
		}
		moveIn3D ('z', cylinder.ang, -0.300 + params.height - 0.100, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
		for (xx = 0 ; xx < 6 ; ++xx) {
			moveIn3D ('z', cylinder.ang, -0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			elemList.Push (placeHole (tempGuid, cylinder));
		}
		moveIn3D ('x', sqrPipe.ang, -(horizontalGap + verticalBarRightOffset) + params.width - (horizontalGap + verticalBarLeftOffset), &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);

		if (nVerticalBar > 1) {
			// 2열
			tempGuid = placeSqrPipe (sqrPipe);
			elemList.Push (tempGuid);
			cylinder.leftBottomX = sqrPipe.leftBottomX;
			cylinder.leftBottomY = sqrPipe.leftBottomY;
			cylinder.leftBottomZ = sqrPipe.leftBottomZ;
			moveIn3D ('x', cylinder.ang, -0.025, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			for (xx = 0 ; xx < 6 ; ++xx) {
				moveIn3D ('z', cylinder.ang, 0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				elemList.Push (placeHole (tempGuid, cylinder));
			}
			moveIn3D ('z', cylinder.ang, -0.300 + params.height - 0.100, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			for (xx = 0 ; xx < 6 ; ++xx) {
				moveIn3D ('z', cylinder.ang, -0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				elemList.Push (placeHole (tempGuid, cylinder));
			}
		}

		// 유로폼 후크 배치 (수평 - 최하단, 최상단)
		hook.leftBottomX = params.leftBottomX;
		hook.leftBottomY = params.leftBottomY;
		hook.leftBottomZ = params.leftBottomZ;
		hook.ang = params.ang;
		hook.iHookType = 2;
		hook.iHookShape = 2;
		hook.angX = DegreeToRad (0.0);
		hook.angY = DegreeToRad (90.0);

		moveIn3D ('y', hook.ang, -0.0885, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);

		if (nHorEuroform >= 2) {
			moveIn3D ('x', hook.ang, width [nHorEuroform-1], &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);
			moveIn3D ('z', hook.ang, 0.030 + 0.150, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);
			// 1행
			width_t = 0.0;
			for (xx = nHorEuroform-2 ; xx >= 0 ; --xx) {
				width_t += width [xx];
				elemList.Push (placeEuroformHook (hook));
				moveIn3D ('x', hook.ang, width [xx], &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);
			}
			moveIn3D ('x', hook.ang, -width_t, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);
			moveIn3D ('z', hook.ang, -0.150 + params.height - 0.150, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);

			// 마지막 행
			for (xx = nHorEuroform-2 ; xx >= 0 ; --xx) {
				width_t += width [xx];
				elemList.Push (placeEuroformHook (hook));
				moveIn3D ('x', hook.ang, width [xx], &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);
			}
		}

		// 각파이프행거 배치 (수평 - 최하단, 최상단을 제외한 나머지)
		hanger.leftBottomX = params.leftBottomX;
		hanger.leftBottomY = params.leftBottomY;
		hanger.leftBottomZ = params.leftBottomZ;
		hanger.ang = params.ang;
		hanger.angX = DegreeToRad (0.0);
		hanger.angY = DegreeToRad (270.0);

		moveIn3D ('y', hanger.ang, -0.0635, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);

		// 2 ~ [n-1]행
		if (nHorEuroform >= 2) {
			moveIn3D ('x', hanger.ang, 0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
			moveIn3D ('z', hanger.ang, height [0], &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
			for (xx = 1 ; xx < nVerEuroform ; ++xx) {
				width_t = 0.0;
				for (yy = nHorEuroform-1 ; yy >= 0 ; --yy) {
					// 1열
					if (yy == nHorEuroform-1) {
						elemList.Push (placeRectpipeHanger (hanger));
						moveIn3D ('x', hanger.ang, width [nHorEuroform-1] - 0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						width_t += width [0] - 0.150;
					// 마지막 열
					} else if (yy == 0) {
						width_t += width [nHorEuroform-1] - 0.150;
						moveIn3D ('x', hanger.ang, width [0] - 0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						elemList.Push (placeRectpipeHanger (hanger));
					// 나머지 열
					} else {
						width_t += width [yy];
						if (abs (width [yy] - 0.600) < EPS) {
							moveIn3D ('x', hanger.ang, 0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
							elemList.Push (placeRectpipeHanger (hanger));
							moveIn3D ('x', hanger.ang, 0.300, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
							elemList.Push (placeRectpipeHanger (hanger));
							moveIn3D ('x', hanger.ang, 0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						} else if (abs (width [yy] - 0.500) < EPS) {
							moveIn3D ('x', hanger.ang, 0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
							elemList.Push (placeRectpipeHanger (hanger));
							moveIn3D ('x', hanger.ang, 0.200, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
							elemList.Push (placeRectpipeHanger (hanger));
							moveIn3D ('x', hanger.ang, 0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						} else if (abs (width [yy] - 0.450) < EPS) {
							moveIn3D ('x', hanger.ang, 0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
							elemList.Push (placeRectpipeHanger (hanger));
							moveIn3D ('x', hanger.ang, 0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
							elemList.Push (placeRectpipeHanger (hanger));
							moveIn3D ('x', hanger.ang, 0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						} else if (abs (width [yy] - 0.400) < EPS) {
							moveIn3D ('x', hanger.ang, 0.100, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
							elemList.Push (placeRectpipeHanger (hanger));
							moveIn3D ('x', hanger.ang, 0.200, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
							elemList.Push (placeRectpipeHanger (hanger));
							moveIn3D ('x', hanger.ang, 0.100, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						} else if (abs (width [yy] - 0.300) < EPS) {
							moveIn3D ('x', hanger.ang, 0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
							elemList.Push (placeRectpipeHanger (hanger));
							moveIn3D ('x', hanger.ang, 0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						} else if (abs (width [yy] - 0.200) < EPS) {
							moveIn3D ('x', hanger.ang, 0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
							elemList.Push (placeRectpipeHanger (hanger));
							moveIn3D ('x', hanger.ang, 0.050, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						}
					}
				}
				moveIn3D ('x', hanger.ang, -width_t, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
				moveIn3D ('z', hanger.ang, height [xx], &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
			}
		}

		// 헤드 피스
		headpiece.leftBottomX = params.leftBottomX;
		headpiece.leftBottomY = params.leftBottomY;
		headpiece.leftBottomZ = params.leftBottomZ;
		headpiece.ang = params.ang;

		moveIn3D ('x', headpiece.ang, horizontalGap + verticalBarRightOffset - 0.0475, &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
		moveIn3D ('y', headpiece.ang, -0.2685, &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
		moveIn3D ('z', headpiece.ang, 0.291, &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);

		// 처음 행
		elemList.Push (placeJointHeadpeace_hor (headpiece));
		moveIn3D ('x', headpiece.ang, -(horizontalGap + verticalBarRightOffset - 0.0475) + params.width - (horizontalGap + verticalBarLeftOffset + 0.0475), &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
		if (nVerticalBar > 1)
			elemList.Push (placeJointHeadpeace_hor (headpiece));
		moveIn3D ('x', headpiece.ang, (horizontalGap + verticalBarRightOffset - 0.0475) - params.width + (horizontalGap + verticalBarLeftOffset + 0.0475), &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
		//if (cell.verLen > 4.000) {
		//	elev_headpiece = 4.000 * 0.80;
		//} else {
		//	elev_headpiece = cell.verLen * 0.80;
		//}
		elev_headpiece = 1.900;
		moveIn3D ('z', headpiece.ang, elev_headpiece, &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
		// 마지막 행
		elemList.Push (placeJointHeadpeace_hor (headpiece));
		moveIn3D ('x', headpiece.ang, -(horizontalGap + verticalBarRightOffset - 0.0475) + params.width - (horizontalGap + verticalBarLeftOffset + 0.0475), &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
		if (nVerticalBar > 1)
			elemList.Push (placeJointHeadpeace_hor (headpiece));

		// 결합철물
		fittings.leftBottomX = params.leftBottomX;
		fittings.leftBottomY = params.leftBottomY;
		fittings.leftBottomZ = params.leftBottomZ;
		fittings.ang = params.ang;
		fittings.angX = DegreeToRad (180.0);
		fittings.angY = DegreeToRad (0.0);
	
		moveIn3D ('x', fittings.ang, horizontalGap + verticalBarRightOffset - 0.081, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
		moveIn3D ('y', fittings.ang, -0.1155, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
		moveIn3D ('z', fittings.ang, 0.230, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);

		// 처음 열
		for (xx = 0 ; xx <= nVerEuroform ; ++xx) {
			// 1행
			if (xx == 0) {
				elemList.Push (placeFittings (fittings));
				moveIn3D ('z', fittings.ang, height [xx] - 0.180, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
			// 마지막 행
			} else if (xx == nVerEuroform) {
				moveIn3D ('z', fittings.ang, -0.120, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
				elemList.Push (placeFittings (fittings));
			// 나머지 행
			} else {
				elemList.Push (placeFittings (fittings));
				moveIn3D ('z', fittings.ang, height [xx], &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
			}
		}

		moveIn3D ('x', fittings.ang, -(horizontalGap + verticalBarRightOffset - 0.081) + params.width - (horizontalGap + verticalBarLeftOffset + 0.081), &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
		moveIn3D ('z', fittings.ang, 0.300 - params.height, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);

		if (nVerticalBar > 1) {
			// 마지막 열
			for (xx = 0 ; xx <= nVerEuroform ; ++xx) {
				// 1행
				if (xx == 0) {
					elemList.Push (placeFittings (fittings));
					moveIn3D ('z', fittings.ang, height [xx] - 0.180, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
				// 마지막 행
				} else if (xx == nVerEuroform) {
					moveIn3D ('z', fittings.ang, -0.120, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
					elemList.Push (placeFittings (fittings));
				// 나머지 행
				} else {
					elemList.Push (placeFittings (fittings));
					moveIn3D ('z', fittings.ang, height [xx], &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
				}
			}
		}

		// 그룹화하기
		if (!elemList.IsEmpty ()) {
			GSSize nElems = elemList.GetSize ();
			API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
			if (elemHead != NULL) {
				for (GSIndex i = 0; i < nElems; i++)
					(*elemHead)[i].guid = elemList[i];

				ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

				BMKillHandle ((GSHandle *) &elemHead);
			}
		}
		elemList.Clear (false);
	}

	return	err;
}

// 테이블폼(벽) 배치 (벽눕히기) 타입2
GSErrCode	placeTableformOnWall_landscape_Type2 (WallTableform params)
{
	// 가로, 세로가 뒤바뀌어야 함
	exchangeDoubles (&params.width, &params.height);

	GSErrCode	err = NoError;

	short	nHorEuroform;			// 수평 방향 유로폼 개수
	short	nVerEuroform;			// 수직 방향 유로폼 개수
	double	width [7];				// 수평 방향 각 유로폼 너비
	double	height [7];				// 수직 방향 각 유로폼 높이

	short	nVerticalBar;
	double	verticalBarLeftOffset;
	double	verticalBarRightOffset;

	short		xx, yy;
	double		height_t;
	double		elev_headpiece;
	double		verticalGap = 0.050;	// 수직재 양쪽 이격거리
	API_Guid	tempGuid;
	Cylinder	cylinder;

	Euroform		euroform;
	SquarePipe		sqrPipe;
	HeadpieceOfPushPullProps	headpiece;
	MetalFittings	fittings;
	EuroformHook	hook;
	RectPipeHanger	hanger;

	nHorEuroform = 0;
	nVerEuroform = 0;
	for (xx = 0 ; xx < 7 ; ++xx) {
		width [xx] = 0.0;
		height [xx] = 0.0;
	}

	if (abs (params.height - 2.300) < EPS) {
		nVerEuroform = 4;
		//height [0] = 0.600;		height [1] = 0.600;		height [2] = 0.500;		height [3] = 0.600;
		height [0] = 0.500;		height [1] = 0.600;		height [2] = 0.600;		height [3] = 0.600;
		verticalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.height - 2.250) < EPS) {
		nVerEuroform = 4;
		height [0] = 0.600;		height [1] = 0.600;		height [2] = 0.450;		height [3] = 0.600;
		verticalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.height - 2.200) < EPS) {
		nVerEuroform = 4;
		//height [0] = 0.600;		height [1] = 0.600;		height [2] = 0.400;		height [3] = 0.600;
		height [0] = 0.400;		height [1] = 0.600;		height [2] = 0.600;		height [3] = 0.600;
		verticalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.height - 2.150) < EPS) {
		nVerEuroform = 4;
		//height [0] = 0.600;		height [1] = 0.500;		height [2] = 0.450;		height [3] = 0.600;
		height [0] = 0.500;		height [1] = 0.600;		height [2] = 0.450;		height [3] = 0.600;
		verticalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.height - 2.100) < EPS) {
		nVerEuroform = 4;
		height [0] = 0.600;		height [1] = 0.600;		height [2] = 0.300;		height [3] = 0.600;
		verticalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.height - 2.050) < EPS) {
		nVerEuroform = 4;
		//height [0] = 0.600;		height [1] = 0.450;		height [2] = 0.400;		height [3] = 0.600;
		height [0] = 0.400;		height [1] = 0.450;		height [2] = 0.600;		height [3] = 0.600;
		verticalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.height - 2.000) < EPS) {
		nVerEuroform = 4;
		//height [0] = 0.600;		height [1] = 0.600;		height [2] = 0.200;		height [3] = 0.600;
		height [0] = 0.200;		height [1] = 0.600;		height [2] = 0.600;		height [3] = 0.600;
		verticalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.height - 1.950) < EPS) {
		nVerEuroform = 4;
		height [0] = 0.600;		height [1] = 0.450;		height [2] = 0.300;		height [3] = 0.600;
		verticalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.height - 1.900) < EPS) {
		nVerEuroform = 4;
		//height [0] = 0.600;		height [1] = 0.500;		height [2] = 0.200;		height [3] = 0.600;
		height [0] = 0.500;		height [1] = 0.200;		height [2] = 0.600;		height [3] = 0.600;
		verticalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.height - 1.850) < EPS) {
		nVerEuroform = 4;
		//height [0] = 0.600;		height [1] = 0.450;		height [2] = 0.200;		height [3] = 0.600;
		height [0] = 0.200;		height [1] = 0.450;		height [2] = 0.600;		height [3] = 0.600;
		verticalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.height - 1.800) < EPS) {
		nVerEuroform = 3;
		height [0] = 0.600;		height [1] = 0.600;		height [2] = 0.600;		height [3] = 0.0;
		verticalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.height - 1.750) < EPS) {
		nVerEuroform = 4;
		//height [0] = 0.600;		height [1] = 0.200;		height [2] = 0.450;		height [3] = 0.500;
		height [0] = 0.500;		height [1] = 0.200;		height [2] = 0.450;		height [3] = 0.600;
		verticalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.height - 1.700) < EPS) {
		nVerEuroform = 3;
		//height [0] = 0.600;		height [1] = 0.500;		height [2] = 0.600;		height [3] = 0.0;
		height [0] = 0.500;		height [1] = 0.600;		height [2] = 0.600;		height [3] = 0.0;
		verticalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.height - 1.650) < EPS) {
		nVerEuroform = 3;
		height [0] = 0.600;		height [1] = 0.450;		height [2] = 0.600;		height [3] = 0.0;
		verticalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.height - 1.600) < EPS) {
		nVerEuroform = 3;
		//height [0] = 0.600;		height [1] = 0.400;		height [2] = 0.600;		height [3] = 0.0;
		height [0] = 0.400;		height [1] = 0.600;		height [2] = 0.600;		height [3] = 0.0;
		verticalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.height - 1.550) < EPS) {
		nVerEuroform = 3;
		//height [0] = 0.600;		height [1] = 0.450;		height [2] = 0.500;		height [3] = 0.0;
		height [0] = 0.500;		height [1] = 0.450;		height [2] = 0.600;		height [3] = 0.0;
		verticalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.height - 1.500) < EPS) {
		nVerEuroform = 3;
		height [0] = 0.600;		height [1] = 0.300;		height [2] = 0.600;		height [3] = 0.0;
		verticalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.height - 1.450) < EPS) {
		nVerEuroform = 3;
		//height [0] = 0.500;		height [1] = 0.450;		height [2] = 0.500;		height [3] = 0.0;
		height [0] = 0.400;		height [1] = 0.450;		height [2] = 0.600;		height [3] = 0.0;
		verticalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.height - 1.400) < EPS) {
		nVerEuroform = 3;
		//height [0] = 0.500;		height [1] = 0.400;		height [2] = 0.500;		height [3] = 0.0;
		height [0] = 0.500;		height [1] = 0.300;		height [2] = 0.600;		height [3] = 0.0;
		verticalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.height - 1.350) < EPS) {
		nVerEuroform = 3;
		height [0] = 0.600;		height [1] = 0.300;		height [2] = 0.450;		height [3] = 0.0;
		verticalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.height - 1.300) < EPS) {
		nVerEuroform = 3;
		//height [0] = 0.600;		height [1] = 0.200;		height [2] = 0.500;		height [3] = 0.0;
		height [0] = 0.500;		height [1] = 0.200;		height [2] = 0.600;		height [3] = 0.0;
		verticalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.height - 1.250) < EPS) {
		nVerEuroform = 3;
		//height [0] = 0.600;		height [1] = 0.200;		height [2] = 0.450;		height [3] = 0.0;
		height [0] = 0.200;		height [1] = 0.600;		height [2] = 0.450;		height [3] = 0.0;
		verticalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.height - 1.200) < EPS) {
		nVerEuroform = 2;
		height [0] = 0.600;		height [1] = 0.600;		height [2] = 0.0;		height [3] = 0.0;
		verticalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.height - 1.150) < EPS) {
		nVerEuroform = 3;
		//height [0] = 0.450;		height [1] = 0.300;		height [2] = 0.400;		height [3] = 0.0;
		height [0] = 0.400;		height [1] = 0.300;		height [2] = 0.450;		height [3] = 0.0;
		verticalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.height - 1.100) < EPS) {
		//nVerEuroform = 3;
		nVerEuroform = 2;
		//height [0] = 0.400;		height [1] = 0.300;		height [2] = 0.400;		height [3] = 0.0;
		height [0] = 0.500;		height [1] = 0.600;		height [2] = 0.0;		height [3] = 0.0;
		verticalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.height - 1.050) < EPS) {
		nVerEuroform = 3;
		height [0] = 0.450;		height [1] = 0.300;		height [2] = 0.300;		height [3] = 0.0;
		verticalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.height - 1.000) < EPS) {
		nVerEuroform = 2;
		//height [0] = 0.600;		height [1] = 0.400;		height [2] = 0.0;		height [3] = 0.0;
		height [0] = 0.400;		height [1] = 0.600;		height [2] = 0.0;		height [3] = 0.0;
		verticalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.height - 0.950) < EPS) {
		nVerEuroform = 2;
		//height [0] = 0.450;		height [1] = 0.500;		height [2] = 0.0;		height [3] = 0.0;
		height [0] = 0.500;		height [1] = 0.450;		height [2] = 0.0;		height [3] = 0.0;
		verticalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.height - 0.900) < EPS) {
		nVerEuroform = 2;
		height [0] = 0.600;		height [1] = 0.300;		height [2] = 0.0;		height [3] = 0.0;
		verticalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.height - 0.850) < EPS) {
		nVerEuroform = 2;
		height [0] = 0.400;		height [1] = 0.450;		height [2] = 0.0;		height [3] = 0.0;
		verticalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.height - 0.800) < EPS) {
		nVerEuroform = 2;
		height [0] = 0.400;		height [1] = 0.400;		height [2] = 0.0;		height [3] = 0.0;
		verticalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.250;
	} else if (abs (params.height - 0.750) < EPS) {
		nVerEuroform = 2;
		height [0] = 0.450;		height [1] = 0.300;		height [2] = 0.0;		height [3] = 0.0;
		verticalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.200;
	} else if (abs (params.height - 0.700) < EPS) {
		nVerEuroform = 2;
		height [0] = 0.400;		height [1] = 0.300;		height [2] = 0.0;		height [3] = 0.0;
		verticalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.250;
		verticalBarRightOffset = 0.150;
	} else if (abs (params.height - 0.650) < EPS) {
		nVerEuroform = 2;
		//height [0] = 0.450;		height [1] = 0.200;		height [2] = 0.0;		height [3] = 0.0;
		height [0] = 0.200;		height [1] = 0.450;		height [2] = 0.0;		height [3] = 0.0;
		verticalGap = 0.025;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.200;
		verticalBarRightOffset = 0.150;
	} else if (abs (params.height - 0.600) < EPS) {
		nVerEuroform = 1;
		height [0] = 0.600;		height [1] = 0.0;		height [2] = 0.0;		height [3] = 0.0;
		verticalGap = 0.050;
		nVerticalBar = 2;
		verticalBarLeftOffset = 0.150;
		verticalBarRightOffset = 0.150;
	} else if (abs (params.height - 0.500) < EPS) {
		nVerEuroform = 1;
		height [0] = 0.500;		height [1] = 0.0;		height [2] = 0.0;		height [3] = 0.0;
		verticalGap = 0.050;
		nVerticalBar = 1;
		verticalBarLeftOffset = 0.200;
		verticalBarRightOffset = 0.200;
	} else if (abs (params.height - 0.450) < EPS) {
		nVerEuroform = 1;
		height [0] = 0.450;		height [1] = 0.0;		height [2] = 0.0;		height [3] = 0.0;
		verticalGap = 0.025;
		nVerticalBar = 1;
		verticalBarLeftOffset = 0.200;
		verticalBarRightOffset = 0.200;
	} else if (abs (params.height - 0.400) < EPS) {
		nVerEuroform = 1;
		height [0] = 0.400;		height [1] = 0.0;		height [2] = 0.0;		height [3] = 0.0;
		verticalGap = 0.050;
		nVerticalBar = 1;
		verticalBarLeftOffset = 0.150;
		verticalBarRightOffset = 0.150;
	} else {
		nVerEuroform = 0;
		height [0] = 0.0;		height [1] = 0.0;		height [2] = 0.0;		height [3] = 0.0;
		nVerticalBar = 0;
		verticalBarLeftOffset = 0.0;
		verticalBarRightOffset = 0.0;
	}

	if (abs (params.width - 6.000) < EPS) {
		nHorEuroform = 5;
		width [0] = 1.200;
		width [1] = 1.200;
		width [2] = 1.200;
		width [3] = 1.200;
		width [4] = 1.200;
	} else if (abs (params.width - 5.700) < EPS) {
		nHorEuroform = 5;
		width [0] = 1.200;
		width [1] = 1.200;
		width [2] = 1.200;
		width [3] = 1.200;
		width [4] = 0.900;
	} else if (abs (params.width - 5.400) < EPS) {
		nHorEuroform = 5;
		width [0] = 1.200;
		width [1] = 1.200;
		width [2] = 1.200;
		width [3] = 0.900;
		width [4] = 0.900;
	} else if (abs (params.width - 5.100) < EPS) {
		nHorEuroform = 5;
		width [0] = 1.200;
		width [1] = 1.200;
		width [2] = 1.200;
		width [3] = 0.900;
		width [4] = 0.600;
	} else if (abs (params.width - 4.800) < EPS) {
		nHorEuroform = 4;
		width [0] = 1.200;
		width [1] = 1.200;
		width [2] = 1.200;
		width [3] = 1.200;
		width [4] = 0.0;
	} else if (abs (params.width - 4.500) < EPS) {
		nHorEuroform = 4;
		width [0] = 1.200;
		width [1] = 1.200;
		width [2] = 1.200;
		width [3] = 0.900;
		width [4] = 0.0;
	} else if (abs (params.width - 4.200) < EPS) {
		nHorEuroform = 4;
		width [0] = 1.200;
		width [1] = 1.200;
		width [2] = 0.900;
		width [3] = 0.900;
		width [4] = 0.0;
	} else if (abs (params.width - 3.900) < EPS) {
		nHorEuroform = 4;
		width [0] = 1.200;
		width [1] = 1.200;
		width [2] = 0.900;
		width [3] = 0.600;
		width [4] = 0.0;
	} else if (abs (params.width - 3.600) < EPS) {
		nHorEuroform = 3;
		width [0] = 1.200;
		width [1] = 1.200;
		width [2] = 1.200;
		width [3] = 0.0;
		width [4] = 0.0;
	} else if (abs (params.width - 3.300) < EPS) {
		nHorEuroform = 3;
		width [0] = 1.200;
		width [1] = 1.200;
		width [2] = 0.900;
		width [3] = 0.0;
		width [4] = 0.0;
	} else if (abs (params.width - 3.000) < EPS) {
		nHorEuroform = 3;
		width [0] = 1.200;
		width [1] = 1.200;
		width [2] = 0.600;
		width [3] = 0.0;
		width [4] = 0.0;
	} else if (abs (params.width - 2.700) < EPS) {
		nHorEuroform = 3;
		width [0] = 1.200;
		width [1] = 0.900;
		width [2] = 0.600;
		width [3] = 0.0;
		width [4] = 0.0;
	} else if (abs (params.width - 2.400) < EPS) {
		nHorEuroform = 2;
		width [0] = 1.200;
		width [1] = 1.200;
		width [2] = 0.0;
		width [3] = 0.0;
		width [4] = 0.0;
	} else if (abs (params.width - 2.100) < EPS) {
		nHorEuroform = 2;
		width [0] = 1.200;
		width [1] = 0.900;
		width [2] = 0.0;
		width [3] = 0.0;
		width [4] = 0.0;
	} else if (abs (params.width - 1.800) < EPS) {
		nHorEuroform = 2;
		width [0] = 0.900;
		width [1] = 0.900;
		width [2] = 0.0;
		width [3] = 0.0;
		width [4] = 0.0;
	} else if (abs (params.width - 1.500) < EPS) {
		nHorEuroform = 2;
		width [0] = 0.900;
		width [1] = 0.600;
		width [2] = 0.0;
		width [3] = 0.0;
		width [4] = 0.0;
	} else {
		nHorEuroform = 0;
		width [0] = 0.0;
		width [1] = 0.0;
		width [2] = 0.0;
		width [3] = 0.0;
		width [4] = 0.0;
	}

	// 너비나 높이가 0이면 아무것도 배치하지 않음
	if ((nHorEuroform == 0) || (nVerEuroform == 0))
		return	NoError;

	//////////////////////////////////////////////////////////////// 현재면
	// 유로폼 설치
	euroform.leftBottomX = params.leftBottomX;
	euroform.leftBottomY = params.leftBottomY;
	euroform.leftBottomZ = params.leftBottomZ;
	euroform.ang = params.ang;
	euroform.eu_stan_onoff = true;
	euroform.u_ins_wall = false;
	euroform.ang_x = DegreeToRad (90.0);

	for (xx = 0 ; xx < nHorEuroform ; ++xx) {
		height_t = 0.0;
		for (yy = nVerEuroform-1 ; yy >= 0 ; --yy) {
			euroform.eu_wid = euroform.width	= height [yy];
			euroform.eu_hei = euroform.height	= width [xx];
			height_t += height [yy];
			elemList.Push (placeEuroform (euroform));
			moveIn3D ('z', euroform.ang, height [yy], &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
		}
		moveIn3D ('x', euroform.ang, width [xx], &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
		moveIn3D ('z', euroform.ang, -height_t, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
	}

	// 비계 파이프 (수직) 배치
	sqrPipe.leftBottomX = params.leftBottomX;
	sqrPipe.leftBottomY = params.leftBottomY;
	sqrPipe.leftBottomZ = params.leftBottomZ;
	sqrPipe.ang = params.ang;
	sqrPipe.length = params.height - (verticalGap * 2);
	sqrPipe.pipeAng = DegreeToRad (90.0);

	moveIn3D ('z', sqrPipe.ang, verticalGap, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
	moveIn3D ('y', sqrPipe.ang, -(0.0635 + 0.025), &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
	moveIn3D ('x', sqrPipe.ang, 0.150 + 0.031, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);

	cylinder.angleFromPlane = DegreeToRad (0.0);
	cylinder.length = 0.050;
	cylinder.radius = 0.013/2;
	cylinder.ang = params.ang;
	cylinder.leftBottomX = sqrPipe.leftBottomX;
	cylinder.leftBottomY = sqrPipe.leftBottomY;
	cylinder.leftBottomZ = sqrPipe.leftBottomZ;

	for (xx = 0 ; xx <= nHorEuroform ; ++xx) {
		if (xx == 0) {
			// 1행
			tempGuid = placeSqrPipe (sqrPipe);
			elemList.Push (tempGuid);
			cylinder.leftBottomX = sqrPipe.leftBottomX;
			cylinder.leftBottomY = sqrPipe.leftBottomY;
			cylinder.leftBottomZ = sqrPipe.leftBottomZ;
			moveIn3D ('x', cylinder.ang, -0.025, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			for (yy = 0 ; yy < 6 ; ++yy) {
				moveIn3D ('z', cylinder.ang, 0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				elemList.Push (placeHole (tempGuid, cylinder));
			}
			moveIn3D ('z', cylinder.ang, -0.300 + params.height - (verticalGap * 2), &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			for (yy = 0 ; yy < 6 ; ++yy) {
				moveIn3D ('z', cylinder.ang, -0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				elemList.Push (placeHole (tempGuid, cylinder));
			}
			moveIn3D ('x', sqrPipe.ang, -0.031 - 0.150 + width [xx], &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
		} else if (xx == nHorEuroform) {
			// 마지막 행
			moveIn3D ('x', sqrPipe.ang, -0.150 + 0.031, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
			tempGuid = placeSqrPipe (sqrPipe);
			elemList.Push (tempGuid);
			cylinder.leftBottomX = sqrPipe.leftBottomX;
			cylinder.leftBottomY = sqrPipe.leftBottomY;
			cylinder.leftBottomZ = sqrPipe.leftBottomZ;
			moveIn3D ('x', cylinder.ang, -0.025, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			for (yy = 0 ; yy < 6 ; ++yy) {
				moveIn3D ('z', cylinder.ang, 0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				elemList.Push (placeHole (tempGuid, cylinder));
			}
			moveIn3D ('z', cylinder.ang, -0.300 + params.height - (verticalGap * 2), &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			for (yy = 0 ; yy < 6 ; ++yy) {
				moveIn3D ('z', cylinder.ang, -0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				elemList.Push (placeHole (tempGuid, cylinder));
			}
		} else {
			// 나머지 행
			tempGuid = placeSqrPipe (sqrPipe);
			elemList.Push (tempGuid);
			cylinder.leftBottomX = sqrPipe.leftBottomX;
			cylinder.leftBottomY = sqrPipe.leftBottomY;
			cylinder.leftBottomZ = sqrPipe.leftBottomZ;
			moveIn3D ('x', cylinder.ang, -0.025, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			for (yy = 0 ; yy < 6 ; ++yy) {
				moveIn3D ('z', cylinder.ang, 0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				elemList.Push (placeHole (tempGuid, cylinder));
			}
			moveIn3D ('z', cylinder.ang, -0.300 + params.height - (verticalGap * 2), &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			for (yy = 0 ; yy < 6 ; ++yy) {
				moveIn3D ('z', cylinder.ang, -0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				elemList.Push (placeHole (tempGuid, cylinder));
			}
			moveIn3D ('x', sqrPipe.ang, width [xx], &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
		}
	}

	// 비계 파이프 (수평) 배치
	sqrPipe.leftBottomX = params.leftBottomX;
	sqrPipe.leftBottomY = params.leftBottomY;
	sqrPipe.leftBottomZ = params.leftBottomZ;
	sqrPipe.ang = params.ang;
	sqrPipe.length = params.width - 0.100;
	sqrPipe.pipeAng = DegreeToRad (0);

	moveIn3D ('z', sqrPipe.ang, verticalGap + verticalBarLeftOffset, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
	moveIn3D ('y', sqrPipe.ang, -(0.0635 + 0.075), &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
	moveIn3D ('x', sqrPipe.ang, 0.050, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);

	// 1행
	cylinder.angleFromPlane = DegreeToRad (90.0);
	cylinder.length = 0.050;
	cylinder.radius = 0.013/2;
	cylinder.ang = params.ang;
	cylinder.leftBottomX = sqrPipe.leftBottomX;
	cylinder.leftBottomY = sqrPipe.leftBottomY;
	cylinder.leftBottomZ = sqrPipe.leftBottomZ;

	tempGuid = placeSqrPipe (sqrPipe);
	elemList.Push (tempGuid);
	moveIn3D ('z', cylinder.ang, -0.025, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
	for (xx = 0 ; xx < 6 ; ++xx) {
		moveIn3D ('x', cylinder.ang, 0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
		elemList.Push (placeHole (tempGuid, cylinder));
	}
	moveIn3D ('x', cylinder.ang, -0.300 + params.width - 0.100, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
	for (xx = 0 ; xx < 6 ; ++xx) {
		moveIn3D ('x', cylinder.ang, -0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
		elemList.Push (placeHole (tempGuid, cylinder));
	}
	moveIn3D ('z', sqrPipe.ang, -(verticalGap + verticalBarLeftOffset) + params.height - (verticalGap + verticalBarRightOffset), &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);

	if (nVerticalBar > 1) {
		// 2열
		tempGuid = placeSqrPipe (sqrPipe);
		elemList.Push (tempGuid);
		cylinder.leftBottomX = sqrPipe.leftBottomX;
		cylinder.leftBottomY = sqrPipe.leftBottomY;
		cylinder.leftBottomZ = sqrPipe.leftBottomZ;
		moveIn3D ('z', cylinder.ang, -0.025, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
		for (xx = 0 ; xx < 6 ; ++xx) {
			moveIn3D ('x', cylinder.ang, 0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			elemList.Push (placeHole (tempGuid, cylinder));
		}
		moveIn3D ('x', cylinder.ang, -0.300 + params.width - 0.100, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
		for (xx = 0 ; xx < 6 ; ++xx) {
			moveIn3D ('x', cylinder.ang, -0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			elemList.Push (placeHole (tempGuid, cylinder));
		}
	}

	// 유로폼 후크 배치 (수평 - 최하단, 최상단)
	hook.leftBottomX = params.leftBottomX;
	hook.leftBottomY = params.leftBottomY;
	hook.leftBottomZ = params.leftBottomZ;
	hook.ang = params.ang;
	hook.iHookType = 2;
	hook.iHookShape = 2;
	hook.angX = DegreeToRad (0.0);
	hook.angY = DegreeToRad (0.0);

	moveIn3D ('y', hook.ang, -0.0885, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);

	if (nVerEuroform >= 2) {
		moveIn3D ('z', hook.ang, params.height - height [0], &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);
		moveIn3D ('x', hook.ang, 0.030 + 0.150, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);
		// 1행
		height_t = 0.0;
		for (xx = 1 ; xx < nVerEuroform ; ++xx) {
			height_t += height [xx];
			elemList.Push (placeEuroformHook (hook));
			moveIn3D ('z', hook.ang, -height [xx], &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);
		}
		moveIn3D ('z', hook.ang, height_t, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);
		moveIn3D ('x', hook.ang, -0.150 + params.width - 0.150, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);

		// 마지막 행
		for (xx = 1 ; xx < nVerEuroform ; ++xx) {
			height_t += height [xx];
			elemList.Push (placeEuroformHook (hook));
			moveIn3D ('z', hook.ang, -height [xx], &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);
		}
	}

	// 각파이프행거 배치 (수평 - 최하단, 최상단을 제외한 나머지)
	hanger.leftBottomX = params.leftBottomX;
	hanger.leftBottomY = params.leftBottomY;
	hanger.leftBottomZ = params.leftBottomZ;
	hanger.ang = params.ang;
	hanger.angX = DegreeToRad (270.0);
	hanger.angY = DegreeToRad (270.0);

	moveIn3D ('y', hanger.ang, -0.0635, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);

	// 2 ~ [n-1]행
	if (nVerEuroform >= 2) {
		moveIn3D ('z', hanger.ang, params.height - 0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
		moveIn3D ('x', hanger.ang, width [0], &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
		for (xx = 1 ; xx < nHorEuroform ; ++xx) {
			height_t = 0.0;
			for (yy = 0 ; yy < nVerEuroform ; ++yy) {
				// 1열
				if (yy == 0) {
					elemList.Push (placeRectpipeHanger (hanger));
					moveIn3D ('z', hanger.ang, -(height [0] - 0.150), &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
					height_t += height [0] - 0.150;
				// 마지막 열
				} else if (yy == nVerEuroform - 1) {
					height_t += height [nVerEuroform-1] - 0.150;
					moveIn3D ('z', hanger.ang, -(height [nVerEuroform-1] - 0.150), &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
					elemList.Push (placeRectpipeHanger (hanger));
				// 나머지 열
				} else {
					height_t += height [yy];
					if (abs (height [yy] - 0.600) < EPS) {
						moveIn3D ('z', hanger.ang, -0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						elemList.Push (placeRectpipeHanger (hanger));
						moveIn3D ('z', hanger.ang, -0.300, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						elemList.Push (placeRectpipeHanger (hanger));
						moveIn3D ('z', hanger.ang, -0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
					} else if (abs (height [yy] - 0.500) < EPS) {
						moveIn3D ('z', hanger.ang, -0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						elemList.Push (placeRectpipeHanger (hanger));
						moveIn3D ('z', hanger.ang, -0.200, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						elemList.Push (placeRectpipeHanger (hanger));
						moveIn3D ('z', hanger.ang, -0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
					} else if (abs (height [yy] - 0.450) < EPS) {
						moveIn3D ('z', hanger.ang, -0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						elemList.Push (placeRectpipeHanger (hanger));
						moveIn3D ('z', hanger.ang, -0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						elemList.Push (placeRectpipeHanger (hanger));
						moveIn3D ('z', hanger.ang, -0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
					} else if (abs (height [yy] - 0.400) < EPS) {
						moveIn3D ('z', hanger.ang, -0.100, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						elemList.Push (placeRectpipeHanger (hanger));
						moveIn3D ('z', hanger.ang, -0.200, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						elemList.Push (placeRectpipeHanger (hanger));
						moveIn3D ('z', hanger.ang, -0.100, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
					} else if (abs (height [yy] - 0.300) < EPS) {
						moveIn3D ('z', hanger.ang, -0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						elemList.Push (placeRectpipeHanger (hanger));
						moveIn3D ('z', hanger.ang, -0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
					} else if (abs (height [yy] - 0.200) < EPS) {
						moveIn3D ('z', hanger.ang, -0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						elemList.Push (placeRectpipeHanger (hanger));
						moveIn3D ('z', hanger.ang, -0.050, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
					}
				}
			}
			moveIn3D ('z', hanger.ang, height_t, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
			moveIn3D ('x', hanger.ang, width [xx], &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
		}
	}

	// 헤드 피스
	headpiece.leftBottomX = params.leftBottomX;
	headpiece.leftBottomY = params.leftBottomY;
	headpiece.leftBottomZ = params.leftBottomZ;
	headpiece.ang = params.ang;

	moveIn3D ('z', headpiece.ang, params.height - (verticalGap + verticalBarRightOffset - 0.0475) - 0.0975, &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
	moveIn3D ('y', headpiece.ang, -0.2685, &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
	moveIn3D ('x', headpiece.ang, 0.291, &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);

	// 처음 행
	elemList.Push (placeJointHeadpeace_hor (headpiece));
	moveIn3D ('z', headpiece.ang, (verticalGap + verticalBarRightOffset - 0.0475) - params.height + (verticalGap + verticalBarLeftOffset + 0.0475), &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
	if (nVerticalBar > 1)
		elemList.Push (placeJointHeadpeace_hor (headpiece));
	moveIn3D ('z', headpiece.ang, -(verticalGap + verticalBarRightOffset - 0.0475) + params.height - (verticalGap + verticalBarLeftOffset + 0.0475), &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
	//if (cell.verLen > 4.000) {
	//	elev_headpiece = 4.000 * 0.80;
	//} else {
	//	elev_headpiece = cell.verLen * 0.80;
	//}
	elev_headpiece = 1.900;
	moveIn3D ('x', headpiece.ang, elev_headpiece, &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
	// 마지막 행
		elemList.Push (placeJointHeadpeace_hor (headpiece));
	moveIn3D ('z', headpiece.ang, (verticalGap + verticalBarRightOffset - 0.0475) - params.height + (verticalGap + verticalBarLeftOffset + 0.0475), &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
	if (nVerticalBar > 1)
		elemList.Push (placeJointHeadpeace_hor (headpiece));

	// 결합철물
	fittings.leftBottomX = params.leftBottomX;
	fittings.leftBottomY = params.leftBottomY;
	fittings.leftBottomZ = params.leftBottomZ;
	fittings.ang = params.ang;
	fittings.angX = DegreeToRad (180.0);
	fittings.angY = DegreeToRad (90.0);
	
	moveIn3D ('z', fittings.ang, params.height - (verticalGap + verticalBarRightOffset + 0.081), &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
	moveIn3D ('y', fittings.ang, -0.1155, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
	moveIn3D ('x', fittings.ang, 0.130, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);

	// 처음 열
	for (xx = 0 ; xx <= nHorEuroform ; ++xx) {
		// 1행
		if (xx == 0) {
			elemList.Push (placeFittings (fittings));
			moveIn3D ('x', fittings.ang, width [xx] - 0.180, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
		// 마지막 행
		} else if (xx == nHorEuroform) {
			moveIn3D ('x', fittings.ang, -0.120, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
			elemList.Push (placeFittings (fittings));
		// 나머지 행
		} else {
			elemList.Push (placeFittings (fittings));
			moveIn3D ('x', fittings.ang, width [xx], &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
		}
	}
	moveIn3D ('z', fittings.ang, (verticalGap + verticalBarRightOffset + 0.081) - params.height + (verticalGap + verticalBarLeftOffset + 0.081) - 0.162, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
	moveIn3D ('x', fittings.ang, 0.300 - params.width, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);

	if (nVerticalBar > 1) {
		// 마지막 열
		for (xx = 0 ; xx <= nHorEuroform ; ++xx) {
			// 1행
			if (xx == 0) {
				elemList.Push (placeFittings (fittings));
				moveIn3D ('x', fittings.ang, width [xx] - 0.180, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
			// 마지막 행
			} else if (xx == nHorEuroform) {
				moveIn3D ('x', fittings.ang, -0.120, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
				elemList.Push (placeFittings (fittings));
			// 나머지 행
			} else {
				elemList.Push (placeFittings (fittings));
				moveIn3D ('x', fittings.ang, width [xx], &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
			}
		}
	}

	// 그룹화하기
	if (!elemList.IsEmpty ()) {
		GSSize nElems = elemList.GetSize ();
		API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
		if (elemHead != NULL) {
			for (GSIndex i = 0; i < nElems; i++)
				(*elemHead)[i].guid = elemList[i];

			ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

			BMKillHandle ((GSHandle *) &elemHead);
		}
	}
	elemList.Clear (false);

	////////////////////////////////////////////////////////////////// 반대면

	if (bDoubleSide) {
		moveIn3D ('x', params.ang, params.width, &params.leftBottomX, &params.leftBottomY, &params.leftBottomY);
		moveIn3D ('y', params.ang, wallThk, &params.leftBottomX, &params.leftBottomY, &params.leftBottomY);
		params.ang += DegreeToRad (180.0);

		// 유로폼 설치
		euroform.leftBottomX = params.leftBottomX;
		euroform.leftBottomY = params.leftBottomY;
		euroform.leftBottomZ = params.leftBottomZ;
		euroform.ang = params.ang;
		euroform.u_ins_wall = false;

		for (xx = nHorEuroform-1 ; xx >= 0  ; --xx) {
			height_t = 0.0;
			for (yy = nVerEuroform-1 ; yy >= 0 ; --yy) {
				euroform.eu_wid = euroform.width	= height [yy];
				euroform.eu_hei = euroform.height	= width [xx];
				height_t += height [yy];
				elemList.Push (placeEuroform (euroform));
				moveIn3D ('z', euroform.ang, height [yy], &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
			}
			moveIn3D ('x', euroform.ang, width [xx], &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
			moveIn3D ('z', euroform.ang, -height_t, &euroform.leftBottomX, &euroform.leftBottomY, &euroform.leftBottomZ);
		}

		// 비계 파이프 (수직) 배치
		sqrPipe.leftBottomX = params.leftBottomX;
		sqrPipe.leftBottomY = params.leftBottomY;
		sqrPipe.leftBottomZ = params.leftBottomZ;
		sqrPipe.ang = params.ang;
		sqrPipe.length = params.height - (verticalGap * 2);
		sqrPipe.pipeAng = DegreeToRad (90.0);

		moveIn3D ('z', sqrPipe.ang, verticalGap, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
		moveIn3D ('y', sqrPipe.ang, -(0.0635 + 0.025), &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
		moveIn3D ('x', sqrPipe.ang, params.width - (0.150 + 0.031), &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);

		cylinder.angleFromPlane = DegreeToRad (0.0);
		cylinder.length = 0.050;
		cylinder.radius = 0.013/2;
		cylinder.ang = params.ang;
		cylinder.leftBottomX = sqrPipe.leftBottomX;
		cylinder.leftBottomY = sqrPipe.leftBottomY;
		cylinder.leftBottomZ = sqrPipe.leftBottomZ;

		for (xx = 0 ; xx <= nHorEuroform ; ++xx) {
			if (xx == 0) {
				// 1행
				tempGuid = placeSqrPipe (sqrPipe);
				elemList.Push (tempGuid);
				cylinder.leftBottomX = sqrPipe.leftBottomX;
				cylinder.leftBottomY = sqrPipe.leftBottomY;
				cylinder.leftBottomZ = sqrPipe.leftBottomZ;
				moveIn3D ('x', cylinder.ang, -0.025, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				for (yy = 0 ; yy < 6 ; ++yy) {
					moveIn3D ('z', cylinder.ang, 0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
					elemList.Push (placeHole (tempGuid, cylinder));
				}
				moveIn3D ('z', cylinder.ang, -0.300 + params.height - (verticalGap * 2), &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				for (yy = 0 ; yy < 6 ; ++yy) {
					moveIn3D ('z', cylinder.ang, -0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
					elemList.Push (placeHole (tempGuid, cylinder));
				}
				moveIn3D ('x', sqrPipe.ang, (0.150 + 0.031) - width [xx], &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
			} else if (xx == nHorEuroform) {
				// 마지막 행
				moveIn3D ('x', sqrPipe.ang, 0.150 - 0.031, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
				tempGuid = placeSqrPipe (sqrPipe);
				elemList.Push (tempGuid);
				cylinder.leftBottomX = sqrPipe.leftBottomX;
				cylinder.leftBottomY = sqrPipe.leftBottomY;
				cylinder.leftBottomZ = sqrPipe.leftBottomZ;
				moveIn3D ('x', cylinder.ang, -0.025, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				for (yy = 0 ; yy < 6 ; ++yy) {
					moveIn3D ('z', cylinder.ang, 0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
					elemList.Push (placeHole (tempGuid, cylinder));
				}
				moveIn3D ('z', cylinder.ang, -0.300 + params.height - (verticalGap * 2), &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				for (yy = 0 ; yy < 6 ; ++yy) {
					moveIn3D ('z', cylinder.ang, -0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
					elemList.Push (placeHole (tempGuid, cylinder));
				}
			} else {
				// 나머지 행
				tempGuid = placeSqrPipe (sqrPipe);
				elemList.Push (tempGuid);
				cylinder.leftBottomX = sqrPipe.leftBottomX;
				cylinder.leftBottomY = sqrPipe.leftBottomY;
				cylinder.leftBottomZ = sqrPipe.leftBottomZ;
				moveIn3D ('x', cylinder.ang, -0.025, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				for (yy = 0 ; yy < 6 ; ++yy) {
					moveIn3D ('z', cylinder.ang, 0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
					elemList.Push (placeHole (tempGuid, cylinder));
				}
				moveIn3D ('z', cylinder.ang, -0.300 + params.height - (verticalGap * 2), &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				for (yy = 0 ; yy < 6 ; ++yy) {
					moveIn3D ('z', cylinder.ang, -0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
					elemList.Push (placeHole (tempGuid, cylinder));
				}
				moveIn3D ('x', sqrPipe.ang, -width [xx], &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
			}
		}

		// 비계 파이프 (수평) 배치
		sqrPipe.leftBottomX = params.leftBottomX;
		sqrPipe.leftBottomY = params.leftBottomY;
		sqrPipe.leftBottomZ = params.leftBottomZ;
		sqrPipe.ang = params.ang;
		sqrPipe.length = params.width - 0.100;
		sqrPipe.pipeAng = DegreeToRad (0);

		moveIn3D ('z', sqrPipe.ang, verticalGap + verticalBarLeftOffset, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
		moveIn3D ('y', sqrPipe.ang, -(0.0635 + 0.075), &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);
		moveIn3D ('x', sqrPipe.ang, 0.050, &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);

		// 1행
		cylinder.angleFromPlane = DegreeToRad (90.0);
		cylinder.length = 0.050;
		cylinder.radius = 0.013/2;
		cylinder.ang = params.ang;
		cylinder.leftBottomX = sqrPipe.leftBottomX;
		cylinder.leftBottomY = sqrPipe.leftBottomY;
		cylinder.leftBottomZ = sqrPipe.leftBottomZ;

		tempGuid = placeSqrPipe (sqrPipe);
		elemList.Push (tempGuid);
		moveIn3D ('z', cylinder.ang, -0.025, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
		for (xx = 0 ; xx < 6 ; ++xx) {
			moveIn3D ('x', cylinder.ang, 0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			elemList.Push (placeHole (tempGuid, cylinder));
		}
		moveIn3D ('x', cylinder.ang, -0.300 + params.width - 0.100, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
		for (xx = 0 ; xx < 6 ; ++xx) {
			moveIn3D ('x', cylinder.ang, -0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			elemList.Push (placeHole (tempGuid, cylinder));
		}
		moveIn3D ('z', sqrPipe.ang, -(verticalGap + verticalBarLeftOffset) + params.height - (verticalGap + verticalBarRightOffset), &sqrPipe.leftBottomX, &sqrPipe.leftBottomY, &sqrPipe.leftBottomZ);

		if (nVerticalBar > 1) {
			// 2열
			tempGuid = placeSqrPipe (sqrPipe);
			elemList.Push (tempGuid);
			cylinder.leftBottomX = sqrPipe.leftBottomX;
			cylinder.leftBottomY = sqrPipe.leftBottomY;
			cylinder.leftBottomZ = sqrPipe.leftBottomZ;
			moveIn3D ('z', cylinder.ang, -0.025, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			for (xx = 0 ; xx < 6 ; ++xx) {
				moveIn3D ('x', cylinder.ang, 0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				elemList.Push (placeHole (tempGuid, cylinder));
			}
			moveIn3D ('x', cylinder.ang, -0.300 + params.width - 0.100, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
			for (xx = 0 ; xx < 6 ; ++xx) {
				moveIn3D ('x', cylinder.ang, -0.050, &cylinder.leftBottomX, &cylinder.leftBottomY, &cylinder.leftBottomZ);
				elemList.Push (placeHole (tempGuid, cylinder));
			}
		}

		// 유로폼 후크 배치 (수평 - 최하단, 최상단)
		hook.leftBottomX = params.leftBottomX;
		hook.leftBottomY = params.leftBottomY;
		hook.leftBottomZ = params.leftBottomZ;
		hook.ang = params.ang;
		hook.iHookType = 2;
		hook.iHookShape = 2;
		hook.angX = DegreeToRad (0.0);
		hook.angY = DegreeToRad (180.0);

		moveIn3D ('y', hook.ang, -0.0885, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);

		if (nVerEuroform >= 2) {
			moveIn3D ('z', hook.ang, params.height - height [0], &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);
			moveIn3D ('x', hook.ang, 0.030 + 0.150 - 0.061, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);
			// 1행
			height_t = 0.0;
			for (xx = 1 ; xx < nVerEuroform ; ++xx) {
				height_t += height [xx];
				elemList.Push (placeEuroformHook (hook));
				moveIn3D ('z', hook.ang, -height [xx], &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);
			}
			moveIn3D ('z', hook.ang, height_t, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);
			moveIn3D ('x', hook.ang, -0.150 + params.width - 0.150, &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);

			// 마지막 행
			for (xx = 1 ; xx < nVerEuroform ; ++xx) {
				height_t += height [xx];
				elemList.Push (placeEuroformHook (hook));
				moveIn3D ('z', hook.ang, -height [xx], &hook.leftBottomX, &hook.leftBottomY, &hook.leftBottomZ);
			}
		}

		// 각파이프행거 배치 (수평 - 최하단, 최상단을 제외한 나머지)
		hanger.leftBottomX = params.leftBottomX;
		hanger.leftBottomY = params.leftBottomY;
		hanger.leftBottomZ = params.leftBottomZ;
		hanger.ang = params.ang;
		hanger.angX = DegreeToRad (90.0);
		hanger.angY = DegreeToRad (270.0);

		moveIn3D ('y', hanger.ang, -0.0635, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);

		// 2 ~ [n-1]행
		if (nVerEuroform >= 2) {
			moveIn3D ('z', hanger.ang, params.height - 0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
			moveIn3D ('x', hanger.ang, params.width - width [0], &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
			for (xx = 1 ; xx < nHorEuroform ; ++xx) {
				height_t = 0.0;
				for (yy = 0 ; yy < nVerEuroform ; ++yy) {
					// 1열
					if (yy == 0) {
						elemList.Push (placeRectpipeHanger (hanger));
						moveIn3D ('z', hanger.ang, -(height [0] - 0.150), &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						height_t += height [0] - 0.150;
					// 마지막 열
					} else if (yy == nVerEuroform - 1) {
						height_t += height [nVerEuroform-1] - 0.150;
						moveIn3D ('z', hanger.ang, -(height [nVerEuroform-1] - 0.150), &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						elemList.Push (placeRectpipeHanger (hanger));
					// 나머지 열
					} else {
						height_t += height [yy];
						if (abs (height [yy] - 0.600) < EPS) {
							moveIn3D ('z', hanger.ang, -0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
							elemList.Push (placeRectpipeHanger (hanger));
							moveIn3D ('z', hanger.ang, -0.300, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
							elemList.Push (placeRectpipeHanger (hanger));
							moveIn3D ('z', hanger.ang, -0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						} else if (abs (height [yy] - 0.500) < EPS) {
							moveIn3D ('z', hanger.ang, -0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
							elemList.Push (placeRectpipeHanger (hanger));
							moveIn3D ('z', hanger.ang, -0.200, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
							elemList.Push (placeRectpipeHanger (hanger));
							moveIn3D ('z', hanger.ang, -0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						} else if (abs (height [yy] - 0.450) < EPS) {
							moveIn3D ('z', hanger.ang, -0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
							elemList.Push (placeRectpipeHanger (hanger));
							moveIn3D ('z', hanger.ang, -0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
							elemList.Push (placeRectpipeHanger (hanger));
							moveIn3D ('z', hanger.ang, -0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						} else if (abs (height [yy] - 0.400) < EPS) {
							moveIn3D ('z', hanger.ang, -0.100, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
							elemList.Push (placeRectpipeHanger (hanger));
							moveIn3D ('z', hanger.ang, -0.200, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
							elemList.Push (placeRectpipeHanger (hanger));
							moveIn3D ('z', hanger.ang, -0.100, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						} else if (abs (height [yy] - 0.300) < EPS) {
							moveIn3D ('z', hanger.ang, -0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
							elemList.Push (placeRectpipeHanger (hanger));
							moveIn3D ('z', hanger.ang, -0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						} else if (abs (height [yy] - 0.200) < EPS) {
							moveIn3D ('z', hanger.ang, -0.150, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
							elemList.Push (placeRectpipeHanger (hanger));
							moveIn3D ('z', hanger.ang, -0.050, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
						}
					}
				}
				moveIn3D ('z', hanger.ang, height_t, &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
				moveIn3D ('x', hanger.ang, -width [xx], &hanger.leftBottomX, &hanger.leftBottomY, &hanger.leftBottomZ);
			}
		}

		// 헤드 피스
		headpiece.leftBottomX = params.leftBottomX;
		headpiece.leftBottomY = params.leftBottomY;
		headpiece.leftBottomZ = params.leftBottomZ;
		headpiece.ang = params.ang;

		moveIn3D ('z', headpiece.ang, params.height - (verticalGap + verticalBarRightOffset - 0.0475) - 0.0975, &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
		moveIn3D ('y', headpiece.ang, -0.2685, &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
		moveIn3D ('x', headpiece.ang, params.width - 0.291 - 0.095, &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);

		// 처음 행
		elemList.Push (placeJointHeadpeace_hor (headpiece));
		moveIn3D ('z', headpiece.ang, (verticalGap + verticalBarRightOffset - 0.0475) - params.height + (verticalGap + verticalBarLeftOffset + 0.0475), &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
		if (nVerticalBar > 1)
			elemList.Push (placeJointHeadpeace_hor (headpiece));
		moveIn3D ('z', headpiece.ang, -(verticalGap + verticalBarRightOffset - 0.0475) + params.height - (verticalGap + verticalBarLeftOffset + 0.0475), &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
		//if (cell.verLen > 4.000) {
		//	elev_headpiece = 4.000 * 0.80;
		//} else {
		//	elev_headpiece = cell.verLen * 0.80;
		//}
		elev_headpiece = 1.900;
		moveIn3D ('x', headpiece.ang, -elev_headpiece, &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
		// 마지막 행
			elemList.Push (placeJointHeadpeace_hor (headpiece));
		moveIn3D ('z', headpiece.ang, (verticalGap + verticalBarRightOffset - 0.0475) - params.height + (verticalGap + verticalBarLeftOffset + 0.0475), &headpiece.leftBottomX, &headpiece.leftBottomY, &headpiece.leftBottomZ);
		if (nVerticalBar > 1)
			elemList.Push (placeJointHeadpeace_hor (headpiece));

		// 결합철물
		fittings.leftBottomX = params.leftBottomX;
		fittings.leftBottomY = params.leftBottomY;
		fittings.leftBottomZ = params.leftBottomZ;
		fittings.ang = params.ang;
		fittings.angX = DegreeToRad (180.0);
		fittings.angY = DegreeToRad (90.0);
	
		moveIn3D ('z', fittings.ang, params.height - (verticalGap + verticalBarRightOffset + 0.081), &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
		moveIn3D ('y', fittings.ang, -0.1155, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
		moveIn3D ('x', fittings.ang, params.width - 0.130 - 0.100, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);

		// 처음 열
		for (xx = 0 ; xx <= nHorEuroform ; ++xx) {
			// 1행
			if (xx == 0) {
				elemList.Push (placeFittings (fittings));
				moveIn3D ('x', fittings.ang, -(width [xx] - 0.180), &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
			// 마지막 행
			} else if (xx == nHorEuroform) {
				moveIn3D ('x', fittings.ang, 0.120, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
				elemList.Push (placeFittings (fittings));
			// 나머지 행
			} else {
				elemList.Push (placeFittings (fittings));
				moveIn3D ('x', fittings.ang, -width [xx], &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
			}
		}
		moveIn3D ('z', fittings.ang, (verticalGap + verticalBarRightOffset + 0.081) - params.height + (verticalGap + verticalBarLeftOffset + 0.081) - 0.162, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
		moveIn3D ('x', fittings.ang, -(0.300 - params.width), &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);

		if (nVerticalBar > 1) {
			// 마지막 열
			for (xx = 0 ; xx <= nHorEuroform ; ++xx) {
				// 1행
				if (xx == 0) {
					elemList.Push (placeFittings (fittings));
					moveIn3D ('x', fittings.ang, -(width [xx] - 0.180), &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
				// 마지막 행
				} else if (xx == nHorEuroform) {
					moveIn3D ('x', fittings.ang, 0.120, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
					elemList.Push (placeFittings (fittings));
				// 나머지 행
				} else {
					elemList.Push (placeFittings (fittings));
					moveIn3D ('x', fittings.ang, -width [xx], &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
				}
			}
		}

		// 그룹화하기
		if (!elemList.IsEmpty ()) {
			GSSize nElems = elemList.GetSize ();
			API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
			if (elemHead != NULL) {
				for (GSIndex i = 0; i < nElems; i++)
					(*elemHead)[i].guid = elemList[i];

				ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

				BMKillHandle ((GSHandle *) &elemHead);
			}
		}
		elemList.Clear (false);
	}

	return	err;
}

// 테이블폼(슬래브) 배치
API_Guid	placeTableformOnSlabBottom (SlabTableform params)
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
		return element.header.guid;
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
	element.object.xRatio = params.horLen;
	element.object.yRatio = params.verLen;
	element.object.angle = params.ang;
	element.header.floorInd = floorInd;
	element.header.layer = layerInd_SlabTableform;

	// 타입
	setParameterByName (&memo, "type", params.type);

	// 객체 배치
	ACAPI_Element_Create (&element, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	// ==================== 여기부터는 슬래브 테이블폼의 부속품을 배치함 ====================
	double	marginEnds;

	// 이동하여 위치 바로잡기
	element.object.pos.x += ( params.verLen * sin(params.ang) );
	element.object.pos.y -= ( params.verLen * cos(params.ang) );

	// C형강 설치
	KSProfile	profile;

	profile.leftBottomX = params.leftBottomX;
	profile.leftBottomY = params.leftBottomY;
	profile.leftBottomZ = params.leftBottomZ;
	profile.ang = params.ang - DegreeToRad (90.0);
	profile.len = floor (params.horLen * 10) / 10;
	profile.angX = DegreeToRad (270.0);
	profile.angY = DegreeToRad (0.0);

	marginEnds = params.horLen - profile.len;

	moveIn3D ('x', profile.ang, -(0.300 - 0.006 - 0.020), &profile.leftBottomX, &profile.leftBottomY, &profile.leftBottomZ);
	moveIn3D ('y', profile.ang, marginEnds / 2, &profile.leftBottomX, &profile.leftBottomY, &profile.leftBottomZ);
	moveIn3D ('z', profile.ang, -0.0615, &profile.leftBottomX, &profile.leftBottomY, &profile.leftBottomZ);
	elemList.Push (placeProfile (profile));
	moveIn3D ('x', profile.ang, -(-(0.300 - 0.006 - 0.020) + params.verLen - (0.300 + 0.006 + 0.020)), &profile.leftBottomX, &profile.leftBottomY, &profile.leftBottomZ);
	elemList.Push (placeProfile (profile));

	profile.ang = params.ang + DegreeToRad (90.0);
	profile.leftBottomX = params.leftBottomX;
	profile.leftBottomY = params.leftBottomY;
	moveIn3D ('x', profile.ang, params.verLen, &profile.leftBottomX, &profile.leftBottomY, &profile.leftBottomZ);
	moveIn3D ('y', profile.ang, -params.horLen, &profile.leftBottomX, &profile.leftBottomY, &profile.leftBottomZ);
	moveIn3D ('x', profile.ang, -(0.300 - 0.006 - 0.020), &profile.leftBottomX, &profile.leftBottomY, &profile.leftBottomZ);
	moveIn3D ('y', profile.ang, marginEnds / 2, &profile.leftBottomX, &profile.leftBottomY, &profile.leftBottomZ);
	elemList.Push (placeProfile (profile));
	moveIn3D ('x', profile.ang, -(-(0.300 - 0.006 - 0.020) + params.verLen - (0.300 + 0.006 + 0.020)), &profile.leftBottomX, &profile.leftBottomY, &profile.leftBottomZ);
	elemList.Push (placeProfile (profile));

	// 결합철물 (사각와셔활용) 설치
	MetalFittingsWithRectWasher	fittings;

	fittings.leftBottomX = params.leftBottomX;
	fittings.leftBottomY = params.leftBottomY;
	fittings.leftBottomZ = params.leftBottomZ;
	fittings.ang = params.ang;
	fittings.angX = DegreeToRad (270.0);
	fittings.angY = DegreeToRad (0.0);
	fittings.bolt_len = 0.150;
	fittings.bolt_dia = 0.012;
	fittings.bWasher1 = false;
	fittings.washer_pos1 = 0.0;
	fittings.bWasher2 = true;
	fittings.washer_pos2 = 0.0766;
	fittings.washer_size = 0.100;
	strncpy (fittings.nutType, "육각너트", strlen ("육각너트"));

	moveIn3D ('y', fittings.ang, params.verLen, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);

	moveIn3D ('x', fittings.ang, 0.328, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
	moveIn3D ('y', fittings.ang, -0.300, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
	moveIn3D ('z', fittings.ang, -0.0499, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
	elemList.Push (placeFittings (fittings));
	moveIn3D ('y', fittings.ang, 0.300 - params.verLen + 0.300, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
	elemList.Push (placeFittings (fittings));
	moveIn3D ('x', fittings.ang, -0.328 + params.horLen - 0.328, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
	elemList.Push (placeFittings (fittings));
	moveIn3D ('y', fittings.ang, -0.300 + params.verLen - 0.300, &fittings.leftBottomX, &fittings.leftBottomY, &fittings.leftBottomZ);
	elemList.Push (placeFittings (fittings));
	
	return	element.header.guid;
}

// 유로폼 배치
API_Guid	placeEuroform (Euroform params)
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
		return element.header.guid;
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
		moveIn2D ('x', params.ang, params.height, &element.object.pos.x, &element.object.pos.y);
	}
	setParameterByName (&memo, "u_ins", tempString);
	setParameterByName (&memo, "ang_x", params.ang_x);	// 회전X

	// 객체 배치
	ACAPI_Element_Create (&element, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	element.header.guid;
}

// 스틸폼 배치
API_Guid	placeSteelform (Euroform params)
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
		return element.header.guid;
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

	return	element.header.guid;
}

// 합판 배치
API_Guid	placePlywood (Plywood params)
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
		return element.header.guid;
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
	setParameterByName (&memo, "sogak", 0.0);
	setParameterByName (&memo, "prof", "소각");

	// 객체 배치
	ACAPI_Element_Create (&element, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	element.header.guid;
}

// 휠러스페이서 배치
API_Guid	placeFillersp (FillerSpacer params)
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
		return element.header.guid;
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

	return	element.header.guid;
}

// 아웃코너앵글 배치
API_Guid	placeOutcornerAngle (OutcornerAngle params)
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
		return element.header.guid;
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

	return	element.header.guid;
}

// 아웃코너판넬 배치
API_Guid	placeOutcornerPanel (OutcornerPanel params)
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
		return element.header.guid;
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

	return	element.header.guid;
}

// 인코너판넬 배치
API_Guid	placeIncornerPanel (IncornerPanel params)
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
		return element.header.guid;
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

	return	element.header.guid;
}

// KS프로파일 배치
API_Guid	placeProfile (KSProfile params)
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
	GS::ucscpy (libPart.file_UName, L("KS프로파일v1.0.gsm"));
	err = ACAPI_LibPart_Search (&libPart, false);
	if (err != NoError)
		return element.header.guid;
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
	element.header.layer = layerInd_Profile;

	setParameterByName (&memo, "type", "보");				// 분류
	setParameterByName (&memo, "shape", "C형강");			// 형태
	setParameterByName (&memo, "iAnchor", 8);				// 앵커 포인트 (8, 하단)
	setParameterByName (&memo, "nom", "75 x 40 x 5 x 7");	// 규격

	setParameterByName (&memo, "len", params.len);			// 길이
	setParameterByName (&memo, "ZZYZX", params.len);		// 길이
	setParameterByName (&memo, "angX", params.angX);		// 회전X
	setParameterByName (&memo, "angY", params.angY);		// 회전Y

	// 객체 배치
	ACAPI_Element_Create (&element, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	element.header.guid;
}

// 결합철물 (사각와셔활용) 배치
API_Guid	placeFittings (MetalFittingsWithRectWasher params)
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
	GS::ucscpy (libPart.file_UName, L("결합철물 (사각와셔활용) v1.0.gsm"));
	err = ACAPI_LibPart_Search (&libPart, false);
	if (err != NoError)
		return element.header.guid;
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
	element.header.layer = layerInd_Join;

	setParameterByName (&memo, "angX", params.angX);	// 회전X
	setParameterByName (&memo, "angY", params.angY);	// 회전Y

	setParameterByName (&memo, "bolt_len", params.bolt_len);			// 볼트 길이
	setParameterByName (&memo, "bolt_dia", params.bolt_dia);			// 볼트 직경
	setParameterByName (&memo, "bWasher1", params.bWasher1);			// 와셔1 On/Off
	setParameterByName (&memo, "washer_pos1", params.washer_pos1);		// 와셔1 위치
	setParameterByName (&memo, "bWasher2", params.bWasher2);			// 와셔2 On/Off
	setParameterByName (&memo, "washer_pos2", params.washer_pos2);		// 와셔2 위치
	setParameterByName (&memo, "washer_size", params.washer_size);		// 와셔 크기
	setParameterByName (&memo, "nutType", params.nutType);				// 너트 타입

	// 객체 배치
	ACAPI_Element_Create (&element, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	element.header.guid;
}

// 비계 파이프 배치
API_Guid	placeSqrPipe (SquarePipe params)
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
	GS::ucscpy (libPart.file_UName, L("비계파이프v1.0.gsm"));
	err = ACAPI_LibPart_Search (&libPart, false);
	if (err != NoError)
		return element.header.guid;
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
	element.header.layer = layerInd_RectPipe;

	setParameterByName (&memo, "p_comp", "사각파이프");		// 사각파이프
	setParameterByName (&memo, "p_leng", params.length);	// 길이
	setParameterByName (&memo, "p_ang", params.pipeAng);	// 각도

	// 객체 배치
	ACAPI_Element_Create (&element, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	element.header.guid;
}

// 핀볼트 세트 배치
API_Guid	placePinbolt (PinBoltSet params)
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
	GS::ucscpy (libPart.file_UName, L("핀볼트세트v1.0.gsm"));
	err = ACAPI_LibPart_Search (&libPart, false);
	if (err != NoError)
		return element.header.guid;
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
	element.header.layer = layerInd_PinBolt;

	// 핀볼트 90도 회전
	if (params.bPinBoltRot90)
		setParameterByName (&memo, "bRotated", 1.0);
	else
		setParameterByName (&memo, "bRotated", 0.0);

	setParameterByName (&memo, "bolt_len", params.boltLen);		// 볼트 길이
	setParameterByName (&memo, "bolt_dia", 0.010);				// 볼트 직경
	setParameterByName (&memo, "washer_pos", 0.050);			// 와셔 위치
	setParameterByName (&memo, "washer_size", 0.100);			// 와셔 크기
	setParameterByName (&memo, "angX", params.angX);			// X축 회전
	setParameterByName (&memo, "angY", params.angY);			// Y축 회전

	// 객체 배치
	ACAPI_Element_Create (&element, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	element.header.guid;
}

// 벽체 타이 배치
API_Guid	placeWalltie (WallTie params)
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
	GS::ucscpy (libPart.file_UName, L("벽체 타이 v1.0.gsm"));
	err = ACAPI_LibPart_Search (&libPart, false);
	if (err != NoError)
		return element.header.guid;
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
	element.object.angle = params.ang + DegreeToRad (90.0);
	element.header.floorInd = floorInd;
	element.header.layer = layerInd_WallTie;

	setParameterByName (&memo, "boltLen", params.boltLen);		// 볼트 길이 (벽 두께 + 327mm 초과이며 100 단위로 나눠지는 가장 작은 수)
	setParameterByName (&memo, "boltDia", 0.012);				// 볼트 직경
	setParameterByName (&memo, "bSqrWasher", 1.0);				// 사각와샤
	setParameterByName (&memo, "washer_size", 0.100);			// 사각와샤 크기
	setParameterByName (&memo, "nutType", "타입 1");			// 너트 타입
	setParameterByName (&memo, "bEmbedPipe", 1.0);				// 벽체 내장 파이프
	setParameterByName (&memo, "pipeInnerDia", 0.012);			// 파이프 내경
	setParameterByName (&memo, "pipeThk", 0.002);				// 파이프 두께
	
	// 파이프 시작점, 끝점 (벽 두께만큼 차이)
	setParameterByName (&memo, "pipeBeginPos", params.pipeBeg);
	setParameterByName (&memo, "pipeEndPos", params.pipeEnd);
	
	// 좌,우측 조임쇠 위치 (벽 두께 + 327mm 만큼 차이)
	setParameterByName (&memo, "posLClamp", params.clampBeg);
	setParameterByName (&memo, "posRClamp", params.clampEnd);
	
	setParameterByName (&memo, "angY", DegreeToRad (0.0));		// 회전 Y

	// 객체 배치
	ACAPI_Element_Create (&element, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	element.header.guid;
}

// 헤드피스 배치 (세로 방향: 타입 A)
API_Guid	placeHeadpiece_ver (HeadpieceOfPushPullProps params)
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
	GS::ucscpy (libPart.file_UName, L("RS Push-Pull Props 헤드피스 v2.0 (인양고리 포함).gsm"));
	err = ACAPI_LibPart_Search (&libPart, false);
	if (err != NoError)
		return element.header.guid;
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
	element.header.layer = layerInd_HeadPiece;

	setParameterByName (&memo, "type", "타입 A");			// 타입
	setParameterByName (&memo, "plateThk", 0.009);			// 철판 두께
	setParameterByName (&memo, "angX", DegreeToRad (0.0));	// 회전X
	setParameterByName (&memo, "angY", DegreeToRad (0.0));	// 회전Y

	// 객체 배치
	ACAPI_Element_Create (&element, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	element.header.guid;
}

// 헤드피스 배치 (가로 방향: 타입 B)
API_Guid	placeHeadpiece_hor (HeadpieceOfPushPullProps params)
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
	GS::ucscpy (libPart.file_UName, L("RS Push-Pull Props 헤드피스 v2.0 (인양고리 포함).gsm"));
	err = ACAPI_LibPart_Search (&libPart, false);
	if (err != NoError)
		return element.header.guid;
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
	element.header.layer = layerInd_HeadPiece;

	setParameterByName (&memo, "type", "타입 B");			// 타입
	setParameterByName (&memo, "plateThk", 0.009);			// 철판 두께
	setParameterByName (&memo, "angX", DegreeToRad (0.0));	// 회전X
	setParameterByName (&memo, "angY", DegreeToRad (90.0));	// 회전Y
	element.object.level += 0.200;

	// 객체 배치
	ACAPI_Element_Create (&element, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	element.header.guid;
}

// 사각파이프 연결철물 배치
API_Guid	placeFittings (MetalFittings params)
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
	GS::ucscpy (libPart.file_UName, L("사각파이프 연결철물 v1.0.gsm"));
	err = ACAPI_LibPart_Search (&libPart, false);
	if (err != NoError)
		return element.header.guid;
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
	element.header.layer = layerInd_Join;

	setParameterByName (&memo, "angX", params.angX);	// 회전X
	setParameterByName (&memo, "angY", params.angY);	// 회전Y

	// 객체 배치
	ACAPI_Element_Create (&element, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	element.header.guid;
}

// 빔조인트용 Push-Pull Props 배치 (세로 방향: 타입 A)
API_Guid	placeJointHeadpeace_ver (HeadpieceOfPushPullProps params)
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
	GS::ucscpy (libPart.file_UName, L("빔조인트용 Push-Pull Props 헤드피스 v1.0.gsm"));
	err = ACAPI_LibPart_Search (&libPart, false);
	if (err != NoError)
		return element.header.guid;
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
	element.header.layer = layerInd_HeadPiece;

	// 타입 지정
	setParameterByName (&memo, "type", "타입 A");

	// 객체 배치
	ACAPI_Element_Create (&element, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	element.header.guid;
}

// 빔조인트용 Push-Pull Props 배치 (가로 방향: 타입 B)
API_Guid	placeJointHeadpeace_hor (HeadpieceOfPushPullProps params)
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
	GS::ucscpy (libPart.file_UName, L("빔조인트용 Push-Pull Props 헤드피스 v1.0.gsm"));
	err = ACAPI_LibPart_Search (&libPart, false);
	if (err != NoError)
		return element.header.guid;
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
	element.header.layer = layerInd_HeadPiece;

	// 타입 지정
	setParameterByName (&memo, "type", "타입 B");

	// 객체 배치
	ACAPI_Element_Create (&element, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	element.header.guid;
}

// 유로폼 후크 배치
API_Guid	placeEuroformHook (EuroformHook params)
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
	GS::ucscpy (libPart.file_UName, L("유로폼 후크.gsm"));
	err = ACAPI_LibPart_Search (&libPart, false);
	if (err != NoError)
		return element.header.guid;
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
	element.object.angle = params.ang + DegreeToRad (180.0);
	element.header.floorInd = floorInd;
	element.header.layer = layerInd_EuroformHook;

	setParameterByName (&memo, "rotationX", params.angX);			// X축 회전
	setParameterByName (&memo, "rotationY", params.angY);			// Y축 회전
	setParameterByName (&memo, "iHookType", params.iHookType);		// (1)수직-대, (2)수평-소
	setParameterByName (&memo, "iHookShape", params.iHookShape);	// (1)원형, (2)사각

	// 객체 배치
	ACAPI_Element_Create (&element, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	element.header.guid;
}

// 각파이프 행거 배치
API_Guid	placeRectpipeHanger (RectPipeHanger params)
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
	GS::ucscpy (libPart.file_UName, L("각파이프행거.gsm"));
	err = ACAPI_LibPart_Search (&libPart, false);
	if (err != NoError)
		return element.header.guid;
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
	element.object.angle = params.ang - DegreeToRad (90);
	element.header.floorInd = floorInd;
	element.header.layer = layerInd_RectpipeHanger;

	setParameterByName (&memo, "m_type", "각파이프행거");	// 품명
	setParameterByName (&memo, "angX", params.angX);		// 회전X
	setParameterByName (&memo, "angY", params.angY);		// 회전Y

	// 객체 배치
	ACAPI_Element_Create (&element, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	element.header.guid;
}

// 타공을 위한 기둥 객체를 배치하고 숨김, "원통 19" 객체를 이용함
API_Guid	placeHole (API_Guid guid_Target, Cylinder operator_Object)
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
	GS::ucscpy (libPart.file_UName, L("원통 19.gsm"));
	err = ACAPI_LibPart_Search (&libPart, false);
	if (err != NoError)
		return element.header.guid;
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
	element.object.pos.x = operator_Object.leftBottomX;
	element.object.pos.y = operator_Object.leftBottomY;
	element.object.level = operator_Object.leftBottomZ;
	element.object.xRatio = aParam;
	element.object.yRatio = bParam;
	element.object.angle = operator_Object.ang;
	element.header.floorInd = floorInd;
	element.header.layer = layerInd_Hidden;

	// 편집 모드는 "각도-길이" 고정
	setParameterByName (&memo, "edit_mode", "각도-길이");
	setParameterByName (&memo, "end_mode", "직각");
	setParameterByName (&memo, "gamma", operator_Object.angleFromPlane);
	setParameterByName (&memo, "length", operator_Object.length);
	setParameterByName (&memo, "radius_1", operator_Object.radius);

	// 객체 배치
	ACAPI_Element_Create (&element, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	// guid_Target 타공하기
	ACAPI_Element_SolidLink_Create (guid_Target, element.header.guid, APISolid_Substract, 0);

	return	element.header.guid;
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

			// 체크박스: 레이어 묶음
			DGSetItemText (dialogID, CHECKBOX_LAYER_COUPLING, "레이어 묶음");
			DGSetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING, TRUE);

			// 레이어 관련 라벨
			DGSetItemText (dialogID, LABEL_LAYER_SETTINGS, "부재별 레이어 설정");
			DGSetItemText (dialogID, LABEL_LAYER_SLABTABLEFORM, "슬래브 테이블폼");
			DGSetItemText (dialogID, LABEL_LAYER_PROFILE, "C형강");
			DGSetItemText (dialogID, LABEL_LAYER_EUROFORM, "유로폼");
			DGSetItemText (dialogID, LABEL_LAYER_RECTPIPE, "비계 파이프");
			DGSetItemText (dialogID, LABEL_LAYER_PINBOLT, "핀볼트 세트");
			DGSetItemText (dialogID, LABEL_LAYER_WALLTIE, "벽체 타이");
			DGSetItemText (dialogID, LABEL_LAYER_JOIN, "결합철물");
			DGSetItemText (dialogID, LABEL_LAYER_HEADPIECE, "헤드피스");
			DGSetItemText (dialogID, LABEL_LAYER_STEELFORM, "스틸폼");
			DGSetItemText (dialogID, LABEL_LAYER_PLYWOOD, "합판");
			DGSetItemText (dialogID, LABEL_LAYER_FILLERSP, "휠러스페이서");
			DGSetItemText (dialogID, LABEL_LAYER_OUTCORNER_ANGLE, "아웃코너앵글");
			DGSetItemText (dialogID, LABEL_LAYER_OUTCORNER_PANEL, "아웃코너판넬");
			DGSetItemText (dialogID, LABEL_LAYER_INCORNER_PANEL, "인코너판넬");
			DGSetItemText (dialogID, LABEL_LAYER_RECTPIPE_HANGER, "각파이프행거");
			DGSetItemText (dialogID, LABEL_LAYER_EUROFORM_HOOK, "유로폼 후크");
			DGSetItemText (dialogID, LABEL_LAYER_HIDDEN, "숨김");

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

			ucb.itemID	 = USERCONTROL_LAYER_RECTPIPE_HANGER;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, 1);
			if (bLayerInd_RectpipeHanger == true) {
				DGEnableItem (dialogID, LABEL_LAYER_RECTPIPE_HANGER);
				DGEnableItem (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_RECTPIPE_HANGER);
				DGDisableItem (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER);
			}

			ucb.itemID	 = USERCONTROL_LAYER_EUROFORM_HOOK;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, 1);
			if (bLayerInd_EuroformHook == true) {
				DGEnableItem (dialogID, LABEL_LAYER_EUROFORM_HOOK);
				DGEnableItem (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_EUROFORM_HOOK);
				DGDisableItem (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK);
			}

			ucb.itemID	 = USERCONTROL_LAYER_HIDDEN;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_HIDDEN, 1);
			if (bLayerInd_Hidden == true) {
				DGEnableItem (dialogID, LABEL_LAYER_HIDDEN);
				DGEnableItem (dialogID, USERCONTROL_LAYER_HIDDEN);
			} else {
				DGDisableItem (dialogID, LABEL_LAYER_HIDDEN);
				DGDisableItem (dialogID, USERCONTROL_LAYER_HIDDEN);
			}
			break;

		case DG_MSG_CHANGE:
			// 레이어 같이 바뀜
			if (DGGetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING) == 1) {
				switch (item) {
					case USERCONTROL_LAYER_SLABTABLEFORM:
						//DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM));
						break;
					case USERCONTROL_LAYER_PROFILE:
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE));
						//DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE));
						break;
					case USERCONTROL_LAYER_EUROFORM:
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
						//DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM));
						break;
					case USERCONTROL_LAYER_RECTPIPE:
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
						//DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE));
						break;
					case USERCONTROL_LAYER_PINBOLT:
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
						//DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT));
						break;
					case USERCONTROL_LAYER_WALLTIE:
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE));
						//DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE));
						break;
					case USERCONTROL_LAYER_JOIN:
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
						//DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN));
						break;
					case USERCONTROL_LAYER_HEADPIECE:
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
						//DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE));
						break;
					case USERCONTROL_LAYER_STEELFORM:
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM));
						//DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP, DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM));
						break;
					case USERCONTROL_LAYER_PLYWOOD:
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
						//DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD));
						break;
					case USERCONTROL_LAYER_FILLERSP:
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP));
						//DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP, DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP));
						break;
					case USERCONTROL_LAYER_OUTCORNER_ANGLE:
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE));
						//DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE));
						break;
					case USERCONTROL_LAYER_OUTCORNER_PANEL:
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL));
						//DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL));
						break;
					case USERCONTROL_LAYER_INCORNER_PANEL:
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL));
						//DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL));
						break;
					case USERCONTROL_LAYER_RECTPIPE_HANGER:
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER));
						//DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER));
						break;
					case USERCONTROL_LAYER_EUROFORM_HOOK:
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK));
						DGSetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK));
						//DGSetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK, DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK));
						break;
				}
			}

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					// 레이어 번호 저장
					if (bLayerInd_SlabTableform == true)	layerInd_SlabTableform	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_SLABTABLEFORM);
					if (bLayerInd_Profile == true)			layerInd_Profile		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PROFILE);
					if (bLayerInd_Euroform == true)			layerInd_Euroform		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM);
					if (bLayerInd_RectPipe == true)			layerInd_RectPipe		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE);
					if (bLayerInd_PinBolt == true)			layerInd_PinBolt		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PINBOLT);
					if (bLayerInd_WallTie == true)			layerInd_WallTie		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_WALLTIE);
					if (bLayerInd_Join == true)				layerInd_Join			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN);
					if (bLayerInd_HeadPiece == true)		layerInd_HeadPiece		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_HEADPIECE);
					if (bLayerInd_Steelform == true)		layerInd_Steelform		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_STEELFORM);
					if (bLayerInd_Plywood == true)			layerInd_Plywood		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PLYWOOD);
					if (bLayerInd_Fillersp == true)			layerInd_Fillersp		= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_FILLERSP);
					if (bLayerInd_OutcornerAngle == true)	layerInd_OutcornerAngle	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_ANGLE);
					if (bLayerInd_OutcornerPanel == true)	layerInd_OutcornerPanel	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_OUTCORNER_PANEL);
					if (bLayerInd_IncornerPanel == true)	layerInd_IncornerPanel	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_INCORNER_PANEL);
					if (bLayerInd_RectpipeHanger == true)	layerInd_RectpipeHanger	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_RECTPIPE_HANGER);
					if (bLayerInd_EuroformHook == true)		layerInd_EuroformHook	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_EUROFORM_HOOK);
					if (bLayerInd_Hidden == true)			layerInd_Hidden			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_HIDDEN);

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
