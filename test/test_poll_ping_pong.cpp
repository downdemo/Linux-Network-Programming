#include <signal.h>
#include <thread>
#include "net/poller_test.h"

namespace jc {

template <uint64_t timeout_millsec>
class Tester {
 public:
  void run() {
    const uint16_t port_ = get_free_port();
    std::jthread t{[&] {
      PollerTCPServer<PollerPoll>{"localhost", port_, timeout_millsec};
    }};
    PollerTCPClient<PollerPoll>{"localhost", port_, timeout_millsec};
  }
};

}  // namespace jc

int main() {
  ::signal(SIGCHLD, SIG_IGN);
  ::signal(SIGPIPE, SIG_IGN);
  jc::Tester<3000>{}.run();
}
