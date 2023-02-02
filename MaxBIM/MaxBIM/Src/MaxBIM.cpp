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
					err = ACAPI_CallUndoableCommand (L"벽에 테이블폼 배치", [&] () -> GSErrCode {
						err = placeTableformOnWall ();
						return err;
					});
					break;
				case 2:
					// 슬래브 하부에 테이블폼 배치하기
					err = ACAPI_CallUndoableCommand (L"슬래브 하부에 테이블폼 배치", [&] () -> GSErrCode {
						err = placeTableformOnSlabBottom ();
						return err;
					});
					break;
				case 3:
					// 보에 테이블폼 배치하기
					err = ACAPI_CallUndoableCommand (L"보에 테이블폼 배치", [&] () -> GSErrCode {
						err = placeTableformOnBeam ();
						return err;
					});
					break;
				case 4:
					// 기둥에 테이블폼 배치하기
					err = ACAPI_CallUndoableCommand (L"기둥에 테이블폼 배치", [&] () -> GSErrCode {
						err = placeTableformOnColumn ();
						return err;
					});
					break;
				case 5:
					// 낮은 슬래브 측면에 테이블폼 배치
					err = ACAPI_CallUndoableCommand (L"낮은 슬래브 측면에 테이블폼 배치", [&] () -> GSErrCode {
						err = placeTableformOnLowSide ();
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
					err = ACAPI_CallUndoableCommand (L"선택한 객체들의 레이어 속성 변경", [&] () -> GSErrCode {
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
				case 1:		// 선택한 부재 정보 내보내기 (Single 모드)
					err = exportSelectedElementInfo ();
					break;
				case 2:		// 선택한 부재 정보 내보내기 (Multi 모드)
					err = exportElementInfoOnVisibleLayers ();
					break;
				case 3:		// 부재별 선택 후 보여주기
					err = filterSelection ();
					break;
				case 4:		// 보 테이블폼 물량표 작성
					err = exportBeamTableformInformation ();
					break;
				case 5:		// 테이블폼 면적 계산
					err = calcTableformArea ();
					break;
				case 6:		// 모든 입면도 PDF로 내보내기 (Single 모드)
					err = exportAllElevationsToPDFSingleMode ();
					break;
				case 7:		// 모든 입면도 PDF로 내보내기 (Multi 모드)
					err = exportAllElevationsToPDFMultiMode ();
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
					err = ACAPI_CallUndoableCommand (L"3D 품질/속도 조정하기", [&] () -> GSErrCode {
						err = select3DQuality ();
						return err;
					});

					break;

				case 2:		// 영역에 3D 라벨 붙이기
					err = ACAPI_CallUndoableCommand (L"영역에 3D 라벨 붙이기", [&] () -> GSErrCode {
						err = attach3DLabelOnZone ();
						return err;
					});

					break;

				case 3:		// 현재 평면도의 테이블폼에 버블 자동 배치
					err = ACAPI_CallUndoableCommand (L"현재 평면도의 테이블폼에 버블 자동 배치", [&] () -> GSErrCode {
						err = attachBubbleOnCurrentFloorPlan ();
						return err;
					});
					break;

				case 4:		// 카메라 위치 저장하기/불러오기
					err = manageCameraInfo ();

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
					//err = ACAPI_CallUndoableCommand (L"개발자 테스트", [&] () -> GSErrCode {
						GSErrCode	err = NoError;
						// *** 원하는 코드를 아래 넣으시오.
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

						// 선택한 요소들의 정보 요약하기
						API_ElementQuantity	quantity;
						API_Quantities		quantities;
						API_QuantitiesMask	mask;
						API_QuantityPar		params;
						char				reportStr [512];


						// 그룹화 일시정지 ON
						suspendGroups (true);

						// 선택한 요소 가져오기 (벽, 기둥, 보, 슬래브, 모프, 객체)
						getGuidsOfSelection (&walls, API_WallID, &nWalls);
						getGuidsOfSelection (&columns, API_ColumnID, &nColumns);
						getGuidsOfSelection (&beams, API_BeamID, &nBeams);
						getGuidsOfSelection (&slabs, API_SlabID, &nSlabs);
						getGuidsOfSelection (&morphs, API_MorphID, &nMorphs);
						getGuidsOfSelection (&objects, API_ObjectID, &nObjects);

						if ( (nWalls == 0) && (nColumns == 0) && (nBeams == 0) && (nSlabs == 0) && (nMorphs == 0) && (nObjects == 0) ) {
							DGAlert (DG_ERROR, L"오류", L"요소들을 선택해야 합니다.", "", L"확인", "", "");
							return err;
						}

						params.minOpeningSize = EPS;

						// 벽에 대한 물량 정보 추출
						ACAPI_ELEMENT_QUANTITY_MASK_CLEAR (mask);
						ACAPI_ELEMENT_QUANTITY_MASK_SET (mask, wall, volume);
						for (xx = 0 ; xx < nWalls ; ++xx) {
							quantities.elements = &quantity;
							err = ACAPI_Element_GetQuantities (walls [xx], &params, &quantities, &mask);

							if (err == NoError) {
								volume_walls += quantity.wall.area;
							}
						}

						// 기둥에 대한 물량 정보 추출
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

						// 보에 대한 물량 정보 추출
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

						// 슬래브에 대한 물량 정보 추출
						ACAPI_ELEMENT_QUANTITY_MASK_CLEAR (mask);
						ACAPI_ELEMENT_QUANTITY_MASK_SET (mask, slab, volume);
						for (xx = 0 ; xx < nSlabs ; ++xx) {
							quantities.elements = &quantity;
							err = ACAPI_Element_GetQuantities (slabs [xx], &params, &quantities, &mask);

							if (err == NoError) {
								volume_slabs += quantity.slab.bottomSurface;
							}
						}

						// 모프에 대한 물량 정보 추출
						ACAPI_ELEMENT_QUANTITY_MASK_CLEAR (mask);
						ACAPI_ELEMENT_QUANTITY_MASK_SET (mask, morph, volume);
						for (xx = 0 ; xx < nMorphs ; ++xx) {
							quantities.elements = &quantity;
							err = ACAPI_Element_GetQuantities (morphs [xx], &params, &quantities, &mask);

							if (err == NoError) {
								volume_morphs += quantity.morph.surface;
							}
						}

						// 객체에 대한 물량 정보 추출
						ACAPI_ELEMENT_QUANTITY_MASK_CLEAR (mask);
						ACAPI_ELEMENT_QUANTITY_MASK_SET (mask, symb, volume);
						for (xx = 0 ; xx < nObjects ; ++xx) {
							quantities.elements = &quantity;
							err = ACAPI_Element_GetQuantities (objects [xx], &params, &quantities, &mask);

							if (err == NoError) {
								volume_objects += quantity.symb.surface;
							}
						}

						// 총 부피 계산
						volume_total = volume_walls + volume_columns + volume_beams + volume_slabs + volume_morphs + volume_objects;

						sprintf (reportStr, "%12s: %lf ㎥\n%12s: %lf ㎥\n%12s: %lf ㎥\n%12s: %lf ㎥\n%12s: %lf ㎥\n%12s: %lf ㎥\n\n%12s: %lf ㎥",
											"벽 부피", volume_walls,
											"기둥 부피", volume_columns,
											"보 부피", volume_beams,
											"슬래브 부피", volume_slabs,
											"모프 부피", volume_morphs,
											"객체 부피", volume_objects,
											"총 부피", volume_total);
						WriteReport_Alert (reportStr);

						// 그룹화 일시정지 OFF
						suspendGroups (false);

						return	err;
						// *** 원하는 코드를 위에 넣으시오.
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

	setlocale (LC_ALL, "Korean");	// 한국어 로케일 설정 (유니코드 문자열)

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
