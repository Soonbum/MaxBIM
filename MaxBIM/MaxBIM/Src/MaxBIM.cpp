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
	
	err = ACAPI_Register_Menu (32011, 32012, MenuCode_UserDef, MenuFlag_Default);	// ���̺��� ��ġ
	err = ACAPI_Register_Menu (32005, 32006, MenuCode_UserDef, MenuFlag_Default);	// ���̾� ��ƿ
	err = ACAPI_Register_Menu (32007, 32008, MenuCode_UserDef, MenuFlag_Default);	// ��������
	err = ACAPI_Register_Menu (32009, 32010, MenuCode_UserDef, MenuFlag_Default);	// ���ڵ� ��ġ
	err = ACAPI_Register_Menu (32013, 32014, MenuCode_UserDef, MenuFlag_Default);	// ���� ���
	err = ACAPI_Register_Menu (32003, 32004, MenuCode_UserDef, MenuFlag_Default);	// ����

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
			// ���̺��� ��ġ
			switch (menuParams->menuItemRef.itemIndex) {
				case 1:
					// ���� ���̺��� ��ġ�ϱ�
					err = ACAPI_CallUndoableCommand ("���� ���̺��� ��ġ", [&] () -> GSErrCode {
						err = placeTableformOnWall ();
						return err;
					});
					break;
				case 2:
					// ������ �Ϻο� ���̺��� ��ġ�ϱ�
					err = ACAPI_CallUndoableCommand ("������ �Ϻο� ���̺��� ��ġ", [&] () -> GSErrCode {
						err = placeTableformOnSlabBottom ();
						return err;
					});
					break;
				case 3:
					// ���� ���̺��� ��ġ�ϱ�
					err = ACAPI_CallUndoableCommand ("���� ���̺��� ��ġ", [&] () -> GSErrCode {
						err = placeTableformOnBeam ();
						return err;
					});
					break;
				case 4:
					// ��տ� ���̺��� ��ġ�ϱ�
					err = ACAPI_CallUndoableCommand ("��տ� ���̺��� ��ġ", [&] () -> GSErrCode {
						err = placeTableformOnColumn ();
						return err;
					});
					break;
			}
			break;

		case 32005:
			// ���̾� ��ƿ
			switch (menuParams->menuItemRef.itemIndex) {
				case 1:		// ���̾� ���� �����ϱ�
					err = showLayersEasily ();
					break;
				case 2:		// ���̾� ���� �����
					err = makeLayersEasily ();
					break;
				case 3:		// ���̾� ���� �����ϱ�
					err = ACAPI_CallUndoableCommand ("������ ��ü���� ���̾� �Ӽ� ����", [&] () -> GSErrCode {
						err = assignLayerEasily ();
						return err;
					});
					break;
				case 4:		// ���̾� �̸� �˻��ϱ�
					err = inspectLayerNames ();
					break;
			}
			break;

		case 32007:
			// ��������
			switch (menuParams->menuItemRef.itemIndex) {
				case 1:		// ����(���,��,������) ���� �������� (CSV) ... ���ߺ���
					err = exportGridElementInfo ();
					break;
				case 2:		// ������ ���� ���� �������� (Single ���)
					err = exportSelectedElementInfo ();
					break;
				case 3:		// ������ ���� ���� �������� (Multi ���)
					err = exportElementInfoOnVisibleLayers ();
					break;
				case 4:		// ���纰 ���� �� �����ֱ�
					err = filterSelection ();
					break;
				case 5:		// �� ���̺��� ����ǥ �ۼ�
					err = exportBeamTableformInformation ();
					break;
				case 6:		// ���̺��� ���� ���
					err = calcTableformArea ();
					break;
				case 7:		// ��ũ��Ʈ ���� ��� (Single ���)
					err = calcConcreteVolumeSingleMode ();
					break;
				case 8:		// ��ũ��Ʈ ���� ��� (Multi ���)
					err = calcConcreteVolumeMultiMode ();
					break;
			}
			break;

		case 32009:
			// ���ڵ� ��ġ
			switch (menuParams->menuItemRef.itemIndex) {
				case 1:		// �������� �����ϱ�
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

				case 2:		// �ܿ��� �����ϱ�
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
			// ���� ���
			switch (menuParams->menuItemRef.itemIndex) {
				case 1:		// 3D ǰ��/�ӵ� �����ϱ�
					err = ACAPI_CallUndoableCommand ("3D ǰ��/�ӵ� �����ϱ�", [&] () -> GSErrCode {
						err = select3DQuality ();
						return err;
					});

					break;

				case 2:		// ������ 3D �� ���̱�
					err = ACAPI_CallUndoableCommand ("������ 3D �� ���̱�", [&] () -> GSErrCode {
						err = attach3DLabelOnZone ();
						return err;
					});

					break;
			}
			break;

		case 32003:
			// ����
			switch (menuParams->menuItemRef.itemIndex) {
				case 1:		// �ֵ�� ���� ����
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
				case 2:		// MaxBIM �ֵ�� ����
					err = showAbout ();

					// *** �ؽ�Ʈ â �ݱ� ��ƾ
					//API_WindowInfo		windowInfo;

					//BNZeroMemory (&windowInfo, sizeof (API_WindowInfo));
					//windowInfo.typeID = APIWind_MyTextID;
					//windowInfo.index  = 1;
					//err = ACAPI_Database (APIDb_CloseWindowID, &windowInfo, NULL);

					break;
				case 3:		// ������ ���� - ������ �׽�Ʈ �޴�
					err = ACAPI_CallUndoableCommand ("������ �׽�Ʈ", [&] () -> GSErrCode {
						GSErrCode	err = NoError;
						// *** ���ϴ� �ڵ带 �Ʒ� �����ÿ�.
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
						//sprintf (filename, "�׽�Ʈ.csv", miscAppInfo.caption);
							
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

						// �Ը� �� DB�� ID���� ȹ����
						err = ACAPI_Database (APIDb_GetElevationDatabasesID, &dbases, NULL);
						if (err == NoError)
							nDbases = BMpGetSize (reinterpret_cast<GSPtr>(dbases)) / Sizeof32 (API_DatabaseUnId);

						// �Ը� ����� �ϳ��� ��ȸ��
						for (GSIndex i = 0; i < nDbases; i++) {
							API_DatabaseInfo dbPars;
							BNZeroMemory (&dbPars, sizeof (API_DatabaseInfo));
							dbPars.databaseUnId = dbases [i];

							// â�� ������
							BNZeroMemory (&windowInfo, sizeof (API_WindowInfo));
							windowInfo.typeID = APIWind_ElevationID;
							windowInfo.databaseUnId = dbPars.databaseUnId;
							ACAPI_Automate (APIDo_ChangeWindowID, &windowInfo, NULL);

							// ���� �����ͺ��̽��� ������
							ACAPI_Database (APIDb_GetCurrentDatabaseID, &currentDB, NULL);

							// ��ü�� ������
							elemList.Clear ();
							ACAPI_Element_GetElemList (API_ObjectID, &elemList, APIFilt_OnVisLayer);	// ���̴� ���̾ ����, ��ü Ÿ�Ը�
							// ... �Ը� ��ü�� ������ �� (�ƴϸ� objectInfo ������ ������ ���ΰ�?)

							// â �̸� ��� !!!
							//WriteReport_Alert ("%s\n%d", GS::UniString(currentDB.name).ToCStr ().Get (), elemList.GetSize ());

							// �� ���� ����� ON
							// ...

							// ȭ�� ��ü�� Fit�ϵ��� ��Ŀ��
							ACAPI_Automate (APIDo_ZoomID, NULL, NULL);

							// DWG�� �����ϱ�?
							ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);

							//ACAPI_Environment (APIEnv_GetMiscAppInfoID, &miscAppInfo);
							//sprintf (filename, "�׽�Ʈ.csv", miscAppInfo.caption);
							
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
						WriteReport_Alert ("������� ���� ��ġ�� �����߽��ϴ�.\n\n%s\n�Ǵ� ������Ʈ ������ �ִ� ����", resultString.ToCStr ().Get ());
						// *** ���ϴ� �ڵ带 ���� �����ÿ�.
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
	
	err = ACAPI_Install_MenuHandler (32011, MenuCommandHandler);	// ���̺��� ��ġ
	err = ACAPI_Install_MenuHandler (32005, MenuCommandHandler);	// ���̾� ��ƿ
	err = ACAPI_Install_MenuHandler (32007, MenuCommandHandler);	// ��������
	err = ACAPI_Install_MenuHandler (32009, MenuCommandHandler);	// ���ڵ� ��ġ
	err = ACAPI_Install_MenuHandler (32013, MenuCommandHandler);	// ���� ���
	err = ACAPI_Install_MenuHandler (32003, MenuCommandHandler);	// ����

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
