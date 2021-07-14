#ifndef	__SUPPORTING_POST__PLACER__
#define __SUPPORTING_POST__PLACER__

#include "MaxBIM.hpp"

namespace SupportingPostPlacerDG {
	// ���̾�α� �׸� �ε���
	enum	idxItems_1_forPERISupportingPostPlacer {
		LABEL_VPOST = 3,
		LABEL_HPOST,

		LABEL_TOTAL_HEIGHT,
		EDITCONTROL_TOTAL_HEIGHT,
		LABEL_REMAIN_HEIGHT,
		EDITCONTROL_REMAIN_HEIGHT,

		CHECKBOX_CROSSHEAD,

		CHECKBOX_VPOST1,
		LABEL_VPOST1_NOMINAL,
		POPUP_VPOST1_NOMINAL,
		LABEL_VPOST1_HEIGHT,
		EDITCONTROL_VPOST1_HEIGHT,

		CHECKBOX_VPOST2,
		LABEL_VPOST2_NOMINAL,
		POPUP_VPOST2_NOMINAL,
		LABEL_VPOST2_HEIGHT,
		EDITCONTROL_VPOST2_HEIGHT,

		SEPARATOR_VPOST_L,
		SEPARATOR_VPOST_R,
		CHECKBOX_HPOST,
		SEPARATOR_HPOST_UP,
		SEPARATOR_HPOST_DOWN,

		SEPARATOR_HPOST_PLAN,

		LABEL_PLAN_WIDTH,
		EDITCONTROL_PLAN_WIDTH,
		LABEL_PLAN_DEPTH,
		EDITCONTROL_PLAN_DEPTH,

		LABEL_WIDTH_NORTH,
		POPUP_WIDTH_NORTH,
		LABEL_WIDTH_WEST,
		POPUP_WIDTH_WEST,
		LABEL_WIDTH_EAST,
		POPUP_WIDTH_EAST,
		LABEL_WIDTH_SOUTH,
		POPUP_WIDTH_SOUTH,

		ICON_LAYER,
		LABEL_LAYER_SETTINGS,
		CHECKBOX_LAYER_COUPLING,
		LABEL_LAYER_VPOST,
		LABEL_LAYER_HPOST,

		USERCONTROL_LAYER_VPOST,
		USERCONTROL_LAYER_HPOST
	};
}

// ���� ���� ����
struct InfoMorphForSupportingPost
{
	API_Guid	guid;		// ������ GUID

	double	width;			// ���� ����
	double	depth;			// ���� ����
	double	height;			// ���� ����

	double	ang;			// ȸ�� ���� (����: Degree, ȸ����: Z��)

	double	leftBottomX;	// ���ϴ� ��ǥ X
	double	leftBottomY;	// ���ϴ� ��ǥ Y
	double	leftBottomZ;	// ���ϴ� ��ǥ Z

	double	rightTopX;		// ���� ��ǥ X
	double	rightTopY;		// ���� ��ǥ Y
	double	rightTopZ;		// ���� ��ǥ Z

	API_Coord3D	points [8];	// 8���� ������ ��ǥ
};

GSErrCode	placePERIPost (void);		// ������ ������ü ������ ������� PERI ���ٸ��� ��ġ��
short DGCALLBACK PERISupportingPostPlacerHandler1 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// ������ �ܼ� (1/2��, ���̰� 6���� �ʰ��Ǹ� 2�� ������ ��), �������� �԰�/����, ������ ����(��, ���̰� 3500 �̻��̸� �߰��� ���� ������ ��), ������ �ʺ�, ũ�ν���� ����, ������/������ ���̾ ����

#endif