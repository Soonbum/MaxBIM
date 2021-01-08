#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "Layers.hpp"

using namespace othersDG;

static LayerNameSystem	layerInfo;
static LayerNameSystem	selectedInfo;

static short	changedBtnItemIdx;	// ���°� ����� ��ư�� �׸� �ε���
static short	SELECTALL_1_CONTYPE;
static short	SELECTALL_2_DONG;
static short	SELECTALL_3_FLOOR;
static short	SELECTALL_4_CJ;
static short	SELECTALL_5_ORDER;
static short	SELECTALL_6_OBJ_01_S;
static short	SELECTALL_6_OBJ_02_A;
static short	SELECTALL_6_OBJ_05_T;
static short	SELECTALL_6_OBJ_06_F;
static short	SELECTALL_6_OBJ_05_T_2;


// ���̾� ���� �ʱ�ȭ
void	initLayerInfo (LayerNameSystem *layerInfo)
{
	short xx;

	// ���� ����
	layerInfo->bType_01_S = false;
	layerInfo->bType_02_A = false;
	layerInfo->bType_03_M = false;
	layerInfo->bType_04_E = false;
	layerInfo->bType_05_T = false;
	layerInfo->bType_06_F = false;
	layerInfo->bType_07_Q = false;
	layerInfo->bType_08_L = false;
	layerInfo->bType_09_C = false;
	layerInfo->bType_10_K = false;

	layerInfo->iType_01_S = 0;
	layerInfo->iType_02_A = 0;
	layerInfo->iType_03_M = 0;
	layerInfo->iType_04_E = 0;
	layerInfo->iType_05_T = 0;
	layerInfo->iType_06_F = 0;
	layerInfo->iType_07_Q = 0;
	layerInfo->iType_08_L = 0;
	layerInfo->iType_09_C = 0;
	layerInfo->iType_10_K = 0;

	// �� ����
	for (xx = 0 ; xx < 100 ; ++xx) {
		layerInfo->bDong_1xx [xx] = false;
		layerInfo->bDong_2xx [xx] = false;
		layerInfo->bDong_3xx [xx] = false;
		layerInfo->bDong_4xx [xx] = false;
		layerInfo->bDong_5xx [xx] = false;
		layerInfo->bDong_6xx [xx] = false;
		layerInfo->bDong_7xx [xx] = false;
		layerInfo->bDong_8xx [xx] = false;
		layerInfo->bDong_9xx [xx] = false;
		layerInfo->bDong_10xx [xx] = false;
		layerInfo->bDong_11xx [xx] = false;
		layerInfo->bDong_12xx [xx] = false;
		layerInfo->bDong_13xx [xx] = false;
		layerInfo->bDong_14xx [xx] = false;
		layerInfo->bDong_15xx [xx] = false;
		layerInfo->bDong_SHOP = false;
		layerInfo->bDong_SECU = false;

		layerInfo->iDong_1xx [xx] = 0;
		layerInfo->iDong_2xx [xx] = 0;
		layerInfo->iDong_3xx [xx] = 0;
		layerInfo->iDong_4xx [xx] = 0;
		layerInfo->iDong_5xx [xx] = 0;
		layerInfo->iDong_6xx [xx] = 0;
		layerInfo->iDong_7xx [xx] = 0;
		layerInfo->iDong_8xx [xx] = 0;
		layerInfo->iDong_9xx [xx] = 0;
		layerInfo->iDong_10xx [xx] = 0;
		layerInfo->iDong_11xx [xx] = 0;
		layerInfo->iDong_12xx [xx] = 0;
		layerInfo->iDong_13xx [xx] = 0;
		layerInfo->iDong_14xx [xx] = 0;
		layerInfo->iDong_15xx [xx] = 0;
		layerInfo->iDong_SHOP = 0;
		layerInfo->iDong_SECU = 0;
	}
	layerInfo->bDongAllShow = false;

	// �� ����
	for (xx = 0 ; xx < 10 ; ++xx) {
		layerInfo->bFloor_Bxx [xx] = false;
		layerInfo->bFloor_PHxx [xx] = false;

		layerInfo->iFloor_Bxx [xx] = 0;
		layerInfo->iFloor_PHxx [xx] = 0;
	}
	for (xx = 0 ; xx < 100 ; ++xx) {
		layerInfo->bFloor_Fxx [xx] = false;

		layerInfo->iFloor_Fxx [xx] = 0;
	}

	// CJ ����
	for (xx = 0 ; xx < 100 ; ++xx) {
		layerInfo->bCJ [xx] = false;

		layerInfo->iCJ [xx] = 0;
	}
	layerInfo->bCJAllShow = false;

	// CJ �� �ð�����
	for (xx = 0 ; xx < 100 ; ++xx) {
		layerInfo->bOrderInCJ [xx] = false;

		layerInfo->iOrderInCJ [xx] = 0;
	}
	layerInfo->bOrderInCJAllShow = false;

	// ���� ����
	// 01-S ����
	layerInfo->bType_01_S_STAR = false;	// ���
	layerInfo->bType_01_S_COLU = false;	// ���
	layerInfo->bType_01_S_FOOT = false;	// ����
	layerInfo->bType_01_S_WALL = false;	// ��ü
	layerInfo->bType_01_S_BEAM = false;	// ��
	layerInfo->bType_01_S_SLAB = false;	// ������
	layerInfo->bType_01_S_CLST = false;	// ö����
	layerInfo->bType_01_S_BMST = false;	// ö��
	layerInfo->bType_01_S_RAMP = false;	// ����
	layerInfo->bType_01_S_CWAL = false;	// �պ�
	layerInfo->bType_01_S_WTWL = false;	// �����
	layerInfo->bType_01_S_CSTN = false;	// ����
	layerInfo->bType_01_S_MPAD = false;	// ����е�
	layerInfo->bType_01_S_GADN = false;	// ȭ��
	layerInfo->bType_01_S_PARA = false;	// �Ķ���
	layerInfo->bType_01_S_CLPC = false;	// PC���
	layerInfo->bType_01_S_BMPC = false;	// ��PC
	layerInfo->bType_01_S_BMWL = false;	// ����ü
	layerInfo->bType_01_S_STST = false;	// ö����
	layerInfo->bType_01_S_TOPP = false;	// ����
	layerInfo->bType_01_S_DECK = false;	// ��ũ
	layerInfo->bType_01_S_AllShow = false;

	layerInfo->iType_01_S_STAR = 0;	// ���
	layerInfo->iType_01_S_COLU = 0;	// ���
	layerInfo->iType_01_S_FOOT = 0;	// ����
	layerInfo->iType_01_S_WALL = 0;	// ��ü
	layerInfo->iType_01_S_BEAM = 0;	// ��
	layerInfo->iType_01_S_SLAB = 0;	// ������
	layerInfo->iType_01_S_CLST = 0;	// ö����
	layerInfo->iType_01_S_BMST = 0;	// ö��
	layerInfo->iType_01_S_RAMP = 0;	// ����
	layerInfo->iType_01_S_CWAL = 0;	// �պ�
	layerInfo->iType_01_S_WTWL = 0;	// �����
	layerInfo->iType_01_S_CSTN = 0;	// ����
	layerInfo->iType_01_S_MPAD = 0;	// ����е�
	layerInfo->iType_01_S_GADN = 0;	// ȭ��
	layerInfo->iType_01_S_PARA = 0;	// �Ķ���
	layerInfo->iType_01_S_CLPC = 0;	// PC���
	layerInfo->iType_01_S_BMPC = 0;	// ��PC
	layerInfo->iType_01_S_BMWL = 0;	// ����ü
	layerInfo->iType_01_S_STST = 0;	// ö����
	layerInfo->iType_01_S_TOPP = 0;	// ����
	layerInfo->iType_01_S_DECK = 0;	// ��ũ

	// 02-A ���ึ��
	layerInfo->bType_02_A_FURN = false;	// ����
	layerInfo->bType_02_A_INSU = false;	// �ܿ���
	layerInfo->bType_02_A_PAIN = false;	// ����
	layerInfo->bType_02_A_MOLD = false;	// ����
	layerInfo->bType_02_A_MORT = false;	// ��Ż
	layerInfo->bType_02_A_WATE = false;	// ���
	layerInfo->bType_02_A_BRIC = false;	// ����
	layerInfo->bType_02_A_PAPE = false;	// ����
	layerInfo->bType_02_A_BLOC = false;	// ���
	layerInfo->bType_02_A_GYPS = false;	// ������
	layerInfo->bType_02_A_STON = false;	// ����
	layerInfo->bType_02_A_INTE = false;	// ����
	layerInfo->bType_02_A_GLAS = false;	// ����
	layerInfo->bType_02_A_HARD = false;	// ö��
	layerInfo->bType_02_A_TILE = false;	// Ÿ��
	layerInfo->bType_02_A_PANE = false;	// �ǳ�
	layerInfo->bType_02_A_PLYW = false;	// ����
	layerInfo->bType_02_A_PCON = false;	// ������ũ��Ʈ
	layerInfo->bType_02_A_AllShow = false;
	
	layerInfo->iType_02_A_FURN = 0;	// ����
	layerInfo->iType_02_A_INSU = 0;	// �ܿ���
	layerInfo->iType_02_A_PAIN = 0;	// ����
	layerInfo->iType_02_A_MOLD = 0;	// ����
	layerInfo->iType_02_A_MORT = 0;	// ��Ż
	layerInfo->iType_02_A_WATE = 0;	// ���
	layerInfo->iType_02_A_BRIC = 0;	// ����
	layerInfo->iType_02_A_PAPE = 0;	// ����
	layerInfo->iType_02_A_BLOC = 0;	// ���
	layerInfo->iType_02_A_GYPS = 0;	// ������
	layerInfo->iType_02_A_STON = 0;	// ����
	layerInfo->iType_02_A_INTE = 0;	// ����
	layerInfo->iType_02_A_GLAS = 0;	// ����
	layerInfo->iType_02_A_HARD = 0;	// ö��
	layerInfo->iType_02_A_TILE = 0;	// Ÿ��
	layerInfo->iType_02_A_PANE = 0;	// �ǳ�
	layerInfo->iType_02_A_PLYW = 0;	// ����
	layerInfo->iType_02_A_PCON = 0;	// ������ũ��Ʈ

	// 05-T ������
	layerInfo->bType_05_T_TIMB = false;	// ����
	layerInfo->bType_05_T_BIMJ = false;	// �����������
	layerInfo->bType_05_T_BDCM = false;	// ��չ��
	layerInfo->bType_05_T_DMGA = false;	// �ٸ���
	layerInfo->bType_05_T_RIBL = false;	// �����
	layerInfo->bType_05_T_BMSP = false;	// ����ħ�ʷ�
	layerInfo->bType_05_T_BSTA = false;	// �����
	layerInfo->bType_05_T_SPIP = false;	// �簢������
	layerInfo->bType_05_T_SUPT = false;	// ����Ʈ
	layerInfo->bType_05_T_SYSU = false;	// �ý��ۼ���Ʈ
	layerInfo->bType_05_T_OUTA = false;	// �ƿ��ڳʾޱ�
	layerInfo->bType_05_T_OUTP = false;	// �ƿ��ڳ��ǳ�
	layerInfo->bType_05_T_AFOM = false;	// ����
	layerInfo->bType_05_T_CPIP = false;	// ����������
	layerInfo->bType_05_T_UGON = false;	// ��������
	layerInfo->bType_05_T_UFOM = false;	// ������
	layerInfo->bType_05_T_INCO = false;	// ���ڳ��ǳ�
	layerInfo->bType_05_T_JOIB = false;	// ����Ʈ��
	layerInfo->bType_05_T_EFOM = false;	// ���̰�Ǫ��
	layerInfo->bType_05_T_JSUPT = false;	// �輭��Ʈ
	layerInfo->bType_05_T_WTST = false;	// ������
	layerInfo->bType_05_T_CLAM = false;	// Ŭ����
	layerInfo->bType_05_T_LUMB = false;	// �����
	layerInfo->bType_05_T_TRUS = false;	// Ʈ����
	layerInfo->bType_05_T_TBBM = false;	// ������
	layerInfo->bType_05_T_BCWF = false;	// �պ�������
	layerInfo->bType_05_T_PLYW = false;	// ����
	layerInfo->bType_05_T_FISP = false;	// �ٷ������̼�
	layerInfo->bType_05_T_STSE = false;	// ���������
	layerInfo->bType_05_T_SLSE = false;	// ������������
	layerInfo->bType_05_T_RAIL = false;	// ��ɷ���
	layerInfo->bType_05_T_AllShow = false;
	
	layerInfo->iType_05_T_TIMB = 0;	// ����
	layerInfo->iType_05_T_BIMJ = 0;	// �����������
	layerInfo->iType_05_T_BDCM = 0;	// ��չ��
	layerInfo->iType_05_T_DMGA = 0;	// �ٸ���
	layerInfo->iType_05_T_RIBL = 0;	// �����
	layerInfo->iType_05_T_BMSP = 0;	// ����ħ�ʷ�
	layerInfo->iType_05_T_BSTA = 0;	// �����
	layerInfo->iType_05_T_SPIP = 0;	// �簢������
	layerInfo->iType_05_T_SUPT = 0;	// ����Ʈ
	layerInfo->iType_05_T_SYSU = 0;	// �ý��ۼ���Ʈ
	layerInfo->iType_05_T_OUTA = 0;	// �ƿ��ڳʾޱ�
	layerInfo->iType_05_T_OUTP = 0;	// �ƿ��ڳ��ǳ�
	layerInfo->iType_05_T_AFOM = 0;	// ����
	layerInfo->iType_05_T_CPIP = 0;	// ����������
	layerInfo->iType_05_T_UGON = 0;	// ��������
	layerInfo->iType_05_T_UFOM = 0;	// ������
	layerInfo->iType_05_T_INCO = 0;	// ���ڳ��ǳ�
	layerInfo->iType_05_T_JOIB = 0;	// ����Ʈ��
	layerInfo->iType_05_T_EFOM = 0;	// ���̰�Ǫ��
	layerInfo->iType_05_T_JSUPT = 0;	// �輭��Ʈ
	layerInfo->iType_05_T_WTST = 0;	// ������
	layerInfo->iType_05_T_CLAM = 0;	// Ŭ����
	layerInfo->iType_05_T_LUMB = 0;	// �����
	layerInfo->iType_05_T_TRUS = 0;	// Ʈ����
	layerInfo->iType_05_T_TBBM = 0;	// ������
	layerInfo->iType_05_T_BCWF = 0;	// �պ�������
	layerInfo->iType_05_T_PLYW = 0;	// ����
	layerInfo->iType_05_T_FISP = 0;	// �ٷ������̼�
	layerInfo->iType_05_T_STSE = 0;	// ���������
	layerInfo->iType_05_T_SLSE = 0;	// ������������
	layerInfo->iType_05_T_RAIL = 0;	// ��ɷ���

	// 06-F ���ü�
	layerInfo->bType_06_F_STRU = false;	// ����H����
	layerInfo->bType_06_F_HFIL = false;	// ����H����
	layerInfo->bType_06_F_SJAK = false;	// ��ũ����
	layerInfo->bType_06_F_PJAK = false;	// �����ε���
	layerInfo->bType_06_F_BRKT = false;	// �����
	layerInfo->bType_06_F_PBKT = false;	// �ǽ� �����
	layerInfo->bType_06_F_CIP = false;	// �븷�� ��ü
	layerInfo->bType_06_F_LAND = false;	// ����
	layerInfo->bType_06_F_ANGL = false;	// �ޱ�
	layerInfo->bType_06_F_ERAC = false;	// ���ݾ�Ŀ
	layerInfo->bType_06_F_LUMB = false;	// �����
	layerInfo->bType_06_F_BPAN = false;	// ������
	layerInfo->bType_06_F_WALE = false;	// ����
	layerInfo->bType_06_F_PILE = false;	// ����
	layerInfo->bType_06_F_AllShow = false;

	layerInfo->iType_06_F_STRU = 0;	// ����H����
	layerInfo->iType_06_F_HFIL = 0;	// ����H����
	layerInfo->iType_06_F_SJAK = 0;	// ��ũ����
	layerInfo->iType_06_F_PJAK = 0;	// �����ε���
	layerInfo->iType_06_F_BRKT = 0;	// �����
	layerInfo->iType_06_F_PBKT = 0;	// �ǽ� �����
	layerInfo->iType_06_F_CIP = 0;	// �븷�� ��ü
	layerInfo->iType_06_F_LAND = 0;	// ����
	layerInfo->iType_06_F_ANGL = 0;	// �ޱ�
	layerInfo->iType_06_F_ERAC = 0;	// ���ݾ�Ŀ
	layerInfo->iType_06_F_LUMB = 0;	// �����
	layerInfo->iType_06_F_BPAN = 0;	// ������
	layerInfo->iType_06_F_WALE = 0;	// ����
	layerInfo->iType_06_F_PILE = 0;	// ����

	// ��ü ����
	// 05-T ������
	layerInfo->bType_05_T_2_TIMB = false;	// ����
	layerInfo->bType_05_T_2_BIMJ = false;	// �����������
	layerInfo->bType_05_T_2_BDCM = false;	// ��չ��
	layerInfo->bType_05_T_2_DMGA = false;	// �ٸ���
	layerInfo->bType_05_T_2_RIBL = false;	// �����
	layerInfo->bType_05_T_2_BMSP = false;	// ����ħ�ʷ�
	layerInfo->bType_05_T_2_BSTA = false;	// �����
	layerInfo->bType_05_T_2_SPIP = false;	// �簢������
	layerInfo->bType_05_T_2_SUPT = false;	// ����Ʈ
	layerInfo->bType_05_T_2_SYSU = false;	// �ý��ۼ���Ʈ
	layerInfo->bType_05_T_2_OUTA = false;	// �ƿ��ڳʾޱ�
	layerInfo->bType_05_T_2_OUTP = false;	// �ƿ��ڳ��ǳ�
	layerInfo->bType_05_T_2_AFOM = false;	// ����
	layerInfo->bType_05_T_2_CPIP = false;	// ����������
	layerInfo->bType_05_T_2_UGON = false;	// ��������
	layerInfo->bType_05_T_2_UFOM = false;	// ������
	layerInfo->bType_05_T_2_INCO = false;	// ���ڳ��ǳ�
	layerInfo->bType_05_T_2_JOIB = false;	// ����Ʈ��
	layerInfo->bType_05_T_2_EFOM = false;	// ���̰�Ǫ��
	layerInfo->bType_05_T_2_JSUPT = false;	// �輭��Ʈ
	layerInfo->bType_05_T_2_WTST = false;	// ������
	layerInfo->bType_05_T_2_CLAM = false;	// Ŭ����
	layerInfo->bType_05_T_2_LUMB = false;	// �����
	layerInfo->bType_05_T_2_TRUS = false;	// Ʈ����
	layerInfo->bType_05_T_2_TBBM = false;	// ������
	layerInfo->bType_05_T_2_BCWF = false;	// �պ�������
	layerInfo->bType_05_T_2_PLYW = false;	// ����
	layerInfo->bType_05_T_2_FISP = false;	// �ٷ������̼�
	layerInfo->bType_05_T_2_STSE = false;	// ���������
	layerInfo->bType_05_T_2_SLSE = false;	// ������������
	layerInfo->bType_05_T_2_RAIL = false;	// ��ɷ���
	layerInfo->bType_05_T_2_AllShow = false;

	layerInfo->iType_05_T_2_TIMB = 0;	// ����
	layerInfo->iType_05_T_2_BIMJ = 0;	// �����������
	layerInfo->iType_05_T_2_BDCM = 0;	// ��չ��
	layerInfo->iType_05_T_2_DMGA = 0;	// �ٸ���
	layerInfo->iType_05_T_2_RIBL = 0;	// �����
	layerInfo->iType_05_T_2_BMSP = 0;	// ����ħ�ʷ�
	layerInfo->iType_05_T_2_BSTA = 0;	// �����
	layerInfo->iType_05_T_2_SPIP = 0;	// �簢������
	layerInfo->iType_05_T_2_SUPT = 0;	// ����Ʈ
	layerInfo->iType_05_T_2_SYSU = 0;	// �ý��ۼ���Ʈ
	layerInfo->iType_05_T_2_OUTA = 0;	// �ƿ��ڳʾޱ�
	layerInfo->iType_05_T_2_OUTP = 0;	// �ƿ��ڳ��ǳ�
	layerInfo->iType_05_T_2_AFOM = 0;	// ����
	layerInfo->iType_05_T_2_CPIP = 0;	// ����������
	layerInfo->iType_05_T_2_UGON = 0;	// ��������
	layerInfo->iType_05_T_2_UFOM = 0;	// ������
	layerInfo->iType_05_T_2_INCO = 0;	// ���ڳ��ǳ�
	layerInfo->iType_05_T_2_JOIB = 0;	// ����Ʈ��
	layerInfo->iType_05_T_2_EFOM = 0;	// ���̰�Ǫ��
	layerInfo->iType_05_T_2_JSUPT = 0;	// �輭��Ʈ
	layerInfo->iType_05_T_2_WTST = 0;	// ������
	layerInfo->iType_05_T_2_CLAM = 0;	// Ŭ����
	layerInfo->iType_05_T_2_LUMB = 0;	// �����
	layerInfo->iType_05_T_2_TRUS = 0;	// Ʈ����
	layerInfo->iType_05_T_2_TBBM = 0;	// ������
	layerInfo->iType_05_T_2_BCWF = 0;	// �պ�������
	layerInfo->iType_05_T_2_PLYW = 0;	// ����
	layerInfo->iType_05_T_2_FISP = 0;	// �ٷ������̼�
	layerInfo->iType_05_T_2_STSE = 0;	// ���������
	layerInfo->iType_05_T_2_SLSE = 0;	// ������������
	layerInfo->iType_05_T_2_RAIL = 0;	// ��ɷ���
}

// ���̾� ���� �����ϱ�
GSErrCode	showLayersEasily (void)
{
	GSErrCode	err = NoError;
	
	API_Attribute	attrib;
	short			xx, i;
	short			nLayers;

	bool	success;
	char	*token;
	char	tempStr [5];
	char	tok1 [5];
	char	tok2 [5];
	char	tok3 [5];
	char	tok4 [5];
	char	tok5 [5];
	char	tok6 [5];
	char	tok7 [5];
	char	tok8 [5];
	char	constructionCode [5];
	int		DongNumber;
	int		CJNumber;
	int		OrderInCJNumber;

	short	result;

	
	// ���̾� ���� �ʱ�ȭ
	initLayerInfo (&layerInfo);
	initLayerInfo (&selectedInfo);

	// ������Ʈ �� ���̾� ������ �˾Ƴ�
	BNZeroMemory (&attrib, sizeof (API_Attribute));
	attrib.header.typeID = API_LayerID;
	err = ACAPI_Attribute_GetNum (API_LayerID, &nLayers);

	for (xx = 1; xx <= nLayers && err == NoError ; ++xx) {
		attrib.header.index = xx;
		err = ACAPI_Attribute_Get (&attrib);
		if (err == NoError) {
			strcpy (tok1, "");
			strcpy (tok2, "");
			strcpy (tok3, "");
			strcpy (tok4, "");
			strcpy (tok5, "");
			strcpy (tok6, "");
			strcpy (tok7, "");
			strcpy (tok8, "");
			i = 1;
			success = false;
			// ���� �ڵ�: 05-T-(0101)-9B1-(01)-(01)-COLU-(UFOM)  ��, ��ȣ ���� ���û���
			// ���̾� �̸��� "-" ���� �������� �ɰ���
			token = strtok (attrib.layer.head.name, "-");
			while (token != NULL) {
				// ���� �� ���� Ȯ��
				// 1�� (�Ϸù�ȣ) - �ʼ� (2����, ����)
				if (i == 1) {
					strcpy (tempStr, token);
					if (strlen (tempStr) == 2) {
						strcpy (tok1, tempStr);
						success = true;
					} else {
						i=100;
						success = false;
					}
				}
				// 2�� (���� ����) - �ʼ� (1����, ����)
				else if (i == 2) {
					strcpy (tempStr, token);
					if (strlen (tempStr) == 1) {
						strcpy (tok2, tempStr);
						success = true;
					} else {
						i=100;
						success = false;
					}
				}
				// 3�� (�� ����) - ���� (4����)
				else if (i == 3) {
					strcpy (tempStr, token);
					// �� ������ ���,
					if (strlen (tempStr) == 4) {
						strcpy (tok3, tempStr);
						success = true;
					// �� ������ ���
					} else if (strlen (tempStr) == 3) {
						strcpy (tok4, tempStr);
						i=4;
						success = true;
					} else {
						i=100;
						success = false;
					}
				}
				// 4�� (�� ����) - �ʼ� (3����)
				else if (i == 4) {
					strcpy (tempStr, token);
					if (strlen (tempStr) == 3) {
						strcpy (tok4, tempStr);
						success = true;
					} else {
						i=100;
						success = false;
					}
				}
				// 5�� (CJ ����) - ���� (2����, ����)
				else if (i == 5) {
					strcpy (tempStr, token);
					if (strlen (tempStr) == 2) {
						strcpy (tok5, tempStr);
						success = true;
					} else if (strlen (tempStr) >= 3) {
						strcpy (tok7, tempStr);
						i=7;
						success = true;
					} else {
						i=100;
						success = false;
					}
				}
				// 6�� (CJ �� �ð�����) - ���� (2����, ����) - �� CJ ������ ������ �̰͵� ������
				else if (i == 6) {
					strcpy (tempStr, token);
					if (strlen (tempStr) == 2) {
						strcpy (tok6, tempStr);
						success = true;
					} else if (strlen (tempStr) >= 3) {
						strcpy (tok7, tempStr);
						i=7;
						success = true;
					} else {
						i=100;
						success = false;
					}
				}
				// 7�� (���� ����) - �ʼ� (3���� �̻�)
				else if (i == 7) {
					strcpy (tempStr, token);
					if (strlen (tempStr) >= 3) {
						strcpy (tok7, tempStr);
						success = true;
					} else {
						success = false;
					}
				}
				// 8�� (��ü ����) - ���� (3���� �̻�)
				else if (i == 8) {
					strcpy (tempStr, token);
					if (strlen (tempStr) >= 3) {
						strcpy (tok8, tempStr);
						success = true;
					} else {
						success = false;
					}
				}
				++i;
				token = strtok (NULL, "-");
			}

			// 8�ܰ���� ���������� �Ϸ�Ǹ� ����ü�� ����
			if (success == true) {
				// �Ϸ� ��ȣ�� ���� ���� ���ڸ� ���� ��ħ
				sprintf (constructionCode, "%s-%s", tok1, tok2);
				
				// 1,2�ܰ�. ���� ���� Ȯ��
				if (strncmp (constructionCode, "01-S", 4) == 0)	layerInfo.bType_01_S = true;
				if (strncmp (constructionCode, "02-A", 4) == 0)	layerInfo.bType_02_A = true;
				if (strncmp (constructionCode, "03-M", 4) == 0)	layerInfo.bType_03_M = true;
				if (strncmp (constructionCode, "04-E", 4) == 0)	layerInfo.bType_04_E = true;
				if (strncmp (constructionCode, "05-T", 4) == 0)	layerInfo.bType_05_T = true;
				if (strncmp (constructionCode, "06-F", 4) == 0)	layerInfo.bType_06_F = true;
				if (strncmp (constructionCode, "07-Q", 4) == 0)	layerInfo.bType_07_Q = true;
				if (strncmp (constructionCode, "08-L", 4) == 0)	layerInfo.bType_08_L = true;
				if (strncmp (constructionCode, "09-C", 4) == 0)	layerInfo.bType_09_C = true;
				if (strncmp (constructionCode, "10-K", 4) == 0)	layerInfo.bType_10_K = true;

				// 3�ܰ�. �� ����
				DongNumber = atoi (tok3);
				if (DongNumber == 0) {
					if (strncmp (tok4, "SHOP", 4) == 0)	{	layerInfo.bDong_SHOP = true;	layerInfo.bDongAllShow = true;	}
					if (strncmp (tok4, "SECU", 4) == 0)	{	layerInfo.bDong_SECU = true;	layerInfo.bDongAllShow = true;	}
				} else {
					if ((DongNumber >= 100) && (DongNumber <= 199)) {	layerInfo.bDong_1xx [DongNumber - 100] = true;		layerInfo.bDongAllShow = true;	}
					if ((DongNumber >= 200) && (DongNumber <= 299)) {	layerInfo.bDong_2xx [DongNumber - 200] = true;		layerInfo.bDongAllShow = true;	}
					if ((DongNumber >= 300) && (DongNumber <= 399)) {	layerInfo.bDong_3xx [DongNumber - 300] = true;		layerInfo.bDongAllShow = true;	}
					if ((DongNumber >= 400) && (DongNumber <= 499)) {	layerInfo.bDong_4xx [DongNumber - 400] = true;		layerInfo.bDongAllShow = true;	}
					if ((DongNumber >= 500) && (DongNumber <= 599)) {	layerInfo.bDong_5xx [DongNumber - 500] = true;		layerInfo.bDongAllShow = true;	}
					if ((DongNumber >= 600) && (DongNumber <= 699)) {	layerInfo.bDong_6xx [DongNumber - 600] = true;		layerInfo.bDongAllShow = true;	}
					if ((DongNumber >= 700) && (DongNumber <= 799)) {	layerInfo.bDong_7xx [DongNumber - 700] = true;		layerInfo.bDongAllShow = true;	}
					if ((DongNumber >= 800) && (DongNumber <= 899)) {	layerInfo.bDong_8xx [DongNumber - 800] = true;		layerInfo.bDongAllShow = true;	}
					if ((DongNumber >= 900) && (DongNumber <= 999)) {	layerInfo.bDong_9xx [DongNumber - 900] = true;		layerInfo.bDongAllShow = true;	}
					if ((DongNumber >= 1000) && (DongNumber <= 1099)) {	layerInfo.bDong_10xx [DongNumber - 1000] = true;	layerInfo.bDongAllShow = true;	}
					if ((DongNumber >= 1100) && (DongNumber <= 1199)) {	layerInfo.bDong_11xx [DongNumber - 1100] = true;	layerInfo.bDongAllShow = true;	}
					if ((DongNumber >= 1200) && (DongNumber <= 1299)) {	layerInfo.bDong_12xx [DongNumber - 1200] = true;	layerInfo.bDongAllShow = true;	}
					if ((DongNumber >= 1300) && (DongNumber <= 1399)) {	layerInfo.bDong_13xx [DongNumber - 1300] = true;	layerInfo.bDongAllShow = true;	}
					if ((DongNumber >= 1400) && (DongNumber <= 1499)) {	layerInfo.bDong_14xx [DongNumber - 1400] = true;	layerInfo.bDongAllShow = true;	}
					if ((DongNumber >= 1500) && (DongNumber <= 1599)) {	layerInfo.bDong_15xx [DongNumber - 1500] = true;	layerInfo.bDongAllShow = true;	}
				}

				// 4�ܰ�. �� ����
				if (strncmp (tok4, "9B1", 3) == 0)	layerInfo.bFloor_Bxx [1] = true;
				if (strncmp (tok4, "8B2", 3) == 0)	layerInfo.bFloor_Bxx [2] = true;
				if (strncmp (tok4, "7B3", 3) == 0)	layerInfo.bFloor_Bxx [3] = true;
				if (strncmp (tok4, "6B4", 3) == 0)	layerInfo.bFloor_Bxx [4] = true;
				if (strncmp (tok4, "5B5", 3) == 0)	layerInfo.bFloor_Bxx [5] = true;
				if (strncmp (tok4, "4B6", 3) == 0)	layerInfo.bFloor_Bxx [6] = true;
				if (strncmp (tok4, "3B7", 3) == 0)	layerInfo.bFloor_Bxx [7] = true;
				if (strncmp (tok4, "2B8", 3) == 0)	layerInfo.bFloor_Bxx [8] = true;
				if (strncmp (tok4, "1B9", 3) == 0)	layerInfo.bFloor_Bxx [9] = true;

				if (strncmp (tok4, "F01", 3) == 0)	layerInfo.bFloor_Fxx [1] = true;
				if (strncmp (tok4, "F02", 3) == 0)	layerInfo.bFloor_Fxx [2] = true;
				if (strncmp (tok4, "F03", 3) == 0)	layerInfo.bFloor_Fxx [3] = true;
				if (strncmp (tok4, "F04", 3) == 0)	layerInfo.bFloor_Fxx [4] = true;
				if (strncmp (tok4, "F05", 3) == 0)	layerInfo.bFloor_Fxx [5] = true;
				if (strncmp (tok4, "F06", 3) == 0)	layerInfo.bFloor_Fxx [6] = true;
				if (strncmp (tok4, "F07", 3) == 0)	layerInfo.bFloor_Fxx [7] = true;
				if (strncmp (tok4, "F08", 3) == 0)	layerInfo.bFloor_Fxx [8] = true;
				if (strncmp (tok4, "F09", 3) == 0)	layerInfo.bFloor_Fxx [9] = true;
				if (strncmp (tok4, "F10", 3) == 0)	layerInfo.bFloor_Fxx [10] = true;
				if (strncmp (tok4, "F11", 3) == 0)	layerInfo.bFloor_Fxx [11] = true;
				if (strncmp (tok4, "F12", 3) == 0)	layerInfo.bFloor_Fxx [12] = true;
				if (strncmp (tok4, "F13", 3) == 0)	layerInfo.bFloor_Fxx [13] = true;
				if (strncmp (tok4, "F14", 3) == 0)	layerInfo.bFloor_Fxx [14] = true;
				if (strncmp (tok4, "F15", 3) == 0)	layerInfo.bFloor_Fxx [15] = true;
				if (strncmp (tok4, "F16", 3) == 0)	layerInfo.bFloor_Fxx [16] = true;
				if (strncmp (tok4, "F17", 3) == 0)	layerInfo.bFloor_Fxx [17] = true;
				if (strncmp (tok4, "F18", 3) == 0)	layerInfo.bFloor_Fxx [18] = true;
				if (strncmp (tok4, "F19", 3) == 0)	layerInfo.bFloor_Fxx [19] = true;
				if (strncmp (tok4, "F20", 3) == 0)	layerInfo.bFloor_Fxx [20] = true;
				if (strncmp (tok4, "F21", 3) == 0)	layerInfo.bFloor_Fxx [21] = true;
				if (strncmp (tok4, "F22", 3) == 0)	layerInfo.bFloor_Fxx [22] = true;
				if (strncmp (tok4, "F23", 3) == 0)	layerInfo.bFloor_Fxx [23] = true;
				if (strncmp (tok4, "F24", 3) == 0)	layerInfo.bFloor_Fxx [24] = true;
				if (strncmp (tok4, "F25", 3) == 0)	layerInfo.bFloor_Fxx [25] = true;
				if (strncmp (tok4, "F26", 3) == 0)	layerInfo.bFloor_Fxx [26] = true;
				if (strncmp (tok4, "F27", 3) == 0)	layerInfo.bFloor_Fxx [27] = true;
				if (strncmp (tok4, "F28", 3) == 0)	layerInfo.bFloor_Fxx [28] = true;
				if (strncmp (tok4, "F29", 3) == 0)	layerInfo.bFloor_Fxx [29] = true;
				if (strncmp (tok4, "F30", 3) == 0)	layerInfo.bFloor_Fxx [30] = true;
				if (strncmp (tok4, "F31", 3) == 0)	layerInfo.bFloor_Fxx [31] = true;
				if (strncmp (tok4, "F32", 3) == 0)	layerInfo.bFloor_Fxx [32] = true;
				if (strncmp (tok4, "F33", 3) == 0)	layerInfo.bFloor_Fxx [33] = true;
				if (strncmp (tok4, "F34", 3) == 0)	layerInfo.bFloor_Fxx [34] = true;
				if (strncmp (tok4, "F35", 3) == 0)	layerInfo.bFloor_Fxx [35] = true;
				if (strncmp (tok4, "F36", 3) == 0)	layerInfo.bFloor_Fxx [36] = true;
				if (strncmp (tok4, "F37", 3) == 0)	layerInfo.bFloor_Fxx [37] = true;
				if (strncmp (tok4, "F38", 3) == 0)	layerInfo.bFloor_Fxx [38] = true;
				if (strncmp (tok4, "F39", 3) == 0)	layerInfo.bFloor_Fxx [39] = true;
				if (strncmp (tok4, "F40", 3) == 0)	layerInfo.bFloor_Fxx [40] = true;
				if (strncmp (tok4, "F41", 3) == 0)	layerInfo.bFloor_Fxx [41] = true;
				if (strncmp (tok4, "F42", 3) == 0)	layerInfo.bFloor_Fxx [42] = true;
				if (strncmp (tok4, "F43", 3) == 0)	layerInfo.bFloor_Fxx [43] = true;
				if (strncmp (tok4, "F44", 3) == 0)	layerInfo.bFloor_Fxx [44] = true;
				if (strncmp (tok4, "F45", 3) == 0)	layerInfo.bFloor_Fxx [45] = true;
				if (strncmp (tok4, "F46", 3) == 0)	layerInfo.bFloor_Fxx [46] = true;
				if (strncmp (tok4, "F47", 3) == 0)	layerInfo.bFloor_Fxx [47] = true;
				if (strncmp (tok4, "F48", 3) == 0)	layerInfo.bFloor_Fxx [48] = true;
				if (strncmp (tok4, "F49", 3) == 0)	layerInfo.bFloor_Fxx [49] = true;
				if (strncmp (tok4, "F50", 3) == 0)	layerInfo.bFloor_Fxx [50] = true;
				if (strncmp (tok4, "F51", 3) == 0)	layerInfo.bFloor_Fxx [51] = true;
				if (strncmp (tok4, "F52", 3) == 0)	layerInfo.bFloor_Fxx [52] = true;
				if (strncmp (tok4, "F53", 3) == 0)	layerInfo.bFloor_Fxx [53] = true;
				if (strncmp (tok4, "F54", 3) == 0)	layerInfo.bFloor_Fxx [54] = true;
				if (strncmp (tok4, "F55", 3) == 0)	layerInfo.bFloor_Fxx [55] = true;
				if (strncmp (tok4, "F56", 3) == 0)	layerInfo.bFloor_Fxx [56] = true;
				if (strncmp (tok4, "F57", 3) == 0)	layerInfo.bFloor_Fxx [57] = true;
				if (strncmp (tok4, "F58", 3) == 0)	layerInfo.bFloor_Fxx [58] = true;
				if (strncmp (tok4, "F59", 3) == 0)	layerInfo.bFloor_Fxx [59] = true;
				if (strncmp (tok4, "F60", 3) == 0)	layerInfo.bFloor_Fxx [60] = true;
				if (strncmp (tok4, "F61", 3) == 0)	layerInfo.bFloor_Fxx [61] = true;
				if (strncmp (tok4, "F62", 3) == 0)	layerInfo.bFloor_Fxx [62] = true;
				if (strncmp (tok4, "F63", 3) == 0)	layerInfo.bFloor_Fxx [63] = true;
				if (strncmp (tok4, "F64", 3) == 0)	layerInfo.bFloor_Fxx [64] = true;
				if (strncmp (tok4, "F65", 3) == 0)	layerInfo.bFloor_Fxx [65] = true;
				if (strncmp (tok4, "F66", 3) == 0)	layerInfo.bFloor_Fxx [66] = true;
				if (strncmp (tok4, "F67", 3) == 0)	layerInfo.bFloor_Fxx [67] = true;
				if (strncmp (tok4, "F68", 3) == 0)	layerInfo.bFloor_Fxx [68] = true;
				if (strncmp (tok4, "F69", 3) == 0)	layerInfo.bFloor_Fxx [69] = true;
				if (strncmp (tok4, "F70", 3) == 0)	layerInfo.bFloor_Fxx [70] = true;
				if (strncmp (tok4, "F71", 3) == 0)	layerInfo.bFloor_Fxx [71] = true;
				if (strncmp (tok4, "F72", 3) == 0)	layerInfo.bFloor_Fxx [72] = true;
				if (strncmp (tok4, "F73", 3) == 0)	layerInfo.bFloor_Fxx [73] = true;
				if (strncmp (tok4, "F74", 3) == 0)	layerInfo.bFloor_Fxx [74] = true;
				if (strncmp (tok4, "F75", 3) == 0)	layerInfo.bFloor_Fxx [75] = true;
				if (strncmp (tok4, "F76", 3) == 0)	layerInfo.bFloor_Fxx [76] = true;
				if (strncmp (tok4, "F77", 3) == 0)	layerInfo.bFloor_Fxx [77] = true;
				if (strncmp (tok4, "F78", 3) == 0)	layerInfo.bFloor_Fxx [78] = true;
				if (strncmp (tok4, "F79", 3) == 0)	layerInfo.bFloor_Fxx [79] = true;
				if (strncmp (tok4, "F80", 3) == 0)	layerInfo.bFloor_Fxx [80] = true;
				if (strncmp (tok4, "F81", 3) == 0)	layerInfo.bFloor_Fxx [81] = true;
				if (strncmp (tok4, "F82", 3) == 0)	layerInfo.bFloor_Fxx [82] = true;
				if (strncmp (tok4, "F83", 3) == 0)	layerInfo.bFloor_Fxx [83] = true;
				if (strncmp (tok4, "F84", 3) == 0)	layerInfo.bFloor_Fxx [84] = true;
				if (strncmp (tok4, "F85", 3) == 0)	layerInfo.bFloor_Fxx [85] = true;
				if (strncmp (tok4, "F86", 3) == 0)	layerInfo.bFloor_Fxx [86] = true;
				if (strncmp (tok4, "F87", 3) == 0)	layerInfo.bFloor_Fxx [87] = true;
				if (strncmp (tok4, "F88", 3) == 0)	layerInfo.bFloor_Fxx [88] = true;
				if (strncmp (tok4, "F89", 3) == 0)	layerInfo.bFloor_Fxx [89] = true;
				if (strncmp (tok4, "F90", 3) == 0)	layerInfo.bFloor_Fxx [90] = true;
				if (strncmp (tok4, "F91", 3) == 0)	layerInfo.bFloor_Fxx [91] = true;
				if (strncmp (tok4, "F92", 3) == 0)	layerInfo.bFloor_Fxx [92] = true;
				if (strncmp (tok4, "F93", 3) == 0)	layerInfo.bFloor_Fxx [93] = true;
				if (strncmp (tok4, "F94", 3) == 0)	layerInfo.bFloor_Fxx [94] = true;
				if (strncmp (tok4, "F95", 3) == 0)	layerInfo.bFloor_Fxx [95] = true;
				if (strncmp (tok4, "F96", 3) == 0)	layerInfo.bFloor_Fxx [96] = true;
				if (strncmp (tok4, "F97", 3) == 0)	layerInfo.bFloor_Fxx [97] = true;
				if (strncmp (tok4, "F98", 3) == 0)	layerInfo.bFloor_Fxx [98] = true;
				if (strncmp (tok4, "F99", 3) == 0)	layerInfo.bFloor_Fxx [99] = true;

				if (strncmp (tok4, "PH1", 3) == 0)	layerInfo.bFloor_PHxx [1] = true;
				if (strncmp (tok4, "PH2", 3) == 0)	layerInfo.bFloor_PHxx [2] = true;
				if (strncmp (tok4, "PH3", 3) == 0)	layerInfo.bFloor_PHxx [3] = true;
				if (strncmp (tok4, "PH4", 3) == 0)	layerInfo.bFloor_PHxx [4] = true;
				if (strncmp (tok4, "PH5", 3) == 0)	layerInfo.bFloor_PHxx [5] = true;
				if (strncmp (tok4, "PH6", 3) == 0)	layerInfo.bFloor_PHxx [6] = true;
				if (strncmp (tok4, "PH7", 3) == 0)	layerInfo.bFloor_PHxx [7] = true;
				if (strncmp (tok4, "PH8", 3) == 0)	layerInfo.bFloor_PHxx [8] = true;
				if (strncmp (tok4, "PH9", 3) == 0)	layerInfo.bFloor_PHxx [9] = true;

				// 5�ܰ�. CJ ����
				CJNumber = atoi (tok5);
				if (CJNumber != 0) {	layerInfo.bCJ [CJNumber] = true;	layerInfo.bCJAllShow = true; }

				// 6�ܰ�. CJ �� �ð�����
				OrderInCJNumber = atoi (tok6);
				if (OrderInCJNumber != 0) {	layerInfo.bOrderInCJ [OrderInCJNumber] = true;	layerInfo.bOrderInCJAllShow = true; }

				// 7�ܰ�. ���� ����
				// 01-S ����
				if ((strncmp (constructionCode, "01-S", 4) == 0) && (strncmp (tok7, "STAR", 4) == 0)) {	layerInfo.bType_01_S_STAR = true;	layerInfo.bType_01_S_AllShow = true; }	// ���
				if ((strncmp (constructionCode, "01-S", 4) == 0) && (strncmp (tok7, "COLU", 4) == 0)) {	layerInfo.bType_01_S_COLU = true;	layerInfo.bType_01_S_AllShow = true; }	// ���
				if ((strncmp (constructionCode, "01-S", 4) == 0) && (strncmp (tok7, "FOOT", 4) == 0)) {	layerInfo.bType_01_S_FOOT = true;	layerInfo.bType_01_S_AllShow = true; }	// ����
				if ((strncmp (constructionCode, "01-S", 4) == 0) && (strncmp (tok7, "WALL", 4) == 0)) {	layerInfo.bType_01_S_WALL = true;	layerInfo.bType_01_S_AllShow = true; }	// ��ü
				if ((strncmp (constructionCode, "01-S", 4) == 0) && (strncmp (tok7, "BEAM", 4) == 0)) {	layerInfo.bType_01_S_BEAM = true;	layerInfo.bType_01_S_AllShow = true; }	// ��
				if ((strncmp (constructionCode, "01-S", 4) == 0) && (strncmp (tok7, "SLAB", 4) == 0)) {	layerInfo.bType_01_S_SLAB = true;	layerInfo.bType_01_S_AllShow = true; }	// ������
				if ((strncmp (constructionCode, "01-S", 4) == 0) && (strncmp (tok7, "CLST", 4) == 0)) {	layerInfo.bType_01_S_CLST = true;	layerInfo.bType_01_S_AllShow = true; }	// ö����
				if ((strncmp (constructionCode, "01-S", 4) == 0) && (strncmp (tok7, "BMST", 4) == 0)) {	layerInfo.bType_01_S_BMST = true;	layerInfo.bType_01_S_AllShow = true; }	// ö��
				if ((strncmp (constructionCode, "01-S", 4) == 0) && (strncmp (tok7, "RAMP", 4) == 0)) {	layerInfo.bType_01_S_RAMP = true;	layerInfo.bType_01_S_AllShow = true; }	// ����
				if ((strncmp (constructionCode, "01-S", 4) == 0) && (strncmp (tok7, "CWAL", 4) == 0)) {	layerInfo.bType_01_S_CWAL = true;	layerInfo.bType_01_S_AllShow = true; }	// �պ�
				if ((strncmp (constructionCode, "01-S", 4) == 0) && (strncmp (tok7, "WTWL", 4) == 0)) {	layerInfo.bType_01_S_WTWL = true;	layerInfo.bType_01_S_AllShow = true; }	// �����
				if ((strncmp (constructionCode, "01-S", 4) == 0) && (strncmp (tok7, "CSTN", 4) == 0)) {	layerInfo.bType_01_S_CSTN = true;	layerInfo.bType_01_S_AllShow = true; }	// ����
				if ((strncmp (constructionCode, "01-S", 4) == 0) && (strncmp (tok7, "MPAD", 4) == 0)) {	layerInfo.bType_01_S_MPAD = true;	layerInfo.bType_01_S_AllShow = true; }	// ����е�
				if ((strncmp (constructionCode, "01-S", 4) == 0) && (strncmp (tok7, "GADN", 4) == 0)) {	layerInfo.bType_01_S_GADN = true;	layerInfo.bType_01_S_AllShow = true; }	// ȭ��
				if ((strncmp (constructionCode, "01-S", 4) == 0) && (strncmp (tok7, "PARA", 4) == 0)) {	layerInfo.bType_01_S_PARA = true;	layerInfo.bType_01_S_AllShow = true; }	// �Ķ���
				if ((strncmp (constructionCode, "01-S", 4) == 0) && (strncmp (tok7, "CLPC", 4) == 0)) {	layerInfo.bType_01_S_CLPC = true;	layerInfo.bType_01_S_AllShow = true; }	// PC���
				if ((strncmp (constructionCode, "01-S", 4) == 0) && (strncmp (tok7, "BMPC", 4) == 0)) {	layerInfo.bType_01_S_BMPC = true;	layerInfo.bType_01_S_AllShow = true; }	// ��PC
				if ((strncmp (constructionCode, "01-S", 4) == 0) && (strncmp (tok7, "BMWL", 4) == 0)) {	layerInfo.bType_01_S_BMWL = true;	layerInfo.bType_01_S_AllShow = true; }	// ����ü
				if ((strncmp (constructionCode, "01-S", 4) == 0) && (strncmp (tok7, "STST", 4) == 0)) {	layerInfo.bType_01_S_STST = true;	layerInfo.bType_01_S_AllShow = true; }	// ö����
				if ((strncmp (constructionCode, "01-S", 4) == 0) && (strncmp (tok7, "TOPP", 4) == 0)) {	layerInfo.bType_01_S_TOPP = true;	layerInfo.bType_01_S_AllShow = true; }	// ����
				if ((strncmp (constructionCode, "01-S", 4) == 0) && (strncmp (tok7, "DECK", 4) == 0)) {	layerInfo.bType_01_S_DECK = true;	layerInfo.bType_01_S_AllShow = true; }	// ��ũ

				// 02-A ���ึ��
				if ((strncmp (constructionCode, "02-A", 4) == 0) && (strncmp (tok7, "FURN", 4) == 0)) {	layerInfo.bType_02_A_FURN = true;	layerInfo.bType_02_A_AllShow = true; }	// ����
				if ((strncmp (constructionCode, "02-A", 4) == 0) && (strncmp (tok7, "INSU", 4) == 0)) {	layerInfo.bType_02_A_INSU = true;	layerInfo.bType_02_A_AllShow = true; }	// �ܿ���
				if ((strncmp (constructionCode, "02-A", 4) == 0) && (strncmp (tok7, "PAIN", 4) == 0)) {	layerInfo.bType_02_A_PAIN = true;	layerInfo.bType_02_A_AllShow = true; }	// ����
				if ((strncmp (constructionCode, "02-A", 4) == 0) && (strncmp (tok7, "MOLD", 4) == 0)) {	layerInfo.bType_02_A_MOLD = true;	layerInfo.bType_02_A_AllShow = true; }	// ����
				if ((strncmp (constructionCode, "02-A", 4) == 0) && (strncmp (tok7, "MORT", 4) == 0)) {	layerInfo.bType_02_A_MORT = true;	layerInfo.bType_02_A_AllShow = true; }	// ��Ż
				if ((strncmp (constructionCode, "02-A", 4) == 0) && (strncmp (tok7, "WATE", 4) == 0)) {	layerInfo.bType_02_A_WATE = true;	layerInfo.bType_02_A_AllShow = true; }	// ���
				if ((strncmp (constructionCode, "02-A", 4) == 0) && (strncmp (tok7, "BRIC", 4) == 0)) {	layerInfo.bType_02_A_BRIC = true;	layerInfo.bType_02_A_AllShow = true; }	// ����
				if ((strncmp (constructionCode, "02-A", 4) == 0) && (strncmp (tok7, "PAPE", 4) == 0)) {	layerInfo.bType_02_A_PAPE = true;	layerInfo.bType_02_A_AllShow = true; }	// ����
				if ((strncmp (constructionCode, "02-A", 4) == 0) && (strncmp (tok7, "BLOC", 4) == 0)) {	layerInfo.bType_02_A_BLOC = true;	layerInfo.bType_02_A_AllShow = true; }	// ���
				if ((strncmp (constructionCode, "02-A", 4) == 0) && (strncmp (tok7, "GYPS", 4) == 0)) {	layerInfo.bType_02_A_GYPS = true;	layerInfo.bType_02_A_AllShow = true; }	// ������
				if ((strncmp (constructionCode, "02-A", 4) == 0) && (strncmp (tok7, "STON", 4) == 0)) {	layerInfo.bType_02_A_STON = true;	layerInfo.bType_02_A_AllShow = true; }	// ����
				if ((strncmp (constructionCode, "02-A", 4) == 0) && (strncmp (tok7, "INTE", 4) == 0)) {	layerInfo.bType_02_A_INTE = true;	layerInfo.bType_02_A_AllShow = true; }	// ����
				if ((strncmp (constructionCode, "02-A", 4) == 0) && (strncmp (tok7, "GLAS", 4) == 0)) {	layerInfo.bType_02_A_GLAS = true;	layerInfo.bType_02_A_AllShow = true; }	// ����
				if ((strncmp (constructionCode, "02-A", 4) == 0) && (strncmp (tok7, "HARD", 4) == 0)) {	layerInfo.bType_02_A_HARD = true;	layerInfo.bType_02_A_AllShow = true; }	// ö��
				if ((strncmp (constructionCode, "02-A", 4) == 0) && (strncmp (tok7, "TILE", 4) == 0)) {	layerInfo.bType_02_A_TILE = true;	layerInfo.bType_02_A_AllShow = true; }	// Ÿ��
				if ((strncmp (constructionCode, "02-A", 4) == 0) && (strncmp (tok7, "PANE", 4) == 0)) {	layerInfo.bType_02_A_PANE = true;	layerInfo.bType_02_A_AllShow = true; }	// �ǳ�
				if ((strncmp (constructionCode, "02-A", 4) == 0) && (strncmp (tok7, "PLYW", 4) == 0)) {	layerInfo.bType_02_A_PLYW = true;	layerInfo.bType_02_A_AllShow = true; }	// ����
				if ((strncmp (constructionCode, "02-A", 4) == 0) && (strncmp (tok7, "PCON", 4) == 0)) {	layerInfo.bType_02_A_PCON = true;	layerInfo.bType_02_A_AllShow = true; }	// ������ũ��Ʈ

				// 05-T ������
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "TIMB", 4) == 0)) {	layerInfo.bType_05_T_TIMB = true;	layerInfo.bType_05_T_AllShow = true; }	// ����
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "BIMJ", 4) == 0)) {	layerInfo.bType_05_T_BIMJ = true;	layerInfo.bType_05_T_AllShow = true; }	// �����������
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "BDCM", 4) == 0)) {	layerInfo.bType_05_T_BDCM = true;	layerInfo.bType_05_T_AllShow = true; }	// ��չ��
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "DMGA", 4) == 0)) {	layerInfo.bType_05_T_DMGA = true;	layerInfo.bType_05_T_AllShow = true; }	// �ٸ���
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "RIBL", 4) == 0)) {	layerInfo.bType_05_T_RIBL = true;	layerInfo.bType_05_T_AllShow = true; }	// �����
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "BMSP", 4) == 0)) {	layerInfo.bType_05_T_BMSP = true;	layerInfo.bType_05_T_AllShow = true; }	// ����ħ�ʷ�
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "BSTA", 4) == 0)) {	layerInfo.bType_05_T_BSTA = true;	layerInfo.bType_05_T_AllShow = true; }	// �����
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "SPIP", 4) == 0)) {	layerInfo.bType_05_T_SPIP = true;	layerInfo.bType_05_T_AllShow = true; }	// �簢������
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "SUPT", 4) == 0)) {	layerInfo.bType_05_T_SUPT = true;	layerInfo.bType_05_T_AllShow = true; }	// ����Ʈ
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "SYSU", 4) == 0)) {	layerInfo.bType_05_T_SYSU = true;	layerInfo.bType_05_T_AllShow = true; }	// �ý��ۼ���Ʈ
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "OUTA", 4) == 0)) {	layerInfo.bType_05_T_OUTA = true;	layerInfo.bType_05_T_AllShow = true; }	// �ƿ��ڳʾޱ�
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "OUTP", 4) == 0)) {	layerInfo.bType_05_T_OUTP = true;	layerInfo.bType_05_T_AllShow = true; }	// �ƿ��ڳ��ǳ�
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "AFOM", 4) == 0)) {	layerInfo.bType_05_T_AFOM = true;	layerInfo.bType_05_T_AllShow = true; }	// ����
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "CPIP", 4) == 0)) {	layerInfo.bType_05_T_CPIP = true;	layerInfo.bType_05_T_AllShow = true; }	// ����������
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "UGON", 4) == 0)) {	layerInfo.bType_05_T_UGON = true;	layerInfo.bType_05_T_AllShow = true; }	// ��������
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "UFOM", 4) == 0)) {	layerInfo.bType_05_T_UFOM = true;	layerInfo.bType_05_T_AllShow = true; }	// ������
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "INCO", 4) == 0)) {	layerInfo.bType_05_T_INCO = true;	layerInfo.bType_05_T_AllShow = true; }	// ���ڳ��ǳ�
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "JOIB", 4) == 0)) {	layerInfo.bType_05_T_JOIB = true;	layerInfo.bType_05_T_AllShow = true; }	// ����Ʈ��
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "EFOM", 4) == 0)) {	layerInfo.bType_05_T_EFOM = true;	layerInfo.bType_05_T_AllShow = true; }	// ���̰�Ǫ��
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "JSUPT", 5) == 0)) {layerInfo.bType_05_T_JSUPT = true;	layerInfo.bType_05_T_AllShow = true; }	// �輭��Ʈ
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "WTST", 4) == 0)) {	layerInfo.bType_05_T_WTST = true;	layerInfo.bType_05_T_AllShow = true; }	// ������
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "CLAM", 4) == 0)) {	layerInfo.bType_05_T_CLAM = true;	layerInfo.bType_05_T_AllShow = true; }	// Ŭ����
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "LUMB", 4) == 0)) {	layerInfo.bType_05_T_LUMB = true;	layerInfo.bType_05_T_AllShow = true; }	// �����
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "TRUS", 4) == 0)) {	layerInfo.bType_05_T_TRUS = true;	layerInfo.bType_05_T_AllShow = true; }	// Ʈ����
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "TBBM", 4) == 0)) {	layerInfo.bType_05_T_TBBM = true;	layerInfo.bType_05_T_AllShow = true; }	// ������
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "BCWF", 4) == 0)) {	layerInfo.bType_05_T_BCWF = true;	layerInfo.bType_05_T_AllShow = true; }	// �պ�������
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "PLYW", 4) == 0)) {	layerInfo.bType_05_T_PLYW = true;	layerInfo.bType_05_T_AllShow = true; }	// ����
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "FISP", 4) == 0)) {	layerInfo.bType_05_T_FISP = true;	layerInfo.bType_05_T_AllShow = true; }	// �ٷ������̼�
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "STSE", 4) == 0)) {	layerInfo.bType_05_T_STSE = true;	layerInfo.bType_05_T_AllShow = true; }	// ���������
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "SLSE", 4) == 0)) {	layerInfo.bType_05_T_SLSE = true;	layerInfo.bType_05_T_AllShow = true; }	// ������������
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok7, "RAIL", 4) == 0)) {	layerInfo.bType_05_T_RAIL = true;	layerInfo.bType_05_T_AllShow = true; }	// ��ɷ���

				// 06-F ���ü�
				if ((strncmp (constructionCode, "06-F", 4) == 0) && (strncmp (tok7, "STRU", 4) == 0)) {	layerInfo.bType_06_F_STRU = true;	layerInfo.bType_06_F_AllShow = true; }	// ����H����
				if ((strncmp (constructionCode, "06-F", 4) == 0) && (strncmp (tok7, "HFIL", 4) == 0)) {	layerInfo.bType_06_F_HFIL = true;	layerInfo.bType_06_F_AllShow = true; }	// ����H����
				if ((strncmp (constructionCode, "06-F", 4) == 0) && (strncmp (tok7, "SJAK", 4) == 0)) {	layerInfo.bType_06_F_SJAK = true;	layerInfo.bType_06_F_AllShow = true; }	// ��ũ����
				if ((strncmp (constructionCode, "06-F", 4) == 0) && (strncmp (tok7, "PJAK", 4) == 0)) {	layerInfo.bType_06_F_PJAK = true;	layerInfo.bType_06_F_AllShow = true; }	// �����ε���
				if ((strncmp (constructionCode, "06-F", 4) == 0) && (strncmp (tok7, "BRKT", 4) == 0)) {	layerInfo.bType_06_F_BRKT = true;	layerInfo.bType_06_F_AllShow = true; }	// �����
				if ((strncmp (constructionCode, "06-F", 4) == 0) && (strncmp (tok7, "PBKT", 4) == 0)) {	layerInfo.bType_06_F_PBKT = true;	layerInfo.bType_06_F_AllShow = true; }	// �ǽ� �����
				if ((strncmp (constructionCode, "06-F", 4) == 0) && (strncmp (tok7, "CIP", 3) == 0)) {	layerInfo.bType_06_F_CIP = true;	layerInfo.bType_06_F_AllShow = true; }	// �븷�� ��ü
				if ((strncmp (constructionCode, "06-F", 4) == 0) && (strncmp (tok7, "LAND", 4) == 0)) {	layerInfo.bType_06_F_LAND = true;	layerInfo.bType_06_F_AllShow = true; }	// ����
				if ((strncmp (constructionCode, "06-F", 4) == 0) && (strncmp (tok7, "ANGL", 4) == 0)) {	layerInfo.bType_06_F_ANGL = true;	layerInfo.bType_06_F_AllShow = true; }	// �ޱ�
				if ((strncmp (constructionCode, "06-F", 4) == 0) && (strncmp (tok7, "ERAC", 4) == 0)) {	layerInfo.bType_06_F_ERAC = true;	layerInfo.bType_06_F_AllShow = true; }	// ���ݾ�Ŀ
				if ((strncmp (constructionCode, "06-F", 4) == 0) && (strncmp (tok7, "LUMB", 4) == 0)) {	layerInfo.bType_06_F_LUMB = true;	layerInfo.bType_06_F_AllShow = true; }	// ������
				if ((strncmp (constructionCode, "06-F", 4) == 0) && (strncmp (tok7, "BPAN", 4) == 0)) {	layerInfo.bType_06_F_BPAN = true;	layerInfo.bType_06_F_AllShow = true; }	// ������
				if ((strncmp (constructionCode, "06-F", 4) == 0) && (strncmp (tok7, "WALE", 4) == 0)) {	layerInfo.bType_06_F_WALE = true;	layerInfo.bType_06_F_AllShow = true; }	// ����
				if ((strncmp (constructionCode, "06-F", 4) == 0) && (strncmp (tok7, "PILE", 4) == 0)) {	layerInfo.bType_06_F_PILE = true;	layerInfo.bType_06_F_AllShow = true; }	// ����

				// 8�ܰ�. ��ü ���� (�����翡 ����)
				// 05-T ������
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok8, "TIMB", 4) == 0)) {	layerInfo.bType_05_T_2_TIMB = true;	layerInfo.bType_05_T_2_AllShow = true; }	// ����
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok8, "BIMJ", 4) == 0)) {	layerInfo.bType_05_T_2_BIMJ = true;	layerInfo.bType_05_T_2_AllShow = true; }	// �����������
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok8, "BDCM", 4) == 0)) {	layerInfo.bType_05_T_2_BDCM = true;	layerInfo.bType_05_T_2_AllShow = true; }	// ��չ��
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok8, "DMGA", 4) == 0)) {	layerInfo.bType_05_T_2_DMGA = true;	layerInfo.bType_05_T_2_AllShow = true; }	// �ٸ���
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok8, "RIBL", 4) == 0)) {	layerInfo.bType_05_T_2_RIBL = true;	layerInfo.bType_05_T_2_AllShow = true; }	// �����
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok8, "BMSP", 4) == 0)) {	layerInfo.bType_05_T_2_BMSP = true;	layerInfo.bType_05_T_2_AllShow = true; }	// ����ħ�ʷ�
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok8, "BSTA", 4) == 0)) {	layerInfo.bType_05_T_2_BSTA = true;	layerInfo.bType_05_T_2_AllShow = true; }	// �����
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok8, "SPIP", 4) == 0)) {	layerInfo.bType_05_T_2_SPIP = true;	layerInfo.bType_05_T_2_AllShow = true; }	// �簢������
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok8, "SUPT", 4) == 0)) {	layerInfo.bType_05_T_2_SUPT = true;	layerInfo.bType_05_T_2_AllShow = true; }	// ����Ʈ
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok8, "SYSU", 4) == 0)) {	layerInfo.bType_05_T_2_SYSU = true;	layerInfo.bType_05_T_2_AllShow = true; }	// �ý��ۼ���Ʈ
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok8, "OUTA", 4) == 0)) {	layerInfo.bType_05_T_2_OUTA = true;	layerInfo.bType_05_T_2_AllShow = true; }	// �ƿ��ڳʾޱ�
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok8, "OUTP", 4) == 0)) {	layerInfo.bType_05_T_2_OUTP = true;	layerInfo.bType_05_T_2_AllShow = true; }	// �ƿ��ڳ��ǳ�
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok8, "AFOM", 4) == 0)) {	layerInfo.bType_05_T_2_AFOM = true;	layerInfo.bType_05_T_2_AllShow = true; }	// ����
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok8, "CPIP", 4) == 0)) {	layerInfo.bType_05_T_2_CPIP = true;	layerInfo.bType_05_T_2_AllShow = true; }	// ����������
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok8, "UGON", 4) == 0)) {	layerInfo.bType_05_T_2_UGON = true;	layerInfo.bType_05_T_2_AllShow = true; }	// ��������
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok8, "UFOM", 4) == 0)) {	layerInfo.bType_05_T_2_UFOM = true;	layerInfo.bType_05_T_2_AllShow = true; }	// ������
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok8, "INCO", 4) == 0)) {	layerInfo.bType_05_T_2_INCO = true;	layerInfo.bType_05_T_2_AllShow = true; }	// ���ڳ��ǳ�
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok8, "JOIB", 4) == 0)) {	layerInfo.bType_05_T_2_JOIB = true;	layerInfo.bType_05_T_2_AllShow = true; }	// ����Ʈ��
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok8, "EFOM", 4) == 0)) {	layerInfo.bType_05_T_2_EFOM = true;	layerInfo.bType_05_T_2_AllShow = true; }	// ���̰�Ǫ��
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok8, "JSUPT", 5) == 0)) {layerInfo.bType_05_T_2_JSUPT = true;layerInfo.bType_05_T_2_AllShow = true; }	// �輭��Ʈ
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok8, "WTST", 4) == 0)) {	layerInfo.bType_05_T_2_WTST = true;	layerInfo.bType_05_T_2_AllShow = true; }	// ������
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok8, "CLAM", 4) == 0)) {	layerInfo.bType_05_T_2_CLAM = true;	layerInfo.bType_05_T_2_AllShow = true; }	// Ŭ����
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok8, "LUMB", 4) == 0)) {	layerInfo.bType_05_T_2_LUMB = true;	layerInfo.bType_05_T_2_AllShow = true; }	// �����
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok8, "TRUS", 4) == 0)) {	layerInfo.bType_05_T_2_TRUS = true;	layerInfo.bType_05_T_2_AllShow = true; }	// Ʈ����
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok8, "TBBM", 4) == 0)) {	layerInfo.bType_05_T_2_TBBM = true;	layerInfo.bType_05_T_2_AllShow = true; }	// ������
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok8, "BCWF", 4) == 0)) {	layerInfo.bType_05_T_2_BCWF = true;	layerInfo.bType_05_T_2_AllShow = true; }	// �պ�������
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok8, "PLYW", 4) == 0)) {	layerInfo.bType_05_T_2_PLYW = true;	layerInfo.bType_05_T_2_AllShow = true; }	// ����
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok8, "FISP", 4) == 0)) {	layerInfo.bType_05_T_2_FISP = true;	layerInfo.bType_05_T_2_AllShow = true; }	// �ٷ������̼�
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok8, "STSE", 4) == 0)) {	layerInfo.bType_05_T_2_STSE = true;	layerInfo.bType_05_T_2_AllShow = true; }	// ���������
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok8, "SLSE", 4) == 0)) {	layerInfo.bType_05_T_2_SLSE = true;	layerInfo.bType_05_T_2_AllShow = true; }	// ������������
				if ((strncmp (constructionCode, "05-T", 4) == 0) && (strncmp (tok8, "RAIL", 4) == 0)) {	layerInfo.bType_05_T_2_RAIL = true;	layerInfo.bType_05_T_2_AllShow = true; }	// ��ɷ���
			}
		}
		if (err == APIERR_DELETED)
			err = NoError;
	}

	// [���̾�α� �ڽ�] ���̾� �����ֱ�
	result = DGBlankModalDialog (700, 450, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, layerShowHandler, 0);

	// ��� ���̾� �����
	BNZeroMemory (&attrib, sizeof (API_Attribute));
	attrib.header.typeID = API_LayerID;
	
	if (result == DG_OK) {
		for (xx = 1; xx <= nLayers ; ++xx) {
			attrib.header.index = xx;
			err = ACAPI_Attribute_Get (&attrib);
			if (err == NoError) {
				//if ((attrib.layer.head.flags & APILay_Hidden) == false) {
					attrib.layer.head.flags |= APILay_Hidden;
					ACAPI_Attribute_Modify (&attrib, NULL);
				//}
			}
		}
	}

	short	z;
	char	code1 [10][5];		// ���� �ڵ�
	short	LenCode1;
	char	code2 [1600][5];	// �� �ڵ�
	short	LenCode2;
	char	code3 [120][5];		// �� �ڵ�
	short	LenCode3;
	char	code4 [100][5];		// CJ �ڵ�
	short	LenCode4;
	char	code5 [100][5];		// CJ �� �ð����� �ڵ�
	short	LenCode5;
	char	code6 [90][5];		// ���� �ڵ�
	short	LenCode6;
	char	code7 [90][5];		// ��ü �ڵ�
	short	LenCode7;

	char	fullLayerName [40];
	short	x1, x2, x3, x4, x5, x6, x7, x8;
	bool	bCode2, bCode4, bCode5, bCode7;

	// 1. ���� �ڵ� ���ڿ� �����
	z = 0;
	if (selectedInfo.bType_01_S == true)	strcpy (code1 [z++], "01-S");
	if (selectedInfo.bType_02_A == true)	strcpy (code1 [z++], "02-A");
	if (selectedInfo.bType_03_M == true)	strcpy (code1 [z++], "03-M");
	if (selectedInfo.bType_04_E == true)	strcpy (code1 [z++], "04-E");
	if (selectedInfo.bType_05_T == true)	strcpy (code1 [z++], "05-T");
	if (selectedInfo.bType_06_F == true)	strcpy (code1 [z++], "06-F");
	if (selectedInfo.bType_07_Q == true)	strcpy (code1 [z++], "07-Q");
	if (selectedInfo.bType_08_L == true)	strcpy (code1 [z++], "08-L");
	if (selectedInfo.bType_09_C == true)	strcpy (code1 [z++], "09-C");
	if (selectedInfo.bType_10_K == true)	strcpy (code1 [z++], "10-K");
	LenCode1 = z;

	// 2. �� �ڵ� ���ڿ� �����
	z = 0;
	strcpy (code2 [z++], "????");
	for (xx = 1 ; xx < 100 ; ++xx) {
		if (selectedInfo.bDong_1xx [xx] == true)	sprintf (code2 [z++], "%04d", 100 + xx);
		if (selectedInfo.bDong_2xx [xx] == true)	sprintf (code2 [z++], "%04d", 200 + xx);
		if (selectedInfo.bDong_3xx [xx] == true)	sprintf (code2 [z++], "%04d", 300 + xx);
		if (selectedInfo.bDong_4xx [xx] == true)	sprintf (code2 [z++], "%04d", 400 + xx);
		if (selectedInfo.bDong_5xx [xx] == true)	sprintf (code2 [z++], "%04d", 500 + xx);
		if (selectedInfo.bDong_6xx [xx] == true)	sprintf (code2 [z++], "%04d", 600 + xx);
		if (selectedInfo.bDong_7xx [xx] == true)	sprintf (code2 [z++], "%04d", 700 + xx);
		if (selectedInfo.bDong_8xx [xx] == true)	sprintf (code2 [z++], "%04d", 800 + xx);
		if (selectedInfo.bDong_9xx [xx] == true)	sprintf (code2 [z++], "%04d", 900 + xx);
		if (selectedInfo.bDong_10xx [xx] == true)	sprintf (code2 [z++], "%04d", 1000 + xx);
		if (selectedInfo.bDong_11xx [xx] == true)	sprintf (code2 [z++], "%04d", 1100 + xx);
		if (selectedInfo.bDong_12xx [xx] == true)	sprintf (code2 [z++], "%04d", 1200 + xx);
		if (selectedInfo.bDong_13xx [xx] == true)	sprintf (code2 [z++], "%04d", 1300 + xx);
		if (selectedInfo.bDong_14xx [xx] == true)	sprintf (code2 [z++], "%04d", 1400 + xx);
		if (selectedInfo.bDong_15xx [xx] == true)	sprintf (code2 [z++], "%04d", 1500 + xx);
	}
	if (selectedInfo.bDong_SHOP == true)	strcpy (code2 [z++], "SHOP");
	if (selectedInfo.bDong_SECU == true)	strcpy (code2 [z++], "SECU");
	LenCode2 = z;

	// 3. �� �ڵ� ���ڿ� �����
	z = 0;
	if (selectedInfo.bFloor_Bxx [1] == true)	strcpy (code3 [z++], "9B1");
	if (selectedInfo.bFloor_Bxx [2] == true)	strcpy (code3 [z++], "8B2");
	if (selectedInfo.bFloor_Bxx [3] == true)	strcpy (code3 [z++], "7B3");
	if (selectedInfo.bFloor_Bxx [4] == true)	strcpy (code3 [z++], "6B4");
	if (selectedInfo.bFloor_Bxx [5] == true)	strcpy (code3 [z++], "5B5");
	if (selectedInfo.bFloor_Bxx [6] == true)	strcpy (code3 [z++], "4B6");
	if (selectedInfo.bFloor_Bxx [7] == true)	strcpy (code3 [z++], "3B7");
	if (selectedInfo.bFloor_Bxx [8] == true)	strcpy (code3 [z++], "2B8");
	if (selectedInfo.bFloor_Bxx [9] == true)	strcpy (code3 [z++], "1B9");
	for (xx = 1 ; xx < 100 ; ++xx) {
		if (selectedInfo.bFloor_Fxx [xx] == true)	sprintf (code3 [z++], "F%02d", xx);
	}
	for (xx = 1 ; xx < 10 ; ++xx) {
		if (selectedInfo.bFloor_PHxx [xx] == true)	sprintf (code3 [z++], "PH%1d", xx);
	}
	LenCode3 = z;

	// 4. CJ �ڵ� ���ڿ� �����
	z = 0;
	strcpy (code4 [z++], "??");
	for (xx = 1 ; xx < 100 ; ++xx) {
		if (selectedInfo.bCJ [xx] == true)	sprintf (code4 [z++], "%02d", xx);
	}
	LenCode4 = z;

	// 5. CJ �� �ð����� ���ڿ� �����
	z = 0;
	strcpy (code5 [z++], "??");
	for (xx = 1 ; xx < 100 ; ++xx) {
		if (selectedInfo.bOrderInCJ [xx] == true)	sprintf (code5 [z++], "%02d", xx);
	}
	LenCode5 = z;

	// 6. ���� �ڵ� ���ڿ� �����
	z = 0;
	if (selectedInfo.bType_01_S_STAR == true)	strcpy (code6 [z++], "STAR");
	if (selectedInfo.bType_01_S_COLU == true)	strcpy (code6 [z++], "COLU");
	if (selectedInfo.bType_01_S_FOOT == true)	strcpy (code6 [z++], "FOOT");
	if (selectedInfo.bType_01_S_WALL == true)	strcpy (code6 [z++], "WALL");
	if (selectedInfo.bType_01_S_BEAM == true)	strcpy (code6 [z++], "BEAM");
	if (selectedInfo.bType_01_S_SLAB == true)	strcpy (code6 [z++], "SLAB");
	if (selectedInfo.bType_01_S_CLST == true)	strcpy (code6 [z++], "CLST");
	if (selectedInfo.bType_01_S_BMST == true)	strcpy (code6 [z++], "BMST");
	if (selectedInfo.bType_01_S_RAMP == true)	strcpy (code6 [z++], "RAMP");
	if (selectedInfo.bType_01_S_CWAL == true)	strcpy (code6 [z++], "CWAL");
	if (selectedInfo.bType_01_S_WTWL == true)	strcpy (code6 [z++], "WTWL");
	if (selectedInfo.bType_01_S_CSTN == true)	strcpy (code6 [z++], "CSTN");
	if (selectedInfo.bType_01_S_MPAD == true)	strcpy (code6 [z++], "MPAD");
	if (selectedInfo.bType_01_S_GADN == true)	strcpy (code6 [z++], "GADN");
	if (selectedInfo.bType_01_S_PARA == true)	strcpy (code6 [z++], "PARA");
	if (selectedInfo.bType_01_S_CLPC == true)	strcpy (code6 [z++], "CLPC");
	if (selectedInfo.bType_01_S_BMPC == true)	strcpy (code6 [z++], "BMPC");
	if (selectedInfo.bType_01_S_BMWL == true)	strcpy (code6 [z++], "BMWL");
	if (selectedInfo.bType_01_S_STST == true)	strcpy (code6 [z++], "STST");
	if (selectedInfo.bType_01_S_TOPP == true)	strcpy (code6 [z++], "TOPP");
	if (selectedInfo.bType_01_S_DECK == true)	strcpy (code6 [z++], "DECK");

	if (selectedInfo.bType_02_A_FURN == true)	strcpy (code6 [z++], "FURN");
	if (selectedInfo.bType_02_A_INSU == true)	strcpy (code6 [z++], "INSU");
	if (selectedInfo.bType_02_A_PAIN == true)	strcpy (code6 [z++], "PAIN");
	if (selectedInfo.bType_02_A_MOLD == true)	strcpy (code6 [z++], "MOLD");
	if (selectedInfo.bType_02_A_MORT == true)	strcpy (code6 [z++], "MORT");
	if (selectedInfo.bType_02_A_WATE == true)	strcpy (code6 [z++], "WATE");
	if (selectedInfo.bType_02_A_BRIC == true)	strcpy (code6 [z++], "BRIC");
	if (selectedInfo.bType_02_A_PAPE == true)	strcpy (code6 [z++], "PAPE");
	if (selectedInfo.bType_02_A_BLOC == true)	strcpy (code6 [z++], "BLOC");
	if (selectedInfo.bType_02_A_GYPS == true)	strcpy (code6 [z++], "GYPS");
	if (selectedInfo.bType_02_A_STON == true)	strcpy (code6 [z++], "STON");
	if (selectedInfo.bType_02_A_INTE == true)	strcpy (code6 [z++], "INTE");
	if (selectedInfo.bType_02_A_GLAS == true)	strcpy (code6 [z++], "GLAS");
	if (selectedInfo.bType_02_A_HARD == true)	strcpy (code6 [z++], "HARD");
	if (selectedInfo.bType_02_A_TILE == true)	strcpy (code6 [z++], "TILE");
	if (selectedInfo.bType_02_A_PANE == true)	strcpy (code6 [z++], "PANE");
	if (selectedInfo.bType_02_A_PLYW == true)	strcpy (code6 [z++], "PLYW");
	if (selectedInfo.bType_02_A_PCON == true)	strcpy (code6 [z++], "PCON");

	if (selectedInfo.bType_05_T_TIMB == true)	strcpy (code6 [z++], "TIMB");
	if (selectedInfo.bType_05_T_BIMJ == true)	strcpy (code6 [z++], "BIMJ");
	if (selectedInfo.bType_05_T_BDCM == true)	strcpy (code6 [z++], "BDCM");
	if (selectedInfo.bType_05_T_DMGA == true)	strcpy (code6 [z++], "DMGA");
	if (selectedInfo.bType_05_T_RIBL == true)	strcpy (code6 [z++], "RIBL");
	if (selectedInfo.bType_05_T_BMSP == true)	strcpy (code6 [z++], "BMSP");
	if (selectedInfo.bType_05_T_BSTA == true)	strcpy (code6 [z++], "BSTA");
	if (selectedInfo.bType_05_T_SPIP == true)	strcpy (code6 [z++], "SPIP");
	if (selectedInfo.bType_05_T_SUPT == true)	strcpy (code6 [z++], "SUPT");
	if (selectedInfo.bType_05_T_SYSU == true)	strcpy (code6 [z++], "SYSU");
	if (selectedInfo.bType_05_T_OUTA == true)	strcpy (code6 [z++], "OUTA");
	if (selectedInfo.bType_05_T_OUTP == true)	strcpy (code6 [z++], "OUTP");
	if (selectedInfo.bType_05_T_AFOM == true)	strcpy (code6 [z++], "AFOM");
	if (selectedInfo.bType_05_T_CPIP == true)	strcpy (code6 [z++], "CPIP");
	if (selectedInfo.bType_05_T_UGON == true)	strcpy (code6 [z++], "UGON");
	if (selectedInfo.bType_05_T_UFOM == true)	strcpy (code6 [z++], "UFOM");
	if (selectedInfo.bType_05_T_INCO == true)	strcpy (code6 [z++], "INCO");
	if (selectedInfo.bType_05_T_JOIB == true)	strcpy (code6 [z++], "JOIB");
	if (selectedInfo.bType_05_T_EFOM == true)	strcpy (code6 [z++], "EFOM");
	if (selectedInfo.bType_05_T_JSUPT == true)	strcpy (code6 [z++], "JSUPT");
	if (selectedInfo.bType_05_T_WTST == true)	strcpy (code6 [z++], "WTST");
	if (selectedInfo.bType_05_T_CLAM == true)	strcpy (code6 [z++], "CLAM");
	if (selectedInfo.bType_05_T_LUMB == true)	strcpy (code6 [z++], "LUMB");
	if (selectedInfo.bType_05_T_TRUS == true)	strcpy (code6 [z++], "TRUS");
	if (selectedInfo.bType_05_T_TBBM == true)	strcpy (code6 [z++], "TBBM");
	if (selectedInfo.bType_05_T_BCWF == true)	strcpy (code6 [z++], "BCWF");
	if (selectedInfo.bType_05_T_PLYW == true)	strcpy (code6 [z++], "PLYW");
	if (selectedInfo.bType_05_T_FISP == true)	strcpy (code6 [z++], "FISP");
	if (selectedInfo.bType_05_T_STSE == true)	strcpy (code6 [z++], "STSE");
	if (selectedInfo.bType_05_T_SLSE == true)	strcpy (code6 [z++], "SLSE");
	if (selectedInfo.bType_05_T_RAIL == true)	strcpy (code6 [z++], "RAIL");

	if (selectedInfo.bType_06_F_STRU == true)	strcpy (code6 [z++], "STRU");
	if (selectedInfo.bType_06_F_HFIL == true)	strcpy (code6 [z++], "HFIL");
	if (selectedInfo.bType_06_F_SJAK == true)	strcpy (code6 [z++], "SJAK");
	if (selectedInfo.bType_06_F_PJAK == true)	strcpy (code6 [z++], "PJAK");
	if (selectedInfo.bType_06_F_BRKT == true)	strcpy (code6 [z++], "BRKT");
	if (selectedInfo.bType_06_F_PBKT == true)	strcpy (code6 [z++], "PBKT");
	if (selectedInfo.bType_06_F_CIP == true)	strcpy (code6 [z++], "CIP");
	if (selectedInfo.bType_06_F_LAND == true)	strcpy (code6 [z++], "LAND");
	if (selectedInfo.bType_06_F_ANGL == true)	strcpy (code6 [z++], "ANGL");
	if (selectedInfo.bType_06_F_ERAC == true)	strcpy (code6 [z++], "ERAC");
	if (selectedInfo.bType_06_F_LUMB == true)	strcpy (code6 [z++], "LUMB");
	if (selectedInfo.bType_06_F_BPAN == true)	strcpy (code6 [z++], "BPAN");
	if (selectedInfo.bType_06_F_WALE == true)	strcpy (code6 [z++], "WALE");
	if (selectedInfo.bType_06_F_PILE == true)	strcpy (code6 [z++], "PILE");
	LenCode6 = z;

	// 7. ��ü �ڵ� ���ڿ� ����� (�����翡 ����)
	z = 0;
	strcpy (code7 [z++], "????");
	if (selectedInfo.bType_05_T_2_TIMB == true)	strcpy (code7 [z++], "TIMB");
	if (selectedInfo.bType_05_T_2_BIMJ == true)	strcpy (code7 [z++], "BIMJ");
	if (selectedInfo.bType_05_T_2_BDCM == true)	strcpy (code7 [z++], "BDCM");
	if (selectedInfo.bType_05_T_2_DMGA == true)	strcpy (code7 [z++], "DMGA");
	if (selectedInfo.bType_05_T_2_RIBL == true)	strcpy (code7 [z++], "RIBL");
	if (selectedInfo.bType_05_T_2_BMSP == true)	strcpy (code7 [z++], "BMSP");
	if (selectedInfo.bType_05_T_2_BSTA == true)	strcpy (code7 [z++], "BSTA");
	if (selectedInfo.bType_05_T_2_SPIP == true)	strcpy (code7 [z++], "SPIP");
	if (selectedInfo.bType_05_T_2_SUPT == true)	strcpy (code7 [z++], "SUPT");
	if (selectedInfo.bType_05_T_2_SYSU == true)	strcpy (code7 [z++], "SYSU");
	if (selectedInfo.bType_05_T_2_OUTA == true)	strcpy (code7 [z++], "OUTA");
	if (selectedInfo.bType_05_T_2_OUTP == true)	strcpy (code7 [z++], "OUTP");
	if (selectedInfo.bType_05_T_2_AFOM == true)	strcpy (code7 [z++], "AFOM");
	if (selectedInfo.bType_05_T_2_CPIP == true)	strcpy (code7 [z++], "CPIP");
	if (selectedInfo.bType_05_T_2_UGON == true)	strcpy (code7 [z++], "UGON");
	if (selectedInfo.bType_05_T_2_UFOM == true)	strcpy (code7 [z++], "UFOM");
	if (selectedInfo.bType_05_T_2_INCO == true)	strcpy (code7 [z++], "INCO");
	if (selectedInfo.bType_05_T_2_JOIB == true)	strcpy (code7 [z++], "JOIB");
	if (selectedInfo.bType_05_T_2_EFOM == true)	strcpy (code7 [z++], "EFOM");
	if (selectedInfo.bType_05_T_2_JSUPT == true)strcpy (code7 [z++], "JSUPT");
	if (selectedInfo.bType_05_T_2_WTST == true)	strcpy (code7 [z++], "WTST");
	if (selectedInfo.bType_05_T_2_CLAM == true)	strcpy (code7 [z++], "CLAM");
	if (selectedInfo.bType_05_T_2_LUMB == true)	strcpy (code7 [z++], "LUMB");
	if (selectedInfo.bType_05_T_2_TRUS == true)	strcpy (code7 [z++], "TRUS");
	if (selectedInfo.bType_05_T_2_TBBM == true)	strcpy (code7 [z++], "TBBM");
	if (selectedInfo.bType_05_T_2_BCWF == true)	strcpy (code7 [z++], "BCWF");
	if (selectedInfo.bType_05_T_2_PLYW == true)	strcpy (code7 [z++], "PLYW");
	if (selectedInfo.bType_05_T_2_FISP == true)	strcpy (code7 [z++], "FISP");
	if (selectedInfo.bType_05_T_2_STSE == true)	strcpy (code7 [z++], "STSE");
	if (selectedInfo.bType_05_T_2_SLSE == true)	strcpy (code7 [z++], "SLSE");
	if (selectedInfo.bType_05_T_2_RAIL == true)	strcpy (code7 [z++], "RAIL");
	LenCode7 = z;

	bCode2 = false;	bCode4 = false;	bCode5 = false; bCode7 = false;

	// ���̾� �̸� �����ϱ�
	for (x1 = 0 ; x1 < LenCode1 ; ++x1) {
		for (x2 = 0 ; x2 < LenCode2 ; ++x2) {
			for (x3 = 0 ; x3 < LenCode3 ; ++x3) {
				for (x4 = 0 ; x4 < LenCode4 ; ++x4) {
					for (x5 = 0 ; x5 < LenCode5 ; ++x5) {
						for (x6 = 0 ; x6 < LenCode6 ; ++x6) {
							for (x7 = 0 ; x7 < LenCode7 ; ++x7) {
								for (x8 = 1 ; x8 <= 12 ; ++x8) {
									// bCode2 (�� �ڵ� ����), bCode4 (CJ �ڵ� ����), bCode5 (CJ �� �ð����� �ڵ� ����), bCode7 (���� ���� ��ü �ڵ� ����)
									if (x8 == 1) { bCode2 = false;	bCode4 = false;	bCode5 = false;	bCode7 = false;	}
									if (x8 == 2) { bCode2 = false;	bCode4 = true;	bCode5 = false;	bCode7 = false;	}
									if (x8 == 3) { bCode2 = false;	bCode4 = true;	bCode5 = true;	bCode7 = false;	}
									if (x8 == 4) { bCode2 = true;	bCode4 = false;	bCode5 = false;	bCode7 = false;	}
									if (x8 == 5) { bCode2 = true;	bCode4 = true;	bCode5 = false;	bCode7 = false;	}
									if (x8 == 6) { bCode2 = true;	bCode4 = true;	bCode5 = true;	bCode7 = false;	}

									if (x8 == 7) { bCode2 = false;	bCode4 = false;	bCode5 = false;	bCode7 = true;	}
									if (x8 == 8) { bCode2 = false;	bCode4 = true;	bCode5 = false;	bCode7 = true;	}
									if (x8 == 9) { bCode2 = false;	bCode4 = true;	bCode5 = true;	bCode7 = true;	}
									if (x8 == 10) { bCode2 = true;	bCode4 = false;	bCode5 = false;	bCode7 = true;	}
									if (x8 == 11) { bCode2 = true;	bCode4 = true;	bCode5 = false;	bCode7 = true;	}
									if (x8 == 12) { bCode2 = true;	bCode4 = true;	bCode5 = true;	bCode7 = true;	}

									// ���� ����
									strcpy (fullLayerName, "");
									strcpy (fullLayerName, code1 [x1]);

									// �� ����
									if ((LenCode2 > 1) && (bCode2 == true)) {
										strcat (fullLayerName, "-");
										strcat (fullLayerName, code2 [x2]);
									}

									// �� ����
									strcat (fullLayerName, "-");
									strcat (fullLayerName, code3 [x3]);

									// CJ ����
									if (bCode4 == true) {
										if (LenCode4 > 1) {
											strcat (fullLayerName, "-");
											strcat (fullLayerName, code4 [x4]);
										} else {
											strcat (fullLayerName, "-00");
										}
									}

									// CJ �� �ð�����
									if (bCode5 == true) {
										if (LenCode5 > 1) {
											strcat (fullLayerName, "-");
											strcat (fullLayerName, code5 [x5]);
										} else {
											strcat (fullLayerName, "-00");
										}
									}

									// ����
									strcat (fullLayerName, "-");
									strcat (fullLayerName, code6 [x6]);

									// ��ü
									if (bCode7 == true) {
										if (LenCode7 > 1) {
											strcat (fullLayerName, "-");
											strcat (fullLayerName, code7 [x7]);
										} else {
											// ������ ����
										}
									}

									// !!!
									//ACAPI_WriteReport (fullLayerName, true);

									// ������ ���̾� �̸� �˻��ϱ�
									BNZeroMemory (&attrib, sizeof (API_Attribute));
									attrib.header.typeID = API_LayerID;
									CHCopyC (fullLayerName, attrib.header.name);
									err = ACAPI_Attribute_Get (&attrib);

									// �ش� ���̾� �����ֱ�
									if ((attrib.layer.head.flags & APILay_Hidden) == true) {
										attrib.layer.head.flags ^= APILay_Hidden;
										ACAPI_Attribute_Modify (&attrib, NULL);
									}
								}
							}
						}
					}
				}
			}
		}
	}
	
	ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
	bool	regenerate = true;
	ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

	return err;
}

// [���̾�α� �ڽ�] ���̾� �����ֱ�
short DGCALLBACK layerShowHandler (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	short	itmIdx;
	short	itmPosX, itmPosY;
	short	xx;
	char	tempStr [20];
	short	dialogSizeX, dialogSizeY;

	switch (message) {
		case DG_MSG_INIT:
			// ���̾�α� Ÿ��Ʋ
			DGSetDialogTitle (dialogID, "���̾� ���� �����ϱ�: LayerName Ex) 05-T-(0101)-9B1-(01)-(01)-COLU-(UFOM)");

			// Ȯ�� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 0, 20, 40, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, "Ȯ��");
			DGShowItem (dialogID, DG_OK);

			// ��� ��ư
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 0, 50, 40, 25);
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, "���");
			DGShowItem (dialogID, DG_CANCEL);

			// ��: ���� ����
			itmPosX = 40;
			itmPosY = 25;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 85, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "���� ����");
			DGShowItem (dialogID, itmIdx);

			// üũ�ڽ�: ���� ���� ��ư
			itmPosX = 150;
			itmPosY = 20;
			if (layerInfo.bType_01_S) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "01-S ����");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_01_S = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_02_A) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "02-A ���ึ��");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_02_A = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_03_M) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "03-M ��輳��");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_03_M = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_04_E) {	
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "04-E ���⼳��");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_04_E = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "05-T ������");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_06_F) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "06-F ���ü�");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_06_F = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_07_Q) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "07-Q ��������");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_07_Q = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_08_L) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "08-L ����");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_08_L = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_09_C) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "09-C ���");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_09_C = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_10_K) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "10-K �Ǽ����");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_10_K = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			// ��� ����
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
			DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_BOLD);
			DGSetItemText (dialogID, itmIdx, "��� ����");
			DGShowItem (dialogID, itmIdx);
			SELECTALL_1_CONTYPE = itmIdx;
			itmPosX += 100;
			if (itmPosX >= 600) {
				itmPosX = 150;
				itmPosY += 30;
			}

			itmPosY += 30;

			// ��: �� ����
			itmPosX = 40;
			itmPosY += 10;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 85, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "�� ����");
			DGShowItem (dialogID, itmIdx);

			// üũ�ڽ�: �� ���� ��ư
			itmPosX = 150;
			itmPosY -= 5;
			for (xx = 1 ; xx < 100 ; ++xx) {
				if (layerInfo.bDong_1xx [xx]) {
					sprintf (tempStr, "%d��", 100 + xx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);
					layerInfo.iDong_1xx [xx] = itmIdx;

					itmPosX += 100;
					if (itmPosX >= 600) {
						itmPosX = 150;
						itmPosY += 30;
					}
				}
			}
			for (xx = 1 ; xx < 100 ; ++xx) {
				if (layerInfo.bDong_2xx [xx]) {
					sprintf (tempStr, "%d��", 200 + xx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);
					layerInfo.iDong_2xx [xx] = itmIdx;

					itmPosX += 100;
					if (itmPosX >= 600) {
						itmPosX = 150;
						itmPosY += 30;
					}
				}
			}
			for (xx = 1 ; xx < 100 ; ++xx) {
				if (layerInfo.bDong_3xx [xx]) {
					sprintf (tempStr, "%d��", 300 + xx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);
					layerInfo.iDong_3xx [xx] = itmIdx;

					itmPosX += 100;
					if (itmPosX >= 600) {
						itmPosX = 150;
						itmPosY += 30;
					}
				}
			}
			for (xx = 1 ; xx < 100 ; ++xx) {
				if (layerInfo.bDong_4xx [xx]) {
					sprintf (tempStr, "%d��", 400 + xx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);
					layerInfo.iDong_4xx [xx] = itmIdx;

					itmPosX += 100;
					if (itmPosX >= 600) {
						itmPosX = 150;
						itmPosY += 30;
					}
				}
			}
			for (xx = 1 ; xx < 100 ; ++xx) {
				if (layerInfo.bDong_5xx [xx]) {
					sprintf (tempStr, "%d��", 500 + xx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);
					layerInfo.iDong_5xx [xx] = itmIdx;

					itmPosX += 100;
					if (itmPosX >= 600) {
						itmPosX = 150;
						itmPosY += 30;
					}
				}
			}
			for (xx = 1 ; xx < 100 ; ++xx) {
				if (layerInfo.bDong_6xx [xx]) {
					sprintf (tempStr, "%d��", 600 + xx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);
					layerInfo.iDong_6xx [xx] = itmIdx;

					itmPosX += 100;
					if (itmPosX >= 600) {
						itmPosX = 150;
						itmPosY += 30;
					}
				}
			}
			for (xx = 1 ; xx < 100 ; ++xx) {
				if (layerInfo.bDong_7xx [xx]) {
					sprintf (tempStr, "%d��", 700 + xx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);
					layerInfo.iDong_7xx [xx] = itmIdx;

					itmPosX += 100;
					if (itmPosX >= 600) {
						itmPosX = 150;
						itmPosY += 30;
					}
				}
			}
			for (xx = 1 ; xx < 100 ; ++xx) {
				if (layerInfo.bDong_8xx [xx]) {
					sprintf (tempStr, "%d��", 800 + xx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);
					layerInfo.iDong_8xx [xx] = itmIdx;

					itmPosX += 100;
					if (itmPosX >= 600) {
						itmPosX = 150;
						itmPosY += 30;
					}
				}
			}
			for (xx = 1 ; xx < 100 ; ++xx) {
				if (layerInfo.bDong_9xx [xx]) {
					sprintf (tempStr, "%d��", 900 + xx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);
					layerInfo.iDong_9xx [xx] = itmIdx;

					itmPosX += 100;
					if (itmPosX >= 600) {
						itmPosX = 150;
						itmPosY += 30;
					}
				}
			}
			for (xx = 1 ; xx < 100 ; ++xx) {
				if (layerInfo.bDong_10xx [xx]) {
					sprintf (tempStr, "%d��", 1000 + xx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);
					layerInfo.iDong_10xx [xx] = itmIdx;

					itmPosX += 100;
					if (itmPosX >= 600) {
						itmPosX = 150;
						itmPosY += 30;
					}
				}
			}
			for (xx = 1 ; xx < 100 ; ++xx) {
				if (layerInfo.bDong_11xx [xx]) {
					sprintf (tempStr, "%d��", 1100 + xx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);
					layerInfo.iDong_11xx [xx] = itmIdx;

					itmPosX += 100;
					if (itmPosX >= 600) {
						itmPosX = 150;
						itmPosY += 30;
					}
				}
			}
			for (xx = 1 ; xx < 100 ; ++xx) {
				if (layerInfo.bDong_12xx [xx]) {
					sprintf (tempStr, "%d��", 1200 + xx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);
					layerInfo.iDong_12xx [xx] = itmIdx;

					itmPosX += 100;
					if (itmPosX >= 600) {
						itmPosX = 150;
						itmPosY += 30;
					}
				}
			}
			for (xx = 1 ; xx < 100 ; ++xx) {
				if (layerInfo.bDong_13xx [xx]) {
					sprintf (tempStr, "%d��", 1300 + xx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);
					layerInfo.iDong_13xx [xx] = itmIdx;

					itmPosX += 100;
					if (itmPosX >= 600) {
						itmPosX = 150;
						itmPosY += 30;
					}
				}
			}
			for (xx = 1 ; xx < 100 ; ++xx) {
				if (layerInfo.bDong_14xx [xx]) {
					sprintf (tempStr, "%d��", 1400 + xx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);
					layerInfo.iDong_14xx [xx] = itmIdx;

					itmPosX += 100;
					if (itmPosX >= 600) {
						itmPosX = 150;
						itmPosY += 30;
					}
				}
			}
			for (xx = 1 ; xx < 100 ; ++xx) {
				if (layerInfo.bDong_15xx [xx]) {
					sprintf (tempStr, "%d��", 1500 + xx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);
					layerInfo.iDong_15xx [xx] = itmIdx;

					itmPosX += 100;
					if (itmPosX >= 600) {
						itmPosX = 150;
						itmPosY += 30;
					}
				}
			}
			if (layerInfo.bDong_SHOP) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "�ٸ�����");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iDong_SHOP = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bDong_SECU) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iDong_SECU = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			// ��� ����
			if (layerInfo.bDongAllShow) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_BOLD);
				DGSetItemText (dialogID, itmIdx, "��� ����");
				DGShowItem (dialogID, itmIdx);
				SELECTALL_2_DONG = itmIdx;
				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}

			itmPosY += 30;

			// ��: �� ����
			itmPosX = 40;
			itmPosY += 10;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 85, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "�� ����");
			DGShowItem (dialogID, itmIdx);

			// üũ�ڽ�: �� ���� ��ư
			itmPosX = 150;
			itmPosY -= 5;
			for (xx = 9 ; xx >= 1 ; --xx) {
				if (layerInfo.bFloor_Bxx [xx]) {
					sprintf (tempStr, "����%d��", xx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);
					layerInfo.iFloor_Bxx [xx] = itmIdx;

					itmPosX += 100;
					if (itmPosX >= 600) {
						itmPosX = 150;
						itmPosY += 30;
					}
				}
			}
			for (xx = 1 ; xx < 100 ; ++xx) {
				if (layerInfo.bFloor_Fxx [xx]) {
					sprintf (tempStr, "����%d��", xx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);
					layerInfo.iFloor_Fxx [xx] = itmIdx;

					itmPosX += 100;
					if (itmPosX >= 600) {
						itmPosX = 150;
						itmPosY += 30;
					}
				}
			}
			for (xx = 1 ; xx < 10 ; ++xx) {
				if (layerInfo.bFloor_PHxx [xx]) {
					sprintf (tempStr, "PH%d��", xx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);
					layerInfo.iFloor_PHxx [xx] = itmIdx;

					itmPosX += 100;
					if (itmPosX >= 600) {
						itmPosX = 150;
						itmPosY += 30;
					}
				}
			}
			// ��� ����
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
			DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_BOLD);
			DGSetItemText (dialogID, itmIdx, "��� ����");
			DGShowItem (dialogID, itmIdx);
			SELECTALL_3_FLOOR = itmIdx;
			itmPosX += 100;
			if (itmPosX >= 600) {
				itmPosX = 150;
				itmPosY += 30;
			}

			itmPosY += 30;

			// ��: CJ ����
			itmPosX = 40;
			itmPosY += 10;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 85, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "CJ ����");
			DGShowItem (dialogID, itmIdx);

			// üũ�ڽ�: CJ ���� ��ư
			itmPosX = 150;
			itmPosY -= 5;
			for (xx = 1 ; xx < 100 ; ++xx) {
				if (layerInfo.bCJ [xx]) {
					sprintf (tempStr, "%02d", xx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);
					layerInfo.iCJ [xx] = itmIdx;

					itmPosX += 100;
					if (itmPosX >= 600) {
						itmPosX = 150;
						itmPosY += 30;
					}
				}
			}
			// ��� ����
			if (layerInfo.bCJAllShow) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_BOLD);
				DGSetItemText (dialogID, itmIdx, "��� ����");
				DGShowItem (dialogID, itmIdx);
				SELECTALL_4_CJ = itmIdx;
				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}

			itmPosY += 30;

			// ��: CJ �� �ð�����
			itmPosX = 40;
			itmPosY += 10;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 85, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "�ð�����");
			DGShowItem (dialogID, itmIdx);

			// üũ�ڽ�: CJ �� �ð����� ��ư
			itmPosX = 150;
			itmPosY -= 5;
			for (xx = 1 ; xx < 100 ; ++xx) {
				if (layerInfo.bOrderInCJ [xx]) {
					sprintf (tempStr, "%02d", xx);
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);
					layerInfo.iOrderInCJ [xx] = itmIdx;

					itmPosX += 100;
					if (itmPosX >= 600) {
						itmPosX = 150;
						itmPosY += 30;
					}
				}
			}
			// ��� ����
			if (layerInfo.bOrderInCJAllShow) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_BOLD);
				DGSetItemText (dialogID, itmIdx, "��� ����");
				DGShowItem (dialogID, itmIdx);
				SELECTALL_5_ORDER = itmIdx;
				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}

			itmPosY += 30;

			// ��: ����(����)
			itmPosX = 40;
			itmPosY += 10;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 85, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "*����");
			DGShowItem (dialogID, itmIdx);

			// üũ�ڽ�: ����(����)
			itmPosX = 150;
			itmPosY -= 5;
			if (layerInfo.bType_01_S_STAR) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "���");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_01_S_STAR = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_01_S_COLU) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "���");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_01_S_COLU = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_01_S_FOOT) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_01_S_FOOT = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_01_S_WALL) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "��ü");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_01_S_WALL = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_01_S_BEAM) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "��");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_01_S_BEAM = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_01_S_SLAB) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "������");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_01_S_SLAB = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_01_S_CLST) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "ö����");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_01_S_CLST = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_01_S_BMST) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "ö��");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_01_S_BMST = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_01_S_RAMP) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_01_S_RAMP = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_01_S_CWAL) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "�պ�");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_01_S_CWAL = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_01_S_WTWL) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "�����");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_01_S_WTWL = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_01_S_CSTN) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_01_S_CSTN = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_01_S_MPAD) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����е�");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_01_S_MPAD = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_01_S_GADN) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "ȭ��");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_01_S_GADN = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_01_S_PARA) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "�Ķ���");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_01_S_PARA = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_01_S_CLPC) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "PC���");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_01_S_CLPC = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_01_S_BMPC) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "��PC");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_01_S_BMPC = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_01_S_BMWL) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����ü");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_01_S_BMWL = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_01_S_STST) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "ö����");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_01_S_STST = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_01_S_TOPP) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_01_S_TOPP = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_01_S_DECK) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "��ũ");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_01_S_DECK = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			// ��� ����
			if (layerInfo.bType_01_S_AllShow) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_BOLD);
				DGSetItemText (dialogID, itmIdx, "��� ����");
				DGShowItem (dialogID, itmIdx);
				SELECTALL_6_OBJ_01_S = itmIdx;
				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}

			itmPosY += 30;

			// ��: ����(���ึ��)
			itmPosX = 40;
			itmPosY += 10;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 85, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "*���ึ��");
			DGShowItem (dialogID, itmIdx);

			// üũ�ڽ�: ����(���ึ��)
			itmPosX = 150;
			itmPosY -= 5;
			if (layerInfo.bType_02_A_FURN) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_02_A_FURN = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_02_A_INSU) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "�ܿ���");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_02_A_INSU = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_02_A_PAIN) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_02_A_PAIN = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_02_A_MOLD) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_02_A_MOLD = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_02_A_MORT) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "��Ż");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_02_A_MORT = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_02_A_WATE) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "���");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_02_A_WATE = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_02_A_BRIC) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_02_A_BRIC = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_02_A_PAPE) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_02_A_PAPE = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_02_A_BLOC) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "���");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_02_A_BLOC = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_02_A_GYPS) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "������");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_02_A_GYPS = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_02_A_STON) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_02_A_STON = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_02_A_INTE) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_02_A_INTE = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_02_A_GLAS) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_02_A_GLAS = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_02_A_HARD) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "ö��");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_02_A_HARD = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_02_A_TILE) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "Ÿ��");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_02_A_TILE = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_02_A_PANE) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "�ǳ�");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_02_A_PANE = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_02_A_PLYW) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_02_A_PLYW = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_02_A_PCON) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "������ũ��Ʈ");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_02_A_PCON = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			// ��� ����
			if (layerInfo.bType_02_A_AllShow) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_BOLD);
				DGSetItemText (dialogID, itmIdx, "��� ����");
				DGShowItem (dialogID, itmIdx);
				SELECTALL_6_OBJ_02_A = itmIdx;
				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}

			itmPosY += 30;

			// ��: ����(������)
			itmPosX = 40;
			itmPosY += 10;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 85, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "*������");
			DGShowItem (dialogID, itmIdx);

			// üũ�ڽ�: ����(������)
			itmPosX = 150;
			itmPosY -= 5;
			if (layerInfo.bType_05_T_TIMB) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_TIMB = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_BIMJ) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "�����������");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_BIMJ = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_BDCM) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "��չ��");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_BDCM = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_DMGA) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "�ٸ���");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_DMGA = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_RIBL) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "�����");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_RIBL = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_BMSP) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����ħ�ʷ�");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_BMSP = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_BSTA) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "�����");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_BSTA = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_SPIP) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "�簢������");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_SPIP = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_SUPT) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����Ʈ");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_SUPT = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_SYSU) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "�ý��ۼ���Ʈ");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_SYSU = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_OUTA) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "�ƿ��ڳʾޱ�");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_OUTA = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_OUTP) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "�ƿ��ڳ��ǳ�");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_OUTP = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_AFOM) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_AFOM = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_CPIP) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����������");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_CPIP = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_UGON) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "��������");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_UGON = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_UFOM) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "������");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_UFOM = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_INCO) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "���ڳ��ǳ�");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_INCO = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_JOIB) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����Ʈ��");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_JOIB = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_EFOM) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "���̰�Ǫ��");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_EFOM = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_JSUPT) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "�輭��Ʈ");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_JSUPT = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_WTST) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "������");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_WTST = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_CLAM) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "Ŭ����");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_CLAM = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_LUMB) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "�����");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_LUMB = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_TRUS) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "Ʈ����");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_TRUS = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_TBBM) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "������");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_TBBM = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_BCWF) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "�պ�������");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_BCWF = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_PLYW) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_PLYW = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_FISP) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "�ٷ������̼�");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_FISP = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_STSE) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "���������");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_STSE = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_SLSE) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "������������");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_SLSE = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_RAIL) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "��ɷ���");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_RAIL = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			// ��� ����
			if (layerInfo.bType_05_T_AllShow) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_BOLD);
				DGSetItemText (dialogID, itmIdx, "��� ����");
				DGShowItem (dialogID, itmIdx);
				SELECTALL_6_OBJ_05_T = itmIdx;
				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}

			itmPosY += 30;

			// ��: ����(���ü�)
			itmPosX = 40;
			itmPosY += 10;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 85, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "*���ü�");
			DGShowItem (dialogID, itmIdx);

			// üũ�ڽ�: ����(���ü�)
			itmPosX = 150;
			itmPosY -= 5;
			if (layerInfo.bType_06_F_STRU) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����H����");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_06_F_STRU = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_06_F_HFIL) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����H����");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_06_F_HFIL = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_06_F_SJAK) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "��ũ����");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_06_F_SJAK = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_06_F_PJAK) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "�����ε���");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_06_F_PJAK = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_06_F_BRKT) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "�����");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_06_F_BRKT = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_06_F_PBKT) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "�ǽ� �����");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_06_F_PBKT = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_06_F_CIP) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "�븷�� ��ü");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_06_F_CIP = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_06_F_LAND) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_06_F_LAND = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_06_F_ANGL) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "�ޱ�");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_06_F_ANGL = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_06_F_ERAC) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "���ݾ�Ŀ");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_06_F_ERAC = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_06_F_LUMB) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "�����");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_06_F_LUMB = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_06_F_BPAN) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "������");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_06_F_BPAN = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_06_F_WALE) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_06_F_WALE = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_06_F_PILE) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_06_F_PILE = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			// ��� ����
			if (layerInfo.bType_06_F_AllShow) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_BOLD);
				DGSetItemText (dialogID, itmIdx, "��� ����");
				DGShowItem (dialogID, itmIdx);
				SELECTALL_6_OBJ_06_F = itmIdx;
				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}

			itmPosY += 30;

			// ��: ��ü(������)
			itmPosX = 40;
			itmPosY += 10;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 85, 23);
			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itmIdx, "**������(����)");
			DGShowItem (dialogID, itmIdx);

			// üũ�ڽ�: ��ü(������)
			itmPosX = 150;
			itmPosY -= 5;
			if (layerInfo.bType_05_T_2_TIMB) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_2_TIMB = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_2_BIMJ) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "�����������");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_2_BIMJ = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_2_BDCM) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "��չ��");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_2_BDCM = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_2_DMGA) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "�ٸ���");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_2_DMGA = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_2_RIBL) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "�����");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_2_RIBL = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_2_BMSP) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����ħ�ʷ�");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_2_BMSP = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_2_BSTA) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "�����");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_2_BSTA = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_2_SPIP) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "�簢������");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_2_SPIP = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_2_SUPT) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����Ʈ");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_2_SUPT = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_2_SYSU) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "�ý��ۼ���Ʈ");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_2_SYSU = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_2_OUTA) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "�ƿ��ڳʾޱ�");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_2_OUTA = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_2_OUTP) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "�ƿ��ڳ��ǳ�");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_2_OUTP = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_2_AFOM) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_2_AFOM = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_2_CPIP) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����������");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_2_CPIP = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_2_UGON) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "��������");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_2_UGON = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_2_UFOM) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "������");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_2_UFOM = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_2_INCO) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "���ڳ��ǳ�");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_2_INCO = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_2_JOIB) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����Ʈ��");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_2_JOIB = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_2_EFOM) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "���̰�Ǫ��");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_2_EFOM = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_2_JSUPT) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "�輭��Ʈ");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_2_JSUPT = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_2_WTST) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "������");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_2_WTST = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_2_CLAM) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "Ŭ����");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_2_CLAM = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_2_LUMB) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "�����");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_2_LUMB = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_2_TRUS) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "Ʈ����");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_2_TRUS = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_2_TBBM) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "������");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_2_TBBM = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_2_BCWF) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "�պ�������");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_2_BCWF = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_2_PLYW) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "����");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_2_PLYW = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_2_FISP) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "�ٷ������̼�");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_2_FISP = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_2_STSE) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "���������");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_2_STSE = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_2_SLSE) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "������������");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_2_SLSE = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			if (layerInfo.bType_05_T_2_RAIL) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "��ɷ���");
				DGShowItem (dialogID, itmIdx);
				layerInfo.iType_05_T_2_RAIL = itmIdx;

				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}
			// ��� ����
			if (layerInfo.bType_05_T_2_AllShow) {
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_BOLD);
				DGSetItemText (dialogID, itmIdx, "��� ����");
				DGShowItem (dialogID, itmIdx);
				SELECTALL_6_OBJ_05_T_2 = itmIdx;
				itmPosX += 100;
				if (itmPosX >= 600) {
					itmPosX = 150;
					itmPosY += 30;
				}
			}

			dialogSizeX = 700;
			dialogSizeY = itmPosY + 150;
			DGSetDialogSize (dialogID, DG_FRAME, dialogSizeX, dialogSizeY, DG_TOPLEFT, true);

			break;
		
		case DG_MSG_CHANGE:
			changedBtnItemIdx = item;
			item = 0;	// �׸��� ��ư�� ������ �� â�� ������ �ʰ� ��

			if (changedBtnItemIdx == SELECTALL_1_CONTYPE) {
				if (DGGetItemValLong (dialogID, SELECTALL_1_CONTYPE) == TRUE) {
					// ��� ����
					DGSetItemValLong (dialogID, layerInfo.iType_01_S, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_02_A, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_03_M, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_04_E, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_06_F, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_07_Q, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_08_L, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_09_C, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_10_K, TRUE);
				} else {
					// ��� ����
					DGSetItemValLong (dialogID, layerInfo.iType_01_S, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_02_A, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_03_M, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_04_E, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_06_F, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_07_Q, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_08_L, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_09_C, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_10_K, FALSE);
				}
			}
			if (changedBtnItemIdx == SELECTALL_2_DONG) {
				if (DGGetItemValLong (dialogID, SELECTALL_2_DONG) == TRUE) {
					// ��� ����
					for (xx = 1 ; xx < 100 ; ++xx) {
						DGSetItemValLong (dialogID, layerInfo.iDong_1xx [xx], TRUE);
						DGSetItemValLong (dialogID, layerInfo.iDong_2xx [xx], TRUE);
						DGSetItemValLong (dialogID, layerInfo.iDong_3xx [xx], TRUE);
						DGSetItemValLong (dialogID, layerInfo.iDong_4xx [xx], TRUE);
						DGSetItemValLong (dialogID, layerInfo.iDong_5xx [xx], TRUE);
						DGSetItemValLong (dialogID, layerInfo.iDong_6xx [xx], TRUE);
						DGSetItemValLong (dialogID, layerInfo.iDong_7xx [xx], TRUE);
						DGSetItemValLong (dialogID, layerInfo.iDong_8xx [xx], TRUE);
						DGSetItemValLong (dialogID, layerInfo.iDong_9xx [xx], TRUE);
						DGSetItemValLong (dialogID, layerInfo.iDong_10xx [xx], TRUE);
						DGSetItemValLong (dialogID, layerInfo.iDong_11xx [xx], TRUE);
						DGSetItemValLong (dialogID, layerInfo.iDong_12xx [xx], TRUE);
						DGSetItemValLong (dialogID, layerInfo.iDong_13xx [xx], TRUE);
						DGSetItemValLong (dialogID, layerInfo.iDong_14xx [xx], TRUE);
						DGSetItemValLong (dialogID, layerInfo.iDong_15xx [xx], TRUE);
					}
					DGSetItemValLong (dialogID, layerInfo.iDong_SHOP, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iDong_SECU, TRUE);
				} else {
					// ��� ����
					for (xx = 1 ; xx < 100 ; ++xx) {
						DGSetItemValLong (dialogID, layerInfo.iDong_1xx [xx], FALSE);
						DGSetItemValLong (dialogID, layerInfo.iDong_2xx [xx], FALSE);
						DGSetItemValLong (dialogID, layerInfo.iDong_3xx [xx], FALSE);
						DGSetItemValLong (dialogID, layerInfo.iDong_4xx [xx], FALSE);
						DGSetItemValLong (dialogID, layerInfo.iDong_5xx [xx], FALSE);
						DGSetItemValLong (dialogID, layerInfo.iDong_6xx [xx], FALSE);
						DGSetItemValLong (dialogID, layerInfo.iDong_7xx [xx], FALSE);
						DGSetItemValLong (dialogID, layerInfo.iDong_8xx [xx], FALSE);
						DGSetItemValLong (dialogID, layerInfo.iDong_9xx [xx], FALSE);
						DGSetItemValLong (dialogID, layerInfo.iDong_10xx [xx], FALSE);
						DGSetItemValLong (dialogID, layerInfo.iDong_11xx [xx], FALSE);
						DGSetItemValLong (dialogID, layerInfo.iDong_12xx [xx], FALSE);
						DGSetItemValLong (dialogID, layerInfo.iDong_13xx [xx], FALSE);
						DGSetItemValLong (dialogID, layerInfo.iDong_14xx [xx], FALSE);
						DGSetItemValLong (dialogID, layerInfo.iDong_15xx [xx], FALSE);
					}
					DGSetItemValLong (dialogID, layerInfo.iDong_SHOP, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iDong_SECU, FALSE);
				}
			}
			if (changedBtnItemIdx == SELECTALL_3_FLOOR) {
				if (DGGetItemValLong (dialogID, SELECTALL_3_FLOOR) == TRUE) {
					// ��� ����
					for (xx = 9 ; xx >= 1 ; --xx)	DGSetItemValLong (dialogID, layerInfo.iFloor_Bxx [xx], TRUE);
					for (xx = 1 ; xx < 100 ; ++xx)	DGSetItemValLong (dialogID, layerInfo.iFloor_Fxx [xx], TRUE);
					for (xx = 1 ; xx < 10 ; ++xx)	DGSetItemValLong (dialogID, layerInfo.iFloor_PHxx [xx], TRUE);
				} else {
					// ��� ����
					for (xx = 9 ; xx >= 1 ; --xx)	DGSetItemValLong (dialogID, layerInfo.iFloor_Bxx [xx], FALSE);
					for (xx = 1 ; xx < 100 ; ++xx)	DGSetItemValLong (dialogID, layerInfo.iFloor_Fxx [xx], FALSE);
					for (xx = 1 ; xx < 10 ; ++xx)	DGSetItemValLong (dialogID, layerInfo.iFloor_PHxx [xx], FALSE);
				}
			}
			if (changedBtnItemIdx == SELECTALL_4_CJ) {
				if (DGGetItemValLong (dialogID, SELECTALL_4_CJ) == TRUE) {
					// ��� ����
					for (xx = 1 ; xx < 100 ; ++xx)	DGSetItemValLong (dialogID, layerInfo.iCJ [xx], TRUE);
				} else {
					// ��� ����
					for (xx = 1 ; xx < 100 ; ++xx)	DGSetItemValLong (dialogID, layerInfo.iCJ [xx], FALSE);
				}
			}
			if (changedBtnItemIdx == SELECTALL_5_ORDER) {
				if (DGGetItemValLong (dialogID, SELECTALL_5_ORDER) == TRUE) {
					// ��� ����
					for (xx = 1 ; xx < 100 ; ++xx)	DGSetItemValLong (dialogID, layerInfo.iOrderInCJ [xx], TRUE);
				} else {
					// ��� ����
					for (xx = 1 ; xx < 100 ; ++xx)	DGSetItemValLong (dialogID, layerInfo.iOrderInCJ [xx], FALSE);
				}
			}
			if (changedBtnItemIdx == SELECTALL_6_OBJ_01_S) {
				if (DGGetItemValLong (dialogID, SELECTALL_6_OBJ_01_S) == TRUE) {
					// ��� ����
					DGSetItemValLong (dialogID, layerInfo.iType_01_S_STAR, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_01_S_COLU, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_01_S_FOOT, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_01_S_WALL, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_01_S_BEAM, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_01_S_SLAB, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_01_S_CLST, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_01_S_BMST, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_01_S_RAMP, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_01_S_CWAL, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_01_S_WTWL, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_01_S_CSTN, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_01_S_MPAD, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_01_S_GADN, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_01_S_PARA, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_01_S_CLPC, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_01_S_BMPC, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_01_S_BMWL, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_01_S_STST, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_01_S_TOPP, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_01_S_DECK, TRUE);
				} else {
					// ��� ����
					DGSetItemValLong (dialogID, layerInfo.iType_01_S_STAR, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_01_S_COLU, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_01_S_FOOT, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_01_S_WALL, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_01_S_BEAM, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_01_S_SLAB, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_01_S_CLST, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_01_S_BMST, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_01_S_RAMP, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_01_S_CWAL, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_01_S_WTWL, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_01_S_CSTN, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_01_S_MPAD, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_01_S_GADN, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_01_S_PARA, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_01_S_CLPC, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_01_S_BMPC, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_01_S_BMWL, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_01_S_STST, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_01_S_TOPP, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_01_S_DECK, FALSE);
				}
			}
			if (changedBtnItemIdx == SELECTALL_6_OBJ_02_A) {
				if (DGGetItemValLong (dialogID, SELECTALL_6_OBJ_02_A) == TRUE) {
					// ��� ����
					DGSetItemValLong (dialogID, layerInfo.iType_02_A_FURN, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_02_A_INSU, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_02_A_PAIN, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_02_A_MOLD, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_02_A_MORT, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_02_A_WATE, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_02_A_BRIC, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_02_A_PAPE, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_02_A_BLOC, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_02_A_GYPS, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_02_A_STON, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_02_A_INTE, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_02_A_GLAS, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_02_A_HARD, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_02_A_TILE, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_02_A_PANE, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_02_A_PLYW, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_02_A_PCON, TRUE);
				} else {
					// ��� ����
					DGSetItemValLong (dialogID, layerInfo.iType_02_A_FURN, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_02_A_INSU, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_02_A_PAIN, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_02_A_MOLD, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_02_A_MORT, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_02_A_WATE, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_02_A_BRIC, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_02_A_PAPE, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_02_A_BLOC, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_02_A_GYPS, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_02_A_STON, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_02_A_INTE, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_02_A_GLAS, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_02_A_HARD, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_02_A_TILE, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_02_A_PANE, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_02_A_PLYW, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_02_A_PCON, FALSE);
				}
			}
			if (changedBtnItemIdx == SELECTALL_6_OBJ_05_T) {
				if (DGGetItemValLong (dialogID, SELECTALL_6_OBJ_05_T) == TRUE) {
					// ��� ����
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_TIMB, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_BIMJ, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_BDCM, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_DMGA, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_RIBL, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_BMSP, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_BSTA, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_SPIP, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_SUPT, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_SYSU, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_OUTA, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_OUTP, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_AFOM, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_CPIP, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_UGON, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_UFOM, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_INCO, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_JOIB, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_EFOM, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_JSUPT, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_WTST, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_CLAM, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_LUMB, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_TRUS, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_TBBM, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_BCWF, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_PLYW, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_FISP, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_STSE, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_SLSE, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_RAIL, TRUE);
				} else {
					// ��� ����
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_TIMB, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_BIMJ, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_BDCM, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_DMGA, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_RIBL, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_BMSP, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_BSTA, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_SPIP, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_SUPT, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_SYSU, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_OUTA, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_OUTP, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_AFOM, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_CPIP, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_UGON, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_UFOM, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_INCO, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_JOIB, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_EFOM, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_JSUPT, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_WTST, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_CLAM, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_LUMB, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_TRUS, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_TBBM, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_BCWF, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_PLYW, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_FISP, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_STSE, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_SLSE, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_RAIL, FALSE);
				}
			}
			if (changedBtnItemIdx == SELECTALL_6_OBJ_06_F) {
				if (DGGetItemValLong (dialogID, SELECTALL_6_OBJ_06_F) == TRUE) {
					// ��� ����
					DGSetItemValLong (dialogID, layerInfo.iType_06_F_STRU, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_06_F_HFIL, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_06_F_SJAK, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_06_F_PJAK, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_06_F_BRKT, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_06_F_PBKT, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_06_F_CIP, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_06_F_LAND, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_06_F_ANGL, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_06_F_ERAC, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_06_F_LUMB, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_06_F_BPAN, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_06_F_WALE, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_06_F_PILE, TRUE);
				} else {
					// ��� ����
					DGSetItemValLong (dialogID, layerInfo.iType_06_F_STRU, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_06_F_HFIL, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_06_F_SJAK, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_06_F_PJAK, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_06_F_BRKT, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_06_F_PBKT, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_06_F_CIP, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_06_F_LAND, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_06_F_ANGL, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_06_F_ERAC, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_06_F_LUMB, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_06_F_BPAN, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_06_F_WALE, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_06_F_PILE, FALSE);
				}
			}
			if (changedBtnItemIdx == SELECTALL_6_OBJ_05_T_2) {
				if (DGGetItemValLong (dialogID, SELECTALL_6_OBJ_05_T_2) == TRUE) {
					// ��� ����
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_2_TIMB, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_2_BIMJ, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_2_BDCM, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_2_DMGA, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_2_RIBL, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_2_BMSP, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_2_BSTA, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_2_SPIP, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_2_SUPT, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_2_SYSU, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_2_OUTA, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_2_OUTP, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_2_AFOM, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_2_CPIP, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_2_UGON, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_2_UFOM, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_2_INCO, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_2_JOIB, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_2_EFOM, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_2_JSUPT, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_2_WTST, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_2_CLAM, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_2_LUMB, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_2_TRUS, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_2_TBBM, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_2_BCWF, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_2_PLYW, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_2_FISP, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_2_STSE, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_2_SLSE, TRUE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_2_RAIL, TRUE);
				} else {
					// ��� ����
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_2_TIMB, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_2_BIMJ, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_2_BDCM, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_2_DMGA, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_2_RIBL, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_2_BMSP, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_2_BSTA, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_2_SPIP, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_2_SUPT, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_2_SYSU, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_2_OUTA, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_2_OUTP, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_2_AFOM, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_2_CPIP, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_2_UGON, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_2_UFOM, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_2_INCO, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_2_JOIB, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_2_EFOM, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_2_JSUPT, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_2_WTST, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_2_CLAM, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_2_LUMB, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_2_TRUS, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_2_TBBM, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_2_BCWF, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_2_PLYW, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_2_FISP, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_2_STSE, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_2_SLSE, FALSE);
					DGSetItemValLong (dialogID, layerInfo.iType_05_T_2_RAIL, FALSE);
				}
			}

			// ���縦 �����ϸ� ���� �ڵ带 �ڵ� ����
			if ((changedBtnItemIdx == layerInfo.iType_01_S_STAR) && (DGGetItemValLong (dialogID, layerInfo.iType_01_S_STAR) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_01_S, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_01_S_COLU) && (DGGetItemValLong (dialogID, layerInfo.iType_01_S_COLU) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_01_S, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_01_S_FOOT) && (DGGetItemValLong (dialogID, layerInfo.iType_01_S_FOOT) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_01_S, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_01_S_WALL) && (DGGetItemValLong (dialogID, layerInfo.iType_01_S_WALL) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_01_S, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_01_S_BEAM) && (DGGetItemValLong (dialogID, layerInfo.iType_01_S_BEAM) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_01_S, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_01_S_SLAB) && (DGGetItemValLong (dialogID, layerInfo.iType_01_S_SLAB) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_01_S, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_01_S_CLST) && (DGGetItemValLong (dialogID, layerInfo.iType_01_S_CLST) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_01_S, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_01_S_BMST) && (DGGetItemValLong (dialogID, layerInfo.iType_01_S_BMST) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_01_S, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_01_S_RAMP) && (DGGetItemValLong (dialogID, layerInfo.iType_01_S_RAMP) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_01_S, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_01_S_CWAL) && (DGGetItemValLong (dialogID, layerInfo.iType_01_S_CWAL) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_01_S, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_01_S_WTWL) && (DGGetItemValLong (dialogID, layerInfo.iType_01_S_WTWL) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_01_S, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_01_S_CSTN) && (DGGetItemValLong (dialogID, layerInfo.iType_01_S_CSTN) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_01_S, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_01_S_MPAD) && (DGGetItemValLong (dialogID, layerInfo.iType_01_S_MPAD) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_01_S, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_01_S_GADN) && (DGGetItemValLong (dialogID, layerInfo.iType_01_S_GADN) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_01_S, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_01_S_PARA) && (DGGetItemValLong (dialogID, layerInfo.iType_01_S_PARA) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_01_S, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_01_S_CLPC) && (DGGetItemValLong (dialogID, layerInfo.iType_01_S_CLPC) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_01_S, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_01_S_BMPC) && (DGGetItemValLong (dialogID, layerInfo.iType_01_S_BMPC) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_01_S, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_01_S_BMWL) && (DGGetItemValLong (dialogID, layerInfo.iType_01_S_BMWL) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_01_S, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_01_S_STST) && (DGGetItemValLong (dialogID, layerInfo.iType_01_S_STST) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_01_S, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_01_S_TOPP) && (DGGetItemValLong (dialogID, layerInfo.iType_01_S_TOPP) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_01_S, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_01_S_DECK) && (DGGetItemValLong (dialogID, layerInfo.iType_01_S_DECK) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_01_S, TRUE);
			if ((changedBtnItemIdx == SELECTALL_6_OBJ_01_S) && (DGGetItemValLong (dialogID, SELECTALL_6_OBJ_01_S) == TRUE))				DGSetItemValLong (dialogID, layerInfo.iType_01_S, TRUE);

			if ((changedBtnItemIdx == layerInfo.iType_02_A_FURN) && (DGGetItemValLong (dialogID, layerInfo.iType_02_A_FURN) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_02_A, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_02_A_INSU) && (DGGetItemValLong (dialogID, layerInfo.iType_02_A_INSU) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_02_A, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_02_A_PAIN) && (DGGetItemValLong (dialogID, layerInfo.iType_02_A_PAIN) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_02_A, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_02_A_MOLD) && (DGGetItemValLong (dialogID, layerInfo.iType_02_A_MOLD) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_02_A, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_02_A_MORT) && (DGGetItemValLong (dialogID, layerInfo.iType_02_A_MORT) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_02_A, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_02_A_WATE) && (DGGetItemValLong (dialogID, layerInfo.iType_02_A_WATE) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_02_A, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_02_A_BRIC) && (DGGetItemValLong (dialogID, layerInfo.iType_02_A_BRIC) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_02_A, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_02_A_PAPE) && (DGGetItemValLong (dialogID, layerInfo.iType_02_A_PAPE) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_02_A, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_02_A_BLOC) && (DGGetItemValLong (dialogID, layerInfo.iType_02_A_BLOC) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_02_A, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_02_A_GYPS) && (DGGetItemValLong (dialogID, layerInfo.iType_02_A_GYPS) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_02_A, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_02_A_STON) && (DGGetItemValLong (dialogID, layerInfo.iType_02_A_STON) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_02_A, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_02_A_INTE) && (DGGetItemValLong (dialogID, layerInfo.iType_02_A_INTE) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_02_A, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_02_A_GLAS) && (DGGetItemValLong (dialogID, layerInfo.iType_02_A_GLAS) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_02_A, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_02_A_HARD) && (DGGetItemValLong (dialogID, layerInfo.iType_02_A_HARD) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_02_A, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_02_A_TILE) && (DGGetItemValLong (dialogID, layerInfo.iType_02_A_TILE) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_02_A, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_02_A_PANE) && (DGGetItemValLong (dialogID, layerInfo.iType_02_A_PANE) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_02_A, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_02_A_PLYW) && (DGGetItemValLong (dialogID, layerInfo.iType_02_A_PLYW) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_02_A, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_02_A_PCON) && (DGGetItemValLong (dialogID, layerInfo.iType_02_A_PCON) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_02_A, TRUE);
			if ((changedBtnItemIdx == SELECTALL_6_OBJ_02_A) && (DGGetItemValLong (dialogID, SELECTALL_6_OBJ_02_A) == TRUE))				DGSetItemValLong (dialogID, layerInfo.iType_02_A, TRUE);

			if ((changedBtnItemIdx == layerInfo.iType_05_T_TIMB) && (DGGetItemValLong (dialogID, layerInfo.iType_05_T_TIMB) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_05_T, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_05_T_BIMJ) && (DGGetItemValLong (dialogID, layerInfo.iType_05_T_BIMJ) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_05_T, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_05_T_BDCM) && (DGGetItemValLong (dialogID, layerInfo.iType_05_T_BDCM) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_05_T, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_05_T_DMGA) && (DGGetItemValLong (dialogID, layerInfo.iType_05_T_DMGA) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_05_T, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_05_T_RIBL) && (DGGetItemValLong (dialogID, layerInfo.iType_05_T_RIBL) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_05_T, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_05_T_BMSP) && (DGGetItemValLong (dialogID, layerInfo.iType_05_T_BMSP) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_05_T, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_05_T_BSTA) && (DGGetItemValLong (dialogID, layerInfo.iType_05_T_BSTA) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_05_T, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_05_T_SPIP) && (DGGetItemValLong (dialogID, layerInfo.iType_05_T_SPIP) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_05_T, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_05_T_SUPT) && (DGGetItemValLong (dialogID, layerInfo.iType_05_T_SUPT) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_05_T, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_05_T_SYSU) && (DGGetItemValLong (dialogID, layerInfo.iType_05_T_SYSU) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_05_T, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_05_T_OUTA) && (DGGetItemValLong (dialogID, layerInfo.iType_05_T_OUTA) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_05_T, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_05_T_OUTP) && (DGGetItemValLong (dialogID, layerInfo.iType_05_T_OUTP) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_05_T, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_05_T_AFOM) && (DGGetItemValLong (dialogID, layerInfo.iType_05_T_AFOM) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_05_T, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_05_T_CPIP) && (DGGetItemValLong (dialogID, layerInfo.iType_05_T_CPIP) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_05_T, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_05_T_UGON) && (DGGetItemValLong (dialogID, layerInfo.iType_05_T_UGON) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_05_T, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_05_T_UFOM) && (DGGetItemValLong (dialogID, layerInfo.iType_05_T_UFOM) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_05_T, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_05_T_INCO) && (DGGetItemValLong (dialogID, layerInfo.iType_05_T_INCO) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_05_T, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_05_T_JOIB) && (DGGetItemValLong (dialogID, layerInfo.iType_05_T_JOIB) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_05_T, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_05_T_EFOM) && (DGGetItemValLong (dialogID, layerInfo.iType_05_T_EFOM) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_05_T, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_05_T_JSUPT) && (DGGetItemValLong (dialogID, layerInfo.iType_05_T_JSUPT) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_05_T, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_05_T_WTST) && (DGGetItemValLong (dialogID, layerInfo.iType_05_T_WTST) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_05_T, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_05_T_CLAM) && (DGGetItemValLong (dialogID, layerInfo.iType_05_T_CLAM) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_05_T, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_05_T_LUMB) && (DGGetItemValLong (dialogID, layerInfo.iType_05_T_LUMB) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_05_T, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_05_T_TRUS) && (DGGetItemValLong (dialogID, layerInfo.iType_05_T_TRUS) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_05_T, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_05_T_TBBM) && (DGGetItemValLong (dialogID, layerInfo.iType_05_T_TBBM) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_05_T, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_05_T_BCWF) && (DGGetItemValLong (dialogID, layerInfo.iType_05_T_BCWF) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_05_T, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_05_T_PLYW) && (DGGetItemValLong (dialogID, layerInfo.iType_05_T_PLYW) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_05_T, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_05_T_FISP) && (DGGetItemValLong (dialogID, layerInfo.iType_05_T_FISP) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_05_T, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_05_T_STSE) && (DGGetItemValLong (dialogID, layerInfo.iType_05_T_STSE) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_05_T, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_05_T_SLSE) && (DGGetItemValLong (dialogID, layerInfo.iType_05_T_SLSE) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_05_T, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_05_T_RAIL) && (DGGetItemValLong (dialogID, layerInfo.iType_05_T_RAIL) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_05_T, TRUE);
			if ((changedBtnItemIdx == SELECTALL_6_OBJ_05_T) && (DGGetItemValLong (dialogID, SELECTALL_6_OBJ_05_T) == TRUE))				DGSetItemValLong (dialogID, layerInfo.iType_05_T, TRUE);

			if ((changedBtnItemIdx == layerInfo.iType_06_F_STRU) && (DGGetItemValLong (dialogID, layerInfo.iType_06_F_STRU) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_06_F, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_06_F_HFIL) && (DGGetItemValLong (dialogID, layerInfo.iType_06_F_HFIL) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_06_F, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_06_F_SJAK) && (DGGetItemValLong (dialogID, layerInfo.iType_06_F_SJAK) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_06_F, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_06_F_PJAK) && (DGGetItemValLong (dialogID, layerInfo.iType_06_F_PJAK) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_06_F, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_06_F_BRKT) && (DGGetItemValLong (dialogID, layerInfo.iType_06_F_BRKT) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_06_F, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_06_F_PBKT) && (DGGetItemValLong (dialogID, layerInfo.iType_06_F_PBKT) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_06_F, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_06_F_CIP)  && (DGGetItemValLong (dialogID, layerInfo.iType_06_F_CIP) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_06_F, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_06_F_LAND) && (DGGetItemValLong (dialogID, layerInfo.iType_06_F_LAND) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_06_F, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_06_F_ANGL) && (DGGetItemValLong (dialogID, layerInfo.iType_06_F_ANGL) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_06_F, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_06_F_ERAC) && (DGGetItemValLong (dialogID, layerInfo.iType_06_F_ERAC) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_06_F, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_06_F_LUMB) && (DGGetItemValLong (dialogID, layerInfo.iType_06_F_LUMB) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_06_F, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_06_F_BPAN) && (DGGetItemValLong (dialogID, layerInfo.iType_06_F_BPAN) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_06_F, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_06_F_WALE) && (DGGetItemValLong (dialogID, layerInfo.iType_06_F_WALE) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_06_F, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_06_F_PILE) && (DGGetItemValLong (dialogID, layerInfo.iType_06_F_PILE) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_06_F, TRUE);
			if ((changedBtnItemIdx == SELECTALL_6_OBJ_06_F) && (DGGetItemValLong (dialogID, SELECTALL_6_OBJ_06_F) == TRUE))				DGSetItemValLong (dialogID, layerInfo.iType_06_F, TRUE);

			if ((changedBtnItemIdx == layerInfo.iType_05_T_2_TIMB) && (DGGetItemValLong (dialogID, layerInfo.iType_05_T_2_TIMB) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_05_T, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_05_T_2_BIMJ) && (DGGetItemValLong (dialogID, layerInfo.iType_05_T_2_BIMJ) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_05_T, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_05_T_2_BDCM) && (DGGetItemValLong (dialogID, layerInfo.iType_05_T_2_BDCM) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_05_T, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_05_T_2_DMGA) && (DGGetItemValLong (dialogID, layerInfo.iType_05_T_2_DMGA) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_05_T, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_05_T_2_RIBL) && (DGGetItemValLong (dialogID, layerInfo.iType_05_T_2_RIBL) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_05_T, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_05_T_2_BMSP) && (DGGetItemValLong (dialogID, layerInfo.iType_05_T_2_BMSP) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_05_T, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_05_T_2_BSTA) && (DGGetItemValLong (dialogID, layerInfo.iType_05_T_2_BSTA) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_05_T, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_05_T_2_SPIP) && (DGGetItemValLong (dialogID, layerInfo.iType_05_T_2_SPIP) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_05_T, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_05_T_2_SUPT) && (DGGetItemValLong (dialogID, layerInfo.iType_05_T_2_SUPT) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_05_T, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_05_T_2_SYSU) && (DGGetItemValLong (dialogID, layerInfo.iType_05_T_2_SYSU) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_05_T, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_05_T_2_OUTA) && (DGGetItemValLong (dialogID, layerInfo.iType_05_T_2_OUTA) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_05_T, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_05_T_2_OUTP) && (DGGetItemValLong (dialogID, layerInfo.iType_05_T_2_OUTP) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_05_T, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_05_T_2_AFOM) && (DGGetItemValLong (dialogID, layerInfo.iType_05_T_2_AFOM) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_05_T, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_05_T_2_CPIP) && (DGGetItemValLong (dialogID, layerInfo.iType_05_T_2_CPIP) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_05_T, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_05_T_2_UGON) && (DGGetItemValLong (dialogID, layerInfo.iType_05_T_2_UGON) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_05_T, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_05_T_2_UFOM) && (DGGetItemValLong (dialogID, layerInfo.iType_05_T_2_UFOM) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_05_T, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_05_T_2_INCO) && (DGGetItemValLong (dialogID, layerInfo.iType_05_T_2_INCO) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_05_T, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_05_T_2_JOIB) && (DGGetItemValLong (dialogID, layerInfo.iType_05_T_2_JOIB) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_05_T, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_05_T_2_EFOM) && (DGGetItemValLong (dialogID, layerInfo.iType_05_T_2_EFOM) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_05_T, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_05_T_2_JSUPT) && (DGGetItemValLong (dialogID, layerInfo.iType_05_T_2_JSUPT) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_05_T, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_05_T_2_WTST) && (DGGetItemValLong (dialogID, layerInfo.iType_05_T_2_WTST) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_05_T, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_05_T_2_CLAM) && (DGGetItemValLong (dialogID, layerInfo.iType_05_T_2_CLAM) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_05_T, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_05_T_2_LUMB) && (DGGetItemValLong (dialogID, layerInfo.iType_05_T_2_LUMB) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_05_T, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_05_T_2_TRUS) && (DGGetItemValLong (dialogID, layerInfo.iType_05_T_2_TRUS) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_05_T, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_05_T_2_TBBM) && (DGGetItemValLong (dialogID, layerInfo.iType_05_T_2_TBBM) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_05_T, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_05_T_2_BCWF) && (DGGetItemValLong (dialogID, layerInfo.iType_05_T_2_BCWF) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_05_T, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_05_T_2_PLYW) && (DGGetItemValLong (dialogID, layerInfo.iType_05_T_2_PLYW) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_05_T, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_05_T_2_FISP) && (DGGetItemValLong (dialogID, layerInfo.iType_05_T_2_FISP) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_05_T, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_05_T_2_STSE) && (DGGetItemValLong (dialogID, layerInfo.iType_05_T_2_STSE) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_05_T, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_05_T_2_SLSE) && (DGGetItemValLong (dialogID, layerInfo.iType_05_T_2_SLSE) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_05_T, TRUE);
			if ((changedBtnItemIdx == layerInfo.iType_05_T_2_RAIL) && (DGGetItemValLong (dialogID, layerInfo.iType_05_T_2_RAIL) == TRUE))	DGSetItemValLong (dialogID, layerInfo.iType_05_T, TRUE);
			if ((changedBtnItemIdx == SELECTALL_6_OBJ_05_T_2) && (DGGetItemValLong (dialogID, SELECTALL_6_OBJ_05_T_2) == TRUE))				DGSetItemValLong (dialogID, layerInfo.iType_05_T, TRUE);

			break;
		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					// ���� ����
					(DGGetItemValLong (dialogID, layerInfo.iType_01_S) == TRUE) ?	selectedInfo.bType_01_S = true : selectedInfo.bType_01_S = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_02_A) == TRUE) ?	selectedInfo.bType_02_A = true : selectedInfo.bType_02_A = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_03_M) == TRUE) ?	selectedInfo.bType_03_M = true : selectedInfo.bType_03_M = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_04_E) == TRUE) ?	selectedInfo.bType_04_E = true : selectedInfo.bType_04_E = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T) == TRUE) ?	selectedInfo.bType_05_T = true : selectedInfo.bType_05_T = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_06_F) == TRUE) ?	selectedInfo.bType_06_F = true : selectedInfo.bType_06_F = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_07_Q) == TRUE) ?	selectedInfo.bType_07_Q = true : selectedInfo.bType_07_Q = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_08_L) == TRUE) ?	selectedInfo.bType_08_L = true : selectedInfo.bType_08_L = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_09_C) == TRUE) ?	selectedInfo.bType_09_C = true : selectedInfo.bType_09_C = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_10_K) == TRUE) ?	selectedInfo.bType_10_K = true : selectedInfo.bType_10_K = false;
					
					// �� ����
					for (xx = 1 ; xx < 100 ; ++xx) {
						(DGGetItemValLong (dialogID, layerInfo.iDong_1xx [xx]) == TRUE) ?	selectedInfo.bDong_1xx [xx] = true : selectedInfo.bDong_1xx [xx] = false;
						(DGGetItemValLong (dialogID, layerInfo.iDong_2xx [xx]) == TRUE) ?	selectedInfo.bDong_2xx [xx] = true : selectedInfo.bDong_2xx [xx] = false;
						(DGGetItemValLong (dialogID, layerInfo.iDong_3xx [xx]) == TRUE) ?	selectedInfo.bDong_3xx [xx] = true : selectedInfo.bDong_3xx [xx] = false;
						(DGGetItemValLong (dialogID, layerInfo.iDong_4xx [xx]) == TRUE) ?	selectedInfo.bDong_4xx [xx] = true : selectedInfo.bDong_4xx [xx] = false;
						(DGGetItemValLong (dialogID, layerInfo.iDong_5xx [xx]) == TRUE) ?	selectedInfo.bDong_5xx [xx] = true : selectedInfo.bDong_5xx [xx] = false;
						(DGGetItemValLong (dialogID, layerInfo.iDong_6xx [xx]) == TRUE) ?	selectedInfo.bDong_6xx [xx] = true : selectedInfo.bDong_6xx [xx] = false;
						(DGGetItemValLong (dialogID, layerInfo.iDong_7xx [xx]) == TRUE) ?	selectedInfo.bDong_7xx [xx] = true : selectedInfo.bDong_7xx [xx] = false;
						(DGGetItemValLong (dialogID, layerInfo.iDong_8xx [xx]) == TRUE) ?	selectedInfo.bDong_8xx [xx] = true : selectedInfo.bDong_8xx [xx] = false;
						(DGGetItemValLong (dialogID, layerInfo.iDong_9xx [xx]) == TRUE) ?	selectedInfo.bDong_9xx [xx] = true : selectedInfo.bDong_9xx [xx] = false;
						(DGGetItemValLong (dialogID, layerInfo.iDong_10xx [xx]) == TRUE) ?	selectedInfo.bDong_10xx [xx] = true : selectedInfo.bDong_10xx [xx] = false;
						(DGGetItemValLong (dialogID, layerInfo.iDong_11xx [xx]) == TRUE) ?	selectedInfo.bDong_11xx [xx] = true : selectedInfo.bDong_11xx [xx] = false;
						(DGGetItemValLong (dialogID, layerInfo.iDong_12xx [xx]) == TRUE) ?	selectedInfo.bDong_12xx [xx] = true : selectedInfo.bDong_12xx [xx] = false;
						(DGGetItemValLong (dialogID, layerInfo.iDong_13xx [xx]) == TRUE) ?	selectedInfo.bDong_13xx [xx] = true : selectedInfo.bDong_13xx [xx] = false;
						(DGGetItemValLong (dialogID, layerInfo.iDong_14xx [xx]) == TRUE) ?	selectedInfo.bDong_14xx [xx] = true : selectedInfo.bDong_14xx [xx] = false;
						(DGGetItemValLong (dialogID, layerInfo.iDong_15xx [xx]) == TRUE) ?	selectedInfo.bDong_15xx [xx] = true : selectedInfo.bDong_15xx [xx] = false;
					}

					(DGGetItemValLong (dialogID, layerInfo.iDong_SHOP) == TRUE) ?	selectedInfo.bDong_SHOP = true : selectedInfo.bDong_SHOP = false;
					(DGGetItemValLong (dialogID, layerInfo.iDong_SECU) == TRUE) ?	selectedInfo.bDong_SECU = true : selectedInfo.bDong_SECU = false;

					// �� ����
					for (xx = 9 ; xx >= 1 ; --xx) {
						(DGGetItemValLong (dialogID, layerInfo.iFloor_Bxx [xx]) == TRUE) ?	selectedInfo.bFloor_Bxx [xx] = true : selectedInfo.bFloor_Bxx [xx] = false;
					}
					for (xx = 1 ; xx < 100 ; ++xx) {
						(DGGetItemValLong (dialogID, layerInfo.iFloor_Fxx [xx]) == TRUE) ?	selectedInfo.bFloor_Fxx [xx] = true : selectedInfo.bFloor_Fxx [xx] = false;
					}
					for (xx = 1 ; xx < 10 ; ++xx) {
						(DGGetItemValLong (dialogID, layerInfo.iFloor_PHxx [xx]) == TRUE) ?	selectedInfo.bFloor_PHxx [xx] = true : selectedInfo.bFloor_PHxx [xx] = false;
					}

					// CJ, CJ �� �ð�����
					for (xx = 1 ; xx < 100 ; ++xx) {
						(DGGetItemValLong (dialogID, layerInfo.iCJ [xx]) == TRUE) ?			selectedInfo.bCJ [xx] = true : selectedInfo.bCJ [xx] = false;
						(DGGetItemValLong (dialogID, layerInfo.iOrderInCJ [xx]) == TRUE) ?	selectedInfo.bOrderInCJ [xx] = true : selectedInfo.bOrderInCJ [xx] = false;
					}

					// 01-S ����
					(DGGetItemValLong (dialogID, layerInfo.iType_01_S_STAR) == TRUE) ?	selectedInfo.bType_01_S_STAR = true : selectedInfo.bType_01_S_STAR = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_01_S_COLU) == TRUE) ?	selectedInfo.bType_01_S_COLU = true : selectedInfo.bType_01_S_COLU = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_01_S_FOOT) == TRUE) ?	selectedInfo.bType_01_S_FOOT = true : selectedInfo.bType_01_S_FOOT = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_01_S_WALL) == TRUE) ?	selectedInfo.bType_01_S_WALL = true : selectedInfo.bType_01_S_WALL = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_01_S_BEAM) == TRUE) ?	selectedInfo.bType_01_S_BEAM = true : selectedInfo.bType_01_S_BEAM = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_01_S_SLAB) == TRUE) ?	selectedInfo.bType_01_S_SLAB = true : selectedInfo.bType_01_S_SLAB = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_01_S_CLST) == TRUE) ?	selectedInfo.bType_01_S_CLST = true : selectedInfo.bType_01_S_CLST = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_01_S_BMST) == TRUE) ?	selectedInfo.bType_01_S_BMST = true : selectedInfo.bType_01_S_BMST = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_01_S_RAMP) == TRUE) ?	selectedInfo.bType_01_S_RAMP = true : selectedInfo.bType_01_S_RAMP = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_01_S_CWAL) == TRUE) ?	selectedInfo.bType_01_S_CWAL = true : selectedInfo.bType_01_S_CWAL = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_01_S_WTWL) == TRUE) ?	selectedInfo.bType_01_S_WTWL = true : selectedInfo.bType_01_S_WTWL = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_01_S_CSTN) == TRUE) ?	selectedInfo.bType_01_S_CSTN = true : selectedInfo.bType_01_S_CSTN = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_01_S_MPAD) == TRUE) ?	selectedInfo.bType_01_S_MPAD = true : selectedInfo.bType_01_S_MPAD = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_01_S_GADN) == TRUE) ?	selectedInfo.bType_01_S_GADN = true : selectedInfo.bType_01_S_GADN = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_01_S_PARA) == TRUE) ?	selectedInfo.bType_01_S_PARA = true : selectedInfo.bType_01_S_PARA = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_01_S_CLPC) == TRUE) ?	selectedInfo.bType_01_S_CLPC = true : selectedInfo.bType_01_S_CLPC = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_01_S_BMPC) == TRUE) ?	selectedInfo.bType_01_S_BMPC = true : selectedInfo.bType_01_S_BMPC = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_01_S_BMWL) == TRUE) ?	selectedInfo.bType_01_S_BMWL = true : selectedInfo.bType_01_S_BMWL = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_01_S_STST) == TRUE) ?	selectedInfo.bType_01_S_STST = true : selectedInfo.bType_01_S_STST = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_01_S_TOPP) == TRUE) ?	selectedInfo.bType_01_S_TOPP = true : selectedInfo.bType_01_S_TOPP = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_01_S_DECK) == TRUE) ?	selectedInfo.bType_01_S_DECK = true : selectedInfo.bType_01_S_DECK = false;

					// 02-A ���ึ��
					(DGGetItemValLong (dialogID, layerInfo.iType_02_A_FURN) == TRUE) ?	selectedInfo.bType_02_A_FURN = true : selectedInfo.bType_02_A_FURN = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_02_A_INSU) == TRUE) ?	selectedInfo.bType_02_A_INSU = true : selectedInfo.bType_02_A_INSU = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_02_A_PAIN) == TRUE) ?	selectedInfo.bType_02_A_PAIN = true : selectedInfo.bType_02_A_PAIN = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_02_A_MOLD) == TRUE) ?	selectedInfo.bType_02_A_MOLD = true : selectedInfo.bType_02_A_MOLD = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_02_A_MORT) == TRUE) ?	selectedInfo.bType_02_A_MORT = true : selectedInfo.bType_02_A_MORT = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_02_A_WATE) == TRUE) ?	selectedInfo.bType_02_A_WATE = true : selectedInfo.bType_02_A_WATE = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_02_A_BRIC) == TRUE) ?	selectedInfo.bType_02_A_BRIC = true : selectedInfo.bType_02_A_BRIC = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_02_A_PAPE) == TRUE) ?	selectedInfo.bType_02_A_PAPE = true : selectedInfo.bType_02_A_PAPE = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_02_A_BLOC) == TRUE) ?	selectedInfo.bType_02_A_BLOC = true : selectedInfo.bType_02_A_BLOC = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_02_A_GYPS) == TRUE) ?	selectedInfo.bType_02_A_GYPS = true : selectedInfo.bType_02_A_GYPS = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_02_A_STON) == TRUE) ?	selectedInfo.bType_02_A_STON = true : selectedInfo.bType_02_A_STON = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_02_A_INTE) == TRUE) ?	selectedInfo.bType_02_A_INTE = true : selectedInfo.bType_02_A_INTE = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_02_A_GLAS) == TRUE) ?	selectedInfo.bType_02_A_GLAS = true : selectedInfo.bType_02_A_GLAS = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_02_A_HARD) == TRUE) ?	selectedInfo.bType_02_A_HARD = true : selectedInfo.bType_02_A_HARD = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_02_A_TILE) == TRUE) ?	selectedInfo.bType_02_A_TILE = true : selectedInfo.bType_02_A_TILE = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_02_A_PANE) == TRUE) ?	selectedInfo.bType_02_A_PANE = true : selectedInfo.bType_02_A_PANE = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_02_A_PLYW) == TRUE) ?	selectedInfo.bType_02_A_PLYW = true : selectedInfo.bType_02_A_PLYW = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_02_A_PCON) == TRUE) ?	selectedInfo.bType_02_A_PCON = true : selectedInfo.bType_02_A_PCON = false;
					
					// 05-T ������
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_TIMB) == TRUE) ?	selectedInfo.bType_05_T_TIMB = true : selectedInfo.bType_05_T_TIMB = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_BIMJ) == TRUE) ?	selectedInfo.bType_05_T_BIMJ = true : selectedInfo.bType_05_T_BIMJ = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_BDCM) == TRUE) ?	selectedInfo.bType_05_T_BDCM = true : selectedInfo.bType_05_T_BDCM = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_DMGA) == TRUE) ?	selectedInfo.bType_05_T_DMGA = true : selectedInfo.bType_05_T_DMGA = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_RIBL) == TRUE) ?	selectedInfo.bType_05_T_RIBL = true : selectedInfo.bType_05_T_RIBL = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_BMSP) == TRUE) ?	selectedInfo.bType_05_T_BMSP = true : selectedInfo.bType_05_T_BMSP = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_BSTA) == TRUE) ?	selectedInfo.bType_05_T_BSTA = true : selectedInfo.bType_05_T_BSTA = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_SPIP) == TRUE) ?	selectedInfo.bType_05_T_SPIP = true : selectedInfo.bType_05_T_SPIP = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_SUPT) == TRUE) ?	selectedInfo.bType_05_T_SUPT = true : selectedInfo.bType_05_T_SUPT = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_SYSU) == TRUE) ?	selectedInfo.bType_05_T_SYSU = true : selectedInfo.bType_05_T_SYSU = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_OUTA) == TRUE) ?	selectedInfo.bType_05_T_OUTA = true : selectedInfo.bType_05_T_OUTA = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_OUTP) == TRUE) ?	selectedInfo.bType_05_T_OUTP = true : selectedInfo.bType_05_T_OUTP = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_AFOM) == TRUE) ?	selectedInfo.bType_05_T_AFOM = true : selectedInfo.bType_05_T_AFOM = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_CPIP) == TRUE) ?	selectedInfo.bType_05_T_CPIP = true : selectedInfo.bType_05_T_CPIP = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_UGON) == TRUE) ?	selectedInfo.bType_05_T_UGON = true : selectedInfo.bType_05_T_UGON = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_UFOM) == TRUE) ?	selectedInfo.bType_05_T_UFOM = true : selectedInfo.bType_05_T_UFOM = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_INCO) == TRUE) ?	selectedInfo.bType_05_T_INCO = true : selectedInfo.bType_05_T_INCO = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_JOIB) == TRUE) ?	selectedInfo.bType_05_T_JOIB = true : selectedInfo.bType_05_T_JOIB = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_EFOM) == TRUE) ?	selectedInfo.bType_05_T_EFOM = true : selectedInfo.bType_05_T_EFOM = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_JSUPT) == TRUE) ?	selectedInfo.bType_05_T_JSUPT = true : selectedInfo.bType_05_T_JSUPT = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_WTST) == TRUE) ?	selectedInfo.bType_05_T_WTST = true : selectedInfo.bType_05_T_WTST = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_CLAM) == TRUE) ?	selectedInfo.bType_05_T_CLAM = true : selectedInfo.bType_05_T_CLAM = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_LUMB) == TRUE) ?	selectedInfo.bType_05_T_LUMB = true : selectedInfo.bType_05_T_LUMB = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_TRUS) == TRUE) ?	selectedInfo.bType_05_T_TRUS = true : selectedInfo.bType_05_T_TRUS = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_TBBM) == TRUE) ?	selectedInfo.bType_05_T_TBBM = true : selectedInfo.bType_05_T_TBBM = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_BCWF) == TRUE) ?	selectedInfo.bType_05_T_BCWF = true : selectedInfo.bType_05_T_BCWF = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_PLYW) == TRUE) ?	selectedInfo.bType_05_T_PLYW = true : selectedInfo.bType_05_T_PLYW = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_FISP) == TRUE) ?	selectedInfo.bType_05_T_FISP = true : selectedInfo.bType_05_T_FISP = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_STSE) == TRUE) ?	selectedInfo.bType_05_T_STSE = true : selectedInfo.bType_05_T_STSE = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_SLSE) == TRUE) ?	selectedInfo.bType_05_T_SLSE = true : selectedInfo.bType_05_T_SLSE = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_RAIL) == TRUE) ?	selectedInfo.bType_05_T_RAIL = true : selectedInfo.bType_05_T_RAIL = false;

					// 06-F ���ü�
					(DGGetItemValLong (dialogID, layerInfo.iType_06_F_STRU) == TRUE) ?	selectedInfo.bType_06_F_STRU = true : selectedInfo.bType_06_F_STRU = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_06_F_HFIL) == TRUE) ?	selectedInfo.bType_06_F_HFIL = true : selectedInfo.bType_06_F_HFIL = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_06_F_SJAK) == TRUE) ?	selectedInfo.bType_06_F_SJAK = true : selectedInfo.bType_06_F_SJAK = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_06_F_PJAK) == TRUE) ?	selectedInfo.bType_06_F_PJAK = true : selectedInfo.bType_06_F_PJAK = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_06_F_BRKT) == TRUE) ?	selectedInfo.bType_06_F_BRKT = true : selectedInfo.bType_06_F_BRKT = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_06_F_PBKT) == TRUE) ?	selectedInfo.bType_06_F_PBKT = true : selectedInfo.bType_06_F_PBKT = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_06_F_CIP) == TRUE) ?	selectedInfo.bType_06_F_CIP = true : selectedInfo.bType_06_F_CIP = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_06_F_LAND) == TRUE) ?	selectedInfo.bType_06_F_LAND = true : selectedInfo.bType_06_F_LAND = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_06_F_ANGL) == TRUE) ?	selectedInfo.bType_06_F_ANGL = true : selectedInfo.bType_06_F_ANGL = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_06_F_ERAC) == TRUE) ?	selectedInfo.bType_06_F_ERAC = true : selectedInfo.bType_06_F_ERAC = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_06_F_LUMB) == TRUE) ?	selectedInfo.bType_06_F_LUMB = true : selectedInfo.bType_06_F_LUMB = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_06_F_BPAN) == TRUE) ?	selectedInfo.bType_06_F_BPAN = true : selectedInfo.bType_06_F_BPAN = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_06_F_WALE) == TRUE) ?	selectedInfo.bType_06_F_WALE = true : selectedInfo.bType_06_F_WALE = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_06_F_PILE) == TRUE) ?	selectedInfo.bType_06_F_PILE = true : selectedInfo.bType_06_F_PILE = false;

					// 05-T ������
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_2_TIMB) == TRUE) ?	selectedInfo.bType_05_T_2_TIMB = true : selectedInfo.bType_05_T_2_TIMB = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_2_BIMJ) == TRUE) ?	selectedInfo.bType_05_T_2_BIMJ = true : selectedInfo.bType_05_T_2_BIMJ = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_2_BDCM) == TRUE) ?	selectedInfo.bType_05_T_2_BDCM = true : selectedInfo.bType_05_T_2_BDCM = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_2_DMGA) == TRUE) ?	selectedInfo.bType_05_T_2_DMGA = true : selectedInfo.bType_05_T_2_DMGA = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_2_RIBL) == TRUE) ?	selectedInfo.bType_05_T_2_RIBL = true : selectedInfo.bType_05_T_2_RIBL = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_2_BMSP) == TRUE) ?	selectedInfo.bType_05_T_2_BMSP = true : selectedInfo.bType_05_T_2_BMSP = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_2_BSTA) == TRUE) ?	selectedInfo.bType_05_T_2_BSTA = true : selectedInfo.bType_05_T_2_BSTA = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_2_SPIP) == TRUE) ?	selectedInfo.bType_05_T_2_SPIP = true : selectedInfo.bType_05_T_2_SPIP = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_2_SUPT) == TRUE) ?	selectedInfo.bType_05_T_2_SUPT = true : selectedInfo.bType_05_T_2_SUPT = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_2_SYSU) == TRUE) ?	selectedInfo.bType_05_T_2_SYSU = true : selectedInfo.bType_05_T_2_SYSU = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_2_OUTA) == TRUE) ?	selectedInfo.bType_05_T_2_OUTA = true : selectedInfo.bType_05_T_2_OUTA = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_2_OUTP) == TRUE) ?	selectedInfo.bType_05_T_2_OUTP = true : selectedInfo.bType_05_T_2_OUTP = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_2_AFOM) == TRUE) ?	selectedInfo.bType_05_T_2_AFOM = true : selectedInfo.bType_05_T_2_AFOM = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_2_CPIP) == TRUE) ?	selectedInfo.bType_05_T_2_CPIP = true : selectedInfo.bType_05_T_2_CPIP = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_2_UGON) == TRUE) ?	selectedInfo.bType_05_T_2_UGON = true : selectedInfo.bType_05_T_2_UGON = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_2_UFOM) == TRUE) ?	selectedInfo.bType_05_T_2_UFOM = true : selectedInfo.bType_05_T_2_UFOM = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_2_INCO) == TRUE) ?	selectedInfo.bType_05_T_2_INCO = true : selectedInfo.bType_05_T_2_INCO = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_2_JOIB) == TRUE) ?	selectedInfo.bType_05_T_2_JOIB = true : selectedInfo.bType_05_T_2_JOIB = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_2_EFOM) == TRUE) ?	selectedInfo.bType_05_T_2_EFOM = true : selectedInfo.bType_05_T_2_EFOM = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_2_JSUPT) == TRUE) ?	selectedInfo.bType_05_T_2_JSUPT = true : selectedInfo.bType_05_T_2_JSUPT = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_2_WTST) == TRUE) ?	selectedInfo.bType_05_T_2_WTST = true : selectedInfo.bType_05_T_2_WTST = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_2_CLAM) == TRUE) ?	selectedInfo.bType_05_T_2_CLAM = true : selectedInfo.bType_05_T_2_CLAM = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_2_LUMB) == TRUE) ?	selectedInfo.bType_05_T_2_LUMB = true : selectedInfo.bType_05_T_2_LUMB = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_2_TRUS) == TRUE) ?	selectedInfo.bType_05_T_2_TRUS = true : selectedInfo.bType_05_T_2_TRUS = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_2_TBBM) == TRUE) ?	selectedInfo.bType_05_T_2_TBBM = true : selectedInfo.bType_05_T_2_TBBM = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_2_BCWF) == TRUE) ?	selectedInfo.bType_05_T_2_BCWF = true : selectedInfo.bType_05_T_2_BCWF = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_2_PLYW) == TRUE) ?	selectedInfo.bType_05_T_2_PLYW = true : selectedInfo.bType_05_T_2_PLYW = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_2_FISP) == TRUE) ?	selectedInfo.bType_05_T_2_FISP = true : selectedInfo.bType_05_T_2_FISP = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_2_STSE) == TRUE) ?	selectedInfo.bType_05_T_2_STSE = true : selectedInfo.bType_05_T_2_STSE = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_2_SLSE) == TRUE) ?	selectedInfo.bType_05_T_2_SLSE = true : selectedInfo.bType_05_T_2_SLSE = false;
					(DGGetItemValLong (dialogID, layerInfo.iType_05_T_2_RAIL) == TRUE) ?	selectedInfo.bType_05_T_2_RAIL = true : selectedInfo.bType_05_T_2_RAIL = false;

					break;

				case DG_CANCEL:
					break;
			}
		case DG_MSG_CLOSE:
			switch (item) {
				case DG_CLOSEBOX:
					break;
			}
	}

	result = item;

	return	result;
}