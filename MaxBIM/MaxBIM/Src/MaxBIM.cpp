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
	
	err = ACAPI_Register_Menu (32001, 32002, MenuCode_UserDef, MenuFlag_Default);	// 유로폼 배치
	err = ACAPI_Register_Menu (32011, 32012, MenuCode_UserDef, MenuFlag_Default);	// 테이블폼 배치
	err = ACAPI_Register_Menu (32015, 32016, MenuCode_UserDef, MenuFlag_Default);	// 동바리 배치
	err = ACAPI_Register_Menu (32013, 32014, MenuCode_UserDef, MenuFlag_Default);	// Library Converting
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
					// 벽에 테이블폼 배치하기 - 세로 방향
					err = ACAPI_CallUndoableCommand ("벽에 테이블폼 배치 - 세로 방향", [&] () -> GSErrCode {
						err = placeTableformOnWall_Vertical ();
						return err;
					});
					break;
				case 2:
					// 벽에 테이블폼 배치하기 - 가로 방향
					err = ACAPI_CallUndoableCommand ("벽에 테이블폼 배치 - 가로 방향", [&] () -> GSErrCode {
						err = placeTableformOnWall_Horizontal ();
						return err;
					});
					break;
				case 3:
					// 벽에 테이블폼 배치하기 - 커스텀
					err = ACAPI_CallUndoableCommand ("벽에 테이블폼 배치 - 커스텀", [&] () -> GSErrCode {
						err = placeTableformOnWall_Custom ();
						return err;
					});
					break;
				case 4:
					// 슬래브 하부에 테이블폼 배치하기
					err = ACAPI_CallUndoableCommand ("슬래브 하부에 테이블폼 배치", [&] () -> GSErrCode {
						err = placeTableformOnSlabBottom ();
						return err;
					});
					break;
				case 5:
					// 보에 테이블폼 배치하기
					err = ACAPI_CallUndoableCommand ("보에 테이블폼 배치", [&] () -> GSErrCode {
						err = placeTableformOnBeam ();
						return err;
					});
					break;
				case 6:
					// 기둥에 테이블폼 배치하기
					err = ACAPI_CallUndoableCommand ("기둥에 테이블폼 배치", [&] () -> GSErrCode {
						err = placeTableformOnColumn ();
						return err;
					});
					break;
			}
			break;

		case 32015:
			// 동바리 배치
			switch (menuParams->menuItemRef.itemIndex) {
			case 1:
				// PERI 동바리 자동 배치
				err = ACAPI_CallUndoableCommand ("PERI 동바리 자동 배치", [&] () -> GSErrCode {
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
					// 가상 가설재 모두 변환
					err = ACAPI_CallUndoableCommand ("가상 가설재 모두 변환", [&] () -> GSErrCode {
						err = convertVirtualTCO ();		// TCO: Temporary Construction Object
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
						unsigned short		xx, yy, zz;
						short		mm;
						bool		regenerate = true;
						bool		suspGrp;

						// 모든 객체들의 원점 좌표를 전부 저장함
						vector<API_Coord3D>	vecPos;

						// 모든 객체, 보 저장
						GS::Array<API_Guid>		elemList;
						GS::Array<API_Guid>		objects;
						long					nObjects = 0;

						// 선택한 요소들의 정보 요약하기
						API_Element			elem;
						API_ElementMemo		memo;

						char			tempStr [512];
						const char*		foundStr;
						bool			foundObject;
						bool			foundExistValue;
						int				retVal;
						int				nInfo;

						// 레이어 관련 변수
						short			nLayers;
						API_Attribute	attrib;
						short			nVisibleLayers = 0;
						short			visLayerList [1024];
						char			fullLayerName [512];

						// 레이어 타입에 따라 캡쳐 방향 지정
						char*			foundLayerName;

						// 기타
						char			buffer [512];
						char			filename [512];

						// 작업 층 정보
						API_StoryInfo	storyInfo;
						double			workLevel_object;		// 벽의 작업 층 높이


						// 진행바를 표현하기 위한 변수
						GS::UniString       title ("내보내기 진행 상황");
						GS::UniString       subtitle ("진행중...");
						short	nPhase;
						Int32	cur, total;

						// 엑셀 파일로 기둥 정보 내보내기
						// 파일 저장을 위한 변수
						API_SpecFolderID	specFolderID = API_ApplicationFolderID;
						IO::Location		location;
						GS::UniString		resultString;
						API_MiscAppInfo		miscAppInfo;


						// 그룹화 일시정지 ON
						ACAPI_Environment (APIEnv_IsSuspendGroupOnID, &suspGrp);
						if (suspGrp == false)	ACAPI_Element_Tool (NULL, NULL, APITool_SuspendGroups, NULL);

						// 화면 새로고침
						ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
						ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

						//ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);

						// 프로젝트 내 레이어 개수를 알아냄
						BNZeroMemory (&attrib, sizeof (API_Attribute));
						attrib.layer.head.typeID = API_LayerID;
						err = ACAPI_Attribute_GetNum (API_LayerID, &nLayers);

						// 보이는 레이어들의 목록 저장하기
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

						// 일시적으로 모든 레이어 숨기기
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

						// 진행 상황 표시하는 기능 - 초기화
						nPhase = 1;
						cur = 1;
						total = nVisibleLayers;
						ACAPI_Interface (APIIo_InitProcessWindowID, &title, &nPhase);
						ACAPI_Interface (APIIo_SetNextProcessPhaseID, &subtitle, &total);

						// 보이는 레이어들을 하나씩 순회하면서 전체 요소들을 선택한 후 "선택한 부재 정보 내보내기" 루틴 실행
						for (mm = 1 ; mm <= nVisibleLayers ; ++mm) {
							BNZeroMemory (&attrib, sizeof (API_Attribute));
							attrib.layer.head.typeID = API_LayerID;
							attrib.layer.head.index = visLayerList [mm-1];
							err = ACAPI_Attribute_Get (&attrib);

							// 초기화
							objects.Clear ();
							vecPos.clear ();

							if (err == NoError) {
								// 레이어 보이기
								if ((attrib.layer.head.flags & APILay_Hidden) == true) {
									attrib.layer.head.flags ^= APILay_Hidden;
									ACAPI_Attribute_Modify (&attrib, NULL);
								}

								// 모든 요소 가져오기
								ACAPI_Element_GetElemList (API_ObjectID, &elemList, APIFilt_OnVisLayer);	// 보이는 레이어에 있음, 객체 타입만
								while (elemList.GetSize () > 0) {
									objects.Push (elemList.Pop ());
								}
								nObjects = objects.GetSize ();

								if (nObjects == 0)
									continue;

								// 레이어 이름 가져옴
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
											// 객체의 원점 수집하기 ==================================
											API_Coord3D	coord;

											coord.x = elem.object.pos.x;
											coord.y = elem.object.pos.y;
											coord.z = elem.object.level;
					
											vecPos.push_back (coord);
											// 객체의 원점 수집하기 ==================================

											// 작업 층 높이 반영 -- 객체
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

								// 3D 투영 정보 ==================================
								API_3DProjectionInfo  proj3DInfo_beforeCapture;
								BNZeroMemory (&proj3DInfo_beforeCapture, sizeof (API_3DProjectionInfo));
								err = ACAPI_Environment (APIEnv_Get3DProjectionSetsID, &proj3DInfo_beforeCapture, NULL);

								API_3DProjectionInfo  proj3DInfo;
								BNZeroMemory (&proj3DInfo, sizeof (API_3DProjectionInfo));
								err = ACAPI_Environment (APIEnv_Get3DProjectionSetsID, &proj3DInfo, NULL);

								double	lowestZ, highestZ, cameraZ, targetZ;	// 가장 낮은 높이, 가장 높은 높이, 카메라 및 대상 높이
								API_Coord3D		p1, p2, p3, p4, p5;				// 점 좌표 저장
								double	distanceOfPoints;						// 두 점 간의 거리
								double	angleOfPoints;							// 두 점 간의 각도
								API_Coord3D		camPos1, camPos2;				// 카메라가 있을 수 있는 점 2개
								API_FileSavePars		fsp;					// 파일 저장을 위한 변수
								API_SavePars_Picture	pars_pict;				// 그림 파일에 대한 설명

								if (err == NoError && proj3DInfo.isPersp) {
									// 높이 값의 범위를 구함
									// 가장 작은 x값을 갖는 점 p1도 찾아냄
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
									// 두 점(p1, p2) 간의 각도 구하기
									angleOfPoints = RadToDegree (atan2 ((p2.y - p1.y), (p2.x - p1.x)));
									// 카메라와 대상이 있을 수 있는 위치 2개를 찾음
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
									// ========== 1번째 캡쳐
									// 카메라 및 대상 위치 설정
									proj3DInfo.isPersp = true;				// 퍼스펙티브 뷰
									proj3DInfo.u.persp.viewCone = 90.0;		// 카메라 시야각
									proj3DInfo.u.persp.rollAngle = 0.0;		// 카메라 롤 각도
									proj3DInfo.u.persp.azimuth = angleOfPoints + 90.0;	// 카메라 방위각
									proj3DInfo.u.persp.pos.x = camPos1.x;
									proj3DInfo.u.persp.pos.y = camPos1.y;
									proj3DInfo.u.persp.cameraZ = camPos1.z;
									proj3DInfo.u.persp.target.x = camPos2.x;
									proj3DInfo.u.persp.target.y = camPos2.y;
									proj3DInfo.u.persp.targetZ = camPos2.z;
									err = ACAPI_Environment (APIEnv_Change3DProjectionSetsID, &proj3DInfo, NULL, NULL);
									// 화면 새로고침
									ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
									ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);
									// 화면 캡쳐
									ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);
									BNZeroMemory (&fsp, sizeof (API_FileSavePars));
									fsp.fileTypeID = APIFType_PNGFile;
									//sprintf (filename, "%s - 캡쳐 (1).png", fullLayerName);
									sprintf (filename, "캡쳐 (1).png", fullLayerName);
									fsp.file = new IO::Location (location, IO::Name (filename));
									BNZeroMemory (&pars_pict, sizeof (API_SavePars_Picture));
									pars_pict.colorDepth	= APIColorDepth_TC32;
									pars_pict.dithered		= false;
									pars_pict.view2D		= false;
									pars_pict.crop			= false;
									err = ACAPI_Automate (APIDo_SaveID, &fsp, &pars_pict);	// 데모 버전에서는 작동하지 않음
					
									delete fsp.file;

									// ========== 2번째 캡쳐
									// 카메라 및 대상 위치 설정
									proj3DInfo.isPersp = true;				// 퍼스펙티브 뷰
									proj3DInfo.u.persp.viewCone = 90.0;		// 카메라 시야각
									proj3DInfo.u.persp.rollAngle = 0.0;		// 카메라 롤 각도
									proj3DInfo.u.persp.azimuth = angleOfPoints - 90.0;	// 카메라 방위각
									proj3DInfo.u.persp.pos.x = camPos2.x;
									proj3DInfo.u.persp.pos.y = camPos2.y;
									proj3DInfo.u.persp.cameraZ = camPos2.z;
									proj3DInfo.u.persp.target.x = camPos1.x;
									proj3DInfo.u.persp.target.y = camPos1.y;
									proj3DInfo.u.persp.targetZ = camPos1.z;
									err = ACAPI_Environment (APIEnv_Change3DProjectionSetsID, &proj3DInfo, NULL, NULL);
									// 화면 새로고침
									ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
									ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);
									// 화면 캡쳐
									ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);
									BNZeroMemory (&fsp, sizeof (API_FileSavePars));
									fsp.fileTypeID = APIFType_PNGFile;
									//sprintf (filename, "%s - 캡쳐 (2).png", fullLayerName);
									sprintf (filename, "캡쳐 (2).png", fullLayerName);
									fsp.file = new IO::Location (location, IO::Name (filename));
									BNZeroMemory (&pars_pict, sizeof (API_SavePars_Picture));
									pars_pict.colorDepth	= APIColorDepth_TC24;
									pars_pict.dithered		= false;
									pars_pict.view2D		= false;
									pars_pict.crop			= false;
									err = ACAPI_Automate (APIDo_SaveID, &fsp, &pars_pict);	// 데모 버전에서는 작동하지 않음
					
									delete fsp.file;
								}
								// 화면을 캡쳐 이전 상태로 돌려놓음
								err = ACAPI_Environment (APIEnv_Change3DProjectionSetsID, &proj3DInfo_beforeCapture, NULL, NULL);
								// 화면 새로고침
								ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
								ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);
								// 3D 투영 정보 ==================================

								// 레이어 숨기기
								attrib.layer.head.flags |= APILay_Hidden;
								ACAPI_Attribute_Modify (&attrib, NULL);
							}

							// 진행 상황 표시하는 기능 - 진행
							cur = mm;
							ACAPI_Interface (APIIo_SetProcessValueID, &cur, NULL);
							if (ACAPI_Interface (APIIo_IsProcessCanceledID, NULL, NULL) == APIERR_CANCEL)
								break;
						}

						// 진행 상황 표시하는 기능 - 마무리
						ACAPI_Interface (APIIo_CloseProcessWindowID, NULL, NULL);

						// 모든 프로세스를 마치면 처음에 수집했던 보이는 레이어들을 다시 켜놓을 것
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

						// 화면 새로고침
						ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
						ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

						ACAPI_Environment (APIEnv_GetSpecFolderID, &specFolderID, &location);
						location.ToDisplayText (&resultString);
						sprintf (buffer, "결과물을 다음 위치에 저장했습니다.\n\n%s\n또는 프로젝트 파일이 있는 폴더", resultString.ToCStr ().Get ());
						ACAPI_WriteReport (buffer, true);
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
	
	err = ACAPI_Install_MenuHandler (32001, MenuCommandHandler);	// 유로폼 배치
	err = ACAPI_Install_MenuHandler (32011, MenuCommandHandler);	// 테이블폼 배치
	err = ACAPI_Install_MenuHandler (32015, MenuCommandHandler);	// 동바리 배치
	err = ACAPI_Install_MenuHandler (32013, MenuCommandHandler);	// Library Converting
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
