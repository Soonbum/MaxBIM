# MaxBIM
* 개발자: 정순범 (canvas84@naver.com)

* 설명: 이것은 (주)스마트맥스가 소유한 아키캐드(이하 AC) 19용 애드온으로서 업무 보조용 도구입니다.

* 개발자 ID 획득하기

  - 다음 사이트에 가입(Sign Up)하면 메일을 받게 됩니다. (https://archicadapi.graphisoft.com/)
  
  - 메일을 Confirm하면 추후 개발자 등록 승인 메일을 받게 되는데 그 안에 개발자 ID가 있을 것입니다.

* 개발환경: Windows 10 64Bit, Visual Studio 2010, ArchiCAD API Development Kit 19

  - Visual Studio 2010을 설치하십시오. C++ 개발 환경 구축은 필수입니다.
  
  - GRAPHISOFT에서 제공하는 API Development Kit 19를 설치하십시오. Visual Studio 2010용 Template Wizard를 이용하여 개발 환경을 구축하십시오.
  
  - API Development Kit의 Example 소스를 통해 API 문서에 대한 실제 예제를 볼 수 있습니다.

* 설치 방법

  - 컴파일을 하면 x64 디렉토리에 MaxBIM.apx 파일이 생성되는데 이것을 C:\Program Files\GRAPHISOFT\ArchiCAD 19의 애드온 디렉토리에 복사하십시오.
  
  - Src\sqlite3 디렉토리의 sqlite3.dll 파일을 C:\Program Files\GRAPHISOFT\ArchiCAD 19 디렉토리에 복사하십시오.

  - Doc 디렉토리의 objectInfo.csv, layer.csv, exceptionLayer.csv 파일은 C:\ 디렉토리에 복사하십시오.
  
  - Doc 디렉토리의 *.gsm 파일은 아키캐드의 라이브러리 관리자에서 로드하시면 됩니다.

* 구동 방법

  - ArchiCAD 19를 구동하면 도움말 메뉴 좌측에 MaxBIM 메뉴가 나옵니다.

  - 만약 메뉴가 제대로 표시되지 않는다면 "옵션 > 애드온 관리자"에서 MaxBIM 항목을 끈 다음 확인하고 다시 켜서 확인해 보거나, "옵션 > 작업 환경 > 메뉴"를 통해 직접 메뉴를 구성하셔야 됩니다.

* 프로젝트 구성 디렉토리 (2022.03.01 업데이트)

  - MaxBIM : MaxBIM.sln

    - MaxBIM : MaxBIM..vcxproj, MaxBIM.vcxproj, MaxBIM.vcxproj.user

      - Doc : 애드온 실행에 필요한 외부 입력 파일(.csv)들이 있음

      - RFIX : MaxBIMFix.grc (애드온 및 메뉴 설명) 및 이미지 리소스 파일들이 있음

      - RFIX.WIN : MaxBIM.rc2

      - RINT : MaxBIM.grc (개발자/애드온 ID, 이미지/아이콘 리소스 ID, 다이얼로그 리소스 ID 등)

      - Src : 여러 가지 C/C++ header, source 파일들이 있음
      
* 참고 문서

  - 초보자를 위한 ArchiCAD 애드온 개발 가이드: https://docs.google.com/document/d/1QkGUc4PjSOUt_8CU0CLBy9zOgTXiSHmsM8XoMb80Smk/edit?usp=sharing
  
  - 아키캐드 애드온 개발 환경 구축하기 (64비트 환경): https://docs.google.com/document/d/1MN-BmUn7pGVzhBGUJiwHW4jsxZ1nxUNN/edit?usp=sharing&ouid=104508791730356351070&rtpof=true&sd=true

* 메뉴 구성 (2022.08.19 업데이트)

  | 대메뉴 | 소메뉴 |
  | -- | -- |
  | 테이블폼 배치  | 벽에 테이블폼 배치 (3D) |
  |               | 슬래브 하부에 테이블폼 배치 (3D) |
  |               | 보에 테이블폼 배치 (3D) |
  |               | 기둥에 테이블폼 배치 (3D) |
  |               | 낮은 슬래브 측면에 테이블폼 배치 (3D) |
  | 레이어 유틸 | 레이어 쉽게 선택하기 |
  |             | 레이어 쉽게 만들기 |
  |             | 레이어 쉽게 지정하기 |
  |             | 레이어 이름 검사하기 |
  | 내보내기 | 선택한 부재 정보 내보내기 (Single 모드) |
  |         | 선택한 부재 정보 내보내기 (Multi 모드) |
  |         | 부재별 선택 후 보여주기 |
  |         | 보 테이블폼 물량표 작성 |
  |         | 테이블폼 면적 계산 |
  |         | 콘크리트 물량 계산 (Single 모드) |
  |         | 콘크리트 물량 계산 (Multi 모드) |
  |         | 슬래브 수량/하부면적 계산 (Single 모드) |
  |         | 슬래브 수량/하부면적 계산 (Multi 모드) |
  |         | 단열재 수량/면적 계산 (Single 모드) |
  |         | 단열재 수량/면적 계산 (Multi 모드) |
  |         | 모든 입면도 PDF로 내보내기 (Single 모드) |
  |         | 모든 입면도 PDF로 내보내기 (Multi 모드) |
  |         | 모프 면적 계산 (Single 모드) |
  | 반자동 배치 | 물량합판 부착하기 |
  |             | 단열재 부착하기 |
  | 편의 기능 | 3D 품질/속도 조정하기 |
  |           | 영역에 3D 라벨 붙이기 |
  |           | 현재 평면도의 테이블폼에 버블 자동 배치 | 
  |           | 카메라 위치 저장하기/불러오기 | 
  | 정보 | 애드온 사용법 보기 |
  |       | MaxBIM 애드온 정보 |
