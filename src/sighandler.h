//
// Created by ryotta205 on 9/23/22.
//

#ifndef DRAMFAULTSIM_SIGHANDLER_H
#define DRAMFAULTSIM_SIGHANDLER_H
#include <iostream>

#include <stdarg.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

#include "configuration.h"
#include "generator.h"
#include "stat.h"
#include "faultresult.h"
#include "faulttrace.h"


using namespace std;

using namespace dramfaultsim;

class SignalHandler {
private :
    static void signal_handler(int _sig_num);
    void print_debug_info(const char *_format, ...);

    int		num_sig_handle;
    bool	is_debug_print;

public :
    SignalHandler(Generator &generator, Stat &stat, Config &config, FaultResult &fault_result, FaultTrace &fault_trace);
    ~SignalHandler(void);

    void set_debug_print(void);

    void print_result(int _sig_num);

    void set_signal(int _sig_num);
    void set_ignore(int _sig_num);
    void set_signal_handler(void (*_func)(int));

    bool is_term(void);

    Generator &generator_;
    Stat &stat_;
    Config &config_;
    FaultResult &fault_result_;
    FaultTrace &fault_trace_;
};
#endif //DRAMFAULTSIM_SIGHANDLER_H
