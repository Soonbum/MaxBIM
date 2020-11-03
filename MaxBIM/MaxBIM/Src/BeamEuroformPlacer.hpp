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
	double	offset;		// �� �߽����κ��� ���� ���۷��� ������ �������Դϴ�.
	double	level;		// �ٴ� ������ ���� ���� ���ʸ� �����Դϴ�.

	API_Coord	begC;	// �� ���� ��ǥ
	API_Coord	endC;	// �� �� ��ǥ
};

// ���� ���� ����
struct InfoMorphForBeam
{
	API_Guid	guid;		// ������ GUID
	short		floorInd;	// �� �ε���
	double		level;		// ������ ��
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
	// ���� ���� ��ǥ�� ����, �� ��ǥ�� �������̶�� ������

	// �� ���� ����
	double	level;				// �� ���� ��
	double	ang;				// ȸ�� ���� (����: Radian, ȸ����: Z��)
	double	areaHeight;			// ���� ����

	API_Coord3D		begC;		// ��ġ ���� ������
	API_Coord3D		endC;		// ��ġ ���� ����
	double	beamLength;			// ���� �� ��ü ����

	// ������ ����
	// ... ���� �� ����
	// ... ���� �� ��ġ
	// ... ���� �� �ʺ�/����

	// ... ���� ���� �κ� ���� (�Է�)
	// ... ���� �� �κ� ���� (�Է�)
	// ... �Ϻ� ���� �κ� ���� (�Է�)
	// ... �Ϻ� �� �κ� ���� (�Է�)

	double	gap;		// ������ ����

	// �⺻ ä���
	std::string		eu_wid;			// ������ �ʺ�
	std::string		eu_hei;			// ������ ����
	double	eu_wid_numeric;			// ������ �ʺ� (�Ǽ���)
	double	eu_hei_numeric;			// ������ ���� (�Ǽ���)
};

// ������ �� ��ġ �Լ�
GSErrCode	placeEuroformOnBeam (void);				// 3�� �޴�: ���� �������� ��ġ�ϴ� ���� ��ƾ

#endif
