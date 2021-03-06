#pragma once

#include "InstrumentMdHandler.h"
#include "ChannelContext.h"
#include "Instrument.h"

/*
 * Manages seqnum and rtpseq values for a specific instrument.
 * (Un)Subscribes to instrument feeds when recovery/gap detected.
 *
 * Valid messages are passed onto InstrumentMDHandler
 */


namespace sp {
namespace lltp {
namespace cme {

  enum InstrumentState {NEW, INITIAL, SYNC, OUTOFSYNC, DISCONTINUED};

  class InstrumentController {

  public:
    InstrumentController(Instrument instrument, ChannelContext *channel);

    void OnSnapshot(SnapshotFullRefresh38 &, std::uint64_t transacttime);

    template<typename T>
    void OnIncremental(T &entry, uint64_t transacttime) {
      auto current_state = state_;
      auto rptseq = entry.rptSeq();
      auto expected_rptseq = processed_rptseq_ + 1;
      if (current_state == InstrumentState::SYNC) {
        if (rptseq == expected_rptseq) {
          processed_rptseq_ = rptseq;
          mdhandler_.OnIncremental(entry, transacttime);
        } else if (rptseq > expected_rptseq) {
          mdhandler_.Reset();
          switchState(InstrumentState::SYNC, InstrumentState::OUTOFSYNC);
          std::cout << "State : SYNC -> OUTOFSYNC" << '\n';
        }
      } else if (current_state == InstrumentState::OUTOFSYNC) {
        if (rptseq == expected_rptseq) {
          std::cout << "Incremental feed synchronised" << '\n';
          processed_rptseq_ = rptseq;
          switchState(InstrumentState::OUTOFSYNC, InstrumentState::SYNC);
          std::cout << "State : OUTOFSYNC -> SYNC" << '\n';
          mdhandler_.OnIncremental(entry, transacttime);
        }
      } else if (current_state == INITIAL) {
        if (processed_rptseq_ == 0 && rptseq == 1) { // start of session, no recovery required
          processed_rptseq_ = rptseq;
          std::cout << "State : INTIAL -> SYNC" << '\n';
          switchState(InstrumentState::INITIAL, InstrumentState::SYNC);
          mdhandler_.OnIncremental(entry, transacttime);
        }
      }
    }

    void OnSecurityStatus(
        SecurityStatus30 &); // TODO: need to support logic from ChannelController if securityID blank send to all
    void OnChannelReset();

    void Commit();

  private:

    Instrument instrument_;
    InstrumentState state_;
    long processed_rptseq_;
    std::string securitydesc_;

    InstrumentMdHandler mdhandler_;
    ChannelContext *channel_;

    void switchState(InstrumentState, InstrumentState);
  };

}}} //
