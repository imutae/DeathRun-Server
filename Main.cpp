#include <iostream>
#include "ServerEngine.h"
#include "DeathRunServerLogic.h"

using namespace SE;

int main()
{
    // 서버 로직 생성
    DeathRunServerLogic logic;
    // 엔진 생성
    ServerEngine engine;

    // 초기화
    if (!engine.Initialize(&logic, "127.0.0.1", 7777))
    {
        std::cout << "Server Initialize Failed\n";
        return 0;
    }

    std::cout << "Server Start\n";

    // 실행
    engine.Run();

    // 종료 입력 대기
    std::cout << "Press Enter to Shutdown...\n";
    std::cin.get();

    // 종료
    engine.Shutdown();

    std::cout << "Server Shutdown\n";

    return 0;
}
