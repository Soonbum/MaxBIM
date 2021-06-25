/**
 * @file Contains the functions required by ArchiCAD.
 */

#include "MaxBIM.hpp"

#include "WallEuroformPlacer.hpp"
#include "SlabEuroformPlacer.hpp"
#include "BeamEuroformPlacer.hpp"
#include "ColumnEuroformPlacer.hpp"

#include "WallTableformPlacer.hpp"
#include "SlabTableformPlacer.hpp"

#include "LibraryConvert.hpp"

#include "Layers.hpp"

#include "Export.hpp"

#include "Quantities.hpp"

#include "Information.hpp"

#include "UtilityFunctions.hpp"

#define	MDID_DEVELOPER_ID	829517673
#define	MDID_LOCAL_ID		3588511626


/**
 * Dependency definitions. Function name is fixed.
 *
 * @param envir [in] ArchiCAD environment values.
 * @return The Add-On loading type.
 */
API_AddonType	__ACENV_CALL	CheckEnvironment (API_EnvirParams* envir)
{
	if (envir->serverInfo.serverApplication != APIAppl_ArchiCADID)
		return APIAddon_DontRegister;

	ACAPI_Resource_GetLocStr (envir->addOnInfo.name, 32000, 1);
	ACAPI_Resource_GetLocStr (envir->addOnInfo.description, 32000, 2);

	return APIAddon_Normal;
}		// CheckEnvironment ()

/**
 * Interface definitions. Function name is fixed.
 *
 * @return ArchiCAD error code.
 */
GSErrCode	__ACENV_CALL	RegisterInterface (void)
{
	//
	// Register a menu
	//
	GSErrCode err;
	
	err = ACAPI_Register_Menu (32001, 32002, MenuCode_UserDef, MenuFlag_Default);	// 유로폼 배치
	err = ACAPI_Register_Menu (32011, 32012, MenuCode_UserDef, MenuFlag_Default);	// 테이블폼 배치
	err = ACAPI_Register_Menu (32013, 32014, MenuCode_UserDef, MenuFlag_Default);	// Library Converting
	err = ACAPI_Register_Menu (32005, 32006, MenuCode_UserDef, MenuFlag_Default);	// 레이어 유틸
	err = ACAPI_Register_Menu (32007, 32008, MenuCode_UserDef, MenuFlag_Default);	// 내보내기
	err = ACAPI_Register_Menu (32009, 32010, MenuCode_UserDef, MenuFlag_Default);	// 물량 산출
	err = ACAPI_Register_Menu (32003, 32004, MenuCode_UserDef, MenuFlag_Default);	// 정보

	return err;
}		// RegisterInterface ()

/**
 * Menu command handler function. Function name is NOT fixed. There can be
 * more than one of these functions. Please check the API Development Kit
 * documentation for more information.
 *
 * @param params [in] Parameters of the menu command.
 * @return ArchiCAD error code.
 */
GSErrCode __ACENV_CALL	MenuCommandHandler (const API_MenuParams *menuParams)
{
	GSErrCode	err = NoError;

	switch (menuParams->menuItemRef.menuResID) {
		case 32001:
			// 유로폼 배치
			switch (menuParams->menuItemRef.itemIndex) {
				case 1:
					// 벽에 유로폼 배치하기
					err = ACAPI_CallUndoableCommand ("벽에 유로폼 배치", [&] () -> GSErrCode {
						err = placeEuroformOnWall ();
						return err;
					});
					break;
				case 2:
					// 슬래브 하부에 유로폼 배치하기
					err = ACAPI_CallUndoableCommand ("슬래브 하부에 유로폼 배치", [&] () -> GSErrCode {
						err = placeEuroformOnSlabBottom ();
						return err;
					});
					break;
				case 3:
					// 보에 유로폼 배치하기
					err = ACAPI_CallUndoableCommand ("보에 유로폼 배치", [&] () -> GSErrCode {
						err = placeEuroformOnBeam ();
						return err;
					});
					break;
				case 4:
					// 기둥에 유로폼 배치하기
					err = ACAPI_CallUndoableCommand ("기둥에 유로폼 배치", [&] () -> GSErrCode {
						err = placeEuroformOnColumn ();
						return err;
					});
					break;
			}
			break;
		case 32011:
			// 테이블폼 배치
			switch (menuParams->menuItemRef.itemIndex) {
				case 1:
					// 벽에 테이블폼 배치하기 - 세로 방향
					err = ACAPI_CallUndoableCommand ("벽에 테이블폼 배치 - 세로 방향", [&] () -> GSErrCode {
						err = placeTableformOnWall_Vertical ();
						return err;
					});
					break;
				case 2:
					// 벽에 테이블폼 배치하기 - 가로 방향
					err = ACAPI_CallUndoableCommand ("벽에 테이블폼 배치 - 가로 방향", [&] () -> GSErrCode {
						err = placeTableformOnWall_Horizontal ();
						return err;
					});
					break;
				case 3:
					// 슬래브 하부에 테이블폼 배치하기
					err = ACAPI_CallUndoableCommand ("슬래브 하부에 테이블폼 배치", [&] () -> GSErrCode {
						err = placeTableformOnSlabBottom ();
						return err;
					});
					break;
			}
			break;
		case 32013:
			// Library Converting
			switch (menuParams->menuItemRef.itemIndex) {
				case 1:
					// 가상 가설재 모두 변환
					err = ACAPI_CallUndoableCommand ("가상 가설재 모두 변환", [&] () -> GSErrCode {
						err = convertVirtualTCO ();		// TCO: Temporary Construction Object
						return err;
					});
					break;
			}
			break;
		case 32005:
			// 레이어 유틸
			switch (menuParams->menuItemRef.itemIndex) {
				case 1:		// 레이어 쉽게 선택하기
					err = showLayersEasily ();
					break;
				case 2:		// 레이어 쉽게 만들기
					err = makeLayersEasily ();
					break;
				case 3:		// 레이어 쉽게 지정하기
					err = ACAPI_CallUndoableCommand ("선택한 객체들의 레이어 속성 변경", [&] () -> GSErrCode {
						err = assignLayerEasily ();
						return err;
					});
					break;
			}
			break;
		case 32007:
			// 내보내기
			switch (menuParams->menuItemRef.itemIndex) {
				case 1:		// 부재(기둥,보,슬래브) 정보 내보내기 (CSV) ... 개발보류
					err = exportGridElementInfo ();
					break;
				case 2:		// 선택한 부재 정보 내보내기 (Single 모드)
					err = exportSelectedElementInfo ();
					break;
				case 3:		// 선택한 부재 정보 내보내기 (Multi 모드)
					err = exportElementInfoOnVisibleLayers ();
					break;
				case 4:		// 부재별 선택 후 보여주기
					err = filterSelection ();
					break;
			}
			break;
		case 32009:
			// 물량 산출
			switch (menuParams->menuItemRef.itemIndex) {
				case 1:		// 물량합판 부착하기
					err = ACAPI_CallUndoableCommand ("물량합판 부착하기", [&] () -> GSErrCode {
						extern qElem qElemInfo;
						if (qElemInfo.dialogID == 0) {
							err = placeQuantityPlywood ();
							// 메뉴 텍스트 변경: 물량합판 부착하기 팔레트 창 닫기
						} else {
							if ((qElemInfo.dialogID != 0) || DGIsDialogOpen (qElemInfo.dialogID)) {
								DGModelessClose (qElemInfo.dialogID);
								qElemInfo.dialogID = 0;
								// 메뉴 텍스트 변경: 물량합판 부착하기
							}
							return NoError;
						}
						return err;
					});
					break;
			}
			break;
		case 32003:

			// 정보
			switch (menuParams->menuItemRef.itemIndex) {
				case 1:		// 애드온 사용법 보기
					err = showHelp ();
					break;
				case 2:		// MaxBIM 애드온 정보
					err = showAbout ();

					API_WindowInfo		windowInfo;

					BNZeroMemory (&windowInfo, sizeof (API_WindowInfo));
					windowInfo.typeID = APIWind_MyTextID;
					windowInfo.index  = 1;
					err = ACAPI_Database (APIDb_CloseWindowID, &windowInfo, NULL);

					break;
				case 3:		// 개발자 전용 - 개발자 테스트 메뉴
					err = ACAPI_CallUndoableCommand ("개발자 테스트", [&] () -> GSErrCode {
						GSErrCode	err = NoError;
						// *** 원하는 코드를 아래 넣으시오.

						//API_PetPaletteType  petPaletteInfo;
						//short**             petItemIdsHdl;
						//short               petItemIds [3] = { 32522, 32523, 32524 };

						//BNZeroMemory (&petPaletteInfo, sizeof (API_PetPaletteType));

						//// 펫 항목 아이콘들의 리소스 ID들을 포함하는 핸들 생성하기
						//short nIcons = sizeof (petItemIds) / sizeof (short);
						//petItemIdsHdl = (short**) BMhAll (nIcons * sizeof (short));
						//for (short i = 0; i < nIcons; i++)
						//	(*petItemIdsHdl)[i] = petItemIds[i];

						//// petPaletteInfo 채우기
						//petPaletteInfo.petPaletteID = 32200;
						//petPaletteInfo.nCols = 5;
						//petPaletteInfo.nRows = 1;
						//petPaletteInfo.value = 0;
						//petPaletteInfo.grayBits = 0;
						//petPaletteInfo.petIconIDsHdl = petItemIdsHdl;
						//petPaletteInfo.dhlpResourceID = 32200;

						//err = ACAPI_Interface (APIIo_PetPaletteID, &petPaletteInfo, PetPaletteCallBack);

						//BMhKill ((GSHandle *) &petItemIdsHdl);


						/*
						API_Element			elem;
						API_ElementMemo		memo;
						API_LibPart			libPart;

						const GS::uchar_t*	gsmName = L("물량합판(핫스팟축소).gsm");
						double				aParam;
						double				bParam;
						Int32				addParNum;

						char				tempStr [256];
						double				horLen, verLen;

						// 라인 입력
						API_GetPointType	pointInfo;
						API_GetPolyType		polyInfo;
						double				dx, dy, dz;

						BNZeroMemory (&pointInfo, sizeof (API_GetPointType));
						BNZeroMemory (&polyInfo, sizeof (API_GetPolyType));

						CHCopyC ("다각형의 1번째 노드를 클릭하십시오.", pointInfo.prompt);
						err = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);

						CHCopyC ("다각형 노드를 입력합니다.", polyInfo.prompt);
						polyInfo.startCoord = pointInfo.pos;
						polyInfo.method = APIPolyGetMethod_Polyline;
						polyInfo.getZCoords = true;
						err = ACAPI_Interface (APIIo_GetPolyID, &polyInfo, NULL);

						// 점 개수가 적으면 중지할 것
						if (polyInfo.nCoords < 3)
							return err;

						// 객체 로드
						BNZeroMemory (&elem, sizeof (API_Element));
						BNZeroMemory (&memo, sizeof (API_ElementMemo));
						BNZeroMemory (&libPart, sizeof (libPart));
						GS::ucscpy (libPart.file_UName, gsmName);
						err = ACAPI_LibPart_Search (&libPart, false);
						if (libPart.location != NULL)
							delete libPart.location;
						if (err != NoError)
							return err;

						ACAPI_LibPart_Get (&libPart);

						elem.header.typeID = API_ObjectID;
						elem.header.guid = GSGuid2APIGuid (GS::Guid (libPart.ownUnID));

						ACAPI_Element_GetDefaults (&elem, &memo);
						ACAPI_LibPart_GetParams (libPart.index, &aParam, &bParam, &addParNum, &memo.params);

						// 라이브러리의 파라미터 값 입력 (공통)
						elem.header.floorInd = 0;
						elem.object.libInd = libPart.index;
						elem.object.xRatio = aParam;
						elem.object.yRatio = bParam;
						elem.header.layer = 1;

						strcpy (tempStr, "벽체(합벽)");
						setParameterByName (&memo, "PANEL_MAT", 72.0);
						setParameterByName (&memo, "m_type", tempStr);

						// 합판두께 (문자열)
						strcpy (tempStr, "12");		// 12mm
						setParameterByName (&memo, "m_size", tempStr);

						// 설치위치
						strcpy (tempStr, "비규격");
						setParameterByName (&memo, "m_size1", tempStr);

						// 품명
						strcpy (tempStr, "합판면적");
						setParameterByName (&memo, "m_size2", tempStr);

						//for (Int32 ind = 1; ind <= polyInfo.nCoords; ind++)
						//	sprintf (tempStr, "[%2d]  (%Lf, %Lf)\n", ind, (*polyInfo.coords) [ind].x, (*polyInfo.coords) [ind].y);

						dx = (*polyInfo.coords) [2].x - (*polyInfo.coords) [1].x;
						dy = (*polyInfo.coords) [2].y - (*polyInfo.coords) [1].y;

						elem.object.angle = atan2 (dy, dx);
						elem.object.pos.x = (*polyInfo.coords) [1].x;
						elem.object.pos.y = (*polyInfo.coords) [1].y;
						elem.object.level = (*polyInfo.zCoords) [1];

						// 가로길이
						horLen = GetDistance ((*polyInfo.coords) [1].x, (*polyInfo.coords) [1].y, (*polyInfo.zCoords) [1], (*polyInfo.coords) [2].x, (*polyInfo.coords) [2].y, (*polyInfo.zCoords) [2]);
						setParameterByName (&memo, "NO_WD", horLen);
						elem.object.xRatio = horLen;

						// 세로길이
						verLen = GetDistance ((*polyInfo.coords) [2].x, (*polyInfo.coords) [2].y, (*polyInfo.zCoords) [2], (*polyInfo.coords) [3].x, (*polyInfo.coords) [3].y, (*polyInfo.zCoords) [3]);
						setParameterByName (&memo, "no_lg1", verLen);
						elem.object.yRatio = verLen;

						// 설치방향
						strcpy (tempStr, "벽에 세우기");	// 기본값
						if ((abs ((*polyInfo.zCoords) [1] - (*polyInfo.zCoords) [2]) < EPS) && (abs ((*polyInfo.zCoords) [2] - (*polyInfo.zCoords) [3]) < EPS)) {
							double angle1, angle2;

							dx = (*polyInfo.coords) [2].x - (*polyInfo.coords) [1].x;
							dy = (*polyInfo.coords) [2].y - (*polyInfo.coords) [1].y;
							angle1 = RadToDegree (atan2 (dy, dx));

							dx = (*polyInfo.coords) [3].x - (*polyInfo.coords) [2].x;
							dy = (*polyInfo.coords) [3].y - (*polyInfo.coords) [2].y;
							angle2 = RadToDegree (atan2 (dy, dx));

							// p2-p3 간의 각도가 p2-p1 간의 각도보다 90도 큼
							if (abs (angle2 - angle1 - 90) < EPS) {
								strcpy (tempStr, "바닥덮기");
							}

							// p2-p3 간의 각도가 p2-p1 간의 각도보다 90도 작음
							if (abs (angle1 - angle2 - 90) < EPS) {
								strcpy (tempStr, "바닥깔기");
								moveIn3D ('y', elem.object.angle, -verLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
							}
						} else {
							if ((abs ((*polyInfo.coords) [2].x - (*polyInfo.coords) [3].x) < EPS) && (abs ((*polyInfo.coords) [2].y - (*polyInfo.coords) [3].y) < EPS)) {
								// p2와 p3의 x, y 좌표는 같음
								strcpy (tempStr, "벽에 세우기");
							} else {
								// p2와 p3의 x, y 좌표는 다름
								strcpy (tempStr, "경사설치");

								// 설치각도: asin ((p3.z - p2.z) / (p3와 p2 간의 거리))
								// 설치각도: acos ((p3와 p2 간의 평면 상의 거리) / (p3와 p2 간의 거리))
								// 설치각도: atan2 ((p3.z - p2.z) / (p3와 p2 간의 평면 상의 거리))
								dx = GetDistance ((*polyInfo.coords) [2].x, (*polyInfo.coords) [2].y, (*polyInfo.coords) [3].x, (*polyInfo.coords) [3].y);
								dy = abs ((*polyInfo.zCoords) [3] - (*polyInfo.zCoords) [2]);
								dz = verLen;
								setParameterByName (&memo, "cons_ang", DegreeToRad (180.0) - acos (dx/dz));
								setParameterByName (&memo, "vertical_cut", 1.0);	// 수직깎기 적용됨
							}
						}
						setParameterByName (&memo, "CONS_DR", tempStr);

						// 객체 배치
						if ((horLen > EPS) && (verLen > EPS))
							ACAPI_Element_Create (&elem, &memo);

						ACAPI_DisposeElemMemoHdls (&memo);
						*/

						// *** 원하는 코드를 위에 넣으시오.
						return err;
					});

					break;
			}
			break;
	}

	return err;
}		// CommandHandler ()

/**
 * Called after the Add-On has been loaded into memory. Function name is fixed.
 *
 * @return ArchiCAD error code.
 */
GSErrCode __ACENV_CALL	Initialize (void)
{
	GSErrCode err;
	
	err = ACAPI_Install_MenuHandler (32001, MenuCommandHandler);	// 유로폼 배치
	err = ACAPI_Install_MenuHandler (32011, MenuCommandHandler);	// 테이블폼 배치
	err = ACAPI_Install_MenuHandler (32013, MenuCommandHandler);	// Library Converting
	err = ACAPI_Install_MenuHandler (32005, MenuCommandHandler);	// 레이어 유틸
	err = ACAPI_Install_MenuHandler (32007, MenuCommandHandler);	// 내보내기
	err = ACAPI_Install_MenuHandler (32009, MenuCommandHandler);	// 물량 산출
	err = ACAPI_Install_MenuHandler (32003, MenuCommandHandler);	// 정보

	// register special help location if needed
	// for Graphisoft's add-ons, this is the Help folder beside the installed ArchiCAD
	IO::Location		helpLoc;
	API_SpecFolderID	specID = API_HelpFolderID;

	ACAPI_Environment (APIEnv_GetSpecFolderID, &specID, (void*) &helpLoc);
	DG::RegisterAdditionalHelpLocation (MDID_DEVELOPER_ID, MDID_LOCAL_ID, &helpLoc);

	return err;
}		// Initialize ()

/**
 * Called when the Add-On is going to be unloaded. Function name is fixed.
 *
 * @return ArchiCAD error code.
 */
GSErrCode __ACENV_CALL	FreeData (void)
{
	DG::UnregisterAdditionalHelpLocation (MDID_DEVELOPER_ID, MDID_LOCAL_ID);

	return NoError;
}		// FreeData ()
