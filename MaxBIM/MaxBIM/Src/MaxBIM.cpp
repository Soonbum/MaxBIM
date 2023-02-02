/**
 * @file Contains the functions required by ArchiCAD.
 */

#include "MaxBIM.hpp"
#include "UtilityFunctions.hpp"

#include "WallTableformPlacer.hpp"
#include "SlabTableformPlacer.hpp"
#include "BeamTableformPlacer.hpp"
#include "ColumnTableformPlacer.hpp"
#include "LowSideTableformPlacer.hpp"

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
					err = ACAPI_CallUndoableCommand (L"���� ���̺��� ��ġ", [&] () -> GSErrCode {
						err = placeTableformOnWall ();
						return err;
					});
					break;
				case 2:
					// ������ �Ϻο� ���̺��� ��ġ�ϱ�
					err = ACAPI_CallUndoableCommand (L"������ �Ϻο� ���̺��� ��ġ", [&] () -> GSErrCode {
						err = placeTableformOnSlabBottom ();
						return err;
					});
					break;
				case 3:
					// ���� ���̺��� ��ġ�ϱ�
					err = ACAPI_CallUndoableCommand (L"���� ���̺��� ��ġ", [&] () -> GSErrCode {
						err = placeTableformOnBeam ();
						return err;
					});
					break;
				case 4:
					// ��տ� ���̺��� ��ġ�ϱ�
					err = ACAPI_CallUndoableCommand (L"��տ� ���̺��� ��ġ", [&] () -> GSErrCode {
						err = placeTableformOnColumn ();
						return err;
					});
					break;
				case 5:
					// ���� ������ ���鿡 ���̺��� ��ġ
					err = ACAPI_CallUndoableCommand (L"���� ������ ���鿡 ���̺��� ��ġ", [&] () -> GSErrCode {
						err = placeTableformOnLowSide ();
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
					err = ACAPI_CallUndoableCommand (L"������ ��ü���� ���̾� �Ӽ� ����", [&] () -> GSErrCode {
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
				case 1:		// ������ ���� ���� �������� (Single ���)
					err = exportSelectedElementInfo ();
					break;
				case 2:		// ������ ���� ���� �������� (Multi ���)
					err = exportElementInfoOnVisibleLayers ();
					break;
				case 3:		// ���纰 ���� �� �����ֱ�
					err = filterSelection ();
					break;
				case 4:		// �� ���̺��� ����ǥ �ۼ�
					err = exportBeamTableformInformation ();
					break;
				case 5:		// ���̺��� ���� ���
					err = calcTableformArea ();
					break;
				case 6:		// ��� �Ը鵵 PDF�� �������� (Single ���)
					err = exportAllElevationsToPDFSingleMode ();
					break;
				case 7:		// ��� �Ը鵵 PDF�� �������� (Multi ���)
					err = exportAllElevationsToPDFMultiMode ();
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
					err = ACAPI_CallUndoableCommand (L"3D ǰ��/�ӵ� �����ϱ�", [&] () -> GSErrCode {
						err = select3DQuality ();
						return err;
					});

					break;

				case 2:		// ������ 3D �� ���̱�
					err = ACAPI_CallUndoableCommand (L"������ 3D �� ���̱�", [&] () -> GSErrCode {
						err = attach3DLabelOnZone ();
						return err;
					});

					break;

				case 3:		// ���� ��鵵�� ���̺����� ���� �ڵ� ��ġ
					err = ACAPI_CallUndoableCommand (L"���� ��鵵�� ���̺����� ���� �ڵ� ��ġ", [&] () -> GSErrCode {
						err = attachBubbleOnCurrentFloorPlan ();
						return err;
					});
					break;

				case 4:		// ī�޶� ��ġ �����ϱ�/�ҷ�����
					err = manageCameraInfo ();

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
					//err = ACAPI_CallUndoableCommand (L"������ �׽�Ʈ", [&] () -> GSErrCode {
						GSErrCode	err = NoError;
						// *** ���ϴ� �ڵ带 �Ʒ� �����ÿ�.
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

						// ������ ��ҵ��� ���� ����ϱ�
						API_ElementQuantity	quantity;
						API_Quantities		quantities;
						API_QuantitiesMask	mask;
						API_QuantityPar		params;
						char				reportStr [512];


						// �׷�ȭ �Ͻ����� ON
						suspendGroups (true);

						// ������ ��� �������� (��, ���, ��, ������, ����, ��ü)
						getGuidsOfSelection (&walls, API_WallID, &nWalls);
						getGuidsOfSelection (&columns, API_ColumnID, &nColumns);
						getGuidsOfSelection (&beams, API_BeamID, &nBeams);
						getGuidsOfSelection (&slabs, API_SlabID, &nSlabs);
						getGuidsOfSelection (&morphs, API_MorphID, &nMorphs);
						getGuidsOfSelection (&objects, API_ObjectID, &nObjects);

						if ( (nWalls == 0) && (nColumns == 0) && (nBeams == 0) && (nSlabs == 0) && (nMorphs == 0) && (nObjects == 0) ) {
							DGAlert (DG_ERROR, L"����", L"��ҵ��� �����ؾ� �մϴ�.", "", L"Ȯ��", "", "");
							return err;
						}

						params.minOpeningSize = EPS;

						// ���� ���� ���� ���� ����
						ACAPI_ELEMENT_QUANTITY_MASK_CLEAR (mask);
						ACAPI_ELEMENT_QUANTITY_MASK_SET (mask, wall, volume);
						for (xx = 0 ; xx < nWalls ; ++xx) {
							quantities.elements = &quantity;
							err = ACAPI_Element_GetQuantities (walls [xx], &params, &quantities, &mask);

							if (err == NoError) {
								volume_walls += quantity.wall.area;
							}
						}

						// ��տ� ���� ���� ���� ����
						ACAPI_ELEMENT_QUANTITY_MASK_CLEAR (mask);
						ACAPI_ELEMENT_QUANTITY_MASK_SET (mask, column, coreVolume);
						ACAPI_ELEMENT_QUANTITY_MASK_SET (mask, column, veneVolume);
						for (xx = 0 ; xx < nColumns ; ++xx) {
							quantities.elements = &quantity;
							err = ACAPI_Element_GetQuantities (columns [xx], &params, &quantities, &mask);

							if (err == NoError) {
								volume_columns += quantity.column.area;
							}
						}

						// ���� ���� ���� ���� ����
						ACAPI_ELEMENT_QUANTITY_MASK_CLEAR (mask);
						ACAPI_ELEMENT_QUANTITY_MASK_SET (mask, beam, volume);
						for (xx = 0 ; xx < nBeams ; ++xx) {
							quantities.elements = &quantity;
							err = ACAPI_Element_GetQuantities (beams [xx], &params, &quantities, &mask);

							if (err == NoError) {
								volume_beams += quantity.beam.bottomSurface;
								volume_beams += quantity.beam.edgeSurfaceLeft;
								volume_beams += quantity.beam.edgeSurfaceRight;
							}
						}

						// �����꿡 ���� ���� ���� ����
						ACAPI_ELEMENT_QUANTITY_MASK_CLEAR (mask);
						ACAPI_ELEMENT_QUANTITY_MASK_SET (mask, slab, volume);
						for (xx = 0 ; xx < nSlabs ; ++xx) {
							quantities.elements = &quantity;
							err = ACAPI_Element_GetQuantities (slabs [xx], &params, &quantities, &mask);

							if (err == NoError) {
								volume_slabs += quantity.slab.bottomSurface;
							}
						}

						// ������ ���� ���� ���� ����
						ACAPI_ELEMENT_QUANTITY_MASK_CLEAR (mask);
						ACAPI_ELEMENT_QUANTITY_MASK_SET (mask, morph, volume);
						for (xx = 0 ; xx < nMorphs ; ++xx) {
							quantities.elements = &quantity;
							err = ACAPI_Element_GetQuantities (morphs [xx], &params, &quantities, &mask);

							if (err == NoError) {
								volume_morphs += quantity.morph.surface;
							}
						}

						// ��ü�� ���� ���� ���� ����
						ACAPI_ELEMENT_QUANTITY_MASK_CLEAR (mask);
						ACAPI_ELEMENT_QUANTITY_MASK_SET (mask, symb, volume);
						for (xx = 0 ; xx < nObjects ; ++xx) {
							quantities.elements = &quantity;
							err = ACAPI_Element_GetQuantities (objects [xx], &params, &quantities, &mask);

							if (err == NoError) {
								volume_objects += quantity.symb.surface;
							}
						}

						// �� ���� ���
						volume_total = volume_walls + volume_columns + volume_beams + volume_slabs + volume_morphs + volume_objects;

						sprintf (reportStr, "%12s: %lf ��\n%12s: %lf ��\n%12s: %lf ��\n%12s: %lf ��\n%12s: %lf ��\n%12s: %lf ��\n\n%12s: %lf ��",
											"�� ����", volume_walls,
											"��� ����", volume_columns,
											"�� ����", volume_beams,
											"������ ����", volume_slabs,
											"���� ����", volume_morphs,
											"��ü ����", volume_objects,
											"�� ����", volume_total);
						WriteReport_Alert (reportStr);

						// �׷�ȭ �Ͻ����� OFF
						suspendGroups (false);

						return	err;
						// *** ���ϴ� �ڵ带 ���� �����ÿ�.
						return err;
					//});

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

	setlocale (LC_ALL, "Korean");	// �ѱ��� ������ ���� (�����ڵ� ���ڿ�)

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

	extern qElem qElemInfo;
	if ((qElemInfo.dialogID != 0) || DGIsDialogOpen (qElemInfo.dialogID)) {
		DGModelessClose (qElemInfo.dialogID);
		qElemInfo.dialogID = 0;
	}

	extern insulElem insulElemInfo;
	if ((insulElemInfo.dialogID != 0) || DGIsDialogOpen (insulElemInfo.dialogID)) {
		DGModelessClose (insulElemInfo.dialogID);
		insulElemInfo.dialogID = 0;
	}

	extern short modelessDialogID;
	if ((modelessDialogID != 0) || DGIsDialogOpen (modelessDialogID)) {
		DGModelessClose (modelessDialogID);
		modelessDialogID = 0;
	}

	return NoError;
}		// FreeData ()
