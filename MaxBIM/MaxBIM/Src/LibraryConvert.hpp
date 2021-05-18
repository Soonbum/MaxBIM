#ifndef	__LIBRARY_CONVERT__
#define __LIBRARY_CONVERT__

#include "MaxBIM.hpp"

GSErrCode	convertVirtualTCO (void);		// ��� ���� ������(TCO: Temporary Construction Object)�� ���� ������� ��ȯ��
GSErrCode	placeTableformOnWall (WallTableform params);		// ���̺���(��) ��ġ
GSErrCode	placeTableformOnSlabBottom (SlabTableform params);	// ���̺���(������) ��ġ
GSErrCode	placeEuroform (Euroform params);					// ������/��ƿ�� ��ġ
GSErrCode	placePlywood (Plywood params);						// ���� ��ġ
GSErrCode	placeFillersp (FillerSpacer params);				// �ٷ������̼� ��ġ
GSErrCode	placeOutcornerAngle (OutcornerAngle params);		// �ƿ��ڳʾޱ� ��ġ
GSErrCode	placeOutcornerPanel (OutcornerPanel params);		// �ƿ��ڳ��ǳ� ��ġ
GSErrCode	placeIncornerPanel (IncornerPanel params);			// ���ڳ��ǳ� ��ġ

#endif