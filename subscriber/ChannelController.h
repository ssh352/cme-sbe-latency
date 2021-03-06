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
#include "sbe/MDInstrumentDefinitionSpread29.h"
#include "sbe/MDInstrumentDefinitionOption41.h"
#include "sbe/MDInstrumentDefinitionFuture27.h"
#include <map>
#include <chrono>
#include <iostream>
#include "Instrument.h"

namespace sp {
namespace lltp {
namespace cme {

  class ChannelController {

  public:
    ChannelController() = default;

    ChannelController(ChannelContext *channel);

    void OnIncrementalPacket(Packet *);

    void OnSnapshotPacket(Packet *);

    void OnInstrumentPacket(Packet *);

    void AddOutOfSyncInstrument(uint32_t securityid);

    bool RemoveOutOfSyncInstrument(uint32_t securityid);

    bool HasOutOfSyncInstruments();

    void Subscribe(Instrument inst);

    void Unsubscribe(uint32_t securityid);

  private:

    void Commit();

    template<typename T>
    void HandleIncrementalMessage(Message &m) {
      auto message = m.Get<T>();
      auto transacttime = message.transactTime();
      auto &entry = message.noMDEntries();
      while (entry.hasNext()) {
        entry.next();
        auto inst_controller = GetInstrumentController(entry.securityID());
        if (inst_controller) {
          inst_controller->OnIncremental(entry, transacttime);
        }
      }
      if (message.matchEventIndicator().endOfEvent()) {
        Commit();
      }
    }

    long expectedseqnum_ = 0;

    void OnIncrementalMessage(Message &);

    void HandleSnapshotMessage(Message &m);

    void HandleIncrementalSecurityStatus(Message &m);

    void HandleIncrementalQuoteRequest(Message &m);

    InstrumentController *GetInstrumentController(uint32_t securityid);

    std::map<uint32_t, InstrumentController> instrument_controllers_;
    std::vector<std::uint32_t> outofsync_instruments_;
    ChannelContext *channel_ = nullptr;
  };


}}}
