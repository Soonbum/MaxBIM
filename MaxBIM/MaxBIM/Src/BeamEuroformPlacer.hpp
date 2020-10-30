#ifndef	__BEAM_EUROFORM_PLACER__
#define __BEAM_EUROFORM_PLACER__

#include "MaxBIM.hpp"

namespace beamPlacerDG {
	// ��ü Ÿ��
	enum	libPartObjType {
		NONE,			// ����
		EUROFORM,		// ������v2.0
		PLYWOOD,		// ����v1.0
		WOOD,			// ����v1.0
		OUTCORNER_ANGLE	// �ƿ��ڳʾޱ�v1.0
	};

	// �����Ǵ� �� ��ġ
	enum	attachedSide {
		BOTTOM_SIDE,
		LEFT_SIDE,
		RIGHT_SIDE
	};
}

// �� ���� ����
struct InfoBeam
{
	API_Guid	guid;	// ���� GUID

	short	floorInd;	// �� �ε���
	double	height;		// �� ����
	double	width;		// �� �ʺ�
	double	offset;		// �߽����κ��� ���� ���۷��� ������ �������Դϴ�.
	double	level;		// �ٴ� ������ ���� ���� ���ʸ� �����Դϴ�.

	API_Coord	begC;	// �� ���� ��ǥ
	API_Coord	endC;	// �� �� ��ǥ
};

// �׸��� �� �� ����
struct CellForBeam
{
	short		objType;	// enum libPartObjType ����

	API_Guid	guid;		// ��ü�� GUID

	double	leftBottomX;	// ���ϴ� ��ǥ X
	double	leftBottomY;	// ���ϴ� ��ǥ Y
	double	leftBottomZ;	// ���ϴ� ��ǥ Z
	double	ang;			// ȸ�� ���� (����: Radian, ȸ����: Z��)

	double	dirLen;			// �� ��ġ���� ����
	double	perLen;			// �� �������� ����
	short	attachd_side;	// �Ϻ�(BOTTOM_SIDE), ������(LEFT_SIDE), ������(RIGHT_SIDE)

	union {
		Euroform		form;
		Plywood			plywood;
		Wood			wood;
		OutcornerAngle	outangle;
	} libPart;
};

// �� ���� ����
struct BeamPlacingZone
{
	// ���� ���� ��ǥ�� �Ʒ�, �� ��ǥ�� ����� ������

	// �� ���� ����
	double	level;				// ��
	double	ang;				// ȸ�� ���� (����: Radian, ȸ����: Z��)
	double	leftBottomX;		// ������ ���� ��ǥ X
	double	leftBottomY;		// ������ ���� ��ǥ Y
	double	leftBottomZ;		// ������ ���� ��ǥ Z

	// ������ ����
	double	remain_hor_at_bottom_beg;	// �Ϻ� �� ���� �κ� ���� ����
	double	remain_hor_at_bottom_end;	// �Ϻ� �� �� �κ� ���� ����
	double	remain_hor_at_Lside_beg;	// ���� �� ���� �κ� ���� ����
	double	remain_hor_at_Lside_end;	// ���� �� �� �κ� ���� ����
	double	remain_hor_at_Rside_beg;	// ���� �� ���� �κ� ���� ����
	double	remain_hor_at_Rside_end;	// ���� �� �� �κ� ���� ����

	double	remain_hor_at_bottom_beg_updated;	// �Ϻ� �� ���� �κ� ���� ���� (������Ʈ ��)
	double	remain_hor_at_bottom_end_updated;	// �Ϻ� �� �� �κ� ���� ���� (������Ʈ ��)
	double	remain_hor_at_Lside_beg_updated;	// ���� �� ���� �κ� ���� ���� (������Ʈ ��)
	double	remain_hor_at_Lside_end_updated;	// ���� �� �� �κ� ���� ���� (������Ʈ ��)
	double	remain_hor_at_Rside_beg_updated;	// ���� �� ���� �κ� ���� ���� (������Ʈ ��)
	double	remain_hor_at_Rside_end_updated;	// ���� �� �� �κ� ���� ���� (������Ʈ ��)

	short	eu_count_at_bottom;		// �Ϻ� ������ ����
	short	eu_count_at_Lside;		// ���� ������ ����
	short	eu_count_at_Rside;		// ���� ������ ����

	double	moveOffset_at_bottom;	// �Ϻ� �̵� ������ (+�� �� ��ǥ ����)
	double	moveOffset_at_Lside;	// ���� �̵� ������ (+�� �� ��ǥ ����)
	double	moveOffset_at_Rside;	// ���� �̵� ������ (+�� �� ��ǥ ����)

	std::string		eu_wid;			// ������ �ʺ�
	std::string		eu_hei;			// ������ ����
	double	eu_wid_numeric;			// ������ �ʺ� (�Ǽ���)
	double	eu_hei_numeric;			// ������ ���� (�Ǽ���)
};

// ������ �� ��ġ �Լ�
GSErrCode	placeEuroformOnBeam (void);				// 3�� �޴�: ���� �������� ��ġ�ϴ� ���� ��ƾ

#endif
