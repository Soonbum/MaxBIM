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

// �Ķ����: ������
struct paramsUFOM_ForWallTableform
{
	double	leftBottomX;	// ���ϴ� ��ǥ X
	double	leftBottomY;	// ���ϴ� ��ǥ Y
	double	leftBottomZ;	// ���ϴ� ��ǥ Z

	double	ang;			// ȸ�� ���� (����: Radian, ȸ����: Z��)

	double	width;			// �ʺ�
	double	height;			// ����
};

// �Ķ����: ��� ������
struct paramsSPIP_ForWallTableform
{
	double	leftBottomX;	// ���ϴ� ��ǥ X
	double	leftBottomY;	// ���ϴ� ��ǥ Y
	double	leftBottomZ;	// ���ϴ� ��ǥ Z

	double	ang;			// ȸ�� ���� (����: Radian, ȸ����: Z��)

	double	length;			// ������ ����
	double	pipeAng;		// ���� (����: 0, ����: 90)
};

// �Ķ����: �ɺ�Ʈ ��Ʈ
struct paramsPINB_ForWallTableform
{
	double	leftBottomX;	// ���ϴ� ��ǥ X
	double	leftBottomY;	// ���ϴ� ��ǥ Y
	double	leftBottomZ;	// ���ϴ� ��ǥ Z

	double	ang;			// ȸ�� ���� (����: Radian, ȸ����: Z��)

	bool	bPinBoltRot90;	// �ɺ�Ʈ 90�� ȸ��
	double	boltLen;		// ��Ʈ ����
	double	angX;			// X�� ȸ��
	double	angY;			// Y�� ȸ��
};

// �Ķ����: ��ü Ÿ��
struct paramsTIE_ForWallTableform
{
	double	leftBottomX;	// ���ϴ� ��ǥ X
	double	leftBottomY;	// ���ϴ� ��ǥ Y
	double	leftBottomZ;	// ���ϴ� ��ǥ Z

	double	ang;			// ȸ�� ���� (����: Radian, ȸ����: Z��)

	double	boltLen;		// ��Ʈ ����
	double	pipeBeg;		// ������ ������
	double	pipeEnd;		// ������ ����
	double	clampBeg;		// ���Ӽ� ������
	double	clampEnd;		// ���Ӽ� ����
};

// �Ķ����: ���� Ŭ����
struct paramsCLAM_ForWallTableform
{
	double	leftBottomX;	// ���ϴ� ��ǥ X
	double	leftBottomY;	// ���ϴ� ��ǥ Y
	double	leftBottomZ;	// ���ϴ� ��ǥ Z

	double	ang;			// ȸ�� ���� (����: Radian, ȸ����: Z��)

	double	angX;			// ��ü ȸ�� (X)
	double	angY;			// ��ü ȸ�� (Y)
};

// �Ķ����: ����ǽ�
struct paramsPUSH_ForWallTableform
{
	double	leftBottomX;	// ���ϴ� ��ǥ X
	double	leftBottomY;	// ���ϴ� ��ǥ Y
	double	leftBottomZ;	// ���ϴ� ��ǥ Z

	double	ang;			// ȸ�� ���� (����: Radian, ȸ����: Z��)
};

GSErrCode	placeTableformOnWall (void);											// ���� ���̺����� ��ġ�ϴ� ���� ��ƾ
void		initCellsForWallTableform (WallTableformPlacingZone* placingZone);		// Cell �迭�� �ʱ�ȭ��
GSErrCode	placeTableformOnWall (CellForWallTableform cell);						// ���̺��� ��ġ�ϱ�
double		getCellPositionLeftBottomXForWallTableForm (WallTableformPlacingZone *placingZone, short idx);			// �ش� ���� ���ϴ� ��ǥX ��ġ�� ����
short DGCALLBACK wallTableformPlacerHandler (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// ���̺��� ��ġ�� ���� ���Ǹ� ��û�ϴ� ���̾�α�

double		moveXinParallel (double prevPosX, double ang, double offset);			// �̵� ���� X ��ǥ�� �˷��� (Z ȸ������ ���) - ���� ������ �������� �̵�
double		moveYinParallel (double prevPosY, double ang, double offset);			// �̵� ���� Y ��ǥ�� �˷��� (Z ȸ������ ���) - ���� ������ �������� �̵�
double		moveXinPerpend (double prevPosX, double ang, double offset);			// �̵� ���� X ��ǥ�� �˷��� (Z ȸ������ ���) - ���� ������ �������� �̵�
double		moveYinPerpend (double prevPosY, double ang, double offset);			// �̵� ���� Y ��ǥ�� �˷��� (Z ȸ������ ���) - ���� ������ �������� �̵�
double		moveZ (double prevPosZ, double offset);									// �̵� ���� Z ��ǥ�� �˷��� (Z ȸ������ ���)

API_Guid	placeUFOM (paramsUFOM_ForWallTableform	params);			// ��ġ: ������
API_Guid	placeSPIP (paramsSPIP_ForWallTableform	params);			// ��ġ: ��� ������
API_Guid	placePINB (paramsPINB_ForWallTableform	params);			// ��ġ: �ɺ�Ʈ ��Ʈ
API_Guid	placeTIE  (paramsTIE_ForWallTableform	params);			// ��ġ: ��ü Ÿ��
API_Guid	placeCLAM (paramsCLAM_ForWallTableform	params);			// ��ġ: ���� Ŭ����
API_Guid	placePUSH (paramsPUSH_ForWallTableform	params);			// ��ġ: ����ǽ�

GSErrCode	tableformOnWall_w2300_h6000 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2300, ���� 6000)
GSErrCode	tableformOnWall_w2300_h5700 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2300, ���� 5700)
GSErrCode	tableformOnWall_w2300_h5400 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2300, ���� 5400)
GSErrCode	tableformOnWall_w2300_h5100 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2300, ���� 5100)
GSErrCode	tableformOnWall_w2300_h4800 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2300, ���� 4800)
GSErrCode	tableformOnWall_w2300_h4500 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2300, ���� 4500)
GSErrCode	tableformOnWall_w2300_h4200 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2300, ���� 4200)
GSErrCode	tableformOnWall_w2300_h3900 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2300, ���� 3900)
GSErrCode	tableformOnWall_w2300_h3600 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2300, ���� 3600)
GSErrCode	tableformOnWall_w2300_h3300 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2300, ���� 3300)
GSErrCode	tableformOnWall_w2300_h3000 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2300, ���� 3000)
GSErrCode	tableformOnWall_w2300_h2700 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2300, ���� 2700)
GSErrCode	tableformOnWall_w2300_h2400 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2300, ���� 2400)
GSErrCode	tableformOnWall_w2300_h2100 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2300, ���� 2100)
GSErrCode	tableformOnWall_w2300_h1800 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2300, ���� 1800)
GSErrCode	tableformOnWall_w2300_h1500 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2300, ���� 1500)

GSErrCode	tableformOnWall_w2250_h6000 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2250, ���� 6000)
GSErrCode	tableformOnWall_w2250_h5700 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2250, ���� 5700)
GSErrCode	tableformOnWall_w2250_h5400 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2250, ���� 5400)
GSErrCode	tableformOnWall_w2250_h5100 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2250, ���� 5100)
GSErrCode	tableformOnWall_w2250_h4800 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2250, ���� 4800)
GSErrCode	tableformOnWall_w2250_h4500 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2250, ���� 4500)
GSErrCode	tableformOnWall_w2250_h4200 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2250, ���� 4200)
GSErrCode	tableformOnWall_w2250_h3900 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2250, ���� 3900)
GSErrCode	tableformOnWall_w2250_h3600 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2250, ���� 3600)
GSErrCode	tableformOnWall_w2250_h3300 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2250, ���� 3300)
GSErrCode	tableformOnWall_w2250_h3000 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2250, ���� 3000)
GSErrCode	tableformOnWall_w2250_h2700 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2250, ���� 2700)
GSErrCode	tableformOnWall_w2250_h2400 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2250, ���� 2400)
GSErrCode	tableformOnWall_w2250_h2100 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2250, ���� 2100)
GSErrCode	tableformOnWall_w2250_h1800 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2250, ���� 1800)
GSErrCode	tableformOnWall_w2250_h1500 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2250, ���� 1500)

GSErrCode	tableformOnWall_w2200_h6000 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2200, ���� 6000)
GSErrCode	tableformOnWall_w2200_h5700 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2200, ���� 5700)
GSErrCode	tableformOnWall_w2200_h5400 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2200, ���� 5400)
GSErrCode	tableformOnWall_w2200_h5100 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2200, ���� 5100)
GSErrCode	tableformOnWall_w2200_h4800 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2200, ���� 4800)
GSErrCode	tableformOnWall_w2200_h4500 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2200, ���� 4500)
GSErrCode	tableformOnWall_w2200_h4200 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2200, ���� 4200)
GSErrCode	tableformOnWall_w2200_h3900 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2200, ���� 3900)
GSErrCode	tableformOnWall_w2200_h3600 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2200, ���� 3600)
GSErrCode	tableformOnWall_w2200_h3300 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2200, ���� 3300)
GSErrCode	tableformOnWall_w2200_h3000 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2200, ���� 3000)
GSErrCode	tableformOnWall_w2200_h2700 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2200, ���� 2700)
GSErrCode	tableformOnWall_w2200_h2400 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2200, ���� 2400)
GSErrCode	tableformOnWall_w2200_h2100 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2200, ���� 2100)
GSErrCode	tableformOnWall_w2200_h1800 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2200, ���� 1800)
GSErrCode	tableformOnWall_w2200_h1500 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2200, ���� 1500)

GSErrCode	tableformOnWall_w2150_h6000 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2150, ���� 6000)
GSErrCode	tableformOnWall_w2150_h5700 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2150, ���� 5700)
GSErrCode	tableformOnWall_w2150_h5400 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2150, ���� 5400)
GSErrCode	tableformOnWall_w2150_h5100 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2150, ���� 5100)
GSErrCode	tableformOnWall_w2150_h4800 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2150, ���� 4800)
GSErrCode	tableformOnWall_w2150_h4500 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2150, ���� 4500)
GSErrCode	tableformOnWall_w2150_h4200 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2150, ���� 4200)
GSErrCode	tableformOnWall_w2150_h3900 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2150, ���� 3900)
GSErrCode	tableformOnWall_w2150_h3600 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2150, ���� 3600)
GSErrCode	tableformOnWall_w2150_h3300 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2150, ���� 3300)
GSErrCode	tableformOnWall_w2150_h3000 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2150, ���� 3000)
GSErrCode	tableformOnWall_w2150_h2700 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2150, ���� 2700)
GSErrCode	tableformOnWall_w2150_h2400 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2150, ���� 2400)
GSErrCode	tableformOnWall_w2150_h2100 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2150, ���� 2100)
GSErrCode	tableformOnWall_w2150_h1800 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2150, ���� 1800)
GSErrCode	tableformOnWall_w2150_h1500 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2150, ���� 1500)

GSErrCode	tableformOnWall_w2100_h6000 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2100, ���� 6000)
GSErrCode	tableformOnWall_w2100_h5700 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2100, ���� 5700)
GSErrCode	tableformOnWall_w2100_h5400 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2100, ���� 5400)
GSErrCode	tableformOnWall_w2100_h5100 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2100, ���� 5100)
GSErrCode	tableformOnWall_w2100_h4800 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2100, ���� 4800)
GSErrCode	tableformOnWall_w2100_h4500 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2100, ���� 4500)
GSErrCode	tableformOnWall_w2100_h4200 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2100, ���� 4200)
GSErrCode	tableformOnWall_w2100_h3900 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2100, ���� 3900)
GSErrCode	tableformOnWall_w2100_h3600 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2100, ���� 3600)
GSErrCode	tableformOnWall_w2100_h3300 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2100, ���� 3300)
GSErrCode	tableformOnWall_w2100_h3000 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2100, ���� 3000)
GSErrCode	tableformOnWall_w2100_h2700 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2100, ���� 2700)
GSErrCode	tableformOnWall_w2100_h2400 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2100, ���� 2400)
GSErrCode	tableformOnWall_w2100_h2100 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2100, ���� 2100)
GSErrCode	tableformOnWall_w2100_h1800 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2100, ���� 1800)
GSErrCode	tableformOnWall_w2100_h1500 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2100, ���� 1500)

GSErrCode	tableformOnWall_w2050_h6000 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2050, ���� 6000)
GSErrCode	tableformOnWall_w2050_h5700 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2050, ���� 5700)
GSErrCode	tableformOnWall_w2050_h5400 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2050, ���� 5400)
GSErrCode	tableformOnWall_w2050_h5100 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2050, ���� 5100)
GSErrCode	tableformOnWall_w2050_h4800 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2050, ���� 4800)
GSErrCode	tableformOnWall_w2050_h4500 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2050, ���� 4500)
GSErrCode	tableformOnWall_w2050_h4200 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2050, ���� 4200)
GSErrCode	tableformOnWall_w2050_h3900 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2050, ���� 3900)
GSErrCode	tableformOnWall_w2050_h3600 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2050, ���� 3600)
GSErrCode	tableformOnWall_w2050_h3300 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2050, ���� 3300)
GSErrCode	tableformOnWall_w2050_h3000 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2050, ���� 3000)
GSErrCode	tableformOnWall_w2050_h2700 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2050, ���� 2700)
GSErrCode	tableformOnWall_w2050_h2400 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2050, ���� 2400)
GSErrCode	tableformOnWall_w2050_h2100 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2050, ���� 2100)
GSErrCode	tableformOnWall_w2050_h1800 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2050, ���� 1800)
GSErrCode	tableformOnWall_w2050_h1500 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2050, ���� 1500)

GSErrCode	tableformOnWall_w2000_h6000 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2000, ���� 6000)
GSErrCode	tableformOnWall_w2000_h5700 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2000, ���� 5700)
GSErrCode	tableformOnWall_w2000_h5400 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2000, ���� 5400)
GSErrCode	tableformOnWall_w2000_h5100 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2000, ���� 5100)
GSErrCode	tableformOnWall_w2000_h4800 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2000, ���� 4800)
GSErrCode	tableformOnWall_w2000_h4500 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2000, ���� 4500)
GSErrCode	tableformOnWall_w2000_h4200 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2000, ���� 4200)
GSErrCode	tableformOnWall_w2000_h3900 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2000, ���� 3900)
GSErrCode	tableformOnWall_w2000_h3600 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2000, ���� 3600)
GSErrCode	tableformOnWall_w2000_h3300 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2000, ���� 3300)
GSErrCode	tableformOnWall_w2000_h3000 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2000, ���� 3000)
GSErrCode	tableformOnWall_w2000_h2700 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2000, ���� 2700)
GSErrCode	tableformOnWall_w2000_h2400 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2000, ���� 2400)
GSErrCode	tableformOnWall_w2000_h2100 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2000, ���� 2100)
GSErrCode	tableformOnWall_w2000_h1800 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2000, ���� 1800)
GSErrCode	tableformOnWall_w2000_h1500 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 2000, ���� 1500)

GSErrCode	tableformOnWall_w1950_h6000 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 1950, ���� 6000)
GSErrCode	tableformOnWall_w1950_h5700 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 1950, ���� 5700)
GSErrCode	tableformOnWall_w1950_h5400 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 1950, ���� 5400)
GSErrCode	tableformOnWall_w1950_h5100 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 1950, ���� 5100)
GSErrCode	tableformOnWall_w1950_h4800 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 1950, ���� 4800)
GSErrCode	tableformOnWall_w1950_h4500 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 1950, ���� 4500)
GSErrCode	tableformOnWall_w1950_h4200 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 1950, ���� 4200)
GSErrCode	tableformOnWall_w1950_h3900 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 1950, ���� 3900)
GSErrCode	tableformOnWall_w1950_h3600 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 1950, ���� 3600)
GSErrCode	tableformOnWall_w1950_h3300 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 1950, ���� 3300)
GSErrCode	tableformOnWall_w1950_h3000 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 1950, ���� 3000)
GSErrCode	tableformOnWall_w1950_h2700 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 1950, ���� 2700)
GSErrCode	tableformOnWall_w1950_h2400 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 1950, ���� 2400)
GSErrCode	tableformOnWall_w1950_h2100 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 1950, ���� 2100)
GSErrCode	tableformOnWall_w1950_h1800 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 1950, ���� 1800)
GSErrCode	tableformOnWall_w1950_h1500 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 1950, ���� 1500)

GSErrCode	tableformOnWall_w1900_h6000 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 1900, ���� 6000)
GSErrCode	tableformOnWall_w1900_h5700 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 1900, ���� 5700)
GSErrCode	tableformOnWall_w1900_h5400 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 1900, ���� 5400)
GSErrCode	tableformOnWall_w1900_h5100 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 1900, ���� 5100)
GSErrCode	tableformOnWall_w1900_h4800 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 1900, ���� 4800)
GSErrCode	tableformOnWall_w1900_h4500 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 1900, ���� 4500)
GSErrCode	tableformOnWall_w1900_h4200 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 1900, ���� 4200)
GSErrCode	tableformOnWall_w1900_h3900 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 1900, ���� 3900)
GSErrCode	tableformOnWall_w1900_h3600 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 1900, ���� 3600)
GSErrCode	tableformOnWall_w1900_h3300 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 1900, ���� 3300)
GSErrCode	tableformOnWall_w1900_h3000 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 1900, ���� 3000)
GSErrCode	tableformOnWall_w1900_h2700 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 1900, ���� 2700)
GSErrCode	tableformOnWall_w1900_h2400 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 1900, ���� 2400)
GSErrCode	tableformOnWall_w1900_h2100 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 1900, ���� 2100)
GSErrCode	tableformOnWall_w1900_h1800 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 1900, ���� 1800)
GSErrCode	tableformOnWall_w1900_h1500 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 1900, ���� 1500)

GSErrCode	tableformOnWall_w1850_h6000 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 1850, ���� 6000)
GSErrCode	tableformOnWall_w1850_h5700 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 1850, ���� 5700)
GSErrCode	tableformOnWall_w1850_h5400 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 1850, ���� 5400)
GSErrCode	tableformOnWall_w1850_h5100 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 1850, ���� 5100)
GSErrCode	tableformOnWall_w1850_h4800 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 1850, ���� 4800)
GSErrCode	tableformOnWall_w1850_h4500 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 1850, ���� 4500)
GSErrCode	tableformOnWall_w1850_h4200 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 1850, ���� 4200)
GSErrCode	tableformOnWall_w1850_h3900 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 1850, ���� 3900)
GSErrCode	tableformOnWall_w1850_h3600 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 1850, ���� 3600)
GSErrCode	tableformOnWall_w1850_h3300 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 1850, ���� 3300)
GSErrCode	tableformOnWall_w1850_h3000 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 1850, ���� 3000)
GSErrCode	tableformOnWall_w1850_h2700 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 1850, ���� 2700)
GSErrCode	tableformOnWall_w1850_h2400 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 1850, ���� 2400)
GSErrCode	tableformOnWall_w1850_h2100 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 1850, ���� 2100)
GSErrCode	tableformOnWall_w1850_h1800 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 1850, ���� 1800)
GSErrCode	tableformOnWall_w1850_h1500 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 1850, ���� 1500)

GSErrCode	tableformOnWall_w1800_h6000 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 1800, ���� 6000)
GSErrCode	tableformOnWall_w1800_h5700 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 1800, ���� 5700)
GSErrCode	tableformOnWall_w1800_h5400 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 1800, ���� 5400)
GSErrCode	tableformOnWall_w1800_h5100 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 1800, ���� 5100)
GSErrCode	tableformOnWall_w1800_h4800 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 1800, ���� 4800)
GSErrCode	tableformOnWall_w1800_h4500 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 1800, ���� 4500)
GSErrCode	tableformOnWall_w1800_h4200 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 1800, ���� 4200)
GSErrCode	tableformOnWall_w1800_h3900 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 1800, ���� 3900)
GSErrCode	tableformOnWall_w1800_h3600 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 1800, ���� 3600)
GSErrCode	tableformOnWall_w1800_h3300 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 1800, ���� 3300)
GSErrCode	tableformOnWall_w1800_h3000 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 1800, ���� 3000)
GSErrCode	tableformOnWall_w1800_h2700 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 1800, ���� 2700)
GSErrCode	tableformOnWall_w1800_h2400 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 1800, ���� 2400)
GSErrCode	tableformOnWall_w1800_h2100 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 1800, ���� 2100)
GSErrCode	tableformOnWall_w1800_h1800 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 1800, ���� 1800)
GSErrCode	tableformOnWall_w1800_h1500 (CellForWallTableform cell);	// ���̺��� ��ġ (�ʺ� 1800, ���� 1500)

#endif