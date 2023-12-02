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

--------------------------------

서버는 server_test4 폴더에 있는 파일 이용

클라이언트는 client_test4 폴더의 경우 GUI는 나오고 연결도 되지만 recv()가 되지 않아 정보를 받지 못함

SNSproClient 폴더의 경우 연결이 되지만(cmd창에서 SNSproClient.exe 127.0.0.1 9000 별명 <- 이렇게 입력해야 실행됨) GUI 없음

server_test4 파일 설명

DialogProc 함수:

주 창 대화 상자에 대한 메시지를 처리하며 버튼 클릭 및 창 초기화와 같은 작업을 수행.

WM_INITDIALOG: 대화 상자를 초기화하고 각종 컨트롤에 대한 핸들을 가져옴.

WM_COMMAND: 버튼 클릭을 처리.

WM_CLOSE: 대화 상자의 닫기 이벤트를 처리.

DisplayText 함수:

대화 상자 내의 채팅 기록을 제공된 텍스트로 업데이트.

EM_SETSEL 및 EM_REPLACESEL 메시지를 사용하여 대화 기록에 텍스트를 추가.

server_init 함수:

서버 소켓을 초기화하고 지정된 포트에서 수신 대기 상태로 설정.

소켓을 로컬 주소 및 포트에 바인딩.

서버 소켓을 반환.

add_client 함수:

새로운 클라이언트 연결을 수락.

새 이벤트를 생성하고 수락된 소켓에 연결.

클라이언트 정보를 sock_array에 저장.

새 연결에 대해 모든 클라이언트에게 알림.

read_client 함수:

특정 클라이언트의 메시지 수신 및 전달을 처리하기 위해 새 스레드(recv_and_forward)를 생성.

remove_client 함수:

클라이언트 연결을 종료하고 해당 클라이언트 정보를 제거.

클라이언트 연결 종료를 모든 클라이언트에게 알림.

notify_client 함수:

주어진 메시지를 모든 클라이언트에게 전송.

server_close 함수:

모든 클라이언트 소켓을 닫고 연관된 이벤트를 제거.

recv_and_forward 함수:

클라이언트로부터 메시지를 수신하고 모든 클라이언트에게 메시지를 전달.

chat_service 함수:

서버 소켓을 초기화하고 클라이언트 연결을 관리하는 메인 서비스 함수.

______________________________________________________

client_test4 설명

client_init 함수:

클라이언트 소켓 초기화를 위한 함수.

ip와 port를 매개변수로 받아서 해당 주소 및 포트에 소켓을 생성하고 서버에 연결.

chat_service 함수:

클라이언트가 서버에 연결되어 채팅 서비스를 수행하는 함수.

서버와의 데이터 통신 및 채팅 메시지의 송수신.

데이터를 주고받을 때 hWriteEvent와 hReadEvent 이벤트를 사용하여 동기화.

DialogProc 함수:

대화 상자 프로시저로, 사용자 인터페이스의 이벤트를 처리.

WM_INITDIALOG: 대화 상자를 초기화하고 컨트롤에 대한 핸들을 얻음.

WM_COMMAND: 버튼 클릭 등의 사용자 액션을 처리.

WM_CLOSE: 대화 상자가 닫힐 때의 동작을 정의.

DisplayText 함수:

대화 상자 내의 채팅 기록을 업데이트하는 함수.

EM_SETSEL 및 EM_REPLACESEL 메시지를 사용하여 대화 기록에 텍스트를 추가.

WinMain 함수:

Windows 애플리케이션의 진입점.

윈속 초기화를 수행하고, 이벤트를 생성하고, 클라이언트 쓰레드를 생성하며, 대화 상자를 생성.

_______________________________________________________________

SNSproClient 파일 설명

main 함수:

프로그램의 진입점.

명령줄 인수를 통해 서버 주소, 포트 번호, 닉네임을 입력받음.

client_init 함수를 호출하여 클라이언트 소켓을 초기화하고 서버에 연결.

do_chat_service 쓰레드를 생성하여 메시지 수신 및 전송을 담당.

client_init 함수:

클라이언트 소켓 초기화를 위한 함수.

서버 주소와 포트를 받아서 해당 주소와 포트에 소켓을 생성하고 서버에 연결.

do_chat_service 함수:

메시지 송수신을 담당하는 함수.

별도의 스레드에서 실행.

WSAEventSelect 함수를 사용하여 이벤트를 설정하고, WSAWaitForMultipleEvents 함수를 통해 이벤트를 대기.

서버로부터 메시지를 수신하면 화면에 출력하고, 서버가 종료되면 알림을 출력하고 클라이언트 소켓을 닫음.
