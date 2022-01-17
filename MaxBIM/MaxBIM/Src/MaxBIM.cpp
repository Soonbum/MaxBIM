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
			}
			break;

		case 32009:
			// 반자동 배치
			switch (menuParams->menuItemRef.itemIndex) {
				case 1:		// 물량합판 부착하기
					extern qElem qElemInfo;
					if (qElemInfo.dialogID == 0) {
						err = placeQuantityPlywood ();
					} else {
						if ((qElemInfo.dialogID != 0) || DGIsDialogOpen (qElemInfo.dialogID)) {
							DGModelessClose (qElemInfo.dialogID);
							qElemInfo.dialogID = 0;
						}
					}
					return err;

					break;

				case 2:		// 단열재 부착하기
					extern insulElem insulElemInfo;
					if (insulElemInfo.dialogID == 0) {
						err = placeInsulation ();
					} else {
						if ((insulElemInfo.dialogID != 0) || DGIsDialogOpen (insulElemInfo.dialogID)) {
							DGModelessClose (insulElemInfo.dialogID);
							insulElemInfo.dialogID = 0;
						}
					}
					return err;

					break;
			}
			break;

		case 32003:
			// 정보
			switch (menuParams->menuItemRef.itemIndex) {
				case 1:		// 애드온 사용법 보기
					extern short modelessDialogID;
					if (modelessDialogID == 0) {
						err = showHelp ();
					} else {
						if ((modelessDialogID != 0) || DGIsDialogOpen (modelessDialogID)) {
							DGModelessClose (modelessDialogID);
							modelessDialogID = 0;
						}
					}
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
						//
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
