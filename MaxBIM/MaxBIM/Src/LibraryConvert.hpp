#ifndef	__LIBRARY_CONVERT__
#define __LIBRARY_CONVERT__

#include "MaxBIM.hpp"

namespace libraryConvertDG {
	// ���̾�α� �׸� �ε���
	enum	idxItems_1_forLibraryConvert {
		ICON_LAYER = 3,
		LABEL_LAYER_SETTINGS,

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
	};
}

GSErrCode	convertVirtualTCO (void);		// ��� ���� ������(TCO: Temporary Construction Object)�� ���� ������� ��ȯ��
GSErrCode	placeTableformOnWall_portrait (WallTableform params);	// ���̺���(��) ��ġ (�������)
GSErrCode	placeTableformOnWall_landscape (WallTableform params);	// ���̺���(��) ��ġ (��������)
API_Guid	placeTableformOnSlabBottom (SlabTableform params);	// ���̺���(������) ��ġ
API_Guid	placeEuroform (Euroform params);					// ������ ��ġ
API_Guid	placeSteelform (Euroform params);					// ��ƿ�� ��ġ
API_Guid	placePlywood (Plywood params);						// ���� ��ġ
API_Guid	placeFillersp (FillerSpacer params);				// �ٷ������̼� ��ġ
API_Guid	placeOutcornerAngle (OutcornerAngle params);		// �ƿ��ڳʾޱ� ��ġ
API_Guid	placeOutcornerPanel (OutcornerPanel params);		// �ƿ��ڳ��ǳ� ��ġ
API_Guid	placeIncornerPanel (IncornerPanel params);			// ���ڳ��ǳ� ��ġ

API_Guid	placeProfile (KSProfile params);					// KS�������� ��ġ
API_Guid	placeFittings (MetalFittingsWithRectWasher params);	// ����ö�� (�簢�ͼ�Ȱ��) ��ġ

API_Guid	placeSqrPipe (SquarePipe params);					// ��� ������ ��ġ
API_Guid	placePinbolt (PinBoltSet params);					// �ɺ�Ʈ ��Ʈ ��ġ
API_Guid	placeWalltie (WallTie params);						// ��ü Ÿ�� ��ġ
API_Guid	placeHeadpiece (HeadpieceOfPushPullProps params);	// ����ǽ� ��ġ (���� ����: Ÿ�� A)
API_Guid	placeHeadpiece2 (HeadpieceOfPushPullProps params);	// ����ǽ� ��ġ (���� ����: Ÿ�� B)

short DGCALLBACK convertVirtualTCOHandler1 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// ��ü�� ���̾ �����ϱ� ���� ���̾�α�

#endif