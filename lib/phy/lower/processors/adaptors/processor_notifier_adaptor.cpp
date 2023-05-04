/*
 *
 * Copyright 2013-2022 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "processor_notifier_adaptor.h"
#include "srsran/phy/support/resource_grid_context.h"
#include "srsran/support/srsran_assert.h"

using namespace srsran;

void processor_notifier_adaptor::downlink_adaptor::on_tti_boundary(const lower_phy_timing_context& context)
{
  srsran_assert(timing_notifier, "The adaptor is not connected to a timing notifier.");
  timing_notifier->on_tti_boundary(context);
}

void processor_notifier_adaptor::uplink_adaptor::on_half_slot(const lower_phy_timing_context& context)
{
  srsran_assert(timing_notifier, "The adaptor is not connected to a timing notifier.");
  timing_notifier->on_ul_half_slot_boundary(context);
}

void processor_notifier_adaptor::uplink_adaptor::on_full_slot(const lower_phy_timing_context& context)
{
  srsran_assert(timing_notifier, "The adaptor is not connected to a timing notifier.");
  timing_notifier->on_ul_full_slot_boundary(context);
}

void processor_notifier_adaptor::pdxch_adaptor::on_late_resource_grid(const resource_grid_context& context)
{
  srsran_assert(error_notifier, "The adaptor is not connected to an error notifier.");
  error_notifier->on_late_resource_grid(context);
}

void processor_notifier_adaptor::pdxch_adaptor::on_overflow_resource_grid(const resource_grid_context& context)
{
  srsran_assert(error_notifier, "The adaptor is not connected to an error notifier.");
  error_notifier->on_overflow_resource_grid(context);
}

void processor_notifier_adaptor::prach_adaptor::on_prach_request_late(const prach_buffer_context& context)
{
  srsran_assert(error_notifier, "The adaptor is not connected to an error notifier.");
  error_notifier->on_prach_request_late(context);
}

void processor_notifier_adaptor::prach_adaptor::on_prach_request_overflow(const prach_buffer_context& context)
{
  srsran_assert(error_notifier, "The adaptor is not connected to an error notifier.");
  error_notifier->on_prach_request_overflow(context);
}

void processor_notifier_adaptor::prach_adaptor::on_rx_prach_window(const prach_buffer&         buffer,
                                                                   const prach_buffer_context& context)
{
  srsran_assert(rx_notifier, "The adaptor is not connected to receiver notifier.");
  rx_notifier->on_rx_prach_window(context, buffer);
}

void processor_notifier_adaptor::puxch_adaptor::on_puxch_request_late(const resource_grid_context& context)
{
  srsran_assert(error_notifier, "The adaptor is not connected to an error notifier.");
  error_notifier->on_puxch_request_late(context);
}

void processor_notifier_adaptor::puxch_adaptor::on_puxch_request_overflow(const resource_grid_context& context)
{
  srsran_assert(error_notifier, "The adaptor is not connected to an error notifier.");
  error_notifier->on_puxch_request_overflow(context);
}

void processor_notifier_adaptor::puxch_adaptor::on_rx_symbol(const resource_grid_reader&        grid,
                                                             const lower_phy_rx_symbol_context& context)
{
  srsran_assert(error_notifier, "The adaptor is not connected to an error notifier.");
  rx_notifier->on_rx_symbol(context, grid);
}