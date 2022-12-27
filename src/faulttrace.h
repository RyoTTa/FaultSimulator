//
// Created by ryotta205 on 9/22/22.
//

#ifndef DRAMFAULTSIM_FAULTTRACE_H
#define DRAMFAULTSIM_FAULTTRACE_H

#include "configuration.h"
#include "stat.h"
#include <iomanip>
#include <fstream>

namespace dramfaultsim{
    class FaultTrace{
    public:

        FaultTrace(Config &config, Stat &stat_);

        void ResetFaultTrace();
        void SetFaultTrace();
        //void PrintFaultTrace(uint64_t addr, int BL, uint64_t *fault_mask, uint64_t *data);
        void PrintFaultTrace(uint64_t addr, uint64_t *correct_data, uint64_t *fault_data);

        void SetRecvAddress(uint64_t addr) {
            recv_addr_ = config_.AddressMapping(addr);

            recv_addr_channel = recv_addr_.channel;
            recv_addr_rank = recv_addr_.rank;
            recv_addr_bankgroup = recv_addr_.bankgroup;
            recv_addr_bank = recv_addr_.bank;
            recv_addr_row = recv_addr_.row;
            recv_addr_column = recv_addr_.column;

            return;
        }

        int recv_addr_channel;
        int recv_addr_rank;
        int recv_addr_bankgroup;
        int recv_addr_bank;
        int recv_addr_row;
        int recv_addr_column;

        Config &config_;
        Stat &stat_;
        std::ofstream writer_;
        Address recv_addr_;
    };
}
#endif //DRAMFAULTSIM_FAULTTRACE_H
