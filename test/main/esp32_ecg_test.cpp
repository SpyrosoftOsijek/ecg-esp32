#include <iostream>
#include <string.h>
#include "unity.h"

static void print_banner(const char* text);

extern "C" void app_main(void)
{
    print_banner("Running all the registered tests");
    UNITY_BEGIN();
    unity_run_all_tests();
    UNITY_END();
}

static void print_banner(const char* text)
{
    std::cout << "\n#### " << text << " #####\n\n";
}