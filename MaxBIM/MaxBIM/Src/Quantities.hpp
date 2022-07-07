#ifndef	__QUANTITIES__
#define __QUANTITIES__

#include "MaxBIM.hpp"

using namespace std;

namespace quantitiesDG
{
	enum qElemPalette {
		LABEL_EXPLANATION_QELEM = 1,
		USERCONTROL_QPLYWOOD_LAYER,
		POPUP_QPLYWOOD_TYPE,
		POPUP_FLOOR_QELEM,
		BUTTON_DRAW_RECT_QELEM,
		BUTTON_DRAW_WINDOW_QELEM
	};

	enum qElemType {
		TYPE_BEAM = 1,
		TYPE_COLUMN_ISOLATION,
		TYPE_COLUMN_IN_WALL,
		TYPE_WALL_INSIDE,
		TYPE_WALL_OUTSIDE,
		TYPE_WALL_DOUBLE,
		TYPE_WALL_PARAPET,
		TYPE_WALL_WATERBLOCK,
		TYPE_SLAB_BASE,
		TYPE_SLAB_RC,
		TYPE_SLAB_DECK,
		TYPE_SLAB_RAMP,
		TYPE_STAIR
	};

	enum insulElemPalette {
		LABEL_EXPLANATION_INS = 1,
		USERCONTROL_INSULATION_LAYER,
		LABEL_INSULATION_THK,
		EDITCONTROL_INSULATION_THK,
		CHECKBOX_INS_LIMIT_SIZE,
		LABEL_INS_HORLEN,
		EDITCONTROL_INS_HORLEN,
		LABEL_INS_VERLEN,
		EDITCONTROL_INS_VERLEN,
		LABEL_FLOOR_INS,
		POPUP_FLOOR_INS,
		BUTTON_DRAW_INSULATION
	};
}

struct qElem
{
	short	dialogID;		// �ȷ�Ʈâ ID
	short	floorInd;		// �� �ε���
	short	layerInd;		// ���̾� �ε���
	char	m_type [32];	// �з�
	short	panel_mat;		// ����
};	// ��������

GSErrCode	placeQuantityPlywood (void);		// ���������� ������ �� �ִ� �ȷ�Ʈ�� ���
static short DGCALLBACK qElemDlgCallBack (short message, short dialID, short item, DGUserData userData, DGMessageData msgData);				// �ȷ�Ʈ�� ���� �ݹ� �Լ� 1
static GSErrCode __ACENV_CALL	qElemPaletteAPIControlCallBack (Int32 referenceID, API_PaletteMessageID messageID, GS::IntPtr param);		// �ȷ�Ʈ�� ���� �ݹ� �Լ� 2


struct insulElem
{
	short	dialogID;		// �ȷ�Ʈâ ID
	short	floorInd;		// �� �ε���
	short	layerInd;		// ���̾� �ε���
	double	thk;			// �β�
	bool	bLimitSize;		// ����/���� ũ�� ����
	double	maxHorLen;			// ���� �ִ� ����
	double	maxVerLen;			// ���� �ִ� ����
};	// �ܿ���

GSErrCode	placeInsulation (void);				// �ܿ��縦 ������ �� �ִ� �ȷ�Ʈ�� ���
static short DGCALLBACK insulElemDlgCallBack (short message, short dialID, short item, DGUserData userData, DGMessageData msgData);			// �ȷ�Ʈ�� ���� �ݹ� �Լ� 1
static GSErrCode __ACENV_CALL	insulElemPaletteAPIControlCallBack (Int32 referenceID, API_PaletteMessageID messageID, GS::IntPtr param);	// �ȷ�Ʈ�� ���� �ݹ� �Լ� 2

#endif