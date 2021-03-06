#include "InstrumentController.h"

#include <iostream>

namespace sp {
namespace lltp {
namespace cme {

  void InstrumentController::switchState(InstrumentState current_state, InstrumentState new_state) {
    state_ = new_state;
    if (new_state == InstrumentState::OUTOFSYNC) {
      channel_->SubscribeToSnapshotsForInstrument(instrument_.securityid);
    } else if (new_state == InstrumentState::SYNC || new_state == InstrumentState::DISCONTINUED) {
      channel_->UnsubscribeToSnapshotsForInstrument(instrument_.securityid);
    }
  }

  void InstrumentController::OnSnapshot(SnapshotFullRefresh38 &refresh, std::uint64_t transacttime) {
    auto current_state = state_;
    auto snpt_seqnum = refresh.rptSeq();
    if (current_state == INITIAL) {
      processed_rptseq_ = snpt_seqnum;
      std::cout << "State : INITIAL -> SYNC" << '\n';
      switchState(InstrumentState::INITIAL, InstrumentState::SYNC);
      mdhandler_.OnSnapshot(refresh, transacttime);
    } else if (current_state == InstrumentState::OUTOFSYNC) {
      if (snpt_seqnum > processed_rptseq_) {
        processed_rptseq_ = snpt_seqnum;
        mdhandler_.Reset();
        switchState(InstrumentState::OUTOFSYNC, InstrumentState::SYNC);
        std::cout << "State : OUTOFSYNC -> SYNC" << '\n';
        mdhandler_.OnSnapshot(refresh, transacttime);
      }
    } else if (current_state == InstrumentState::SYNC && snpt_seqnum > processed_rptseq_) {
      std::cout << "WARNING: Snapshot feed is being received faster than incremental - fast forwarding" << '\n';
      processed_rptseq_ = snpt_seqnum;
      mdhandler_.Reset();
      mdhandler_.OnSnapshot(refresh, transacttime);
    }
  }

  void InstrumentController::OnSecurityStatus(SecurityStatus30 &status) {
    if ((status.securityID() == instrument_.securityid) ||
        (status.securityID() == status.securityIDNullValue() && status.securityGroup() == instrument_.securitygroup)) {
      mdhandler_.OnSecurityStatus(status);
    }
  }

  void InstrumentController::OnChannelReset() {
    processed_rptseq_ = 0;
    // TODO: clear incremental queue? will automatically occur as seqnum < expected_seqnum
    if (state_ != InstrumentState::DISCONTINUED) {
      switchState(state_, InstrumentState::SYNC);
      mdhandler_.OnChannelReset();
    }
  }

  InstrumentController::InstrumentController(Instrument instrument, ChannelContext *channelcontext) :
      instrument_(instrument), state_(InstrumentState::INITIAL), mdhandler_(instrument, channelcontext), channel_(channelcontext) {
    channel_->SubscribeToSnapshotsForInstrument(instrument_.securityid);
  }

  void InstrumentController::Commit() {
    mdhandler_.Commit();
  }

}}}
