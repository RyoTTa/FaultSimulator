//
// Created by ryotta205 on 8/19/22.
//
#include "common.h"
#include "configuration.h"
#include "faultmap.h"
#include "stat.h"

#ifndef DRAMFAULTSIM_FAULTMODEL_H
#define DRAMFAULTSIM_FAULTMODEL_H
namespace dramfaultsim {


    class FaultModel {
    public:
        FaultModel(Config &config, uint64_t *******data_block, Stat &stat)
                : config_(config), data_block_(data_block), stat_(stat) {};

        virtual ~FaultModel() {};

        virtual uint64_t *ErrorInjection(uint64_t addr) = 0;

        virtual void HardFaultError() = 0;

        virtual void VRTFaultError() = 0;

        virtual void DataDependentFaultError() = 0;
        //virtual uint64_t VRTError() = 0;

        double GetRandomDobule(double low, double high) {
            std::uniform_real_distribution<double> dist(low, high);
            std::mt19937_64 gen(rd());

            //std::cout << dist(gen) << std::endl;
            return dist(gen);
        }

        int GetRandomInt(int low, int high) {
            std::uniform_int_distribution<> dist(low, high);
            std::mt19937_64 gen(rd());

            //std::cout << dist(gen) << std::endl;
            return dist(gen);
        }

        double GetNormalDouble(int m, int s) {
            std::normal_distribution<double> dist(m, s);
            std::mt19937_64 gen(rd());

            //std::cout << dist(gen) << std::endl;
            return dist(gen);
        }

        int GetNormalInt(int m, int s) {
            std::normal_distribution<double> dist(m, s);
            std::mt19937_64 gen(rd());

            //std::cout << dist(gen) << std::endl;
            return std::round(dist(gen));
        }

    protected:
        Config &config_;
        Address recv_addr_;
        uint64_t *******data_block_;
        Stat &stat_;

        int recv_addr_channel;
        int recv_addr_rank;
        int recv_addr_bankgroup;
        int recv_addr_bank;
        int recv_addr_row;
        int recv_addr_column;

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

    private:
        std::random_device rd;
#ifdef TEST_MODE
        std::mt19937_64 gen;
#endif
    };


    class NaiveFaultModel : public FaultModel {
    public:
        NaiveFaultModel(Config &config, uint64_t *******data_block, Stat &stat);

        ~NaiveFaultModel() override;

        uint64_t *ErrorInjection(uint64_t addr) override;

        void HardFaultError() override;

        void VRTFaultError() override;

        void DataDependentFaultError() override;

        //uint64_t VRTError() override;

        void HardFaultGenerator();

        void HardFaultGeneratorThread(uint64_t num_generate);

        void VRTErrorGenerator();

        void
        VRTErrorGeneratorThread(uint64_t num_generate_once, uint64_t num_generate_low_low, uint64_t num_generate_low,
                                uint64_t num_generate_mid, uint64_t num_generate_high, uint64_t num_generate_high_high);



    protected:

    private:
        FaultStruct *******fault_map_;
        std::random_device rd;

        uint64_t *ErrorMask;
        uint64_t num_all_cell;
        uint64_t num_hard_fault_cell;
        uint64_t num_vrt_fault_cell;
        uint64_t num_vrt_fault_once_cell;
        uint64_t num_vrt_fault_low_low_cell;
        uint64_t num_vrt_fault_low_cell;
        uint64_t num_vrt_fault_mid_cell;
        uint64_t num_vrt_fault_high_cell;
        uint64_t num_vrt_fault_high_high_cell;

#ifdef TEST_MODE
        std::mt19937_64 gen;
#endif
    };

    class BetaDistFaultModel : public FaultModel {
    public:
        BetaDistFaultModel(Config &config, uint64_t *******data_block, Stat &stat);

        ~BetaDistFaultModel() override;

        uint64_t *ErrorInjection(uint64_t addr) override;

        void HardFaultError() override;

        void HardFaultGenerator();

        void HardFaultGeneratorThread(uint64_t num_generate);


        void VRTFaultError() override;

        void VRTErrorGenerator();

        void VRTErrorGeneratorThread(int thread_id);


        void DataDependentFaultError() override;

    protected:

    private:
        FaultStruct *******fault_map_;
        std::random_device rd;

        uint64_t *ErrorMask;
        uint64_t num_all_cell;
        uint64_t num_fault_cell;

        uint64_t num_fault_array[101];

        uint64_t num_fault_array_ext[1001];

#ifdef TEST_MODE
        std::mt19937_64 gen;
#endif
    };

}
#endif //DRAMFAULTSIM_FAULTMODEL_H
