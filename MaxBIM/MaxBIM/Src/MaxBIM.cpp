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
				case 2:		// ������ ���� ���� ��������
					err = exportSelectedElementInfo ();
					break;
			}
			break;
		case 32009:
			// ���� ����
			switch (menuParams->menuItemRef.itemIndex) {
				case 1:		// �������� �����ϱ�
					err = ACAPI_CallUndoableCommand ("�������� �����ϱ�", [&] () -> GSErrCode {
						err = placeQuantityPlywood ();
						return err;
					});
					break;
				case 2:		// �������� ���� ����ϱ�
					err = calcAreasOfQuantityPlywood ();
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
					break;
				case 3:		// ������ ���� - ������ �׽�Ʈ �޴�
					err = ACAPI_CallUndoableCommand ("������ �׽�Ʈ", [&] () -> GSErrCode {
						GSErrCode	err = NoError;
						short		xx, yy;

						// Selection Manager ���� ����
						long		nSel;
						API_SelectionInfo		selectionInfo;
						API_Element				tElem;
						API_Neig				**selNeigs;
						GS::Array<API_Guid>&	morphs = GS::Array<API_Guid> ();
						GS::Array<API_Guid>&	polylines = GS::Array<API_Guid> ();
						long					nMorphs = 0;
						long					nPolylines = 0;

						// ��ü ���� ��������
						API_Element				elem;
						API_ElementMemo			memo;
						API_ElemInfo3D			info3D;

						// ���� 3D ������� ��������
						API_Component3D			component;
						API_Tranmat				tm;
						Int32					nVert, nEdge, nPgon;
						Int32					elemIdx, bodyIdx;
						API_Coord3D				trCoord;
						GS::Array<API_Coord3D>&	coords = GS::Array<API_Coord3D> ();

						// �������� ���ܵǴ� �� 4��
						API_Coord3D		excludeP1;
						API_Coord3D		excludeP2;
						API_Coord3D		excludeP3;
						API_Coord3D		excludeP4;

						excludeP1.x = 0;	excludeP1.y = 0;	excludeP1.z = 0;
						excludeP2.x = 1;	excludeP2.y = 0;	excludeP2.z = 0;
						excludeP3.x = 0;	excludeP3.y = 1;	excludeP3.z = 0;
						excludeP4.x = 0;	excludeP4.y = 0;	excludeP4.z = 1;

						// ��Ÿ
						char	buffer [256];


						// ������ ��� ��������
						err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, true);
						BMKillHandle ((GSHandle *) &selectionInfo.marquee.coords);
						if (err != NoError) {
							BMKillHandle ((GSHandle *) &selNeigs);
							return err;
						}

						if (selectionInfo.typeID != API_SelEmpty) {
							nSel = BMGetHandleSize ((GSHandle) selNeigs) / sizeof (API_Neig);
							for (xx = 0 ; xx < nSel && err == NoError ; ++xx) {
								tElem.header.typeID = Neig_To_ElemID ((*selNeigs)[xx].neigID);

								tElem.header.guid = (*selNeigs)[xx].guid;
								if (ACAPI_Element_Get (&tElem) != NoError)	// ������ �� �ִ� ����ΰ�?
									continue;

								if (tElem.header.typeID == API_MorphID)		// �����ΰ�?
									morphs.Push (tElem.header.guid);

								if (tElem.header.typeID == API_PolyLineID)	// ���������ΰ�?
									polylines.Push (tElem.header.guid);
							}
						}
						BMKillHandle ((GSHandle *) &selNeigs);
						nMorphs = morphs.GetSize ();
						nPolylines = polylines.GetSize ();

						if (nMorphs > 0) {
							for (xx = 0 ; xx < nMorphs ; ++xx) {
								// ������ ������ ������
								BNZeroMemory (&elem, sizeof (API_Element));
								elem.header.guid = morphs.Pop ();
								err = ACAPI_Element_Get (&elem);
								err = ACAPI_Element_Get3DInfo (elem.header, &info3D);

								// ������ �� ��ǥ���� ������
								BNZeroMemory (&component, sizeof (API_Component3D));
								component.header.typeID = API_BodyID;
								component.header.index = info3D.fbody;
								err = ACAPI_3D_GetComponent (&component);

								nVert = component.body.nVert;
								nEdge = component.body.nEdge;
								nPgon = component.body.nPgon;
								tm = component.body.tranmat;
								elemIdx = component.body.head.elemIndex - 1;
								bodyIdx = component.body.head.bodyIndex - 1;
	
								// ���� ��ǥ�� ������
								for (yy = 1 ; yy <= nVert ; ++yy) {
									component.header.typeID	= API_VertID;
									component.header.index	= yy;
									err = ACAPI_3D_GetComponent (&component);
									if (err == NoError) {
										trCoord.x = tm.tmx[0]*component.vert.x + tm.tmx[1]*component.vert.y + tm.tmx[2]*component.vert.z + tm.tmx[3];
										trCoord.y = tm.tmx[4]*component.vert.x + tm.tmx[5]*component.vert.y + tm.tmx[6]*component.vert.z + tm.tmx[7];
										trCoord.z = tm.tmx[8]*component.vert.x + tm.tmx[9]*component.vert.y + tm.tmx[10]*component.vert.z + tm.tmx[11];
										coords.Push (trCoord);

										sprintf (buffer, "%d ", yy);

										if ( !(isSamePoint (excludeP1, trCoord) || isSamePoint (excludeP2, trCoord) || isSamePoint (excludeP3, trCoord) || isSamePoint (excludeP4, trCoord)) ) {
											placeCoordinateLabel (trCoord.x, trCoord.y, trCoord.z, true, buffer);
										}
									}
								}
							}
						}

						if (nPolylines > 0) {
							for (xx = 0 ; xx < nPolylines ; ++xx) {
								// ���������� ������ ������
								BNZeroMemory (&elem, sizeof (API_Element));
								BNZeroMemory (&memo, sizeof (API_ElementMemo));
								elem.header.guid = polylines.Pop ();
								err = ACAPI_Element_Get (&elem);
								err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

								// ���� ��ǥ�� ������
								for (yy = 1 ; yy <= elem.polyLine.poly.nCoords ; ++yy) {
									sprintf (buffer, "%d ", yy);
									err = placeCoordinateLabel (memo.coords [0][yy].x, memo.coords [0][yy].y, 0, true, buffer);
								}
							}
						}

						// ���� ���� �� ���� ��������
						// ...

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
