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
	
	err = ACAPI_Register_Menu (32001, 32002, MenuCode_UserDef, MenuFlag_Default);	// 유로폼 배치
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
				case 2:		// 선택한 부재 정보 내보내기
					err = exportSelectedElementInfo ();
					break;
			}
			break;
		case 32009:
			// 물량 산출
			switch (menuParams->menuItemRef.itemIndex) {
				case 1:		// 물량합판 부착하기
					err = ACAPI_CallUndoableCommand ("물량합판 부착하기", [&] () -> GSErrCode {
						err = placeQuantityPlywood ();
						return err;
					});
					break;
				case 2:		// 물량합판 면적 계산하기
					err = calcAreasOfQuantityPlywood ();
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
					break;
				case 3:		// 개발자 전용 - 개발자 테스트 메뉴
					err = ACAPI_CallUndoableCommand ("개발자 테스트", [&] () -> GSErrCode {
						GSErrCode	err = NoError;
						short		xx, yy;

						// Selection Manager 관련 변수
						long		nSel;
						API_SelectionInfo		selectionInfo;
						API_Element				tElem;
						API_Neig				**selNeigs;
						GS::Array<API_Guid>&	morphs = GS::Array<API_Guid> ();
						GS::Array<API_Guid>&	polylines = GS::Array<API_Guid> ();
						long					nMorphs = 0;
						long					nPolylines = 0;

						// 객체 정보 가져오기
						API_Element				elem;
						API_ElementMemo			memo;
						API_ElemInfo3D			info3D;

						// 모프 3D 구성요소 가져오기
						API_Component3D			component;
						API_Tranmat				tm;
						Int32					nVert, nEdge, nPgon;
						Int32					elemIdx, bodyIdx;
						API_Coord3D				trCoord;
						GS::Array<API_Coord3D>&	coords = GS::Array<API_Coord3D> ();

						// 모프에서 제외되는 점 4개
						API_Coord3D		excludeP1;
						API_Coord3D		excludeP2;
						API_Coord3D		excludeP3;
						API_Coord3D		excludeP4;

						excludeP1.x = 0;	excludeP1.y = 0;	excludeP1.z = 0;
						excludeP2.x = 1;	excludeP2.y = 0;	excludeP2.z = 0;
						excludeP3.x = 0;	excludeP3.y = 1;	excludeP3.z = 0;
						excludeP4.x = 0;	excludeP4.y = 0;	excludeP4.z = 1;

						// 기타
						char	buffer [256];


						// 선택한 요소 가져오기
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
								if (ACAPI_Element_Get (&tElem) != NoError)	// 가져올 수 있는 요소인가?
									continue;

								if (tElem.header.typeID == API_MorphID)		// 모프인가?
									morphs.Push (tElem.header.guid);

								if (tElem.header.typeID == API_PolyLineID)	// 폴리라인인가?
									polylines.Push (tElem.header.guid);
							}
						}
						BMKillHandle ((GSHandle *) &selNeigs);
						nMorphs = morphs.GetSize ();
						nPolylines = polylines.GetSize ();

						if (nMorphs > 0) {
							for (xx = 0 ; xx < nMorphs ; ++xx) {
								// 모프의 정보를 가져옴
								BNZeroMemory (&elem, sizeof (API_Element));
								elem.header.guid = morphs.Pop ();
								err = ACAPI_Element_Get (&elem);
								err = ACAPI_Element_Get3DInfo (elem.header, &info3D);

								// 모프의 점 좌표들을 가져옴
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
	
								// 정점 좌표를 가져옴
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
								// 폴리라인의 정보를 가져옴
								BNZeroMemory (&elem, sizeof (API_Element));
								BNZeroMemory (&memo, sizeof (API_ElementMemo));
								elem.header.guid = polylines.Pop ();
								err = ACAPI_Element_Get (&elem);
								err = ACAPI_Element_GetMemo (elem.header.guid, &memo);

								// 정점 좌표를 가져옴
								for (yy = 1 ; yy <= elem.polyLine.poly.nCoords ; ++yy) {
									sprintf (buffer, "%d ", yy);
									err = placeCoordinateLabel (memo.coords [0][yy].x, memo.coords [0][yy].y, 0, true, buffer);
								}
							}
						}

						// 여러 개의 벽 구간 가져오기
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
	
	err = ACAPI_Install_MenuHandler (32001, MenuCommandHandler);	// 유로폼 배치
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
