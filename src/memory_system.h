//
// Created by ryotta205 on 8/17/22.
//

#ifndef DRAMFAULTSIM_MEMORY_SYSTEM_H
#define DRAMFAULTSIM_MEMORY_SYSTEM_H

#include "common.h"
#include "configuration.h"
#include "faultmodel.h"
#include "stat.h"
#include "faultresult.h"
#include "faulttrace.h"

namespace dramfaultsim {

    class MemorySystem {
    public:
        MemorySystem(Config &config, Stat &stat, FaultResult &fault_result, FaultTrace &fault_trace)
                : config_(config), stat_(stat), fault_result_(fault_result), fault_trace_(fault_trace) {}

        virtual ~MemorySystem() {};

        virtual void RecvRequest(uint64_t addr, bool is_write, uint64_t *data) = 0;

        virtual void Read(uint64_t *data) = 0;

        virtual void Write(uint64_t *data) = 0;

        virtual void FaultData(uint64_t *data) = 0;

        virtual void CheckFaultMask() = 0;

    protected:
        Config &config_;
        Stat &stat_;
        FaultResult &fault_result_;
        FaultTrace &fault_trace_;
        Address recv_addr_;
        FaultModel *faultmodel_;

        //data_block_[Channel][Rank][BankGourp][Bank][Row][Col]
        uint64_t *******data_block_;
        int recv_addr_channel;
        int recv_addr_rank;
        int recv_addr_bankgroup;
        int recv_addr_bank;
        int recv_addr_row;
        int recv_addr_column;

        //For FaultModel Return Value
        uint64_t *fault_mask;
        uint64_t *fault_data;

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

    };

    class NaiveMemorySystem : public MemorySystem {
    public:
        NaiveMemorySystem(Config &config, Stat &stat, FaultResult &fault_result, FaultTrace &fault_trace);

        ~NaiveMemorySystem() override;

        void RecvRequest(uint64_t addr, bool is_write, uint64_t *data) override;

        void Read(uint64_t *data) override;

        void Write(uint64_t *data) override;

        void FaultData(uint64_t *data) override;

        void CheckFaultMask() override;

        int FaultCountInColumn(int BL);

        void PrintFaultTrace();

    protected:


    private:
        std::random_device rd;
        std::ofstream writer_;
#ifdef TEST_MODE
        std::mt19937_64 gen;
#endif


    };
}

#endif //DRAMFAULTSIM_MEMORY_SYSTEM_H
