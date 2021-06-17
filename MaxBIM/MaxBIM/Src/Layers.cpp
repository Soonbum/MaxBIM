#include <cstdio>
#include <cstdlib>
#include "MaxBIM.hpp"
#include "Definitions.hpp"
#include "StringConversion.hpp"
#include "UtilityFunctions.hpp"
#include "Layers.hpp"

using namespace	layersDG;

LayerNameSystem	layerInfo;
LayerNameSystem	selectedInfo;
StatusOfLayerNameSystem	selectedInfoSaved;	// 저장된 버튼 상태를 여기로 로드함

short	changedBtnItemIdx;	// 상태가 변경된 버튼의 항목 인덱스
short	clickedBtnItemIdx;	// 클릭한 버튼의 항목 인덱스

short	BUTTON_LOAD;
short	LABEL_CODE;
short	SELECTALL_2_DONG;
short	SELECTALL_3_FLOOR;
short	SELECTALL_4_CAST;
short	SELECTALL_5_CJ;
short	SELECTALL_6_ORDER;
short	SELECTALL_7_OBJ;
short	SELECTALL_8_PRODUCT_SITE;
short	SELECTALL_9_PRODUCT_NUM;

short	DONG_STRING_RADIOBUTTON;
short	DONG_STRING_EDITCONTROL;	// 0101,0102 등 숫자로 된 동 문자열을 담는 Edit 컨트롤의 인덱스 
short	DONG_REST_BUTTONS [100];	// 숫자가 아닌 동 문자열 체크버튼의 인덱스
short	OBJ_BUTTONS [100];			// 부재 버튼의 인덱스

// [다이얼로그] 레이어 옵션 필드 On/Off 여부
bool	extendedLayer;


// 메모리 할당
void		allocateMemory (LayerNameSystem *layerInfo)
{
	short	xx;

	// 레이어 정보 메모리 할당
	layerInfo->code_state			= new bool [layerInfo->code_name.size ()];
	layerInfo->dong_state			= new bool [layerInfo->dong_name.size ()];
	layerInfo->floor_state			= new bool [layerInfo->floor_name.size ()];
	layerInfo->cast_state			= new bool [layerInfo->cast_name.size ()];
	layerInfo->CJ_state				= new bool [layerInfo->CJ_name.size ()];
	layerInfo->orderInCJ_state		= new bool [layerInfo->orderInCJ_name.size ()];
	layerInfo->obj_state			= new bool [layerInfo->obj_name.size ()];
	layerInfo->productSite_state	= new bool [layerInfo->productSite_name.size ()];
	layerInfo->productNum_state		= new bool [layerInfo->productNum_name.size ()];

	layerInfo->code_idx			= new short [layerInfo->code_name.size ()];
	layerInfo->dong_idx			= new short [layerInfo->dong_name.size ()];
	layerInfo->floor_idx		= new short [layerInfo->floor_name.size ()];
	layerInfo->cast_idx			= new short [layerInfo->cast_name.size ()];
	layerInfo->CJ_idx			= new short [layerInfo->CJ_name.size ()];
	layerInfo->orderInCJ_idx	= new short [layerInfo->orderInCJ_name.size ()];
	layerInfo->obj_idx			= new short [layerInfo->obj_name.size ()];
	layerInfo->productSite_idx	= new short [layerInfo->productSite_name.size ()];
	layerInfo->productNum_idx	= new short [layerInfo->productNum_name.size ()];

	// 배열 초기화
	for (xx = 0 ; xx < layerInfo->code_name.size () ; ++xx) {
		layerInfo->code_state [xx] = false;
		layerInfo->code_idx [xx] = 0;
	}
	for (xx = 0 ; xx < layerInfo->dong_name.size () ; ++xx) {
		layerInfo->dong_state [xx] = false;
		layerInfo->dong_idx [xx] = 0;
	}
	for (xx = 0 ; xx < layerInfo->floor_name.size () ; ++xx) {
		layerInfo->floor_state [xx] = false;
		layerInfo->floor_idx [xx] = 0;
	}
	for (xx = 0 ; xx < layerInfo->cast_name.size () ; ++xx) {
		layerInfo->cast_state [xx] = false;
		layerInfo->cast_idx [xx] = 0;
	}
	for (xx = 0 ; xx < layerInfo->CJ_name.size () ; ++xx) {
		layerInfo->CJ_state [xx] = false;
		layerInfo->CJ_idx [xx] = 0;
	}
	for (xx = 0 ; xx < layerInfo->orderInCJ_name.size () ; ++xx) {
		layerInfo->orderInCJ_state [xx] = false;
		layerInfo->orderInCJ_idx [xx] = 0;
	}
	for (xx = 0 ; xx < layerInfo->obj_name.size () ; ++xx) {
		layerInfo->obj_state [xx] = false;
		layerInfo->obj_idx [xx] = 0;
	}
	for (xx = 0 ; xx < layerInfo->productSite_name.size () ; ++xx) {
		layerInfo->productSite_state [xx] = false;
		layerInfo->productSite_idx [xx] = 0;
	}
	for (xx = 0 ; xx < layerInfo->productNum_name.size () ; ++xx) {
		layerInfo->productNum_state [xx] = false;
		layerInfo->productNum_idx [xx] = 0;
	}
}

// 메모리 해제
void		deallocateMemory (LayerNameSystem *layerInfo)
{
	// 레이어 정보 메모리 해제
	delete []	layerInfo->code_state;
	delete []	layerInfo->dong_state;
	delete []	layerInfo->floor_state;
	delete []	layerInfo->cast_state;
	delete []	layerInfo->CJ_state;
	delete []	layerInfo->orderInCJ_state;
	delete []	layerInfo->obj_state;
	delete []	layerInfo->productSite_state;
	delete []	layerInfo->productNum_state;

	delete []	layerInfo->code_idx;
	delete []	layerInfo->dong_idx;
	delete []	layerInfo->floor_idx;
	delete []	layerInfo->cast_idx;
	delete []	layerInfo->CJ_idx;
	delete []	layerInfo->orderInCJ_idx;
	delete []	layerInfo->obj_idx;
	delete []	layerInfo->productSite_idx;
	delete []	layerInfo->productNum_idx;
}

// 레이어 쉽게 선택하기
GSErrCode	showLayersEasily (void)
{
	GSErrCode	err = NoError;

	//FILE	*fp;			// 파일 포인터
	//char	line [10240];	// 파일에서 읽어온 라인 하나
	//string	insElem;		// 토큰을 string으로 변환해서 vector로 넣음
	//char	*token;			// 읽어온 문자열의 토큰
	//short	lineCount;		// 읽어온 라인 수
	//short	tokCount;		// 읽어온 토큰 개수

	//API_Attribute	attrib;
	//short			xx, yy, i;
	//short			nLayers;

	//bool	success;
	//char	tempStr [20];
	//char	tok1 [5];
	//char	tok2 [5];
	//char	tok3 [5];
	//char	tok4 [5];
	//char	tok5 [5];
	//char	tok6 [5];
	//char	tok7 [5];
	//char	tok8 [5];
	//char	constructionCode [5];

	//short	result;


	//// 레이어 정보 파일 가져오기
	//fp = fopen ("C:\\layer.csv", "r");

	//for (lineCount = 1 ; lineCount <= 14 ; ++lineCount) {
	//	if (fp != NULL) {
	//		tokCount = 0;
	//		fscanf (fp, "%s\n", line);
	//		token = strtok (line, ",");
	//		tokCount ++;
	//		while (token != NULL) {
	//			if (strlen (token) > 0) {
	//				if (tokCount > 1) {
	//					insElem = token;

	//					if (lineCount == 1)		layerInfo.code_name.push_back (insElem);		// 공사구분
	//					if (lineCount == 2)		layerInfo.code_desc.push_back (insElem);		// 공사구분 설명
	//					if (lineCount == 3) {
	//						// 만약 동 번호 숫자가 3자리로 들어오면 앞에 0을 붙일 것
	//						if (strlen (token) == 3) {
	//							strcpy (tempStr, "0");
	//							strcat (tempStr, token);
	//							insElem = tempStr;
	//						}

	//						layerInfo.dong_name.push_back (insElem);							// 동
	//					}
	//					if (lineCount == 4)		layerInfo.dong_desc.push_back (insElem);		// 동 설명
	//					if (lineCount == 5)		layerInfo.floor_name.push_back (insElem);		// 층
	//					if (lineCount == 6)		layerInfo.floor_desc.push_back (insElem);		// 층 설명
	//					if (lineCount == 7) {
	//						// 만약 CJ 번호 숫자가 1자리로 들어오면 앞에 0을 붙일 것
	//						if (strlen (token) == 1) {
	//							strcpy (tempStr, "0");
	//							strcat (tempStr, token);
	//							insElem = tempStr;
	//						}

	//						layerInfo.CJ_name.push_back (insElem);			// CJ
	//					}
	//					if (lineCount == 8) {
	//						// 만약 CJ 속 시공순서 번호 숫자가 1자리로 들어오면 앞에 0을 붙일 것
	//						if (strlen (token) == 1) {
	//							strcpy (tempStr, "0");
	//							strcat (tempStr, token);
	//							insElem = tempStr;
	//						}

	//						layerInfo.orderInCJ_name.push_back (insElem);	// CJ 속 시공순서
	//					}
	//					if (lineCount == 9)		layerInfo.obj_name.push_back (insElem);			// 부재
	//					if (lineCount == 10)	layerInfo.obj_desc.push_back (insElem);			// 부재 설명
	//					if (lineCount == 11)	layerInfo.obj_cat.push_back (insElem);			// 부재가 속한 카테고리(공사구분)
	//					if (lineCount == 12)	layerInfo.subObj_name.push_back (insElem);		// 객체
	//					if (lineCount == 13)	layerInfo.subObj_desc.push_back (insElem);		// 객체 설명
	//					if (lineCount == 14)	layerInfo.subObj_cat.push_back (insElem);		// 객체가 속한 카테고리(공사구분)
	//				}
	//			}
	//			token = strtok (NULL, ",");
	//			tokCount ++;
	//		}
	//	} else {
	//		ACAPI_WriteReport ("layer.csv 파일을 C:\\로 복사하십시오.", true);
	//		return	err;
	//	}
	//}

	//fclose (fp);

	//// 구조체 초기화
	//allocateMemory (&layerInfo);
	//selectedInfo = layerInfo;	// selectedInfo에는 vector가 비어 있으므로 초기화를 위해 복사해 둠
	//allocateMemory (&selectedInfo);

	//// 프로젝트 내 레이어 개수를 알아냄
	//BNZeroMemory (&attrib, sizeof (API_Attribute));
	//attrib.header.typeID = API_LayerID;
	//err = ACAPI_Attribute_GetNum (API_LayerID, &nLayers);

	//for (xx = 1; xx <= nLayers && err == NoError ; ++xx) {
	//	attrib.header.index = xx;
	//	err = ACAPI_Attribute_Get (&attrib);
	//	if (err == NoError) {
	//		strcpy (tok1, "");
	//		strcpy (tok2, "");
	//		strcpy (tok3, "");
	//		strcpy (tok4, "");
	//		strcpy (tok5, "");
	//		strcpy (tok6, "");
	//		strcpy (tok7, "");
	//		strcpy (tok8, "");
	//		i = 1;
	//		success = false;
	//		// 정규 코드: 05-T-(0101)-9B1-(01)-(01)-COLU-(UFOM)  단, 괄호 안은 선택사항
	//		// 레이어 이름을 "-" 문자 기준으로 쪼개기
	//		token = strtok (attrib.layer.head.name, "-");
	//		while (token != NULL) {
	//			// 내용 및 길이 확인
	//			// 1차 (일련번호) - 필수 (2글자, 숫자)
	//			if (i == 1) {
	//				strcpy (tempStr, token);
	//				if (strlen (tempStr) == 2) {
	//					strcpy (tok1, tempStr);
	//					success = true;
	//				} else {
	//					i=100;
	//					success = false;
	//				}
	//			}
	//			// 2차 (공사 구분) - 필수 (1글자, 문자)
	//			else if (i == 2) {
	//				strcpy (tempStr, token);
	//				if (strlen (tempStr) == 1) {
	//					strcpy (tok2, tempStr);
	//					success = true;
	//				} else {
	//					i=100;
	//					success = false;
	//				}
	//			}
	//			// 3차 (동 구분) - 선택 (4글자)
	//			else if (i == 3) {
	//				strcpy (tempStr, token);
	//				// 동 구분일 경우,
	//				if (strlen (tempStr) == 4) {
	//					strcpy (tok3, tempStr);
	//					success = true;
	//				// 층 구분일 경우
	//				} else if (strlen (tempStr) == 3) {
	//					strcpy (tok4, tempStr);
	//					i=4;
	//					success = true;
	//				} else {
	//					i=100;
	//					success = false;
	//				}
	//			}
	//			// 4차 (층 구분) - 필수 (3글자)
	//			else if (i == 4) {
	//				strcpy (tempStr, token);
	//				if (strlen (tempStr) == 3) {
	//					strcpy (tok4, tempStr);
	//					success = true;
	//				} else {
	//					i=100;
	//					success = false;
	//				}
	//			}
	//			// 5차 (CJ 구간) - 선택 (2글자, 숫자)
	//			else if (i == 5) {
	//				strcpy (tempStr, token);
	//				if (strlen (tempStr) == 2) {
	//					strcpy (tok5, tempStr);
	//					success = true;
	//				} else if (strlen (tempStr) >= 3) {
	//					strcpy (tok7, tempStr);
	//					i=7;
	//					success = true;
	//				} else {
	//					i=100;
	//					success = false;
	//				}
	//			}
	//			// 6차 (CJ 속 시공순서) - 선택 (2글자, 숫자) - 단 CJ 구간이 없으면 이것도 없어짐
	//			else if (i == 6) {
	//				strcpy (tempStr, token);
	//				if (strlen (tempStr) == 2) {
	//					strcpy (tok6, tempStr);
	//					success = true;
	//				} else if (strlen (tempStr) >= 3) {
	//					strcpy (tok7, tempStr);
	//					i=7;
	//					success = true;
	//				} else {
	//					i=100;
	//					success = false;
	//				}
	//			}
	//			// 7차 (부재 구분) - 필수 (3글자 이상)
	//			else if (i == 7) {
	//				strcpy (tempStr, token);
	//				if (strlen (tempStr) >= 3) {
	//					strcpy (tok7, tempStr);
	//					success = true;
	//				} else {
	//					success = false;
	//				}
	//			}
	//			// 8차 (객체 구분) - 선택 (3글자 이상)
	//			else if (i == 8) {
	//				strcpy (tempStr, token);
	//				if (strlen (tempStr) >= 3) {
	//					strcpy (tok8, tempStr);
	//					success = true;
	//				} else {
	//					success = false;
	//				}
	//			}
	//			++i;
	//			token = strtok (NULL, "-");
	//		}

	//		// 8단계까지 성공적으로 완료되면 구조체에 적용
	//		if (success == true) {
	//			// 일련 번호와 공사 구분 문자를 먼저 합침
	//			sprintf (constructionCode, "%s-%s", tok1, tok2);
	//			
	//			// 1,2단계. 공사 구분 확인
	//			for (yy = 0 ; yy < layerInfo.code_name.size () ; ++yy) {
	//				if (strncmp (constructionCode, layerInfo.code_name [yy].c_str (), 4) == 0)
	//					layerInfo.code_state [yy] = true;
	//			}

	//			// 3단계. 동 구분
	//			for (yy = 0 ; yy < layerInfo.dong_name.size () ; ++yy) {
	//				if (strncmp (tok3, layerInfo.dong_name [yy].c_str (), 4) == 0) {
	//					layerInfo.dong_state [yy] = true;
	//					layerInfo.bDongAllShow = true;
	//				}
	//			}

	//			// 4단계. 층 구분
	//			for (yy = 0 ; yy < layerInfo.floor_name.size () ; ++yy) {
	//				if (strncmp (tok4, layerInfo.floor_name [yy].c_str (), 3) == 0) {
	//					layerInfo.floor_state [yy] = true;
	//				}
	//			}

	//			// 5단계. CJ 구간
	//			for (yy = 0 ; yy < layerInfo.CJ_name.size () ; ++yy) {
	//				if (strncmp (tok5, layerInfo.CJ_name [yy].c_str (), 2) == 0) {
	//					layerInfo.CJ_state [yy] = true;
	//					layerInfo.bCJAllShow = true;
	//				}
	//			}

	//			// 6단계. CJ 속 시공순서
	//			for (yy = 0 ; yy < layerInfo.CJ_name.size () ; ++yy) {
	//				if (strncmp (tok6, layerInfo.orderInCJ_name [yy].c_str (), 2) == 0) {
	//					layerInfo.orderInCJ_state [yy] = true;
	//					layerInfo.bOrderInCJAllShow = true;
	//				}
	//			}

	//			// 7단계. 부재 구분
	//			for (yy = 0 ; yy < layerInfo.obj_name.size () ; ++yy) {
	//				if ((strncmp (constructionCode, layerInfo.obj_cat [yy].c_str (), 4) == 0) && (strncmp (tok7, layerInfo.obj_name [yy].c_str (), 5) == 0)) {
	//					layerInfo.obj_state [yy] = true;
	//				}
	//			}

	//			// 8단계. 객체 구분
	//			for (yy = 0 ; yy < layerInfo.subObj_name.size () ; ++yy) {
	//				if ((strncmp (constructionCode, layerInfo.subObj_cat [yy].c_str (), 4) == 0) && (strncmp (tok8, layerInfo.subObj_name [yy].c_str (), 5) == 0)) {
	//					layerInfo.subObj_state [yy] = true;
	//				}
	//			}
	//		}
	//	}
	//	if (err == APIERR_DELETED)
	//		err = NoError;
	//}

	//// [다이얼로그 박스] 레이어 쉽게 선택하기
	//result = DGBlankModalDialog (700, 450, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, layerShowHandler, 0);

	//// OK 버튼이 아니면 메모리 해제하고 종료
	//if (result != DG_OK) {
	//	deallocateMemory (&layerInfo);
	//	deallocateMemory (&selectedInfo);
	//	return	err;
	//}

	//// 모든 레이어 숨기기
	//BNZeroMemory (&attrib, sizeof (API_Attribute));
	//attrib.header.typeID = API_LayerID;
	//
	//for (xx = 1; xx <= nLayers ; ++xx) {
	//	attrib.header.index = xx;
	//	err = ACAPI_Attribute_Get (&attrib);
	//	if (err == NoError) {
	//		//if (!(attrib.layer.head.flags & APILay_Hidden == true)) {
	//			attrib.layer.head.flags |= APILay_Hidden;
	//			ACAPI_Attribute_Modify (&attrib, NULL);
	//		//}
	//	}
	//}

	//short	z;
	//char	code1 [10][5];		// 공사 코드
	//short	LenCode1;
	//char	code2 [1600][5];	// 동 코드
	//short	LenCode2;
	//char	code3 [120][5];		// 층 코드
	//short	LenCode3;
	//char	code4 [100][5];		// CJ 코드
	//short	LenCode4;
	//char	code5 [100][5];		// CJ 속 시공순서 코드
	//short	LenCode5;
	//char	code6 [90][5];		// 부재 코드
	//short	LenCode6;
	//char	code7 [90][5];		// 객체 코드
	//short	LenCode7;

	//char	fullLayerName [40];
	//short	x1, x2, x3, x4, x5, x6, x7, x8;
	//bool	bCode2, bCode4, bCode5, bCode7;

	//// 1. 공사 코드 문자열 만들기
	//z = 0;
	//for (xx = 0 ; xx < layerInfo.code_name.size () ; ++xx) {
	//	if (selectedInfo.code_state [xx] == true) {
	//		sprintf (tempStr, "%s", layerInfo.code_name [xx].c_str ());
	//		strcpy (code1 [z++], tempStr);
	//	}
	//}
	//LenCode1 = z;

	//// 2. 동 코드 문자열 만들기
	//z = 0;
	//strcpy (code2 [z++], "????");
	//for (xx = 0 ; xx < layerInfo.dong_name.size () ; ++xx) {
	//	if (selectedInfo.dong_state [xx] == true) {
	//		sprintf (tempStr, "%s", layerInfo.dong_name [xx].c_str ());
	//		strcpy (code2 [z++], tempStr);
	//	}
	//}
	//LenCode2 = z;

	//// 3. 층 코드 문자열 만들기
	//z = 0;
	//for (xx = 0 ; xx < layerInfo.floor_name.size () ; ++xx) {
	//	if (selectedInfo.floor_state [xx] == true) {
	//		sprintf (tempStr, "%s", layerInfo.floor_name [xx].c_str ());
	//		strcpy (code3 [z++], tempStr);
	//	}
	//}
	//LenCode3 = z;

	//// 4. CJ 코드 문자열 만들기
	//z = 0;
	//strcpy (code4 [z++], "??");
	//for (xx = 0 ; xx < layerInfo.CJ_name.size () ; ++xx) {
	//	if (selectedInfo.CJ_state [xx] == true) {
	//		sprintf (tempStr, "%s", layerInfo.CJ_name [xx].c_str ());
	//		strcpy (code4 [z++], tempStr);
	//	}
	//}
	//LenCode4 = z;

	//// 5. CJ 속 시공순서 문자열 만들기
	//z = 0;
	//strcpy (code5 [z++], "??");
	//for (xx = 0 ; xx < layerInfo.orderInCJ_name.size () ; ++xx) {
	//	if (selectedInfo.orderInCJ_state [xx] == true) {
	//		sprintf (tempStr, "%s", layerInfo.orderInCJ_name [xx].c_str ());
	//		strcpy (code4 [z++], tempStr);
	//	}
	//}
	//LenCode5 = z;

	//// 6. 부재 코드 문자열 만들기
	//z = 0;
	//for (xx = 0 ; xx < layerInfo.obj_name.size () ; ++xx) {
	//	if (selectedInfo.obj_state [xx] == true) {
	//		sprintf (tempStr, "%s", layerInfo.obj_name [xx].c_str ());
	//		strcpy (code6 [z++], tempStr);
	//	}
	//}
	//LenCode6 = z;

	//// 7. 객체 코드 문자열 만들기 (가설재에 한함)
	//z = 0;
	//strcpy (code7 [z++], "????");
	//for (xx = 0 ; xx < layerInfo.subObj_name.size () ; ++xx) {
	//	if (selectedInfo.subObj_state [xx] == true) {
	//		sprintf (tempStr, "%s", layerInfo.subObj_name [xx].c_str ());
	//		strcpy (code7 [z++], tempStr);
	//	}
	//}
	//LenCode7 = z;

	//bCode2 = false;	bCode4 = false;	bCode5 = false; bCode7 = false;

	//// 레이어 이름 조합하기
	//for (x1 = 0 ; x1 < LenCode1 ; ++x1) {
	//	for (x2 = 0 ; x2 < LenCode2 ; ++x2) {
	//		for (x3 = 0 ; x3 < LenCode3 ; ++x3) {
	//			for (x4 = 0 ; x4 < LenCode4 ; ++x4) {
	//				for (x5 = 0 ; x5 < LenCode5 ; ++x5) {
	//					for (x6 = 0 ; x6 < LenCode6 ; ++x6) {
	//						for (x7 = 0 ; x7 < LenCode7 ; ++x7) {
	//							for (x8 = 1 ; x8 <= 12 ; ++x8) {
	//								// bCode2 (동 코드 포함), bCode4 (CJ 코드 포함), bCode5 (CJ 속 시공순서 코드 포함), bCode7 (부재 하위 객체 코드 포함)
	//								if (x8 == 1) { bCode2 = false;	bCode4 = false;	bCode5 = false;	bCode7 = false;	}
	//								if (x8 == 2) { bCode2 = false;	bCode4 = true;	bCode5 = false;	bCode7 = false;	}
	//								if (x8 == 3) { bCode2 = false;	bCode4 = true;	bCode5 = true;	bCode7 = false;	}
	//								if (x8 == 4) { bCode2 = true;	bCode4 = false;	bCode5 = false;	bCode7 = false;	}
	//								if (x8 == 5) { bCode2 = true;	bCode4 = true;	bCode5 = false;	bCode7 = false;	}
	//								if (x8 == 6) { bCode2 = true;	bCode4 = true;	bCode5 = true;	bCode7 = false;	}

	//								if (x8 == 7) { bCode2 = false;	bCode4 = false;	bCode5 = false;	bCode7 = true;	}
	//								if (x8 == 8) { bCode2 = false;	bCode4 = true;	bCode5 = false;	bCode7 = true;	}
	//								if (x8 == 9) { bCode2 = false;	bCode4 = true;	bCode5 = true;	bCode7 = true;	}
	//								if (x8 == 10) { bCode2 = true;	bCode4 = false;	bCode5 = false;	bCode7 = true;	}
	//								if (x8 == 11) { bCode2 = true;	bCode4 = true;	bCode5 = false;	bCode7 = true;	}
	//								if (x8 == 12) { bCode2 = true;	bCode4 = true;	bCode5 = true;	bCode7 = true;	}

	//								// 공사 구분
	//								strcpy (fullLayerName, "");
	//								strcpy (fullLayerName, code1 [x1]);

	//								// 동 구분
	//								if ((LenCode2 > 1) && (bCode2 == true)) {
	//									strcat (fullLayerName, "-");
	//									strcat (fullLayerName, code2 [x2]);
	//								}

	//								// 층 구분
	//								strcat (fullLayerName, "-");
	//								strcat (fullLayerName, code3 [x3]);

	//								// CJ 구간
	//								if (bCode4 == true) {
	//									if (LenCode4 > 1) {
	//										strcat (fullLayerName, "-");
	//										strcat (fullLayerName, code4 [x4]);
	//									} else {
	//										strcat (fullLayerName, "-00");
	//									}
	//								}

	//								// CJ 속 시공순서
	//								if (bCode5 == true) {
	//									if (LenCode5 > 1) {
	//										strcat (fullLayerName, "-");
	//										strcat (fullLayerName, code5 [x5]);
	//									} else {
	//										strcat (fullLayerName, "-00");
	//									}
	//								}

	//								// 부재
	//								strcat (fullLayerName, "-");
	//								strcat (fullLayerName, code6 [x6]);

	//								// 객체
	//								if (bCode7 == true) {
	//									if (LenCode7 > 1) {
	//										strcat (fullLayerName, "-");
	//										strcat (fullLayerName, code7 [x7]);
	//									} else {
	//										// 없으면 생략
	//									}
	//								}

	//								// 조합한 레이어 이름 검색하기
	//								BNZeroMemory (&attrib, sizeof (API_Attribute));
	//								attrib.header.typeID = API_LayerID;
	//								CHCopyC (fullLayerName, attrib.header.name);
	//								err = ACAPI_Attribute_Get (&attrib);

	//								// 해당 레이어 보여주기
	//								if ((attrib.layer.head.flags & APILay_Hidden) == true) {
	//									attrib.layer.head.flags ^= APILay_Hidden;
	//									ACAPI_Attribute_Modify (&attrib, NULL);
	//								}
	//							}
	//						}
	//					}
	//				}
	//			}
	//		}
	//	}
	//}
	//
	//// 화면 새로고침
	//ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
	//bool	regenerate = true;
	//ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

	//// 메모리 해제
	//deallocateMemory (&layerInfo);
	//deallocateMemory (&selectedInfo);

	return err;
}

// [다이얼로그 박스] 레이어 쉽게 선택하기
short DGCALLBACK layerShowHandler (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	//short	itmIdx;
	//short	itmPosX, itmPosY;
	//short	xx, yy;
	//char	tempStr [20];
	//short	dialogSizeX, dialogSizeY;

	//GSErrCode err = NoError;
	//API_ModulData  info;

	//switch (message) {
	//	case DG_MSG_INIT:
	//		// 다이얼로그 타이틀
	//		DGSetDialogTitle (dialogID, "레이어 쉽게 선택하기: LayerName Ex) 05-T-(0101)-9B1-(01)-(01)-COLU-(UFOM)");

	//		// 확인 버튼
	//		DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 0, 20, 40, 25);
	//		DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
	//		DGSetItemText (dialogID, DG_OK, "확인");
	//		DGShowItem (dialogID, DG_OK);

	//		// 취소 버튼
	//		DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 0, 50, 40, 25);
	//		DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
	//		DGSetItemText (dialogID, DG_CANCEL, "취소");
	//		DGShowItem (dialogID, DG_CANCEL);

	//		// 로드 버튼
	//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 0, 80, 40, 25);
	//		DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
	//		DGSetItemText (dialogID, itmIdx, "로드");
	//		DGShowItem (dialogID, itmIdx);
	//		BUTTON_LOAD = itmIdx;

	//		// 라벨: 코드 보여주기
	//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 0, 120, 50, 23);
	//		DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
	//		DGSetItemText (dialogID, itmIdx, "CODE");
	//		DGShowItem (dialogID, itmIdx);
	//		LABEL_CODE = itmIdx;

	//		// 라벨: 공사 구분
	//		itmPosX = 40;
	//		itmPosY = 25;
	//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 85, 23);
	//		DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
	//		DGSetItemText (dialogID, itmIdx, "공사 구분");
	//		DGShowItem (dialogID, itmIdx);

	//		// 체크박스: 공사 구분 버튼
	//		itmPosX = 150;
	//		itmPosY = 20;
	//		for (xx = 0 ; xx < layerInfo.code_name.size () ; ++xx) {
	//			if (layerInfo.code_state [xx] == true) {
	//				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
	//				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
	//				sprintf (tempStr, "%s %s", layerInfo.code_name [xx].c_str (), layerInfo.code_desc [xx].c_str ());
	//				DGSetItemText (dialogID, itmIdx, tempStr);
	//				DGShowItem (dialogID, itmIdx);
	//				layerInfo.code_idx [xx] = itmIdx;

	//				itmPosX += 100;
	//				if (itmPosX >= 600) {
	//					itmPosX = 150;
	//					itmPosY += 30;
	//				}
	//			}
	//		}
	//		// 모두 선택
	//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
	//		DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_BOLD);
	//		DGSetItemText (dialogID, itmIdx, "모두 선택");
	//		DGShowItem (dialogID, itmIdx);
	//		SELECTALL_1_CONTYPE = itmIdx;
	//		itmPosX += 100;
	//		if (itmPosX >= 600) {
	//			itmPosX = 150;
	//			itmPosY += 30;
	//		}

	//		itmPosY += 30;

	//		// 라벨: 동 구분
	//		itmPosX = 40;
	//		itmPosY += 10;
	//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 85, 23);
	//		DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
	//		DGSetItemText (dialogID, itmIdx, "동 구분");
	//		DGShowItem (dialogID, itmIdx);

	//		// 체크박스: 동 구분 버튼
	//		itmPosX = 150;
	//		itmPosY -= 5;
	//		for (xx = 0 ; xx < layerInfo.dong_name.size () ; ++xx) {
	//			if (layerInfo.dong_state [xx] == true) {
	//				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
	//				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
	//				DGSetItemText (dialogID, itmIdx, layerInfo.dong_desc [xx].c_str ());
	//				DGShowItem (dialogID, itmIdx);
	//				layerInfo.dong_idx [xx] = itmIdx;

	//				itmPosX += 100;
	//				if (itmPosX >= 600) {
	//					itmPosX = 150;
	//					itmPosY += 30;
	//				}
	//			}
	//		}
	//		// 모두 선택
	//		if (layerInfo.bDongAllShow) {
	//			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
	//			DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_BOLD);
	//			DGSetItemText (dialogID, itmIdx, "모두 선택");
	//			DGShowItem (dialogID, itmIdx);
	//			SELECTALL_2_DONG = itmIdx;
	//			itmPosX += 100;
	//			if (itmPosX >= 600) {
	//				itmPosX = 150;
	//				itmPosY += 30;
	//			}
	//		}

	//		itmPosY += 30;

	//		// 라벨: 층 구분
	//		itmPosX = 40;
	//		itmPosY += 10;
	//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 85, 23);
	//		DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
	//		DGSetItemText (dialogID, itmIdx, "층 구분");
	//		DGShowItem (dialogID, itmIdx);

	//		// 체크박스: 층 구분 버튼
	//		itmPosX = 150;
	//		itmPosY -= 5;
	//		for (xx = 0 ; xx < layerInfo.floor_name.size () ; ++xx) {
	//			if (layerInfo.floor_state [xx] == true) {
	//				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
	//				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
	//				DGSetItemText (dialogID, itmIdx, layerInfo.floor_desc [xx].c_str ());
	//				DGShowItem (dialogID, itmIdx);
	//				layerInfo.floor_idx [xx] = itmIdx;

	//				itmPosX += 100;
	//				if (itmPosX >= 600) {
	//					itmPosX = 150;
	//					itmPosY += 30;
	//				}
	//			}
	//		}
	//		// 모두 선택
	//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
	//		DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_BOLD);
	//		DGSetItemText (dialogID, itmIdx, "모두 선택");
	//		DGShowItem (dialogID, itmIdx);
	//		SELECTALL_3_FLOOR = itmIdx;
	//		itmPosX += 100;
	//		if (itmPosX >= 600) {
	//			itmPosX = 150;
	//			itmPosY += 30;
	//		}

	//		itmPosY += 30;

	//		// 라벨: CJ 구간
	//		itmPosX = 40;
	//		itmPosY += 10;
	//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 85, 23);
	//		DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
	//		DGSetItemText (dialogID, itmIdx, "CJ 구간");
	//		DGShowItem (dialogID, itmIdx);

	//		// 체크박스: CJ 구간 버튼
	//		itmPosX = 150;
	//		itmPosY -= 5;
	//		for (xx = 0 ; xx < layerInfo.CJ_name.size () ; ++xx) {
	//			if (layerInfo.CJ_state [xx] == true) {
	//				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
	//				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
	//				DGSetItemText (dialogID, itmIdx, layerInfo.CJ_name [xx].c_str ());
	//				DGShowItem (dialogID, itmIdx);
	//				layerInfo.CJ_idx [xx] = itmIdx;

	//				itmPosX += 100;
	//				if (itmPosX >= 600) {
	//					itmPosX = 150;
	//					itmPosY += 30;
	//				}
	//			}
	//		}
	//		// 모두 선택
	//		if (layerInfo.bCJAllShow) {
	//			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
	//			DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_BOLD);
	//			DGSetItemText (dialogID, itmIdx, "모두 선택");
	//			DGShowItem (dialogID, itmIdx);
	//			SELECTALL_4_CJ = itmIdx;
	//			itmPosX += 100;
	//			if (itmPosX >= 600) {
	//				itmPosX = 150;
	//				itmPosY += 30;
	//			}
	//		}

	//		itmPosY += 30;

	//		// 라벨: CJ 속 시공순서
	//		itmPosX = 40;
	//		itmPosY += 10;
	//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 85, 23);
	//		DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
	//		DGSetItemText (dialogID, itmIdx, "시공순서");
	//		DGShowItem (dialogID, itmIdx);

	//		// 체크박스: CJ 속 시공순서 버튼
	//		itmPosX = 150;
	//		itmPosY -= 5;
	//		for (xx = 0 ; xx < layerInfo.orderInCJ_name.size () ; ++xx) {
	//			if (layerInfo.orderInCJ_state [xx] == true) {
	//				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
	//				DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
	//				DGSetItemText (dialogID, itmIdx, layerInfo.orderInCJ_name [xx].c_str ());
	//				DGShowItem (dialogID, itmIdx);
	//				layerInfo.orderInCJ_idx [xx] = itmIdx;

	//				itmPosX += 100;
	//				if (itmPosX >= 600) {
	//					itmPosX = 150;
	//					itmPosY += 30;
	//				}
	//			}
	//		}
	//		// 모두 선택
	//		if (layerInfo.bOrderInCJAllShow) {
	//			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
	//			DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_BOLD);
	//			DGSetItemText (dialogID, itmIdx, "모두 선택");
	//			DGShowItem (dialogID, itmIdx);
	//			SELECTALL_5_ORDER = itmIdx;
	//			itmPosX += 100;
	//			if (itmPosX >= 600) {
	//				itmPosX = 150;
	//				itmPosY += 30;
	//			}
	//		}

	//		itmPosY += 30;

	//		// 라벨: 부재(구조)
	//		itmPosX = 40;
	//		itmPosY += 10;
	//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 85, 23);
	//		DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
	//		DGSetItemText (dialogID, itmIdx, "*구조");
	//		DGShowItem (dialogID, itmIdx);

	//		// 체크박스: 부재(구조)
	//		itmPosX = 150;
	//		itmPosY -= 5;
	//		for (xx = 0 ; xx < layerInfo.obj_name.size () ; ++xx) {
	//			if (strncmp (layerInfo.obj_cat [xx].c_str (), "01-S", 4) == 0) {
	//				if (layerInfo.obj_state [xx] == true) {
	//					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
	//					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
	//					DGSetItemText (dialogID, itmIdx, layerInfo.obj_desc [xx].c_str ());
	//					DGShowItem (dialogID, itmIdx);
	//					layerInfo.obj_idx [xx] = itmIdx;

	//					itmPosX += 100;
	//					if (itmPosX >= 600) {
	//						itmPosX = 150;
	//						itmPosY += 30;
	//					}
	//				}
	//			}
	//		}

	//		itmPosY += 30;

	//		// 라벨: 부재(건축마감)
	//		itmPosX = 40;
	//		itmPosY += 10;
	//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 85, 23);
	//		DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
	//		DGSetItemText (dialogID, itmIdx, "*건축마감");
	//		DGShowItem (dialogID, itmIdx);

	//		// 체크박스: 부재(건축마감)
	//		itmPosX = 150;
	//		itmPosY -= 5;
	//		for (xx = 0 ; xx < layerInfo.obj_name.size () ; ++xx) {
	//			if (strncmp (layerInfo.obj_cat [xx].c_str (), "02-A", 4) == 0) {
	//				if (layerInfo.obj_state [xx] == true) {
	//					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
	//					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
	//					DGSetItemText (dialogID, itmIdx, layerInfo.obj_desc [xx].c_str ());
	//					DGShowItem (dialogID, itmIdx);
	//					layerInfo.obj_idx [xx] = itmIdx;

	//					itmPosX += 100;
	//					if (itmPosX >= 600) {
	//						itmPosX = 150;
	//						itmPosY += 30;
	//					}
	//				}
	//			}
	//		}

	//		itmPosY += 30;

	//		// 라벨: 부재(가설재)
	//		itmPosX = 40;
	//		itmPosY += 10;
	//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 85, 23);
	//		DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
	//		DGSetItemText (dialogID, itmIdx, "*가설재");
	//		DGShowItem (dialogID, itmIdx);

	//		// 체크박스: 부재(가설재)
	//		itmPosX = 150;
	//		itmPosY -= 5;
	//		for (xx = 0 ; xx < layerInfo.obj_name.size () ; ++xx) {
	//			if (strncmp (layerInfo.obj_cat [xx].c_str (), "05-T", 4) == 0) {
	//				if (layerInfo.obj_state [xx] == true) {
	//					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
	//					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
	//					DGSetItemText (dialogID, itmIdx, layerInfo.obj_desc [xx].c_str ());
	//					DGShowItem (dialogID, itmIdx);
	//					layerInfo.obj_idx [xx] = itmIdx;

	//					itmPosX += 100;
	//					if (itmPosX >= 600) {
	//						itmPosX = 150;
	//						itmPosY += 30;
	//					}
	//				}
	//			}
	//		}

	//		itmPosY += 30;

	//		// 라벨: 부재(가시설)
	//		itmPosX = 40;
	//		itmPosY += 10;
	//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 85, 23);
	//		DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
	//		DGSetItemText (dialogID, itmIdx, "*가시설");
	//		DGShowItem (dialogID, itmIdx);

	//		// 체크박스: 부재(가시설)
	//		itmPosX = 150;
	//		itmPosY -= 5;
	//		for (xx = 0 ; xx < layerInfo.obj_name.size () ; ++xx) {
	//			if (strncmp (layerInfo.obj_cat [xx].c_str (), "06-F", 4) == 0) {
	//				if (layerInfo.obj_state [xx] == true) {
	//					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
	//					DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
	//					DGSetItemText (dialogID, itmIdx, layerInfo.obj_desc [xx].c_str ());
	//					DGShowItem (dialogID, itmIdx);
	//					layerInfo.obj_idx [xx] = itmIdx;

	//					itmPosX += 100;
	//					if (itmPosX >= 600) {
	//						itmPosX = 150;
	//						itmPosY += 30;
	//					}
	//				}
	//			}
	//		}

	//		itmPosY += 30;

	//		// 라벨: 객체(가설재)
	//		itmPosX = 40;
	//		itmPosY += 10;
	//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_RIGHT, DG_FT_NONE, itmPosX, itmPosY, 85, 23);
	//		DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
	//		DGSetItemText (dialogID, itmIdx, "**가설재(세부)");
	//		DGShowItem (dialogID, itmIdx);

	//		// 체크박스: 객체(가설재)
	//		itmPosX = 150;
	//		itmPosY -= 5;
	//		for (xx = 0 ; xx < layerInfo.subObj_name.size () ; ++xx) {
	//			for (yy = 0 ; yy < layerInfo.code_name.size () ; ++yy) {
	//				if ((strncmp (layerInfo.subObj_cat [xx].c_str (), "05-T", 4) == 0) && (strncmp (layerInfo.code_name [yy].c_str (), layerInfo.subObj_cat [xx].c_str (), 4) == 0) && (layerInfo.code_state [yy] == true)) {
	//					if (layerInfo.obj_state [xx] == true) {
	//						itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
	//						DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
	//						DGSetItemText (dialogID, itmIdx, layerInfo.subObj_desc [xx].c_str ());
	//						DGShowItem (dialogID, itmIdx);
	//						layerInfo.subObj_idx [xx] = itmIdx;

	//						itmPosX += 100;
	//						if (itmPosX >= 600) {
	//							itmPosX = 150;
	//							itmPosY += 30;
	//						}
	//					}
	//				}
	//			}
	//		}

	//		dialogSizeX = 700;
	//		dialogSizeY = itmPosY + 150;
	//		DGSetDialogSize (dialogID, DG_FRAME, dialogSizeX, dialogSizeY, DG_TOPLEFT, true);

	//		break;
	//	
	//	case DG_MSG_CHANGE:
	//		changedBtnItemIdx = item;

	//		if (changedBtnItemIdx == SELECTALL_1_CONTYPE) {
	//			if (DGGetItemValLong (dialogID, SELECTALL_1_CONTYPE) == TRUE) {
	//				// 모두 선택
	//				for (xx = 0 ; xx < layerInfo.code_name.size () ; ++xx)
	//					DGSetItemValLong (dialogID, layerInfo.code_idx [xx], TRUE);
	//			} else {
	//				// 모두 제외
	//				for (xx = 0 ; xx < layerInfo.code_name.size () ; ++xx)
	//					DGSetItemValLong (dialogID, layerInfo.code_idx [xx], FALSE);
	//			}
	//		}
	//		if (changedBtnItemIdx == SELECTALL_2_DONG) {
	//			if (DGGetItemValLong (dialogID, SELECTALL_2_DONG) == TRUE) {
	//				// 모두 선택
	//				for (xx = 0 ; xx < layerInfo.dong_name.size () ; ++xx)
	//					DGSetItemValLong (dialogID, layerInfo.dong_idx [xx], TRUE);
	//			} else {
	//				// 모두 제외
	//				for (xx = 0 ; xx < layerInfo.dong_name.size () ; ++xx)
	//					DGSetItemValLong (dialogID, layerInfo.dong_idx [xx], FALSE);
	//			}
	//		}
	//		if (changedBtnItemIdx == SELECTALL_3_FLOOR) {
	//			if (DGGetItemValLong (dialogID, SELECTALL_3_FLOOR) == TRUE) {
	//				// 모두 선택
	//				for (xx = 0 ; xx < layerInfo.floor_name.size () ; ++xx)
	//					DGSetItemValLong (dialogID, layerInfo.floor_idx [xx], TRUE);
	//			} else {
	//				// 모두 제외
	//				for (xx = 0 ; xx < layerInfo.floor_name.size () ; ++xx)
	//					DGSetItemValLong (dialogID, layerInfo.floor_idx [xx], FALSE);
	//			}
	//		}
	//		if (changedBtnItemIdx == SELECTALL_4_CJ) {
	//			if (DGGetItemValLong (dialogID, SELECTALL_4_CJ) == TRUE) {
	//				// 모두 선택
	//				for (xx = 0 ; xx < layerInfo.CJ_name.size () ; ++xx)
	//					DGSetItemValLong (dialogID, layerInfo.CJ_idx [xx], TRUE);
	//			} else {
	//				// 모두 제외
	//				for (xx = 0 ; xx < layerInfo.CJ_name.size () ; ++xx)
	//					DGSetItemValLong (dialogID, layerInfo.CJ_idx [xx], FALSE);
	//			}
	//		}
	//		if (changedBtnItemIdx == SELECTALL_5_ORDER) {
	//			if (DGGetItemValLong (dialogID, SELECTALL_5_ORDER) == TRUE) {
	//				// 모두 선택
	//				for (xx = 0 ; xx < layerInfo.orderInCJ_name.size () ; ++xx)
	//					DGSetItemValLong (dialogID, layerInfo.orderInCJ_idx [xx], TRUE);
	//			} else {
	//				// 모두 제외
	//				for (xx = 0 ; xx < layerInfo.orderInCJ_name.size () ; ++xx)
	//					DGSetItemValLong (dialogID, layerInfo.orderInCJ_idx [xx], FALSE);
	//			}
	//		}

	//		// 부재를 선택하면 공사 코드를 자동 선택
	//		for (xx = 0 ; xx < layerInfo.obj_name.size () ; ++xx) {
	//			if (DGGetItemValLong (dialogID, layerInfo.obj_idx [xx]) == TRUE) {
	//				for (yy = 0 ; yy < layerInfo.code_name.size () ; ++yy) {
	//					if (strncmp (layerInfo.obj_cat [xx].c_str (), layerInfo.code_name [yy].c_str (), 4) == 0) {
	//						DGSetItemValLong (dialogID, layerInfo.code_idx [yy], TRUE);
	//					}
	//				}
	//			}
	//		}

	//		// 객체를 선택하면 공사 코드를 자동 선택
	//		for (xx = 0 ; xx < layerInfo.subObj_name.size () ; ++xx) {
	//			if (DGGetItemValLong (dialogID, layerInfo.subObj_idx [xx]) == TRUE) {
	//				for (yy = 0 ; yy < layerInfo.code_name.size () ; ++yy) {
	//					if (strncmp (layerInfo.subObj_cat [xx].c_str (), layerInfo.code_name [yy].c_str (), 4) == 0) {
	//						DGSetItemValLong (dialogID, layerInfo.code_idx [yy], TRUE);
	//					}
	//				}
	//			}
	//		}

	//		// 버튼의 이름 보여주기
	//		for (xx = 0 ; xx < layerInfo.code_name.size () ; ++xx) {
	//			if ((layerInfo.code_idx [xx] == changedBtnItemIdx) && (layerInfo.code_state [xx] == true)) {
	//				sprintf (tempStr, "%s", layerInfo.code_name [xx].c_str ());
	//				DGSetItemText (dialogID, LABEL_CODE, tempStr);
	//			}
	//		}
	//		for (xx = 0 ; xx < layerInfo.dong_name.size () ; ++xx) {
	//			if ((layerInfo.dong_idx [xx] == changedBtnItemIdx) && (layerInfo.dong_state [xx] == true)) {
	//				sprintf (tempStr, "%s", layerInfo.dong_name [xx].c_str ());
	//				DGSetItemText (dialogID, LABEL_CODE, tempStr);
	//			}
	//		}
	//		for (xx = 0 ; xx < layerInfo.floor_name.size () ; ++xx) {
	//			if ((layerInfo.floor_idx [xx] == changedBtnItemIdx) && (layerInfo.floor_state [xx] == true)) {
	//				sprintf (tempStr, "%s", layerInfo.floor_name [xx].c_str ());
	//				DGSetItemText (dialogID, LABEL_CODE, tempStr);
	//			}
	//		}
	//		for (xx = 0 ; xx < layerInfo.CJ_name.size () ; ++xx) {
	//			if ((layerInfo.CJ_idx [xx] == changedBtnItemIdx) && (layerInfo.CJ_state [xx] == true)) {
	//				sprintf (tempStr, "%s", layerInfo.CJ_name [xx].c_str ());
	//				DGSetItemText (dialogID, LABEL_CODE, tempStr);
	//			}
	//		}
	//		for (xx = 0 ; xx < layerInfo.orderInCJ_name.size () ; ++xx) {
	//			if ((layerInfo.orderInCJ_idx [xx] == changedBtnItemIdx) && (layerInfo.orderInCJ_state [xx] == true)) {
	//				sprintf (tempStr, "%s", layerInfo.orderInCJ_name [xx].c_str ());
	//				DGSetItemText (dialogID, LABEL_CODE, tempStr);
	//			}
	//		}
	//		for (xx = 0 ; xx < layerInfo.obj_name.size () ; ++xx) {
	//			if ((layerInfo.obj_idx [xx] == changedBtnItemIdx) && (layerInfo.obj_state [xx] == true)) {
	//				sprintf (tempStr, "%s", layerInfo.obj_name [xx].c_str ());
	//				DGSetItemText (dialogID, LABEL_CODE, tempStr);
	//			}
	//		}
	//		for (xx = 0 ; xx < layerInfo.subObj_name.size () ; ++xx) {
	//			if ((layerInfo.subObj_idx [xx] == changedBtnItemIdx) && (layerInfo.subObj_state [xx] == true)) {
	//				sprintf (tempStr, "%s", layerInfo.subObj_name [xx].c_str ());
	//				DGSetItemText (dialogID, LABEL_CODE, tempStr);
	//			}
	//		}

	//		break;

	//	case DG_MSG_CLICK:
	//		switch (item) {
	//			case DG_OK:
	//				// 공사 구분
	//				for (xx = 0 ; xx < layerInfo.code_name.size () ; ++xx)
	//					(DGGetItemValLong (dialogID, layerInfo.code_idx [xx]) == TRUE) ? selectedInfo.code_state [xx] = true : selectedInfo.code_state [xx] = false;

	//				// 동 구분
	//				for (xx = 0 ; xx < layerInfo.dong_name.size () ; ++xx)
	//					(DGGetItemValLong (dialogID, layerInfo.dong_idx [xx]) == TRUE) ? selectedInfo.dong_state [xx] = true : selectedInfo.dong_state [xx] = false;

	//				// 층 구분
	//				for (xx = 0 ; xx < layerInfo.floor_name.size () ; ++xx)
	//					(DGGetItemValLong (dialogID, layerInfo.floor_idx [xx]) == TRUE) ? selectedInfo.floor_state [xx] = true : selectedInfo.floor_state [xx] = false;

	//				// CJ
	//				for (xx = 0 ; xx < layerInfo.CJ_name.size () ; ++xx)
	//					(DGGetItemValLong (dialogID, layerInfo.CJ_idx [xx]) == TRUE) ? selectedInfo.CJ_state [xx] = true : selectedInfo.CJ_state [xx] = false;

	//				// CJ 속 시공순서
	//				for (xx = 0 ; xx < layerInfo.orderInCJ_name.size () ; ++xx)
	//					(DGGetItemValLong (dialogID, layerInfo.orderInCJ_idx [xx]) == TRUE) ? selectedInfo.orderInCJ_state [xx] = true : selectedInfo.orderInCJ_state [xx] = false;

	//				// 부재
	//				for (xx = 0 ; xx < layerInfo.obj_name.size () ; ++xx)
	//					(DGGetItemValLong (dialogID, layerInfo.obj_idx [xx]) == TRUE) ? selectedInfo.obj_state [xx] = true : selectedInfo.obj_state [xx] = false;

	//				// 객체
	//				for (xx = 0 ; xx < layerInfo.subObj_name.size () ; ++xx)
	//					(DGGetItemValLong (dialogID, layerInfo.subObj_idx [xx]) == TRUE) ? selectedInfo.subObj_state [xx] = true : selectedInfo.subObj_state [xx] = false;

	//				// 버튼 상태 저장
	//				saveButtonStatus ();

	//				break;

	//			case DG_CANCEL:
	//				break;

	//			default:
	//				clickedBtnItemIdx = item;
	//				item = 0;

	//				// 저장된 버튼 상태를 불러옴
	//				if (clickedBtnItemIdx == BUTTON_LOAD) {
	//					BNZeroMemory (&info, sizeof (API_ModulData));
	//					err = ACAPI_ModulData_Get (&info, "ButtonStatus");

	//					if (err == NoError && info.dataVersion == 1) {
	//						selectedInfoSaved = *(reinterpret_cast<StatusOfLayerNameSystem*> (*info.dataHdl));
	//						
	//						for (xx = 0 ; xx < layerInfo.code_name.size () ; ++xx) {
	//							if (selectedInfoSaved.code_state [xx] == true)
	//								DGSetItemValLong (dialogID, layerInfo.code_idx [xx], TRUE);
	//						}
	//						for (xx = 0 ; xx < layerInfo.dong_name.size () ; ++xx) {
	//							if (selectedInfoSaved.dong_state [xx] == true)
	//								DGSetItemValLong (dialogID, layerInfo.dong_idx [xx], TRUE);
	//						}
	//						for (xx = 0 ; xx < layerInfo.floor_name.size () ; ++xx) {
	//							if (selectedInfoSaved.floor_state [xx] == true)
	//								DGSetItemValLong (dialogID, layerInfo.floor_idx [xx], TRUE);
	//						}
	//						for (xx = 0 ; xx < layerInfo.CJ_name.size () ; ++xx) {
	//							if (selectedInfoSaved.CJ_state [xx] == true)
	//								DGSetItemValLong (dialogID, layerInfo.CJ_idx [xx], TRUE);
	//						}
	//						for (xx = 0 ; xx < layerInfo.orderInCJ_name.size () ; ++xx) {
	//							if (selectedInfoSaved.orderInCJ_state [xx] == true)
	//								DGSetItemValLong (dialogID, layerInfo.orderInCJ_idx [xx], TRUE);
	//						}
	//						for (xx = 0 ; xx < layerInfo.obj_name.size () ; ++xx) {
	//							if (selectedInfoSaved.obj_state [xx] == true)
	//								DGSetItemValLong (dialogID, layerInfo.obj_idx [xx], TRUE);
	//						}
	//						for (xx = 0 ; xx < layerInfo.subObj_name.size () ; ++xx) {
	//							if (selectedInfoSaved.subObj_state [xx] == true)
	//								DGSetItemValLong (dialogID, layerInfo.subObj_idx [xx], TRUE);
	//						}
	//					}

	//					BMKillHandle (&info.dataHdl);
	//				}

	//				break;
	//		}
	//		break;

	//	case DG_MSG_CLOSE:
	//		switch (item) {
	//			case DG_CLOSEBOX:
	//				break;
	//		}
	//}

	//result = item;

	return	result;
}

// 최근 버튼 상태 저장하기
GSErrCode	saveButtonStatus (void)
{
	GSErrCode err = NoError;

	//short	xx;
	//API_ModulData	info;
	//BNZeroMemory (&info, sizeof (API_ModulData));
	//info.dataVersion = 1;
	//info.platformSign = GS::Act_Platform_Sign;
	//info.dataHdl = BMAllocateHandle (sizeof (StatusOfLayerNameSystem), 0, 0);
	//if (info.dataHdl != NULL) {

	//	for (xx = 0 ; xx < layerInfo.code_name.size () ; ++xx)
	//		selectedInfoSaved.code_state [xx] = selectedInfo.code_state [xx];
	//	for (xx = 0 ; xx < layerInfo.dong_name.size () ; ++xx)
	//		selectedInfoSaved.dong_state [xx] = selectedInfo.dong_state [xx];
	//	for (xx = 0 ; xx < layerInfo.floor_name.size () ; ++xx)
	//		selectedInfoSaved.floor_state [xx] = selectedInfo.floor_state [xx];
	//	for (xx = 0 ; xx < layerInfo.CJ_name.size () ; ++xx)
	//		selectedInfoSaved.CJ_state [xx] = selectedInfo.CJ_state [xx];
	//	for (xx = 0 ; xx < layerInfo.orderInCJ_name.size () ; ++xx)
	//		selectedInfoSaved.orderInCJ_state [xx] = selectedInfo.orderInCJ_state [xx];
	//	for (xx = 0 ; xx < layerInfo.obj_name.size () ; ++xx)
	//		selectedInfoSaved.obj_state [xx] = selectedInfo.obj_state [xx];
	//	for (xx = 0 ; xx < layerInfo.subObj_name.size () ; ++xx)
	//		selectedInfoSaved.subObj_state [xx] = selectedInfo.subObj_state [xx];

	//	*(reinterpret_cast<StatusOfLayerNameSystem*> (*info.dataHdl)) = selectedInfoSaved;
	//	err = ACAPI_ModulData_Store (&info, "ButtonStatus");
	//	BMKillHandle (&info.dataHdl);
	//} else {
	//	err = APIERR_MEMFULL;
	//}

	return	err;
}

// 레이어 쉽게 만들기
GSErrCode	makeLayersEasily (void)
{
	GSErrCode	err = NoError;

	FILE	*fp;			// 파일 포인터
	char	line [10240];	// 파일에서 읽어온 라인 하나
	string	insElem;		// 토큰을 string으로 변환해서 vector로 넣음
	char	*token;			// 읽어온 문자열의 토큰
	short	lineCount;		// 읽어온 라인 수
	short	tokCount;		// 읽어온 토큰 개수

	API_Attribute	attrib;
	API_AttributeDef  defs;
	short			xx;

	char	tempStr [30];

	short	result;


	// 레이어 정보 파일 가져오기
	fp = fopen ("C:\\layer.csv", "r");

	for (lineCount = 1 ; lineCount <= 14 ; ++lineCount) {
		if (fp != NULL) {
			tokCount = 0;
			fscanf (fp, "%s\n", line);
			token = strtok (line, ",");
			tokCount ++;
			while (token != NULL) {
				if (strlen (token) > 0) {
					if (tokCount > 1) {
						insElem = token;

						if (lineCount == 1)		layerInfo.code_name.push_back (insElem);		// 공사구분
						if (lineCount == 2)		layerInfo.code_desc.push_back (insElem);		// 공사구분 설명
						if (lineCount == 3) {
							// 만약 동 번호 숫자가 1자리로 들어오면 앞에 000을 붙일 것
							if (strlen (token) == 1) {
								strcpy (tempStr, "000");
								strcat (tempStr, token);
								insElem = tempStr;
							}

							// 만약 동 번호 숫자가 3자리로 들어오면 앞에 0을 붙일 것
							if (strlen (token) == 3) {
								strcpy (tempStr, "0");
								strcat (tempStr, token);
								insElem = tempStr;
							}

							layerInfo.dong_name.push_back (insElem);							// 동
						}
						if (lineCount == 4)		layerInfo.dong_desc.push_back (insElem);		// 동 설명
						if (lineCount == 5)		layerInfo.floor_name.push_back (insElem);		// 층
						if (lineCount == 6)		layerInfo.floor_desc.push_back (insElem);		// 층 설명
						if (lineCount == 7) {
							// 만약 타설번호 숫자가 1자리로 들어오면 앞에 0을 붙일 것
							if (strlen (token) == 1) {
								strcpy (tempStr, "0");
								strcat (tempStr, token);
								insElem = tempStr;
							}

							layerInfo.cast_name.push_back (insElem);		// 타설번호
						}
						if (lineCount == 8) {
							// 만약 CJ 번호 숫자가 1자리로 들어오면 앞에 0을 붙일 것
							if (strlen (token) == 1) {
								strcpy (tempStr, "0");
								strcat (tempStr, token);
								insElem = tempStr;
							}

							layerInfo.CJ_name.push_back (insElem);			// CJ
						}
						if (lineCount == 9) {
							// 만약 CJ 속 시공순서 번호 숫자가 1자리로 들어오면 앞에 0을 붙일 것
							if (strlen (token) == 1) {
								strcpy (tempStr, "0");
								strcat (tempStr, token);
								insElem = tempStr;
							}

							layerInfo.orderInCJ_name.push_back (insElem);	// CJ 속 시공순서
						}
						if (lineCount == 10)	layerInfo.obj_name.push_back (insElem);			// 부재
						if (lineCount == 11)	layerInfo.obj_desc.push_back (insElem);			// 부재 설명
						if (lineCount == 12)	layerInfo.obj_cat.push_back (insElem);			// 부재가 속한 카테고리(공사구분)
						if (lineCount == 13)	layerInfo.productSite_name.push_back (insElem);	// 제작처 구분
						if (lineCount == 14) {
							// 만약 제작 번호가 1자리로 들어오면 앞에 00을 붙일 것
							if (strlen (token) == 1) {
								strcpy (tempStr, "00");
								strcat (tempStr, token);
								insElem = tempStr;
							}
							// 만약 제작 번호가 2자리로 들어오면 앞에 0을 붙일 것
							if (strlen (token) == 2) {
								strcpy (tempStr, "0");
								strcat (tempStr, token);
								insElem = tempStr;
							}

							layerInfo.productNum_name.push_back (insElem);	// 제작 번호
						}
					}
				}
				token = strtok (NULL, ",");
				tokCount ++;
			}
		} else {
			ACAPI_WriteReport ("layer.csv 파일을 C:\\로 복사하십시오.", true);
			return	err;
		}
	}

	fclose (fp);

	// 구조체 초기화
	allocateMemory (&layerInfo);

	// [다이얼로그 박스] 레이어 쉽게 만들기
	result = DGBlankModalDialog (1100, 120, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, layerMakeHandler, 0);

	// OK 버튼이 아니면 메모리 해제하고 종료
	if (result != DG_OK) {
		deallocateMemory (&layerInfo);
		return	err;
	}

	//short	z;
	//char	code1 [10][5];		// 공사 코드
	//short	LenCode1;
	//char	code2 [1600][5];	// 동 코드
	//short	LenCode2;
	//char	code3 [120][5];		// 층 코드
	//short	LenCode3;
	//char	code4 [100][5];		// CJ 코드
	//short	LenCode4;
	//char	code5 [100][5];		// CJ 속 시공순서 코드
	//short	LenCode5;
	//char	code6 [90][5];		// 부재 코드
	//short	LenCode6;
	//char	code7 [90][5];		// 객체 코드
	//short	LenCode7;

	//char	fullLayerName [40];
	//short	madeLayers;
	//bool	bNormalLayer;
	//short	x1, x2, x3, x4, x5, x6, x7;

	//// 1. 공사 코드 문자열 만들기
	//z = 0;
	//for (xx = 0 ; xx < layerInfo.code_name.size () ; ++xx) {
	//	if (layerInfo.code_state [xx] == true) {
	//		sprintf (tempStr, "%s", layerInfo.code_name [xx].c_str ());
	//		strcpy (code1 [z++], tempStr);
	//	}
	//}
	//LenCode1 = z;

	//// 2. 동 코드 문자열 만들기
	//z = 0;
	//strcpy (code2 [z++], "????");
	//for (xx = 0 ; xx < layerInfo.dong_name.size () ; ++xx) {
	//	if (layerInfo.dong_state [xx] == true) {
	//		sprintf (tempStr, "%s", layerInfo.dong_name [xx].c_str ());
	//		strcpy (code2 [z++], tempStr);
	//	}
	//}
	//LenCode2 = z;

	//// 3. 층 코드 문자열 만들기
	//z = 0;
	//for (xx = 0 ; xx < layerInfo.floor_name.size () ; ++xx) {
	//	if (layerInfo.floor_state [xx] == true) {
	//		sprintf (tempStr, "%s", layerInfo.floor_name [xx].c_str ());
	//		strcpy (code3 [z++], tempStr);
	//	}
	//}
	//LenCode3 = z;

	//// 4. CJ 코드 문자열 만들기
	//z = 0;
	//strcpy (code4 [z++], "??");
	//for (xx = 0 ; xx < layerInfo.CJ_name.size () ; ++xx) {
	//	if (layerInfo.CJ_state [xx] == true) {
	//		sprintf (tempStr, "%s", layerInfo.CJ_name [xx].c_str ());
	//		strcpy (code4 [z++], tempStr);
	//	}
	//}
	//LenCode4 = z;

	//// 5. CJ 속 시공순서 문자열 만들기
	//z = 0;
	//strcpy (code5 [z++], "??");
	//for (xx = 0 ; xx < layerInfo.orderInCJ_name.size () ; ++xx) {
	//	if (layerInfo.orderInCJ_state [xx] == true) {
	//		sprintf (tempStr, "%s", layerInfo.orderInCJ_name [xx].c_str ());
	//		strcpy (code4 [z++], tempStr);
	//	}
	//}
	//LenCode5 = z;

	//// 6. 부재 코드 문자열 만들기
	//z = 0;
	//for (xx = 0 ; xx < layerInfo.obj_name.size () ; ++xx) {
	//	if (layerInfo.obj_state [xx] == true) {
	//		sprintf (tempStr, "%s", layerInfo.obj_name [xx].c_str ());
	//		strcpy (code6 [z++], tempStr);
	//	}
	//}
	//LenCode6 = z;

	//// 7. 객체 코드 문자열 만들기 (가설재에 한함)
	//z = 0;
	//strcpy (code7 [z++], "????");
	//for (xx = 0 ; xx < layerInfo.subObj_name.size () ; ++xx) {
	//	if (layerInfo.subObj_state [xx] == true) {
	//		sprintf (tempStr, "%s", layerInfo.subObj_name [xx].c_str ());
	//		strcpy (code7 [z++], tempStr);
	//	}
	//}
	//LenCode7 = z;

	//// 생성한 레이어 개수
	//madeLayers = 0;

	//// 레이어 이름 조합하기
	//for (x1 = 0 ; x1 < LenCode1 ; ++x1) {
	//	for (x2 = 0 ; x2 < LenCode2 ; ++x2) {
	//		for (x3 = 0 ; x3 < LenCode3 ; ++x3) {
	//			for (x4 = 0 ; x4 < LenCode4 ; ++x4) {
	//				for (x5 = 0 ; x5 < LenCode5 ; ++x5) {
	//					for (x6 = 0 ; x6 < LenCode6 ; ++x6) {
	//						for (x7 = 0 ; x7 < LenCode7 ; ++x7) {

	//							bNormalLayer = false;	// 레이어 이름이 정상적인 체계의 이름인가?

	//							// 공사 구분
	//							strcpy (fullLayerName, "");
	//							strcpy (fullLayerName, code1 [x1]);

	//							// 동 구분
	//							if ((LenCode2 > 1) && (onCode2 == true)) {
	//								strcat (fullLayerName, "-");
	//								strcat (fullLayerName, code2 [x2]);

	//								if (strncmp (code2 [x2], "????", 4) == 0) {
	//									bNormalLayer = false;
	//									continue;
	//								}
	//							}

	//							// 층 구분
	//							strcat (fullLayerName, "-");
	//							strcat (fullLayerName, code3 [x3]);

	//							// CJ 구간
	//							if (onCode4 == true) {
	//								if (LenCode4 > 1) {
	//									strcat (fullLayerName, "-");
	//									strcat (fullLayerName, code4 [x4]);

	//									if (strncmp (code4 [x4], "??", 2) == 0) {
	//										bNormalLayer = false;
	//										continue;
	//									}
	//								} else {
	//									strcat (fullLayerName, "-00");
	//								}
	//							}

	//							// CJ 속 시공순서
	//							if (onCode5 == true) {
	//								if (LenCode5 > 1) {
	//									strcat (fullLayerName, "-");
	//									strcat (fullLayerName, code5 [x5]);

	//									if (strncmp (code5 [x5], "??", 2) == 0) {
	//										bNormalLayer = false;
	//										continue;
	//									}
	//								} else {
	//									strcat (fullLayerName, "-00");
	//								}
	//							}

	//							// 부재
	//							if (onCode7 == true) {
	//								strcat (fullLayerName, "-");
	//								strcat (fullLayerName, code6 [x6]);
	//							} else {
	//								// 단, 객체 필드가 없으면 부재의 카테고리는 공사 구분 코드에 속한 것이어야 함
	//								for (xx = 0 ; xx < layerInfo.obj_name.size () ; ++xx) {
	//									if (strncmp (code6 [x6], layerInfo.obj_name [xx].c_str (), strlen (code6 [x6])) == 0) {
	//										if (strncmp (layerInfo.obj_cat [xx].c_str (), code1 [x1], 4) == 0)
	//											bNormalLayer = true;
	//										else
	//											bNormalLayer = false;
	//									}
	//								}

	//								strcat (fullLayerName, "-");
	//								strcat (fullLayerName, code6 [x6]);
	//							}

	//							// 객체
	//							if (onCode7 == true) {
	//								if (LenCode7 > 1) {
	//									// 객체의 카테고리는 공사 구분 코드에 속한 것이어야 함
	//									for (xx = 0 ; xx < layerInfo.subObj_name.size () ; ++xx) {
	//										if (strncmp (code7 [x7], layerInfo.subObj_name [xx].c_str (), strlen (code7 [x7])) == 0) {
	//											if (strncmp (layerInfo.subObj_cat [xx].c_str (), code1 [x1], 4) == 0)
	//												bNormalLayer = true;
	//											else
	//												bNormalLayer = false;
	//										}
	//									}

	//									strcat (fullLayerName, "-");
	//									strcat (fullLayerName, code7 [x7]);
	//								} else {
	//									// 없으면 생략
	//								}
	//							}

	//							// 정상적인 레이어 이름이면 레이어 속성을 생성함
	//							if (bNormalLayer == true) {
	//								// 레이어 생성하기
	//								BNZeroMemory (&attrib, sizeof (API_Attribute));
	//								BNZeroMemory (&defs, sizeof (API_AttributeDef));

	//								attrib.header.typeID = API_LayerID;
	//								CHCopyC (fullLayerName, attrib.header.name);
	//								err = ACAPI_Attribute_Create (&attrib, &defs);

	//								ACAPI_DisposeAttrDefsHdls (&defs);

	//								// 성공하면 레이어 생성 개수 누적
	//								if (err == NoError)
	//									madeLayers ++;
	//							}
	//						}
	//					}
	//				}
	//			}
	//		}
	//	}
	//}

	//// 메모리 해제
	//deallocateMemory (&layerInfo);

	//// 화면 새로고침
	//ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
	//bool	regenerate = true;
	//ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

	//// 생성된 레이어 개수 보여주기
	//sprintf (tempStr, "총 %d 개의 레이어가 생성되었습니다.", madeLayers);
	//ACAPI_WriteReport (tempStr, true);

	return	err;
}

// [다이얼로그 박스] 레이어 쉽게 만들기
short DGCALLBACK layerMakeHandler (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	short	itmIdx;
	short	itmPosX, itmPosY;
	short	dialogSizeX, dialogSizeY;
	short	xx;
	short	anyTrue;

	switch (message) {
		case DG_MSG_INIT:
			// 다이얼로그 타이틀
			DGSetDialogTitle (dialogID, "레이어 쉽게 만들기");

			// 확인 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 820, 20, 60, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, "생성");
			DGShowItem (dialogID, DG_OK);

			// 취소 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 820, 55, 60, 25);
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, "취소");
			DGShowItem (dialogID, DG_CANCEL);

			// 버튼: 공사 구분
			itmPosX = 30;
			itmPosY = 50;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, itmPosY, 80, 23);
			DGSetItemFont (dialogID, BUTTON_CODE, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, BUTTON_CODE, "공사 구분");
			DGShowItem (dialogID, BUTTON_CODE);

			// 버튼: 동 구분
			itmPosX += 110;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, itmPosY, 80, 23);
			DGSetItemFont (dialogID, BUTTON_DONG, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, BUTTON_DONG, "동 구분");
			DGShowItem (dialogID, BUTTON_DONG);

			// 버튼: 층 구분
			itmPosX += 110;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, itmPosY, 80, 23);
			DGSetItemFont (dialogID, BUTTON_FLOOR, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, BUTTON_FLOOR, "층 구분");
			DGShowItem (dialogID, BUTTON_FLOOR);

			// 버튼: 타설 번호
			itmPosX += 110;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, itmPosY, 80, 23);
			DGSetItemFont (dialogID, BUTTON_CAST, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, BUTTON_CAST, "타설 번호");
			DGShowItem (dialogID, BUTTON_CAST);

			// 버튼: CJ
			itmPosX += 110;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, itmPosY, 80, 23);
			DGSetItemFont (dialogID, BUTTON_CJ, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, BUTTON_CJ, "CJ");
			DGShowItem (dialogID, BUTTON_CJ);

			// 버튼: 시공순서
			itmPosX += 110;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, itmPosY, 80, 23);
			DGSetItemFont (dialogID, BUTTON_ORDER, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, BUTTON_ORDER, "시공순서");
			DGShowItem (dialogID, BUTTON_ORDER);

			// 버튼: 부재
			itmPosX += 110;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, itmPosY, 80, 23);
			DGSetItemFont (dialogID, BUTTON_OBJ, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, BUTTON_OBJ, "부재");
			DGShowItem (dialogID, BUTTON_OBJ);

			// 버튼: 제작처 구분
			itmPosX += 110;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, itmPosY, 80, 23);
			DGSetItemFont (dialogID, BUTTON_PRODUCT_SITE, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, BUTTON_PRODUCT_SITE, "제작처 구분");
			DGShowItem (dialogID, BUTTON_PRODUCT_SITE);

			// 버튼: 제작 번호
			itmPosX += 110;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, itmPosY, 80, 23);
			DGSetItemFont (dialogID, BUTTON_PRODUCT_NUM, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, BUTTON_PRODUCT_NUM, "제작 번호");
			DGShowItem (dialogID, BUTTON_PRODUCT_NUM);

			// 구분자
			itmPosX = 115;
			itmPosY = 60;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, itmPosX, itmPosY, 20, 1);
			DGShowItem (dialogID, SEPARATOR_1);

			itmPosX += 110;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, itmPosX, itmPosY, 20, 1);
			DGShowItem (dialogID, SEPARATOR_2);

			itmPosX += 110;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, itmPosX, itmPosY, 20, 1);
			DGShowItem (dialogID, SEPARATOR_3);

			itmPosX += 110;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, itmPosX, itmPosY, 20, 1);
			DGShowItem (dialogID, SEPARATOR_4);

			itmPosX += 110;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, itmPosX, itmPosY, 20, 1);
			DGShowItem (dialogID, SEPARATOR_5);

			itmPosX += 110;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, itmPosX, itmPosY, 20, 1);
			DGShowItem (dialogID, SEPARATOR_6);

			itmPosX += 110;
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, itmPosX, itmPosY, 20, 1);
			DGShowItem (dialogID, SEPARATOR_7);

			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 800, 5, 1, 110);
			DGShowItem (dialogID, SEPARATOR_8);

			// 체크박스 표시
			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 680, 25, 80, 20);
			DGSetItemFont (dialogID, CHECKBOX_PRODUCT_SITE_NUM, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, CHECKBOX_PRODUCT_SITE_NUM, "제작처/번호 포함");
			DGShowItem (dialogID, CHECKBOX_PRODUCT_SITE_NUM);
			DGSetItemValLong (dialogID, CHECKBOX_PRODUCT_SITE_NUM, TRUE);

			break;
		
		case DG_MSG_CHANGE:
			switch (item) {
				case CHECKBOX_PRODUCT_SITE_NUM:
					if (DGGetItemValLong (dialogID, CHECKBOX_PRODUCT_SITE_NUM) == TRUE) {
						DGEnableItem (dialogID, BUTTON_PRODUCT_SITE);
						DGEnableItem (dialogID, BUTTON_PRODUCT_NUM);
					} else {
						DGDisableItem (dialogID, BUTTON_PRODUCT_SITE);
						DGDisableItem (dialogID, BUTTON_PRODUCT_NUM);
					}

					break;
			}

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					if (DGGetItemValLong (dialogID, CHECKBOX_PRODUCT_SITE_NUM) == TRUE)
						extendedLayer = true;
					else
						extendedLayer = false;
					break;

				case DG_CANCEL:
					break;

				default:
					clickedBtnItemIdx = item;
					item = 0;	// 다른 버튼을 눌렀을 때 다이얼로그가 닫히지 않게 함

					dialogSizeX = 600;
					dialogSizeY = 500;

					// 메인 창 크기
					if (clickedBtnItemIdx == BUTTON_CODE) {
						dialogSizeX = 600;
						dialogSizeY = 90;
					} else if (clickedBtnItemIdx == BUTTON_DONG) {
						dialogSizeX = 600;
						dialogSizeY = 250;
					} else if (clickedBtnItemIdx == BUTTON_FLOOR) {
						dialogSizeX = 1000;
						dialogSizeY = 550;
					} else if (clickedBtnItemIdx == BUTTON_CAST) {
						dialogSizeX = 500;
						dialogSizeY = 350;
					} else if (clickedBtnItemIdx == BUTTON_CJ) {
						dialogSizeX = 500;
						dialogSizeY = 350;
					} else if (clickedBtnItemIdx == BUTTON_ORDER) {
						dialogSizeX = 500;
						dialogSizeY = 350;
					} else if (clickedBtnItemIdx == BUTTON_OBJ) {
						dialogSizeX = 1200;
						dialogSizeY = 800;
					} else if (clickedBtnItemIdx == BUTTON_PRODUCT_SITE) {
						dialogSizeX = 600;
						dialogSizeY = 90;
					} else if (clickedBtnItemIdx == BUTTON_PRODUCT_NUM) {
						dialogSizeX = 600;
						dialogSizeY = 250;
					}

					// [다이얼로그 박스] 레이어 쉽게 만들기 2차
					result = DGBlankModalDialog (dialogSizeX, dialogSizeY, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, layerMakeHandler_2, 0);

	//				// 버튼의 글꼴 설정 (공사 구분)
	//				anyTrue = 0;
	//				for (xx = 0 ; xx < layerInfo.code_name.size () ; ++xx)
	//					if (layerInfo.code_state [xx] == true)	anyTrue++;
	//				if (anyTrue > 0)
	//					DGSetItemFont (dialogID, BUTTON_CODE, DG_IS_LARGE | DG_IS_BOLD);
	//				else
	//					DGSetItemFont (dialogID, BUTTON_CODE, DG_IS_LARGE | DG_IS_PLAIN);

	//				// 버튼의 글꼴 설정 (동 구분)
	//				anyTrue = 0;
	//				for (xx = 0 ; xx < layerInfo.dong_name.size () ; ++xx)
	//					if (layerInfo.dong_state [xx] == true)	anyTrue++;
	//				if (anyTrue > 0)
	//					DGSetItemFont (dialogID, BUTTON_DONG, DG_IS_LARGE | DG_IS_BOLD);
	//				else
	//					DGSetItemFont (dialogID, BUTTON_DONG, DG_IS_LARGE | DG_IS_PLAIN);

	//				// 버튼의 글꼴 설정 (층 구분)
	//				anyTrue = 0;
	//				for (xx = 0 ; xx < layerInfo.floor_name.size () ; ++xx)
	//					if (layerInfo.floor_state [xx] == true)	anyTrue++;
	//				if (anyTrue > 0)
	//					DGSetItemFont (dialogID, BUTTON_FLOOR, DG_IS_LARGE | DG_IS_BOLD);
	//				else
	//					DGSetItemFont (dialogID, BUTTON_FLOOR, DG_IS_LARGE | DG_IS_PLAIN);

	//				// 버튼의 글꼴 설정 (CJ)
	//				anyTrue = 0;
	//				for (xx = 0 ; xx < layerInfo.CJ_name.size () ; ++xx)
	//					if (layerInfo.CJ_state [xx] == true)	anyTrue++;
	//				if (anyTrue > 0)
	//					DGSetItemFont (dialogID, BUTTON_CJ, DG_IS_LARGE | DG_IS_BOLD);
	//				else
	//					DGSetItemFont (dialogID, BUTTON_CJ, DG_IS_LARGE | DG_IS_PLAIN);

	//				// 버튼의 글꼴 설정 (시공순서)
	//				anyTrue = 0;
	//				for (xx = 0 ; xx < layerInfo.orderInCJ_name.size () ; ++xx)
	//					if (layerInfo.orderInCJ_state [xx] == true)	anyTrue++;
	//				if (anyTrue > 0)
	//					DGSetItemFont (dialogID, BUTTON_ORDER, DG_IS_LARGE | DG_IS_BOLD);
	//				else
	//					DGSetItemFont (dialogID, BUTTON_ORDER, DG_IS_LARGE | DG_IS_PLAIN);

	//				// 버튼의 글꼴 설정 (부재)
	//				anyTrue = 0;
	//				for (xx = 0 ; xx < layerInfo.obj_name.size () ; ++xx)
	//					if (layerInfo.obj_state [xx] == true)	anyTrue++;
	//				if (anyTrue > 0)
	//					DGSetItemFont (dialogID, BUTTON_OBJ, DG_IS_LARGE | DG_IS_BOLD);
	//				else
	//					DGSetItemFont (dialogID, BUTTON_OBJ, DG_IS_LARGE | DG_IS_PLAIN);

	//				// 버튼의 글꼴 설정 (객체)
	//				anyTrue = 0;
	//				for (xx = 0 ; xx < layerInfo.subObj_name.size () ; ++xx)
	//					if (layerInfo.subObj_state [xx] == true)	anyTrue++;
	//				if (anyTrue > 0)
	//					DGSetItemFont (dialogID, BUTTON_SUBOBJ, DG_IS_LARGE | DG_IS_BOLD);
	//				else
	//					DGSetItemFont (dialogID, BUTTON_SUBOBJ, DG_IS_LARGE | DG_IS_PLAIN);

					break;
			}
			break;

		case DG_MSG_CLOSE:
			switch (item) {
				case DG_CLOSEBOX:
					break;
			}
	}

	result = item;

	return	result;
}

// [다이얼로그 박스] 레이어 쉽게 만들기 2차
short DGCALLBACK layerMakeHandler_2 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	short	itmIdx;
	short	itmPosX, itmPosY;
	short	xx, yy;
	char	tempStr [30];
	char	dongStr [256];			// Edit 컨트롤로부터 입력받은 "," 또는 "-"로 구분된 동 문자열을 담은 변수
	char	productNumStr [256];	// Edit 컨트롤로부터 입력받은 "," 또는 "-"로 구분된 동 문자열을 담은 변수
	short	restIdx;		// 숫자가 아닌 문자로 된 동 문자열 버튼의 인덱스
	char	*token;			// 읽어온 문자열의 토큰
	short	count;

	switch (message) {
		case DG_MSG_INIT:

			// 다이얼로그 타이틀
			DGSetDialogTitle (dialogID, "세부 설정");

			// 확인 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 10, 10, 60, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, "확인");
			DGShowItem (dialogID, DG_OK);

			// 취소 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 10, 45, 60, 25);
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, "취소");
			DGShowItem (dialogID, DG_CANCEL);

			if (clickedBtnItemIdx == BUTTON_CODE) {
				itmPosX = 90;
				itmPosY = 10;
				for (xx = 0 ; xx < layerInfo.code_name.size () ; ++xx) {
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					sprintf (tempStr, "%s %s", layerInfo.code_name [xx].c_str (), layerInfo.code_desc [xx].c_str ());
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);
					(layerInfo.code_state [xx] == true) ? DGSetItemValLong (dialogID, itmIdx, TRUE) : DGSetItemValLong (dialogID, itmIdx, FALSE);

					itmPosX += 100;
					if (itmPosX >= 500) {
						itmPosX = 90;
						itmPosY += 30;
					}
				}
			} else if (clickedBtnItemIdx == BUTTON_DONG) {
				itmPosX = 90;
				itmPosY = 10;

				// 동 번호는 별도로 입력 받을 것
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, itmPosX, itmPosY, 300, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "동 번호 [101~1599동까지 가능] (예: 0101-0105,1102)");
				DGShowItem (dialogID, itmIdx);

				itmPosY += 30;

				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, itmPosX, itmPosY, 90, 20);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, "동 번호 입력");
				DGShowItem (dialogID, itmIdx);

				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_TEXT, 256, itmPosX + 90, itmPosY - 5, 210, 20);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGShowItem (dialogID, itmIdx);
				DONG_STRING_EDITCONTROL = itmIdx;

				itmPosY += 40;

				// 이미 등록된 동 번호 출력하기
				strcpy (dongStr, "");
				for (xx = 0 ; xx < layerInfo.dong_name.size () ; ++xx) {
					if (strlen (layerInfo.dong_name [xx].c_str ()) == 3)
						sprintf (tempStr, "0%s", layerInfo.dong_name [xx].c_str ());
					else if (strlen (layerInfo.dong_name [xx].c_str ()) == 1)
						sprintf (tempStr, "000%s", layerInfo.dong_name [xx].c_str ());
					else
						sprintf (tempStr, "%s", layerInfo.dong_name [xx].c_str ());

					if (isStringDouble (tempStr) == TRUE) {
						if (layerInfo.dong_state [xx] == true) {
							strcat (dongStr, tempStr);
							strcat (dongStr, ",");
						}
					}
				}
				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, itmPosX, itmPosY, 500, 23);
				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
				DGSetItemText (dialogID, itmIdx, dongStr);
				DGShowItem (dialogID, itmIdx);
				
				// 이미 등록된 동 번호를 Edit 컨트롤에도 넣어둘 것
				DGSetItemText (dialogID, DONG_STRING_EDITCONTROL, dongStr);

				itmPosY += 40;

				// 만약 동 번호이면 생략하고, 동 번호가 아니면 출력할 것
				restIdx = 0;
				for (xx = 0 ; xx < layerInfo.dong_name.size () ; ++xx) {
					if (strlen (layerInfo.dong_name [xx].c_str ()) == 3)
						sprintf (tempStr, "0%s", layerInfo.dong_name [xx].c_str ());
					else if (strlen (layerInfo.dong_name [xx].c_str ()) == 1)
						sprintf (tempStr, "000%s", layerInfo.dong_name [xx].c_str ());
					else
						sprintf (tempStr, layerInfo.dong_name [xx].c_str ());

					// 문자이면 출력, 숫자이면 스킵
					if (isStringDouble (tempStr) == FALSE) {
						itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 35);
						DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
						sprintf (tempStr, "%s\n%s", layerInfo.dong_name [xx].c_str (), layerInfo.dong_desc [xx].c_str ());
						DGSetItemText (dialogID, itmIdx, tempStr);
						DGShowItem (dialogID, itmIdx);
						DONG_REST_BUTTONS [restIdx] = itmIdx;
						restIdx ++;
						(layerInfo.dong_state [xx] == true) ? DGSetItemValLong (dialogID, itmIdx, TRUE) : DGSetItemValLong (dialogID, itmIdx, FALSE);

						itmPosX += 100;
						if (itmPosX >= 500) {
							itmPosX = 90;
							itmPosY += 40;
						}
					}
				}
			} else if (clickedBtnItemIdx == BUTTON_FLOOR) {
				itmPosX = 90;
				itmPosY = 10;
				for (xx = 0 ; xx < layerInfo.floor_name.size () ; ++xx) {
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 90, 35);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					sprintf (tempStr, "%s\n%s", layerInfo.floor_name [xx].c_str (), layerInfo.floor_desc [xx].c_str ());
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);
					(layerInfo.floor_state [xx] == true) ? DGSetItemValLong (dialogID, itmIdx, TRUE) : DGSetItemValLong (dialogID, itmIdx, FALSE);

					itmPosX += 100;
					if (itmPosX >= 950) {
						itmPosX = 90;
						itmPosY += 40;
					}
				}
			} else if (clickedBtnItemIdx == BUTTON_CAST) {
				itmPosX = 90;
				itmPosY = 10;
				for (xx = 0 ; xx < layerInfo.cast_name.size () ; ++xx) {
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 35, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					sprintf (tempStr, "%s", layerInfo.cast_name [xx].c_str ());
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);
					(layerInfo.cast_state [xx] == true) ? DGSetItemValLong (dialogID, itmIdx, TRUE) : DGSetItemValLong (dialogID, itmIdx, FALSE);

					itmPosX += 40;
					if (itmPosX >= 480) {
						itmPosX = 90;
						itmPosY += 30;
					}
				}
			} else if (clickedBtnItemIdx == BUTTON_CJ) {
				itmPosX = 90;
				itmPosY = 10;
				for (xx = 0 ; xx < layerInfo.CJ_name.size () ; ++xx) {
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 35, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					sprintf (tempStr, "%s", layerInfo.CJ_name [xx].c_str ());
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);
					(layerInfo.CJ_state [xx] == true) ? DGSetItemValLong (dialogID, itmIdx, TRUE) : DGSetItemValLong (dialogID, itmIdx, FALSE);

					itmPosX += 40;
					if (itmPosX >= 480) {
						itmPosX = 90;
						itmPosY += 30;
					}
				}
			} else if (clickedBtnItemIdx == BUTTON_ORDER) {
				itmPosX = 90;
				itmPosY = 10;
				for (xx = 0 ; xx < layerInfo.orderInCJ_name.size () ; ++xx) {
					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 35, 25);
					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
					sprintf (tempStr, "%s", layerInfo.orderInCJ_name [xx].c_str ());
					DGSetItemText (dialogID, itmIdx, tempStr);
					DGShowItem (dialogID, itmIdx);
					(layerInfo.orderInCJ_state [xx] == true) ? DGSetItemValLong (dialogID, itmIdx, TRUE) : DGSetItemValLong (dialogID, itmIdx, FALSE);

					itmPosX += 40;
					if (itmPosX >= 480) {
						itmPosX = 90;
						itmPosY += 30;
					}
				}
			} else if (clickedBtnItemIdx == BUTTON_OBJ) {
				itmPosX = 90;
				itmPosY = 10;
				for (xx = 0 ; xx < 100 ; ++xx) {
					OBJ_BUTTONS [xx] = 0;
				}
				count = 0;
				for (xx = 0 ; xx < layerInfo.code_name.size () ; ++xx) {
					for (yy = 0 ; yy < layerInfo.obj_name.size () ; ++yy) {
						if (yy == 0) {
							itmPosY += 5;

							itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, itmPosX, itmPosY, 100, 23);
							DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
							sprintf (tempStr, "%s %s", layerInfo.code_name [xx].c_str (), layerInfo.code_desc [xx].c_str ());
							DGSetItemText (dialogID, itmIdx, tempStr);
							DGShowItem (dialogID, itmIdx);

							itmPosY += 23;
						}

						if (strncmp (layerInfo.code_name [xx].c_str (), layerInfo.obj_cat [yy].c_str (), 4) == 0) {
							itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_PUSHTEXT, 0, itmPosX, itmPosY, 95, 28);
							DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
							sprintf (tempStr, "%s\n%s", layerInfo.obj_name [yy].c_str (), layerInfo.obj_desc [yy].c_str ());
							DGSetItemText (dialogID, itmIdx, tempStr);
							DGShowItem (dialogID, itmIdx);
							OBJ_BUTTONS [count] = itmIdx;
							(layerInfo.obj_state [yy] == true) ? DGSetItemValLong (dialogID, itmIdx, TRUE) : DGSetItemValLong (dialogID, itmIdx, FALSE);
							count ++;

							itmPosX += 100;
							if (itmPosX >= 1100) {
								itmPosX = 90;
								itmPosY += 30;
							}
						}
					}
					itmPosX = 90;
					itmPosY += 30;
				}
			}

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:

					if (clickedBtnItemIdx == BUTTON_CODE) {
	//					for (xx = 0 ; xx < layerInfo.code_name.size () ; ++xx) {
	//						if (DGGetItemValLong (dialogID, 3 + xx) == TRUE)
	//							layerInfo.code_state [xx] = true;
	//						else
	//							layerInfo.code_state [xx] = false;
	//					}
					} else if (clickedBtnItemIdx == BUTTON_DONG) {
	//					// 일단 false로 초기화
	//					for (xx = 0 ; xx < layerInfo.dong_name.size () ; ++xx) {
	//						layerInfo.dong_state [xx] = false;
	//					}

	//					// DONG_STRING_EDITCONTROL 문자열을 ","로 잘라서 해당 동 번호를 찾아서 true
	//					strcpy (dongStr, DGGetItemText (dialogID, DONG_STRING_EDITCONTROL).ToCStr ().Get ());
	//					token = strtok (dongStr, ",");
	//					while (token != NULL) {
	//						// 숫자가 맞다면,
	//						if (isStringDouble (token) == TRUE) {
	//							for (xx = 0 ; xx < layerInfo.dong_name.size () ; ++xx) {
	//								if (strncmp (layerInfo.dong_name [xx].c_str (), token, 4) == 0) {
	//									layerInfo.dong_state [xx] = true;
	//								}
	//							}
	//						}

	//						token = strtok (NULL, ",");
	//					}

	//					// 만약 동 번호이면 생략하고, 동 번호가 아니면 상태를 저장할 것
	//					restIdx = 0;
	//					for (xx = 0 ; xx < layerInfo.dong_name.size () ; ++xx) {
	//						if (strlen (layerInfo.dong_name [xx].c_str ()) != 4)
	//							sprintf (tempStr, "0%s", layerInfo.dong_name [xx].c_str ());
	//						else
	//							sprintf (tempStr, "%s", layerInfo.dong_name [xx].c_str ());

	//						// 문자이면 출력, 숫자이면 스킵
	//						if (isStringDouble (tempStr) == FALSE) {
	//							if (DGGetItemValLong (dialogID, DONG_REST_BUTTONS [restIdx]) == TRUE) {
	//								layerInfo.dong_state [xx] = true;
	//							} else {
	//								layerInfo.dong_state [xx] = false;
	//							}

	//							restIdx ++;
	//						}
	//					}
					} else if (clickedBtnItemIdx == BUTTON_FLOOR) {
	//					for (xx = 0 ; xx < layerInfo.floor_name.size () ; ++xx) {
	//						if (DGGetItemValLong (dialogID, 3 + xx) == TRUE)
	//							layerInfo.floor_state [xx] = true;
	//						else
	//							layerInfo.floor_state [xx] = false;
	//					}
					} else if (clickedBtnItemIdx == BUTTON_CAST) {
						// ...
					} else if (clickedBtnItemIdx == BUTTON_CJ) {
	//					for (xx = 0 ; xx < layerInfo.CJ_name.size () ; ++xx) {
	//						if (DGGetItemValLong (dialogID, 3 + xx) == TRUE)
	//							layerInfo.CJ_state [xx] = true;
	//						else
	//							layerInfo.CJ_state [xx] = false;
	//					}
					} else if (clickedBtnItemIdx == BUTTON_ORDER) {
	//					for (xx = 0 ; xx < layerInfo.orderInCJ_name.size () ; ++xx) {
	//						if (DGGetItemValLong (dialogID, 3 + xx) == TRUE)
	//							layerInfo.orderInCJ_state [xx] = true;
	//						else
	//							layerInfo.orderInCJ_state [xx] = false;
	//					}
					} else if (clickedBtnItemIdx == BUTTON_OBJ) {
	//					for (xx = 0 ; xx < layerInfo.obj_name.size () ; ++xx) {
	//						if (DGGetItemValLong (dialogID, OBJ_BUTTONS [xx]) == TRUE)
	//							layerInfo.obj_state [xx] = true;
	//						else
	//							layerInfo.obj_state [xx] = false;
	//					}
					} else if (clickedBtnItemIdx == BUTTON_PRODUCT_SITE) {
						// ...
					} else if (clickedBtnItemIdx == BUTTON_PRODUCT_NUM) {
						// ...
					}

					break;

				case DG_CANCEL:
					break;
			}
			break;
		case DG_MSG_CLOSE:
			switch (item) {
				case DG_CLOSEBOX:
					break;
			}
	}

	result = item;

	return	result;
}

// 레이어 쉽게 지정하기
GSErrCode	assignLayerEasily (void)
{
	GSErrCode	err = NoError;

	//FILE	*fp;			// 파일 포인터
	//char	line [10240];	// 파일에서 읽어온 라인 하나
	//string	insElem;		// 토큰을 string으로 변환해서 vector로 넣음
	//char	*token;			// 읽어온 문자열의 토큰
	//short	lineCount;		// 읽어온 라인 수
	//short	tokCount;		// 읽어온 토큰 개수

	//// Selection Manager 관련 변수
	//API_SelectionInfo		selectionInfo;
	//API_Element				tElem;
	//API_Neig				**selNeigs;
	//GS::Array<API_Guid>&	objects = GS::Array<API_Guid> ();
	//long					nObjects;
	//long					nSel;
	//API_Element				elem, mask;

	//API_Attribute	attrib;
	//API_AttributeDef  defs;
	//short			xx;

	//char	tempStr [30];

	//short	result;


	//// 선택한 객체가 있는지 확인함
	//err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, true);	// 선택한 요소 가져오기
	//BMKillHandle ((GSHandle *) &selectionInfo.marquee.coords);
	//if (err == APIERR_NOPLAN) {
	//	ACAPI_WriteReport ("열린 프로젝트 창이 없습니다.", true);
	//}
	//if (err == APIERR_NOSEL) {
	//	ACAPI_WriteReport ("객체를 아무 것도 선택하지 않았습니다.", true);
	//}
	//if (err != NoError) {
	//	BMKillHandle ((GSHandle *) &selNeigs);
	//	return err;
	//}

	//if (selectionInfo.typeID != API_SelEmpty) {
	//	nSel = BMGetHandleSize ((GSHandle) selNeigs) / sizeof (API_Neig);
	//	for (xx = 0 ; xx < nSel && err == NoError ; ++xx) {
	//		tElem.header.typeID = Neig_To_ElemID ((*selNeigs)[xx].neigID);

	//		tElem.header.guid = (*selNeigs)[xx].guid;
	//		ACAPI_Element_Get (&tElem);
	//		objects.Push (tElem.header.guid);
	//	}
	//}
	//BMKillHandle ((GSHandle *) &selNeigs);
	//nObjects = objects.GetSize ();

	//// 레이어 정보 파일 가져오기
	//fp = fopen ("C:\\layer.csv", "r");

	//for (lineCount = 1 ; lineCount <= 14 ; ++lineCount) {
	//	if (fp != NULL) {
	//		tokCount = 0;
	//		fscanf (fp, "%s\n", line);
	//		token = strtok (line, ",");
	//		tokCount ++;
	//		while (token != NULL) {
	//			if (strlen (token) > 0) {
	//				if (tokCount > 1) {
	//					insElem = token;

	//					if (lineCount == 1)		layerInfo.code_name.push_back (insElem);		// 공사구분
	//					if (lineCount == 2)		layerInfo.code_desc.push_back (insElem);		// 공사구분 설명
	//					if (lineCount == 3) {
	//						// 만약 동 번호 숫자가 3자리로 들어오면 앞에 0을 붙일 것
	//						if (strlen (token) == 3) {
	//							strcpy (tempStr, "0");
	//							strcat (tempStr, token);
	//							insElem = tempStr;
	//						}

	//						layerInfo.dong_name.push_back (insElem);							// 동
	//					}
	//					if (lineCount == 4)		layerInfo.dong_desc.push_back (insElem);		// 동 설명
	//					if (lineCount == 5)		layerInfo.floor_name.push_back (insElem);		// 층
	//					if (lineCount == 6)		layerInfo.floor_desc.push_back (insElem);		// 층 설명
	//					if (lineCount == 7) {
	//						// 만약 CJ 번호 숫자가 1자리로 들어오면 앞에 0을 붙일 것
	//						if (strlen (token) == 1) {
	//							strcpy (tempStr, "0");
	//							strcat (tempStr, token);
	//							insElem = tempStr;
	//						}

	//						layerInfo.CJ_name.push_back (insElem);			// CJ
	//					}
	//					if (lineCount == 8) {
	//						// 만약 CJ 속 시공순서 번호 숫자가 1자리로 들어오면 앞에 0을 붙일 것
	//						if (strlen (token) == 1) {
	//							strcpy (tempStr, "0");
	//							strcat (tempStr, token);
	//							insElem = tempStr;
	//						}

	//						layerInfo.orderInCJ_name.push_back (insElem);	// CJ 속 시공순서
	//					}
	//					if (lineCount == 9)		layerInfo.obj_name.push_back (insElem);			// 부재
	//					if (lineCount == 10)	layerInfo.obj_desc.push_back (insElem);			// 부재 설명
	//					if (lineCount == 11)	layerInfo.obj_cat.push_back (insElem);			// 부재가 속한 카테고리(공사구분)
	//					if (lineCount == 12)	layerInfo.subObj_name.push_back (insElem);		// 객체
	//					if (lineCount == 13)	layerInfo.subObj_desc.push_back (insElem);		// 객체 설명
	//					if (lineCount == 14)	layerInfo.subObj_cat.push_back (insElem);		// 객체가 속한 카테고리(공사구분)
	//				}
	//			}
	//			token = strtok (NULL, ",");
	//			tokCount ++;
	//		}
	//	} else {
	//		ACAPI_WriteReport ("layer.csv 파일을 C:\\로 복사하십시오.", true);
	//		return	err;
	//	}
	//}

	//fclose (fp);

	//// 구조체 초기화
	//allocateMemory (&layerInfo);

	//// [다이얼로그 박스] 레이어 쉽게 지정하기
	//result = DGBlankModalDialog (900, 120, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, layerAssignHandler, 0);

	//// OK 버튼이 아니면 메모리 해제하고 종료
	//if (result != DG_OK) {
	//	deallocateMemory (&layerInfo);
	//	return	err;
	//}

	//short	z;
	//char	code1 [10][5];		// 공사 코드
	//short	LenCode1;
	//char	code2 [1600][5];	// 동 코드
	//short	LenCode2;
	//char	code3 [120][5];		// 층 코드
	//short	LenCode3;
	//char	code4 [100][5];		// CJ 코드
	//short	LenCode4;
	//char	code5 [100][5];		// CJ 속 시공순서 코드
	//short	LenCode5;
	//char	code6 [90][5];		// 부재 코드
	//short	LenCode6;
	//char	code7 [90][5];		// 객체 코드
	//short	LenCode7;

	//char	fullLayerName [40];
	//bool	bNormalLayer;
	//short	x1, x2, x3, x4, x5, x6, x7;

	//// 1. 공사 코드 문자열 만들기
	//z = 0;
	//for (xx = 0 ; xx < layerInfo.code_name.size () ; ++xx) {
	//	if (layerInfo.code_state [xx] == true) {
	//		sprintf (tempStr, "%s", layerInfo.code_name [xx].c_str ());
	//		strcpy (code1 [z++], tempStr);
	//	}
	//}
	//LenCode1 = z;

	//// 2. 동 코드 문자열 만들기
	//z = 0;
	//strcpy (code2 [z++], "????");
	//for (xx = 0 ; xx < layerInfo.dong_name.size () ; ++xx) {
	//	if (layerInfo.dong_state [xx] == true) {
	//		sprintf (tempStr, "%s", layerInfo.dong_name [xx].c_str ());
	//		strcpy (code2 [z++], tempStr);
	//	}
	//}
	//LenCode2 = z;

	//// 3. 층 코드 문자열 만들기
	//z = 0;
	//for (xx = 0 ; xx < layerInfo.floor_name.size () ; ++xx) {
	//	if (layerInfo.floor_state [xx] == true) {
	//		sprintf (tempStr, "%s", layerInfo.floor_name [xx].c_str ());
	//		strcpy (code3 [z++], tempStr);
	//	}
	//}
	//LenCode3 = z;

	//// 4. CJ 코드 문자열 만들기
	//z = 0;
	//strcpy (code4 [z++], "??");
	//for (xx = 0 ; xx < layerInfo.CJ_name.size () ; ++xx) {
	//	if (layerInfo.CJ_state [xx] == true) {
	//		sprintf (tempStr, "%s", layerInfo.CJ_name [xx].c_str ());
	//		strcpy (code4 [z++], tempStr);
	//	}
	//}
	//LenCode4 = z;

	//// 5. CJ 속 시공순서 문자열 만들기
	//z = 0;
	//strcpy (code5 [z++], "??");
	//for (xx = 0 ; xx < layerInfo.orderInCJ_name.size () ; ++xx) {
	//	if (layerInfo.orderInCJ_state [xx] == true) {
	//		sprintf (tempStr, "%s", layerInfo.orderInCJ_name [xx].c_str ());
	//		strcpy (code4 [z++], tempStr);
	//	}
	//}
	//LenCode5 = z;

	//// 6. 부재 코드 문자열 만들기
	//z = 0;
	//for (xx = 0 ; xx < layerInfo.obj_name.size () ; ++xx) {
	//	if (layerInfo.obj_state [xx] == true) {
	//		sprintf (tempStr, "%s", layerInfo.obj_name [xx].c_str ());
	//		strcpy (code6 [z++], tempStr);
	//	}
	//}
	//LenCode6 = z;

	//// 7. 객체 코드 문자열 만들기 (가설재에 한함)
	//z = 0;
	//strcpy (code7 [z++], "????");
	//for (xx = 0 ; xx < layerInfo.subObj_name.size () ; ++xx) {
	//	if (layerInfo.subObj_state [xx] == true) {
	//		sprintf (tempStr, "%s", layerInfo.subObj_name [xx].c_str ());
	//		strcpy (code7 [z++], tempStr);
	//	}
	//}
	//LenCode7 = z;

	//// 레이어 이름 조합하기
	//for (x1 = 0 ; x1 < LenCode1 ; ++x1) {
	//	for (x2 = 0 ; x2 < LenCode2 ; ++x2) {
	//		for (x3 = 0 ; x3 < LenCode3 ; ++x3) {
	//			for (x4 = 0 ; x4 < LenCode4 ; ++x4) {
	//				for (x5 = 0 ; x5 < LenCode5 ; ++x5) {
	//					for (x6 = 0 ; x6 < LenCode6 ; ++x6) {
	//						for (x7 = 0 ; x7 < LenCode7 ; ++x7) {

	//							bNormalLayer = false;	// 레이어 이름이 정상적인 체계의 이름인가?

	//							// 공사 구분
	//							strcpy (fullLayerName, "");
	//							strcpy (fullLayerName, code1 [x1]);

	//							// 동 구분
	//							if ((LenCode2 > 1) && (onCode2 == true)) {
	//								strcat (fullLayerName, "-");
	//								strcat (fullLayerName, code2 [x2]);

	//								if (strncmp (code2 [x2], "????", 4) == 0) {
	//									bNormalLayer = false;
	//									continue;
	//								}
	//							}

	//							// 층 구분
	//							strcat (fullLayerName, "-");
	//							strcat (fullLayerName, code3 [x3]);

	//							// CJ 구간
	//							if (onCode4 == true) {
	//								if (LenCode4 > 1) {
	//									strcat (fullLayerName, "-");
	//									strcat (fullLayerName, code4 [x4]);

	//									if (strncmp (code4 [x4], "??", 2) == 0) {
	//										bNormalLayer = false;
	//										continue;
	//									}
	//								} else {
	//									strcat (fullLayerName, "-00");
	//								}
	//							}

	//							// CJ 속 시공순서
	//							if (onCode5 == true) {
	//								if (LenCode5 > 1) {
	//									strcat (fullLayerName, "-");
	//									strcat (fullLayerName, code5 [x5]);

	//									if (strncmp (code5 [x5], "??", 2) == 0) {
	//										bNormalLayer = false;
	//										continue;
	//									}
	//								} else {
	//									strcat (fullLayerName, "-00");
	//								}
	//							}

	//							// 부재
	//							if (onCode7 == true) {
	//								strcat (fullLayerName, "-");
	//								strcat (fullLayerName, code6 [x6]);
	//							} else {
	//								// 단, 객체 필드가 없으면 부재의 카테고리는 공사 구분 코드에 속한 것이어야 함
	//								for (xx = 0 ; xx < layerInfo.obj_name.size () ; ++xx) {
	//									if (strncmp (code6 [x6], layerInfo.obj_name [xx].c_str (), strlen (code6 [x6])) == 0) {
	//										if (strncmp (layerInfo.obj_cat [xx].c_str (), code1 [x1], 4) == 0)
	//											bNormalLayer = true;
	//										else
	//											bNormalLayer = false;
	//									}
	//								}

	//								strcat (fullLayerName, "-");
	//								strcat (fullLayerName, code6 [x6]);
	//							}

	//							// 객체
	//							if (onCode7 == true) {
	//								if (LenCode7 > 1) {
	//									// 객체의 카테고리는 공사 구분 코드에 속한 것이어야 함
	//									for (xx = 0 ; xx < layerInfo.subObj_name.size () ; ++xx) {
	//										if (strncmp (code7 [x7], layerInfo.subObj_name [xx].c_str (), strlen (code7 [x7])) == 0) {
	//											if (strncmp (layerInfo.subObj_cat [xx].c_str (), code1 [x1], 4) == 0)
	//												bNormalLayer = true;
	//											else
	//												bNormalLayer = false;
	//										}
	//									}

	//									strcat (fullLayerName, "-");
	//									strcat (fullLayerName, code7 [x7]);
	//								} else {
	//									// 없으면 생략
	//								}
	//							}

	//							// 정상적인 레이어 이름이면,
	//							if (bNormalLayer == true) {

	//								// 선택한 레이어가 존재하는지 확인
	//								BNZeroMemory (&attrib, sizeof (API_Attribute));
	//								attrib.header.typeID = API_LayerID;
	//								CHCopyC (fullLayerName, attrib.header.name);
	//								err = ACAPI_Attribute_Get (&attrib);

	//								// 레이어가 존재하면,
	//								if (err == NoError) {
	//									// 객체들의 레이어 속성을 변경함
	//									for (xx = 0 ; xx < nObjects ; ++xx) {
	//										BNZeroMemory (&elem, sizeof (API_Element));
	//										elem.header.guid = objects.Pop ();
	//										err = ACAPI_Element_Get (&elem);

	//										ACAPI_ELEMENT_MASK_CLEAR (mask);
	//										ACAPI_ELEMENT_MASK_SET (mask, API_Elem_Head, layer);
	//										elem.header.layer = attrib.layer.head.index;	// 가져온 레이어 속성의 인덱스를 부여함

	//										err = ACAPI_Element_Change (&elem, &mask, NULL, 0, true);
	//									}

	//								// 레이어가 존재하지 않으면,
	//								} else {

	//									result = DGAlert (DG_INFORMATION, "레이어가 존재하지 않음", "지정한 레이어가 존재하지 않습니다.\n새로 만드시겠습니까?", "", "예", "아니오", "");

	//									if (result == DG_OK) {
	//										// 레이어를 새로 생성함
	//										BNZeroMemory (&attrib, sizeof (API_Attribute));
	//										BNZeroMemory (&defs, sizeof (API_AttributeDef));

	//										attrib.header.typeID = API_LayerID;
	//										CHCopyC (fullLayerName, attrib.header.name);
	//										err = ACAPI_Attribute_Create (&attrib, &defs);

	//										ACAPI_DisposeAttrDefsHdls (&defs);

	//										// 객체들의 레이어 속성을 변경함
	//										for (xx = 0 ; xx < nObjects ; ++xx) {
	//											BNZeroMemory (&elem, sizeof (API_Element));
	//											elem.header.guid = objects.Pop ();
	//											err = ACAPI_Element_Get (&elem);

	//											ACAPI_ELEMENT_MASK_CLEAR (mask);
	//											ACAPI_ELEMENT_MASK_SET (mask, API_Elem_Head, layer);
	//											elem.header.layer = attrib.layer.head.index;	// 가져온 레이어 속성의 인덱스를 부여함

	//											err = ACAPI_Element_Change (&elem, &mask, NULL, 0, true);
	//										}
	//									} else {
	//										ACAPI_WriteReport ("레이어가 없으므로 실행을 중지합니다.", true);
	//										return	err;
	//									}
	//								}
	//							} else {
	//								// 정상적인 레이어가 아님
	//								ACAPI_WriteReport ("레이어 지정을 잘못하셨습니다.", true);
	//							}
	//						}
	//					}
	//				}
	//			}
	//		}
	//	}
	//}

	//// 메모리 해제
	//deallocateMemory (&layerInfo);

	//// 화면 새로고침
	//ACAPI_Automate (APIDo_RedrawID, NULL, NULL);
	//bool	regenerate = true;
	//ACAPI_Automate (APIDo_RebuildID, &regenerate, NULL);

	return	err;
}

// [다이얼로그 박스] 레이어 쉽게 지정하기
short DGCALLBACK layerAssignHandler (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	//short	itmIdx;
	//short	itmPosX, itmPosY;
	//short	dialogSizeX, dialogSizeY;
	//short	xx;
	//short	anyTrue;

	//switch (message) {
	//	case DG_MSG_INIT:
	//		// 다이얼로그 타이틀
	//		DGSetDialogTitle (dialogID, "레이어 쉽게 지정하기: LayerName Ex) 05-T-(0101)-9B1-(01)-(01)-COLU-(UFOM)");

	//		// 확인 버튼
	//		DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 820, 20, 60, 25);
	//		DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
	//		DGSetItemText (dialogID, DG_OK, "지정");
	//		DGShowItem (dialogID, DG_OK);

	//		// 취소 버튼
	//		DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 820, 55, 60, 25);
	//		DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
	//		DGSetItemText (dialogID, DG_CANCEL, "취소");
	//		DGShowItem (dialogID, DG_CANCEL);

	//		// 버튼: 공사 구분
	//		itmPosX = 30;
	//		itmPosY = 50;
	//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, itmPosY, 80, 23);
	//		DGSetItemFont (dialogID, BUTTON_CODE, DG_IS_LARGE | DG_IS_PLAIN);
	//		DGSetItemText (dialogID, BUTTON_CODE, "공사 구분");
	//		DGShowItem (dialogID, BUTTON_CODE);

	//		// 버튼: 동 구분
	//		itmPosX += 110;
	//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, itmPosY, 80, 23);
	//		DGSetItemFont (dialogID, BUTTON_DONG, DG_IS_LARGE | DG_IS_PLAIN);
	//		DGSetItemText (dialogID, BUTTON_DONG, "동 구분");
	//		DGShowItem (dialogID, BUTTON_DONG);

	//		// 버튼: 층 구분
	//		itmPosX += 110;
	//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, itmPosY, 80, 23);
	//		DGSetItemFont (dialogID, BUTTON_FLOOR, DG_IS_LARGE | DG_IS_PLAIN);
	//		DGSetItemText (dialogID, BUTTON_FLOOR, "층 구분");
	//		DGShowItem (dialogID, BUTTON_FLOOR);

	//		// 버튼: CJ
	//		itmPosX += 110;
	//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, itmPosY, 80, 23);
	//		DGSetItemFont (dialogID, BUTTON_CJ, DG_IS_LARGE | DG_IS_PLAIN);
	//		DGSetItemText (dialogID, BUTTON_CJ, "CJ");
	//		DGShowItem (dialogID, BUTTON_CJ);

	//		// 버튼: 시공순서
	//		itmPosX += 110;
	//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, itmPosY, 80, 23);
	//		DGSetItemFont (dialogID, BUTTON_ORDER, DG_IS_LARGE | DG_IS_PLAIN);
	//		DGSetItemText (dialogID, BUTTON_ORDER, "시공순서");
	//		DGShowItem (dialogID, BUTTON_ORDER);

	//		// 버튼: 부재
	//		itmPosX += 110;
	//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, itmPosY, 80, 23);
	//		DGSetItemFont (dialogID, BUTTON_OBJ, DG_IS_LARGE | DG_IS_PLAIN);
	//		DGSetItemText (dialogID, BUTTON_OBJ, "부재");
	//		DGShowItem (dialogID, BUTTON_OBJ);

	//		// 버튼: 객체
	//		itmPosX += 110;
	//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, itmPosX, itmPosY, 80, 23);
	//		DGSetItemFont (dialogID, BUTTON_SUBOBJ, DG_IS_LARGE | DG_IS_PLAIN);
	//		DGSetItemText (dialogID, BUTTON_SUBOBJ, "객체");
	//		DGShowItem (dialogID, BUTTON_SUBOBJ);

	//		// 구분자
	//		itmPosX = 115;
	//		itmPosY = 60;
	//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, itmPosX, itmPosY, 20, 1);
	//		DGShowItem (dialogID, SEPARATOR_1);

	//		itmPosX += 110;
	//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, itmPosX, itmPosY, 20, 1);
	//		DGShowItem (dialogID, SEPARATOR_2);

	//		itmPosX += 110;
	//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, itmPosX, itmPosY, 20, 1);
	//		DGShowItem (dialogID, SEPARATOR_3);

	//		itmPosX += 110;
	//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, itmPosX, itmPosY, 20, 1);
	//		DGShowItem (dialogID, SEPARATOR_4);

	//		itmPosX += 110;
	//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, itmPosX, itmPosY, 20, 1);
	//		DGShowItem (dialogID, SEPARATOR_5);

	//		itmPosX += 110;
	//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, itmPosX, itmPosY, 20, 1);
	//		DGShowItem (dialogID, SEPARATOR_6);

	//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_SEPARATOR, 0, 0, 800, 5, 1, 110);
	//		DGShowItem (dialogID, SEPARATOR_7);

	//		// 체크박스 표시
	//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 160, 25, 40, 20);
	//		DGSetItemFont (dialogID, CHECKBOX_DONG, DG_IS_LARGE | DG_IS_PLAIN);
	//		DGSetItemText (dialogID, CHECKBOX_DONG, "포함");
	//		DGShowItem (dialogID, CHECKBOX_DONG);
	//		DGSetItemValLong (dialogID, CHECKBOX_DONG, TRUE);

	//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 380, 25, 40, 20);
	//		DGSetItemFont (dialogID, CHECKBOX_CJ, DG_IS_LARGE | DG_IS_PLAIN);
	//		DGSetItemText (dialogID, CHECKBOX_CJ, "포함");
	//		DGShowItem (dialogID, CHECKBOX_CJ);
	//		DGSetItemValLong (dialogID, CHECKBOX_CJ, TRUE);

	//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 490, 25, 40, 20);
	//		DGSetItemFont (dialogID, CHECKBOX_ORDER, DG_IS_LARGE | DG_IS_PLAIN);
	//		DGSetItemText (dialogID, CHECKBOX_ORDER, "포함");
	//		DGShowItem (dialogID, CHECKBOX_ORDER);
	//		DGSetItemValLong (dialogID, CHECKBOX_ORDER, TRUE);

	//		itmIdx = DGAppendDialogItem (dialogID, DG_ITM_CHECKBOX, DG_BT_TEXT, 0, 710, 25, 40, 20);
	//		DGSetItemFont (dialogID, CHECKBOX_SUBOBJ, DG_IS_LARGE | DG_IS_PLAIN);
	//		DGSetItemText (dialogID, CHECKBOX_SUBOBJ, "포함");
	//		DGShowItem (dialogID, CHECKBOX_SUBOBJ);
	//		DGSetItemValLong (dialogID, CHECKBOX_SUBOBJ, TRUE);

	//		break;
	//	
	//	case DG_MSG_CHANGE:
	//		switch (item) {
	//			case CHECKBOX_DONG:
	//			case CHECKBOX_CJ:
	//			case CHECKBOX_ORDER:
	//			case CHECKBOX_SUBOBJ:

	//				if (DGGetItemValLong (dialogID, CHECKBOX_DONG) == TRUE) {
	//					DGEnableItem (dialogID, BUTTON_DONG);
	//				} else {
	//					DGDisableItem (dialogID, BUTTON_DONG);
	//				}

	//				if (DGGetItemValLong (dialogID, CHECKBOX_CJ) == TRUE) {
	//					DGEnableItem (dialogID, BUTTON_CJ);
	//					DGEnableItem (dialogID, CHECKBOX_ORDER);
	//				} else {
	//					DGDisableItem (dialogID, BUTTON_CJ);
	//					DGDisableItem (dialogID, CHECKBOX_ORDER);
	//				}

	//				if ((DGGetItemValLong (dialogID, CHECKBOX_CJ) == TRUE) && (DGGetItemValLong (dialogID, CHECKBOX_ORDER) == TRUE)) {
	//					DGEnableItem (dialogID, BUTTON_ORDER);
	//				} else {
	//					DGDisableItem (dialogID, BUTTON_ORDER);
	//				}

	//				if (DGGetItemValLong (dialogID, CHECKBOX_SUBOBJ) == TRUE) {
	//					DGEnableItem (dialogID, BUTTON_SUBOBJ);
	//				} else {
	//					DGDisableItem (dialogID, BUTTON_SUBOBJ);
	//				}

	//				break;
	//		}

	//		break;

	//	case DG_MSG_CLICK:
	//		switch (item) {
	//			case DG_OK:
	//				if (DGGetItemValLong (dialogID, CHECKBOX_DONG) == TRUE)
	//					onCode2 = true;
	//				else
	//					onCode2 = false;

	//				if (DGGetItemValLong (dialogID, CHECKBOX_CJ) == TRUE)
	//					onCode4 = true;
	//				else
	//					onCode4 = false;

	//				if ((DGGetItemValLong (dialogID, CHECKBOX_CJ) == TRUE) && (DGGetItemValLong (dialogID, CHECKBOX_ORDER) == TRUE))
	//					onCode5 = true;
	//				else
	//					onCode5 = false;

	//				if (DGGetItemValLong (dialogID, CHECKBOX_SUBOBJ) == TRUE)
	//					onCode7 = true;
	//				else
	//					onCode7 = false;

	//				break;

	//			case DG_CANCEL:
	//				break;

	//			default:
	//				clickedBtnItemIdx = item;
	//				item = 0;	// 다른 버튼을 눌렀을 때 다이얼로그가 닫히지 않게 함

	//				dialogSizeX = 600;
	//				dialogSizeY = 500;

	//				// 메인 창 크기
	//				if (clickedBtnItemIdx == BUTTON_CODE) {
	//					dialogSizeX = 600;
	//					dialogSizeY = 90;
	//				} else if (clickedBtnItemIdx == BUTTON_DONG) {
	//					dialogSizeX = 600;
	//					dialogSizeY = 250;
	//				} else if (clickedBtnItemIdx == BUTTON_FLOOR) {
	//					dialogSizeX = 1000;
	//					dialogSizeY = 550;
	//				} else if (clickedBtnItemIdx == BUTTON_CJ) {
	//					dialogSizeX = 500;
	//					dialogSizeY = 350;
	//				} else if (clickedBtnItemIdx == BUTTON_ORDER) {
	//					dialogSizeX = 500;
	//					dialogSizeY = 350;
	//				} else if (clickedBtnItemIdx == BUTTON_OBJ) {
	//					dialogSizeX = 1200;
	//					dialogSizeY = 800;
	//				} else if (clickedBtnItemIdx == BUTTON_SUBOBJ) {
	//					dialogSizeX = 1200;
	//					dialogSizeY = 800;
	//				}

	//				// [다이얼로그 박스] 레이어 쉽게 지정하기 2차
	//				result = DGBlankModalDialog (dialogSizeX, dialogSizeY, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, layerAssignHandler_2, 0);

	//				// 버튼의 글꼴 설정 (공사 구분)
	//				anyTrue = 0;
	//				for (xx = 0 ; xx < layerInfo.code_name.size () ; ++xx)
	//					if (layerInfo.code_state [xx] == true)	anyTrue++;
	//				if (anyTrue > 0)
	//					DGSetItemFont (dialogID, BUTTON_CODE, DG_IS_LARGE | DG_IS_BOLD);
	//				else
	//					DGSetItemFont (dialogID, BUTTON_CODE, DG_IS_LARGE | DG_IS_PLAIN);

	//				// 버튼의 글꼴 설정 (동 구분)
	//				anyTrue = 0;
	//				for (xx = 0 ; xx < layerInfo.dong_name.size () ; ++xx)
	//					if (layerInfo.dong_state [xx] == true)	anyTrue++;
	//				if (anyTrue > 0)
	//					DGSetItemFont (dialogID, BUTTON_DONG, DG_IS_LARGE | DG_IS_BOLD);
	//				else
	//					DGSetItemFont (dialogID, BUTTON_DONG, DG_IS_LARGE | DG_IS_PLAIN);

	//				// 버튼의 글꼴 설정 (층 구분)
	//				anyTrue = 0;
	//				for (xx = 0 ; xx < layerInfo.floor_name.size () ; ++xx)
	//					if (layerInfo.floor_state [xx] == true)	anyTrue++;
	//				if (anyTrue > 0)
	//					DGSetItemFont (dialogID, BUTTON_FLOOR, DG_IS_LARGE | DG_IS_BOLD);
	//				else
	//					DGSetItemFont (dialogID, BUTTON_FLOOR, DG_IS_LARGE | DG_IS_PLAIN);

	//				// 버튼의 글꼴 설정 (CJ)
	//				anyTrue = 0;
	//				for (xx = 0 ; xx < layerInfo.CJ_name.size () ; ++xx)
	//					if (layerInfo.CJ_state [xx] == true)	anyTrue++;
	//				if (anyTrue > 0)
	//					DGSetItemFont (dialogID, BUTTON_CJ, DG_IS_LARGE | DG_IS_BOLD);
	//				else
	//					DGSetItemFont (dialogID, BUTTON_CJ, DG_IS_LARGE | DG_IS_PLAIN);

	//				// 버튼의 글꼴 설정 (시공순서)
	//				anyTrue = 0;
	//				for (xx = 0 ; xx < layerInfo.orderInCJ_name.size () ; ++xx)
	//					if (layerInfo.orderInCJ_state [xx] == true)	anyTrue++;
	//				if (anyTrue > 0)
	//					DGSetItemFont (dialogID, BUTTON_ORDER, DG_IS_LARGE | DG_IS_BOLD);
	//				else
	//					DGSetItemFont (dialogID, BUTTON_ORDER, DG_IS_LARGE | DG_IS_PLAIN);

	//				// 버튼의 글꼴 설정 (부재)
	//				anyTrue = 0;
	//				for (xx = 0 ; xx < layerInfo.obj_name.size () ; ++xx)
	//					if (layerInfo.obj_state [xx] == true)	anyTrue++;
	//				if (anyTrue > 0)
	//					DGSetItemFont (dialogID, BUTTON_OBJ, DG_IS_LARGE | DG_IS_BOLD);
	//				else
	//					DGSetItemFont (dialogID, BUTTON_OBJ, DG_IS_LARGE | DG_IS_PLAIN);

	//				// 버튼의 글꼴 설정 (객체)
	//				anyTrue = 0;
	//				for (xx = 0 ; xx < layerInfo.subObj_name.size () ; ++xx)
	//					if (layerInfo.subObj_state [xx] == true)	anyTrue++;
	//				if (anyTrue > 0)
	//					DGSetItemFont (dialogID, BUTTON_SUBOBJ, DG_IS_LARGE | DG_IS_BOLD);
	//				else
	//					DGSetItemFont (dialogID, BUTTON_SUBOBJ, DG_IS_LARGE | DG_IS_PLAIN);

	//				break;
	//		}
	//		break;

	//	case DG_MSG_CLOSE:
	//		switch (item) {
	//			case DG_CLOSEBOX:
	//				break;
	//		}
	//}

	//result = item;

	return	result;
}

// [다이얼로그 박스] 레이어 쉽게 지정하기 2차
short DGCALLBACK layerAssignHandler_2 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	//short	itmIdx;
	//short	itmPosX, itmPosY;
	//short	xx, yy;
	//char	tempStr [30];
	//char	dongStr [256];	// Edit 컨트롤로부터 입력받은 ","로 구분된 동 문자열을 담은 변수
	//short	restIdx;		// 숫자가 아닌 문자로 된 동 문자열 버튼의 인덱스
	//char	*token;			// 읽어온 문자열의 토큰
	//bool	bFirstTok;		// 첫번째 토큰인가?
	//short	count;

	//switch (message) {
	//	case DG_MSG_INIT:

	//		// 다이얼로그 타이틀
	//		DGSetDialogTitle (dialogID, "세부 설정");

	//		// 확인 버튼
	//		DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 10, 10, 60, 25);
	//		DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
	//		DGSetItemText (dialogID, DG_OK, "확인");
	//		DGShowItem (dialogID, DG_OK);

	//		// 취소 버튼
	//		DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 10, 45, 60, 25);
	//		DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
	//		DGSetItemText (dialogID, DG_CANCEL, "취소");
	//		DGShowItem (dialogID, DG_CANCEL);

	//		if (clickedBtnItemIdx == BUTTON_CODE) {
	//			itmPosX = 90;
	//			itmPosY = 10;
	//			for (xx = 0 ; xx < layerInfo.code_name.size () ; ++xx) {
	//				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, BUTTON_CODE, itmPosX, itmPosY, 90, 25);
	//				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
	//				sprintf (tempStr, "%s %s", layerInfo.code_name [xx].c_str (), layerInfo.code_desc [xx].c_str ());
	//				DGSetItemText (dialogID, itmIdx, tempStr);
	//				DGShowItem (dialogID, itmIdx);
	//				(layerInfo.code_state [xx] == true) ? DGSetItemValLong (dialogID, itmIdx, TRUE) : DGSetItemValLong (dialogID, itmIdx, FALSE);

	//				itmPosX += 100;
	//				if (itmPosX >= 500) {
	//					itmPosX = 90;
	//					itmPosY += 30;
	//				}
	//			}
	//		} else if (clickedBtnItemIdx == BUTTON_DONG) {
	//			itmPosX = 90;
	//			itmPosY = 10;

	//			// 동 번호는 별도로 입력 받을 것
	//			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, itmPosX, itmPosY, 300, 23);
	//			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
	//			DGSetItemText (dialogID, itmIdx, "동 번호 [101~1599동까지 가능] (예: 0101)");
	//			DGShowItem (dialogID, itmIdx);

	//			itmPosY += 30;

	//			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_TEXT, BUTTON_DONG, itmPosX, itmPosY, 90, 20);
	//			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
	//			DGSetItemText (dialogID, itmIdx, "동 번호 입력");
	//			DGShowItem (dialogID, itmIdx);
	//			DONG_STRING_RADIOBUTTON = itmIdx;

	//			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_EDITTEXT, DG_ET_TEXT, 256, itmPosX + 90, itmPosY, 210, 20);
	//			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
	//			DGShowItem (dialogID, itmIdx);
	//			DONG_STRING_EDITCONTROL = itmIdx;

	//			itmPosY += 40;

	//			// 이미 등록된 동 번호 출력하기
	//			strcpy (dongStr, "");
	//			for (xx = 0 ; xx < layerInfo.dong_name.size () ; ++xx) {
	//				if (strlen (layerInfo.dong_name [xx].c_str ()) != 4)
	//					sprintf (tempStr, "0%s", layerInfo.dong_name [xx].c_str ());
	//				else
	//					sprintf (tempStr, "%s", layerInfo.dong_name [xx].c_str ());

	//				if (isStringDouble (tempStr) == TRUE) {
	//					if (layerInfo.dong_state [xx] == true) {
	//						strcat (dongStr, tempStr);
	//					}
	//				}
	//			}
	//			itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, itmPosX, itmPosY, 500, 23);
	//			DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
	//			DGSetItemText (dialogID, itmIdx, dongStr);
	//			DGShowItem (dialogID, itmIdx);
	//			
	//			// 이미 등록된 동 번호를 Edit 컨트롤에도 넣어둘 것
	//			DGSetItemText (dialogID, DONG_STRING_EDITCONTROL, dongStr);

	//			itmPosY += 40;

	//			// 만약 동 번호이면 생략하고, 동 번호가 아니면 출력할 것
	//			restIdx = 0;
	//			for (xx = 0 ; xx < layerInfo.dong_name.size () ; ++xx) {
	//				if (strlen (layerInfo.dong_name [xx].c_str ()) != 4)
	//					sprintf (tempStr, "0%s", layerInfo.dong_name [xx].c_str ());
	//				else
	//					sprintf (tempStr, layerInfo.dong_name [xx].c_str ());

	//				// 문자이면 출력, 숫자이면 스킵
	//				if (isStringDouble (tempStr) == FALSE) {
	//					itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, BUTTON_DONG, itmPosX, itmPosY, 90, 35);
	//					DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
	//					sprintf (tempStr, "%s\n%s", layerInfo.dong_name [xx].c_str (), layerInfo.dong_desc [xx].c_str ());
	//					DGSetItemText (dialogID, itmIdx, tempStr);
	//					DGShowItem (dialogID, itmIdx);
	//					DONG_REST_BUTTONS [restIdx] = itmIdx;
	//					restIdx ++;
	//					(layerInfo.dong_state [xx] == true) ? DGSetItemValLong (dialogID, itmIdx, TRUE) : DGSetItemValLong (dialogID, itmIdx, FALSE);

	//					itmPosX += 100;
	//					if (itmPosX >= 500) {
	//						itmPosX = 90;
	//						itmPosY += 40;
	//					}
	//				}
	//			}
	//		} else if (clickedBtnItemIdx == BUTTON_FLOOR) {
	//			itmPosX = 90;
	//			itmPosY = 10;
	//			for (xx = 0 ; xx < layerInfo.floor_name.size () ; ++xx) {
	//				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, BUTTON_FLOOR, itmPosX, itmPosY, 90, 35);
	//				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
	//				sprintf (tempStr, "%s\n%s", layerInfo.floor_name [xx].c_str (), layerInfo.floor_desc [xx].c_str ());
	//				DGSetItemText (dialogID, itmIdx, tempStr);
	//				DGShowItem (dialogID, itmIdx);
	//				(layerInfo.floor_state [xx] == true) ? DGSetItemValLong (dialogID, itmIdx, TRUE) : DGSetItemValLong (dialogID, itmIdx, FALSE);

	//				itmPosX += 100;
	//				if (itmPosX >= 950) {
	//					itmPosX = 90;
	//					itmPosY += 40;
	//				}
	//			}
	//		} else if (clickedBtnItemIdx == BUTTON_CJ) {
	//			itmPosX = 90;
	//			itmPosY = 10;
	//			for (xx = 0 ; xx < layerInfo.CJ_name.size () ; ++xx) {
	//				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, BUTTON_CJ, itmPosX, itmPosY, 35, 25);
	//				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
	//				sprintf (tempStr, "%s", layerInfo.CJ_name [xx].c_str ());
	//				DGSetItemText (dialogID, itmIdx, tempStr);
	//				DGShowItem (dialogID, itmIdx);
	//				(layerInfo.CJ_state [xx] == true) ? DGSetItemValLong (dialogID, itmIdx, TRUE) : DGSetItemValLong (dialogID, itmIdx, FALSE);

	//				itmPosX += 40;
	//				if (itmPosX >= 480) {
	//					itmPosX = 90;
	//					itmPosY += 30;
	//				}
	//			}
	//		} else if (clickedBtnItemIdx == BUTTON_ORDER) {
	//			itmPosX = 90;
	//			itmPosY = 10;
	//			for (xx = 0 ; xx < layerInfo.orderInCJ_name.size () ; ++xx) {
	//				itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, BUTTON_ORDER, itmPosX, itmPosY, 35, 25);
	//				DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
	//				sprintf (tempStr, "%s", layerInfo.orderInCJ_name [xx].c_str ());
	//				DGSetItemText (dialogID, itmIdx, tempStr);
	//				DGShowItem (dialogID, itmIdx);
	//				(layerInfo.orderInCJ_state [xx] == true) ? DGSetItemValLong (dialogID, itmIdx, TRUE) : DGSetItemValLong (dialogID, itmIdx, FALSE);

	//				itmPosX += 40;
	//				if (itmPosX >= 480) {
	//					itmPosX = 90;
	//					itmPosY += 30;
	//				}
	//			}
	//		} else if (clickedBtnItemIdx == BUTTON_OBJ) {
	//			itmPosX = 90;
	//			itmPosY = 10;
	//			for (xx = 0 ; xx < 100 ; ++xx) {
	//				OBJ_BUTTONS [xx] = 0;
	//			}
	//			count = 0;
	//			for (xx = 0 ; xx < layerInfo.code_name.size () ; ++xx) {
	//				for (yy = 0 ; yy < layerInfo.obj_name.size () ; ++yy) {
	//					if (yy == 0) {
	//						itmPosY += 5;

	//						itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, itmPosX, itmPosY, 100, 23);
	//						DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
	//						sprintf (tempStr, "%s %s", layerInfo.code_name [xx].c_str (), layerInfo.code_desc [xx].c_str ());
	//						DGSetItemText (dialogID, itmIdx, tempStr);
	//						DGShowItem (dialogID, itmIdx);

	//						itmPosY += 23;
	//					}

	//					if (strncmp (layerInfo.code_name [xx].c_str (), layerInfo.obj_cat [yy].c_str (), 4) == 0) {
	//						itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, BUTTON_OBJ, itmPosX, itmPosY, 95, 28);
	//						DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
	//						sprintf (tempStr, "%s\n%s", layerInfo.obj_name [yy].c_str (), layerInfo.obj_desc [yy].c_str ());
	//						DGSetItemText (dialogID, itmIdx, tempStr);
	//						DGShowItem (dialogID, itmIdx);
	//						OBJ_BUTTONS [count] = itmIdx;
	//						(layerInfo.obj_state [yy] == true) ? DGSetItemValLong (dialogID, itmIdx, TRUE) : DGSetItemValLong (dialogID, itmIdx, FALSE);
	//						count ++;

	//						itmPosX += 100;
	//						if (itmPosX >= 1100) {
	//							itmPosX = 90;
	//							itmPosY += 30;
	//						}
	//					}
	//				}
	//				itmPosX = 90;
	//				itmPosY += 30;
	//			}
	//		} else if (clickedBtnItemIdx == BUTTON_SUBOBJ) {
	//			itmPosX = 90;
	//			itmPosY = 10;
	//			for (xx = 0 ; xx < 100 ; ++xx) {
	//				SUBOBJ_BUTTONS [xx] = 0;
	//			}
	//			count = 0;
	//			for (xx = 0 ; xx < layerInfo.code_name.size () ; ++xx) {
	//				for (yy = 0 ; yy < layerInfo.subObj_name.size () ; ++yy) {
	//					if (yy == 0) {
	//						itmPosY += 5;

	//						itmIdx = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, itmPosX, itmPosY, 100, 23);
	//						DGSetItemFont (dialogID, itmIdx, DG_IS_LARGE | DG_IS_PLAIN);
	//						sprintf (tempStr, "%s %s", layerInfo.code_name [xx].c_str (), layerInfo.code_desc [xx].c_str ());
	//						DGSetItemText (dialogID, itmIdx, tempStr);
	//						DGShowItem (dialogID, itmIdx);

	//						itmPosY += 23;
	//					}

	//					if (strncmp (layerInfo.code_name [xx].c_str (), layerInfo.subObj_cat [yy].c_str (), 4) == 0) {
	//						itmIdx = DGAppendDialogItem (dialogID, DG_ITM_RADIOBUTTON, DG_BT_PUSHTEXT, BUTTON_SUBOBJ, itmPosX, itmPosY, 95, 28);
	//						DGSetItemFont (dialogID, itmIdx, DG_IS_SMALL | DG_IS_PLAIN);
	//						sprintf (tempStr, "%s\n%s", layerInfo.subObj_name [yy].c_str (), layerInfo.subObj_desc [yy].c_str ());
	//						DGSetItemText (dialogID, itmIdx, tempStr);
	//						DGShowItem (dialogID, itmIdx);
	//						SUBOBJ_BUTTONS [count] = itmIdx;
	//						(layerInfo.subObj_state [yy] == true) ? DGSetItemValLong (dialogID, itmIdx, TRUE) : DGSetItemValLong (dialogID, itmIdx, FALSE);
	//						count ++;

	//						itmPosX += 100;
	//						if (itmPosX >= 1100) {
	//							itmPosX = 90;
	//							itmPosY += 30;
	//						}
	//					}
	//				}
	//				itmPosX = 90;
	//				itmPosY += 30;
	//			}
	//		}

	//		break;

	//	case DG_MSG_CLICK:
	//		switch (item) {
	//			case DG_OK:

	//				if (clickedBtnItemIdx == BUTTON_CODE) {
	//					for (xx = 0 ; xx < layerInfo.code_name.size () ; ++xx) {
	//						if (DGGetItemValLong (dialogID, 3 + xx) == TRUE)
	//							layerInfo.code_state [xx] = true;
	//						else
	//							layerInfo.code_state [xx] = false;
	//					}
	//				} else if (clickedBtnItemIdx == BUTTON_DONG) {
	//					// 일단 false로 초기화
	//					for (xx = 0 ; xx < layerInfo.dong_name.size () ; ++xx) {
	//						layerInfo.dong_state [xx] = false;
	//					}

	//					// 동 번호 입력을 선택한 경우
	//					if (DGGetItemValLong (dialogID, DONG_STRING_RADIOBUTTON) == TRUE) {
	//						bFirstTok = true;

	//						// DONG_STRING_EDITCONTROL 문자열을 ","로 잘라서 해당 동 번호를 찾아서 true
	//						strcpy (dongStr, DGGetItemText (dialogID, DONG_STRING_EDITCONTROL).ToCStr ().Get ());
	//						token = strtok (dongStr, ",");
	//						while (token != NULL) {
	//							if (bFirstTok == true) {
	//								// 숫자가 맞다면,
	//								if (isStringDouble (token) == TRUE) {
	//									for (xx = 0 ; xx < layerInfo.dong_name.size () ; ++xx) {
	//										if (strncmp (layerInfo.dong_name [xx].c_str (), token, 4) == 0) {
	//											layerInfo.dong_state [xx] = true;
	//										}
	//									}
	//								}
	//							}

	//							token = strtok (NULL, ",");
	//							bFirstTok = false;
	//						}
	//					}

	//					// 만약 동 번호이면 생략하고, 동 번호가 아니면 상태를 저장할 것
	//					restIdx = 0;
	//					for (xx = 0 ; xx < layerInfo.dong_name.size () ; ++xx) {
	//						if (strlen (layerInfo.dong_name [xx].c_str ()) != 4)
	//							sprintf (tempStr, "0%s", layerInfo.dong_name [xx].c_str ());
	//						else
	//							sprintf (tempStr, "%s", layerInfo.dong_name [xx].c_str ());

	//						// 문자이면 출력, 숫자이면 스킵
	//						if (isStringDouble (tempStr) == FALSE) {
	//							if (DGGetItemValLong (dialogID, DONG_REST_BUTTONS [restIdx]) == TRUE) {
	//								layerInfo.dong_state [xx] = true;
	//							} else {
	//								layerInfo.dong_state [xx] = false;
	//							}

	//							restIdx ++;
	//						}
	//					}
	//				} else if (clickedBtnItemIdx == BUTTON_FLOOR) {
	//					for (xx = 0 ; xx < layerInfo.floor_name.size () ; ++xx) {
	//						if (DGGetItemValLong (dialogID, 3 + xx) == TRUE)
	//							layerInfo.floor_state [xx] = true;
	//						else
	//							layerInfo.floor_state [xx] = false;
	//					}
	//				} else if (clickedBtnItemIdx == BUTTON_CJ) {
	//					for (xx = 0 ; xx < layerInfo.CJ_name.size () ; ++xx) {
	//						if (DGGetItemValLong (dialogID, 3 + xx) == TRUE)
	//							layerInfo.CJ_state [xx] = true;
	//						else
	//							layerInfo.CJ_state [xx] = false;
	//					}
	//				} else if (clickedBtnItemIdx == BUTTON_ORDER) {
	//					for (xx = 0 ; xx < layerInfo.orderInCJ_name.size () ; ++xx) {
	//						if (DGGetItemValLong (dialogID, 3 + xx) == TRUE)
	//							layerInfo.orderInCJ_state [xx] = true;
	//						else
	//							layerInfo.orderInCJ_state [xx] = false;
	//					}
	//				} else if (clickedBtnItemIdx == BUTTON_OBJ) {
	//					for (xx = 0 ; xx < layerInfo.obj_name.size () ; ++xx) {
	//						if (DGGetItemValLong (dialogID, OBJ_BUTTONS [xx]) == TRUE)
	//							layerInfo.obj_state [xx] = true;
	//						else
	//							layerInfo.obj_state [xx] = false;
	//					}
	//				} else if (clickedBtnItemIdx == BUTTON_SUBOBJ) {
	//					for (xx = 0 ; xx < layerInfo.subObj_name.size () ; ++xx) {
	//						if (DGGetItemValLong (dialogID, SUBOBJ_BUTTONS [xx]) == TRUE)
	//							layerInfo.subObj_state [xx] = true;
	//						else
	//							layerInfo.subObj_state [xx] = false;
	//					}
	//				}

	//				break;

	//			case DG_CANCEL:
	//				break;
	//		}
	//		break;
	//	case DG_MSG_CLOSE:
	//		switch (item) {
	//			case DG_CLOSEBOX:
	//				break;
	//		}
	//}

	//result = item;

	return	result;
}
