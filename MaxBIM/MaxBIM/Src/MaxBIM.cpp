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
#include "BeamTableformPlacer.hpp"

#include "SupportingPostPlacer.hpp"

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
	
	err = ACAPI_Register_Menu (32001, 32002, MenuCode_UserDef, MenuFlag_Default);	// ������ ��ġ
	err = ACAPI_Register_Menu (32011, 32012, MenuCode_UserDef, MenuFlag_Default);	// ���̺��� ��ġ
	err = ACAPI_Register_Menu (32015, 32016, MenuCode_UserDef, MenuFlag_Default);	// ���ٸ� ��ġ
	err = ACAPI_Register_Menu (32013, 32014, MenuCode_UserDef, MenuFlag_Default);	// Library Converting
	err = ACAPI_Register_Menu (32005, 32006, MenuCode_UserDef, MenuFlag_Default);	// ���̾� ��ƿ
	err = ACAPI_Register_Menu (32007, 32008, MenuCode_UserDef, MenuFlag_Default);	// ��������
	err = ACAPI_Register_Menu (32009, 32010, MenuCode_UserDef, MenuFlag_Default);	// ���� ����
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

	switch (menuParams->menuItemRef.menuResID) {

		case 32001:
			// ������ ��ġ
			switch (menuParams->menuItemRef.itemIndex) {
				case 1:
					// ���� ������ ��ġ�ϱ�
					err = ACAPI_CallUndoableCommand ("���� ������ ��ġ", [&] () -> GSErrCode {
						err = placeEuroformOnWall ();
						return err;
					});
					break;
				case 2:
					// ������ �Ϻο� ������ ��ġ�ϱ�
					err = ACAPI_CallUndoableCommand ("������ �Ϻο� ������ ��ġ", [&] () -> GSErrCode {
						err = placeEuroformOnSlabBottom ();
						return err;
					});
					break;
				case 3:
					// ���� ������ ��ġ�ϱ�
					err = ACAPI_CallUndoableCommand ("���� ������ ��ġ", [&] () -> GSErrCode {
						err = placeEuroformOnBeam ();
						return err;
					});
					break;
				case 4:
					// ��տ� ������ ��ġ�ϱ�
					err = ACAPI_CallUndoableCommand ("��տ� ������ ��ġ", [&] () -> GSErrCode {
						err = placeEuroformOnColumn ();
						return err;
					});
					break;
			}
			break;

		case 32011:
			// ���̺��� ��ġ
			switch (menuParams->menuItemRef.itemIndex) {
				case 1:
					// ���� ���̺��� ��ġ�ϱ� - ���� ����
					err = ACAPI_CallUndoableCommand ("���� ���̺��� ��ġ - ���� ����", [&] () -> GSErrCode {
						err = placeTableformOnWall_Vertical ();
						return err;
					});
					break;
				case 2:
					// ���� ���̺��� ��ġ�ϱ� - ���� ����
					err = ACAPI_CallUndoableCommand ("���� ���̺��� ��ġ - ���� ����", [&] () -> GSErrCode {
						err = placeTableformOnWall_Horizontal ();
						return err;
					});
					break;
				case 3:
					// ���� ���̺��� ��ġ�ϱ� - Ŀ����
					err = ACAPI_CallUndoableCommand ("���� ���̺��� ��ġ - Ŀ����", [&] () -> GSErrCode {
						err = placeTableformOnWall_Custom ();
						return err;
					});
					break;
				case 4:
					// ������ �Ϻο� ���̺��� ��ġ�ϱ�
					err = ACAPI_CallUndoableCommand ("������ �Ϻο� ���̺��� ��ġ", [&] () -> GSErrCode {
						err = placeTableformOnSlabBottom ();
						return err;
					});
					break;
				case 5:
					// ���� ���̺��� ��ġ�ϱ�
					err = ACAPI_CallUndoableCommand ("���� ���̺��� ��ġ", [&] () -> GSErrCode {
						err = placeTableformOnBeam ();
						return err;
					});
					break;
			}
			break;

		case 32015:
			// ���ٸ� ��ġ
			switch (menuParams->menuItemRef.itemIndex) {
			case 1:
				// PERI ���ٸ� �ڵ� ��ġ
				err = ACAPI_CallUndoableCommand ("PERI ���ٸ� �ڵ� ��ġ", [&] () -> GSErrCode {
					err = placePERIPost ();
					return err;
				});
				break;
			}
			break;

		case 32013:
			// Library Converting
			switch (menuParams->menuItemRef.itemIndex) {
				case 1:
					// ���� ������ ��� ��ȯ
					err = ACAPI_CallUndoableCommand ("���� ������ ��� ��ȯ", [&] () -> GSErrCode {
						err = convertVirtualTCO ();		// TCO: Temporary Construction Object
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
			}
			break;

		case 32009:
			// ���ڵ� ��ġ
			switch (menuParams->menuItemRef.itemIndex) {
				case 1:		// �������� �����ϱ�
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

				case 2:		// �ܿ��� �����ϱ�
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
			// ����
			switch (menuParams->menuItemRef.itemIndex) {
				case 1:		// �ֵ�� ���� ����
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
						API_SelectionInfo		selectionInfo;
						API_Neig				**selNeigs;
						API_Element				tElem;
						long					nSel;
						GS::Array<API_Guid>		objects;

						// ������ ��� ��������
						err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, true);
						BMKillHandle ((GSHandle *) &selectionInfo.marquee.coords);
						if (err != NoError) {
							BMKillHandle ((GSHandle *) &selNeigs);
							ACAPI_WriteReport ("��ҵ��� �����ؾ� �մϴ�.", true);
							return err;
						}

						// ��ü Ÿ���� ��� GUID�� ������
						if (selectionInfo.typeID != API_SelEmpty) {
							nSel = BMGetHandleSize ((GSHandle) selNeigs) / sizeof (API_Neig);
							for (short xx = 0 ; xx < nSel && err == NoError ; ++xx) {
								tElem.header.typeID = Neig_To_ElemID ((*selNeigs)[xx].neigID);
								tElem.header.guid = (*selNeigs)[xx].guid;

								if (ACAPI_Element_Get (&tElem) != NoError)	// ������ �� �ִ� ����ΰ�?
									continue;

								if (tElem.header.typeID == API_ObjectID)	// ��ü Ÿ�� ����ΰ�?
									objects.Push (tElem.header.guid);
							}
						}
						BMKillHandle ((GSHandle *) &selNeigs);

						API_Element			elem;
						API_ElementMemo		memo;
						const char*			outStr;
						API_AddParID		type;
						double				value;

						BNZeroMemory (&elem, sizeof (API_Element));
						BNZeroMemory (&memo, sizeof (API_ElementMemo));
						elem.header.guid = objects.Pop ();
						err = ACAPI_Element_Get (&elem);

						if (err == NoError && elem.header.hasMemo) {
							err = ACAPI_Element_GetMemo (elem.header.guid, &memo);
							if (err == NoError) {
								type = getParameterTypeByName (&memo, "A");		// Ÿ��
								WriteReport_Alert ("Ÿ��: %d", type);

								outStr = getParameterStringByName (&memo, "A");	// ���ڿ�
								WriteReport_Alert ("�̸�: %s(%d)", outStr, strlen (outStr));

								value = getParameterValueByName (&memo, "A");	// ��
								WriteReport_Alert ("��: %.3f", value);
							}
						}

						ACAPI_DisposeElemMemoHdls (&memo);

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
	
	err = ACAPI_Install_MenuHandler (32001, MenuCommandHandler);	// ������ ��ġ
	err = ACAPI_Install_MenuHandler (32011, MenuCommandHandler);	// ���̺��� ��ġ
	err = ACAPI_Install_MenuHandler (32015, MenuCommandHandler);	// ���ٸ� ��ġ
	err = ACAPI_Install_MenuHandler (32013, MenuCommandHandler);	// Library Converting
	err = ACAPI_Install_MenuHandler (32005, MenuCommandHandler);	// ���̾� ��ƿ
	err = ACAPI_Install_MenuHandler (32007, MenuCommandHandler);	// ��������
	err = ACAPI_Install_MenuHandler (32009, MenuCommandHandler);	// ���� ����
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
