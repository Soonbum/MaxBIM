#ifndef	__LAYERS__
#define __LAYERS__

#include "MaxBIM.hpp"

using namespace std;

// ���̾� �ڵ� ü��
struct LayerNameSystem
{
	// ���� �ڵ� ����: 05-T-(0101)-9B1-(01)-(01)-COLU-(UFOM)  ��, ��ȣ ���� ���û���

	// ���� ���� (�ʼ�)
	vector<string>	code_name;
	vector<string>	code_desc;
	bool	*code_state;
	short	*code_idx;

	// �� ���� (����)
	vector<string>	dong_name;
	vector<string>	dong_desc;
	bool	*dong_state;
	short	*dong_idx;
	bool	bDongAllShow;		// ��� ���� ��ư �����ֱ�

	// �� ���� (�ʼ�)
	vector<string>	floor_name;
	vector<string>	floor_desc;
	bool	*floor_state;
	short	*floor_idx;

	// CJ ���� (����)
	vector<string>	CJ_name;
	bool	*CJ_state;
	short	*CJ_idx;
	bool	bCJAllShow;			// ��� ���� ��ư �����ֱ�

	// CJ �� �ð����� (����)
	vector<string>	orderInCJ_name;
	bool	*orderInCJ_state;
	short	*orderInCJ_idx;
	bool	bOrderInCJAllShow;	// ��� ���� ��ư �����ֱ�

	// ���� ���� (�ʼ�)
	vector<string>	obj_name;
	vector<string>	obj_desc;
	vector<string>	obj_cat;
	bool	*obj_state;
	short	*obj_idx;

	// ��ü ���� (����)
	vector<string>	subObj_name;
	vector<string>	subObj_desc;
	vector<string>	subObj_cat;
	bool	*subObj_state;
	short	*subObj_idx;
};

struct StatusOfLayerNameSystem
{
	// ���� ���� (�ʼ�)
	bool	code_state [10];

	// �� ���� (����)
	bool	dong_state [2000];

	// �� ���� (�ʼ�)
	bool	floor_state [200];

	// CJ ���� (����)
	bool	CJ_state [10];

	// CJ �� �ð����� (����)
	bool	orderInCJ_state [10];

	// ���� ���� (�ʼ�)
	bool	obj_state [500];

	// ��ü ���� (����)
	bool	subObj_state [500];
};

void		allocateMemory (void);			// �޸� �Ҵ�
void		deallocateMemory (void);		// �޸� ����
GSErrCode	showLayersEasily (void);		// ���̾� ���� �����ϱ�
short DGCALLBACK layerShowHandler (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// [���̾�α� �ڽ�] ���̾� �����ֱ�
GSErrCode	saveButtonStatus (void);		// �ֱ� ��ư ���� �����ϱ�

#endif