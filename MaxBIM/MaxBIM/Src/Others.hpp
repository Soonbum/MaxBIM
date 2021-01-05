#ifndef	__OTHERS__
#define __OTHERS__

#include "MaxBIM.hpp"

namespace othersDG {
	// ���̾�α� �׸� �ε���
	enum	idxItems_1_others {
	};
}

// ���̾� �ڵ� ü��
struct LayerNameSystem
{
	// ���� �ڵ� ����: 01-S-(0101)-9B1-(01)-(01)-WALL  ��, ��ȣ ���� ���û���

	// ���� ���� (�ʼ�)
	bool	bType_01_S;		// ���� (01-S)
	bool	bType_02_A;		// ���ึ�� (02-A)
	bool	bType_03_M;		// ��輳�� (03-M)
	bool	bType_04_E;		// ���⼳�� (04-E)
	bool	bType_05_T;		// ������ (05-T)
	bool	bType_06_F;		// ���ü� (06-F)
	bool	bType_07_Q;		// �������� (07-Q)
	bool	bType_08_L;		// ���� (08-L)
	bool	bType_09_C;		// ��� (09-C)
	bool	bType_10_K;		// �Ǽ���� (10-K)

	short	iType_01_S;		// ���� (01-S)
	short	iType_02_A;		// ���ึ�� (02-A)
	short	iType_03_M;		// ��輳�� (03-M)
	short	iType_04_E;		// ���⼳�� (04-E)
	short	iType_05_T;		// ������ (05-T)
	short	iType_06_F;		// ���ü� (06-F)
	short	iType_07_Q;		// �������� (07-Q)
	short	iType_08_L;		// ���� (08-L)
	short	iType_09_C;		// ��� (09-C)
	short	iType_10_K;		// �Ǽ���� (10-K)

	// �� ���� (����)
	bool	bDong_1xx [100];	// 100 ~ 199�� (0100~0199)
	bool	bDong_2xx [100];	// 200 ~ 299�� (0200~0299)
	bool	bDong_3xx [100];	// 300 ~ 399�� (0300~0399)
	bool	bDong_4xx [100];	// 400 ~ 499�� (0400~0499)
	bool	bDong_5xx [100];	// 500 ~ 599�� (0500~0599)
	bool	bDong_6xx [100];	// 600 ~ 699�� (0600~0699)
	bool	bDong_7xx [100];	// 700 ~ 799�� (0700~0799)
	bool	bDong_8xx [100];	// 800 ~ 899�� (0800~0899)
	bool	bDong_9xx [100];	// 900 ~ 999�� (0900~0999)
	bool	bDong_10xx [100];	// 1000 ~ 1099�� (1000~1099)
	bool	bDong_11xx [100];	// 1100 ~ 1199�� (1100~1199)
	bool	bDong_12xx [100];	// 1200 ~ 1299�� (1200~1299)
	bool	bDong_13xx [100];	// 1300 ~ 1399�� (1300~1399)
	bool	bDong_14xx [100];	// 1400 ~ 1499�� (1400~1499)
	bool	bDong_15xx [100];	// 1500 ~ 1599�� (1500~1599)
	bool	bDong_SHOP;			// �ٸ����� (SHOP)
	bool	bDong_SECU;			// ���� (SECU)

	short	iDong_1xx [100];	// 100 ~ 199�� (0100~0199)
	short	iDong_2xx [100];	// 200 ~ 299�� (0200~0299)
	short	iDong_3xx [100];	// 300 ~ 399�� (0300~0399)
	short	iDong_4xx [100];	// 400 ~ 499�� (0400~0499)
	short	iDong_5xx [100];	// 500 ~ 599�� (0500~0599)
	short	iDong_6xx [100];	// 600 ~ 699�� (0600~0699)
	short	iDong_7xx [100];	// 700 ~ 799�� (0700~0799)
	short	iDong_8xx [100];	// 800 ~ 899�� (0800~0899)
	short	iDong_9xx [100];	// 900 ~ 999�� (0900~0999)
	short	iDong_10xx [100];	// 1000 ~ 1099�� (1000~1099)
	short	iDong_11xx [100];	// 1100 ~ 1199�� (1100~1199)
	short	iDong_12xx [100];	// 1200 ~ 1299�� (1200~1299)
	short	iDong_13xx [100];	// 1300 ~ 1399�� (1300~1399)
	short	iDong_14xx [100];	// 1400 ~ 1499�� (1400~1499)
	short	iDong_15xx [100];	// 1500 ~ 1599�� (1500~1599)
	short	iDong_SHOP;			// �ٸ����� (SHOP)
	short	iDong_SECU;			// ���� (SECU)

	// �� ���� (�ʼ�)
	bool	bFloor_Bxx [10];	// ������ (�ε��� 0�� ������� ����) 9B1, 8B2, 7B3, 6B4, 5B5, 4B6, 3B7, 2B8, 1B9 (1B9~9B1���� 1��° ���ڴ� ���� �������� ���� ������ �ǹ̴� ����)
	bool	bFloor_Fxx [100];	// ������ (�ε��� 0�� ������� ����) F01 ~ F99
	bool	bFloor_PHxx [10];	// ��ž�� (�ε��� 0�� ������� ����) PH1 ~ PH9

	short	iFloor_Bxx [10];	// ������ (�ε��� 0�� ������� ����) 9B1, 8B2, 7B3, 6B4, 5B5, 4B6, 3B7, 2B8, 1B9 (1B9~9B1���� 1��° ���ڴ� ���� �������� ���� ������ �ǹ̴� ����)
	short	iFloor_Fxx [100];	// ������ (�ε��� 0�� ������� ����) F01 ~ F99
	short	iFloor_PHxx [10];	// ��ž�� (�ε��� 0�� ������� ����) PH1 ~ PH9

	// CJ ���� (����)
	bool	bCJ [100];			// 01~99 (�ε��� 0�� ������� ����)

	short	iCJ [100];			// 01~99 (�ε��� 0�� ������� ����)

	// CJ �� �ð����� (����)
	bool	bOrderInCJ [100];	// 01~99 (�ε��� 0�� ������� ����)

	short	iOrderInCJ [100];	// 01~99 (�ε��� 0�� ������� ����)

	// ���� �� ��ü ���� (�ʼ�)
	// 01-S ����
	bool	bType_01_S_STAR;	// ���
	bool	bType_01_S_COLU;	// ���
	bool	bType_01_S_FOOT;	// ����
	bool	bType_01_S_WALL;	// ��ü
	bool	bType_01_S_BEAM;	// ��
	bool	bType_01_S_SLAB;	// ������
	bool	bType_01_S_CLST;	// ö����
	bool	bType_01_S_BMST;	// ö��
	bool	bType_01_S_RAMP;	// ����
	bool	bType_01_S_CWAL;	// �պ�
	bool	bType_01_S_WTWL;	// �����
	bool	bType_01_S_CSTN;	// ����
	bool	bType_01_S_MPAD;	// ����е�
	bool	bType_01_S_GADN;	// ȭ��
	bool	bType_01_S_PARA;	// �Ķ���
	bool	bType_01_S_CLPC;	// PC���
	bool	bType_01_S_BMPC;	// ��PC
	bool	bType_01_S_BMWL;	// ����ü
	bool	bType_01_S_STST;	// ö����

	short	iType_01_S_STAR;	// ���
	short	iType_01_S_COLU;	// ���
	short	iType_01_S_FOOT;	// ����
	short	iType_01_S_WALL;	// ��ü
	short	iType_01_S_BEAM;	// ��
	short	iType_01_S_SLAB;	// ������
	short	iType_01_S_CLST;	// ö����
	short	iType_01_S_BMST;	// ö��
	short	iType_01_S_RAMP;	// ����
	short	iType_01_S_CWAL;	// �պ�
	short	iType_01_S_WTWL;	// �����
	short	iType_01_S_CSTN;	// ����
	short	iType_01_S_MPAD;	// ����е�
	short	iType_01_S_GADN;	// ȭ��
	short	iType_01_S_PARA;	// �Ķ���
	short	iType_01_S_CLPC;	// PC���
	short	iType_01_S_BMPC;	// ��PC
	short	iType_01_S_BMWL;	// ����ü
	short	iType_01_S_STST;	// ö����

	// 02-A ���ึ��
	bool	bType_02_A_FURN;	// ����
	bool	bType_02_A_INSU;	// �ܿ���
	bool	bType_02_A_PAIN;	// ����
	bool	bType_02_A_MOLD;	// ����
	bool	bType_02_A_MORT;	// ��Ż
	bool	bType_02_A_WATE;	// ���
	bool	bType_02_A_BRIC;	// ����
	bool	bType_02_A_PAPE;	// ����
	bool	bType_02_A_BLOC;	// ���
	bool	bType_02_A_GYPS;	// ������
	bool	bType_02_A_STON;	// ����
	bool	bType_02_A_INTE;	// ����
	bool	bType_02_A_GLAS;	// ����
	bool	bType_02_A_HARD;	// ö��
	bool	bType_02_A_TILE;	// Ÿ��
	bool	bType_02_A_PANE;	// �ǳ�
	bool	bType_02_A_PLYW;	// ����
	bool	bType_02_A_PCON;	// ������ũ��Ʈ
	
	short	iType_02_A_FURN;	// ����
	short	iType_02_A_INSU;	// �ܿ���
	short	iType_02_A_PAIN;	// ����
	short	iType_02_A_MOLD;	// ����
	short	iType_02_A_MORT;	// ��Ż
	short	iType_02_A_WATE;	// ���
	short	iType_02_A_BRIC;	// ����
	short	iType_02_A_PAPE;	// ����
	short	iType_02_A_BLOC;	// ���
	short	iType_02_A_GYPS;	// ������
	short	iType_02_A_STON;	// ����
	short	iType_02_A_INTE;	// ����
	short	iType_02_A_GLAS;	// ����
	short	iType_02_A_HARD;	// ö��
	short	iType_02_A_TILE;	// Ÿ��
	short	iType_02_A_PANE;	// �ǳ�
	short	iType_02_A_PLYW;	// ����
	short	iType_02_A_PCON;	// ������ũ��Ʈ

	// 05-T ������
	bool	bType_05_T_TIMB;	// ����
	bool	bType_05_T_BIMJ;	// �����������
	bool	bType_05_T_BDCM;	// ��չ��
	bool	bType_05_T_DMGA;	// �ٸ���
	bool	bType_05_T_RIBL;	// �����
	bool	bType_05_T_BMSP;	// ����ħ�ʷ�
	bool	bType_05_T_BSTA;	// �����
	bool	bType_05_T_SPIP;	// �簢������
	bool	bType_05_T_SUPT;	// ����Ʈ
	bool	bType_05_T_SYSU;	// �ý��ۼ���Ʈ
	bool	bType_05_T_OUTA;	// �ƿ��ڳʾޱ�
	bool	bType_05_T_OUTP;	// �ƿ��ڳ��ǳ�
	bool	bType_05_T_AFOM;	// ����
	bool	bType_05_T_CPIP;	// ����������
	bool	bType_05_T_UGON;	// ��������
	bool	bType_05_T_UFOM;	// ������
	bool	bType_05_T_INCO;	// ���ڳ��ǳ�
	bool	bType_05_T_JOIB;	// ����Ʈ��
	bool	bType_05_T_EFOM;	// ���̰�Ǫ��
	bool	bType_05_T_JSUPT;	// �輭��Ʈ
	bool	bType_05_T_WTST;	// ������
	bool	bType_05_T_CLAM;	// Ŭ����
	bool	bType_05_T_LUMB;	// �����
	bool	bType_05_T_TRUS;	// Ʈ����
	bool	bType_05_T_TBBM;	// ������
	bool	bType_05_T_BCWF;	// �պ�������
	bool	bType_05_T_PLYW;	// ����
	bool	bType_05_T_FISP;	// �ٷ������̼�
	bool	bType_05_T_STSE;	// ���������
	bool	bType_05_T_SLSE;	// ������������
	bool	bType_05_T_RAIL;	// ��ɷ���
	
	short	iType_05_T_TIMB;	// ����
	short	iType_05_T_BIMJ;	// �����������
	short	iType_05_T_BDCM;	// ��չ��
	short	iType_05_T_DMGA;	// �ٸ���
	short	iType_05_T_RIBL;	// �����
	short	iType_05_T_BMSP;	// ����ħ�ʷ�
	short	iType_05_T_BSTA;	// �����
	short	iType_05_T_SPIP;	// �簢������
	short	iType_05_T_SUPT;	// ����Ʈ
	short	iType_05_T_SYSU;	// �ý��ۼ���Ʈ
	short	iType_05_T_OUTA;	// �ƿ��ڳʾޱ�
	short	iType_05_T_OUTP;	// �ƿ��ڳ��ǳ�
	short	iType_05_T_AFOM;	// ����
	short	iType_05_T_CPIP;	// ����������
	short	iType_05_T_UGON;	// ��������
	short	iType_05_T_UFOM;	// ������
	short	iType_05_T_INCO;	// ���ڳ��ǳ�
	short	iType_05_T_JOIB;	// ����Ʈ��
	short	iType_05_T_EFOM;	// ���̰�Ǫ��
	short	iType_05_T_JSUPT;	// �輭��Ʈ
	short	iType_05_T_WTST;	// ������
	short	iType_05_T_CLAM;	// Ŭ����
	short	iType_05_T_LUMB;	// �����
	short	iType_05_T_TRUS;	// Ʈ����
	short	iType_05_T_TBBM;	// ������
	short	iType_05_T_BCWF;	// �պ�������
	short	iType_05_T_PLYW;	// ����
	short	iType_05_T_FISP;	// �ٷ������̼�
	short	iType_05_T_STSE;	// ���������
	short	iType_05_T_SLSE;	// ������������
	short	iType_05_T_RAIL;	// ��ɷ���

	// 06-F ���ü�
	bool	bType_06_F_STRU;	// ����H����
	bool	bType_06_F_HFIL;	// ����H����
	bool	bType_06_F_SJAK;	// ��ũ����
	bool	bType_06_F_PJAK;	// �����ε���
	bool	bType_06_F_BRKT;	// �����
	bool	bType_06_F_PBKT;	// �ǽ� �����
	bool	bType_06_F_CIP;		// �븷�� ��ü
	bool	bType_06_F_LAND;	// ����
	bool	bType_06_F_ANGL;	// �ޱ�
	bool	bType_06_F_ERAC;	// ���ݾ�Ŀ
	bool	bType_06_F_LUMB;	// �����
	bool	bType_06_F_BPAN;	// ������
	bool	bType_06_F_WALE;	// ����
	bool	bType_06_F_PILE;	// ����

	short	iType_06_F_STRU;	// ����H����
	short	iType_06_F_HFIL;	// ����H����
	short	iType_06_F_SJAK;	// ��ũ����
	short	iType_06_F_PJAK;	// �����ε���
	short	iType_06_F_BRKT;	// �����
	short	iType_06_F_PBKT;	// �ǽ� �����
	short	iType_06_F_CIP;		// �븷�� ��ü
	short	iType_06_F_LAND;	// ����
	short	iType_06_F_ANGL;	// �ޱ�
	short	iType_06_F_ERAC;	// ���ݾ�Ŀ
	short	iType_06_F_LUMB;	// �����
	short	iType_06_F_BPAN;	// ������
	short	iType_06_F_WALE;	// ����
	short	iType_06_F_PILE;	// ����
};

void		initLayerInfo (LayerNameSystem *layerInfo);		// ���̾� ���� �ʱ�ȭ
GSErrCode	showLayersEasily (void);						// ���̾� ���� �����ϱ�
short DGCALLBACK layerShowHandler (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// [���̾�α� �ڽ�] ���̾� �����ֱ�

#endif