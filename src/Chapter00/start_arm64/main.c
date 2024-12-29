#include "func.h"
#include <assert.h>
#include <stdint.h>

int main(){
    uint64_t ans = mySum(5);
    assert(ans==15);
}