# DeathRun-Server

> `ServerEngine.IOCP` 기반 C++ 멀티플레이 게임 서버 프로토타입

## 프로젝트 소개

`DeathRun-Server`는 직접 구현한 `ServerEngine.IOCP` 엔진 위에서 동작하는 C++ 게임 서버 프로토타입입니다.

처음에는 2D 플랫포머 데스런 게임 서버를 목표로 시작했지만, 현재는 포트폴리오와 학습 목적에 맞춰 게임 서버의 핵심 흐름인 세션 관리, 패킷 처리, 로비 채팅, 룸 생성/참가, 룸 단위 브로드캐스트, 위치 동기화, 이탈 처리를 중심으로 구현하고 있습니다.

이 프로젝트는 완성형 게임 서버라기보다, 게임 서버 개발자로서 필요한 네트워크 엔진 사용, 프로토콜 설계, 세션 상태 관리, 룸 상태 관리 역량을 보여주기 위한 서버 프로토타입입니다.

## 개발 목표

- `ServerEngine.IOCP` 기반 서버 로직 구현
- 클라이언트 접속 및 세션 식별 처리
- 패킷 ID 기반 요청 분기
- 로비 채팅 처리
- 룸 생성, 참가, 퇴장 처리
- 룸 단위 플레이어 상태 관리
- 위치 동기화 패킷 브로드캐스트
- 서버 코드의 가독성, 유지보수성, 안정성 개선

## 현재 구현 내용

| 기능 | 상태 | 설명 |
|---|---|---|
| 접속 승인 | 구현 | 클라이언트 연결 시 `E_ACCEPT`로 서버 기준 `sessionId` 전달 |
| 로비 채팅 | 구현 | 룸에 들어가지 않은 세션 대상으로 `S_CHAT` 브로드캐스트 |
| 방 생성 / 참가 | 구현 | `R_JOIN` 요청으로 새 방 생성 또는 기존 방 참가 |
| 방 목록 조회 | 구현 | `R_ROOM_LIST` 요청 시 `S_ROOM_LIST`로 현재 방 목록 응답 |
| 위치 동기화 | 구현 | `R_MOVE` 수신 시 같은 방 세션들에게 `S_MOVE` 전파 |
| 방 이탈 처리 | 구현 | `N_LEAVE` 또는 disconnect 시 룸에서 세션 제거 |
| 룸 생명주기 관리 | 구현 | 빈 방은 `RoomManager`에서 제거 |
| 패킷 길이 검증 | 구현 | 고정 크기 패킷에 대해 `len == sizeof(Packet)` 기준으로 검증 |
| 로그 정책 | 보강 필요 | 현재 `std::cout` 기반 출력, 추후 로그 helper 또는 logger 분리 권장 |

## 서버 실행 흐름

1. `Main.cpp`에서 `DeathRunServerLogic` 객체를 생성합니다.
2. `ServerEngine` 객체를 생성하고 초기화합니다.
3. 서버가 `127.0.0.1:7777`에서 실행됩니다.
4. 클라이언트가 접속하면 `OnConnected()`가 호출됩니다.
5. 서버는 `E_ACCEPT` 패킷으로 클라이언트에게 `sessionId`를 전달합니다.
6. 이후 엔진은 수신한 패킷을 `DispatchPacket()`으로 전달합니다.
7. `DeathRunServerLogic`은 패킷 ID에 따라 채팅, 룸, 이동, 이탈 로직을 처리합니다.
8. 클라이언트 연결이 종료되면 `OnDisconnected()`에서 세션과 룸 상태를 정리합니다.

## 프로토콜 요약

| PacketId | 방향 | 설명 |
|---|---|---|
| `E_ACCEPT` | 서버 → 클라이언트 | 접속 직후 서버 기준 `sessionId` 전달 |
| `R_CHAT` | 클라이언트 → 서버 | 로비 채팅 전송 |
| `S_CHAT` | 서버 → 클라이언트 | 로비 채팅 브로드캐스트 |
| `R_ROOM_LIST` | 클라이언트 → 서버 | 현재 방 목록 요청 |
| `S_ROOM_LIST` | 서버 → 클라이언트 | 현재 방 목록 응답 |
| `R_JOIN` | 클라이언트 → 서버 | 방 생성 또는 기존 방 참가 요청 |
| `S_JOIN` | 서버 → 클라이언트 | 방 참가 성공 여부와 현재 플레이어 목록 응답 |
| `E_JOIN` | 서버 → 클라이언트 | 같은 방에 다른 플레이어가 입장했음을 알림 |
| `R_MOVE` | 클라이언트 → 서버 | 자신의 위치 전송 |
| `S_MOVE` | 서버 → 클라이언트 | 같은 방 플레이어들에게 위치 전파 |
| `N_LEAVE` | 클라이언트 → 서버 | 방 이탈 요청 |
| `E_LEAVE` | 서버 → 클라이언트 | 같은 방의 플레이어 퇴장 알림 |

## 주요 클래스

### `DeathRunServerLogic`

`ServerEngine.IOCP`에서 제공하는 `IServerLogic`을 구현한 게임 서버 로직 클래스입니다.

주요 역할은 다음과 같습니다.

- 클라이언트 접속 처리
- 클라이언트 연결 종료 처리
- 패킷 ID 기반 요청 분기
- 로비 채팅 브로드캐스트
- 방 생성 및 참가 처리
- 방 목록 응답
- 위치 동기화 처리
- 방 이탈 및 세션 정리

### `Room`

하나의 게임 방을 표현하는 클래스입니다.

주요 역할은 다음과 같습니다.

- 방에 속한 세션 관리
- 플레이어 입장 처리
- 플레이어 퇴장 처리
- 룸 단위 패킷 브로드캐스트
- 특정 세션을 제외한 브로드캐스트
- 현재 플레이어 목록 조회

### `RoomManager`

전체 방 목록을 관리하는 클래스입니다.

주요 역할은 다음과 같습니다.

- 방 생성
- 방 제거
- 방 조회
- 전체 방 목록 조회
- room id 생성 및 관리

## 디렉터리 구성

```text
DeathRun-Server/
├─ Main.cpp
├─ DeathRunServerLogic.h
├─ DeathRunServerLogic.cpp
├─ Room.h
├─ Room.cpp
├─ RoomManager.h
├─ RoomManager.cpp
├─ PacketId.h
├─ PacketStruct.h
├─ DeathRunServer.vcxproj
└─ README.md
```

## 빌드 및 실행 환경

- OS: Windows
- IDE: Visual Studio 2022
- Language: C++20
- Network: Windows IOCP
- Engine: `ServerEngine.IOCP`

이 프로젝트는 `ServerEngine.IOCP`를 sibling repository로 참조합니다.

권장 디렉터리 구조는 다음과 같습니다.

```text
GitHub/
├─ DeathRun-Server/
└─ ServerEngine.IOCP/
```

`DeathRunServer.vcxproj`는 다음 엔진 프로젝트를 참조합니다.

```text
..\ServerEngine.IOCP\ServerEngine.vcxproj
```

## 실행 방법

1. `DeathRun-Server`와 `ServerEngine.IOCP`를 같은 상위 디렉터리에 clone합니다.

```text
GitHub/
├─ DeathRun-Server/
└─ ServerEngine.IOCP/
```

2. Visual Studio에서 `DeathRunServer.vcxproj`를 엽니다.

3. 빌드 구성을 선택합니다.

```text
Debug | x64
```

4. 프로젝트를 빌드하고 실행합니다.

5. 서버가 정상적으로 실행되면 다음 주소에서 클라이언트 접속을 기다립니다.

```text
127.0.0.1:7777
```

## 패킷 처리 방식

`ServerEngine.IOCP`는 네트워크 수신 데이터를 패킷 단위로 분리한 뒤, 게임 서버 로직의 `DispatchPacket()`으로 전달합니다.

`DeathRunServerLogic`은 전달받은 `packetId`를 기준으로 요청을 분기합니다.

```cpp
void DeathRunServerLogic::DispatchPacket(
    SE::Net::Session* session,
    std::uint16_t packetId,
    const char* data,
    std::int32_t len
);
```

현재 서버 로직은 다음 handler로 역할을 분리합니다.

```text
HandleChat()
HandleJoin()
HandleMove()
HandleRoomList()
```

이를 통해 패킷 분기와 실제 처리 로직을 분리하고, 각 기능의 책임을 명확히 유지합니다.

## 룸 처리 흐름

### 방 생성 또는 참가

클라이언트는 `R_JOIN` 패킷을 서버로 보냅니다.

- `roomId`가 유효하지 않으면 새 방을 생성합니다.
- `roomId`가 유효하면 해당 방 참가를 시도합니다.
- 참가 성공 시 `S_JOIN` 패킷으로 결과를 응답합니다.
- 기존 방에 다른 플레이어가 참가한 경우, 같은 방의 기존 플레이어들에게 `E_JOIN`을 전송합니다.

### 방 목록 조회

클라이언트는 `R_ROOM_LIST` 패킷을 서버로 보냅니다.

서버는 현재 생성되어 있는 방 목록을 `S_ROOM_LIST`로 응답합니다.

### 위치 동기화

클라이언트는 `R_MOVE` 패킷으로 자신의 위치를 서버에 보냅니다.

서버는 해당 세션이 속한 방을 찾고, 같은 방의 플레이어들에게 `S_MOVE` 패킷을 전파합니다.

### 방 이탈

클라이언트는 `N_LEAVE` 패킷으로 방 이탈을 요청합니다.

또는 클라이언트 연결이 끊긴 경우에도 서버는 동일하게 방 이탈 처리를 수행합니다.

방에서 플레이어가 나가면 같은 방의 다른 플레이어들에게 `E_LEAVE` 패킷을 전송합니다.

플레이어가 모두 나가 비어 있는 방은 `RoomManager`에서 제거합니다.

## 현재 보강 예정 사항

- malformed packet 로그 추가
- `Session::Send()` 실패 처리 정책 정리
- 로그 helper 또는 logger 분리
- 서버 IP/port 설정 분리
- Visual Studio 프로젝트 설정 정리
  - 로컬 절대 include path 제거
  - `Debug` / `Release` 설정 일관화
- 방 생성, 참가, 이탈, 이동 동기화 테스트 시나리오 정리
- 클라이언트와 서버 프로토콜 문서 동기화

## 프로젝트 방향

이 프로젝트의 현재 주 목적은 기능을 계속 추가하는 것이 아니라, 이미 구현된 서버 로직을 더 안정적이고 읽기 쉬운 구조로 개선하는 것입니다.

중점적으로 보는 기준은 다음과 같습니다.

- 기존 동작 유지
- 코드 가독성 향상
- 책임 분리
- 중복 제거
- 예외 상황 방어
- 동시성 안정성 개선
- 포트폴리오 코드로서의 설명 가능성 강화