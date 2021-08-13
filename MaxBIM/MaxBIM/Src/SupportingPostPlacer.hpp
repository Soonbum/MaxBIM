#ifndef	__SUPPORTING_POST__PLACER__
#define __SUPPORTING_POST__PLACER__

#include "MaxBIM.hpp"

namespace SupportingPostPlacerDG {
	// ���̾�α� �׸� �ε���
	enum	idxItems_1_forPERISupportingPostPlacer {
		LABEL_TYPE = 3,
		POPUP_TYPE,

		LABEL_SIDE_VIEW,
		LABEL_PLAN_VIEW,

		LABEL_UPWARD,
		SEPARATOR_UPWARD_BORDER,
		SEPARATOR_TIMBER,
		LABEL_TIMBER,
		SEPARATOR_CROSSHEAD,
		SEPARATOR_VERTICAL_2ND,
		LABEL_VERTICAL_2ND,
		SEPARATOR_VERTICAL_1ST,
		LABEL_VERTICAL_1ST,
		SEPARATOR_DOWNWARD_BORDER,
		LABEL_DOWNWARD,

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

		LABEL_TIMBER_HEIGHT,
		LABEL_CROSSHEAD_HEIGHT,

		ICON_LAYER,
		LABEL_LAYER_SETTINGS,
		CHECKBOX_LAYER_COUPLING,
		LABEL_LAYER_SUPPORT,
		LABEL_LAYER_VPOST,
		LABEL_LAYER_HPOST,
		LABEL_LAYER_TIMBER,
		LABEL_LAYER_GIRDER,
		LABEL_LAYER_BEAM_BRACKET,
		LABEL_LAYER_YOKE,

		USERCONTROL_LAYER_SUPPORT,
		USERCONTROL_LAYER_VPOST,
		USERCONTROL_LAYER_HPOST,
		USERCONTROL_LAYER_TIMBER,
		USERCONTROL_LAYER_GIRDER,
		USERCONTROL_LAYER_BEAM_BRACKET,
		USERCONTROL_LAYER_YOKE,

		SEPARATOR_CENTER,

		LABEL_TOTAL_WIDTH,
		EDITCONTROL_TOTAL_WIDTH,
		LABEL_EXPLANATION,

		LABEL_TOTAL_LENGTH,
		EDITCONTROL_TOTAL_LENGTH,
		LABEL_REMAIN_LENGTH,
		EDITCONTROL_REMAIN_LENGTH
	};
}

// ���� ���� ����
struct InfoMorphForSupportingPost
{
	API_Guid	guid;		// ������ GUID
	short		floorInd;	// �� �ε���

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

// PERI ���ٸ� ��ġ ���� ����
class PERISupportingPostPlacementInfo
{
public:
	bool	bFlipped;				// ����/���� ���� ������ �ڹٲ�� �ִ°�?
	double	width;					// ���� ���� ����
	double	depth;					// ���� ���� ����

	// ... �Ʒ� ������ ��
	char	nomVPost1 [16];			// ������ 1�� �԰� - GDL�� ������ �԰ݰ� ����
	double	heightVPost1;			// ������ 1�� ����

	bool	bVPost2;				// ������ 2�� ����
	char	nomVPost2 [16];			// ������ 2�� �԰� - GDL�� ������ �԰ݰ� ����
	double	heightVPost2;			// ������ 2�� ����

	bool	bCrosshead;				// ũ�ν���� ����
	double	heightCrosshead;		// ũ�ν���� ����

	double	heightTimber;			// ��°�/�����/GT24�Ŵ� �Ǵ� �� �ۿ��� ����

	bool	bHPost;					// ������ ����
	char	nomHPost_North [16];	// ������ �ʺ�(��) - GDL�� ������ �԰ݰ� ����, ������ ��� �� ���ڿ�
	char	nomHPost_West [16];		// ������ �ʺ�(��) - GDL�� ������ �԰ݰ� ����, ������ ��� �� ���ڿ�
	char	nomHPost_East [16];		// ������ �ʺ�(��) - GDL�� ������ �԰ݰ� ����, ������ ��� �� ���ڿ�
	char	nomHPost_South [16];	// ������ �ʺ�(��) - GDL�� ������ �԰ݰ� ����, ������ ��� �� ���ڿ�

public:
	API_Guid	placeVPost (PERI_VPost params);		// ������ ��ġ
	API_Guid	placeHPost (PERI_HPost params);		// ������ ��ġ
};

GSErrCode	placePERIPost (void);		// ������ ������ü ������ ������� PERI ���ٸ��� ��ġ��
short DGCALLBACK PERISupportingPostPlacerHandler1 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// ���ٸ� ��ġ �ɼ��� ������

#endif