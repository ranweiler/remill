/*
 * Copyright (c) 2017 Trail of Bits, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

namespace {

DEF_SEM(CallSupervisor, I32 imm) {
  HYPER_CALL = AsyncHyperCall::kAArch64SupervisorCall;
  HYPER_CALL_VECTOR = Read(imm);
  return memory;
}

DEF_SEM(Breakpoint, I32 imm) {
  HYPER_CALL_VECTOR = Read(imm);
  return __remill_sync_hyper_call(
      state, memory, SyncHyperCall::kAArch64Breakpoint);
}

DEF_SEM(DoMRS_RS_SYSTEM_FPSR, R64W dest) {
  auto fpsr = state.fpsr;
  fpsr.ixc = state.sr.ixc;
  fpsr.ofc = state.sr.ofc;
  fpsr.ufc = state.sr.ufc;
  fpsr.idc = state.sr.idc;
  fpsr.ioc = state.sr.ioc;
  WriteZExt(dest, fpsr.flat);
  return memory;
}

DEF_SEM(DoMSR_SR_SYSTEM_FPSR, R64W src) {
  FPSR fpsr;
  WriteZExt(fpsr.flat, Read(src));
  fpsr._res0 = 0;
  fpsr._res1 = 0;
  state.fpsr = fpsr;
  state.sr.ioc = fpsr.ioc;
  state.sr.ofc = fpsr.ofc;
  state.sr.ixc = fpsr.ixc;
  state.sr.ufc = fpsr.ufc;
  state.sr.idc = fpsr.idc;
  return memory;
}

DEF_SEM(DataMemoryBarrier) {
  // TODO(pag): Full-system data memory barrier probably requires a synchronous
  //            hypercall if it behaves kind of like Linux's `sys_membarrier`.
  return __remill_barrier_store_store(memory);
}

}  // namespace

DEF_ISEL(SVC_EX_EXCEPTION) = CallSupervisor;
DEF_ISEL(BRK_EX_EXCEPTION) = Breakpoint;
DEF_ISEL(MRS_RS_SYSTEM_FPSR) = DoMRS_RS_SYSTEM_FPSR;
DEF_ISEL(MSR_SR_SYSTEM_FPSR) = DoMSR_SR_SYSTEM_FPSR;
DEF_ISEL(DMB_BO_SYSTEM) = DataMemoryBarrier;
