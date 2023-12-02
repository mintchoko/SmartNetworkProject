# SmartNetworkProject
server_test4와 client_test4는 서로 소통이 되지 않습니다. server_test4 ui 수정했습니다. ui를 담당하는 코드는 server_test4.rc와 resource.h입니다. 클라이언트 경우도 동일합니다.

client_test4와 GUITCPServer의 경우 소통이 됩니다. 확인용 입니다. 둘에 사용한 코드는 수업자료 코드입니다. 둘을 실행시키면 작동하는 것을 확인할 수 있습니다. 주소와 닉네임의 경우 작동하지 않습니다. 

ui는 c++ MFC를 사용했습니다.

client_test4 ui 변수이름의 경우

resource.h를 참고해 주세요.

IDD_CHAT_DIALOG <- ui 이름

IDC_IPADDRESS  <- 주소 에디트 컨트롤 

IDC_CONNECT_BUTTON  <- 연결 버튼

IDC_NICKNAME_EDIT <- 닉네임 에디트 컨트롤   

IDC_SETNICK_BUTTON <- 닉네임 버튼

IDC_CHAT_HISTORY_EDIT <- 메세지를 보여주는 에디트 컨트롤

IDC_MESSAGE_EDIT <- 메세지 쓰는 에디트 컨트롤

IDC_SEND_BUTTON  <- 보내는 버튼

IDC_EXIT_BUTTON <- 끝내기 버튼

서버의 경우 같은 기능이면 같은 이름을 사용합니다.

서버는 server_test4 폴더에 있는 파일 이용
클라이언트는 client_test4 폴더의 경우 GUI는 나오고 연결도 되지만 recv()가 되지 않아 정보를 받지 못함
SNSproClient 폴더의 경우 연결이 되지만(cmd창에서 SNSproClient.exe 127.0.0.1 9000 별명 <- 이렇게 입력해야 실행됨) GUI 없음

