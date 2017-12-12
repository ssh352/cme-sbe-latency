#pragma once

#include "Packet.h"
#include "Message.h"
#include "InstrumentController.h"

#include "sbe/MDIncrementalRefreshBook32.h"
#include "sbe/MDIncrementalRefreshDailyStatistics33.h"
#include "sbe/MDIncrementalRefreshLimitsBanding34.h"
#include "sbe/MDIncrementalRefreshSessionStatistics35.h"
#include "sbe/MDIncrementalRefreshTrade36.h"
#include "sbe/MDIncrementalRefreshTradeSummary42.h"
#include "sbe/MDIncrementalRefreshVolume37.h"
#include "sbe/QuoteRequest39.h"
#include "sbe/SecurityStatus30.h"
#include "sbe/SnapshotFullRefresh38.h"
#include <map>
#include <chrono>
#include <iostream>

class ChannelController {

public:
  ChannelController() = default;
  ChannelController(ChannelAccessor* channel);
  void OnIncrementalPacket(Packet*);
  void OnSnapshotPacket(Packet*);
  void AddOutOfSyncInstrument(uint32_t securityid);
  bool RemoveOutOfSyncInstrument(uint32_t securityid);
  bool HasOutOfSyncInstruments();
  void Subscribe(uint32_t securityid);
  void Unsubscribe(uint32_t securityid);

private:
  template<typename T>
  void HandleIncrementalMessage(Message& m) {
    auto message = m.Get<T>();
    auto transacttime = message.transactTime();
    auto& entry = message.noMDEntries();
    while(entry.hasNext()) {
      entry.next();
      auto inst_controller = GetInstrumentController(entry.securityID());
      if(inst_controller) {
        inst_controller->OnIncremental(entry, transacttime);
      }
    }
  }
  void OnIncrementalMessage(Message&);
  void HandleSnapshotMessage(Message& m);
//  template<typename T> void HandleIncrementalMessage(Message&);
  void HandleIncrementalSecurityStatus(Message& m);
  void HandleIncrementalQuoteRequest(Message& m);
  InstrumentController* GetInstrumentController(uint32_t securityid);
  std::map<uint32_t, InstrumentController> instrument_controllers_;
  std::vector<uint32_t> outofsync_instruments_;
  ChannelAccessor* channel_ = nullptr;
};



