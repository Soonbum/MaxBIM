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
	
	err = ACAPI_Register_Menu (32001, 32002, MenuCode_UserDef, MenuFlag_Default);	// ������ ��ġ
	err = ACAPI_Register_Menu (32011, 32012, MenuCode_UserDef, MenuFlag_Default);	// ���̺��� ��ġ
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
					// ������ �Ϻο� ���̺��� ��ġ�ϱ�
					err = ACAPI_CallUndoableCommand ("������ �Ϻο� ���̺��� ��ġ", [&] () -> GSErrCode {
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
			// ���� ����
			switch (menuParams->menuItemRef.itemIndex) {
				case 1:		// �������� �����ϱ�
					err = ACAPI_CallUndoableCommand ("�������� �����ϱ�", [&] () -> GSErrCode {
						extern qElem qElemInfo;
						if (qElemInfo.dialogID == 0) {
							err = placeQuantityPlywood ();
							// �޴� �ؽ�Ʈ ����: �������� �����ϱ� �ȷ�Ʈ â �ݱ�
						} else {
							if ((qElemInfo.dialogID != 0) || DGIsDialogOpen (qElemInfo.dialogID)) {
								DGModelessClose (qElemInfo.dialogID);
								qElemInfo.dialogID = 0;
								// �޴� �ؽ�Ʈ ����: �������� �����ϱ�
							}
							return NoError;
						}
						return err;
					});
					break;
			}
			break;
		case 32003:

			// ����
			switch (menuParams->menuItemRef.itemIndex) {
				case 1:		// �ֵ�� ���� ����
					err = showHelp ();
					break;
				case 2:		// MaxBIM �ֵ�� ����
					err = showAbout ();

					API_WindowInfo		windowInfo;

					BNZeroMemory (&windowInfo, sizeof (API_WindowInfo));
					windowInfo.typeID = APIWind_MyTextID;
					windowInfo.index  = 1;
					err = ACAPI_Database (APIDb_CloseWindowID, &windowInfo, NULL);

					break;
				case 3:		// ������ ���� - ������ �׽�Ʈ �޴�
					err = ACAPI_CallUndoableCommand ("������ �׽�Ʈ", [&] () -> GSErrCode {
						GSErrCode	err = NoError;
						// *** ���ϴ� �ڵ带 �Ʒ� �����ÿ�.

						//API_PetPaletteType  petPaletteInfo;
						//short**             petItemIdsHdl;
						//short               petItemIds [3] = { 32522, 32523, 32524 };

						//BNZeroMemory (&petPaletteInfo, sizeof (API_PetPaletteType));

						//// �� �׸� �����ܵ��� ���ҽ� ID���� �����ϴ� �ڵ� �����ϱ�
						//short nIcons = sizeof (petItemIds) / sizeof (short);
						//petItemIdsHdl = (short**) BMhAll (nIcons * sizeof (short));
						//for (short i = 0; i < nIcons; i++)
						//	(*petItemIdsHdl)[i] = petItemIds[i];

						//// petPaletteInfo ä���
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

						const GS::uchar_t*	gsmName = L("��������(�ֽ������).gsm");
						double				aParam;
						double				bParam;
						Int32				addParNum;

						char				tempStr [256];
						double				horLen, verLen;

						// ���� �Է�
						API_GetPointType	pointInfo;
						API_GetPolyType		polyInfo;
						double				dx, dy, dz;

						BNZeroMemory (&pointInfo, sizeof (API_GetPointType));
						BNZeroMemory (&polyInfo, sizeof (API_GetPolyType));

						CHCopyC ("�ٰ����� 1��° ��带 Ŭ���Ͻʽÿ�.", pointInfo.prompt);
						err = ACAPI_Interface (APIIo_GetPointID, &pointInfo, NULL);

						CHCopyC ("�ٰ��� ��带 �Է��մϴ�.", polyInfo.prompt);
						polyInfo.startCoord = pointInfo.pos;
						polyInfo.method = APIPolyGetMethod_Polyline;
						polyInfo.getZCoords = true;
						err = ACAPI_Interface (APIIo_GetPolyID, &polyInfo, NULL);

						// �� ������ ������ ������ ��
						if (polyInfo.nCoords < 3)
							return err;

						// ��ü �ε�
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

						// ���̺귯���� �Ķ���� �� �Է� (����)
						elem.header.floorInd = 0;
						elem.object.libInd = libPart.index;
						elem.object.xRatio = aParam;
						elem.object.yRatio = bParam;
						elem.header.layer = 1;

						strcpy (tempStr, "��ü(�պ�)");
						setParameterByName (&memo, "PANEL_MAT", 72.0);
						setParameterByName (&memo, "m_type", tempStr);

						// ���ǵβ� (���ڿ�)
						strcpy (tempStr, "12");		// 12mm
						setParameterByName (&memo, "m_size", tempStr);

						// ��ġ��ġ
						strcpy (tempStr, "��԰�");
						setParameterByName (&memo, "m_size1", tempStr);

						// ǰ��
						strcpy (tempStr, "���Ǹ���");
						setParameterByName (&memo, "m_size2", tempStr);

						//for (Int32 ind = 1; ind <= polyInfo.nCoords; ind++)
						//	sprintf (tempStr, "[%2d]  (%Lf, %Lf)\n", ind, (*polyInfo.coords) [ind].x, (*polyInfo.coords) [ind].y);

						dx = (*polyInfo.coords) [2].x - (*polyInfo.coords) [1].x;
						dy = (*polyInfo.coords) [2].y - (*polyInfo.coords) [1].y;

						elem.object.angle = atan2 (dy, dx);
						elem.object.pos.x = (*polyInfo.coords) [1].x;
						elem.object.pos.y = (*polyInfo.coords) [1].y;
						elem.object.level = (*polyInfo.zCoords) [1];

						// ���α���
						horLen = GetDistance ((*polyInfo.coords) [1].x, (*polyInfo.coords) [1].y, (*polyInfo.zCoords) [1], (*polyInfo.coords) [2].x, (*polyInfo.coords) [2].y, (*polyInfo.zCoords) [2]);
						setParameterByName (&memo, "NO_WD", horLen);
						elem.object.xRatio = horLen;

						// ���α���
						verLen = GetDistance ((*polyInfo.coords) [2].x, (*polyInfo.coords) [2].y, (*polyInfo.zCoords) [2], (*polyInfo.coords) [3].x, (*polyInfo.coords) [3].y, (*polyInfo.zCoords) [3]);
						setParameterByName (&memo, "no_lg1", verLen);
						elem.object.yRatio = verLen;

						// ��ġ����
						strcpy (tempStr, "���� �����");	// �⺻��
						if ((abs ((*polyInfo.zCoords) [1] - (*polyInfo.zCoords) [2]) < EPS) && (abs ((*polyInfo.zCoords) [2] - (*polyInfo.zCoords) [3]) < EPS)) {
							double angle1, angle2;

							dx = (*polyInfo.coords) [2].x - (*polyInfo.coords) [1].x;
							dy = (*polyInfo.coords) [2].y - (*polyInfo.coords) [1].y;
							angle1 = RadToDegree (atan2 (dy, dx));

							dx = (*polyInfo.coords) [3].x - (*polyInfo.coords) [2].x;
							dy = (*polyInfo.coords) [3].y - (*polyInfo.coords) [2].y;
							angle2 = RadToDegree (atan2 (dy, dx));

							// p2-p3 ���� ������ p2-p1 ���� �������� 90�� ŭ
							if (abs (angle2 - angle1 - 90) < EPS) {
								strcpy (tempStr, "�ٴڵ���");
							}

							// p2-p3 ���� ������ p2-p1 ���� �������� 90�� ����
							if (abs (angle1 - angle2 - 90) < EPS) {
								strcpy (tempStr, "�ٴڱ��");
								moveIn3D ('y', elem.object.angle, -verLen, &elem.object.pos.x, &elem.object.pos.y, &elem.object.level);
							}
						} else {
							if ((abs ((*polyInfo.coords) [2].x - (*polyInfo.coords) [3].x) < EPS) && (abs ((*polyInfo.coords) [2].y - (*polyInfo.coords) [3].y) < EPS)) {
								// p2�� p3�� x, y ��ǥ�� ����
								strcpy (tempStr, "���� �����");
							} else {
								// p2�� p3�� x, y ��ǥ�� �ٸ�
								strcpy (tempStr, "��缳ġ");

								// ��ġ����: asin ((p3.z - p2.z) / (p3�� p2 ���� �Ÿ�))
								// ��ġ����: acos ((p3�� p2 ���� ��� ���� �Ÿ�) / (p3�� p2 ���� �Ÿ�))
								// ��ġ����: atan2 ((p3.z - p2.z) / (p3�� p2 ���� ��� ���� �Ÿ�))
								dx = GetDistance ((*polyInfo.coords) [2].x, (*polyInfo.coords) [2].y, (*polyInfo.coords) [3].x, (*polyInfo.coords) [3].y);
								dy = abs ((*polyInfo.zCoords) [3] - (*polyInfo.zCoords) [2]);
								dz = verLen;
								setParameterByName (&memo, "cons_ang", DegreeToRad (180.0) - acos (dx/dz));
								setParameterByName (&memo, "vertical_cut", 1.0);	// ������� �����
							}
						}
						setParameterByName (&memo, "CONS_DR", tempStr);

						// ��ü ��ġ
						if ((horLen > EPS) && (verLen > EPS))
							ACAPI_Element_Create (&elem, &memo);

						ACAPI_DisposeElemMemoHdls (&memo);
						*/

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
