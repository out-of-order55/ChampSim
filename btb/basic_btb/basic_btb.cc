
/*
 * This file implements a basic Branch Target Buffer (BTB) structure.
 * It uses a set-associative BTB to predict the targets of non-return branches,
 * and it uses a small Return Address Stack (RAS) to predict the target of
 * returns.
 */

#include "basic_btb.h"

#include "instruction.h"
// #include "ooo_cpu.h"
std::tuple<champsim::address, bool,bool> basic_btb::btb_prediction(champsim::address ip)
{
  // use BTB for all other branches + direct calls
  auto btb_entry = direct.check_hit(ip);


  // no prediction for this IP
  if (!btb_entry.has_value())
    return {champsim::address{}, false,false};

  if (btb_entry->type == direct_predictor::branch_info::RETURN){
    auto [tgt, ok] = ras.prediction();
    return {tgt, ok, true};
  }
  if (btb_entry->type == direct_predictor::branch_info::INDIRECT){
    auto [tgt, ok] = indirect.prediction(ip);
    return {tgt, ok, true};
  }
    return {btb_entry->target, btb_entry->type != direct_predictor::branch_info::CONDITIONAL,true};
}

void basic_btb::update_btb(champsim::address ip, champsim::address branch_target, bool taken, uint8_t branch_type)
{
  // add something to the RAS
  if (branch_type == BRANCH_DIRECT_CALL || branch_type == BRANCH_INDIRECT_CALL)
    ras.push(ip);

  // updates for indirect branches
  if ((branch_type == BRANCH_INDIRECT) || (branch_type == BRANCH_INDIRECT_CALL))
    indirect.update_target(ip, branch_target);

  if (branch_type == BRANCH_CONDITIONAL)
    indirect.update_direction(taken);

  if (branch_type == BRANCH_RETURN)
    ras.calibrate_call_size(branch_target);

  direct.update(ip, branch_target, branch_type);
}
