#include "SessionWorker.h"


void SessionWorker::exec()
{
  while(running_) {
    for (auto &session : sessionList_) {
        std::visit([] (auto* s) {
            if (s->isSessionReady()) {
                s->sendMessages();
                s->readMessages();
                s->checkAndSendHeartBeat();
            }
        }, session);
    }
  }  
}

void SessionWorker::start()
{
    if (running_)
        return;

    running_ = true;
    workerThread_ = std::jthread(&SessionWorker::exec, this);
}

void SessionWorker::stop()
{
    running_ = false;
    
    if (workerThread_.joinable())
        workerThread_.join();
}

