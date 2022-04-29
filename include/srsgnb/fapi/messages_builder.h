#ifndef SRSGNB_FAPI_MESSAGES_BUILDER_H
#define SRSGNB_FAPI_MESSAGES_BUILDER_H

#include "srsgnb/fapi/messages.h"
#include "srsgnb/ran/pci.h"
#include "srsgnb/ran/ssb_mapping.h"
#include "srsgnb/support/math_utils.h"

namespace srsgnb {
namespace fapi {

/// DL SSB pdu builder that helps to fill the parameters specified in SCF-222 v4.0 section 3.4.2.4.
class dl_ssb_pdu_builder
{
public:
  explicit dl_ssb_pdu_builder(dl_ssb_pdu& pdu) : pdu(pdu), v3(pdu.ssb_maintenance_v3)
  {
    v3.ss_pbch_block_power_scaling = std::numeric_limits<int16_t>::min();
    v3.beta_pss_profile_sss        = std::numeric_limits<int16_t>::min();
  }

  /// Sets the basic parameters for the fields of the SSB/PBCH PDU.
  /// \note these parameters are specified in SCF-222 v4.0 section 3.4.2.4, in table SSB/PBCH PDU.
  dl_ssb_pdu_builder& set_basic_parameters(pci_t                 phys_cell_id,
                                           beta_pss_profile_type beta_pss_profile_nr,
                                           uint8_t               ssb_block_index,
                                           uint8_t               ssb_subcarrier_offset,
                                           uint16_t              ssb_offset_pointA)
  {
    pdu.phys_cell_id          = phys_cell_id;
    pdu.beta_pss_profile_nr   = beta_pss_profile_nr;
    pdu.ssb_block_index       = ssb_block_index;
    pdu.ssb_subcarrier_offset = ssb_subcarrier_offset;
    pdu.ssb_offset_pointA     = ssb_offset_pointA;

    return *this;
  }

  /// Sets the BCH payload configured by the MAC and returns a reference to the builder.
  /// \note use this function when the MAC generates the full PBCH payload.
  /// \note these parameters are specified in SCF-222 v4.0 section 3.4.2.4, in table MAC generated MIB PDU.
  /// \note this function assumes that given bch_payload value is codified as: a0,a1,a2,...,a29,a30,a31, with the most
  /// significant bit the leftmost bit (in this case a0 in position 31 of the uint32_t).
  dl_ssb_pdu_builder& set_bch_payload_mac_full(uint32_t bch_payload)
  {
    // Configure BCH payload to full MAC.
    pdu.bch_payload_flag        = bch_payload_type::mac_full;
    pdu.bch_payload.bch_payload = bch_payload;

    return *this;
  }

  /// Sets the BCH payload and returns a reference to the builder. PHY configures the timing PBCH bits.
  /// \note use this function when the PHY generates the timing PBCH information.
  /// \note these parameters are specified in SCF-222 v4.0 section 3.4.2.4, in table MAC generated MIB PDU.
  /// \note this function assumes that given bch_payload value is codified as: 0,0,0,0,0,0,0,0,a0,a1,a2,...,a21,a22,a23,
  /// with the most significant bit the leftmost bit (in this case a0 in position 24 of the uint32_t).
  dl_ssb_pdu_builder& set_bch_payload_phy_timing_info(uint32_t bch_payload)
  {
    pdu.bch_payload_flag = bch_payload_type::phy_timing_info;
    // Use the 24 LSB.
    pdu.bch_payload.bch_payload = (bch_payload & 0xFFFFFF);

    return *this;
  }

  /// Sets the BCH payload configured by the PHY and returns a reference to the builder.
  /// \note use this function when the PHY generates the full PBCH payload.
  /// \note these parameters are specified in SCF-222 v4.0 section 3.4.2.4, in table PHY generated MIB PDU.
  dl_ssb_pdu_builder& set_bch_payload_phy_full(uint8_t dmrs_type_a_position,
                                               uint8_t pdcch_config_sib1,
                                               uint8_t cell_barred,
                                               uint8_t intra_freq_reselection)
  {
    pdu.bch_payload_flag                              = bch_payload_type::phy_full;
    pdu.bch_payload.phy_mib_pdu.dmrs_typeA_position   = dmrs_type_a_position;
    pdu.bch_payload.phy_mib_pdu.pdcch_config_sib1     = pdcch_config_sib1;
    pdu.bch_payload.phy_mib_pdu.cell_barred           = cell_barred;
    pdu.bch_payload.phy_mib_pdu.intrafreq_reselection = intra_freq_reselection;

    return *this;
  }

  /// Sets the maintenance v3 basic parameters and returns a reference to the builder.
  /// \note these parameters are specified in SCF-222 v4.0 section 3.4.2.4, in table SSB/PBCH PDU maintenance FAPIv3.
  /// \note ssbPduIndex field is automatically filled when adding a new SSB pdu to the DL TTI request message.
  dl_ssb_pdu_builder&
  set_maintenance_v3_basic_parameters(ssb_pattern_case case_type, subcarrier_spacing scs, uint8_t l_max)
  {
    v3.case_type = case_type;
    v3.scs       = scs;
    v3.lmax      = l_max;

    return *this;
  }

  /// Sets the SSB power information and returns a reference to the builder.
  /// \note these parameters are specified in SCF-222 v4.0 section 3.4.2.4, in table SSB/PBCH PDU maintenance FAPIv3.
  dl_ssb_pdu_builder& set_maintenance_v3_tx_power_info(float power_scaling_ss_pbch_dB, float pss_to_sss_ratio_dB)
  {
    // Power scaling in SS-PBCH in hundredth of dB.
    int ss_block_power = power_scaling_ss_pbch_dB * 100;
    srsran_assert(ss_block_power <= std::numeric_limits<int16_t>::max(),
                  "SS PBCH block power scaling ({}) exceeds the maximum ({}).",
                  ss_block_power,
                  std::numeric_limits<int16_t>::max());
    srsran_assert(ss_block_power >= std::numeric_limits<int16_t>::min(),
                  "SS PBCH block power scaling ({}) exceeds the minimum ({}).",
                  ss_block_power,
                  std::numeric_limits<int16_t>::min());
    v3.ss_pbch_block_power_scaling = static_cast<int16_t>(ss_block_power);

    int beta_pss = pss_to_sss_ratio_dB * 1000;
    // SSS to PSS ratio in thousandth of dB.
    srsran_assert(beta_pss <= std::numeric_limits<int16_t>::max(),
                  "PSS to SSS ratio ({}) exceeds the maximum ({}).",
                  beta_pss,
                  std::numeric_limits<int16_t>::max());
    srsran_assert(beta_pss >= std::numeric_limits<int16_t>::min(),
                  "PSS to SSS ratio ({}) does not reach the minimum ({}).",
                  beta_pss,
                  std::numeric_limits<int16_t>::min());

    v3.beta_pss_profile_sss = static_cast<int16_t>(beta_pss);

    return *this;
  }

  //: TODO: params v4 - MU-MIMO.
  // :TODO: beamforming.
private:
  dl_ssb_pdu&            pdu;
  dl_ssb_maintenance_v3& v3;
};

/// DL PDCCH pdu builder. Helps with the pdu build.
class dl_pdcch_pdu_builder
{
public:
  explicit dl_pdcch_pdu_builder(dl_pdcch_pdu& pdu) : pdu(pdu) {}

  // :TODO: add rest of parameters.
  dl_pdcch_pdu_builder& set_basic_parameters(subcarrier_spacing scs)
  {
    pdu.scs = scs;

    return *this;
  }

private:
  dl_pdcch_pdu& pdu;
};

/// DL PDSCH pdu builder. Helps with the pdu build.
class dl_pdsch_pdu_builder
{
public:
  explicit dl_pdsch_pdu_builder(dl_pdsch_pdu& pdu) : pdu(pdu) {}

  // :TODO: add rest of parameters.
  dl_pdsch_pdu_builder& set_basic_parameters(subcarrier_spacing scs)
  {
    pdu.scs = scs;

    return *this;
  }

private:
  dl_pdsch_pdu& pdu;
};

/// DL CSI RS pdu builder. Helps with the pdu build.
class dl_csi_rs_pdu_builder
{
public:
  explicit dl_csi_rs_pdu_builder(dl_csi_rs_pdu& pdu) : pdu(pdu) {}

  // :TODO: add rest of parameters.
  dl_csi_rs_pdu_builder& set_basic_parameters(subcarrier_spacing scs)
  {
    pdu.scs = scs;

    return *this;
  }

private:
  dl_csi_rs_pdu& pdu;
};

/// DL TTI request message builder that helps to fill the parameters specified in SCF-222 v4.0 section 3.4.2.
class dl_tti_request_message_builder
{
  /// Maximum number of DL PDU types supported. The value is specified in SCF-222 v4.0 section 3.4.2.
  static constexpr unsigned NUM_DL_TYPES = 5;

public:
  /// Constructs a builder that will help to fill the given DL TTI request message.
  explicit dl_tti_request_message_builder(dl_tti_request_message& msg) : msg(msg) { msg.num_dl_types = NUM_DL_TYPES; }

  /// Sets the DL TTI request basic parameters and returns a reference to the builder.
  /// \note nPDUs and nPDUsOfEachType properties are filled by the add_*_pdu() functions.
  /// \note these parameters are specified in SCF-222 v4.0 section 3.4.2 in table DL_TTI.request message body.
  dl_tti_request_message_builder& set_basic_parameters(uint16_t sfn, uint16_t slot, uint16_t n_group)
  {
    msg.sfn        = sfn;
    msg.slot       = slot;
    msg.num_groups = n_group;

    return *this;
  }

  /// Adds a PDCCH pdu to the message and returns a PDCCH pdu builder.
  dl_pdcch_pdu_builder add_pdcch_pdu()
  {
    ++msg.num_pdus_of_each_type[static_cast<int>(dl_pdu_type::PDCCH)];
    // :TODO: Need to fill the number of DLDCIs across all PDCCH PDU in the message.

    // Add a new pdu.
    msg.pdus.emplace_back();
    dl_tti_request_pdu& pdu = msg.pdus.back();
    pdu.pdu_type            = dl_pdu_type::PDCCH;

    dl_pdcch_pdu_builder builder(pdu.pdcch_pdu);

    return builder;
  }

  /// Adds a PDSCH pdu to the message and returns a PDSCH pdu builder.
  dl_pdsch_pdu_builder add_pdsch_pdu()
  {
    ++msg.num_pdus_of_each_type[static_cast<int>(dl_pdu_type::PDSCH)];

    // Add a new pdu.
    msg.pdus.emplace_back();
    dl_tti_request_pdu& pdu = msg.pdus.back();
    pdu.pdu_type            = dl_pdu_type::PDSCH;

    dl_pdsch_pdu_builder builder(pdu.pdsch_pdu);

    return builder;
  }

  /// Adds a CSI-RS pdu to the message and returns a CSI-RS pdu builder.
  dl_csi_rs_pdu_builder add_csi_rs_pdu()
  {
    ++msg.num_pdus_of_each_type[static_cast<int>(dl_pdu_type::CSI_RS)];

    // Add a new pdu.
    msg.pdus.emplace_back();
    dl_tti_request_pdu& pdu = msg.pdus.back();
    pdu.pdu_type            = dl_pdu_type::CSI_RS;

    dl_csi_rs_pdu_builder builder(pdu.csi_rs_pdu);

    return builder;
  }

  /// Adds a SSB pdu to the message, fills its basic parameters using the arguments and returns a SSB pdu builder.
  dl_ssb_pdu_builder add_ssb_pdu(pci_t                 phys_cell_id,
                                 beta_pss_profile_type beta_pss_profile_nr,
                                 uint8_t               ssb_block_index,
                                 uint8_t               ssb_subcarrier_offset,
                                 uint16_t              ssb_offset_pointA)
  {
    // Add a new pdu.
    msg.pdus.emplace_back();
    dl_tti_request_pdu& pdu = msg.pdus.back();

    // Fill the ssb pdu index value. The index value will be the index of the pdu in the array of SSB pdus.
    dl_ssb_maintenance_v3& info        = pdu.ssb_pdu.ssb_maintenance_v3;
    auto&                  num_ssb_pdu = msg.num_pdus_of_each_type[static_cast<int>(dl_pdu_type::SSB)];
    info.ssb_pdu_index                 = num_ssb_pdu;

    // Increase the number of SSB pdus in the request.
    ++num_ssb_pdu;

    pdu.pdu_type = dl_pdu_type::SSB;

    dl_ssb_pdu_builder builder(pdu.ssb_pdu);

    // Fills the pdu's basic parameters.
    builder.set_basic_parameters(
        phys_cell_id, beta_pss_profile_nr, ssb_block_index, ssb_subcarrier_offset, ssb_offset_pointA);

    return builder;
  }

  //: TODO: groups array
  //: TODO: top level rate match patterns

private:
  dl_tti_request_message& msg;
};

} // namespace fapi
} // namespace srsgnb

#endif // SRSGNB_FAPI_MESSAGES_BUILDER_H
