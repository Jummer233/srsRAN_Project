/*
 *
 * Copyright 2021-2023 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#pragma once

#include "du_repository.h"
#include "mobility_manager_config.h"
#include "ue_manager.h"
#include "srsran/cu_cp/cu_cp_types.h"
#include "srsran/ran/pci.h"

namespace srsran {
namespace srs_cu_cp {

/// Methods used by the mobility manager to initiate handover procedures.
class mobility_manager_cu_cp_notifier
{
public:
  virtual ~mobility_manager_cu_cp_notifier() = default;

  /// \brief Notify the CU-CP to perform a Inter DU handover
  /// \param[in] ue_index The UE index to be handed over to the new cell.
  /// \param[in] target_pci The PCI of the target cell.
  virtual void on_inter_du_handover_request(ue_index_t ue_index, pci_t target_pci) = 0;
};

/// Handler for measurement related events.
class mobility_manager_measurement_handler
{
public:
  virtual ~mobility_manager_measurement_handler() = default;

  /// \brief Handle event where neighbor became better than serving cell.
  virtual void handle_neighbor_better_than_spcell(ue_index_t ue_index, pci_t neighbor_pci) = 0;
};

/// Object to manage mobility. An instance of this class resides in the CU-CP and handles all kinds of events that might
/// trigger the change of the serving cell of a user. It consumes (measurement) events from local cells as well as from
/// cells not managed by the CU-CP itself. As such it checks the requests and dispatches them to perform:
/// * Intra DU handover (delegate to DU processor)
/// * Inter DU handover (delegate to CU-CP)
/// * Inter CU handover over N2 (delegate to CU-CP/NGAP)
class mobility_manager : public mobility_manager_measurement_handler
{
public:
  virtual ~mobility_manager() = default;
};

/// Creates an instance of an cell measurement manager.
std::unique_ptr<mobility_manager>
create_mobility_manager(const mobility_manager_cfg& cfg_, du_repository& du_db_, du_processor_ue_manager& ue_mng_);

} // namespace srs_cu_cp
} // namespace srsran
