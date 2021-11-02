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
#include "ColumnTableformPlacer.hpp"

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
				case 6:
					// ��տ� ���̺��� ��ġ�ϱ�
					err = ACAPI_CallUndoableCommand ("��տ� ���̺��� ��ġ", [&] () -> GSErrCode {
						err = placeTableformOnColumn ();
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
						unsigned short		xx, yy, zz;
						short		mm;
						bool		regenerate = true;
						bool		suspGrp;

						// ��� ��ü���� ���� ��ǥ�� ���� ������
						vector<API_Coord3D>	vecPos;

						// ��� ��ü, �� ����
						GS::Array<API_Guid>		elemList;
						GS::Array<API_Guid>		objects;
						long					nObjects = 0;

						// ������ ��ҵ��� ���� ����ϱ�
						API_Element			elem;
						API_ElementMemo		memo;

						char			tempStr [512];
						const char*		foundStr;
						bool			foundObject;
						bool			foundExistValue;
						int				retVal;
						int				nInfo;

						// ���̾� ���� ����
						short			nLayers;
						API_Attribute	attrib;
						short			nVisibleLayers = 0;
						short			visLayerList [1024];
						char			fullLayerName [512];

						// ���̾� Ÿ�Կ� ���� ĸ�� ���� ����
						char*			foundLayerName;

						// ��Ÿ
						char			buffer [512];
						char			filename [512];

						// �۾� �� ����
						API_StoryInfo	storyInfo;
						double			workLevel_object;		// ���� �۾� �� ����


						// ����ٸ� ǥ���ϱ� ���� ����
						GS::UniString       title ("�������� ���� ��Ȳ");
						GS::UniString       subtitle ("������...");
						short	nPhase;
						Int32	cur, total;

						// ���� ���Ϸ� ��� ���� ��������
						// ���� ������ ���� ����
						API_SpecFolderID	specFolderID = API_ApplicationFolderID;
						IO::Location		location;
						GS::UniString		resultString;
						API_MiscAppInfo		miscAppInfo;


						// �׷�ȭ �Ͻ����� ON
						ACAPI_Environment (APIEnv_IsSuspendGroupOnID, &suspGrp);
						if (suspGrp == false)	ACAPI_Element_Tool (NULL, NULL, APITool_SuspendGroups, NULL);

						// ȭ�� ���ΰ�ħ
						ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
						ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

						//ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);

						// ������Ʈ �� ���̾� ������ �˾Ƴ�
						BNZeroMemory (&attrib, sizeof (API_Attribute));
						attrib.layer.head.typeID = API_LayerID;
						err = ACAPI_Attribute_GetNum (API_LayerID, &nLayers);

						// ���̴� ���̾���� ��� �����ϱ�
						for (xx = 1 ; xx <= nLayers ; ++xx) {
							BNZeroMemory (&attrib, sizeof (API_Attribute));
							attrib.layer.head.typeID = API_LayerID;
							attrib.layer.head.index = xx;
							err = ACAPI_Attribute_Get (&attrib);
							if (err == NoError) {
								if (!((attrib.layer.head.flags & APILay_Hidden) == true)) {
									visLayerList [nVisibleLayers++] = attrib.layer.head.index;
								}
							}
						}

						// �Ͻ������� ��� ���̾� �����
						for (xx = 1 ; xx <= nLayers ; ++xx) {
							BNZeroMemory (&attrib, sizeof (API_Attribute));
							attrib.layer.head.typeID = API_LayerID;
							attrib.layer.head.index = xx;
							err = ACAPI_Attribute_Get (&attrib);
							if (err == NoError) {
								attrib.layer.head.flags |= APILay_Hidden;
								ACAPI_Attribute_Modify (&attrib, NULL);
							}
						}

						ACAPI_Environment (APIEnv_GetMiscAppInfoID, &miscAppInfo);

						// ���� ��Ȳ ǥ���ϴ� ��� - �ʱ�ȭ
						nPhase = 1;
						cur = 1;
						total = nVisibleLayers;
						ACAPI_Interface (APIIo_InitProcessWindowID, &title, &nPhase);
						ACAPI_Interface (APIIo_SetNextProcessPhaseID, &subtitle, &total);

						// ���̴� ���̾���� �ϳ��� ��ȸ�ϸ鼭 ��ü ��ҵ��� ������ �� "������ ���� ���� ��������" ��ƾ ����
						for (mm = 1 ; mm <= nVisibleLayers ; ++mm) {
							BNZeroMemory (&attrib, sizeof (API_Attribute));
							attrib.layer.head.typeID = API_LayerID;
							attrib.layer.head.index = visLayerList [mm-1];
							err = ACAPI_Attribute_Get (&attrib);

							// �ʱ�ȭ
							objects.Clear ();
							vecPos.clear ();

							if (err == NoError) {
								// ���̾� ���̱�
								if ((attrib.layer.head.flags & APILay_Hidden) == true) {
									attrib.layer.head.flags ^= APILay_Hidden;
									ACAPI_Attribute_Modify (&attrib, NULL);
								}

								// ��� ��� ��������
								ACAPI_Element_GetElemList (API_ObjectID, &elemList, APIFilt_OnVisLayer);	// ���̴� ���̾ ����, ��ü Ÿ�Ը�
								while (elemList.GetSize () > 0) {
									objects.Push (elemList.Pop ());
								}
								nObjects = objects.GetSize ();

								if (nObjects == 0)
									continue;

								// ���̾� �̸� ������
								sprintf (fullLayerName, "%s", attrib.layer.head.name);
								fullLayerName [strlen (fullLayerName)] = '\0';

								for (xx = 0 ; xx < nObjects ; ++xx) {
									foundObject = false;

									BNZeroMemory (&elem, sizeof (API_Element));
									BNZeroMemory (&memo, sizeof (API_ElementMemo));
									elem.header.guid = objects.Pop ();
									err = ACAPI_Element_Get (&elem);

									if (err == NoError && elem.header.hasMemo) {
										err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

										if (err == NoError) {
											// ��ü�� ���� �����ϱ� ==================================
											API_Coord3D	coord;

											coord.x = elem.object.pos.x;
											coord.y = elem.object.pos.y;
											coord.z = elem.object.level;
					
											vecPos.push_back (coord);
											// ��ü�� ���� �����ϱ� ==================================

											// �۾� �� ���� �ݿ� -- ��ü
											if (xx == 0) {
												BNZeroMemory (&storyInfo, sizeof (API_StoryInfo));
												workLevel_object = 0.0;
												ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo);
												for (yy = 0 ; yy < (storyInfo.lastStory - storyInfo.firstStory) ; ++yy) {
													if (storyInfo.data [0][yy].index == elem.header.floorInd) {
														workLevel_object = storyInfo.data [0][yy].level;
														break;
													}
												}
												BMKillHandle ((GSHandle *) &storyInfo.data);
											}
										}

										ACAPI_DisposeElemMemoHdls (&memo);
									}
								}

								// 3D ���� ���� ==================================
								API_3DProjectionInfo  proj3DInfo_beforeCapture;
								BNZeroMemory (&proj3DInfo_beforeCapture, sizeof (API_3DProjectionInfo));
								err = ACAPI_Environment (APIEnv_Get3DProjectionSetsID, &proj3DInfo_beforeCapture, NULL);

								API_3DProjectionInfo  proj3DInfo;
								BNZeroMemory (&proj3DInfo, sizeof (API_3DProjectionInfo));
								err = ACAPI_Environment (APIEnv_Get3DProjectionSetsID, &proj3DInfo, NULL);

								double	lowestZ, highestZ, cameraZ, targetZ;	// ���� ���� ����, ���� ���� ����, ī�޶� �� ��� ����
								API_Coord3D		p1, p2, p3, p4, p5;				// �� ��ǥ ����
								double	distanceOfPoints;						// �� �� ���� �Ÿ�
								double	angleOfPoints;							// �� �� ���� ����
								API_Coord3D		camPos1, camPos2;				// ī�޶� ���� �� �ִ� �� 2��
								API_FileSavePars		fsp;					// ���� ������ ���� ����
								API_SavePars_Picture	pars_pict;				// �׸� ���Ͽ� ���� ����

								if (err == NoError && proj3DInfo.isPersp) {
									// ���� ���� ������ ����
									// ���� ���� x���� ���� �� p1�� ã�Ƴ�
									lowestZ = highestZ = vecPos [0].z;
									p1 = vecPos [0];
									for (xx = 1 ; xx < vecPos.size () ; ++xx) {
										if (lowestZ > vecPos [xx].z)	lowestZ = vecPos [xx].z;
										if (highestZ < vecPos [xx].z)	highestZ = vecPos [xx].z;
										if (vecPos [xx].x < p1.x)	p1 = vecPos [xx];
									}
									cameraZ = (highestZ - lowestZ)/2 + workLevel_object;
									distanceOfPoints = 0.0;
									for (xx = 0 ; xx < vecPos.size () ; ++xx) {
										if (distanceOfPoints < GetDistance (p1, vecPos [xx])) {
											distanceOfPoints = GetDistance (p1, vecPos [xx]);
											p2 = vecPos [xx];
										}
									}
									// �� ��(p1, p2) ���� ���� ���ϱ�
									angleOfPoints = RadToDegree (atan2 ((p2.y - p1.y), (p2.x - p1.x)));
									// ī�޶�� ����� ���� �� �ִ� ��ġ 2���� ã��
									camPos1 = p1;
									moveIn3D ('x', DegreeToRad (angleOfPoints), distanceOfPoints/2, &camPos1.x, &camPos1.y, &camPos1.z);
									if (distanceOfPoints > (highestZ - lowestZ))
										moveIn3D ('y', DegreeToRad (angleOfPoints), -distanceOfPoints * 1.5, &camPos1.x, &camPos1.y, &camPos1.z);
									else
										moveIn3D ('y', DegreeToRad (angleOfPoints), -(highestZ - lowestZ) * 1.5, &camPos1.x, &camPos1.y, &camPos1.z);
									camPos2 = p1;
									moveIn3D ('x', DegreeToRad (angleOfPoints), distanceOfPoints/2, &camPos2.x, &camPos2.y, &camPos2.z);
									if (distanceOfPoints > (highestZ - lowestZ))
										moveIn3D ('y', DegreeToRad (angleOfPoints), distanceOfPoints * 1.5, &camPos2.x, &camPos2.y, &camPos2.z);
									else
										moveIn3D ('y', DegreeToRad (angleOfPoints), (highestZ - lowestZ) * 1.5, &camPos2.x, &camPos2.y, &camPos2.z);
									camPos1.z = cameraZ;
									camPos2.z = cameraZ;
									// ========== 1��° ĸ��
									// ī�޶� �� ��� ��ġ ����
									proj3DInfo.isPersp = true;				// �۽���Ƽ�� ��
									proj3DInfo.u.persp.viewCone = 90.0;		// ī�޶� �þ߰�
									proj3DInfo.u.persp.rollAngle = 0.0;		// ī�޶� �� ����
									proj3DInfo.u.persp.azimuth = angleOfPoints + 90.0;	// ī�޶� ������
									proj3DInfo.u.persp.pos.x = camPos1.x;
									proj3DInfo.u.persp.pos.y = camPos1.y;
									proj3DInfo.u.persp.cameraZ = camPos1.z;
									proj3DInfo.u.persp.target.x = camPos2.x;
									proj3DInfo.u.persp.target.y = camPos2.y;
									proj3DInfo.u.persp.targetZ = camPos2.z;
									err = ACAPI_Environment (APIEnv_Change3DProjectionSetsID, &proj3DInfo, NULL, NULL);
									// ȭ�� ���ΰ�ħ
									ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
									ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);
									// ȭ�� ĸ��
									ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);
									BNZeroMemory (&fsp, sizeof (API_FileSavePars));
									fsp.fileTypeID = APIFType_PNGFile;
									//sprintf (filename, "%s - ĸ�� (1).png", fullLayerName);
									sprintf (filename, "ĸ�� (1).png", fullLayerName);
									fsp.file = new IO::Location (location, IO::Name (filename));
									BNZeroMemory (&pars_pict, sizeof (API_SavePars_Picture));
									pars_pict.colorDepth	= APIColorDepth_TC32;
									pars_pict.dithered		= false;
									pars_pict.view2D		= false;
									pars_pict.crop			= false;
									err = ACAPI_Automate (APIDo_SaveID, &fsp, &pars_pict);	// ���� ���������� �۵����� ����
					
									delete fsp.file;

									// ========== 2��° ĸ��
									// ī�޶� �� ��� ��ġ ����
									proj3DInfo.isPersp = true;				// �۽���Ƽ�� ��
									proj3DInfo.u.persp.viewCone = 90.0;		// ī�޶� �þ߰�
									proj3DInfo.u.persp.rollAngle = 0.0;		// ī�޶� �� ����
									proj3DInfo.u.persp.azimuth = angleOfPoints - 90.0;	// ī�޶� ������
									proj3DInfo.u.persp.pos.x = camPos2.x;
									proj3DInfo.u.persp.pos.y = camPos2.y;
									proj3DInfo.u.persp.cameraZ = camPos2.z;
									proj3DInfo.u.persp.target.x = camPos1.x;
									proj3DInfo.u.persp.target.y = camPos1.y;
									proj3DInfo.u.persp.targetZ = camPos1.z;
									err = ACAPI_Environment (APIEnv_Change3DProjectionSetsID, &proj3DInfo, NULL, NULL);
									// ȭ�� ���ΰ�ħ
									ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
									ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);
									// ȭ�� ĸ��
									ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);
									BNZeroMemory (&fsp, sizeof (API_FileSavePars));
									fsp.fileTypeID = APIFType_PNGFile;
									//sprintf (filename, "%s - ĸ�� (2).png", fullLayerName);
									sprintf (filename, "ĸ�� (2).png", fullLayerName);
									fsp.file = new IO::Location (location, IO::Name (filename));
									BNZeroMemory (&pars_pict, sizeof (API_SavePars_Picture));
									pars_pict.colorDepth	= APIColorDepth_TC24;
									pars_pict.dithered		= false;
									pars_pict.view2D		= false;
									pars_pict.crop			= false;
									err = ACAPI_Automate (APIDo_SaveID, &fsp, &pars_pict);	// ���� ���������� �۵����� ����
					
									delete fsp.file;
								}
								// ȭ���� ĸ�� ���� ���·� ��������
								err = ACAPI_Environment (APIEnv_Change3DProjectionSetsID, &proj3DInfo_beforeCapture, NULL, NULL);
								// ȭ�� ���ΰ�ħ
								ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
								ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);
								// 3D ���� ���� ==================================

								// ���̾� �����
								attrib.layer.head.flags |= APILay_Hidden;
								ACAPI_Attribute_Modify (&attrib, NULL);
							}

							// ���� ��Ȳ ǥ���ϴ� ��� - ����
							cur = mm;
							ACAPI_Interface (APIIo_SetProcessValueID, &cur, NULL);
							if (ACAPI_Interface (APIIo_IsProcessCanceledID, NULL, NULL) == APIERR_CANCEL)
								break;
						}

						// ���� ��Ȳ ǥ���ϴ� ��� - ������
						ACAPI_Interface (APIIo_CloseProcessWindowID, NULL, NULL);

						// ��� ���μ����� ��ġ�� ó���� �����ߴ� ���̴� ���̾���� �ٽ� �ѳ��� ��
						for (xx = 1 ; xx <= nVisibleLayers ; ++xx) {
							BNZeroMemory (&attrib, sizeof (API_Attribute));
							attrib.layer.head.typeID = API_LayerID;
							attrib.layer.head.index = visLayerList [xx-1];
							err = ACAPI_Attribute_Get (&attrib);
							if (err == NoError) {
								if ((attrib.layer.head.flags & APILay_Hidden) == true) {
									attrib.layer.head.flags ^= APILay_Hidden;
									ACAPI_Attribute_Modify (&attrib, NULL);
								}
							}
						}

						// ȭ�� ���ΰ�ħ
						ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
						ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

						ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);
						location.ToDisplayText (&resultString);
						sprintf (buffer, "������� ���� ��ġ�� �����߽��ϴ�.\n\n%s\n�Ǵ� ������Ʈ ������ �ִ� ����", resultString.ToCStr ().Get ());
						ACAPI_WriteReport (buffer, true);
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
