/*
 *
 * Copyright 2021-2024 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "rrc_ue_helpers.h"
#include "rrc_ue_impl.h"

using namespace srsran;
using namespace srs_cu_cp;
using namespace asn1::rrc_nr;

void rrc_ue_impl::send_dl_ccch(const dl_ccch_msg_s& dl_ccch_msg)
{
  // pack DL CCCH msg
  byte_buffer pdu = pack_into_pdu(dl_ccch_msg);

  // Log Tx message
  log_rrc_message(logger, Tx, pdu, dl_ccch_msg, "CCCH DL");

  // send down the stack
  logger.log_debug(pdu.begin(), pdu.end(), "TX {} PDU", srb_id_t::srb0);
  f1ap_pdu_notifier.on_new_rrc_pdu(srb_id_t::srb0, std::move(pdu));
}

void rrc_ue_impl::send_dl_dcch(srb_id_t srb_id, const dl_dcch_msg_s& dl_dcch_msg)
{
  if (context.srbs.find(srb_id) == context.srbs.end()) {
    logger.log_error("Dropping DlDcchMessage. TX {} is not set up", srb_id);
    return;
  }

  // pack DL CCCH msg
  byte_buffer pdu = pack_into_pdu(dl_dcch_msg);

  // Log Tx message
  log_rrc_message(logger, Tx, pdu, dl_dcch_msg, "DCCH DL");

  // pack PDCP PDU and send down the stack
  byte_buffer pdcp_pdu = context.srbs.at(srb_id).pack_rrc_pdu(std::move(pdu));
  logger.log_debug(pdcp_pdu.begin(), pdcp_pdu.end(), "TX {} PDU", context.ue_index, context.c_rnti, srb_id);
  f1ap_pdu_notifier.on_new_rrc_pdu(srb_id, std::move(pdcp_pdu));
}
