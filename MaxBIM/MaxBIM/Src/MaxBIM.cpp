/**
 * @file Contains the functions required by ArchiCAD.
 */

#include "MaxBIM.hpp"
#include "UtilityFunctions.hpp"

#include "WallTableformPlacer.hpp"
#include "SlabTableformPlacer.hpp"
#include "BeamTableformPlacer.hpp"
#include "ColumnTableformPlacer.hpp"

#include "Layers.hpp"
#include "Export.hpp"
#include "Quantities.hpp"
#include "Facility.hpp"

#include "Information.hpp"

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
	
	err = ACAPI_Register_Menu (32011, 32012, MenuCode_UserDef, MenuFlag_Default);	// 테이블폼 배치
	err = ACAPI_Register_Menu (32005, 32006, MenuCode_UserDef, MenuFlag_Default);	// 레이어 유틸
	err = ACAPI_Register_Menu (32007, 32008, MenuCode_UserDef, MenuFlag_Default);	// 내보내기
	err = ACAPI_Register_Menu (32009, 32010, MenuCode_UserDef, MenuFlag_Default);	// 반자동 배치
	err = ACAPI_Register_Menu (32013, 32014, MenuCode_UserDef, MenuFlag_Default);	// 편의 기능
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

	API_MenuItemRef	itemRef;
	GSFlags			itemFlags;

	switch (menuParams->menuItemRef.menuResID) {
		case 32011:
			// 테이블폼 배치
			switch (menuParams->menuItemRef.itemIndex) {
				case 1:
					// 벽에 테이블폼 배치하기
					err = ACAPI_CallUndoableCommand ("벽에 테이블폼 배치", [&] () -> GSErrCode {
						err = placeTableformOnWall ();
						return err;
					});
					break;
				case 2:
					// 슬래브 하부에 테이블폼 배치하기
					err = ACAPI_CallUndoableCommand ("슬래브 하부에 테이블폼 배치", [&] () -> GSErrCode {
						err = placeTableformOnSlabBottom ();
						return err;
					});
					break;
				case 3:
					// 보에 테이블폼 배치하기
					err = ACAPI_CallUndoableCommand ("보에 테이블폼 배치", [&] () -> GSErrCode {
						err = placeTableformOnBeam ();
						return err;
					});
					break;
				case 4:
					// 기둥에 테이블폼 배치하기
					err = ACAPI_CallUndoableCommand ("기둥에 테이블폼 배치", [&] () -> GSErrCode {
						err = placeTableformOnColumn ();
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
				case 4:		// 레이어 이름 검사하기
					err = inspectLayerNames ();
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
				case 5:		// 보 테이블폼 물량표 작성
					err = exportBeamTableformInformation ();
					break;
				case 6:		// 테이블폼 면적 계산
					err = calcTableformArea ();
					break;
				case 7:		// 콘크리트 물량 계산 (Single 모드)
					err = calcConcreteVolumeSingleMode ();
					break;
				case 8:		// 콘크리트 물량 계산 (Multi 모드)
					err = calcConcreteVolumeMultiMode ();
					break;
			}
			break;

		case 32009:
			// 반자동 배치
			switch (menuParams->menuItemRef.itemIndex) {
				case 1:		// 물량합판 부착하기
					BNZeroMemory (&itemRef, sizeof (API_MenuItemRef));
					itemRef.menuResID = 32009;
					itemRef.itemIndex = 1;
					itemFlags = 0;
					ACAPI_Interface (APIIo_GetMenuItemFlagsID, &itemRef, &itemFlags);

					extern qElem qElemInfo;

					if (qElemInfo.dialogID == 0) {
						err = placeQuantityPlywood ();
						itemFlags |= API_MenuItemChecked;
					} else {
						if ((qElemInfo.dialogID != 0) || DGIsDialogOpen (qElemInfo.dialogID)) {
							DGModelessClose (qElemInfo.dialogID);
							qElemInfo.dialogID = 0;
							itemFlags &= ~API_MenuItemChecked;
						}
					}
					ACAPI_Interface (APIIo_SetMenuItemFlagsID, &itemRef, &itemFlags);
					return err;

					break;

				case 2:		// 단열재 부착하기
					BNZeroMemory (&itemRef, sizeof (API_MenuItemRef));
					itemRef.menuResID = 32009;
					itemRef.itemIndex = 2;
					itemFlags = 0;
					ACAPI_Interface (APIIo_GetMenuItemFlagsID, &itemRef, &itemFlags);

					extern insulElem insulElemInfo;

					if (insulElemInfo.dialogID == 0) {
						err = placeInsulation ();
						itemFlags |= API_MenuItemChecked;
					} else {
						if ((insulElemInfo.dialogID != 0) || DGIsDialogOpen (insulElemInfo.dialogID)) {
							DGModelessClose (insulElemInfo.dialogID);
							insulElemInfo.dialogID = 0;
							itemFlags &= ~API_MenuItemChecked;
						}
					}
					ACAPI_Interface (APIIo_SetMenuItemFlagsID, &itemRef, &itemFlags);
					return err;

					break;
			}
			break;

		case 32013:
			// 편의 기능
			switch (menuParams->menuItemRef.itemIndex) {
				case 1:		// 3D 품질/속도 조정하기
					err = ACAPI_CallUndoableCommand ("3D 품질/속도 조정하기", [&] () -> GSErrCode {
						err = select3DQuality ();
						return err;
					});

					break;

				case 2:		// 영역에 3D 라벨 붙이기
					err = ACAPI_CallUndoableCommand ("영역에 3D 라벨 붙이기", [&] () -> GSErrCode {
						err = attach3DLabelOnZone ();
						return err;
					});

					break;
			}
			break;

		case 32003:
			// 정보
			switch (menuParams->menuItemRef.itemIndex) {
				case 1:		// 애드온 사용법 보기
					BNZeroMemory (&itemRef, sizeof (API_MenuItemRef));
					itemRef.menuResID = 32003;
					itemRef.itemIndex = 1;
					itemFlags = 0;
					ACAPI_Interface (APIIo_GetMenuItemFlagsID, &itemRef, &itemFlags);

					extern short modelessDialogID;

					if (modelessDialogID == 0) {
						err = showHelp ();
						itemFlags |= API_MenuItemChecked;
					} else {
						if ((modelessDialogID != 0) || DGIsDialogOpen (modelessDialogID)) {
							DGModelessClose (modelessDialogID);
							modelessDialogID = 0;
							itemFlags &= ~API_MenuItemChecked;
						}
					}
					ACAPI_Interface (APIIo_SetMenuItemFlagsID, &itemRef, &itemFlags);
					return err;

					break;
				case 2:		// MaxBIM 애드온 정보
					err = showAbout ();

					// *** 텍스트 창 닫기 루틴
					//API_WindowInfo		windowInfo;

					//BNZeroMemory (&windowInfo, sizeof (API_WindowInfo));
					//windowInfo.typeID = APIWind_MyTextID;
					//windowInfo.index  = 1;
					//err = ACAPI_Database (APIDb_CloseWindowID, &windowInfo, NULL);

					break;
				case 3:		// 개발자 전용 - 개발자 테스트 메뉴
					err = ACAPI_CallUndoableCommand ("개발자 테스트", [&] () -> GSErrCode {
						GSErrCode	err = NoError;
						// *** 원하는 코드를 아래 넣으시오.
						bool				regenerate = true;
						//char				filename [256];

						API_DatabaseUnId*	dbases = NULL;
						GSSize				nDbases = 0;
						API_WindowInfo		windowInfo;
						API_DatabaseInfo	currentDB;
						API_Element			elem;
						GS::Array<API_Guid>	elemList;

						API_FileSavePars	fsp;
						API_SavePars_Pdf	pars_pdf;
						API_SavePars_Picture	pars_pict;

						API_SpecFolderID	specFolderID = API_ApplicationFolderID;
						IO::Location		location;
						GS::UniString		resultString;
						API_MiscAppInfo		miscAppInfo;

						ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
						ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);
						
						//////////////////////////////////////////////
						ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);

						//ACAPI_Environment (APIEnv_GetMiscAppInfoID, &miscAppInfo);
						//sprintf (filename, "테스트.csv", miscAppInfo.caption);
							
						BNZeroMemory (&fsp, sizeof (API_FileSavePars));
						//fsp.fileTypeID = APIFType_PdfFile;
						fsp.fileTypeID = APIFType_PNGFile;
						fsp.file = new IO::Location (location, IO::Name ("test.png"));

						BNZeroMemory (&pars_pdf, sizeof (API_SavePars_Pdf));
						pars_pdf.leftMargin = 0.0;
						pars_pdf.rightMargin = 0.0;
						pars_pdf.topMargin = 0.0;
						pars_pdf.bottomMargin = 0.0;
						pars_pdf.sizeX = 210;
						pars_pdf.sizeY = 297;

						BNZeroMemory (&pars_pict, sizeof (API_SavePars_Picture));
						pars_pict.colorDepth = APIColorDepth_256C;
						pars_pict.dithered = false;
						pars_pict.view2D = false;
						pars_pict.crop = false;

						err = ACAPI_Automate (APIDo_SaveID, &fsp, &pars_pict);

						delete	fsp.file;
						//////////////////////////////////////////////

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

							// 객체를 수집함
							elemList.Clear ();
							ACAPI_Element_GetElemList (API_ObjectID, &elemList, APIFilt_OnVisLayer);	// 보이는 레이어에 있음, 객체 타입만
							// ... 입면 객체는 제외할 것 (아니면 objectInfo 파일을 참조할 것인가?)

							// 창 이름 출력 !!!
							//WriteReport_Alert ("%s\n%d", GS::UniString(currentDB.name).ToCStr ().Get (), elemList.GetSize ());

							// 층 레벨 숨기기 ON
							// ...

							// 화면 전체에 Fit하도록 포커싱
							ACAPI_Automate (APIDo_ZoomID, NULL, NULL);

							// DWG로 저장하기?
							ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);

							//ACAPI_Environment (APIEnv_GetMiscAppInfoID, &miscAppInfo);
							//sprintf (filename, "테스트.csv", miscAppInfo.caption);
							
							BNZeroMemory (&fsp, sizeof (API_FileSavePars));
							//fsp.fileTypeID = APIFType_PdfFile;
							fsp.fileTypeID = APIFType_PNGFile;
							fsp.file = new IO::Location (location, IO::Name ("test.png"));

							BNZeroMemory (&pars_pdf, sizeof (API_SavePars_Pdf));
							pars_pdf.leftMargin = 0.0;
							pars_pdf.rightMargin = 0.0;
							pars_pdf.topMargin = 0.0;
							pars_pdf.bottomMargin = 0.0;
							pars_pdf.sizeX = 210;
							pars_pdf.sizeY = 297;

							BNZeroMemory (&pars_pict, sizeof (API_SavePars_Picture));
							pars_pict.colorDepth = APIColorDepth_TC24;
							pars_pict.dithered = false;
							pars_pict.view2D = false;
							pars_pict.crop = false;

							err = ACAPI_Automate (APIDo_SaveID, &fsp, &pars_pict);

							delete	fsp.file;


							//ACAPI_Element_GetElemList (API_ObjectID, &elemList);
							//for (GS::Array<API_Guid>::ConstIterator it = elemList.Enumerate (); it != NULL; ++it) {
							//	ACAPI_Database (APIDb_ChangeCurrentDatabaseID, &elevDB, NULL);
							//	BNZeroMemory (&elem, sizeof (API_Element));
							//	elem.header.guid = *it;

							//	err = ACAPI_Element_Get (&elem);
							//	if (err == NoError) {
							//		BNZeroMemory (&windowInfo, sizeof (API_WindowInfo));
							//	}
							//}
						}

						if (dbases != NULL)
							BMpFree (reinterpret_cast<GSPtr>(dbases));

						ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);
						location.ToDisplayText (&resultString);
						WriteReport_Alert ("결과물을 다음 위치에 저장했습니다.\n\n%s\n또는 프로젝트 파일이 있는 폴더", resultString.ToCStr ().Get ());
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
	
	err = ACAPI_Install_MenuHandler (32011, MenuCommandHandler);	// 테이블폼 배치
	err = ACAPI_Install_MenuHandler (32005, MenuCommandHandler);	// 레이어 유틸
	err = ACAPI_Install_MenuHandler (32007, MenuCommandHandler);	// 내보내기
	err = ACAPI_Install_MenuHandler (32009, MenuCommandHandler);	// 반자동 배치
	err = ACAPI_Install_MenuHandler (32013, MenuCommandHandler);	// 편의 기능
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
