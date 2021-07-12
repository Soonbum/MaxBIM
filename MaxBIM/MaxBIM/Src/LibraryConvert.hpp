#ifndef	__LIBRARY_CONVERT__
#define __LIBRARY_CONVERT__

#include "MaxBIM.hpp"

namespace libraryConvertDG {
	// 다이얼로그 항목 인덱스
	enum	idxItems_1_forLibraryConvert {
		ICON_LAYER = 3,
		LABEL_LAYER_SETTINGS,
		CHECKBOX_LAYER_COUPLING,

		LABEL_LAYER_SLABTABLEFORM,
		LABEL_LAYER_PROFILE,
		LABEL_LAYER_EUROFORM,
		LABEL_LAYER_RECTPIPE,
		LABEL_LAYER_PINBOLT,
		LABEL_LAYER_WALLTIE,
		LABEL_LAYER_JOIN,
		LABEL_LAYER_HEADPIECE,
		LABEL_LAYER_STEELFORM,
		LABEL_LAYER_PLYWOOD,
		LABEL_LAYER_FILLERSP,
		LABEL_LAYER_OUTCORNER_ANGLE,
		LABEL_LAYER_OUTCORNER_PANEL,
		LABEL_LAYER_INCORNER_PANEL,
		LABEL_LAYER_RECTPIPE_HANGER,
		LABEL_LAYER_EUROFORM_HOOK,
		LABEL_LAYER_HIDDEN,

		USERCONTROL_LAYER_SLABTABLEFORM,
		USERCONTROL_LAYER_PROFILE,
		USERCONTROL_LAYER_EUROFORM,
		USERCONTROL_LAYER_RECTPIPE,
		USERCONTROL_LAYER_PINBOLT,
		USERCONTROL_LAYER_WALLTIE,
		USERCONTROL_LAYER_JOIN,
		USERCONTROL_LAYER_HEADPIECE,
		USERCONTROL_LAYER_STEELFORM,
		USERCONTROL_LAYER_PLYWOOD,
		USERCONTROL_LAYER_FILLERSP,
		USERCONTROL_LAYER_OUTCORNER_ANGLE,
		USERCONTROL_LAYER_OUTCORNER_PANEL,
		USERCONTROL_LAYER_INCORNER_PANEL,
		USERCONTROL_LAYER_RECTPIPE_HANGER,
		USERCONTROL_LAYER_EUROFORM_HOOK,
		USERCONTROL_LAYER_HIDDEN
	};
}

GSErrCode	convertVirtualTCO (void);		// 모든 가상 가설재(TCO: Temporary Construction Object)를 실제 가설재로 변환함
GSErrCode	placeTableformOnWall_portrait_Type1 (WallTableform params);		// 테이블폼(벽) 배치 (벽세우기) 타입1
GSErrCode	placeTableformOnWall_landscape_Type1 (WallTableform params);	// 테이블폼(벽) 배치 (벽눕히기) 타입1
GSErrCode	placeTableformOnWall_portrait_Type2 (WallTableform params);		// 테이블폼(벽) 배치 (벽세우기) 타입2
GSErrCode	placeTableformOnWall_landscape_Type2 (WallTableform params);	// 테이블폼(벽) 배치 (벽눕히기) 타입2
API_Guid	placeTableformOnSlabBottom (SlabTableform params);		// 테이블폼(슬래브) 배치
API_Guid	placeEuroform (Euroform params);						// 유로폼 배치
API_Guid	placeSteelform (Euroform params);						// 스틸폼 배치
API_Guid	placePlywood (Plywood params);							// 합판 배치
API_Guid	placeFillersp (FillerSpacer params);					// 휠러스페이서 배치
API_Guid	placeOutcornerAngle (OutcornerAngle params);			// 아웃코너앵글 배치
API_Guid	placeOutcornerPanel (OutcornerPanel params);			// 아웃코너판넬 배치
API_Guid	placeIncornerPanel (IncornerPanel params);				// 인코너판넬 배치

API_Guid	placeProfile (KSProfile params);						// KS프로파일 배치
API_Guid	placeFittings (MetalFittingsWithRectWasher params);		// 결합철물 (사각와셔활용) 배치

API_Guid	placeSqrPipe (SquarePipe params);						// 비계 파이프 배치
API_Guid	placePinbolt (PinBoltSet params);						// 핀볼트 세트 배치
API_Guid	placeWalltie (WallTie params);							// 벽체 타이 배치
API_Guid	placeHeadpiece_ver (HeadpieceOfPushPullProps params);	// 헤드피스 배치 (세로 방향: 타입 A)
API_Guid	placeHeadpiece_hor (HeadpieceOfPushPullProps params);	// 헤드피스 배치 (가로 방향: 타입 B)

API_Guid	placeFittings (MetalFittings params);						// 사각파이프 연결철물 배치
API_Guid	placeJointHeadpeace_ver (HeadpieceOfPushPullProps params);	// 빔조인트용 Push-Pull Props 배치 (세로 방향: 타입 A)
API_Guid	placeJointHeadpeace_hor (HeadpieceOfPushPullProps params);	// 빔조인트용 Push-Pull Props 배치 (가로 방향: 타입 B)
API_Guid	placeEuroformHook (EuroformHook params);					// 유로폼 후크 배치
API_Guid	placeRectpipeHanger (RectPipeHanger params);				// 각파이프 행거 배치
API_Guid	placeHole (API_Guid guid_Target, Cylinder operator_Object);	// 타공을 위한 기둥 객체를 배치하고 숨김, "원통 19" 객체를 이용함

short DGCALLBACK convertVirtualTCOHandler1 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// 객체의 레이어를 선택하기 위한 다이얼로그

#endif