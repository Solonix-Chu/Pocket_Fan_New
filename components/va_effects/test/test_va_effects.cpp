#include "unity.h"
#include "va_effects.h"

TEST_CASE("VaEffect compilation check", "[va_effects]")
{
    // Just checking if we can link against the class
    // Since methods are static and currently empty/commented out, 
    // we just verify header inclusion works.
    TEST_ASSERT_TRUE(true);
}
