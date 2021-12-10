# MaxBIM
* 개발자: 정순범 (canvas84@naver.com)

* 설명: 이것은 (주)맥스BIM, (주)스마트맥스가 소유한 아키캐드(이하 AC) 19용 애드온으로서 업무 보조용 도구입니다.

* 개발환경: Windows 10 64Bit, Visual Studio 2010, ArchiCAD API Development Kit 19

* 설치 방법

  - 컴파일을 하면 x64 디렉토리에 MaxBIM.apx 파일이 생성되는데 이것을 C:\Program Files\GRAPHISOFT\ArchiCAD 19의 애드온 디렉토리에 복사하시면 됩니다.

  - Doc 디렉토리의 layer.csv 파일은 C:\ 디렉토리에 복사하시면 됩니다.
  
  - Doc 디렉토리의 *.gsm 파일은 AC의 라이브러리 관리자에서 로드하시면 됩니다.

* 구동 방법

  - ArchiCAD 19를 구동하면 도움말 메뉴 좌측에 MaxBIM 메뉴가 나옵니다.

  - 만약 메뉴가 제대로 표시되지 않는다면 옵션 > 작업 환경 > 메뉴를 통해 직접 메뉴를 구성하셔야 됩니다.

* 프로젝트 구성 디렉토리 (2021.10.08 업데이트)

  - MaxBIM : MaxBIM.sln

    - MaxBIM : MaxBIM..vcxproj, MaxBIM.vcxproj, MaxBIM.vcxproj.user

      - Doc : 애드온 실행에 필요한 외부 입력 파일(.csv)들이 있음

      - RFIX : MaxBIMFix.grc (애드온 및 메뉴 설명) 및 이미지 리소스 파일들이 있음

      - RFIX.WIN : MaxBIM.rc2

      - RINT : MaxBIM.grc (개발자/애드온 ID, 이미지/아이콘 리소스 ID, 다이얼로그 리소스 ID 등)

      - Src : 여러 가지 C/C++ header, source 파일들이 있음

* 메뉴 구성 (2021.11.24 업데이트)

  - 테이블폼 배치
  
    - 벽에 테이블폼 배치 (3D)

    - 슬래브 하부에 테이블폼 배치 (3D)

    - 보에 테이블폼 배치 (3D)

    - 기둥에 테이블폼 배치 (3D)

  - 동바리 배치

    - PERI 동바리 자동 배치 (3D)
  
  - 레이어 유틸
  
    - 레이어 쉽게 선택하기
    
    - 레이어 쉽게 만들기
    
    - 레이어 쉽게 지정하기

    - 레이어 이름 검사하기
  
  - 내보내기
  
    - 부재(기둥,보,슬래브) 정보 내보내기 (CSV)

    - 선택한 부재 정보 내보내기 (Single 모드)
  
    - 선택한 부재 정보 내보내기 (Multi 모드)

    - 부재별 선택 후 보여주기

  - 반자동 배치
  
    - 물량합판 부착하기

    - 단열재 부착하기

  - 정보
  
    - 애드온 사용법 보기
    
    - MaxBIM 애드온 정보
