#ifndef	__FACILITY__
#define __FACILITY__

#include "MaxBIM.hpp"

namespace FacilityDG {
	// ���� ���� ����
	enum	circularBubblePos {
		UP = 1,
		DOWN,
		LEFT,
		RIGHT
	};

	enum	dgCircularBubbleSettings {
		LABEL_DIAMETER = 3,
		EDITCONTROL_DIAMETER,
		LABEL_LETTER_SIZE,
		EDITCONTROL_LETTER_SIZE,
		LABEL_WITHDRAWAL_LENGTH,
		EDITCONTROL_WITHDRAWAL_LENGTH,
		LABEL_BUBBLE_POS,
		POPUPCONTROL_BUBBLE_POS,

		ICON_LAYER,
		LABEL_LAYER_SETTINGS,
		LABEL_LAYER_CIRCULAR_BUBBLE,
		USERCONTROL_LAYER_CIRCULAR_BUBBLE
	};

	enum	dgCameraManager {
		BUTTON_LOAD = 3,
		BUTTON_CLOSE,
		LISTVIEW_CAMERA_POS_NAME,
		EDITCONTROL_CAMERA_POS_NAME
	};
}

struct CircularBubble
{
	short	floorInd;		// �� �ε���

	short	layerInd;		// ���̾� �ε���
	short	pos;			// ��ġ (UP/DOWN/LEFT/RIGHT)
	double	szBubbleDia;	// ���� ����
	double	szFont;			// ���� ũ��
	double	lenWithdrawal;	// ���⼱ ���� (������)
};

GSErrCode	select3DQuality (void);					// 3D ǰ��/�ӵ� �����ϱ�
GSErrCode	attach3DLabelOnZone (void);				// ������ 3D �� ���̱�
short DGCALLBACK selectLayerHandler (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// [���̾�α� �ڽ�] ���̾� ����

GSErrCode	attachBubbleOnCurrentFloorPlan (void);	// ���� ��鵵�� ���̺����� ���� �ڵ� ��ġ
short DGCALLBACK setBubbleHandler (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// [���̾�α� �ڽ�] ���� ���� ����
GSErrCode	saveDialogStatus_bubble (CircularBubble	*cbInfo);		// ���� ���� ���� ���� ����
GSErrCode	loadDialogStatus_bubble (CircularBubble	*cbInfo);		// ���� ���� ���� ���� �ε�

GSErrCode	manageCameraInfo (void);				// ī�޶� ��ġ �����ϱ�/�ҷ�����
short DGCALLBACK cameraPosManagerHandler (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// [���̾�α� �ڽ�] ī�޶� ��ġ �����ϱ�

#endif