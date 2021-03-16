#ifndef	__WALL_TABLEFORM_PLACER__
#define __WALL_TABLEFORM_PLACER__

#include "MaxBIM.hpp"

namespace wallTableformPlacerDG {
	// ���̾�α� �׸� �ε���
	enum	idxItems_1_forWallTableformPlacer {
		LABEL_HEIGHT	= 3,
		EDITCONTROL_HEIGHT,
		LABEL_ERR_MESSAGE,
		LABEL_WIDTH,
		EDITCONTROL_WIDTH,
		LABEL_GAP_LENGTH,
		EDITCONTROL_GAP_LENGTH,

		ICON_LAYER,
		LABEL_LAYER_SETTINGS,
		LABEL_LAYER_EUROFORM,
		LABEL_LAYER_RECTPIPE,
		LABEL_LAYER_PINBOLT,
		LABEL_LAYER_WALLTIE,
		LABEL_LAYER_CLAMP,
		LABEL_LAYER_HEADPIECE,

		USERCONTROL_LAYER_EUROFORM,
		USERCONTROL_LAYER_RECTPIPE,
		USERCONTROL_LAYER_PINBOLT,
		USERCONTROL_LAYER_WALLTIE,
		USERCONTROL_LAYER_CLAMP,
		USERCONTROL_LAYER_HEADPIECE
	};
}

// �� ���� ����
struct InfoWallForWallTableform
{
	double	wallThk;			// �� �β�
	short	floorInd;			// �� �ε���
	double	bottomOffset;		// �� �ϴ� ������

	double	begX;				// ������ X
	double	begY;				// ������ Y
	double	endX;				// ���� X
	double	endY;				// ���� Y
};

// ���� ���� ����
struct InfoMorphForWallTableform
{
	API_Guid	guid;		// ������ GUID

	double	leftBottomX;	// ���ϴ� ��ǥ X
	double	leftBottomY;	// ���ϴ� ��ǥ Y
	double	leftBottomZ;	// ���ϴ� ��ǥ Z

	double	rightTopX;		// ���� ��ǥ X
	double	rightTopY;		// ���� ��ǥ Y
	double	rightTopZ;		// ���� ��ǥ Z

	double	horLen;			// ���� ����
	double	verLen;			// ���� ����
	double	ang;			// ȸ�� ���� (����: Degree, ȸ����: Z��)
};

// �׸��� �� �� ����
struct CellForWallTableform
{
	double	leftBottomX;	// ���ϴ� ��ǥ X
	double	leftBottomY;	// ���ϴ� ��ǥ Y
	double	leftBottomZ;	// ���ϴ� ��ǥ Z

	double	horLen;			// ���� ���� (1800~2300, 50 ����)
	double	verLen;			// ���� ���� (1500~6000, 300 ����)
	double	ang;			// ȸ�� ���� (����: Radian, ȸ����: Z��)
};

// ���� ���� ����
struct WallTableformPlacingZone
{
	double	leftBottomX;	// ���ϴ� ��ǥ X
	double	leftBottomY;	// ���ϴ� ��ǥ Y
	double	leftBottomZ;	// ���ϴ� ��ǥ Z

	double	horLen;			// ���� ����
	double	verLen;			// ���� ����
	double	ang;			// ȸ�� ���� (����: Radian, ȸ����: Z��)

	double	gap;			// ������ ����

	// ������ ���� (2. ��ġ�� ��ü ������ �׸���� ����)
	CellForWallTableform	cells [50];
	short		nCells;

	// ���̺��� ���� (������ �ʺ� 1800~2300�� ���̺����� �ǹ���)
	short	n1800w;
	short	n1850w;
	short	n1900w;
	short	n1950w;
	short	n2000w;
	short	n2050w;
	short	n2100w;
	short	n2150w;
	short	n2200w;
	short	n2250w;
	short	n2300w;
};

GSErrCode	placeTableformOnWall (void);	// ���� ���̺����� ��ġ�ϴ� ���� ��ƾ
short DGCALLBACK wallTableformPlacerHandler (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// ���̺��� ��ġ�� ���� ���Ǹ� ��û�ϴ� ���̾�α�

#endif