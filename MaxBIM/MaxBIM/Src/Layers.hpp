#ifndef	__LAYERS__
#define __LAYERS__

#include "MaxBIM.hpp"

using namespace std;

namespace layersDG {
	// ���̾�α� �׸� �ε���
	enum	idxItems_layerMakeDG {
		BUTTON_CODE	= 3,
		BUTTON_DONG,
		BUTTON_FLOOR,
		BUTTON_CAST,
		BUTTON_CJ,
		BUTTON_ORDER,
		BUTTON_OBJ,
		BUTTON_PRODUCT_SITE,
		BUTTON_PRODUCT_NUM,

		SEPARATOR_1,
		SEPARATOR_2,
		SEPARATOR_3,
		SEPARATOR_4,
		SEPARATOR_5,
		SEPARATOR_6,
		SEPARATOR_7,
		SEPARATOR_8,
		SEPARATOR_9,

		CHECKBOX_PRODUCT_SITE_NUM
	};
}

// ���̾� �ڵ� ü��
struct LayerNameSystem
{
	// �⺻ ���� �ڵ� ����: �Ϸù�ȣ - ���籸�� - ������ - ������ - Ÿ����ȣ - CJ - CJ�ӽð����� - ���籸��
	// Ȯ�� ���� �ڵ� ����: �Ϸù�ȣ - ���籸�� - ������ - ������ - Ÿ����ȣ - CJ - CJ�ӽð����� - ���籸�� - ����ó���� - ���۹�ȣ
	/*
	 �Ϸù�ȣ-���籸��: 01-S (����), 02-A (���ึ��), 03-M (��輳��), 04-E (���⼳��), 05-T (������), 06-F (���ü�), 07-Q (��������), 08-L (����), 09-C (���), 10-K (�Ǽ����), 50-S,A,M,E,T,F,Q,L,C,K (�� ���纰 2D ����)
	 �� ����: 101~1599�� (0101~1599), SHOP (�ٸ���Ȱ�ü�), SECU (����) ... (��, ������ ������ 0000)
	 �� ����: 1B9~9B1 (����9��~1��), F01~F99 (����1��~99��), PH1~PH9 (��ž1��~9��) ...
	 Ÿ����ȣ: 01~99 (��, ������ ������ 01)
	 CJ: 01~99 (��, ������ ������ 01)
	 CJ �� �ð�����: 01~99 (��, ������ ������ 01)
	 ���籸��: WALL (��), COLU (���) ...
	 ����ó����: ��������, ��������
	 ���۹�ȣ: 001~999
	 */
	// ����(�⺻): 05-T-0000-F01-01-01-01-WALL
	// ����(Ȯ��): 05-T-0000-F01-01-01-01-WALL-��������-001

	bool	extendedLayer;		// Ȯ���̸� true, �⺻�̸� false

	// 1. ���� ���� (�ʼ�)
	vector<string>	code_name;	// �ڵ� �̸�
	vector<string>	code_desc;	// �ڵ� ����
	bool	*code_state;		// On/Off ����
	short	*code_idx;			// ���̾�α� ���� �ε���
	bool	bCodeAllShow;		// ��� ���� ��ư �����ֱ�

	// 2. �� ���� (�ʼ�)
	vector<string>	dong_name;
	vector<string>	dong_desc;
	bool	*dong_state;
	short	*dong_idx;
	bool	bDongAllShow;		// ��� ���� ��ư �����ֱ�

	// 3. �� ���� (�ʼ�)
	vector<string>	floor_name;
	vector<string>	floor_desc;
	bool	*floor_state;
	short	*floor_idx;
	bool	bFloorAllShow;		// ��� ���� ��ư �����ֱ�

	// 4. Ÿ����ȣ (�ʼ�)
	vector<string>	cast_name;
	bool	*cast_state;
	short	*cast_idx;
	bool	bCastAllShow;		// ��� ���� ��ư �����ֱ�

	// 5. CJ ���� (�ʼ�)
	vector<string>	CJ_name;
	bool	*CJ_state;
	short	*CJ_idx;
	bool	bCJAllShow;			// ��� ���� ��ư �����ֱ�

	// 6. CJ �� �ð����� (�ʼ�)
	vector<string>	orderInCJ_name;
	bool	*orderInCJ_state;
	short	*orderInCJ_idx;
	bool	bOrderInCJAllShow;	// ��� ���� ��ư �����ֱ�

	// 7. ���� ���� (�ʼ�)
	vector<string>	obj_name;
	vector<string>	obj_desc;
	vector<string>	obj_cat;
	bool	*obj_state;
	short	*obj_idx;

	// 8. ����ó ���� (����)
	vector<string>	productSite_name;
	bool	*productSite_state;
	short	*productSite_idx;
	bool	bProductSiteAllShow;// ��� ���� ��ư �����ֱ�

	// 9. ���� ��ȣ (����)
	vector<string>	productNum_name;
	bool	*productNum_state;
	short	*productNum_idx;
	bool	bProductNumAllShow;	// ��� ���� ��ư �����ֱ�
};

struct StatusOfLayerNameSystem
{
	// ���� ���� (�ʼ�)
	bool	code_state [25];

	// �� ���� (�ʼ�)
	bool	dong_state [2000];

	// �� ���� (�ʼ�)
	bool	floor_state [200];

	// Ÿ����ȣ (�ʼ�)
	bool	cast_state [120];

	// CJ ���� (�ʼ�)
	bool	CJ_state [120];

	// CJ �� �ð����� (�ʼ�)
	bool	orderInCJ_state [120];

	// ���� ���� (�ʼ�)
	bool	obj_state [500];

	// ����ó ���� (����)
	bool	productSite_state [10];

	// ���� ��ȣ (����)
	bool	productNum_state [1000];
};

void		allocateMemory (LayerNameSystem *layerInfo);		// �޸� �Ҵ�
void		deallocateMemory (LayerNameSystem *layerInfo);		// �޸� ����
bool		isFullLayer (LayerNameSystem *layerInfo);			// ���̾� �ʵ� �ڵ忡 ������ ���°�?
bool		importLayerInfo (LayerNameSystem *layerInfo);		// ���̾� ���� ���� ��������

GSErrCode	showLayersEasily (void);		// ���̾� ���� �����ϱ�
short DGCALLBACK layerShowHandler (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// [���̾�α� �ڽ�] ���̾� ���� �����ϱ�
GSErrCode	saveButtonStatus_show (void);	// �ֱ� ��ư ���� �����ϱ� (���̾� ���� �����ֱ�)

GSErrCode	makeLayersEasily (void);		// ���̾� ���� �����
short DGCALLBACK layerMakeHandler (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// [���̾�α� �ڽ�] ���̾� ���� �����
short DGCALLBACK layerMakeHandler_2 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// [���̾�α� �ڽ�] ���̾� ���� ����� 2��

GSErrCode	assignLayerEasily (void);		// ���̾� ���� �����ϱ�
short DGCALLBACK layerAssignHandler (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// [���̾�α� �ڽ�] ���̾� ���� �����ϱ�
short DGCALLBACK layerAssignHandler_2 (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// [���̾�α� �ڽ�] ���̾� ���� �����ϱ� 2��
GSErrCode	saveButtonStatus_assign (void);	// �ֱ� ��ư ���� �����ϱ� (���̾� ���� �����ϱ�)

GSErrCode	inspectLayerNames (void);		// ���̾� �̸� �˻��ϱ�
short DGCALLBACK layerNameInspectionHandler (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// [���̾�α� �ڽ�] ���̾� �̸� �˻��ϱ�

#endif