// *********************************************************************************************************************
// API dll functions
//
// Module:			API
// Namespace:		-
// Contact person:
//
// [SG compatible]
// *********************************************************************************************************************

#if !defined (ACAPINC_DLL_H)
#define	ACAPINC_DLL_H

#if defined (_MSC_VER)
#pragma once
#endif


// --- Includes	--------------------------------------------------------------------------------------------------------

#if defined (WINDOWS)
#include "Win32Interface.hpp"
#endif

// GSRoot
#include "GSRoot.hpp"
#include "Array.hpp"
#include "PagedArray.hpp"
#include "UniString.hpp"

// own
#include "APIcalls.h"
#include "APIdefs.h"
#include "APIdefs_Registration.h"
#include "API_Guid.hpp"

// --- Predeclarations -------------------------------------------------------------------------------------------

namespace Geometry {
	class PolyId;

	template <class VertexUserData, class EdgeUserData, class ContourUserData, class PolygonUserData> class CustomMultiPolygon2D;
	typedef CustomMultiPolygon2D<PolyId, PolyId, PolyId, PolyId> MultiPolygon2D;
}

// --- Compiler settings -----------------------------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

#if defined (_MSC_VER)
	#pragma pack(push, 8)
#endif


// --- Predeclarations -------------------------------------------------------------------------------------------

namespace Modeler {
	class Body;
}


// --- Function declarations -------------------------------------------------------------------------------------------

// =============================================================================
// Functions must be supported by the addon
//
// Following functions must be implemented in each extension.
// They are called from ArchiCAD.
// For detailed description refer to the enclosed documentation.
//
// =============================================================================

API_AddonType	__ACDLL_CALL	CheckEnvironment (API_EnvirParams* envirParams);
GSErrCode		__ACDLL_CALL	RegisterInterface (void);
GSErrCode		__ACDLL_CALL	Initialize (void);
GSErrCode		__ACDLL_CALL	FreeData (void);

typedef void*	ClientID;

#if defined (ACExtension)
/* [ --> */
// =============================================================================
// Function supported by the API
//
// Following functions are supported by the ACAP library.
// For detailed description refer to the enclosed documentation.
//
// =============================================================================

#include "APIdefs_Callback.h"


/* -- Server ------------------------------ */
#define	ServerMainVers_600			0x0001			// Package 6.00:  (ArchiCAD 6.0, PlotMaker 2.1)
#define	ServerMainVers_650			0x0002			// Package 6.50:  (ArchiCAD 6.5, PlotMaker 2.2)
#define	ServerMainVers_700			0x0003			// Package 7.00:  (ArchiCAD 7.0, PlotMaker 2.3)
#define	ServerMainVers_800			0x0004			// Package 8.00:  (ArchiCAD 8,   PlotMaker 3)
#define	ServerMainVers_810			0x0005			// Package 8.10:  (ArchiCAD 8.1, PlotMaker 3.1)
#define	ServerMainVers_900			0x0006			// Package 9.00:  (ArchiCAD 9,   PlotMaker 9)
#define	ServerMainVers_1000			0x000A			// ArchiCAD 10
#define	ServerMainVers_1100			0x000B			// ArchiCAD 11
#define	ServerMainVers_1200			0x000C			// ArchiCAD 12
#define	ServerMainVers_1300			0x000D			// ArchiCAD 13
#define	ServerMainVers_1400			0x000E			// ArchiCAD 14
#define	ServerMainVers_1500			0x000F			// ArchiCAD 15
#define	ServerMainVers_1600			0x0010			// ArchiCAD 16
#define	ServerMainVers_1700			0x0011			// ArchiCAD 17
#define	ServerMainVers_1800			0x0012			// ArchiCAD 18
#define	ServerMainVers_1900			0x0013			// ArchiCAD 19

#define	ServerReleaseVers_R1		1



__APIEXPORT void		__ACENV_CALL	ACAPI_GetReleaseNumber (ClientID clientPtr, API_ServerApplicationInfo	*serverApplicationInfo);

/* -- System ------------------------------ */
#if defined (WINDOWS)
__APIEXPORT HINSTANCE	__ACENV_CALL	ACAPI_GetExtensionInstance (void);

__APIEXPORT HWND		__ACENV_CALL	ACAPI_GetMainWindow (ClientID clientPtr);
#endif
__APIEXPORT GSResModule	__ACENV_CALL	ACAPI_GetOwnResModule (void);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_GetOwnLocation (IO::Location *ownFileLoc);

__APIEXPORT void		__ACENV_CALL	ACAPI_SetDllTermHookProc (APIDllTermHookProc* dllTermHookProc);


/* -- Registration ------------------ */

	// CheckEnvironment
__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Register_SupportedService (GSType cmdID, Int32 cmdVersion);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Register_RequiredService (const API_ModulID	*modulID,
																		GSType				cmdID,
																		Int32				cmdVersion);

	// RegisterInterface
__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Register_Menu		(short			menuStrResID,
																 short			promptStrResID,
																 APIMenuCodeID	menuPosCode,
																 GSFlags		menuFlags);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Register_FileType (Int32			refCon,
																 GSType			ftype,
																 GSType			fcreator,
																 const char		*extname,
																 short			iconResID,
																 short			descStrResID,
																 short			descStrResItemID,
																 API_IOMethod	methodFlags);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Register_Subtype	(GSType				signature,				// identifier
																 short				templateFileResID,		// subtype template (FILE resource ID)
																 bool				transparentNode,		// allow ancestor tools to place object instances
																 short				stringResID,			// tool descriptions (STR# resource ID)
																 short				smallIconResID,			// toolbar icon
																 short				largeIconResID,			// toolbox icon
																 API_TBoxGroup		toolGroupId,			// toolbar group
																 API_TBoxEnabled	enableFlags,			// toolbar enable flags
																 short				defaultsResID);			// factory defaults (ACP0 resource ID)

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Register_SettingsPanel	(Int32					refCon,
																		 API_ElemTypeID			tool,
																		 API_ElemVariationID	variation,
																		 short					iconId,
																		 short					stringId,
																		 short					pageId,
																		 short					growType = 0,	// means NoGrow
																		 bool					visibleByDefault = false);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Register_AttributePanel	(Int32					refCon,
																		 API_AttrTypeID			attr,
																		 short					iconId,
																		 short					stringId,
																		 short					pageId,
																		 short					growType = 0);	// means NoGrow

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Register_InfoBoxPanel		(Int32					refCon,
																		 API_ElemTypeID			tool,
																		 API_ElemVariationID	variation,
																		 short					stringId,
																		 short					pageId,
																		 bool					visibleByDefault = false);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Register_ESYM		(GSType			signature,				// LibPart owner ID
																 API_LibTypeID	typeID,					// LibPart base type (Window, Door, Object, Lamp)
																 short			stringResID);			// description

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Register_PropertyHandler (short assignStrResID);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Register_BuiltInLibrary (void);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Register_ModulDataHandler (void);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Register_AddOnObjectHandler (void);


	// Initialize
__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Install_ModulCommandHandler  (ClientID				clientPtr,
																			GSType					cmdID,
																			Int32					cmdVersion,
																			APIModulCommandProc*	handlerProc);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Install_MenuHandler		   (ClientID				clientPtr,
																			short					menuStrResID,
																			APIMenuCommandProc*		handlerProc);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Install_FileTypeHandler	   (ClientID				clientPtr,
																			GSType					cmdID,
																			APIIOCommandProc*		handlerProc);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Install_FileTypeHandler3D	   (ClientID				clientPtr,
																			GSType					cmdID,
																			APIIO3DCommandProc*		handlerProc);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Install_PanelHandler (ClientID				clientPtr,
																	Int32					refCon,
																	APIPanelCreateProc*		handlerCreateProc,
																	APIPanelDestroyProc*	handlerDestroyProc);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Install_ESYMHandler		   (ClientID				clientPtr,
																			GSType					signature,
																			APIESYMCommandProc*		handlerProc);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Install_PropertyHandler		(ClientID				clientPtr,
																			 short					strResID,
																			 APIPropertyCommandProc* propertyCommandProc);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Install_ModulDataMergeHandler	(ClientID						clientPtr,
																			 APIModulDataMergeHandlerProc*	mergeHandlerProc);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Install_ModulDataSaveOldFormatHandler (ClientID								clientPtr,
																					 APIModulDataSaveOldFormatHandlerProc*	modulDataSaveOldFormatProc);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Install_ElemSetSaveOldFormatHandler (ClientID								clientPtr,
																				   APIElemSetSaveOldFormatHandlerProc*	elemSetSaveOldFormatProc);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Install_AddOnObjectMergeHandler			(ClientID										clientPtr,
																						 APIAddonObjectMergeHandlerProc*				mergeHandlerProc);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Install_AddOnObjectSaveOldFormatHandler	(ClientID										clientPtr,
																						 APIAddonObjectSaveOldFormatHandlerProc*		saveOldFormatProc);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Install_AddOnObjectConvertNewFormatHandler (ClientID										clientPtr,
																						  APIAddonObjectConvertNewFormatHandlerProc*	convertNewFormatProc);


/* -- Memory Manager ---------------------- */
/**
 * @defgroup MemMgr Memory Manager
 */
/*@{*/

__APIEXPORT void		__ACENV_CALL	ACAPI_DisposeAddParHdl (API_AddParType ***addPars);

__APIEXPORT void		__ACENV_CALL	ACAPI_DisposeParagraphsHdl (API_ParagraphType ***paragraphs);

__APIEXPORT void		__ACENV_CALL	ACAPI_DisposeElemMemoHdls (API_ElementMemo *memo);

__APIEXPORT void		__ACENV_CALL	ACAPI_DisposeAttrDefsHdls (API_AttributeDef *defs);

__APIEXPORT void		__ACENV_CALL	ACAPI_DisposeAttrDefsHdlsExt (API_AttributeDefExt *defs);

__APIEXPORT void		__ACENV_CALL	ACAPI_DisposeRoomRelationHdls (API_RoomRelation *roomInfo);

__APIEXPORT void		__ACENV_CALL	ACAPI_DisposeWallRelationHdls (API_WallRelation *wallInfo);

__APIEXPORT void		__ACENV_CALL	ACAPI_DisposeBeamRelationHdls (API_BeamRelation *beamInfo);

__APIEXPORT void		__ACENV_CALL	ACAPI_DisposeWorkspaceInfoHdls (API_WorkspaceInfo *workspaceInfo);

__APIEXPORT void		__ACENV_CALL	ACAPI_FreeGridMeshPtr (API_GridMesh **gridMeshes);

__APIEXPORT void		__ACENV_CALL	ACAPI_FreeCWContourPtr (API_CWContourType **contours);

__APIEXPORT void		__ACENV_CALL	ACAPI_FreeGDLModelViewOptionsPtr (API_GDLModelViewOptions **gdlModelViewOptions);

/*@}*/

/* -- Resource Manager -------------------- */
/**
 * @defgroup ResMgr Resource Manager
 */
/*@{*/

__APIEXPORT bool		__ACENV_CALL	ACAPI_Resource_GetLocStr (char *str, short resID, short index, GSResModule resModule = ACAPI_GetOwnResModule ());

__APIEXPORT bool		__ACENV_CALL	ACAPI_Resource_GetLocUStr (GS::uchar_t *str, short resID, short index, GSResModule resModule = ACAPI_GetOwnResModule ());

__APIEXPORT bool		__ACENV_CALL	ACAPI_Resource_GetFixStr (char *str, short resID, short index, GSResModule resModule = ACAPI_GetOwnResModule ());

/*@}*/

/* -- Client Manager ---------------------- */
__APIEXPORT void		__ACENV_CALL	ACAPI_KeepInMemory (ClientID clientPtr, bool keepIn);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_SetPreferences (ClientID clientPtr, Int32 version, GSSize nByte, const void *data);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_GetPreferences (ClientID clientPtr, Int32 *version, GSSize *nByte, void *data);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_GetPreferences_Platform (Int32			*version,
																	   GSSize			*nByte,
																	   void				*data,
																	   unsigned short	*platformSign);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_SetPreferences_OldVersion (ClientID			clientPtr,
																		 Int32				version,
																		 GSSize				nByte,
																		 const void*		data,
																		 unsigned short		platformSign,
																		 API_FTypeID		oldPlanFileID);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_RegisterModelessWindow (ClientID						clientPtr,
																	  Int32							referenceID,
																	  APIPaletteControlCallBackProc	*callBackProc,
																	  GSFlags						controlFlags,
																	  const API_Guid&				paletteGuid = APINULLGuid);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_UnregisterModelessWindow (ClientID clientPtr, Int32 referenceID);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_CallUndoableCommand (ClientID clientPtr, const GS::UniString& undoString, const std::function<GSErrCode ()>& command);

__APIEXPORT void		__ACENV_CALL	ACAPI_WriteReport (ClientID clientPtr, const char *msg, bool withDial);


/* -- General Manager --------------------- */
/**
 * @defgroup GenFunc General Functions
 */
/*@{*/

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Database		(ClientID clientPtr, API_DatabaseID code, void* par1 = NULL, void* par2 = NULL, void* par3 = NULL);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Environment	(ClientID clientPtr, API_EnvironmentID code, void* par1 = NULL, void* par2 = NULL, void* par3 = NULL);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Interface		(ClientID clientPtr, API_InterfaceID code, void* par1 = NULL, void* par2 = NULL, void* par3 = NULL, void* par4 = NULL);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Goodies		(ClientID clientPtr, API_GoodiesID code, void* par1 = NULL, void* par2 = NULL, void* par3 = NULL);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Automate		(ClientID clientPtr, API_AutomateID code, void* par1 = NULL, void* par2 = NULL, void* par3 = NULL);

/*@}*/

/* -- Attribute Manager ------------------- */

/**
 * @defgroup AttrMan Attribute Manager
 */
/*@{*/

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Attribute_GetNum (ClientID clientPtr, API_AttrTypeID typeID, short *count);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Attribute_Get (ClientID clientPtr, API_Attribute *attribute);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Attribute_GetDef (ClientID clientPtr, API_AttrTypeID typeID, short index, API_AttributeDef *defs);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Attribute_GetDefExt (ClientID				clientPtr,
																   API_AttrTypeID		typeID,
																   short				index,
																   API_AttributeDefExt	*defs);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Attribute_Create (ClientID clientPtr, API_Attribute *attribute, API_AttributeDef *defs);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Attribute_CreateExt (ClientID clientPtr, API_Attribute *attribute, API_AttributeDefExt *defs);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Attribute_Modify (ClientID clientPtr, API_Attribute *attribute, API_AttributeDef *defs);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Attribute_ModifyExt (ClientID clientPtr, API_Attribute *attribute, API_AttributeDefExt *defs);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Attribute_Search (ClientID clientPtr, API_Attr_Head *attrHead);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Attribute_Delete (ClientID clientPtr, const API_Attr_Head *attrHead);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Attribute_DeleteMore (ClientID clientPtr, const API_Attr_Head *attrHead, short* attrIndexListPtr);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Attribute_GetUserData (ClientID				clientPtr,
																	 API_Attr_Head			*attrHead,
																	 API_AttributeUserData	*userData);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Attribute_SetUserData (ClientID						clientPtr,
																	 API_Attr_Head					*attrHead,
																	 const API_AttributeUserData	*userData);

/*@}*/

/* -- Library Part Manager ---------------- */

/**
 * \defgroup LPMan Library Part Manager
 */
/*@{*/

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_LibPart_GetNum (ClientID clientPtr, Int32 *count);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_LibPart_Search (ClientID clientPtr, API_LibPart *ancestor, bool createIfMissing, bool onlyPlaceable = false);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_LibPart_PatternSearch (ClientID clientPtr, const API_LibPart *ancestor, const GS::UniString& pattern, API_LibPart result[50], Int32* numFound);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_LibPart_Get (ClientID clientPtr, API_LibPart *libPart);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_LibPart_GetParams (ClientID		clientPtr,
																 Int32			libInd,
																 double			*a,
																 double			*b,
																 Int32			*addParNum,
																 API_AddParType	***addPars);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_LibPart_GetSection (ClientID				clientPtr,
																  Int32					libInd,
																  API_LibPartSection	*section,
																  GSHandle				*sectionHdl,
																  GS::UniString			*sectionStr,
																  GS::UniString			*password = NULL);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_LibPart_UpdateSection (ClientID					clientPtr,
																	 Int32						libInd,
																	 const API_LibPartSection	*section,
																	 GSHandle					sectionHdl,
																	 GS::UniString				*sectionStr,
																	 GS::UniString				*password = NULL);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_LibPart_SetUpSect_2DDrawHdl (ClientID clientPtr);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_LibPart_GetSect_2DDrawHdl (ClientID clientPtr, GSHandle *sectionHdl);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_LibPart_GetSect_ParamDef (ClientID			clientPtr,
																		const API_LibPart	*libPart,
																		API_AddParType		**addPars,
																		double				*a,
																		double				*b,
																		GSHandle			sect2DDrawHdl,
																		GSHandle			*sectionHdl,
																		GS::UniString		*password = NULL);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_LibPart_Create (ClientID clientPtr, const API_LibPart *libPart);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_LibPart_AddSection (ClientID					clientPtr,
																  const API_LibPartSection	*section,
																  GSHandle					sectionHdl,
																  GS::UniString				*sectionStr);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_LibPart_NewSection (ClientID clientPtr, const API_LibPartSection *section);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_LibPart_WriteSection (ClientID clientPtr, Int32 nByte, GSPtr data);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_LibPart_EndSection (ClientID clientPtr);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_LibPart_Save (ClientID clientPtr, API_LibPart *libPart);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_LibPart_ShapePrims (ClientID			clientPtr,
																  Int32				libInd,
																  const API_Guid&	instanceElemGuid,
																  short				gdlContext,
																  ShapePrimsProc	*shapePrimsProc);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_LibPart_GetHotspots (ClientID				clientPtr,
																   Int32				libInd,
																   const API_Guid&		instanceElemGuid,
																   Int32*				nHotspots,
																   API_PrimHotspot***	hotspots);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_LibPart_Register (ClientID clientPtr, API_LibPart *libPart);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_LibPart_RegisterAll (ClientID clientPtr, GS::Array<API_LibPart>* libParts);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_LibPart_GetSectionList (ClientID				clientPtr,
																	  Int32					libInd,
																	  Int32					*nSection,
																	  API_LibPartSection	***sections);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_LibPart_GetDetails (ClientID				clientPtr,
																  Int32					libInd,
																  API_LibPartDetails	*details);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_LibPart_SetDetails_ParamDef (ClientID				clientPtr,
																		   const API_LibPart		*libPart,
																		   GSHandle					paramHdl,
																		   const API_LibPartDetails	*details);

/*@}*/

/* -- Element Manager --------------------- */

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_GetDefaults (ClientID clientPtr, API_Element *element, API_ElementMemo *memo);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_GetDefaultsExt (ClientID				clientPtr,
																	  API_Element			*element,
																	  API_ElementMemo		*memo,
																	  UInt32				nSubElems,
																	  API_SubElemMemoMask	*subElems);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_ChangeDefaults (ClientID			clientPtr,
																	  API_Element		*element,
																	  API_ElementMemo	*memo,
																	  const API_Element	*mask);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_ChangeDefaultsExt (ClientID				clientPtr,
																		 API_Element			*element,
																		 API_ElementMemo		*memo,
																		 const API_Element		*mask,
																		 UInt32					nSubElems,
																		 API_SubElemMemoMask	*subElems);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_GetElemList (ClientID				clientPtr,
																   API_ElemTypeID		typeID,
																   GS::Array<API_Guid>*	elemList,
																   API_ElemFilterFlags	filterBits = APIFilt_None,
																   API_ElemVariationID	variationID = APIVarId_Generic,
																   const API_Guid&		renovationFilterGuid = APINULLGuid);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_GetConnectedElements (ClientID				clientPtr,
																			const API_Guid&			guid,
																			API_ElemTypeID			connectedElemTypeID,
																			GS::Array<API_Guid>*	connectedElements,
																			API_ElemFilterFlags		filterBits = APIFilt_None,
																			API_ElemVariationID		variationID = APIVarId_Generic,
																			const API_Guid&		renovationFilterGuid = APINULLGuid);

__APIEXPORT bool		__ACENV_CALL	ACAPI_Element_Filter (ClientID				clientPtr,
															  const API_Guid&		guid,
															  API_ElemFilterFlags	filterBits,
															  API_ElemVariationID	variationID = APIVarId_Generic,
															  const API_Guid&		renovationFilterGuid = APINULLGuid);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_Get (ClientID		clientPtr,
														   API_Element	*element,
														   UInt32		mask = 0);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_GetHeader (ClientID		clientPtr,
														  		 API_Elem_Head	*elementHead,
																 UInt32			mask = 0);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_GetMemo	(ClientID					clientPtr,
																 const API_Guid&			guid,
																 API_ElementMemo*			memo,
																 UInt64						mask = APIMemoMask_All);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_ChangeMemo (ClientID				clientPtr,
																  API_Guid&				guid,
																  UInt64				mask,
											   					  const API_ElementMemo	*memo);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_GetRelations (ClientID			clientPtr,
																	const API_Guid&		guid,
																	API_ElemTypeID		otherID,
																	void				*relData);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_GetSyTran (ClientID clientPtr, const API_Elem_Head *elemHead, API_SyTran *syTran);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_GetHotspots (ClientID							clientPtr,
																   const API_Guid&					guid,
																   GS::Array<API_ElementHotspot>*	hotspotArray);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_Create (ClientID clientPtr, API_Element *element, API_ElementMemo *memo);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_CreateExt (ClientID				clientPtr,
																 API_Element			*element,
																 API_ElementMemo		*memo,
																 UInt32					nSubElems,
																 API_SubElemMemoMask	*subElems);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_ChangeParameters (ClientID				clientPtr,
																		API_Elem_Head			**elemHead,
																		Int32					nItem,
							   											const API_Element		*defPars,
																		const API_ElementMemo	*defMemo,
												   						const API_Element		*mask);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_Change (ClientID				clientPtr,
															  API_Element			*element,
															  const API_Element		*mask,
															  const API_ElementMemo	*memo,
															  UInt64				memoMask,
															  bool					withdel);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_ChangeMore (ClientID				clientPtr,
																  API_Elem_Head			*elemHead,
																  Int32					nItem,
							   									  const API_Element		*defPars,
																  const API_ElementMemo	*defMemo,
												   				  const API_Element		*mask,
																  UInt64				memoMask,
																  bool					withdel);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_ChangeExt (ClientID				clientPtr,
															  	 API_Element			*element,
																 const API_Element		*mask,
																 API_ElementMemo		*memo,
																 UInt64					memoMask,
																 UInt32					nSubElems,
																 API_SubElemMemoMask	*subElems,
																 bool					withdel,
																 Int32					subIndex);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_Delete (ClientID clientPtr, API_Elem_Head **elemHead, Int32 nItem);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_Edit (ClientID clientPtr, API_Neig **items, Int32 nItem, const API_EditPars *pars);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_Tool (ClientID		clientPtr,
															API_Elem_Head	**elemHead,
															Int32			nItem,
															API_ToolCmdID	typeID,
															void			*pars);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_Select (ClientID clientPtr, API_Neig **selNeig, Int32 nItem, bool add);


__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_GetUserData	(ClientID					clientPtr,
																	 API_Elem_Head				*elemHead,
																	 API_ElementUserData		*userData,
																	 UInt32						mask = 0);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_SetUserData	(ClientID					clientPtr,
																	 API_Elem_Head				*elemHead,
																	 const API_ElementUserData	*userData);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_DeleteUserData (ClientID clientPtr, API_Elem_Head *elemHead);

__APIEXPORT API_Guid	__ACENV_CALL	ACAPI_Element_UnIdToGuid (ClientID clientPtr, API_DatabaseUnId databaseUnId, UInt32 elemUnId);

__APIEXPORT UInt32		__ACENV_CALL	ACAPI_Element_GuidToUnId (ClientID clientPtr, API_Guid elemGuid, API_DatabaseUnId* databaseUnId);

__APIEXPORT API_Guid	__ACENV_CALL	ACAPI_Element_LinkIdToDrwGuid (ClientID clientPtr, Int32 linkId);

__APIEXPORT Int32		__ACENV_CALL	ACAPI_Element_DrwGuidToLinkId (ClientID clientPtr, API_Guid drwGuid);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_Link (ClientID	clientPtr,
															API_Guid	guid_linkFrom,
															API_Guid	guid_linkTo,
															GSFlags		linkFlags);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_Unlink (ClientID clientPtr, API_Guid guid_linkFrom, API_Guid guid_linkTo);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_GetLinks (ClientID	clientPtr,
																API_Guid	guid_linkFrom,
																API_Guid	***guid_linkTo,
																Int32 		*nLinks);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_GetLinkFlags (ClientID	clientPtr,
																	API_Guid	guid_linkFrom,
																	API_Guid	guid_linkTo,
																	GSFlags		*linkFlags);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_AttachObserver (ClientID clientPtr, API_Elem_Head *elemHead, GSFlags notifyFlags);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_DetachObserver (ClientID clientPtr, API_Elem_Head *elemHead);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_GetObservedElements (ClientID clientPtr, API_Elem_Head ***elemHead, Int32 *nElems);


__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_SolidLink_Create (ClientID				clientPtr,
																		API_Guid				guid_Target,
																		API_Guid				guid_Operator,
																		API_SolidOperationID	operation,
																		GSFlags					linkFlags);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_SolidLink_Remove (ClientID	clientPtr,
																		API_Guid	guid_Target,
																		API_Guid	guid_Operator);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_SolidLink_GetTime (ClientID		clientPtr,
																		 API_Guid		guid_Target,
																		 API_Guid		guid_Operator,
																		 GSTime			*linkTime,
																		 UInt32			*linkSubTime);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_SolidLink_GetOperation (ClientID				clientPtr,
																			  API_Guid				guid_Target,
																			  API_Guid				guid_Operator,
																			  API_SolidOperationID	*operation);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_SolidLink_GetFlags (ClientID	clientPtr,
																		  API_Guid	guid_Target,
																		  API_Guid	guid_Operator,
																		  GSFlags	*linkFlags);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_SolidLink_GetTargets (ClientID	clientPtr,
																			API_Guid	guid_Operator,
																			API_Guid	***guid_Targets,
																			Int32		*nLinks);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_SolidLink_GetOperators (ClientID	clientPtr,
																			  API_Guid	guid_Target,
																			  API_Guid	***guid_Operators,
																			  Int32		*nLinks);


__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_Get3DInfo			(ClientID					clientPtr,
																		 const API_Elem_Head&		elemHead,
																		 API_ElemInfo3D				*info3D);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_GetQuantities		(ClientID							clientPtr,
																		 API_Guid							elemGuid,
								  										 const API_QuantityPar				*params,
																		 API_Quantities						*quantities,
																		 const API_QuantitiesMask			*mask = NULL);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_GetMoreQuantities	(ClientID							clientPtr,
																		 const GS::Array<API_Guid>			*elemGuids,
								  										 const API_QuantityPar				*params,
																		 GS::Array<API_Quantities>			*quantities,
																		 const API_QuantitiesMask			*mask = NULL);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_GetSurfaceQuantities	(ClientID								clientPtr,
																			 const GS::Array<API_Guid>				*elemGuids,
								  											 const GS::Array<API_Guid>				*coverElemGuids,
																			 GS::Array<API_ElemPartSurfaceQuantity>	*quantities);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_GetProperties		(ClientID					clientPtr,
																		 const API_Elem_Head*		elemHead,
																		 API_PropertyRefType		***propRefs,
																		 Int32						*nProp);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_GetComponents		(ClientID					clientPtr,
																		 const API_Elem_Head*		elemHead,
																		 API_ComponentRefType		***compRefs,
																		 Int32						*nComp);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_GetDescriptors	(ClientID					clientPtr,
																		 const API_Elem_Head*		elemHead,
																		 API_DescriptorRefType		***descRef,
																		 Int32						*nDesc);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_ShapePrims		(ClientID				clientPtr,
																		 const API_Elem_Head&	elemHead,
																		 ShapePrimsProc			*shapePrimsProc);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_ShapePrimsExt (ClientID				clientPtr,
																	 const API_Elem_Head&	elemHead,
																	 ShapePrimsProc			*shapePrimsProc,
																	 API_ShapePrimsParams*	shapePrimsParams);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_GetLinkedProperties (ClientID				clientPtr,
																		   const API_Elem_Head	*head,
																		   bool					*criteria,
																		   Int32				*inviduallyLibInd,
																		   Int32				**critLibInds,
																		   Int32				*nCrits);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_SetLinkedProperties (ClientID				clientPtr,
																		   API_Elem_Head		*head,
																		   bool					criteria,
																		   Int32				inviduallyLibInd);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_GetCategoryValue (ClientID					clientPtr,
																		const API_Guid&				elemGuid,
																		const API_ElemCategory&		elemCategory,
																		API_ElemCategoryValue*		elemCategoryValue);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_GetCategoryValueDefault (ClientID						clientPtr,
																			   const API_ElemTypeID&		typeID,
																			   const API_ElemVariationID&	variationID,
																			   const API_ElemCategory&		elemCategory,
																			   API_ElemCategoryValue*		elemCategoryValue);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_SetCategoryValue (ClientID						clientPtr,
																		const API_Guid&					elemGuid,
																		const API_ElemCategory&			elemCategory,
																		const API_ElemCategoryValue&	elemCategoryValue);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_SetCategoryValueDefault (ClientID							clientPtr,
																			   const API_ElemTypeID&			typeID,
																			   const API_ElemVariationID&		variationID,
																			   const API_ElemCategory&			elemCategory,
																			   const API_ElemCategoryValue&		elemCategoryValue);

/* -- Element Composition ----------------- */

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_Trim_Elements			(ClientID					clientPtr,
																			 const GS::Array<API_Guid>&	guid_ElementsToTrim);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_Trim_ElementsWith		(ClientID					clientPtr,
																			 const GS::Array<API_Guid>&	guid_ElementsToTrim,
																			 const API_Guid&			guid_Element,
																			 API_TrimTypeID				trimType);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_Trim_Remove			(ClientID					clientPtr,
																			 const API_Guid&			guid_Element1,
																			 const API_Guid&			guid_Element2);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_Trim_GetTrimType		(ClientID					clientPtr,
																			 const API_Guid&			guid_Element1,
																			 const API_Guid&			guid_Element2,
																			 API_TrimTypeID*			trimType);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_Trim_GetTrimmedElements	(ClientID				clientPtr,
																				 const API_Guid&		guid_Element,
																				 GS::Array<API_Guid>*	guid_TrimmedElements);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_Trim_GetTrimmingElements	(ClientID				clientPtr,
																				 const API_Guid&		guid_Element,
																				 GS::Array<API_Guid>*	guid_TrimmingElements);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_Merge_Elements			(ClientID					clientPtr,
																				 const GS::Array<API_Guid>&	guid_ElementsToMerge);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_Merge_GetMergedElements	(ClientID				clientPtr,
																				 const API_Guid&		guid_Element,
																				 GS::Array<API_Guid>*	guid_MergedElements);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_Merge_Remove				(ClientID			clientPtr,
																				 const API_Guid&	guid_Element1,
																				 const API_Guid&	guid_Element2);

/* -- IFC --------------------------------- */

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_GetIFCIdentifier		(const API_Guid&				elementID,
																			 API_Guid&						archicadIFCID,
																			 API_Guid&						externalIFCID);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_GetElemListByIFCIdentifier	(const API_Guid*				archicadIFCID,
																					 const API_Guid*				externalIFCID,
																					 GS::Array<API_Guid>&			elements);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_GetIFCType			(ClientID						clientPtr,
																			const API_Guid&					elementID,
																			GS::UniString*					ifcType,
																			GS::UniString*					typeObjectIFCType);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_GetIFCProperties		(ClientID						clientPtr,
																			 const API_Guid&				guid_Element,
																			 bool							storedOnly,
																			 GS::Array<API_IFCProperty>*	properties);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_SetIFCProperty		(ClientID						clientPtr,
																			 const API_Guid&				guid_Element,
																			 const API_IFCProperty&			property);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_RemoveIFCProperty		(ClientID						clientPtr,
																			 const API_Guid&				guid_Element,
																			 const API_IFCProperty&			property);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_GetIFCAttributes		(ClientID						clientPtr,
																			 const API_Guid&				guid_Element,
																			 bool							storedOnly,
																			 GS::Array<API_IFCAttribute>*	attributes);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_SetIFCAttribute		(ClientID						clientPtr,
																			 const API_Guid&				guid_Element,
																			 const API_IFCAttribute&		attribute);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_GetIFCClassificationReferences	(ClientID									clientPtr,
																						 const API_Guid&							guid_Element,
																						 bool										storedOnly,
																						 GS::Array<API_IFCClassificationReference>*	classificationReferences);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_SetIFCClassificationReference		(ClientID								clientPtr,
																						 const API_Guid&						guid_Element,
																						 const API_IFCClassificationReference&	classificationReference);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_RemoveIFCClassificationReference	(ClientID								clientPtr,
																						 const API_Guid&						guid_Element,
																						 const API_IFCClassificationReference&	classificationReference);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_GetIFCPropertyValuePrimitiveType	(ClientID								clientPtr,
																						 const GS::UniString&					valueType,
																						 API_IFCPropertyValuePrimitiveType*		primitiveType);


/* -- Body Editing ------------------------ */

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Body_Create			(ClientID							clientPtr,
																	 const Modeler::Body*				body,
																	 const API_MaterialOverrideType*	bodyMaterialMapTable,
																	 void**								bodyData);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Body_Finish			(ClientID						clientPtr,
																	 void*							bodyData,
																	 Modeler::Body**				body,
																	 API_MaterialOverrideType**		bodyMaterialMapTable);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Body_Dispose			(ClientID				clientPtr,
																	 void**					bodyData);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Body_AddVertex		(ClientID				clientPtr,
																	 void*					bodyData,
																	 const API_Coord3D&		coord,
																	 UInt32&				index);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Body_AddEdge			(ClientID				clientPtr,
																	 void*					bodyData,
																	 const UInt32&			vertex1,
																	 const UInt32&			vertex2,
																	 Int32&					index);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Body_AddPolyNormal	(ClientID				clientPtr,
																	 void*					bodyData,
																	 const API_Vector3D&	normal,
																	 Int32&					index);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Body_AddPolygon		(ClientID					clientPtr,
																	 void*						bodyData,
																	 Int32*						edges,
																	 Int32						nEdges,
																	 Int32						polyNormal,
																	 API_MaterialOverrideType	material,
																	 UInt32&					index);


/* -- PolyRoof Decomposer ----------------- */

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Element_Decompose	(ClientID							clientPtr,
																 const API_Guid&					elemGuid,
																 API_ElementDecomposerInterface*	elementDecomposer);

/* -- ElementSet Manager ------------------ */

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_ElementSet_Create (ClientID				clientPtr,
																 GS::Array<API_Guid>	*guids,
																 const API_UserData		*userData,
																 API_Guid				*setGuid);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_ElementSet_Delete (ClientID clientPtr, API_Guid setGuid);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_ElementSet_GetData (ClientID				clientPtr,
																  API_Guid				setGuid,
																  GS::Array<API_Guid>	*elemGuids,
																  API_UserData			*data);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_ElementSet_Identify (ClientID clientPtr, API_Guid elemGuid, GS::Array<API_Guid> *setGuids);


/* -- Group Manager ------------------ */

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_ElementGroup_Create (ClientID						clientPtr,
																   const GS::Array<API_Guid>&	elemGuids,
																   API_Guid*					groupGuid = NULL,
																   const API_Guid*				parentGroupGuid = NULL);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_ElementGroup_GetGroup (ClientID				clientPtr,
																	 const API_Guid&		elemGuid,
																	 API_Guid*				groupGuid);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_ElementGroup_GetRootGroup (ClientID				clientPtr,
																		 const API_Guid&		elemGuid,
																		 API_Guid*				rootGroupGuid);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_ElementGroup_GetGroupedElems (ClientID				clientPtr,
																			const API_Guid&			groupGuid,
																			GS::Array<API_Guid>*	elemGuids);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_ElementGroup_GetAllGroupedElems (ClientID				clientPtr,
																			   const API_Guid&		groupGuid,
																			   GS::Array<API_Guid>*	elemGuids);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_ElementGroup_GetUserData (ClientID			clientPtr,
																		const API_Guid&		groupGuid,
																		API_UserData*		userData);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_ElementGroup_SetUserData (ClientID			clientPtr,
																		const API_Guid&		groupGuid,
																		const API_UserData*	userData);

/* -- Selection Manager ------------------- */

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Selection_Get (ClientID				clientPtr,
															 API_SelectionInfo*		selectionInfo,
															 API_Neig***			selNeigs,
															 bool					onlyEditable,
															 bool					ignorePartialSelection = true,
															 API_SelRelativePosID	relativePosToMarquee = API_InsidePartially);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Selection_SetMarquee (ClientID clientPtr, API_SelectionInfo *selectionInfo);


/* -- 3D Manager -------------------------- */

__APIEXPORT GSErrCode			__ACENV_CALL	ACAPI_3D_GetNum (ClientID clientPtr, API_3DTypeID typeID, Int32 *count);

__APIEXPORT GSErrCode			__ACENV_CALL	ACAPI_3D_GetComponent (ClientID clientPtr, API_Component3D *component);

__APIEXPORT GSErrCode			__ACENV_CALL	ACAPI_3D_DecomposePgon (ClientID clientPtr, Int32 ipgon, Int32 ***cpoly);

__APIEXPORT GSErrCode			__ACENV_CALL	ACAPI_3D_CreateSight (ClientID clientPtr, void** newSightPtr);

__APIEXPORT GSErrCode			__ACENV_CALL	ACAPI_3D_SelectSight (ClientID clientPtr, void* sight, void** oldSightPtr);

__APIEXPORT GSErrCode			__ACENV_CALL	ACAPI_3D_DeleteSight (ClientID clientPtr, void* sight);

__APIEXPORT GSErrCode			__ACENV_CALL	ACAPI_3D_GetCurrentWindowSight (ClientID clientPtr, void** sightPtr);

__APIEXPORT GSErrCode			__ACENV_CALL	ACAPI_3D_GetCutPolygonInfo (ClientID 								clientPtr,
																			Int32 									bodyIndex,
																			const API_Plane3D& 						cutPlane,
																			GS::Array<Geometry::MultiPolygon2D>* 	resPolygons = NULL,
																			double* 								area		= NULL);
/* -- ListData Manager -------------------- */

__APIEXPORT Int32		__ACENV_CALL	ACAPI_ListData_GetSetNum (ClientID clientPtr);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_ListData_GetSet (ClientID clientPtr, API_ListDataSetType *listdataSet);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_ListData_GetNum (ClientID clientPtr, Int32 setIndex, API_ListDataID typeID, Int32 *count);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_ListData_Get (ClientID clientPtr, API_ListData *listdata);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_ListData_Search (ClientID clientPtr, API_ListData *listdata);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_ListData_GetLocal (ClientID clientPtr, Int32 libIndex, const API_Elem_Head *elemHead, API_ListData *listdata);


/* -- Communication Manager ------- */

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Command_Test	(ClientID			clientPtr,
															 const API_ModulID*	mdid,
															 GSType				cmdID,
															 Int32				cmdVersion);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Command_Call	(ClientID			clientPtr,
															 const API_ModulID*	mdid,
															 GSType				cmdID,
															 Int32				cmdVersion,
															 GSHandle			params,
															 GSPtr				resultData,
															 bool				silentMode);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Command_ExternalCall	(ClientID					clientPtr,
																	 const IO::Location*		projectFileLoc,
																	 const API_ModulID*			mdid,
																	 GSType						cmdID,
																	 Int32						cmdVersion,
																	 GSHandle					params,
																	 bool						silentMode,
																	 APIExternalCallBackProc*	externalCallbackProc);

__APIEXPORT GSErrCode __ACENV_CALL		ACAPI_Command_CallFromEventLoop (ClientID					clientPtr,
																		 const API_ModulID*			mdid,
																		 GSType						cmdID,
																		 Int32						cmdVersion,
																		 GSHandle					paramsHandle,
																		 bool						silentMode,
																		 APICommandCallBackProc*	callbackProc);

/* -- ModulData Manager ---------------- */

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_ModulData_Store (ClientID clientPtr, const API_ModulData* info, const GS::UniString& modulName = "");

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_ModulData_GetInfo (ClientID clientPtr, API_ModulData* info, const GS::UniString& modulName = "");

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_ModulData_Get (ClientID clientPtr, API_ModulData* info, const GS::UniString& modulName = "");

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_ModulData_GetList (ClientID clientPtr, GS::Array<GS::UniString>* modulNameList);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_ModulData_Delete (ClientID clientPtr, const GS::UniString& modulName = "");

/* -- AddOnObject Manager ---------------- */

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_AddOnObject_CreateObject (ClientID clientPtr, const GS::UniString& objectName, const GSHandle& content, API_Guid* objectGuid);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_AddOnObject_GetObjectList (ClientID clientPtr, GS::Array<API_Guid>* objects);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_AddOnObject_GetObjectContent (ClientID clientPtr, const API_Guid& objectGuid, GS::UniString* objectName, GSHandle* objectContent);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_AddOnObject_ModifyObject (ClientID clientPtr, const API_Guid& objectGuid, const GS::UniString* newObjectName, const GSHandle* newObjectContent);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_AddOnObject_DeleteObject (ClientID clientPtr, const API_Guid& objectGuid);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_AddOnObject_GetObjectGuidFromName (ClientID clientPtr, const GS::UniString& name, API_Guid* objectGuid);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_AddOnObject_ReserveObjects (ClientID clientPtr, const GS::Array<API_Guid>& objectGuids, GS::HashTable<API_Guid, short>* conflicts);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_AddOnObject_ReleaseObjects (ClientID clientPtr, const GS::Array<API_Guid>& objectGuids);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_AddOnObject_RequestObjects (ClientID clientPtr, const GS::Array<API_Guid>& objectGuids, const GS::UniString& mailText);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_AddOnObject_GrantObjects (ClientID clientPtr, const GS::Array<API_Guid>& objectGuids, short toUserId, const GS::UniString* grantMailText);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_AddOnObject_GetTeamworkOwnerId (ClientID clientPtr, const API_Guid& objectGuid, short* ownerId);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_AddOnObject_ExistsObject (ClientID clientPtr, const API_Guid& objectGuid, bool* objectExists);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_AddOnObject_SetUIProperties (ClientID clientPtr, const API_Guid* objectGuid, const GS::UniString* name, const GS::UniString& localisedName, short dialogMenuStringId);

/* -- Notification Manager ---------------- */

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Notify_CatchProjectEvent		(ClientID						clientPtr,
																			 GSFlags						eventTypes,
																			 APIProjectEventHandlerProc		*handlerProc);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Notify_CatchViewEvent			(ClientID						clientPtr,
																			 GSFlags						eventTypes,
																			 API_NavigatorMapID				mapId,
																			 APIViewEventHandlerProc		*handlerProc);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Notify_CatchToolChange		(ClientID clientPtr, APIToolChangeHandlerProc *handlerProc);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Notify_CatchSelectionChange	(ClientID clientPtr, APISelectionChangeHandlerProc *handlerProc);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Notify_CatchChangeDefaults	(ClientID						clientPtr,
																			 const API_ToolBoxItem			*elemType,
																			 APIDefaultsChangeHandlerProc	*handlerProc);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Notify_CatchNewElement		(ClientID						clientPtr,
																			 const API_ToolBoxItem			*elemType,
																			 APIElementEventHandlerProc		*handlerProc);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Notify_InstallElementObserver	(ClientID clientPtr, APIElementEventHandlerProc *handlerProc);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Notify_GetTranParams			(ClientID clientPtr, API_ActTranPars *actTranPars);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Notify_GetParentElement		(ClientID				clientPtr,
																			 API_Element*			element,
																			 API_ElementMemo*		memo,
																			 UInt64					mask,
																			 API_ElementUserData*	userData);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Notify_CatchElementReservationChange	(ClientID							clientPtr,
																					 APIReservationChangeHandlerProc*	handlerProc,
																					 const GS::HashSet<API_Guid>*		filterElementsInterestedOnly = NULL);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Notify_CatchLockableReservationChange	(ClientID							clientPtr,
																					 APILockChangeHandlerProc*			handlerProc,
																					 const GS::HashSet<API_Guid>*		filterLockablesInterestedOnly = NULL);


/* -- Miscellaneous ----------------------- */

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_ActivatePropertyHandler (ClientID clientPtr, short strResID, bool active);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Favorite_GetDefaults (ClientID			clientPtr,
																	API_Element			*element,
																	API_ElementMemo		*memo,
																	API_Element			*elementMarker,
																	API_ElementMemo		*memoMarker,
																	GS::UniString		*name,
																	short				*index);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Favorite_Create (ClientID				clientPtr,
															   API_Element			*element,
															   API_ElementMemo		*memo,
															   API_Element			*elementMarker,
															   API_ElementMemo		*memoMarker,
															   const GS::UniString	*name);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Favorite_Delete (ClientID clientPtr, const GS::UniString* name, short *index);

__APIEXPORT GSErrCode	__ACENV_CALL	ACAPI_Favorite_GetNum (ClientID					clientPtr,
															   API_ElemTypeID			typeID,
															   API_ElemVariationID		variationID,
															   short					*count,
															   GS::Array<short>			*indexes,
															   GS::Array<GS::UniString>	*names);


/* -- Teamwork Control -------------------- */

__APIEXPORT bool				__ACENV_CALL	ACAPI_TeamworkControl_HasConnection (ClientID clientPtr);

__APIEXPORT bool				__ACENV_CALL	ACAPI_TeamworkControl_IsOnline (ClientID clientPtr);

__APIEXPORT bool				__ACENV_CALL	ACAPI_TeamworkControl_IsServerLibPart (ClientID clientPtr, const IO::Location& location);

__APIEXPORT bool				__ACENV_CALL	ACAPI_TeamworkControl_HasCreateRight (ClientID clientPtr, const API_Guid& objectId);

__APIEXPORT bool				__ACENV_CALL	ACAPI_TeamworkControl_HasDeleteModifyRight (ClientID clientPtr, const API_Guid& objectId);

__APIEXPORT API_LockableStatus	__ACENV_CALL	ACAPI_TeamworkControl_GetLockableStatus (ClientID clientPtr, const API_Guid& objectId, GS::PagedArray<GS::UniString>* conflicts = NULL);

__APIEXPORT API_Guid			__ACENV_CALL	ACAPI_TeamworkControl_FindLockableObjectSet (ClientID clientPtr, const GS::UniString& objectSetName);

__APIEXPORT GSErrCode			__ACENV_CALL	ACAPI_TeamworkControl_ReserveLockable (ClientID clientPtr, const API_Guid& objectId, GS::PagedArray<GS::UniString>* conflicts = NULL);

__APIEXPORT GSErrCode			__ACENV_CALL	ACAPI_TeamworkControl_ReleaseLockable (ClientID clientPtr, const API_Guid& objectId);

__APIEXPORT GSErrCode			__ACENV_CALL	ACAPI_TeamworkControl_ReserveElements (ClientID clientPtr, const GS::PagedArray<API_Guid>& objectIdArray, GS::HashTable<API_Guid, short>* conflicts = NULL);

__APIEXPORT GSErrCode			__ACENV_CALL	ACAPI_TeamworkControl_ReleaseElements (ClientID clientPtr, const GS::PagedArray<API_Guid>& objectIdArray);

__APIEXPORT GSErrCode			__ACENV_CALL	ACAPI_TeamworkControl_ReserveHotlinkCacheManagement (ClientID clientPtr, short* conflict);

__APIEXPORT GSErrCode			__ACENV_CALL	ACAPI_TeamworkControl_ReleaseHotlinkCacheManagement (ClientID clientPtr);

__APIEXPORT GSErrCode			__ACENV_CALL	ACAPI_TeamworkControl_GetHotlinkCacheManagementOwner (ClientID clientPtr, short* owner);

/* --> ] */
#endif


// ---------------------------------------------------------------------------------------------------------------------

#if defined (_MSC_VER)
	#pragma pack(pop)
#endif

#ifdef __cplusplus
}
#endif

#endif
