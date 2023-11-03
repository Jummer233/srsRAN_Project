/*
 *
 * Copyright 2021-2023 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "tx_buffer_pool_impl.h"

using namespace srsran;

namespace fmt {

/// Default formatter for tx_buffer_identifier.
template <>
struct formatter<srsran::tx_buffer_identifier> {
  template <typename ParseContext>
  auto parse(ParseContext& ctx) -> decltype(ctx.begin())
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const srsran::tx_buffer_identifier& value, FormatContext& ctx)
      -> decltype(std::declval<FormatContext>().out())
  {
    return format_to(ctx.out(), "rnti={:#x} h_id={}", value.rnti, value.harq_ack_id);
  }
};

} // namespace fmt

unique_tx_buffer
tx_buffer_pool_impl::reserve_buffer(const slot_point& slot, const tx_buffer_identifier& id, unsigned nof_codeblocks)
{
  std::unique_lock<std::mutex> lock(mutex);
  slot_point                   expire_slot = slot + expire_timeout_slots;

  // Look for the same identifier within the reserved buffers.
  for (unsigned i_buffer : reserved_buffers) {
    tx_buffer_impl& buffer = *buffer_pool[i_buffer];
    if (buffer.match_id(id)) {
      // Reserve buffer.
      tx_buffer_status status = buffer.reserve(id, expire_slot, nof_codeblocks);
      if (status != tx_buffer_status::successful) {
        logger.set_context(slot.sfn(), slot.slot_index());
        logger.warning("DL HARQ {}: failed to reserve, {}.", id, to_string(status));

        // If the reservation failed, return an invalid buffer.
        return unique_tx_buffer();
      }

      return unique_tx_buffer(buffer);
    }
  }

  // If no available buffer is available, return an invalid buffer.
  if (available_buffers.empty()) {
    logger.set_context(slot.sfn(), slot.slot_index());
    logger.warning("DL HARQ {}: failed to reserve, insufficient buffers in the pool.", id);
    return unique_tx_buffer();
  }

  // Select the first available buffer.
  unsigned buffer_i = available_buffers.top();

  // Select an available buffer.
  tx_buffer_impl& buffer = *buffer_pool[buffer_i];

  // Try to reserve codeblocks.
  tx_buffer_status status = buffer.reserve(id, expire_slot, nof_codeblocks);

  // If the reservation failed, return an invalid buffer.
  if (status != tx_buffer_status::successful) {
    logger.set_context(slot.sfn(), slot.slot_index());
    logger.warning("DL HARQ {}: failed to reserve, {}.", id, to_string(status));
    return unique_tx_buffer();
  }

  // Move the buffer to reserved list and remove from available if the reservation was successful.
  unique_tx_buffer unique_buffer(buffer);
  reserved_buffers.push(buffer_i);
  available_buffers.pop();
  return unique_buffer;
}

unique_tx_buffer tx_buffer_pool_impl::reserve_buffer(const slot_point& slot, unsigned nof_codeblocks)
{
  std::unique_lock<std::mutex> lock(mutex);
  slot_point                   expire_slot = slot + 1;
  tx_buffer_identifier         id          = {};

  // If no available buffer is available, return an invalid buffer.
  if (available_buffers.empty()) {
    logger.set_context(slot.sfn(), slot.slot_index());
    logger.warning("DL HARQ {}: failed to reserve, insufficient buffers in the pool.", id);
    return unique_tx_buffer();
  }

  // Select the first available buffer.
  unsigned buffer_i = available_buffers.top();

  // Select an available buffer.
  tx_buffer_impl& buffer = *buffer_pool[buffer_i];

  // Try to reserve codeblocks.
  tx_buffer_status status = buffer.reserve(id, expire_slot, nof_codeblocks);

  // If the reservation failed, return an invalid buffer.
  if (status != tx_buffer_status::successful) {
    logger.set_context(slot.sfn(), slot.slot_index());
    logger.warning("DL HARQ {}: failed to reserve, {}.", id, to_string(status));
    return unique_tx_buffer();
  }

  // Move the buffer to reserved list and remove from available if the reservation was successful.
  unique_tx_buffer unique_buffer(buffer);
  reserved_buffers.push(buffer_i);
  available_buffers.pop();
  return unique_buffer;
}

void tx_buffer_pool_impl::run_slot(const slot_point& slot)
{
  std::unique_lock<std::mutex> lock(mutex);

  // Run TTI for each reserved buffer.
  unsigned count = reserved_buffers.size();
  for (unsigned i = 0; i != count; ++i) {
    // Pop top reserved buffer identifier.
    unsigned buffer_i = reserved_buffers.top();
    reserved_buffers.pop();

    // Select buffer.
    tx_buffer_impl& buffer = *buffer_pool[buffer_i];

    // Run buffer slot.
    bool available = buffer.run_slot(slot);

    // Return buffer to queue.
    if (available) {
      available_buffers.push(buffer_i);
    } else {
      reserved_buffers.push(buffer_i);
    }
  }
}

std::unique_ptr<tx_buffer_pool> srsran::create_tx_buffer_pool(const tx_buffer_pool_config& config)
{
  return std::make_unique<tx_buffer_pool_impl>(config);
}
