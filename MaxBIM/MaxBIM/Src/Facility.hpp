#ifndef	__FACILITY__
#define __FACILITY__

#include "MaxBIM.hpp"

GSErrCode	select3DQuality (void);			// 3D ǰ��/�ӵ� �����ϱ�
GSErrCode	attach3DLabelOnZone (void);		// ������ 3D �� ���̱�
short DGCALLBACK selectLayerHandler (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// [���̾�α� �ڽ�] ���̾� ����

#endif