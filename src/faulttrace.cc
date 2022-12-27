//
// Created by ryotta205 on 9/22/22.
//
//
// Created by ryotta205 on 8/31/22.
//
#include "faulttrace.h"

namespace dramfaultsim{

    FaultTrace::FaultTrace(Config &config, Stat &stat)
            : config_(config), stat_(stat){
    }

    void FaultTrace::PrintFaultTrace(uint64_t addr, uint64_t *correct_data, uint64_t *fault_data) {
        SetRecvAddress(addr);

        writer_ << std::hex << recv_addr_.hex_addr << " ";
        for (int i = 0; i < config_.BL; i++) {
            writer_ << std::hex
                    << correct_data[i]
                    << " ";
        }
        //writer_ << "\n";
        for (int i = 0; i < config_.BL; i++) {
            writer_ << std::hex << fault_data[i] << " ";
        }
        writer_ << "\n";
    }
    void FaultTrace::SetFaultTrace(){
        writer_.open(
                config_.output_dir + "/" + config_.output_prefix + "_Round_" + std::to_string(stat_.repeat_round) +
                "_FaultTrace.txt",
                std::ios::out | std::ios::trunc);
        if (writer_.fail()) {
            std::cerr << "Can't write fault result file - " << config_.output_dir << "/" << config_.output_prefix
                      << "_Round_" << stat_.repeat_round << "_FaultTrace.txt" << std::endl;
            AbruptExit(__FILE__, __LINE__);
        }

        //writer_ << ",Round,Id,Channel,Rank,BankGroup,Bank,Row,Col,Dev,Bit,Result\n";
    }

    void FaultTrace::ResetFaultTrace() {
        writer_.close();
    }
}